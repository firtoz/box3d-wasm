//! Exercise the real GPU grid boundary without allocating a full physics world.
use crate::dispatch::{linear_dispatch_groups, LinearDispatch};
use wgpu::util::DeviceExt;

#[test]
fn tiled_dispatch_direct_indirect_and_cached_cover_each_element_once() {
    let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).unwrap();
    let d = &gpu.device;
    let q = &gpu.queue;
    d.push_error_scope(wgpu::ErrorFilter::Validation);
    let maximum = 65_537u32 * 64 + 64;
    let bytes = u64::from(maximum) * 4;
    let uniform = d.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: Some("dispatch-boundary-count"), contents: &[0; 16],
        usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
    });
    let output = d.create_buffer(&wgpu::BufferDescriptor {
        label: Some("dispatch-boundary-output"), size: bytes,
        usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC | wgpu::BufferUsages::COPY_DST,
        mapped_at_creation: false,
    });
    let scratch = d.create_buffer(&wgpu::BufferDescriptor {
        label: Some("dispatch-boundary-args-source"), size: 16,
        usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC,
        mapped_at_creation: false,
    });
    let indirect = d.create_buffer(&wgpu::BufferDescriptor {
        label: Some("dispatch-boundary-args"), size: 16,
        usage: wgpu::BufferUsages::INDIRECT | wgpu::BufferUsages::COPY_DST,
        mapped_at_creation: false,
    });
    let staging = d.create_buffer(&wgpu::BufferDescriptor {
        label: Some("dispatch-boundary-readback"), size: bytes,
        usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
        mapped_at_creation: false,
    });
    let layout = d.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: None,
        entries: &[0, 1, 2].map(|binding| wgpu::BindGroupLayoutEntry {
            binding, visibility: wgpu::ShaderStages::COMPUTE,
            ty: wgpu::BindingType::Buffer {
                ty: if binding == 0 { wgpu::BufferBindingType::Uniform }
                    else { wgpu::BufferBindingType::Storage { read_only: false } },
                has_dynamic_offset: binding == 0, min_binding_size: None,
            }, count: None,
        }),
    });
    let group = d.create_bind_group(&wgpu::BindGroupDescriptor {
        label: None, layout: &layout,
        entries: &[
            wgpu::BindGroupEntry { binding: 0, resource: uniform.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 1, resource: output.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 2, resource: scratch.as_entire_binding() },
        ],
    });
    let pl = d.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
        label: None, bind_group_layouts: &[&layout], push_constant_ranges: &[],
    });
    let source = format!("{}\n{}", include_str!("../shaders/physics/dispatch.wgsl"), r#"
@group(0) @binding(0) var<uniform> config: vec4<u32>;
@group(0) @binding(1) var<storage, read_write> result: array<atomic<u32>>;
@group(0) @binding(2) var<storage, read_write> args: array<u32>;
@compute @workgroup_size(1) fn produce() {
    let grid = linear_dispatch_groups(config.y);
    args[0] = grid.x; args[1] = grid.y; args[2] = grid.z; args[3] = 0u;
}
@compute @workgroup_size(64) fn visit(
    @builtin(global_invocation_id) id: vec3<u32>,
    @builtin(workgroup_id) group: vec3<u32>,
    @builtin(local_invocation_index) lane: u32) {
    let i = linear_invocation_id(id, 64u);
    if (i >= config.x) { return; }
    let expected = linear_workgroup_id(group) * 64u + lane;
    atomicAdd(&result[i], select(2u, 1u, i == expected));
}
"#);
    let shader = d.create_shader_module(wgpu::ShaderModuleDescriptor {
        label: None, source: wgpu::ShaderSource::Wgsl(source.into()),
    });
    let pipeline = |name| d.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
        label: None, layout: Some(&pl), module: &shader, entry_point: Some(name),
        compilation_options: Default::default(), cache: None,
    });
    let visit = pipeline("visit");
    let produce = pipeline("produce");
    let mut modes = vec!["direct", "indirect"];
    #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
    modes.extend(["cached-direct", "cached-indirect"]);
    for groups in [0u32, 1, 65_535, 65_536, 65_537] {
        let n = if groups == 0 { 0 } else { (groups - 1) * 64 + 17 };
        for mode in &modes {
            q.write_buffer(&uniform, 0, bytemuck::cast_slice(&[n, groups, 0, 0]));
            let mut enc = d.create_command_encoder(&Default::default());
            enc.clear_buffer(&output, 0, None);
            if mode.starts_with("cached") {
                #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
                {
                    use crate::native_command_cache::{Command, RadixCache};
                    let commands = if *mode == "cached-direct" {
                        vec![Command::DispatchLinear(&visit, groups)]
                    } else {
                        vec![Command::Dispatch(&produce, 1),
                            Command::CopyArgs { source: 0, destination: 0, bytes: 16 },
                            Command::Indirect(&visit, 0)]
                    };
                    let cache = RadixCache::record(d, &group, &pl, &indirect,
                        Some(&scratch), 0, [groups, 0, 0], &commands);
                    enc.transition_resources([
                        wgpu::BufferTransition { buffer: &uniform, state: wgpu::BufferUses::UNIFORM },
                        wgpu::BufferTransition { buffer: &output, state: wgpu::BufferUses::STORAGE_READ_WRITE },
                        wgpu::BufferTransition { buffer: &scratch, state: wgpu::BufferUses::STORAGE_READ_WRITE },
                        wgpu::BufferTransition { buffer: &indirect, state: wgpu::BufferUses::INDIRECT },
                    ].into_iter(), std::iter::empty());
                    cache.encode(&mut enc);
                }
            } else {
                if *mode == "indirect" {
                    {
                        let mut pass = enc.begin_compute_pass(&Default::default());
                        pass.set_pipeline(&produce); pass.set_bind_group(0, &group, &[0]);
                        pass.dispatch_workgroups(1, 1, 1);
                    }
                    enc.copy_buffer_to_buffer(&scratch, 0, &indirect, 0, 16);
                }
                let mut pass = enc.begin_compute_pass(&Default::default());
                pass.set_pipeline(&visit); pass.set_bind_group(0, &group, &[0]);
                if *mode == "direct" { pass.dispatch_linear(groups); }
                else { pass.dispatch_workgroups_indirect(&indirect, 0); }
            }
            enc.copy_buffer_to_buffer(&output, 0, &staging, 0, bytes);
            q.submit(Some(enc.finish()));
            let (tx, rx) = std::sync::mpsc::channel();
            staging.slice(..).map_async(wgpu::MapMode::Read, move |v| tx.send(v).unwrap());
            d.poll(wgpu::PollType::Wait).unwrap(); rx.recv().unwrap().unwrap();
            {
                let data = staging.slice(..).get_mapped_range();
                let values = bytemuck::cast_slice::<u8, u32>(&data);
                assert!(values[..n as usize].iter().all(|&v| v == 1), "{mode}, {groups}: missing or duplicate element");
                assert!(values[n as usize..].iter().all(|&v| v == 0), "{mode}, {groups}: padding wrote past logical end");
            }
            staging.unmap();
            let grid = linear_dispatch_groups(groups);
            eprintln!("{mode}: {groups} groups -> {grid:?}, {n} elements exactly once");
        }
    }
    assert!(pollster::block_on(d.pop_error_scope()).is_none());
}
