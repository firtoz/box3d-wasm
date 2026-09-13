//! Split explicit-layout aggregates used outside buffers in Vulkan SPIR-V.
//!
//! Buffer interfaces retain their original declarations. Logical value copies
//! bridge the distinct types at loads/stores, without changing shader arithmetic.
//! This pass targets Naga's logical-addressing SPIR-V 1.4–1.6 output. It is not a
//! replacement for SPIR-V validation; unsupported constructs return an error.
use rspirv::{
    binary::Assemble,
    dr::{self, Instruction, Operand},
    spirv::{AddressingModel, Decoration, Op, StorageClass, Word},
};
use std::collections::{BTreeMap, BTreeSet};

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Error(pub String);
impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.0.fmt(f)
    }
}
impl std::error::Error for Error {}
type Result<T> = std::result::Result<T, Error>;
fn fail<T>(message: impl Into<String>) -> Result<T> {
    Err(Error(message.into()))
}
fn id(operand: Option<&Operand>) -> Result<Word> {
    match operand {
        Some(Operand::IdRef(x)) => Ok(*x),
        _ => fail("expected an ID operand"),
    }
}
fn fresh(next: &mut Word) -> Result<Word> {
    let result = *next;
    *next = next
        .checked_add(1)
        .ok_or_else(|| Error("SPIR-V ID bound overflow".into()))?;
    Ok(result)
}
fn forbidden(d: Decoration) -> bool {
    matches!(
        d,
        Decoration::ArrayStride
            | Decoration::Offset
            | Decoration::MatrixStride
            | Decoration::RowMajor
            | Decoration::ColMajor
            | Decoration::Block
            | Decoration::BufferBlock
    )
}
fn annotation(inst: &Instruction) -> Result<(Word, Decoration)> {
    let at = match inst.class.opcode {
        Op::Decorate => 1,
        Op::MemberDecorate => 2,
        _ => return fail("unsupported annotation"),
    };
    let target = id(inst.operands.first())?;
    match inst.operands.get(at) {
        Some(Operand::Decoration(d)) => Ok((target, *d)),
        _ => fail("expected decoration"),
    }
}
fn children(inst: &Instruction) -> Result<Vec<Word>> {
    match inst.class.opcode {
        Op::TypeArray => Ok(vec![id(inst.operands.first())?]),
        Op::TypeStruct => inst.operands.iter().map(|o| id(Some(o))).collect(),
        Op::TypeRuntimeArray => fail("runtime array reached through a non-buffer pointer"),
        _ => Ok(Vec::new()),
    }
}
struct Constants<'a> {
    definitions: &'a BTreeMap<Word, Instruction>,
    clones: &'a BTreeMap<Word, Word>,
    next: &'a mut Word,
    ids: BTreeMap<Word, Word>,
    additions: BTreeMap<Word, Instruction>,
}
impl Constants<'_> {
    fn convert(&mut self, value: Word, expected: Word, depth: usize) -> Result<Word> {
        if depth > 128 {
            return fail("aggregate initializer nesting exceeds supported depth");
        }
        let old = self
            .definitions
            .get(&value)
            .ok_or_else(|| Error("undefined initializer".into()))?
            .clone();
        let ty = old
            .result_type
            .ok_or_else(|| Error("initializer has no type".into()))?;
        if ty == expected {
            return Ok(value);
        }
        if self.clones.get(&ty) != Some(&expected) {
            return fail("initializer type mismatch");
        }
        if let Some(&cached) = self.ids.get(&value) {
            return Ok(cached);
        }
        let mut new = old.clone();
        new.result_type = Some(expected);
        new.result_id = Some(fresh(self.next)?);
        match old.class.opcode {
            Op::ConstantNull => {}
            Op::ConstantComposite | Op::SpecConstantComposite => {
                let aggregate = self
                    .definitions
                    .get(&ty)
                    .ok_or_else(|| Error("undefined aggregate type".into()))?;
                let members = children(aggregate)?;
                let element_types = if aggregate.class.opcode == Op::TypeArray {
                    vec![members[0]; old.operands.len()]
                } else {
                    members
                };
                if element_types.len() != old.operands.len() {
                    return fail("composite initializer arity mismatch");
                }
                new.operands = old
                    .operands
                    .iter()
                    .zip(element_types)
                    .map(|(v, t)| {
                        let target = self.clones.get(&t).copied().unwrap_or(t);
                        self.convert(id(Some(v))?, target, depth + 1)
                            .map(Operand::IdRef)
                    })
                    .collect::<Result<_>>()?;
            }
            op => return fail(format!("unsupported aggregate initializer {op:?}")),
        }
        let result = new.result_id.unwrap();
        self.ids.insert(value, result);
        self.additions.insert(value, new);
        Ok(result)
    }
}

/// Legalize supported modules; parsing and transformation errors never fall back
/// to the original, potentially invalid, shader. The caller owns validation.
pub fn repair(words: &[u32]) -> Result<Vec<u32>> {
    // rspirv normalizes header version fields; validate the caller's raw header
    // first so reserved bits and unsupported versions cannot be silently accepted.
    if words.len() < 5 || words[0] != rspirv::spirv::MAGIC_NUMBER || words[4] != 0 {
        return fail("invalid SPIR-V header");
    }
    if !matches!(words[1], 0x0001_0400 | 0x0001_0500 | 0x0001_0600) {
        return fail("requires SPIR-V 1.4 through 1.6");
    }

    let mut module = dr::load_words(words).map_err(|e| Error(format!("invalid SPIR-V: {e}")))?;
    let header = module
        .header
        .as_ref()
        .ok_or_else(|| Error("missing SPIR-V header".into()))?;
    if !matches!(header.version, 0x0001_0400 | 0x0001_0500 | 0x0001_0600) {
        return fail("requires SPIR-V 1.4 through 1.6");
    }
    if !matches!(
        module
            .memory_model
            .as_ref()
            .and_then(|i| i.operands.first()),
        Some(Operand::AddressingModel(AddressingModel::Logical))
    ) {
        return fail("requires logical addressing");
    }
    let mut next = header.bound;
    let mut definitions = BTreeMap::new();
    let mut value_types = BTreeMap::new();
    for inst in module.all_inst_iter() {
        if matches!(
            inst.class.opcode,
            Op::CopyMemory
                | Op::CopyMemorySized
                | Op::PtrAccessChain
                | Op::InBoundsPtrAccessChain
                | Op::TypeForwardPointer
                | Op::DecorationGroup
                | Op::GroupDecorate
                | Op::GroupMemberDecorate
        ) {
            return fail(format!("unsupported instruction {:?}", inst.class.opcode));
        }
        if let Some(result) = inst.result_id {
            if result == 0 || result >= next || definitions.insert(result, inst.clone()).is_some() {
                return fail("invalid or duplicate result ID");
            }
            if let Some(ty) = inst.result_type {
                value_types.insert(result, ty);
            }
        }
    }
    let mut pointers = BTreeMap::new();
    let mut aggregate = BTreeSet::new();
    let mut decorated = BTreeSet::new();
    for inst in &module.annotations {
        if matches!(inst.class.opcode, Op::Decorate | Op::MemberDecorate) {
            let (target, d) = annotation(inst)?;
            if forbidden(d) {
                decorated.insert(target);
            }
        } else {
            return fail(format!("unsupported annotation {:?}", inst.class.opcode));
        }
    }
    for inst in &module.types_global_values {
        if matches!(
            inst.class.opcode,
            Op::TypeArray | Op::TypeRuntimeArray | Op::TypeStruct
        ) {
            aggregate.insert(inst.result_id.unwrap());
        }
        if inst.class.opcode == Op::TypePointer {
            let space = match inst.operands.first() {
                Some(Operand::StorageClass(s)) => *s,
                _ => return fail("invalid pointer storage class"),
            };
            if matches!(
                space,
                StorageClass::Function | StorageClass::Private | StorageClass::Workgroup
            ) {
                pointers.insert(inst.result_id.unwrap(), id(inst.operands.get(1))?);
            }
        }
    }
    let mut needed = BTreeSet::new();
    let mut stack: Vec<_> = pointers.values().copied().collect();
    while let Some(t) = stack.pop() {
        if !aggregate.contains(&t) || !needed.insert(t) {
            continue;
        }
        stack.extend(children(&definitions[&t])?);
    }
    // Naga emits non-recursive aggregate types in dependency order. Only clone
    // a type when its own layout or a member layout requires it; this makes the
    // pass idempotent and avoids altering already valid, unrelated declarations.
    let mut clones = BTreeMap::new();
    for inst in &module.types_global_values {
        if let Some(t) = inst.result_id.filter(|t| needed.contains(t)) {
            if decorated.contains(&t) || children(inst)?.iter().any(|c| clones.contains_key(c)) {
                clones.insert(t, fresh(&mut next)?);
            }
        }
    }
    if clones.is_empty() {
        return Ok(words.to_vec());
    }
    let changed: BTreeMap<_, _> = pointers
        .iter()
        .filter_map(|(&p, &t)| clones.get(&t).map(|&new| (p, (t, new))))
        .collect();
    let mut constants = Constants {
        definitions: &definitions,
        clones: &clones,
        next: &mut next,
        ids: BTreeMap::new(),
        additions: BTreeMap::new(),
    };
    let mut initializers = BTreeMap::new();
    for inst in module.all_inst_iter() {
        if inst.class.opcode == Op::Variable {
            if let Some(&(_, new)) = inst.result_type.and_then(|t| changed.get(&t)) {
                if let Some(init) = inst.operands.get(1) {
                    initializers.insert(
                        inst.result_id.unwrap(),
                        constants.convert(id(Some(init))?, new, 0)?,
                    );
                }
            }
        }
    }
    let additions = constants.additions;
    drop(constants.ids);
    let mut globals = Vec::new();
    for mut inst in std::mem::take(&mut module.types_global_values) {
        let old_id = inst.result_id;
        if inst.class.opcode == Op::TypePointer {
            if let Some(&(_, new)) = old_id.and_then(|t| changed.get(&t)) {
                inst.operands[1] = Operand::IdRef(new);
            }
        }
        if let Some(&init) = old_id.and_then(|t| initializers.get(&t)) {
            inst.operands[1] = Operand::IdRef(init);
        }
        globals.push(inst.clone());
        if let Some(&new) = old_id.and_then(|t| clones.get(&t)) {
            let mut copy = inst;
            copy.result_id = Some(new);
            let count = if copy.class.opcode == Op::TypeArray {
                1
            } else {
                copy.operands.len()
            };
            for operand in &mut copy.operands[..count] {
                let old = id(Some(operand))?;
                if let Some(&cloned) = clones.get(&old) {
                    *operand = Operand::IdRef(cloned);
                }
            }
            globals.push(copy);
        }
        if let Some(extra) = old_id.and_then(|t| additions.get(&t)) {
            globals.push(extra.clone());
        }
    }
    module.types_global_values = globals;
    let mut extra_annotations = Vec::new();
    for inst in &module.annotations {
        let (target, d) = annotation(inst)?;
        if !forbidden(d) {
            if let Some(&new) = clones.get(&target) {
                let mut copy = inst.clone();
                copy.operands[0] = Operand::IdRef(new);
                extra_annotations.push(copy);
            }
        }
    }
    module.annotations.extend(extra_annotations);
    for function in &mut module.functions {
        for block in &mut function.blocks {
            let mut output = Vec::new();
            for mut inst in std::mem::take(&mut block.instructions) {
                match inst.class.opcode {
                    Op::Variable => {
                        if let Some(&init) = inst.result_id.and_then(|t| initializers.get(&t)) {
                            inst.operands[1] = Operand::IdRef(init);
                        }
                    }
                    Op::Load => {
                        let ptr = id(inst.operands.first())?;
                        if let Some(&(old, new)) =
                            value_types.get(&ptr).and_then(|t| changed.get(t))
                        {
                            if inst.result_type != Some(old) {
                                return fail("load type mismatch");
                            }
                            let result = inst
                                .result_id
                                .ok_or_else(|| Error("load without result".into()))?;
                            let temp = fresh(&mut next)?;
                            inst.result_type = Some(new);
                            inst.result_id = Some(temp);
                            output.push(inst);
                            inst = Instruction::new(
                                Op::CopyLogical,
                                Some(old),
                                Some(result),
                                vec![Operand::IdRef(temp)],
                            );
                        }
                    }
                    Op::Store => {
                        let ptr = id(inst.operands.first())?;
                        if let Some(&(old, new)) =
                            value_types.get(&ptr).and_then(|t| changed.get(t))
                        {
                            let value = id(inst.operands.get(1))?;
                            if value_types.get(&value) != Some(&old) {
                                return fail("store type mismatch");
                            }
                            let temp = fresh(&mut next)?;
                            output.push(Instruction::new(
                                Op::CopyLogical,
                                Some(new),
                                Some(temp),
                                vec![Operand::IdRef(value)],
                            ));
                            inst.operands[1] = Operand::IdRef(temp);
                        }
                    }
                    _ => {}
                }
                output.push(inst);
            }
            block.instructions = output;
        }
    }
    module.header.as_mut().unwrap().bound = next;
    Ok(module.assemble())
}
