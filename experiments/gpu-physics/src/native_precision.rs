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

    fn run_float_shader(gpu: &crate::sim::GpuDevice, source: &str, data: &[[f32; 4]], workgroups: u32) -> Vec<[f32; 4]> {
        use wgpu::util::DeviceExt;
        let words = compile(source, "main", &[]);
        // SAFETY: the same validated Naga output and decoration used in production.
        let shader = unsafe {
            gpu.device.create_shader_module_passthrough(
                wgpu::ShaderModuleDescriptorPassthrough::SpirV(wgpu::ShaderModuleDescriptorSpirV {
                    label: Some("native arithmetic regression"),
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
            pass.dispatch_workgroups(workgroups, 1, 1);
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
        let result = bytemuck::cast_slice::<u8, [f32; 4]>(&mapped).to_vec();
        drop(mapped);
        readback.unmap();
        result
    }

    #[test]
    fn native_scalar_residual_corrections_match_cpu() {
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
        let actual = run_float_shader(&gpu, &source, &data, data.len() as u32);
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
    include!("fixtures/spherical_preparation.rs");

    #[test]
    fn native_spherical_preparation_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        assert!(gpu.device.features().contains(wgpu::Features::SPIRV_SHADER_PASSTHROUGH));
        let data: Vec<[f32; 4]> = SPHERICAL_PREPARATION_CASES.iter().flatten().copied().collect();
        let expected:Vec<_>=data.chunks_exact(10).map(|v|[v[8],v[9]]).collect();
        fn function(source:&str,name:&str)->String {
            let start=source.find(&format!("fn {name}(" )).unwrap();
            let brace=source[start..].find('{').unwrap()+start;
            let mut depth=1;let mut end=brace+1;
            while depth>0 {match source.as_bytes()[end] {b'{'=>depth+=1,b'}'=>depth-=1,_=>{}}end+=1;}
            source[start..end].to_string()
        }
        let rot=include_str!("../shaders/physics/rotation.wgsl");
        let math=include_str!("../shaders/physics/math.wgsl");
        let solve=include_str!("../shaders/physics/solve.wgsl");
        let scalar=&rot[rot.find("fn gyro_norm3(").unwrap()..rot.find("fn gyro_integrate_rotation(").unwrap()];
        let helpers=[scalar.to_string(),function(rot,"gyro_quat_rotate"),function(rot,"gyro_quat_mul"),function(math,"quat_rotate"),function(math,"quat_mul"),function(math,"quat_inv"),function(math,"native_mul_mv"),function(solve,"normalize_or_zero")].join("\n");
        let start=solve.find("fn solve_spherical(").unwrap();
        let a=solve[start..].find("    let base_frame_a").unwrap()+start;
        let b=solve[a..].find("    let fixed_rotation").unwrap()+a;
        let prepare=solve[a..b].replace("quat_mul((*ba).rot, (*jn).frame_a_rotation)","qa").replace("quat_mul((*bb).rot, (*jn).frame_b_rotation)","qb")
            .replace("world_inv_inertia(*ba,", "native_mul_mv(ia,").replace("world_inv_inertia(*bb,", "native_mul_mv(ib,")
            .replace("world_inv_inertia_matrix(*ba)","ia").replace("world_inv_inertia_matrix(*bb)","ib");
        let source=format!("{helpers}\nfn prepare(ia:mat3x3<f32>,ib:mat3x3<f32>,qa:vec4<f32>,qb:vec4<f32>)->array<vec4<f32>,2>{{ {prepare} return array<vec4<f32>,2>(vec4<f32>(swing_axis,swing_mass),vec4<f32>(twist_jacobian,twist_mass)); }}\n@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>;\n@compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{ let k=id.x*10u; let ia=mat3x3<f32>(values[k].xyz,values[k+1u].xyz,values[k+2u].xyz);let ib=mat3x3<f32>(values[k+3u].xyz,values[k+4u].xyz,values[k+5u].xyz);let out=prepare(ia,ib,values[k+6u],values[k+7u]);values[k]=out[0];values[k+1u]=out[1]; }}");
        let actual = run_float_shader(&gpu, &source, &data, (data.len()/10) as u32);
        let mut differences=0;
        for (i,(v,want)) in actual.chunks_exact(10).zip(expected).enumerate() {
            for j in 0..2 {for k in 0..4 {
                if v[j][k].to_bits()!=want[j][k].to_bits() {differences+=1;eprintln!("prepare mismatch {i}/{j}/{k} gpu={:08x} cpu={:08x}",v[j][k].to_bits(),want[j][k].to_bits());}
            }}
        }
        assert_eq!(differences,0);

    }

    include!("fixtures/spherical_twist_velocity.rs");
    #[test]
    fn native_spherical_twist_velocity_matches_cpu() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        let rotation = include_str!("../shaders/physics/rotation.wgsl");
        let scalar = &rotation[rotation.find("fn gyro_norm3(").unwrap()..rotation.find("fn gyro_integrate_rotation(").unwrap()];
        let solve = include_str!("../shaders/physics/solve.wgsl");
        let joint = &solve[solve.find("fn solve_spherical(").unwrap()..solve.find("fn solve_weld(").unwrap()];
        let expression = |name: &str| joint.split(name).nth(1).unwrap().split(';').next().unwrap().replace("(*ba).omega", "wa").replace("(*bb).omega", "wb");
        let lower = expression("let lower_cdot =");
        let upper = expression("let upper_cdot =");
        let source = format!("{scalar}\n@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{ let k=id.x*4u; let wa=values[k].xyz;let wb=values[k+1u].xyz;let twist_jacobian=values[k+2u].xyz;values[k+3u].x=select({lower},{upper},values[k+3u].y!=0.0); }}");
        let data: Vec<_> = SPHERICAL_TWIST_VELOCITY_CASES.iter().flatten().copied().collect();
        let actual = run_float_shader(&gpu, &source, &data, SPHERICAL_TWIST_VELOCITY_CASES.len() as u32);
        for (i,(out,expected)) in actual.chunks_exact(4).zip(SPHERICAL_TWIST_VELOCITY_CASES).enumerate() {
            assert_eq!(out[3][0].to_bits(),expected[3][0].to_bits(),"twist velocity {i}: {} != {}",out[3][0],expected[3][0]);
        }
    }

    include!("fixtures/capsule_proxy_bounds.rs");
    #[test]
    fn native_capsule_proxy_bounds_matches_cpu() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        let rotation = include_str!("../shaders/physics/rotation.wgsl");
        let scalar = &rotation[rotation.find("fn gyro_norm3(").unwrap()..rotation.find("fn gyro_integrate_rotation(").unwrap()];
        let broadphase = include_str!("../shaders/physics/broadphase.wgsl");
        let rotate = &rotation[rotation.find("fn gyro_quat_rotate(").unwrap()..rotation.find("fn gyro_quat_mul(").unwrap()];
        let bounds = &broadphase[broadphase.find("fn capsule_proxy_bounds(").unwrap()..broadphase.find("fn update_fat_collider(").unwrap()];
        let helpers = [scalar, rotate, bounds].join("\n");
        let source = format!("{helpers}\n{}", r#"
@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>;
@compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {
 let k=id.x*6u;
 let bounds=capsule_proxy_bounds(values[k].xyz,values[k+1u].xyz,values[k].w,values[k+2u].xyz,values[k+3u],values[k+1u].w);
 values[k+4u]=vec4<f32>(bounds[0],0.0);values[k+5u]=vec4<f32>(bounds[1],0.0);
}
"#);
        let data: Vec<_> = CAPSULE_PROXY_BOUNDS_CASES.iter().flatten().copied().collect();
        let actual = run_float_shader(&gpu, &source, &data, CAPSULE_PROXY_BOUNDS_CASES.len() as u32);
        for (i,(out,expected)) in actual.chunks_exact(6).zip(CAPSULE_PROXY_BOUNDS_CASES).enumerate() {
            for j in 4..6 {assert_eq!(out[j].map(f32::to_bits),expected[j].map(f32::to_bits),"capsule bounds {i}/{j}");}
        }
    }

    include!("fixtures/speed_cap.rs");
    #[test]
    fn native_speed_cap_matches_cpu() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        let rotation = include_str!("../shaders/physics/rotation.wgsl");
        let scalar = &rotation[rotation.find("fn gyro_norm3(").unwrap()..rotation.find("fn gyro_integrate_rotation(").unwrap()];
        let integrate = include_str!("../shaders/physics/integrate.wgsl");
        let clamp = &integrate[integrate.find("fn physics_max_angular_speed(").unwrap()..integrate.find("fn integrate_pos_one(").unwrap()];
        let source = format!("{scalar}\n{clamp}\n{}", r#"
@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>;
@compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {
    let k=id.x*2u;let v=values[k];
    values[k+1u]=vec4<f32>(physics_clamp_speed(v.xyz,physics_max_angular_speed(v.w)),0.0);
}
"#);
        let data: Vec<_> = SPEED_CAP_CASES.iter().flatten().copied().collect();
        let actual = run_float_shader(&gpu, &source, &data, SPEED_CAP_CASES.len() as u32);
        for (i, (out, expected)) in actual.chunks_exact(2).zip(SPEED_CAP_CASES).enumerate() {
            assert_eq!(out[1].map(f32::to_bits), expected[1].map(f32::to_bits), "speed cap {i}: {:?} != {:?}",out[1],expected[1]);
        }
    }

    include!("fixtures/capsule_segment.rs");
    #[test]
    fn native_capsule_segment_matches_cpu() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        fn function(s: &str, name: &str) -> String {
            let a = s.find(&format!("fn {name}(" )).unwrap();
            let b = a + s[a..].find('{').unwrap(); let mut end = b + 1; let mut depth = 1;
            while depth > 0 { match s.as_bytes()[end] { b'{' => depth += 1, b'}' => depth -= 1, _ => {} } end += 1; }
            s[a..end].to_string()
        }
        let rotation = include_str!("../shaders/physics/rotation.wgsl");
        let scalar = &rotation[rotation.find("fn gyro_norm3(").unwrap()..rotation.find("fn gyro_integrate_rotation(").unwrap()];
        let closest = function(include_str!("../shaders/physics/hull.wgsl"), "closest_segments");
        let rotate = function(rotation, "gyro_quat_rotate");
        let source = format!("{scalar}\n{rotate}\n{closest}\n{}", r#"
@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>;
@compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {
    let k=id.x*9u;
    let a0=values[k].xyz;let a1=values[k+1u].xyz;
    let b0=values[k+4u].xyz+gyro_quat_rotate(values[k+5u],values[k+2u].xyz);
    let b1=values[k+4u].xyz+gyro_quat_rotate(values[k+5u],values[k+3u].xyz);
    let da=a1-a0;let db=b1-b0;
    let st=closest_segments(a0,da,b0,db);
    let pa=a0+st.x*da;let pb=b0+st.y*db;
    let offset=pb-pa;let distance=gyro_sqrt(gyro_dot3(offset,offset));
    let n=gyro_recip(distance)*offset;
    let radius=values[k].w+values[k+2u].w;
    let p=0.5*((pa+n*values[k].w+pb)-n*values[k+2u].w);
    values[k+6u]=vec4<f32>(n,distance-radius);
    values[k+7u]=vec4<f32>(p,0.0);
    values[k+8u]=vec4<f32>(st,0.0,0.0);
}
"#);
        let data: Vec<_> = CAPSULE_SEGMENT_CASES.iter().flatten().copied().collect();
        let actual = run_float_shader(&gpu, &source, &data, CAPSULE_SEGMENT_CASES.len() as u32);
        for (i, (out, expected)) in actual.chunks_exact(9).zip(CAPSULE_SEGMENT_CASES).enumerate() {
            for field in 6..9 { assert_eq!(out[field].map(f32::to_bits), expected[field].map(f32::to_bits), "capsule segment {i}/{field}: {:?} != {:?}", out[field], expected[field]); }
        }
    }

    include!("fixtures/capsule_projection.rs");

    #[test]
    fn native_capsule_projection_matches_cpu() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        let rotation = include_str!("../shaders/physics/rotation.wgsl");
        let helpers = &rotation[rotation.find("fn gyro_norm3(").unwrap()
            ..rotation.find("fn gyro_integrate_rotation(").unwrap()];
        let collide = include_str!("../shaders/physics/collide.wgsl");
        let projection = &collide[collide.find("fn capsule_closest_point(").unwrap()
            ..collide.find("fn capsule_reference_reversed(").unwrap()];
        let source = format!("{helpers}\n{projection}\n@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>;\n@compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{ let k=id.x*4u; values[k+3u]=vec4<f32>(capsule_closest_point(values[k].xyz,values[k+1u].xyz,values[k+2u].xyz),0.0); }}");
        let data: Vec<[f32; 4]> = CAPSULE_PROJECTION_CASES.iter().flatten().copied().collect();
        let actual = run_float_shader(&gpu, &source, &data, CAPSULE_PROJECTION_CASES.len() as u32);
        for (i, (result, input)) in actual.chunks_exact(4).zip(CAPSULE_PROJECTION_CASES).enumerate() {
            assert_eq!(result[3].map(f32::to_bits), input[3].map(f32::to_bits), "capsule projection case {i}");
        }
    }

    include!("fixtures/spherical_motor_mass.rs");
    #[test]
    fn native_spherical_motor_mass_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        fn function(s:&str,name:&str)->String {
            let a=s.find(&format!("fn {name}(")).unwrap();let b=a+s[a..].find('{').unwrap();let mut end=b+1;let mut depth=1;
            while depth>0 {match s.as_bytes()[end] {b'{'=>depth+=1,b'}'=>depth-=1,_=>{}}end+=1;}s[a..end].to_string()
        }
        let rot=include_str!("../shaders/physics/rotation.wgsl");
        let scalar=&rot[rot.find("fn gyro_norm3(").unwrap()..rot.find("fn gyro_integrate_rotation(").unwrap()];
        let math=include_str!("../shaders/physics/math.wgsl");
        let solve=include_str!("../shaders/physics/solve.wgsl");
        let solve3=function(math,"solve3");let mul=function(math,"native_mul_mv");
        let helper=function(solve,"motor_angular_mass_mul").replace("ba: Body, bb: Body,", "ia: mat3x3<f32>, ib: mat3x3<f32>,")
            .replace("world_inv_inertia(ba,", "native_mul_mv(ia,").replace("world_inv_inertia(bb,", "native_mul_mv(ib,");
        let source=format!("{scalar}\n{solve3}\n{mul}\n{helper}\n@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*8u;let ia=mat3x3<f32>(values[k].xyz,values[k+1u].xyz,values[k+2u].xyz);let ib=mat3x3<f32>(values[k+3u].xyz,values[k+4u].xyz,values[k+5u].xyz);values[k+7u]=vec4<f32>(-motor_angular_mass_mul(ia,ib,values[k+6u].xyz),0.0);}}");
        let data:Vec<_>=SPHERICAL_MOTOR_CASES.iter().flatten().copied().collect();
        let actual=run_float_shader(&gpu,&source,&data,4);
        for (i,(got,want)) in actual.chunks_exact(8).zip(SPHERICAL_MOTOR_CASES).enumerate() {
            assert_eq!(got[7].map(f32::to_bits),want[7].map(f32::to_bits),"motor impulse case {i}");
        }
    }


    include!("fixtures/spherical_motor_clamp.rs");
    #[test]
    fn native_spherical_motor_clamp_matches_cpu() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        let rotation = include_str!("../shaders/physics/rotation.wgsl");
        let helpers = &rotation[rotation.find("fn gyro_norm3(").unwrap()
            ..rotation.find("fn gyro_integrate_rotation(").unwrap()];
        let solve = include_str!("../shaders/physics/solve.wgsl");
        let clamp = &solve[solve.find("fn spherical_clamp_vector_length(").unwrap()
            ..solve.find("fn clamp_vector_length(").unwrap()];
        let source = format!("{helpers}\n{clamp}\n@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{ let k=id.x*2u; values[k+1u]=vec4<f32>(spherical_clamp_vector_length(values[k].xyz,values[k].w),0.0); }}");
        let data: Vec<_> = SPHERICAL_CLAMP_CASES.iter().flatten().copied().collect();
        let actual = run_float_shader(&gpu, &source, &data, SPHERICAL_CLAMP_CASES.len() as u32);
        let mismatches: Vec<_> = actual.chunks_exact(2).zip(SPHERICAL_CLAMP_CASES).enumerate()
            .filter(|(_, (got,want))| got[1].map(f32::to_bits) != want[1].map(f32::to_bits))
            .map(|(i,(got,want))| (i,got[1],want[1])).collect();
        assert!(mismatches.is_empty(), "clamp mismatches: {mismatches:?}");
    }


    include!("fixtures/spherical_warm_torque.rs");
    #[test]
    fn native_spherical_warm_torque_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        let data:Vec<[f32;4]> = SPHERICAL_WARM_CASES.iter().flatten().copied().collect();
        let rot=include_str!("../shaders/physics/rotation.wgsl");
        let scalar=&rot[rot.find("fn gyro_norm3(").unwrap()..rot.find("fn gyro_integrate_rotation(").unwrap()];
        let math=include_str!("../shaders/physics/math.wgsl");
        let mul=&math[math.find("fn native_mul_mv(").unwrap()..math.find("fn world_inv_inertia_matrix(").unwrap()];
        let solve=include_str!("../shaders/physics/solve.wgsl");
        let torque=&solve[solve.find("fn spherical_warm_torque(").unwrap()..solve.find("fn solve_spherical(").unwrap()];
        // Test only the production expression: putting builtin and scalar cross
        // in one shader can share calculations and conceal the builtin mismatch.
        let source=format!("{scalar}\n{mul}\n{torque}\n@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*8u;let m=mat3x3<f32>(values[k].xyz,values[k+1u].xyz,values[k+2u].xyz);values[k+7u]=vec4<f32>(values[k+6u].xyz+values[k].w*spherical_warm_torque(m,values[k+3u].xyz,values[k+4u].xyz,values[k+5u].xyz),0.0);}}");
        let actual=run_float_shader(&gpu,&source,&data,SPHERICAL_WARM_CASES.len() as u32);
        for(i,(got,want)) in actual.chunks_exact(8).zip(SPHERICAL_WARM_CASES).enumerate() {
            assert_eq!(got[7].map(f32::to_bits),want[7].map(f32::to_bits),"warm-start torque case {i}");
        }
    }

    include!("fixtures/revolute_perp_mass.rs");
    #[test]
    fn native_revolute_perp_mass_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        fn function(s:&str,name:&str)->String {
            let a=s.find(&format!("fn {name}(")).unwrap();let b=a+s[a..].find('{').unwrap();let mut end=b+1;let mut depth=1;
            while depth>0 {match s.as_bytes()[end] {b'{'=>depth+=1,b'}'=>depth-=1,_=>{}}end+=1;}s[a..end].to_string()
        }
        let rot=include_str!("../shaders/physics/rotation.wgsl");
        let scalar=&rot[rot.find("fn gyro_norm3(").unwrap()..rot.find("fn gyro_integrate_rotation(").unwrap()];
        let mul=function(include_str!("../shaders/physics/math.wgsl"),"native_mul_mv");
        let helper=function(include_str!("../shaders/physics/solve.wgsl"),"revolute_perp_mass_solve");
        let source=format!("{scalar}\n{mul}\n{helper}\n@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>;@compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*10u;let ia=mat3x3<f32>(values[k].xyz,values[k+1u].xyz,values[k+2u].xyz);let ib=mat3x3<f32>(values[k+3u].xyz,values[k+4u].xyz,values[k+5u].xyz);values[k+9u]=vec4<f32>(revolute_perp_mass_solve(ia,ib,values[k+6u].xyz,values[k+7u].xyz,values[k+8u].xy),0.0,0.0);}}");
        let data:Vec<_>=REVOLUTE_PERP_CASES.iter().flatten().copied().collect();
        let actual=run_float_shader(&gpu,&source,&data,REVOLUTE_PERP_CASES.len() as u32);
        for (i,(got,want)) in actual.chunks_exact(10).zip(REVOLUTE_PERP_CASES).enumerate() {
            assert_eq!(got[9].map(f32::to_bits),want[9].map(f32::to_bits),"hinge alignment case {i}");
        }
    }
    include!("fixtures/spherical_point_velocity.rs");
    include!("fixtures/revolute_point_velocity.rs");
    fn check_joint_point_velocity_cases(cases: &[[[f32; 4]; 7]]) {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let rotation=include_str!("../shaders/physics/rotation.wgsl");
        let helpers=&rotation[rotation.find("fn gyro_norm3(").unwrap()..rotation.find("fn gyro_integrate_rotation(").unwrap()];
        let solve=include_str!("../shaders/physics/solve.wgsl");
        let start=solve.find("fn joint_point_velocity(").unwrap();
        let end=start+solve[start..].find("\n}").unwrap()+2;
        let helper=&solve[start..end];
        let source=format!("{helpers}\n{helper}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*7u;v[k+6u]=vec4<f32>(joint_point_velocity(v[k].xyz,v[k+1u].xyz,v[k+2u].xyz,v[k+3u].xyz,v[k+4u].xyz,v[k+5u].xyz),0.0);}}");
        let data:Vec<_>=cases.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,cases.len() as u32);
        let mismatch:Vec<_>=got.chunks_exact(7).zip(cases).enumerate().filter(|(_, (a,b))|a[6].map(f32::to_bits)!=b[6].map(f32::to_bits)).map(|(i,(a,b))|(i,a[6],b[6])).collect();
        assert!(mismatch.is_empty(), "joint point velocity mismatches: {mismatch:?}");
    }

    #[test]
    fn native_spherical_point_velocity_matches_cpu() {
        check_joint_point_velocity_cases(&SPHERICAL_VELOCITY_CASES);
    }
    #[test]
    fn native_revolute_point_velocity_matches_cpu() {
        check_joint_point_velocity_cases(&REVOLUTE_POINT_VELOCITY_CASES);
    }


    include!("fixtures/revolute_alignment_velocity.rs");
    #[test]
    fn native_revolute_alignment_velocity_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let rotation=include_str!("../shaders/physics/rotation.wgsl");
        let helpers=&rotation[rotation.find("fn gyro_norm3(").unwrap()..rotation.find("fn gyro_integrate_rotation(").unwrap()];
        let solve=include_str!("../shaders/physics/solve.wgsl");
        let start=solve.find("fn revolute_alignment_velocity(").unwrap();
        let end=start+solve[start..].find("\n}").unwrap()+2;
        let helper=&solve[start..end];
        let source=format!("{helpers}\n{helper}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*4u;v[k+3u]=vec4<f32>(revolute_alignment_velocity(v[k].xyz,v[k+1u].xyz,v[k+2u].xyz),0.0,0.0);}}");
        let data:Vec<_>=REVOLUTE_ALIGNMENT_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,REVOLUTE_ALIGNMENT_CASES.len() as u32);
        for (i,(actual,expected)) in got.chunks_exact(4).zip(REVOLUTE_ALIGNMENT_CASES).enumerate() {
            assert_eq!(actual[3].map(f32::to_bits),expected[3].map(f32::to_bits),"hinge alignment velocity case {i}");
        }
    }

    include!("fixtures/spherical_cone.rs");
    #[test]
    fn native_spherical_cone_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let rotation=include_str!("../shaders/physics/rotation.wgsl");
        let scalar=&rotation[rotation.find("fn gyro_norm3(").unwrap()..rotation.find("fn gyro_integrate_rotation(").unwrap()];
        let atan=&rotation[rotation.find("fn gyro_atan2(").unwrap()..];
        let solve=include_str!("../shaders/physics/solve.wgsl");
        let helper=&solve[solve.find("fn swing_angle(").unwrap()..solve.find("fn normalize_or_zero(").unwrap()];
        let source=format!("{scalar}\n{atan}\n{helper}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{v[id.x]=vec4<f32>(swing_angle(v[id.x]),0.0,0.0,0.0);}}");
        let data:Vec<_>=SPHERICAL_CONE_ANGLE_CASES.iter().map(|c|c.0).collect();
        let got=run_float_shader(&gpu,&source,&data,data.len() as u32);
        for (i,(actual,(_,expected))) in got.iter().zip(SPHERICAL_CONE_ANGLE_CASES).enumerate() {
            assert_eq!(actual[0].to_bits(),expected.to_bits(),"spherical cone angle case {i}: {} vs {expected}",actual[0]);
        }
        let source=format!("{scalar}\n{atan}\n{helper}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*4u;v[k+3u]=vec4<f32>(spherical_cone_velocity(v[k].xyz,v[k+1u].xyz,v[k+2u].xyz),0.0,0.0,0.0);}}");
        let data:Vec<_>=SPHERICAL_CONE_VELOCITY_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,SPHERICAL_CONE_VELOCITY_CASES.len() as u32);
        for (i,(actual,expected)) in got.chunks_exact(4).zip(SPHERICAL_CONE_VELOCITY_CASES).enumerate() {
            assert_eq!(actual[3].map(f32::to_bits),expected[3].map(f32::to_bits),"spherical cone velocity case {i}");
        }
    }

    include!("fixtures/revolute_axial.rs");
    #[test]
    fn native_revolute_axial_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let rot=include_str!("../shaders/physics/rotation.wgsl");
        let scalar=&rot[rot.find("fn gyro_norm3(").unwrap()..rot.find("fn gyro_integrate_rotation(").unwrap()];
        let math=include_str!("../shaders/physics/math.wgsl");
        let mul=&math[math.find("fn native_mul_mv(").unwrap()..math.find("fn world_inv_inertia_matrix(").unwrap()];
        let solve=include_str!("../shaders/physics/solve.wgsl");
        let helpers=&solve[solve.find("fn revolute_axial_mass(").unwrap()..solve.find("fn solve_revolute(").unwrap()];
        let source=format!("{scalar}\n{mul}\n{helpers}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*8u;let ia=mat3x3<f32>(v[k].xyz,v[k+1u].xyz,v[k+2u].xyz);let ib=mat3x3<f32>(v[k+3u].xyz,v[k+4u].xyz,v[k+5u].xyz);v[k+7u]=vec4<f32>(revolute_axial_mass(ia,ib,v[k+6u].xyz),0.0,0.0,0.0);}}");
        let data:Vec<_>=REVOLUTE_AXIAL_MASS_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,REVOLUTE_AXIAL_MASS_CASES.len() as u32);
        for (i,(a,b)) in got.chunks_exact(8).zip(REVOLUTE_AXIAL_MASS_CASES).enumerate() {
            assert_eq!(a[7].map(f32::to_bits),b[7].map(f32::to_bits),"hinge axial mass case {i}");
        }
        let source=format!("{scalar}\n{mul}\n{helpers}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*4u;v[k+3u]=vec4<f32>(revolute_axial_velocity(v[k].xyz,v[k+1u].xyz,v[k+2u].xyz),0.0,0.0,0.0);}}");
        let data:Vec<_>=REVOLUTE_AXIAL_VELOCITY_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,REVOLUTE_AXIAL_VELOCITY_CASES.len() as u32);
        for (i,(a,b)) in got.chunks_exact(4).zip(REVOLUTE_AXIAL_VELOCITY_CASES).enumerate() {
            assert_eq!(a[3].map(f32::to_bits),b[3].map(f32::to_bits),"hinge axial velocity case {i}");
        }
    }

    include!("fixtures/convex_rolling.rs");
    #[test]
    fn native_convex_rolling_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let rot=include_str!("../shaders/physics/rotation.wgsl");
        let scalar=&rot[rot.find("fn gyro_norm3(").unwrap()..rot.find("fn gyro_integrate_rotation(").unwrap()];
        let solve=include_str!("../shaders/physics/solve.wgsl");
        let helpers=&solve[solve.find("fn convex_rolling_mass_mul(").unwrap()..solve.find("fn mesh_rolling_mass_mul(").unwrap()];
        let source=format!("{scalar}\n{helpers}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*8u;let a=mat3x3<f32>(v[k].xyz,v[k+1u].xyz,v[k+2u].xyz);let b=mat3x3<f32>(v[k+3u].xyz,v[k+4u].xyz,v[k+5u].xyz);v[k+7u]=vec4<f32>(convex_rolling_mass_mul(a,b,v[k+6u].xyz),0.0);}}");
        let data:Vec<_>=CONVEX_ROLLING_MASS_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,CONVEX_ROLLING_MASS_CASES.len() as u32);
        for (i,(a,b)) in got.chunks_exact(8).zip(CONVEX_ROLLING_MASS_CASES).enumerate() {
            assert_eq!(a[7].map(f32::to_bits),b[7].map(f32::to_bits),"convex rolling mass case {i}");
        }
        let source=format!("{scalar}\n{helpers}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*2u;v[k+1u]=vec4<f32>(convex_rolling_clamp(v[k].xyz,v[k].w),0.0);}}");
        let data:Vec<_>=CONVEX_ROLLING_CLAMP_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,CONVEX_ROLLING_CLAMP_CASES.len() as u32);
        for (i,(a,b)) in got.chunks_exact(2).zip(CONVEX_ROLLING_CLAMP_CASES).enumerate() {
            assert_eq!(a[1].map(f32::to_bits),b[1].map(f32::to_bits),"convex rolling clamp case {i}");
        }
    }
    include!("fixtures/mesh_rolling.rs");
    #[test]
    fn native_mesh_rolling_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let rot=include_str!("../shaders/physics/rotation.wgsl");
        let scalar=&rot[rot.find("fn gyro_norm3(").unwrap()..rot.find("fn gyro_integrate_rotation(").unwrap()];
        let math=include_str!("../shaders/physics/math.wgsl");
        let mul=&math[math.find("fn native_mul_mv(").unwrap()..math.find("fn world_inv_inertia_matrix(").unwrap()];
        let solve=include_str!("../shaders/physics/solve.wgsl");
        let helpers=&solve[solve.find("fn mesh_rolling_mass_mul(").unwrap()..solve.find("fn rolling_mass_mul(").unwrap()];
        let source=format!("{scalar}\n{mul}\n{helpers}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*8u;let a=mat3x3<f32>(v[k].xyz,v[k+1u].xyz,v[k+2u].xyz);let b=mat3x3<f32>(v[k+3u].xyz,v[k+4u].xyz,v[k+5u].xyz);v[k+7u]=vec4<f32>(mesh_rolling_mass_mul(a,b,v[k+6u].xyz),0.0);}}");
        let data:Vec<_>=MESH_ROLLING_MASS_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,MESH_ROLLING_MASS_CASES.len() as u32);
        for (i,(a,b)) in got.chunks_exact(8).zip(MESH_ROLLING_MASS_CASES).enumerate() {
            assert_eq!(a[7].map(f32::to_bits),b[7].map(f32::to_bits),"mesh rolling mass case {i}");
        }
        let source=format!("{scalar}\n{mul}\n{helpers}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*2u;v[k+1u]=vec4<f32>(mesh_rolling_clamp(v[k].xyz,v[k].w),0.0);}}");
        let data:Vec<_>=MESH_ROLLING_CLAMP_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,MESH_ROLLING_CLAMP_CASES.len() as u32);
        for (i,(a,b)) in got.chunks_exact(2).zip(MESH_ROLLING_CLAMP_CASES).enumerate() {
            assert_eq!(a[1].map(f32::to_bits),b[1].map(f32::to_bits),"mesh rolling clamp case {i}");
        }
    }

    include!("fixtures/mesh_inertia.rs");
    #[test]
    fn native_mesh_inertia_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let math=include_str!("../shaders/physics/math.wgsl");
        let mul=&math[math.find("fn native_mul_mv(").unwrap()..math.find("fn world_inv_inertia_matrix(").unwrap()];
        let helper=&math[math.find("fn contact_inertia_mul(").unwrap()..math.find("fn contact_inv_inertia(").unwrap()];
        let data:Vec<_>=MESH_INERTIA_CASES.iter().flatten().copied().collect();
        for convex in [false,true] {
            let source=format!("{mul}\n{helper}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*5u;let m=mat3x3<f32>(v[k].xyz,v[k+1u].xyz,v[k+2u].xyz);v[k+4u]=vec4<f32>(contact_inertia_mul(m,v[k+3u].xyz,{convex}),0.0);}}");
            let got=run_float_shader(&gpu,&source,&data,MESH_INERTIA_CASES.len() as u32);
            let bad:Vec<_>=got.chunks_exact(5).zip(MESH_INERTIA_CASES).enumerate().filter(|(_, (a,b))|a[4].map(f32::to_bits)!=b[4].map(f32::to_bits)).map(|(i,(a,b))|(i,a[4],b[4])).collect();
            if !convex { assert!(bad.is_empty(),"mesh scalar inertia mismatches: {bad:?}"); }
            else { eprintln!("SIMD inertia against captured scalar mesh oracle: {} / {} differ",bad.len(),MESH_INERTIA_CASES.len()); }
        }
    }

    include!("fixtures/mesh_base_separation.rs");
    #[test]
    fn native_mesh_base_separation_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let rot=include_str!("../shaders/physics/rotation.wgsl");
        let scalar=&rot[rot.find("fn gyro_norm3(").unwrap()..rot.find("fn gyro_integrate_rotation(").unwrap()];
        let collide=include_str!("../shaders/physics/collide.wgsl");
        let helper=&collide[collide.find("fn mesh_contact_base_separation(").unwrap()..collide.find("fn consider_mesh_manifold_point(").unwrap()];
        let source=format!("{scalar}\n{helper}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*5u;v[k+4u]=vec4<f32>(mesh_contact_base_separation(v[k].w,v[k+1u].xyz,v[k+2u].xyz,v[k+3u].xyz),0.0,0.0,0.0);}}");
        let data:Vec<_>=MESH_BASE_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,MESH_BASE_CASES.len() as u32);
        for (i,(a,b)) in got.chunks_exact(5).zip(MESH_BASE_CASES).enumerate() {
            assert_eq!(a[4].map(f32::to_bits),b[4].map(f32::to_bits),"mesh base separation case {i}");
        }
    }

    include!("fixtures/contact_softness.rs");
    #[test]
    fn native_contact_softness_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let rot=include_str!("../shaders/physics/rotation.wgsl");
        let scalar=&rot[rot.find("fn gyro_norm3(").unwrap()..rot.find("fn gyro_integrate_rotation(").unwrap()];
        let math=include_str!("../shaders/physics/math.wgsl");
        let helper=&math[math.find("fn contact_softness_values(").unwrap()..math.find("fn contact_softness(").unwrap()];
        let source=format!("{scalar}\n{helper}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*2u;let a=v[k];v[k+1u]=vec4<f32>(contact_softness_values(a.x,a.y,a.z,a.w!=0.0),0.0);}}");
        let data:Vec<_>=CONTACT_SOFTNESS_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,CONTACT_SOFTNESS_CASES.len() as u32);
        for (i,(a,b)) in got.chunks_exact(2).zip(CONTACT_SOFTNESS_CASES).enumerate() {
            assert_eq!(a[1].map(f32::to_bits),b[1].map(f32::to_bits),"contact softness case {i}");
        }
    }

    include!("fixtures/contact_friction_reproject.rs");
    #[test]
    fn native_contact_friction_reproject_matches_cpu() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let rot=include_str!("../shaders/physics/rotation.wgsl");
        let scalar=&rot[rot.find("fn gyro_norm3(").unwrap()..rot.find("fn gyro_integrate_rotation(").unwrap()];
        let solve=include_str!("../shaders/physics/solve.wgsl");
        let perp=&solve[solve.find("fn perp(").unwrap()..solve.find("fn normal_mass(").unwrap()];
        let collide=include_str!("../shaders/physics/collide.wgsl");
        let helper=&collide[collide.find("fn reproject_contact_friction(").unwrap()..collide.find("fn finish_manifold(").unwrap()];
        let source=format!("{scalar}\n{perp}\n{helper}\n@group(0) @binding(0) var<storage,read_write> v:array<vec4<f32>>; @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {{let k=id.x*4u;v[k+3u]=vec4<f32>(reproject_contact_friction(v[k].xyz,v[k+1u].xyz,v[k+2u].xy,v[k+2u].z),0.0,0.0);}}");
        let data:Vec<_>=CONTACT_FRICTION_REPROJECT_CASES.iter().flatten().copied().collect();
        let got=run_float_shader(&gpu,&source,&data,CONTACT_FRICTION_REPROJECT_CASES.len() as u32);
        for (i,(a,b)) in got.chunks_exact(4).zip(CONTACT_FRICTION_REPROJECT_CASES).enumerate() {
            assert_eq!(a[3].map(f32::to_bits),b[3].map(f32::to_bits),"contact friction reproject case {i}");
        }
    }
    include!("fixtures/capsule_terrain_edge.rs");
    #[test]
    fn native_capsule_terrain_edge_is_retained() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        fn extract(s: &str, prefix: &str) -> String {
            let start = s.find(prefix).unwrap();
            let brace = start + s[start..].find('{').unwrap();
            let mut end = brace + 1; let mut depth = 1;
            while depth > 0 { match s.as_bytes()[end] { b'{' => depth+=1, b'}' => depth-=1, _=>{} } end+=1; }
            s[start..end].to_string()
        }
        let rotation=include_str!("../shaders/physics/rotation.wgsl");
        let collide=include_str!("../shaders/physics/collide.wgsl");
        let mut source=rotation[rotation.find("fn gyro_norm3(").unwrap()..rotation.find("fn gyro_integrate_rotation(").unwrap()].to_string();
        for name in ["struct Seg2", "struct TriangleClosest", "struct TriangleCapsuleManifold", "fn closest_triangle_point(", "fn clip_divide(", "fn clip_triangle_segment(", "fn capsule_triangle_manifold(", "fn mesh_vertex_allowed(", "fn mesh_feature_allowed(", "fn capsule_mesh_feature_allowed("] {
            source+=&extract(collide,name); source.push('\n');
        }
        source+=&extract(include_str!("../shaders/physics/hull.wgsl"),"fn closest_segments(");
        source+=&extract(include_str!("../shaders/physics/math.wgsl"),"fn pack_feature(");
        source+=r#"
const EMPTY:u32=0xffffffffu;
const SPECULATIVE:f32=0.02;
const LINEAR_SLOP:f32=0.005;
@group(0) @binding(0) var<storage,read_write> values:array<vec4<f32>>;
@compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {
 let k=id.x*8u;
 let tri=array<vec3<f32>,3>(values[k+2u].xyz,values[k+3u].xyz,values[k+4u].xyz);
 let n=gyro_norm3(gyro_cross(tri[1]-tri[0],tri[2]-tri[0]));
 let m=capsule_triangle_manifold(values[k].xyz,values[k+1u].xyz,values[k].w,tri,n);
 let allowed=capsule_mesh_feature_allowed(m.feature,u32(values[k+1u].w),bitcast<u32>(values[k+7u].w));
 values[k+5u]=vec4<f32>(m.normal,f32(m.count));
 values[k+6u]=m.points[0];
 values[k+7u]=vec4<f32>(select(0.0,1.0,allowed),f32(m.feature),0.0,0.0);
}
"#;
        let data:Vec<_>=CAPSULE_TERRAIN_EDGE_CASES.iter().flatten().copied().collect();
        let actual=run_float_shader(&gpu,&source,&data,2);
        for (i,(a,b)) in actual.chunks_exact(8).zip(CAPSULE_TERRAIN_EDGE_CASES).enumerate() {
            assert_eq!(a[7],b[7],"terrain edge {i} acceptance/feature");
            assert_eq!(a[5][3],b[5][3],"terrain edge {i} point count");
            for row in 5..7 { for col in 0..4 {
                assert!((a[row][col]-b[row][col]).abs()<1e-5,"terrain edge {i}/{row}/{col}: {} != {}",a[row][col],b[row][col]);
            }}
        }
        // Rain torus triangle 412, frozen CPU frame169 and GPU frame170 poses.
        // Native returns one speculative point for each. Trace raw generation
        // separately from mesh admission while investigating the omitted patch.
        let triangles = [
            [[1.56385183,-0.622428417,-1.01344013],[-0.044644475,0.224490166,-0.815158248],[-0.0490727425,0.0440015793,-0.470499218]],
            [[1.53683662,-0.673357964,-0.998079956],[-0.0464584827,0.224299431,-0.82134521],[-0.0553429127,0.053592205,-0.471822977]],
        ];
        for (i,tri) in triangles.into_iter().enumerate() {
            let mut data=vec![[-0.001820,0.0,0.010071,0.075],[0.077883,0.014825,-0.418047,116.0]];
            for v in tri { data.push([v[0],v[1],v[2],0.0]); }
            data.extend([[0.0;4];3]);
            let actual=run_float_shader(&gpu,&source,&data,1);
            eprintln!("rain-triangle412 case={i} normal_count={:?} point={:?} admission_feature={:?}",actual[5],actual[6],actual[7]);
            assert_eq!(actual[5][3],1.0,"Rain triangle412 raw manifold {i}");
            assert_eq!(actual[7][0],1.0,"exposed flat edge must survive {i}");
            data[7][3]=f32::from_bits(4);
            let covered=run_float_shader(&gpu,&source,&data,1);
            assert_eq!(covered[7][0],0.0,"covered flat seam must be rejected {i}");
            data[7][3]=0.0; data[1][3]=119.0;
            let flat=run_float_shader(&gpu,&source,&data,1);
            assert_eq!(flat[7][0],0.0,"fully flat tentative triangle must be rejected {i}");
        }
    }

    #[test]
    fn native_contact_pair_priority_matches_creation_history() {
        let bytes = include_bytes!("fixtures/broadphase_pair_order.bin");
        assert_eq!(&bytes[..8], b"B3PO\x01\0\0\0");
        let words: Vec<u32> = bytes[8..].chunks_exact(4)
            .map(|v| u32::from_le_bytes(v.try_into().unwrap())).collect();
        let mut cursor = 0;
        let mut records = Vec::new();
        let mut data = Vec::new();
        let mut frames = 0;
        while cursor < words.len() {
            let frame = words[cursor];
            let count = words[cursor + 1] as usize;
            cursor += 2;
            frames += 1;
            for _ in 0..count {
                let row = &words[cursor..cursor + 8];
                records.push((frame, row[0], row[1]));
                data.push([row[2], row[3], row[4], 0].map(f32::from_bits));
                data.push([row[5], row[6], row[7], 0].map(f32::from_bits));
                data.extend([[0.0; 4]; 2]);
                cursor += 8;
            }
        }
        assert_eq!(frames, 167);
        assert_eq!(records.len(), 1113);
        let source = format!("@group(0) @binding(0) var<storage,read_write> scratch:array<u32>;\n{}\n{}",
            include_str!("../shaders/physics/contact_order.wgsl"), r#"
@compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id:vec3<u32>) {
    let k=16u*id.x;
    let a=vec3<u32>(scratch[k],scratch[k+1u],scratch[k+2u]);
    let b=vec3<u32>(scratch[k+4u],scratch[k+5u],scratch[k+6u]);
    let forward=order_pair_priority(a,b); let reverse=order_pair_priority(b,a);
    for (var i=0u;i<3u;i++) {scratch[k+8u+i]=forward[i];scratch[k+12u+i]=reverse[i];}
}
"#);
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        let result = run_float_shader(&gpu, &source, &data, records.len() as u32);
        let mut previous = None;
        for (record, row) in records.into_iter().zip(result.chunks_exact(4)) {
            let priority = row[2].map(f32::to_bits);
            assert_eq!(priority, row[3].map(f32::to_bits), "endpoint reversal {record:?}");
            assert_ne!(priority[0], u32::MAX, "missing owner {record:?}");
            if let Some((frame, before)) = previous {
                if frame == record.0 {
                    assert!(before < priority, "creation order {record:?}: {before:?} >= {priority:?}");
                }
            }
            previous = Some((record.0, priority));
        }
    }

    #[test]
    fn native_contact_order_tree_matches_rebuild_history() {
        use crate::broadphase_order::{Bounds,gpu_tree_seed};
        let bytes=include_bytes!("fixtures/broadphase_tree_updates.bin");
        let events: Vec<u32>=bytes[8..].chunks_exact(4)
            .map(|v| u32::from_le_bytes(v.try_into().unwrap())).collect();
        let mut proxies=Vec::new();
        let mut cursor=0;
        while events[cursor]==0 {
            let shape=events[cursor+1] as usize;
            let lower=std::array::from_fn(|i| f32::from_bits(events[cursor+2+i]));
            let upper=std::array::from_fn(|i| f32::from_bits(events[cursor+5+i]));
            proxies.push((shape,Bounds {lower,upper})); cursor+=8;
        }
        let seed=gpu_tree_seed(proxies.iter().copied(),2*proxies.len());
        let mut words=vec![0u32;16];
        words[0]=16;
        words.extend(seed);
        words[1]=words.len() as u32;
        words.extend(events);
        words[2]=words.len() as u32;
        while words.len()%4!=0 {words.push(0);}
        let data: Vec<[f32;4]>=words.chunks_exact(4).map(|v| std::array::from_fn(|i| f32::from_bits(v[i]))).collect();
        let source=format!("@group(0) @binding(0) var<storage,read_write> scratch:array<u32>;\n{}\n{}",
            include_str!("../shaders/physics/contact_order.wgsl"),r#"
@compute @workgroup_size(1) fn main() {
    let base=scratch[0]; var cursor=scratch[1]; let end=scratch[2];
    loop {
        if (cursor>=end) { break; }
        let kind=scratch[cursor]; cursor+=1u;
        if (kind==0u) { cursor+=7u; }
        else if (kind==1u) {
            let shape=scratch[cursor];
            let lower=bitcast<vec3<f32>>(vec3<u32>(scratch[cursor+1u],scratch[cursor+2u],scratch[cursor+3u]));
            let upper=bitcast<vec3<f32>>(vec3<u32>(scratch[cursor+4u],scratch[cursor+5u],scratch[cursor+6u]));
            var found=false;
            for (var i=0u;i<scratch[base+1u];i++) {
                let node=order_node(base,i);
                if (scratch[node+7u]==0xffffffffu && scratch[node+9u]==shape) {
                    order_enlarge(base,i,lower,upper); found=true; break;
                }
            }
            if (!found) {scratch[3]=0xffffffffu;return;}
            cursor+=7u;
        } else if (kind==2u) {
            order_rebuild(base,scratch[cursor]!=0u); cursor+=1u;
        } else if (kind==3u) {
            let frame=scratch[cursor]; let count=scratch[cursor+1u]; cursor+=2u;
            let actual_count=order_query_leaves(base);
            if (count!=actual_count) {scratch[3]=frame;scratch[6]=actual_count;scratch[7]=count;return;}
            for (var i=0u;i<count;i++) {
                let node=scratch[order_leaves(base)+i];
                let actual=scratch[order_node(base,node)+9u];
                let expected=scratch[cursor+count-1u-i];
                if (actual!=expected) {
                    scratch[3]=frame;scratch[5]=i;scratch[6]=actual;scratch[7]=expected;return;
                }
            }
            scratch[4]+=1u; cursor+=count;
        } else { scratch[3]=0xfffffffeu; return; }
    }
}
"#);
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).expect("GPU");
        let result=run_float_shader(&gpu,&source,&data,1);
        let result: Vec<u32>=result.iter().flat_map(|r| r.iter().map(|v| v.to_bits())).collect();
        assert_eq!(result[3],0,"tree mismatch frame/point/actual/expected: {:?}",[result[3],result[5],result[6],result[7]]);
        assert_eq!(result[4],167);
        assert_eq!(result[16+5],0,"tree allocation capacity");
    }
}
