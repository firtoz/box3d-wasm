use gpu_spirv_layout::repair;
use rspirv::{
    binary::Assemble,
    dr::{self, Instruction, Operand},
    spirv::{Op, StorageClass},
};
fn words(bytes: &[u8]) -> Vec<u32> {
    bytes
        .chunks_exact(4)
        .map(|b| u32::from_le_bytes(b.try_into().unwrap()))
        .collect()
}
fn basic() -> Vec<u32> {
    words(include_bytes!("fixtures/shared-array.spv"))
}
#[test]
fn malformed_input_is_rejected() {
    assert!(repair(&[0, 1, 2]).is_err());
}
#[test]
fn unsupported_versions_are_rejected() {
    for version in [0x10300, 0x10401, 0x10700] {
        let mut input = basic();
        input[1] = version;
        assert!(repair(&input).is_err());
    }
}
#[test]
fn id_exhaustion_is_rejected() {
    let mut input = basic();
    input[3] = u32::MAX;
    assert!(repair(&input).unwrap_err().0.contains("overflow"));
}
#[test]
fn duplicate_results_are_rejected() {
    let mut m = dr::load_words(basic()).unwrap();
    let duplicate = m.types_global_values[0].clone();
    m.types_global_values.push(duplicate);
    assert!(repair(&m.assemble()).unwrap_err().0.contains("duplicate"));
}
#[test]
fn unsupported_memory_copy_is_rejected() {
    let mut m = dr::load_words(basic()).unwrap();
    m.functions[0].blocks[0].instructions.insert(
        0,
        Instruction::new(
            Op::CopyMemory,
            None,
            None,
            vec![Operand::IdRef(1), Operand::IdRef(2)],
        ),
    );
    assert!(repair(&m.assemble()).unwrap_err().0.contains("CopyMemory"));
}
#[test]
fn explicit_buffer_types_and_annotations_survive() {
    let input = basic();
    let before = dr::load_words(&input).unwrap();
    let fixed = repair(&input).unwrap();
    let after = dr::load_words(&fixed).unwrap();
    assert_ne!(input, fixed);
    assert_eq!(fixed, repair(&fixed).unwrap());
    assert!(after.annotations.starts_with(&before.annotations));
    for original in &before.types_global_values {
        let op = original.class.opcode;
        if matches!(op, Op::TypeArray | Op::TypeRuntimeArray | Op::TypeStruct)
            || (op == Op::TypePointer
                && matches!(
                    original.operands.first(),
                    Some(Operand::StorageClass(
                        StorageClass::StorageBuffer
                            | StorageClass::Uniform
                            | StorageClass::PushConstant
                    ))
                ))
        {
            assert_eq!(
                Some(original),
                after
                    .types_global_values
                    .iter()
                    .find(|i| i.result_id == original.result_id)
            );
        }
    }
    assert_eq!(before.entry_points, after.entry_points);
    assert_eq!(before.execution_modes, after.execution_modes);
}
#[test]
fn composite_values_use_logical_copies_and_preserve_arithmetic() {
    let input = words(include_bytes!("fixtures/composites.spv"));
    let before = dr::load_words(&input).unwrap();
    let fixed = repair(&input).unwrap();
    let after = dr::load_words(&fixed).unwrap();
    assert_eq!(fixed, repair(&fixed).unwrap());
    assert!(after
        .all_inst_iter()
        .any(|i| i.class.opcode == Op::CopyLogical));
    for original in before.all_inst_iter().filter(|i| {
        matches!(
            i.class.opcode,
            Op::FAdd | Op::IAdd | Op::UMod | Op::FSub | Op::FMul
        )
    }) {
        assert_eq!(
            Some(original),
            after
                .all_inst_iter()
                .find(|i| i.result_id == original.result_id)
        );
    }
}
