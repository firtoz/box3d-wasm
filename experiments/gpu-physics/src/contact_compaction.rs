//! Deterministic GPU child placement; roots and physical generation counters remain fixed.
use crate::types::{ContactHotGpu, ContactPersistentGpu, ContactPreparedGpu, SimParams};
use std::mem::{size_of,offset_of};
use wgpu::util::DeviceExt;

pub(crate) struct ContactCompaction {
    group:wgpu::BindGroup,
    pipelines:Vec<wgpu::ComputePipeline>,
    groups:u32,
    work:wgpu::Buffer,
}
impl ContactCompaction {
    pub(crate) fn new(device:&wgpu::Device, capacity:u32, hot:&wgpu::Buffer,
        persistent:&wgpu::Buffer, prepared:&wgpu::Buffer, query:&wgpu::Buffer, atom:&wgpu::Buffer, parameters:&wgpu::Buffer)->Self {
        let groups=capacity.div_ceil(256);
        assert!(capacity>0 && groups<=device.limits().max_compute_workgroups_per_dimension);
        let strides=[size_of::<ContactHotGpu>(),size_of::<ContactPersistentGpu>(),size_of::<ContactPreparedGpu>()].map(|n|n as u32/4);
        let source=format!("const HOT:u32={}u;const PERSISTENT:u32={}u;const PREPARED:u32={}u;const LINK:u32={}u;const LIFE:u32={}u;const PAIR:u32={}u;const STEP_WORD:u32={}u;\n{}",
            strides[0],strides[1],strides[2],offset_of!(ContactHotGpu,manifold_link)/4,
            offset_of!(ContactPersistentGpu,lifecycle)/4,offset_of!(ContactPersistentGpu,pair)/4,offset_of!(SimParams,physics_step)/4,
            include_str!("../shaders/contact_compaction.wgsl"));
        let shader=device.create_shader_module(wgpu::ShaderModuleDescriptor{label:Some("contact-compaction"),source:wgpu::ShaderSource::Wgsl(source.into())});
        let entries:Vec<_>=(0..9).map(|binding|wgpu::BindGroupLayoutEntry{binding,visibility:wgpu::ShaderStages::COMPUTE,
            ty:wgpu::BindingType::Buffer{ty:if binding==0 || binding==8{wgpu::BufferBindingType::Uniform}else{wgpu::BufferBindingType::Storage{read_only:false}},has_dynamic_offset:false,min_binding_size:None},count:None}).collect();
        let layout=device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor{label:Some("contact-compaction"),entries:&entries});
        let pipeline_layout=device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor{label:Some("contact-compaction"),bind_group_layouts:&[&layout],push_constant_ranges:&[]});
        let uniform=device.create_buffer_init(&wgpu::util::BufferInitDescriptor{label:Some("contact-compaction-config"),contents:bytemuck::cast_slice(&[capacity,groups,0,0]),usage:wgpu::BufferUsages::UNIFORM});
        let buffer=|label,words:u64|device.create_buffer(&wgpu::BufferDescriptor{label:Some(label),size:4*words,usage:wgpu::BufferUsages::STORAGE|wgpu::BufferUsages::COPY_DST,mapped_at_creation:false});
        let stage_words=u64::from(capacity)*u64::from(strides.iter().sum::<u32>());
        assert!(stage_words*4 <= u64::from(device.limits().max_storage_buffer_binding_size),
            "mesh child snapshot exceeds the device storage binding limit");
        let staged=buffer("contact-compaction-snapshot",stage_words);
        let work=buffer("contact-compaction-work",4+5*u64::from(capacity)+6*u64::from(groups));
        let buffers=[&uniform,hot,persistent,prepared,&staged,&work,query,atom,parameters];
        let bindings:Vec<_>=buffers.iter().enumerate().map(|(binding,buffer)|wgpu::BindGroupEntry{binding:binding as u32,resource:if binding==8{wgpu::BindingResource::Buffer(wgpu::BufferBinding{buffer,offset:0,size:std::num::NonZeroU64::new(size_of::<SimParams>() as u64)})}else{buffer.as_entire_binding()}}).collect();
        let group=device.create_bind_group(&wgpu::BindGroupDescriptor{label:Some("contact-compaction"),layout:&layout,entries:&bindings});
        let pipelines=["snapshot","block_bases","map_children","publish"].iter().map(|entry|device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor{
            label:Some(entry),layout:Some(&pipeline_layout),module:&shader,entry_point:Some(entry),compilation_options:Default::default(),cache:None})).collect();
        Self{group,pipelines,groups,work}
    }
    pub(crate) fn encode(&self,enc:&mut wgpu::CommandEncoder){
        enc.clear_buffer(&self.work,0,Some(16));
        for (i,pipeline) in self.pipelines.iter().enumerate(){
            let mut pass=enc.begin_compute_pass(&wgpu::ComputePassDescriptor{label:Some("contact-compaction"),timestamp_writes:None});
            pass.set_bind_group(0,&self.group,&[]);pass.set_pipeline(pipeline);
            pass.dispatch_workgroups(if i==1{1}else{self.groups},1,1);
        }
    }
}
