//! GPU endpoint ownership for the forthcoming continuous collision pass.
//! This module does not replace CCD: candidates still require exact swept tests.
//! No default step uses it until the complete correction path is validated.
use wgpu::util::DeviceExt;
use crate::types::BodyStateGpu;

pub(crate) struct CcdMotion {
    pub start: wgpu::Buffer,
    pub candidates: wgpu::Buffer,
    binding: wgpu::BindGroup,
    pipeline: wgpu::ComputePipeline,
    count: u32,
}
impl CcdMotion {
    /// Recreate after slot-span/extent changes. Slots include disabled deletion holes.
    pub fn new(device: &wgpu::Device, bodies: &wgpu::Buffer, extents: &[[f32; 2]]) -> Self {
        assert!(!extents.is_empty());
        assert!(extents.len() <= crate::types::MAX_BODY_SLOTS as usize);
        let count = extents.len() as u32;
        let bytes = count as u64 * std::mem::size_of::<BodyStateGpu>() as u64;
        assert!(bodies.size() >= bytes);
        let start = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("ccd-start"), size: bytes,
            usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::COPY_SRC,
            mapped_at_creation: false,
        });
        let candidates = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("ccd-candidates"), size: count as u64 * 4,
            usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC,
            mapped_at_creation: false,
        });
        let extents = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("ccd-extents"), contents: bytemuck::cast_slice(extents),
            usage: wgpu::BufferUsages::STORAGE,
        });
        let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("ccd-motion"), source: wgpu::ShaderSource::Wgsl(include_str!("../shaders/physics/ccd_motion.wgsl").into()),
        });
        let pipeline = device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
            label: Some("ccd-motion"), layout: None, module: &shader, entry_point: Some("classify"),
            compilation_options: Default::default(), cache: None,
        });
        let binding = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: Some("ccd-motion"), layout: &pipeline.get_bind_group_layout(0),
            entries: &[
                wgpu::BindGroupEntry { binding: 0, resource: start.as_entire_binding() },
                wgpu::BindGroupEntry { binding: 1, resource: bodies.as_entire_binding() },
                wgpu::BindGroupEntry { binding: 2, resource: extents.as_entire_binding() },
                wgpu::BindGroupEntry { binding: 3, resource: candidates.as_entire_binding() },
            ],
        });
        Self { start, candidates, binding, pipeline, count }
    }
    /// Encode before any integration; queue ordering includes prior corrections/mutations.
    pub fn capture(&self, encoder: &mut wgpu::CommandEncoder, bodies: &wgpu::Buffer) {
        encoder.copy_buffer_to_buffer(bodies, 0, &self.start, 0, self.start.size());
    }
    /// Encode after apply-deltas; one owner per candidate, no CPU polling or mapping.
    pub fn classify(&self, encoder: &mut wgpu::CommandEncoder) {
        let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
            label: Some("ccd-motion"), timestamp_writes: None,
        });
        pass.set_pipeline(&self.pipeline);
        pass.set_bind_group(0, &self.binding, &[]);
        pass.dispatch_workgroups(self.count.div_ceil(64), 1, 1);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use bytemuck::Zeroable;
    use crate::types::{FLAG_BULLET, FLAG_DISABLED, FLAG_KINEMATIC, FLAG_SLEEP, FLAG_STATIC};

    #[test]
    fn gpu_ccd_motion_uses_ordered_endpoints_and_slot_flags() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).unwrap();
        let device = &gpu.device;
        let queue = &gpu.queue;
        // Cross a workgroup boundary and retain a disabled hole.
        let mut initial = vec![BodyStateGpu::zeroed(); 65];
        for body in &mut initial { body.rot = [0.0, 0.0, 0.0, 1.0]; }
        let bodies = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("motion-test"), contents: bytemuck::cast_slice(&initial),
            usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::COPY_SRC,
        });
        let motion = CcdMotion::new(device, &bodies, &vec![[1.0, 2.0]; 65]);
        let mut finish = initial.clone();
        finish[0].pos[0] = 0.5; // Strict boundary: not fast.
        finish[1].pos[0] = 0.6;
        finish[2].rot = glam::Quat::from_rotation_y(0.4).to_array(); // Rotation alone.
        finish[3].pos[0] = 5.0; finish[3].flags = FLAG_SLEEP;
        finish[4].pos[0] = 5.0; finish[4].flags = FLAG_STATIC;
        finish[5].pos[0] = 5.0; finish[5].flags = FLAG_KINEMATIC;
        finish[6].pos[0] = 5.0; finish[6].flags = FLAG_DISABLED;
        finish[64].pos[0] = 0.6; finish[64].flags = FLAG_BULLET;
        let readback = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("motion-test-results"), size: 260,
            usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });
        for round in 0..2 {
            let mut encoder = device.create_command_encoder(&Default::default());
            motion.capture(&mut encoder, &bodies);
            queue.submit([encoder.finish()]);
            // Ordered mutation after capture. No host wait between these submissions.
            queue.write_buffer(&bodies, 0, bytemuck::cast_slice(&finish));
            let mut encoder = device.create_command_encoder(&Default::default());
            motion.classify(&mut encoder);
            encoder.copy_buffer_to_buffer(&motion.candidates, 0, &readback, 0, 260);
            queue.submit([encoder.finish()]);
            let (tx, rx) = std::sync::mpsc::channel();
            readback.slice(..).map_async(wgpu::MapMode::Read, move |r| { tx.send(r).unwrap(); });
            device.poll(wgpu::PollType::wait()).unwrap();
            rx.recv().unwrap().unwrap();
            let data = readback.slice(..).get_mapped_range();
            let actual: &[u32] = bytemuck::cast_slice(&data);
            let mut expected = vec![0u32; 65];
            if round == 0 { expected[1] = 1; expected[2] = 1; expected[64] = 2; }
            assert_eq!(actual, expected, "round {round}: second capture must use latest GPU state");
            drop(data);
            readback.unmap();
        }
    }
}

// Validation dispatch only. Production CCD will source transforms from GPU
// state and enumerate/filter candidates there, not upload these per-step jobs.
#[cfg(test)]
#[repr(C)]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
pub(crate) struct SweepProxy { pub first: u32, pub count: u32, pub radius: f32, pub unused: u32 }
#[cfg(test)]
#[repr(C)]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
pub(crate) struct SweepTransform { pub p: [f32; 4], pub q: [f32; 4] }
#[cfg(test)]
impl From<crate::api::WorldTransform> for SweepTransform {
    fn from(x: crate::api::WorldTransform) -> Self { Self { p: [x.p[0],x.p[1],x.p[2],0.0], q:x.q } }
}
#[cfg(test)]
#[repr(C)]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
pub(crate) struct SweepJob {
    pub a: SweepProxy, pub b: SweepProxy,
    pub a0: SweepTransform, pub a1: SweepTransform,
    pub b0: SweepTransform, pub b1: SweepTransform,
    pub max_fraction: f32, pub iterations: u32, pub unused: [u32; 2],
}
#[cfg(test)]
#[repr(C)]
#[derive(Clone, Copy, Debug, bytemuck::Pod, bytemuck::Zeroable)]
pub(crate) struct SweepResult {
    pub point: [f32; 4], pub normal: [f32; 4], pub fraction: f32, pub status: u32, pub unused: [u32; 2],
}
#[cfg(test)]
pub(crate) fn test_sweeps(gpu: &crate::sim::GpuDevice, points: &[[f32; 4]], jobs: &[SweepJob]) -> Vec<SweepResult> {
    let device=&gpu.device; let queue=&gpu.queue;
    device.push_error_scope(wgpu::ErrorFilter::Validation);
    let source = format!("{}\n{}", include_str!("../shaders/physics/ccd_convex.wgsl"), r#"
        struct Job {
            a:CcdProxy, b:CcdProxy, a0:CcdTransform, a1:CcdTransform, b0:CcdTransform, b1:CcdTransform,
            max_fraction:f32, iterations:u32, unused:vec2<u32>
        }
        @group(0) @binding(0) var<storage,read> ccd_points:array<vec4<f32>>;
        @group(0) @binding(1) var<storage,read> jobs:array<Job>;
        @group(0) @binding(2) var<storage,read_write> results:array<CcdHit>;
        @compute @workgroup_size(64) fn test_sweep(@builtin(global_invocation_id) id:vec3<u32>) {
            if (id.x>=arrayLength(&jobs)) { return; }
            let j=jobs[id.x];
            results[id.x]=ccd_sweep(j.a,j.a0,j.a1,j.b,j.b0,j.b1,j.max_fraction,j.iterations);
        }
    "#);
    let shader=device.create_shader_module(wgpu::ShaderModuleDescriptor{label:Some("ccd-sweep-test"),source:wgpu::ShaderSource::Wgsl(source.into())});
    let pipeline=device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor{label:Some("ccd-sweep-test"),layout:None,module:&shader,entry_point:Some("test_sweep"),compilation_options:Default::default(),cache:None});
    let input=|label,data:&[u8]| device.create_buffer_init(&wgpu::util::BufferInitDescriptor{label:Some(label),contents:data,usage:wgpu::BufferUsages::STORAGE});
    let points=input("ccd-points",bytemuck::cast_slice(points));
    let jobs_buffer=input("ccd-jobs",bytemuck::cast_slice(jobs));
    let size=(jobs.len()*std::mem::size_of::<SweepResult>()) as u64;
    let output=device.create_buffer(&wgpu::BufferDescriptor{label:Some("ccd-results"),size,usage:wgpu::BufferUsages::STORAGE|wgpu::BufferUsages::COPY_SRC,mapped_at_creation:false});
    let staging=device.create_buffer(&wgpu::BufferDescriptor{label:Some("ccd-test-readback"),size,usage:wgpu::BufferUsages::MAP_READ|wgpu::BufferUsages::COPY_DST,mapped_at_creation:false});
    let binding=device.create_bind_group(&wgpu::BindGroupDescriptor{label:None,layout:&pipeline.get_bind_group_layout(0),entries:&[
        wgpu::BindGroupEntry{binding:0,resource:points.as_entire_binding()},wgpu::BindGroupEntry{binding:1,resource:jobs_buffer.as_entire_binding()},wgpu::BindGroupEntry{binding:2,resource:output.as_entire_binding()},
    ]});
    let mut encoder=device.create_command_encoder(&Default::default());
    { let mut pass=encoder.begin_compute_pass(&wgpu::ComputePassDescriptor{label:None,timestamp_writes:None});
      pass.set_pipeline(&pipeline);pass.set_bind_group(0,&binding,&[]);pass.dispatch_workgroups((jobs.len() as u32).div_ceil(64),1,1); }
    encoder.copy_buffer_to_buffer(&output,0,&staging,0,size);
    queue.submit([encoder.finish()]);
    let (tx,rx)=std::sync::mpsc::channel();
    staging.slice(..).map_async(wgpu::MapMode::Read,move |r|{tx.send(r).unwrap();});
    device.poll(wgpu::PollType::wait()).unwrap();rx.recv().unwrap().unwrap();
    assert!(pollster::block_on(device.pop_error_scope()).is_none(), "GPU sweep validation failed");
    let mapped=staging.slice(..).get_mapped_range();
    let results=bytemuck::cast_slice(&mapped).to_vec();drop(mapped);staging.unmap();results
}

#[repr(C)]
#[derive(Clone,Copy,bytemuck::Pod,bytemuck::Zeroable)]
pub(crate) struct ConvexBody { pub first:u32, pub count:u32, pub min_extent:f32, pub max_extent:f32, pub center:[f32;4] }
#[repr(C)]
#[derive(Clone,Copy,bytemuck::Pod,bytemuck::Zeroable)]
pub(crate) struct ConvexShape {
    pub first:u32, pub count:u32, pub radius:f32, pub unused:u32,
    pub body:u32, pub group:i32, pub category:[u32;2], pub mask:[u32;2], pub sweep_radius:f32, pub unused2:u32,
}
pub(crate) struct ConvexScene {
    pub bodies:Vec<ConvexBody>, pub shapes:Vec<ConvexShape>, pub points:Vec<[f32;4]>,
    pub indices:Vec<u32>, pub targets_first:u32, pub targets_count:u32, pub joints_first:u32, pub joints_count:u32,
}
#[cfg(feature="replay-diagnostics")]
pub(crate) struct DiagnosticCcdBuffers {
    pub buffers:[wgpu::Buffer;5],
    pub counts:[u32;4],
}
#[cfg(feature="replay-diagnostics")]
pub(crate) struct DiagnosticCcdScene {
    pub points:Vec<[f32;4]>, pub shapes:Vec<ConvexShape>, pub bodies:Vec<ConvexBody>,
    pub indices:Vec<u32>, pub config:Vec<u32>, pub start:Vec<BodyStateGpu>,
}
pub(crate) struct ConvexCcd {
    #[cfg(feature="replay-diagnostics")]
    pub diagnostic:DiagnosticCcdBuffers,
    pub(crate) start:wgpu::Buffer, binding:wgpu::BindGroup, pipeline:wgpu::ComputePipeline, count:u32,
}
impl ConvexCcd {
    pub fn new(device:&wgpu::Device,bodies:&wgpu::Buffer,scene:&ConvexScene) -> Self {
        let count=scene.bodies.len() as u32;
        assert!(count>0 && count<=crate::types::MAX_BODY_SLOTS);
        let start=device.create_buffer(&wgpu::BufferDescriptor{label:Some("ccd-start-state"),size:count as u64*std::mem::size_of::<BodyStateGpu>() as u64,
            usage:wgpu::BufferUsages::STORAGE|wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::COPY_SRC,mapped_at_creation:false});
        let diagnostic_usage=if cfg!(feature="replay-diagnostics") {wgpu::BufferUsages::COPY_SRC} else {wgpu::BufferUsages::empty()}
            | if cfg!(all(test,feature="replay-diagnostics")) {wgpu::BufferUsages::COPY_DST} else {wgpu::BufferUsages::empty()};
        let input=|label,data:&[u8]| device.create_buffer_init(&wgpu::util::BufferInitDescriptor{
            label:Some(label),contents:if data.is_empty(){&[0u8;48]}else{data},usage:wgpu::BufferUsages::STORAGE|diagnostic_usage});
        let points=input("ccd-convex-points",bytemuck::cast_slice(&scene.points));
        let shapes=input("ccd-convex-shapes",bytemuck::cast_slice(&scene.shapes));
        let metadata=input("ccd-convex-bodies",bytemuck::cast_slice(&scene.bodies));
        let indices=input("ccd-convex-indices",bytemuck::cast_slice(&scene.indices));
        // vec3 alignment in WGSL puts the final unused member at byte 32.
        let config=[count,scene.targets_first,scene.targets_count,scene.joints_first,scene.joints_count,0,0,0,0,0,0,0];
        let config=device.create_buffer_init(&wgpu::util::BufferInitDescriptor{label:Some("ccd-config"),
            contents:bytemuck::cast_slice(&config),usage:wgpu::BufferUsages::UNIFORM|diagnostic_usage});
        let source=format!("{}\n{}",include_str!("../shaders/physics/ccd_convex.wgsl"),include_str!("../shaders/physics/ccd_world.wgsl"));
        let shader=device.create_shader_module(wgpu::ShaderModuleDescriptor{label:Some("world-convex-ccd"),source:wgpu::ShaderSource::Wgsl(source.into())});
        let pipeline=device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor{label:Some("world-convex-ccd"),layout:None,module:&shader,entry_point:Some("ccd_correct"),compilation_options:Default::default(),cache:None});
        let buffers=[&start,bodies,&points,&shapes,&metadata,&indices,&config];
        let entries:Vec<_>=buffers.iter().enumerate().map(|(i,b)|wgpu::BindGroupEntry{binding:i as u32,resource:b.as_entire_binding()}).collect();
        let binding=device.create_bind_group(&wgpu::BindGroupDescriptor{label:Some("world-convex-ccd"),layout:&pipeline.get_bind_group_layout(0),entries:&entries});
        Self{start,binding,pipeline,count,
            #[cfg(feature="replay-diagnostics")]
            diagnostic:DiagnosticCcdBuffers{buffers:[points,shapes,metadata,indices,config],
                counts:[scene.points.len() as u32,scene.shapes.len() as u32,scene.bodies.len() as u32,scene.indices.len() as u32]}}

    }
    pub fn capture(&self,encoder:&mut wgpu::CommandEncoder,bodies:&wgpu::Buffer) {
        encoder.copy_buffer_to_buffer(bodies,0,&self.start,0,self.start.size());
    }
    pub fn correct(&self,encoder:&mut wgpu::CommandEncoder) {
        let mut pass=encoder.begin_compute_pass(&wgpu::ComputePassDescriptor{label:Some("convex-ccd"),timestamp_writes:None});
        pass.set_pipeline(&self.pipeline);pass.set_bind_group(0,&self.binding,&[]);pass.dispatch_workgroups(self.count.div_ceil(64),1,1);
    }
}
