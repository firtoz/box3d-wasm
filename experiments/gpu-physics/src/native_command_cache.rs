//! Opt-in Vulkan replay for broadphase, contacts, graph construction and solver tails.
//! Requires the fingerprint-checked backend prepared by scripts/build-native-cache.sh.
use ash::vk;
use wgpu::hal::api::Vulkan;

pub(crate) enum Command<'a> {
    Dispatch(&'a wgpu::ComputePipeline, u32),
    // Opt-in: the pipeline must flatten indices using dispatch.wgsl helpers.
    DispatchLinear(&'a wgpu::ComputePipeline, u32),
    Indirect(&'a wgpu::ComputePipeline, u64),
    CopyArgs { source: u64, destination: u64, bytes: u64 },
    CopyBuffer { buffer: &'a wgpu::Buffer, source: u64, destination: u64, bytes: u64 },
}

pub(crate) struct RadixCache(std::sync::Arc<RadixCacheData>);
impl std::ops::Deref for RadixCache {type Target=RadixCacheData; fn deref(&self)->&Self::Target{&self.0}}
pub(crate) struct RadixCacheData {
    pub key: [u32;3],
    _scratch: Option<wgpu::Buffer>,
    device: wgpu::Device,
    raw: ash::Device,
    pool: vk::CommandPool,
    command: vk::CommandBuffer,
    pub group: wgpu::BindGroup,
    _layout: wgpu::PipelineLayout,
    _pipelines: Vec<wgpu::ComputePipeline>,
    _indirect: wgpu::Buffer,
    _copy_sources: Vec<wgpu::Buffer>,
}
impl RadixCache {
    pub fn new(device:&wgpu::Device, group:&wgpu::BindGroup, layout:&wgpu::PipelineLayout,
        indirect:&wgpu::Buffer, dynamic_offset:u32, indirect_offset:u64,
        commands:&[(&wgpu::ComputePipeline,bool)]) -> Self {
        let commands: Vec<_> = commands.iter().map(|&(p,i)| if i {
            Command::Indirect(p,indirect_offset)
        } else {Command::Dispatch(p,1)}).collect();
        Self::record(device,group,layout,indirect,None,dynamic_offset,[1,0,0],&commands)
    }
    pub fn record(device:&wgpu::Device, group:&wgpu::BindGroup, layout:&wgpu::PipelineLayout,
        indirect:&wgpu::Buffer, scratch:Option<&wgpu::Buffer>, dynamic_offset:u32,
        key:[u32;3], commands:&[Command<'_>]) -> Self { unsafe {
        // All handles are borrowed; clones below retain every referenced resource.
        let hal=device.as_hal::<Vulkan>().expect("Vulkan required");
        let raw=hal.raw_device().clone();
        let pool=raw.create_command_pool(&vk::CommandPoolCreateInfo::default()
            .queue_family_index(hal.queue_family_index()),None).unwrap();
        let command=raw.allocate_command_buffers(&vk::CommandBufferAllocateInfo::default()
            .command_pool(pool).level(vk::CommandBufferLevel::PRIMARY).command_buffer_count(1)).unwrap()[0];
        let inheritance=vk::CommandBufferInheritanceInfo::default();
        raw.begin_command_buffer(command,&vk::CommandBufferBeginInfo::default()
            .flags(vk::CommandBufferUsageFlags::SIMULTANEOUS_USE).inheritance_info(&inheritance)).unwrap();
        let native_layout=layout.as_hal::<Vulkan>().unwrap().cache_raw_handle();
        let set=group.as_hal::<Vulkan,_,_>(|g|g.unwrap().cache_raw_handle());
        let args=indirect.as_hal::<Vulkan>().unwrap().cache_raw_handle();
        raw.cmd_bind_descriptor_sets(command,vk::PipelineBindPoint::COMPUTE,native_layout,0,&[set],&[dynamic_offset]);
        let mut copy_sources=Vec::new();
        let mut pipelines=Vec::new();
        for operation in commands {
            match *operation {
                Command::Dispatch(pipeline,groups) => {
                    assert!(groups>0 && groups<=device.limits().max_compute_workgroups_per_dimension);
                    raw.cmd_bind_pipeline(command, vk::PipelineBindPoint::COMPUTE, pipeline.as_hal::<Vulkan>().unwrap().cache_raw_handle());
                    raw.cmd_dispatch(command,groups,1,1);
                    pipelines.push(pipeline.clone());
                }
                Command::DispatchLinear(pipeline,groups) => {
                    let [x,y,z] = crate::dispatch::linear_dispatch_groups(groups);
                    assert!(x <= device.limits().max_compute_workgroups_per_dimension
                        && y <= device.limits().max_compute_workgroups_per_dimension);
                    raw.cmd_bind_pipeline(command, vk::PipelineBindPoint::COMPUTE, pipeline.as_hal::<Vulkan>().unwrap().cache_raw_handle());
                    raw.cmd_dispatch(command,x,y,z);
                    pipelines.push(pipeline.clone());
                }
                Command::Indirect(pipeline,offset) => {
                    assert!(offset%4==0 && offset+12<=indirect.size());
                    raw.cmd_bind_pipeline(command,vk::PipelineBindPoint::COMPUTE,pipeline.as_hal::<Vulkan>().unwrap().cache_raw_handle());
                    raw.cmd_dispatch_indirect(command,args,offset);
                    pipelines.push(pipeline.clone());
                }
                Command::CopyArgs{..} | Command::CopyBuffer{..} => {
                    let (copy_source,offset,destination,bytes)=match *operation {
                        Command::CopyArgs{source,destination,bytes}=>(scratch.unwrap(),source,destination,bytes),
                        Command::CopyBuffer{buffer,source,destination,bytes}=>(buffer,source,destination,bytes),
                        _=>unreachable!(),
                    };
                    copy_sources.push(copy_source.clone());
                    let source=copy_source.as_hal::<Vulkan>().unwrap().cache_raw_handle();
                    assert!(offset%4==0 && destination%4==0 && bytes%4==0);
                    assert!(offset+bytes<=copy_source.size() && destination+bytes<=indirect.size());
                    // Scratch producer -> transfer read; prior indirect reads -> transfer write.
                    // Include wgpu's initial transfer clear of the indirect buffer.
                    raw.cmd_pipeline_barrier(command,
                        vk::PipelineStageFlags::COMPUTE_SHADER|vk::PipelineStageFlags::DRAW_INDIRECT|vk::PipelineStageFlags::TRANSFER,
                        vk::PipelineStageFlags::TRANSFER,vk::DependencyFlags::empty(),
                        &[vk::MemoryBarrier::default()
                            .src_access_mask(vk::AccessFlags::SHADER_WRITE|vk::AccessFlags::INDIRECT_COMMAND_READ|vk::AccessFlags::TRANSFER_WRITE)
                            .dst_access_mask(vk::AccessFlags::TRANSFER_READ|vk::AccessFlags::TRANSFER_WRITE)],&[],&[]);
                    raw.cmd_copy_buffer(command,source,args,&[vk::BufferCopy{src_offset:offset,dst_offset:destination,size:bytes}]);
                    // Copy destination -> indirect arguments; transfer source -> subsequent shader writes.
                    raw.cmd_pipeline_barrier(command,vk::PipelineStageFlags::TRANSFER,
                        vk::PipelineStageFlags::DRAW_INDIRECT|vk::PipelineStageFlags::COMPUTE_SHADER,
                        vk::DependencyFlags::empty(),&[vk::MemoryBarrier::default()
                            .src_access_mask(vk::AccessFlags::TRANSFER_WRITE|vk::AccessFlags::TRANSFER_READ)
                            .dst_access_mask(vk::AccessFlags::INDIRECT_COMMAND_READ|vk::AccessFlags::SHADER_READ|vk::AccessFlags::SHADER_WRITE)],&[],&[]);
                    continue;
                }
            }
            raw.cmd_pipeline_barrier(command,vk::PipelineStageFlags::COMPUTE_SHADER,
                vk::PipelineStageFlags::COMPUTE_SHADER,vk::DependencyFlags::empty(),
                &[vk::MemoryBarrier::default().src_access_mask(vk::AccessFlags::SHADER_WRITE)
                    .dst_access_mask(vk::AccessFlags::SHADER_READ|vk::AccessFlags::SHADER_WRITE)],&[],&[]);
        }
        raw.end_command_buffer(command).unwrap();
        Self(std::sync::Arc::new(RadixCacheData{device:device.clone(),raw,pool,command,group:group.clone(),_layout:layout.clone(),
            _pipelines:pipelines,_indirect:indirect.clone(),_scratch:scratch.cloned(),_copy_sources:copy_sources,key}))
    }}
    pub fn encode(&self,encoder:&mut wgpu::CommandEncoder) {unsafe {
        // Caller transitions all resources through wgpu before this native block.
        // Queue ownership remains wgpu's; no independent queue submissions occur.
        encoder.as_hal_mut::<Vulkan,_,_>(|e| {
            e.unwrap().cache_insert_primary(self.command,self.0.clone()).expect("insert owned primary");
        });
    }}
}
impl Drop for RadixCacheData {
    fn drop(&mut self) {
        // Invalidation/teardown only, never a per-step wait.
        // HAL submission retains this owner until completion (or unsubmitted discard).
        unsafe{self.raw.destroy_command_pool(self.pool,None);}
    }
}

#[cfg(test)]
mod replay_lifetime_tests {
    use super::*;
    #[test]
    fn primary_replay_upload_order_and_owner_lifetime() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).unwrap();
        let d=&gpu.device; let q=&gpu.queue;
        d.push_error_scope(wgpu::ErrorFilter::Validation);
        let uniform=d.create_buffer(&wgpu::BufferDescriptor{label:None,size:16,usage:wgpu::BufferUsages::UNIFORM|wgpu::BufferUsages::COPY_DST,mapped_at_creation:false});
        let output=d.create_buffer(&wgpu::BufferDescriptor{label:None,size:16,usage:wgpu::BufferUsages::STORAGE|wgpu::BufferUsages::COPY_SRC|wgpu::BufferUsages::COPY_DST,mapped_at_creation:false});
        let args=d.create_buffer(&wgpu::BufferDescriptor{label:None,size:16,usage:wgpu::BufferUsages::INDIRECT,mapped_at_creation:false});
        let layout=d.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor{label:None,entries:&[
            wgpu::BindGroupLayoutEntry{binding:0,visibility:wgpu::ShaderStages::COMPUTE,ty:wgpu::BindingType::Buffer{ty:wgpu::BufferBindingType::Uniform,has_dynamic_offset:true,min_binding_size:None},count:None},
            wgpu::BindGroupLayoutEntry{binding:1,visibility:wgpu::ShaderStages::COMPUTE,ty:wgpu::BindingType::Buffer{ty:wgpu::BufferBindingType::Storage{read_only:false},has_dynamic_offset:false,min_binding_size:None},count:None},
        ]});
        let group=d.create_bind_group(&wgpu::BindGroupDescriptor{label:None,layout:&layout,entries:&[
            wgpu::BindGroupEntry{binding:0,resource:uniform.as_entire_binding()},wgpu::BindGroupEntry{binding:1,resource:output.as_entire_binding()},
        ]});
        let pl=d.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor{label:None,bind_group_layouts:&[&layout],push_constant_ranges:&[]});
        let shader=d.create_shader_module(wgpu::ShaderModuleDescriptor{label:None,source:wgpu::ShaderSource::Wgsl("@group(0) @binding(0) var<uniform> amount:vec4<u32>; @group(0) @binding(1) var<storage,read_write> result:array<u32>; @compute @workgroup_size(1) fn main(){result[0]+=amount.x;}".into())});
        let pipeline=d.create_compute_pipeline(&wgpu::ComputePipelineDescriptor{label:None,layout:Some(&pl),module:&shader,entry_point:Some("main"),compilation_options:Default::default(),cache:None});
        let cache=RadixCache::record(d,&group,&pl,&args,None,0,[1,0,0],&[Command::Dispatch(&pipeline,1)]);
        let weak=std::sync::Arc::downgrade(&cache.0);
        let encode=|cache:&RadixCache| {
            let mut enc=d.create_command_encoder(&Default::default());
            enc.transition_resources([
                wgpu::BufferTransition{buffer:&uniform,state:wgpu::BufferUses::UNIFORM},
                wgpu::BufferTransition{buffer:&output,state:wgpu::BufferUses::STORAGE_READ_WRITE},
            ].into_iter(),std::iter::empty());
            cache.encode(&mut enc);cache.encode(&mut enc);enc
        };
        q.write_buffer(&output,0,&[0u8;16]);
        // Discarded encoders must not execute, and must not release the cache's pool.
        drop(encode(&cache));
        // Queue writes must precede both replays in each tracked submission.
        for value in 1u32..=8 {q.write_buffer(&uniform,0,bytemuck::cast_slice(&[value,0,0,0]));q.submit(Some(encode(&cache).finish()));}
        q.write_buffer(&uniform,0,bytemuck::cast_slice(&[10u32,0,0,0]));
        let last=encode(&cache).finish();
        drop(cache);
        assert!(weak.upgrade().is_some(),"unsubmitted buffer must retain primary owner");
        q.submit(Some(last));
        let staging=d.create_buffer(&wgpu::BufferDescriptor{label:None,size:16,usage:wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::MAP_READ,mapped_at_creation:false});
        let mut enc=d.create_command_encoder(&Default::default());enc.copy_buffer_to_buffer(&output,0,&staging,0,16);q.submit(Some(enc.finish()));
        let(tx,rx)=std::sync::mpsc::channel();staging.slice(..).map_async(wgpu::MapMode::Read,move|v|tx.send(v).unwrap());
        d.poll(wgpu::PollType::Wait).unwrap();rx.recv().unwrap().unwrap();
        assert!(pollster::block_on(d.pop_error_scope()).is_none());
        assert_eq!(bytemuck::cast_slice::<u8,u32>(&staging.slice(..).get_mapped_range())[0],92);
        staging.unmap();
    }
}
