// Isolated command-reuse feasibility probe; never linked into stepping.
struct RawCompute {
    raw: ash::Device, queue: vk::Queue, pool: vk::CommandPool, command: vk::CommandBuffer,
    buffer: vk::Buffer, memory: vk::DeviceMemory, descriptors: vk::DescriptorPool,
    layout: vk::DescriptorSetLayout, pipeline_layout: vk::PipelineLayout,
    pipeline: vk::Pipeline, set: vk::DescriptorSet, query: vk::QueryPool, fence: vk::Fence,
}
impl RawCompute {
    unsafe fn new(device:&wgpu::Device,queue:&wgpu::Queue,family:u32)->Self {
        let hal=device.as_hal::<Vulkan>().unwrap();let raw=hal.raw_device().clone();
        let props=hal.shared_instance().raw_instance().get_physical_device_memory_properties(hal.raw_physical_device());
        let queue=queue.as_hal::<Vulkan>().unwrap().as_raw();
        let buffer=raw.create_buffer(&vk::BufferCreateInfo::default().size(256).usage(vk::BufferUsageFlags::STORAGE_BUFFER),None).unwrap();
        let req=raw.get_buffer_memory_requirements(buffer);
        let memory_type=(0..props.memory_type_count).find(|&i|req.memory_type_bits&(1<<i)!=0
            && props.memory_types[i as usize].property_flags.contains(vk::MemoryPropertyFlags::HOST_VISIBLE|vk::MemoryPropertyFlags::HOST_COHERENT)).unwrap();
        let memory=raw.allocate_memory(&vk::MemoryAllocateInfo::default().allocation_size(req.size).memory_type_index(memory_type),None).unwrap();
        raw.bind_buffer_memory(buffer,memory,0).unwrap();
        let ptr=raw.map_memory(memory,0,256,vk::MemoryMapFlags::empty()).unwrap();std::ptr::write_bytes(ptr,0,256);raw.unmap_memory(memory);
        let bindings=[vk::DescriptorSetLayoutBinding::default().binding(0).descriptor_type(vk::DescriptorType::STORAGE_BUFFER).descriptor_count(1).stage_flags(vk::ShaderStageFlags::COMPUTE)];
        let layout=raw.create_descriptor_set_layout(&vk::DescriptorSetLayoutCreateInfo::default().bindings(&bindings),None).unwrap();
        let sizes=[vk::DescriptorPoolSize::default().ty(vk::DescriptorType::STORAGE_BUFFER).descriptor_count(1)];
        let descriptors=raw.create_descriptor_pool(&vk::DescriptorPoolCreateInfo::default().max_sets(1).pool_sizes(&sizes),None).unwrap();
        let layouts=[layout];let set=raw.allocate_descriptor_sets(&vk::DescriptorSetAllocateInfo::default().descriptor_pool(descriptors).set_layouts(&layouts)).unwrap()[0];
        let buffers=[vk::DescriptorBufferInfo::default().buffer(buffer).range(256)];
        raw.update_descriptor_sets(&[vk::WriteDescriptorSet::default().dst_set(set).dst_binding(0).descriptor_type(vk::DescriptorType::STORAGE_BUFFER).buffer_info(&buffers)],&[]);
        let pipeline_layout=raw.create_pipeline_layout(&vk::PipelineLayoutCreateInfo::default().set_layouts(&layouts),None).unwrap();
        let module=naga::front::wgsl::parse_str(COMPUTE).unwrap();
        let info=naga::valid::Validator::new(naga::valid::ValidationFlags::all(),naga::valid::Capabilities::all()).validate(&module).unwrap();
        let words=naga::back::spv::write_vec(&module,&info,&naga::back::spv::Options::default(),Some(&naga::back::spv::PipelineOptions{shader_stage:naga::ShaderStage::Compute,entry_point:"main".into()})).unwrap();
        let shader=raw.create_shader_module(&vk::ShaderModuleCreateInfo::default().code(&words),None).unwrap();
        let stage=vk::PipelineShaderStageCreateInfo::default().stage(vk::ShaderStageFlags::COMPUTE).module(shader).name(c"main");
        let pipeline=raw.create_compute_pipelines(vk::PipelineCache::null(),&[vk::ComputePipelineCreateInfo::default().stage(stage).layout(pipeline_layout)],None).unwrap()[0];
        raw.destroy_shader_module(shader,None);
        let pool=raw.create_command_pool(&vk::CommandPoolCreateInfo::default().queue_family_index(family).flags(vk::CommandPoolCreateFlags::RESET_COMMAND_BUFFER),None).unwrap();
        let command=raw.allocate_command_buffers(&vk::CommandBufferAllocateInfo::default().command_pool(pool).level(vk::CommandBufferLevel::PRIMARY).command_buffer_count(1)).unwrap()[0];
        let query=raw.create_query_pool(&vk::QueryPoolCreateInfo::default().query_type(vk::QueryType::TIMESTAMP).query_count(2),None).unwrap();
        let fence=raw.create_fence(&vk::FenceCreateInfo::default(),None).unwrap();
        let result=Self{raw,queue,pool,command,buffer,memory,descriptors,layout,pipeline_layout,pipeline,set,query,fence};
        result.record(); result
    }
    unsafe fn record(&self) {
        let raw=&self.raw; let command=self.command; let query=self.query;
        let pipeline=self.pipeline; let pipeline_layout=self.pipeline_layout; let set=self.set;
        raw.reset_command_buffer(command,vk::CommandBufferResetFlags::empty()).unwrap();
        raw.begin_command_buffer(command,&vk::CommandBufferBeginInfo::default()).unwrap();
        raw.cmd_pipeline_barrier(command,vk::PipelineStageFlags::COMPUTE_SHADER,vk::PipelineStageFlags::COMPUTE_SHADER,vk::DependencyFlags::empty(),
            &[vk::MemoryBarrier::default().src_access_mask(vk::AccessFlags::SHADER_WRITE).dst_access_mask(vk::AccessFlags::SHADER_READ|vk::AccessFlags::SHADER_WRITE)],&[],&[]);
        raw.cmd_reset_query_pool(command,query,0,2);
        raw.cmd_write_timestamp(command,vk::PipelineStageFlags::TOP_OF_PIPE,query,0);
        raw.cmd_bind_pipeline(command,vk::PipelineBindPoint::COMPUTE,pipeline);
        raw.cmd_bind_descriptor_sets(command,vk::PipelineBindPoint::COMPUTE,pipeline_layout,0,&[set],&[]);
        for _ in 0..DISPATCHES {
            raw.cmd_dispatch(command,1,1,1);
            raw.cmd_pipeline_barrier(command,vk::PipelineStageFlags::COMPUTE_SHADER,vk::PipelineStageFlags::COMPUTE_SHADER,vk::DependencyFlags::empty(),
                &[vk::MemoryBarrier::default().src_access_mask(vk::AccessFlags::SHADER_WRITE).dst_access_mask(vk::AccessFlags::SHADER_READ|vk::AccessFlags::SHADER_WRITE)],&[],&[]);
        }
        raw.cmd_write_timestamp(command,vk::PipelineStageFlags::BOTTOM_OF_PIPE,query,1);
        raw.cmd_pipeline_barrier(command,vk::PipelineStageFlags::COMPUTE_SHADER,vk::PipelineStageFlags::HOST,vk::DependencyFlags::empty(),
            &[vk::MemoryBarrier::default().src_access_mask(vk::AccessFlags::SHADER_WRITE).dst_access_mask(vk::AccessFlags::HOST_READ)],&[],&[]);
        raw.end_command_buffer(command).unwrap();
    }
    unsafe fn submit(&self,semaphore:vk::Semaphore,value:u64) {
        self.raw.reset_fences(&[self.fence]).unwrap();
        let commands=[self.command];let semaphores=[semaphore];let values=[value];
        let mut time=vk::TimelineSemaphoreSubmitInfo::default().signal_semaphore_values(&values);
        self.raw.queue_submit(self.queue,&[vk::SubmitInfo::default().command_buffers(&commands).signal_semaphores(&semaphores).push_next(&mut time)],self.fence).unwrap();
    }
    unsafe fn finish(&self)->[u64;2] {
        self.raw.wait_for_fences(&[self.fence],true,30_000_000_000).unwrap();
        let mut result=[0u64;2];self.raw.get_query_pool_results(self.query,0,&mut result,vk::QueryResultFlags::TYPE_64|vk::QueryResultFlags::WAIT).unwrap();result
    }
    unsafe fn values(&self)->Vec<u32> {
        let ptr=self.raw.map_memory(self.memory,0,256,vk::MemoryMapFlags::empty()).unwrap();
        let values=std::slice::from_raw_parts(ptr.cast::<u32>(),64).to_vec();self.raw.unmap_memory(self.memory);values
    }
}
impl Drop for RawCompute {
    fn drop(&mut self) {unsafe {
        self.raw.queue_wait_idle(self.queue).unwrap();
        self.raw.destroy_command_pool(self.pool,None);self.raw.destroy_query_pool(self.query,None);self.raw.destroy_fence(self.fence,None);
        self.raw.destroy_pipeline(self.pipeline,None);self.raw.destroy_pipeline_layout(self.pipeline_layout,None);
        self.raw.destroy_descriptor_pool(self.descriptors,None);self.raw.destroy_descriptor_set_layout(self.layout,None);
        self.raw.destroy_buffer(self.buffer,None);self.raw.free_memory(self.memory,None);
    }}
}
