//! Preserve the CPU rotational integrator's rounding on native Vulkan.
//! Only gyro_* functions receive NoContraction. No contact/constraint arithmetic
//! is decorated, and no per-step compilation or CPU simulation is involved.
use std::{
    collections::HashMap,
    sync::{Arc, Mutex, OnceLock},
};

const ENTRIES: &[&str] = &[
    "integrate_vel",
    "integrate_pos",
    "apply_deltas",
    "solve_tiny_islands",
    "solve_complete_components",
    "solve_large_components",
];

pub(crate) fn module(
    device: &wgpu::Device,
    source: &str,
    entry: &str,
    constants: &[(&str, f64)],
) -> Option<wgpu::ShaderModule> {
    if !ENTRIES.contains(&entry)
        || !device
            .features()
            .contains(wgpu::Features::SPIRV_SHADER_PASSTHROUGH)
    {
        return None;
    }
    // All callers use the same compile-time physics source. Constants distinguish
    // the small-component workgroup variants; cache lives across scene switches.
    type Cache = Mutex<HashMap<String, Arc<Vec<u32>>>>;
    static CACHE: OnceLock<Cache> = OnceLock::new();
    let key = format!("{entry}:{constants:?}");
    let mut cache = CACHE.get_or_init(Default::default).lock().unwrap();
    let words = cache
        .entry(key)
        .or_insert_with(|| Arc::new(compile(source, entry, constants)))
        .clone();
    drop(cache);
    // SAFETY: source is our validated WGSL, compiled by Naga for this entry point.
    // The only binary transformation adds valid NoContraction decorations to
    // floating-point arithmetic results; types, bindings and control flow are unchanged.
    Some(unsafe {
        device.create_shader_module_passthrough(wgpu::ShaderModuleDescriptorPassthrough::SpirV(
            wgpu::ShaderModuleDescriptorSpirV {
                label: Some(entry),
                source: std::borrow::Cow::Borrowed(&words),
            },
        ))
    })
}

fn compile(source: &str, entry: &str, constants: &[(&str, f64)]) -> Vec<u32> {
    let parsed = naga::front::wgsl::parse_str(source).expect("physics WGSL");
    let info = naga::valid::Validator::new(
        naga::valid::ValidationFlags::all(),
        naga::valid::Capabilities::all(),
    )
    .validate(&parsed)
    .expect("validated physics WGSL");
    let constants = constants.iter().map(|(k, v)| (k.to_string(), *v)).collect();
    let (parsed, info) = naga::back::pipeline_constants::process_overrides(
        &parsed,
        &info,
        Some((naga::ShaderStage::Compute, entry)),
        &constants,
    )
    .expect("physics pipeline constants");
    let mut options = naga::back::spv::Options::default();
    options.flags.insert(naga::back::spv::WriterFlags::DEBUG);
    // Passthrough must not silently drop the bounds protection provided by wgpu.
    // Use software checks rather than depending on adapter-private robustness flags.
    options.bounds_check_policies = naga::proc::BoundsCheckPolicies {
        index: naga::proc::BoundsCheckPolicy::Restrict,
        buffer: naga::proc::BoundsCheckPolicy::Restrict,
        image_load: naga::proc::BoundsCheckPolicy::Restrict,
        binding_array: naga::proc::BoundsCheckPolicy::Restrict,
    };
    let words = naga::back::spv::write_vec(
        &parsed,
        &info,
        &options,
        Some(&naga::back::spv::PipelineOptions {
            shader_stage: naga::ShaderStage::Compute,
            entry_point: entry.into(),
        }),
    )
    .expect("physics SPIR-V");
    decorate(&words)
}

fn decorate(words: &[u32]) -> Vec<u32> {
    let mut functions = std::collections::HashSet::new();
    let mut insert = None;
    let mut annotations = Vec::new();
    let mut inside = false;
    let mut pos = 5;
    while pos < words.len() {
        let count = (words[pos] >> 16) as usize;
        let op = words[pos] & 65535;
        assert!(count > 0 && pos + count <= words.len());
        match op {
            5 => {
                // OpName
                let bytes: Vec<_> = words[pos + 2..pos + count]
                    .iter()
                    .flat_map(|v| v.to_le_bytes())
                    .collect();
                let end = bytes.iter().position(|b| *b == 0).unwrap_or(bytes.len());
                if std::str::from_utf8(&bytes[..end])
                    .unwrap()
                    .starts_with("gyro_")
                {
                    functions.insert(words[pos + 1]);
                }
            }
            19..=39 if insert.is_none() => insert = Some(pos), // type declarations
            54 => inside = functions.contains(&words[pos + 2]), // OpFunction
            56 => inside = false,                              // OpFunctionEnd
            129 | 131 | 133 | 136 | 142..=148 if inside => {
                // OpDecorate result-id NoContraction (42).
                annotations.extend_from_slice(&[(3 << 16) | 71, words[pos + 2], 42]);
            }
            _ => {}
        }
        pos += count;
    }
    assert!(
        !annotations.is_empty(),
        "rotational pipeline must retain gyro_* arithmetic names"
    );
    let insert = insert.expect("SPIR-V types");
    let mut out = words[..insert].to_vec();
    out.extend(annotations);
    out.extend_from_slice(&words[insert..]);
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn precision_is_scoped_to_rotation_functions() {
        let source = r#"
@group(0) @binding(0) var<storage,read_write> values:array<f32>;
fn gyro_probe(a:f32,b:f32,c:f32)->f32{return a*b+c;}
fn ordinary(a:f32,b:f32,c:f32)->f32{return a*b+c;}
@compute @workgroup_size(1) fn main(){values[3]=gyro_probe(values[0],values[1],values[2]);values[4]=ordinary(values[0],values[1],values[2]);}
"#;
        let words = compile(source, "main", &[]);
        let mut names = HashMap::new();
        let mut decorated = Vec::new();
        let mut owners = HashMap::new();
        let mut owner = 0;
        let mut pos = 5;
        while pos < words.len() {
            let count = (words[pos] >> 16) as usize;
            let op = words[pos] & 65535;
            match op {
                5 => {
                    let bytes: Vec<_> = words[pos + 2..pos + count]
                        .iter()
                        .flat_map(|v| v.to_le_bytes())
                        .take_while(|b| *b != 0)
                        .collect();
                    names.insert(words[pos + 1], String::from_utf8(bytes).unwrap());
                }
                54 => owner = words[pos + 2],
                129 | 131 | 133 | 136 | 142..=148 => {
                    owners.insert(words[pos + 2], owner);
                }
                71 if count == 3 && words[pos + 2] == 42 => decorated.push(words[pos + 1]),
                _ => {}
            }
            pos += count;
        }
        assert_eq!(decorated.len(), 2);
        for result in decorated {
            assert!(names[&owners[&result]].starts_with("gyro_"));
        }
    }
}
