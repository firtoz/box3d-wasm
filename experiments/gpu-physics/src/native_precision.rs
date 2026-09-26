//! Preserve native physics arithmetic order on Vulkan.
//! Explicit WGSL scalar operations and protected FMA residuals match the CPU
//! without per-step compilation or CPU simulation.
use std::{
    collections::{HashMap, HashSet},
    sync::{Arc, Mutex, OnceLock},
};

type PipelineKey = (String, Vec<(String, u64)>);

struct SourceCache {
    parsed: naga::Module,
    info: naga::valid::ModuleInfo,
    entries: HashSet<String>,
    modules: HashMap<PipelineKey, Arc<Vec<u32>>>,
}

pub(crate) fn module(
    device: &wgpu::Device,
    source: &str,
    entry: &str,
    constants: &[(&str, f64)],
) -> Option<wgpu::ShaderModule> {
    if !device
        .features()
        .contains(wgpu::Features::SPIRV_SHADER_PASSTHROUGH)
    {
        return None;
    }
    // These builders also create pipelines from other shaders. Select compute
    // entry points from the parsed physics module, not a text substring.
    static CACHE: OnceLock<Mutex<HashMap<String, SourceCache>>> = OnceLock::new();
    let mut cache = CACHE.get_or_init(Default::default).lock().unwrap();
    let source_cache = cache.entry(source.to_owned()).or_insert_with(|| {
        let start = std::time::Instant::now();
        let (parsed, info) = parse_and_validate(source);
        if std::env::var_os("GPU_PHYSICS_TRACE_PIPELINES").is_some() {
            eprintln!("gpu-frontend-source parse_validate_ms={:.3}", start.elapsed().as_secs_f64()*1000.0);
        }
        SourceCache {
            entries: parsed.entry_points.iter()
                .filter(|e| e.stage == naga::ShaderStage::Compute)
                .map(|e| e.name.clone()).collect(),
            parsed, info, modules: HashMap::new(),
        }
    });
    if !source_cache.entries.contains(entry) {
        return None;
    }
    // Source, entry point, and exact override bits determine the compiled module.
    let key = (
        entry.to_owned(),
        constants
            .iter()
            .map(|(name, value)| ((*name).to_owned(), value.to_bits()))
            .collect(),
    );
    let SourceCache { parsed, info, modules, .. } = source_cache;
    let words = modules
        .entry(key)
        .or_insert_with(|| Arc::new(compile_parsed(parsed, info, entry, constants)))
        .clone();
    if std::env::var_os("GPU_PHYSICS_VERIFY_SHADER_REUSE").is_some() {
        let (fresh, fresh_info) = parse_and_validate(source);
        assert_eq!(&*words, &compile_parsed(&fresh, &fresh_info, entry, constants),
            "reused shader differs from fresh compilation: {entry}");
        eprintln!("gpu-shader-reuse verified {entry}");
    }
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

fn parse_and_validate(source: &str) -> (naga::Module, naga::valid::ModuleInfo) {
    let parsed = naga::front::wgsl::parse_str(source).expect("physics WGSL");
    let info = naga::valid::Validator::new(
        naga::valid::ValidationFlags::all(),
        naga::valid::Capabilities::all(),
    ).validate(&parsed).expect("validated physics WGSL");
    (parsed, info)
}

#[cfg(test)]
fn compile(source: &str, entry: &str, constants: &[(&str, f64)]) -> Vec<u32> {
    let (parsed, info) = parse_and_validate(source);
    compile_parsed(&parsed, &info, entry, constants)
}

// Parsing/validation are source-wide. Only override processing and emission
// depend on the entry point. Keep the validated module immutable so each
// specialization starts with exactly the same input as a fresh compilation.
fn compile_parsed(parsed: &naga::Module, info: &naga::valid::ModuleInfo,
    entry: &str, constants: &[(&str, f64)]) -> Vec<u32> {
    let validated_at = std::time::Instant::now();
    let constants = constants.iter().map(|(k, v)| (k.to_string(), *v)).collect();
    let (parsed, info) = naga::back::pipeline_constants::process_overrides(
        parsed,
        info,
        Some((naga::ShaderStage::Compute, entry)),
        &constants,
    )
    .expect("physics pipeline constants");
    let overrides_at = std::time::Instant::now();
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
    let decorated = decorate(&words);
    if std::env::var_os("GPU_PHYSICS_TRACE_PIPELINES").is_some() {
        eprintln!("gpu-frontend {entry} parse_ms={:.3} validate_ms={:.3} overrides_ms={:.3} emit_ms={:.3}",
            0.0,
            0.0,
            overrides_at.duration_since(validated_at).as_secs_f64()*1000.0,
            overrides_at.elapsed().as_secs_f64()*1000.0);
    }
    decorated
}

fn decorate(words: &[u32]) -> Vec<u32> {
    let mut glsl_set = None;
    let mut insert = None;
    let mut annotations = Vec::new();
    let mut inside = false;
    let mut pos = 5;
    while pos < words.len() {
        let count = (words[pos] >> 16) as usize;
        let op = words[pos] & 65535;
        assert!(count > 0 && pos + count <= words.len());
        match op {
            11 => {
                let bytes: Vec<_> = words[pos + 2..pos + count]
                    .iter()
                    .flat_map(|v| v.to_le_bytes())
                    .take_while(|b| *b != 0)
                    .collect();
                if bytes == b"GLSL.std.450" {
                    glsl_set = Some(words[pos + 1]);
                }
            }
            12 if inside && Some(words[pos + 3]) == glsl_set && words[pos + 4] == 50 => {
                // GLSL Fma needs NoContraction to preserve single-operation semantics.
                annotations.extend_from_slice(&[(3 << 16) | 71, words[pos + 2], 42]);
            }
            19..=39 if insert.is_none() => insert = Some(pos), // type declarations
            54 => inside = true,                               // OpFunction
            56 => inside = false,                              // OpFunctionEnd
            129 | 131 | 133 | 136 | 142..=148 if inside => {
                // OpDecorate result-id NoContraction (42).
                annotations.extend_from_slice(&[(3 << 16) | 71, words[pos + 2], 42]);
            }
            _ => {}
        }
        pos += count;
    }
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
    fn source_reuse_keeps_entry_points_and_overrides_independent() {
        let source = r#"
@group(0) @binding(0) var<storage,read_write> values:array<f32>;
override scale:f32=1.0;
@compute @workgroup_size(1) fn first(){values[0]=values[1]*scale+values[2];}
@compute @workgroup_size(1) fn second(){values[1]=values[0]/scale;}
"#;
        let (parsed, info) = parse_and_validate(source);
        let mut previous = Vec::new();
        for (entry, scale) in [("first", 2.0), ("second", 3.0), ("first", 7.0), ("second", 3.0), ("first", 2.0)] {
            let cached = compile_parsed(&parsed, &info, entry, &[("scale", scale)]);
            assert_eq!(cached, compile(source, entry, &[("scale", scale)]));
            previous.push(cached);
        }
        assert_eq!(previous[0], previous[4]);
        assert_eq!(previous[1], previous[3]);
        assert_ne!(previous[0], previous[2]);
    }

    #[test]
    fn precision_covers_arithmetic_and_explicit_fma() {
        let source = r#"
@group(0) @binding(0) var<storage,read_write> values:array<f32>;
fn gyro_probe(a:f32,b:f32,c:f32)->f32{return a*b+c;}
fn ordinary(a:f32,b:f32,c:f32)->f32{return a*b+c;}
fn gyro_fused(a:f32,b:f32,c:f32)->f32{return fma(a,b,c);}
fn ordinary_fused(a:f32,b:f32,c:f32)->f32{return fma(a,b,c);}
@compute @workgroup_size(1) fn main(){values[3]=gyro_probe(values[0],values[1],values[2]);values[4]=ordinary(values[0],values[1],values[2]);values[5]=gyro_fused(values[0],values[1],values[2]);values[6]=ordinary_fused(values[0],values[1],values[2]);}
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
                12 if words[pos + 4] == 50 => {
                    owners.insert(words[pos + 2], owner);
                }
                129 | 131 | 133 | 136 | 142..=148 => {
                    owners.insert(words[pos + 2], owner);
                }
                71 if count == 3 && words[pos + 2] == 42 => decorated.push(words[pos + 1]),
                _ => {}
            }
            pos += count;
        }
        assert_eq!(decorated.len(), 6);
        for result in decorated {
            assert!(["gyro_probe", "ordinary", "gyro_fused", "ordinary_fused"]
                .contains(&names[&owners[&result]].as_str()));
        }
    }

    #[test]
    fn integer_only_entry_needs_no_float_decorations() {
        let words = compile("@compute @workgroup_size(1) fn main() {}", "main", &[]);
        assert_eq!(words[0], 0x07230203);
    }

    #[test]
    fn native_scalar_residual_corrections_match_cpu() {
        use wgpu::util::DeviceExt;
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        if !gpu
            .device
            .features()
            .contains(wgpu::Features::SPIRV_SHADER_PASSTHROUGH)
        {
            return;
        }
        // First integrated rotation from the independent ground-drag fixture.
        // Its squared length lies just below a square-root rounding midpoint.
        let inputs = [
            [-0.0014589281_f32, 0.00035152573, 0.00005638938, 1.0],
            [0.0003, -0.002, 0.0001, 1.0],
            [0.4, -0.2, 0.7, 0.5],
            [0.0, 0.0, 0.0, 1.0],
        ];
        // Tilted contact normal from frame 63 of the independent drag fixture.
        let vectors = [
            [0.0_f32, -0.06706716, -0.99774843],
            [0.8, -0.6, 0.0],
            [0.3, -0.4, 0.5],
            [0.0, 0.0, 0.0],
        ];
        let reciprocals = [0.006_f32, 0.004, 0.0025, 1.0000011];
        let mut data = inputs.to_vec();
        data.extend(vectors.map(|[x, y, z]| [x, y, z, 0.0]));
        data.extend(reciprocals.map(|x| [x, 0.0, 0.0, 0.0]));
        let divisions = [
            [1.0_f32, 0.006],
            [0.16949959, 0.006],
            [0.5, 1.0000011],
            [0.00001, 0.000010119209],
        ];
        data.extend(divisions.map(|[x, y]| [x, y, 0.0, 0.0]));
        let mut roots = vec![f32::from_bits(0x3f7fffff), 0.0, 1.0, 2.0];
        let mut seed = 0x6a09e667_u32;
        for exponent in 1..255_u32 {
            for mantissa in [0, 1, 0x3fffff, 0x7fffff] {
                roots.push(f32::from_bits((exponent << 23) | mantissa));
            }
            seed = seed.wrapping_mul(1664525).wrapping_add(1013904223);
            roots.push(f32::from_bits((exponent << 23) | (seed & 0x7fffff)));
        }
        data.extend(roots.iter().map(|&x| [x, 0.0, 0.0, 0.0]));
        // Independent b3Atan2 outputs from Box3D C, including all quadrants,
        // axes, and the minimax polynomial's visible difference from atan2f.
        let angles: [([f32; 2], u32); 14] = [
            ([0.0, 0.0], 0x00000000), ([0.0, 1.0], 0x00000000),
            ([1.0, 0.0], 0x3fc90fdb), ([0.0, -1.0], 0x40490fdb),
            ([-1.0, 0.0], 0xbfc90fdb), ([0.1, 0.7], 0x3e114e31),
            ([-0.1, 0.7], 0xbe114e31), ([0.1, -0.7], 0x403ffaf8),
            ([-0.1, -0.7], 0xc03ffaf8), ([0.7, 0.1], 0x3fb6e615),
            ([-0.7, -0.1], 0xbfdb39a1), ([1.0, 1.0], 0x3f4911aa),
            ([0.3, 0.4], 0x3f24bda6), ([0.05, 1.0], 0x3d4ca154),
        ];
        let atan_start = data.len();
        data.extend(angles.iter().map(|([y, x], _)| [*y, *x, 0.0, 0.0]));
        let rotation = include_str!("../shaders/physics/rotation.wgsl");
        let helpers = &rotation[rotation.find("fn gyro_norm3(").unwrap()
            ..rotation.find("fn gyro_integrate_rotation(").unwrap()];
        let helpers = format!("{helpers}\n{}", &rotation[rotation.find("fn gyro_atan2(").unwrap()..]);
        let source = format!("{helpers}\n@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>;\n@compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{ if (id.x < 4u) {{ values[id.x]=gyro_norm4(values[id.x]); }} else if (id.x < 8u) {{ values[id.x]=vec4<f32>(gyro_norm3(values[id.x].xyz),0.0); }} else if (id.x < 12u) {{ values[id.x]=vec4<f32>(gyro_recip(values[id.x].x),0.0,0.0,0.0); }} else if (id.x < 16u) {{ values[id.x]=vec4<f32>(gyro_divide(values[id.x].x,values[id.x].y),0.0,0.0,0.0); }} else if (id.x < {atan_start}u) {{ values[id.x]=vec4<f32>(gyro_sqrt(values[id.x].x),0.0,0.0,0.0); }} else {{ values[id.x]=vec4<f32>(gyro_atan2(values[id.x].x,values[id.x].y),0.0,0.0,0.0); }} }}");
        let words = compile(&source, "main", &[]);
        // SAFETY: the same validated Naga output and decoration used in production.
        let shader = unsafe {
            gpu.device.create_shader_module_passthrough(
                wgpu::ShaderModuleDescriptorPassthrough::SpirV(wgpu::ShaderModuleDescriptorSpirV {
                    label: Some("normalization regression"),
                    source: std::borrow::Cow::Owned(words),
                }),
            )
        };
        let binding_layout =
            gpu.device
                .create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
                    label: None,
                    entries: &[wgpu::BindGroupLayoutEntry {
                        binding: 0,
                        visibility: wgpu::ShaderStages::COMPUTE,
                        ty: wgpu::BindingType::Buffer {
                            ty: wgpu::BufferBindingType::Storage { read_only: false },
                            has_dynamic_offset: false,
                            min_binding_size: None,
                        },
                        count: None,
                    }],
                });
        let layout = gpu
            .device
            .create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
                label: None,
                bind_group_layouts: &[&binding_layout],
                push_constant_ranges: &[],
            });
        let pipeline = gpu
            .device
            .create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
                label: None,
                layout: Some(&layout),
                module: &shader,
                entry_point: Some("main"),
                compilation_options: Default::default(),
                cache: None,
            });
        let buffer = gpu
            .device
            .create_buffer_init(&wgpu::util::BufferInitDescriptor {
                label: None,
                contents: bytemuck::cast_slice(&data),
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC,
            });
        let readback = gpu.device.create_buffer(&wgpu::BufferDescriptor {
            label: None,
            size: buffer.size(),
            mapped_at_creation: false,
            usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
        });
        let group = gpu.device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &pipeline.get_bind_group_layout(0),
            entries: &[wgpu::BindGroupEntry {
                binding: 0,
                resource: buffer.as_entire_binding(),
            }],
        });
        let mut encoder = gpu.device.create_command_encoder(&Default::default());
        {
            let mut pass = encoder.begin_compute_pass(&Default::default());
            pass.set_pipeline(&pipeline);
            pass.set_bind_group(0, &group, &[]);
            pass.dispatch_workgroups(data.len() as u32, 1, 1);
        }
        encoder.copy_buffer_to_buffer(&buffer, 0, &readback, 0, buffer.size());
        gpu.queue.submit([encoder.finish()]);
        let (tx, rx) = std::sync::mpsc::channel();
        readback
            .slice(..)
            .map_async(wgpu::MapMode::Read, move |r| tx.send(r).unwrap());
        gpu.device.poll(wgpu::PollType::Wait).unwrap();
        rx.recv().unwrap().unwrap();
        let mapped = readback.slice(..).get_mapped_range();
        let actual: &[[f32; 4]] = bytemuck::cast_slice(&mapped);
        for ((input, expected), result) in angles.iter().zip(&actual[atan_start..]) {
            assert_eq!(result[0].to_bits(), *expected, "Box3D atan2 {input:?}");
        }
        for (x, result) in roots.iter().zip(&actual[16..]) {
            assert_eq!(
                result[0].to_bits(),
                x.sqrt().to_bits(),
                "sqrt {x} ({:08x})",
                x.to_bits()
            );
        }
        for (v, result) in vectors.iter().zip(&actual[inputs.len()..]) {
            let squared = (v[0] * v[0] + v[1] * v[1]) + v[2] * v[2];
            let scale = if squared > 1000.0 * f32::MIN_POSITIVE {
                1.0 / squared.sqrt()
            } else {
                0.0
            };
            for axis in 0..3 {
                assert_eq!(
                    result[axis].to_bits(),
                    (scale * v[axis]).to_bits(),
                    "vector {v:?} axis {axis}"
                );
            }
        }
        for (x, result) in reciprocals
            .iter()
            .zip(&actual[inputs.len() + vectors.len()..])
        {
            assert_eq!(
                result[0].to_bits(),
                (1.0_f32 / x).to_bits(),
                "reciprocal {x}"
            );
        }
        for ([x, y], result) in divisions
            .iter()
            .zip(&actual[inputs.len() + vectors.len() + reciprocals.len()..])
        {
            assert_eq!(result[0].to_bits(), (x / y).to_bits(), "division {x}/{y}");
        }
        for (q, result) in inputs.iter().zip(actual) {
            let squared = ((q[0] * q[0] + q[1] * q[1]) + q[2] * q[2]) + q[3] * q[3];
            let scale = 1.0 / squared.sqrt();
            let expected = q.map(|v| (scale * v).to_bits());
            assert_eq!(result.map(f32::to_bits), expected, "input {q:?}");
        }
    }
}
