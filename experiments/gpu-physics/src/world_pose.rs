//! Publish global poses without rounding the internal physics/host mirror.
use crate::types::{BodyGpu, BodyStateGpu};
use wgpu::util::DeviceExt;

pub(super) struct WorldPose {
    pub buffer: wgpu::Buffer,
    origin: [f32; 3],
    pipeline: wgpu::ComputePipeline,
    group: wgpu::BindGroup,
    slots: u32,
}

impl WorldPose {
    pub fn new(device: &wgpu::Device, source: &wgpu::Buffer, origin: [f32; 3]) -> Self {
        assert_eq!(std::mem::size_of::<BodyStateGpu>(), 28 * 4);
        let buffer = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("world-space-render-poses"), size: source.size(),
            usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC | wgpu::BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });
        let uniform = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("physics-coordinate-origin"),
            contents: bytemuck::cast_slice(&[origin[0], origin[1], origin[2], 0.0]),
            usage: wgpu::BufferUsages::UNIFORM,
        });
        let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("world-space-render-poses"),
            source: wgpu::ShaderSource::Wgsl(include_str!("../shaders/world_pose.wgsl").into()),
        });
        let pipeline = device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
            label: Some("world-space-render-poses"), layout: None, module: &shader,
            entry_point: Some("export_poses"), compilation_options: Default::default(), cache: None,
        });
        let group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: Some("world-space-render-poses"), layout: &pipeline.get_bind_group_layout(0),
            entries: &[
                wgpu::BindGroupEntry { binding: 0, resource: source.as_entire_binding() },
                wgpu::BindGroupEntry { binding: 1, resource: buffer.as_entire_binding() },
                wgpu::BindGroupEntry { binding: 2, resource: uniform.as_entire_binding() },
            ],
        });
        Self { buffer, origin, pipeline, group, slots: (source.size() / 112) as u32 }
    }

    pub fn encode(&self, encoder: &mut wgpu::CommandEncoder) {
        let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
            label: Some("publish-world-space-poses"), timestamp_writes: None,
        });
        pass.set_pipeline(&self.pipeline);
        pass.set_bind_group(0, &self.group, &[]);
        pass.dispatch_workgroups(self.slots.div_ceil(64), 1, 1);
    }

    fn state(&self, body: &BodyGpu) -> BodyStateGpu {
        let mut state = BodyStateGpu::from_body(body);
        for i in 0..3 {
            if self.origin[i] != 0.0 {
                state.pos[i] += self.origin[i];
                if state.origin_valid != 0 { state.origin[i] += self.origin[i]; }
            }
        }
        state
    }

    pub fn write_all(&self, queue: &wgpu::Queue, bodies: &[BodyGpu]) {
        if bodies.is_empty() { return; }
        let states: Vec<_> = bodies.iter().map(|body| self.state(body)).collect();
        queue.write_buffer(&self.buffer, 0, bytemuck::cast_slice(&states));
    }

    pub fn write_at(&self, queue: &wgpu::Queue, slot: u32, body: &BodyGpu) {
        let state = self.state(body);
        queue.write_buffer(&self.buffer, u64::from(slot) * 112, bytemuck::bytes_of(&state));
    }
}
