//! Opt-in Vulkan replay for broadphase, contacts, graph construction and solver tails.
//! Requires the fingerprint-checked backend prepared by scripts/build-native-cache.sh.
use ash::vk;
use wgpu::hal::api::Vulkan;

pub(crate) enum Command<'a> {
    Dispatch(&'a wgpu::ComputePipeline, u32),
    Indirect(&'a wgpu::ComputePipeline, u64),
    CopyArgs { source: u64, destination: u64, bytes: u64 },
    CopyBuffer { buffer: &'a wgpu::Buffer, source: u64, destination: u64, bytes: u64 },
}

pub(crate) struct RadixCache {
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
            .command_pool(pool).level(vk::CommandBufferLevel::SECONDARY).command_buffer_count(1)).unwrap()[0];
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
        Self{device:device.clone(),raw,pool,command,group:group.clone(),_layout:layout.clone(),
            _pipelines:pipelines,_indirect:indirect.clone(),_scratch:scratch.cloned(),_copy_sources:copy_sources,key}
    }}
    pub fn encode(&self,encoder:&mut wgpu::CommandEncoder) {unsafe {
        // Caller transitions all resources through wgpu before this native block.
        // Queue ownership remains wgpu's; no independent queue submissions occur.
        encoder.as_hal_mut::<Vulkan,_,_>(|e| {
            self.raw.cmd_execute_commands(e.unwrap().raw_handle(),&[self.command]);
        });
    }}
}
impl Drop for RadixCache {
    fn drop(&mut self) {
        // Invalidation/teardown only, never a per-step wait.
        let _=self.device.poll(wgpu::PollType::Wait);
        unsafe{self.raw.destroy_command_pool(self.pool,None);}
    }
}
