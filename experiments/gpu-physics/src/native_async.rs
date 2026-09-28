//! Experimental same-family secondary queue. Default stepping does not use it.
use ash::vk;
use std::sync::{Arc, Mutex, atomic::{AtomicBool, Ordering}};
use wgpu::hal::api::Vulkan;

pub(crate) struct SecondaryQueue {
    _device: wgpu::Device,
    raw: ash::Device,
    queue: vk::Queue,
    pub(crate) family: u32,
    submit_lock: Mutex<()>,
    tracked_owner: AtomicBool,
}
impl SecondaryQueue {
    /// Caller retains resources until completion and supplies all dependencies.
    /// wgpu does not track this queue or submissions made through it.
    pub(crate) unsafe fn submit(&self, submits: &[vk::SubmitInfo<'_>], fence: vk::Fence) -> Result<(), vk::Result> {
        let _guard = self.submit_lock.lock().unwrap();
        assert!(!self.tracked_owner.load(Ordering::Acquire), "secondary queue belongs to wgpu");
        self.raw.queue_submit(self.queue, submits, fence)
    }
    /// Give queue one independent wgpu resource/completion tracking. The primary
    /// device remains alive through the HAL drop callback; it alone owns VkDevice.
    /// Cross-device resources still require explicit sharing and GPU dependencies.
    pub(crate) fn create_tracked_device(self: &Arc<Self>, adapter: &wgpu::Adapter)
        -> Result<(wgpu::Device, wgpu::Queue), String>
    {
        let _guard = self.submit_lock.lock().unwrap();
        if self.tracked_owner.swap(true, Ordering::AcqRel) {
            return Err("secondary queue already has a tracked owner".into());
        }
        struct Lease(Arc<SecondaryQueue>);
        impl Drop for Lease {
            fn drop(&mut self) { self.0.tracked_owner.store(false, Ordering::Release); }
        }
        let lease = Lease(self.clone());
        let descriptor = wgpu::DeviceDescriptor {
            label: Some("gpu-render-secondary-device"),
            required_features: self._device.features(),
            required_limits: self._device.limits(),
            memory_hints: wgpu::MemoryHints::MemoryUsage,
            trace: wgpu::Trace::Off,
        };
        unsafe {
            let source = self._device.as_hal::<Vulkan>().ok_or("primary device is not Vulkan")?;
            let hal = adapter.as_hal::<Vulkan>().ok_or("adapter is not Vulkan")?;
            if hal.raw_physical_device() != source.raw_physical_device()
                || hal.shared_instance().raw_instance().handle() != source.shared_instance().raw_instance().handle() {
                return Err("secondary device adapter differs from primary".into());
            }
            let opened = hal.device_from_raw(self.raw.clone(),
                Some(Box::new(move || drop(lease))), source.enabled_device_extensions(),
                descriptor.required_features, &descriptor.memory_hints, self.family, 1)
                .map_err(|e| format!("secondary tracked device creation failed: {e}"))?;
            drop(source);
            drop(hal);
            adapter.create_device_from_hal::<Vulkan>(opened, &descriptor)
                .map_err(|e| format!("secondary tracked device import failed: {e}"))
        }
    }
    pub(crate) fn wait_idle(&self) -> Result<(), vk::Result> {
        let _guard = self.submit_lock.lock().unwrap();
        unsafe { self.raw.queue_wait_idle(self.queue) }
    }
}
impl Drop for SecondaryQueue {
    fn drop(&mut self) {
        // Teardown only; never a per-frame completion shortcut.
        let _ = self.wait_idle();
    }
}

pub(crate) fn open_device(adapter: &wgpu::Adapter, desc: &wgpu::DeviceDescriptor<'_>)
    -> Result<(wgpu::Device, wgpu::Queue, Arc<SecondaryQueue>), String>
{
    unsafe {
        let hal = adapter.as_hal::<Vulkan>().ok_or("secondary queue requires Vulkan")?;
        let families = hal.shared_instance().raw_instance()
            .get_physical_device_queue_family_properties(hal.raw_physical_device());
        // The pinned backend selects family zero. Do not silently substitute
        // a queue with incompatible capabilities or ownership requirements.
        let family = 0;
        let flags = vk::QueueFlags::GRAPHICS | vk::QueueFlags::COMPUTE;
        if families.first().is_none_or(|f| f.queue_count < 2 || !f.queue_flags.contains(flags)) {
            return Err("secondary queue requires two graphics/compute queues in family zero".into());
        }
        let opened = hal.open_with_callback(desc.required_features, &desc.memory_hints,
            Some(Box::new(move |args| {
                let info = args.queue_create_infos.iter_mut()
                    .find(|info| info.queue_family_index == family).expect("backend family changed");
                *info = (*info).queue_priorities(&[1.0, 1.0]);
            }))).map_err(|e| format!("secondary device creation failed: {e}"))?;
        drop(hal);
        let (device, queue) = adapter.create_device_from_hal::<Vulkan>(opened, desc)
            .map_err(|e| format!("secondary device import failed: {e}"))?;
        let raw = device.as_hal::<Vulkan>().unwrap().raw_device().clone();
        let extra = Arc::new(SecondaryQueue {
            queue: raw.get_device_queue(family, 1), raw, family,
            _device: device.clone(), submit_lock: Mutex::new(()), tracked_owner: AtomicBool::new(false),
        });
        Ok((device, queue, extra))
    }
}

/// Distinct Vulkan buffers/memory handles alias one allocation. Each wgpu
/// device owns its own handles; either wrapper may outlive the other.
pub(crate) struct SharedPoseBuffer {
    pub producer: wgpu::Buffer,
    pub consumer: wgpu::Buffer,
    pub size: u64,
}

impl SharedPoseBuffer {
    /// Setup only. Caller excludes concurrent submits on producer_queue and
    /// supplies devices sharing the same raw VkDevice and queue family.
    /// Subsequent accesses require external ready/release GPU dependencies.
    pub(crate) unsafe fn new(producer: &wgpu::Device, producer_queue: &wgpu::Queue,
        consumer: &wgpu::Device, size: u64) -> Result<Self, String> {
        if size == 0 || size % 4 != 0 { return Err("shared pose size must be positive and four-byte aligned".into()); }
        let a = producer.as_hal::<Vulkan>().ok_or("producer is not Vulkan")?;
        let b = consumer.as_hal::<Vulkan>().ok_or("consumer is not Vulkan")?;
        if a.raw_device().handle() != b.raw_device().handle() || a.queue_family_index() != b.queue_family_index() {
            return Err("shared pose devices must share VkDevice and queue family".into());
        }
        if !a.enabled_device_extensions().contains(&ash::khr::external_memory_fd::NAME) {
            return Err("external memory FD extension unavailable".into());
        }
        let raw = a.raw_device();
        let instance = a.shared_instance().raw_instance();
        let usage = vk::BufferUsageFlags::TRANSFER_SRC | vk::BufferUsageFlags::TRANSFER_DST | vk::BufferUsageFlags::STORAGE_BUFFER;
        let external_info = vk::PhysicalDeviceExternalBufferInfo::default().usage(usage)
            .handle_type(vk::ExternalMemoryHandleTypeFlags::OPAQUE_FD);
        let mut external = vk::ExternalBufferProperties::default();
        instance.get_physical_device_external_buffer_properties(a.raw_physical_device(), &external_info, &mut external);
        let features = external.external_memory_properties.external_memory_features;
        if !features.contains(vk::ExternalMemoryFeatureFlags::EXPORTABLE | vk::ExternalMemoryFeatureFlags::IMPORTABLE)
            || features.contains(vk::ExternalMemoryFeatureFlags::DEDICATED_ONLY) {
            return Err("shared pose external memory requires supported non-dedicated FD aliasing".into());
        }
        struct RawBuffer<'a> { raw: &'a ash::Device, buffer: vk::Buffer, memory: vk::DeviceMemory }
        impl Drop for RawBuffer<'_> {
            fn drop(&mut self) { unsafe {
                if self.buffer != vk::Buffer::null() { self.raw.destroy_buffer(self.buffer, None); }
                if self.memory != vk::DeviceMemory::null() { self.raw.free_memory(self.memory, None); }
            }}
        }
        let make = || -> Result<RawBuffer<'_>, String> {
            let mut ext = vk::ExternalMemoryBufferCreateInfo::default().handle_types(vk::ExternalMemoryHandleTypeFlags::OPAQUE_FD);
            let info = vk::BufferCreateInfo::default().size(size).usage(usage)
                .sharing_mode(vk::SharingMode::EXCLUSIVE).push_next(&mut ext);
            Ok(RawBuffer { raw, buffer: raw.create_buffer(&info, None).map_err(|e| format!("shared buffer: {e}"))?, memory: vk::DeviceMemory::null() })
        };
        let mut first = make()?; let mut second = make()?;
        let req = raw.get_buffer_memory_requirements(first.buffer);
        let other = raw.get_buffer_memory_requirements(second.buffer);
        if req.size != other.size || req.alignment != other.alignment || req.memory_type_bits != other.memory_type_bits {
            return Err("shared pose alias memory requirements differ".into());
        }
        // Dedicated requirements are checked separately from external capability.
        for buffer in [first.buffer, second.buffer] {
            let mut dedicated = vk::MemoryDedicatedRequirements::default();
            let mut requirements = vk::MemoryRequirements2::default().push_next(&mut dedicated);
            raw.get_buffer_memory_requirements2(&vk::BufferMemoryRequirementsInfo2::default().buffer(buffer), &mut requirements);
            if dedicated.requires_dedicated_allocation != 0 { return Err("shared pose requires dedicated allocation".into()); }
        }
        let props = instance.get_physical_device_memory_properties(a.raw_physical_device());
        let mt = (0..props.memory_type_count).find(|&i| req.memory_type_bits & (1 << i) != 0
            && props.memory_types[i as usize].property_flags.contains(vk::MemoryPropertyFlags::DEVICE_LOCAL))
            .ok_or("no device-local shared pose memory")?;
        let mut export = vk::ExportMemoryAllocateInfo::default().handle_types(vk::ExternalMemoryHandleTypeFlags::OPAQUE_FD);
        first.memory = raw.allocate_memory(&vk::MemoryAllocateInfo::default().allocation_size(req.size)
            .memory_type_index(mt).push_next(&mut export), None).map_err(|e| format!("shared allocation: {e}"))?;
        raw.bind_buffer_memory(first.buffer, first.memory, 0).map_err(|e| format!("shared bind: {e}"))?;
        let fd_api = ash::khr::external_memory_fd::Device::new(instance, raw);
        let fd = fd_api.get_memory_fd(&vk::MemoryGetFdInfoKHR::default().memory(first.memory)
            .handle_type(vk::ExternalMemoryHandleTypeFlags::OPAQUE_FD)).map_err(|e| format!("shared export: {e}"))?;
        let mut import = vk::ImportMemoryFdInfoKHR::default().handle_type(vk::ExternalMemoryHandleTypeFlags::OPAQUE_FD).fd(fd);
        // OPAQUE_FD import must use the exporter's exact allocation size/type.
        let imported = raw.allocate_memory(&vk::MemoryAllocateInfo::default().allocation_size(req.size)
            .memory_type_index(mt).push_next(&mut import), None);
        second.memory = match imported { Ok(memory) => memory, Err(e) => {
            libc::close(fd); return Err(format!("shared import: {e}"));
        }}; // Successful import consumes the FD.
        raw.bind_buffer_memory(second.buffer, second.memory, 0).map_err(|e| format!("shared alias bind: {e}"))?;
        // Imported HAL buffers promise initialized contents. Clear once at setup,
        // before either wrapper is exposed; never perform this wait per frame.
        let pool = raw.create_command_pool(&vk::CommandPoolCreateInfo::default().queue_family_index(a.queue_family_index()), None)
            .map_err(|e| format!("shared initialization pool: {e}"))?;
        let init = (|| -> Result<(), vk::Result> {
            let cmd = raw.allocate_command_buffers(&vk::CommandBufferAllocateInfo::default().command_pool(pool).command_buffer_count(1))?[0];
            raw.begin_command_buffer(cmd, &vk::CommandBufferBeginInfo::default())?;
            raw.cmd_fill_buffer(cmd, first.buffer, 0, size, 0);
            raw.end_command_buffer(cmd)?;
            let fence = raw.create_fence(&vk::FenceCreateInfo::default(), None)?;
            let commands = [cmd];
            let queue = producer_queue.as_hal::<Vulkan>().unwrap();
            let result = raw.queue_submit(queue.as_raw(), &[vk::SubmitInfo::default().command_buffers(&commands)], fence)
                .and_then(|_| raw.wait_for_fences(&[fence], true, u64::MAX));
            raw.destroy_fence(fence, None);
            result
        })();
        raw.destroy_command_pool(pool, None);
        init.map_err(|e| format!("shared initialization: {e}"))?;
        let wrap = |device: &wgpu::Device, allocation: &mut RawBuffer<'_>| {
            let hal = wgpu::hal::vulkan::Buffer::from_raw_managed(allocation.buffer, allocation.memory, 0, size);
            allocation.buffer = vk::Buffer::null(); allocation.memory = vk::DeviceMemory::null();
            device.create_buffer_from_hal::<Vulkan>(hal, &wgpu::BufferDescriptor {
                label: Some("shared-queue-pose"), size,
                usage: wgpu::BufferUsages::COPY_SRC | wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::STORAGE,
                mapped_at_creation: false,
            })
        };
        Ok(Self { producer: wrap(producer, &mut first), consumer: wrap(consumer, &mut second), size })
    }
}

/// One-shot request, consumed only by the resident world-step encoder.
pub(crate) struct RenderCopy {
    state: wgpu::Buffer, cold: wgpu::Buffer, ready: vk::Semaphore,
    release: Option<vk::Semaphore>, pub submitted: Arc<AtomicBool>,
}
impl RenderCopy {
    pub(crate) fn encode(self, encoder: &mut wgpu::CommandEncoder, queue: &wgpu::Queue,
        state: &wgpu::Buffer, cold: &wgpu::Buffer) {
        if self.state.size() != state.size() || self.cold.size() > cold.size() { return; }
        encoder.copy_buffer_to_buffer(state, 0, &self.state, 0, self.state.size());
        encoder.copy_buffer_to_buffer(cold, 0, &self.cold, 0, self.cold.size());
        unsafe {
            let hal = queue.as_hal::<Vulkan>().unwrap();
            if let Some(release) = self.release { hal.add_wait_binary_semaphore(release, vk::PipelineStageFlags::TRANSFER); }
            hal.add_signal_semaphore(self.ready, None);
        }
        self.submitted.store(true, Ordering::Release);
    }
}
struct RenderSlot { state: SharedPoseBuffer, cold: SharedPoseBuffer, ready: vk::Semaphore, release: vk::Semaphore, used: bool }
pub(crate) struct RenderBridge {
    producer_device: wgpu::Device, producer_queue: wgpu::Queue,
    consumer_device: wgpu::Device, consumer_queue: wgpu::Queue,
    slots: Vec<RenderSlot>, next: usize, copy_heap: bool, merge_copy: bool, merged: Option<Arc<AtomicBool>>, merged_copies: u64, separate_copies: u64,
}
impl RenderBridge {
    pub(crate) fn new(producer: &crate::sim::GpuDevice, consumer: &crate::sim::GpuDevice) -> Self {
        Self { producer_device: producer.device.clone(), producer_queue: producer.queue.clone(),
            consumer_device: consumer.device.clone(), consumer_queue: consumer.queue.clone(), slots: Vec::new(), next: 0,
            copy_heap: std::env::var("GPU_PHYSICS_RENDER_COPY_HEAP").as_deref() == Ok("1"),
            merge_copy: std::env::var("GPU_PHYSICS_RENDER_MERGE_COPY").as_deref() == Ok("1"), merged: None, merged_copies: 0, separate_copies: 0 }
    }
    pub(crate) fn arm(&mut self, world: crate::api::WorldId) {
        self.merged = None;
        if !self.merge_copy || self.slots.is_empty() { return; }
        let slot = &self.slots[self.next];
        let submitted = Arc::new(AtomicBool::new(false));
        let request = RenderCopy { state: slot.state.producer.clone(), cold: slot.cold.producer.clone(),
            ready: slot.ready, release: slot.used.then_some(slot.release), submitted: submitted.clone() };
        if crate::api::b3_world_arm_render_copy(world, request) { self.merged = Some(submitted); }
    }
    fn clear(&mut self) {
        if self.slots.is_empty() { return; }
        self.producer_device.poll(wgpu::PollType::Wait).expect("pose producer completion");
        self.consumer_device.poll(wgpu::PollType::Wait).expect("pose consumer completion");
        unsafe {
            let hal = self.producer_device.as_hal::<Vulkan>().unwrap();
            for slot in self.slots.drain(..) {
                hal.raw_device().destroy_semaphore(slot.ready, None);
                hal.raw_device().destroy_semaphore(slot.release, None);
            }
        }
        self.next = 0;
    }
    /// Single viewer thread owns both queues and calls finish immediately before
    /// its render submission. Surface acquisition must succeed before this call.
    pub(crate) fn copy(&mut self, state: &wgpu::Buffer, cold: &wgpu::Buffer, body_slots: u32) -> (wgpu::Buffer, wgpu::Buffer) {
        if self.merged.take().is_some_and(|done| done.load(Ordering::Acquire)) {
            self.merged_copies += 1;
            let slot = &self.slots[self.next];
            return (slot.state.consumer.clone(), slot.cold.consumer.clone());
        }
        // render_scene.wgsl indexes only the leading BodyColdGpu array; mesh,
        // material and joint heap sections are not referenced by the renderer.
        let cold_bytes = if self.copy_heap { cold.size() } else {
            (u64::from(body_slots) * std::mem::size_of::<crate::types::BodyColdGpu>() as u64).max(64)
        };
        assert!(cold_bytes <= cold.size(), "render body slot span exceeds cold buffer");
        if self.slots.first().is_none_or(|s| s.state.size != state.size() || s.cold.size != cold_bytes) {
            self.clear();
            unsafe {
                let hal = self.producer_device.as_hal::<Vulkan>().unwrap();
                for _ in 0..2 {
                    self.slots.push(RenderSlot {
                        state: SharedPoseBuffer::new(&self.producer_device, &self.producer_queue, &self.consumer_device, state.size()).expect("shared render state"),
                        cold: SharedPoseBuffer::new(&self.producer_device, &self.producer_queue, &self.consumer_device, cold_bytes).expect("shared render cold"),
                        ready: hal.raw_device().create_semaphore(&vk::SemaphoreCreateInfo::default(), None).unwrap(),
                        release: hal.raw_device().create_semaphore(&vk::SemaphoreCreateInfo::default(), None).unwrap(), used: false,
                    });
                }
            }
        }
        let slot = &self.slots[self.next];
        let mut encoder = self.producer_device.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: Some("render-pose-copy") });
        encoder.copy_buffer_to_buffer(state, 0, &slot.state.producer, 0, state.size());
        encoder.copy_buffer_to_buffer(cold, 0, &slot.cold.producer, 0, cold_bytes);
        unsafe {
            let hal = self.producer_queue.as_hal::<Vulkan>().unwrap();
            if slot.used { hal.add_wait_binary_semaphore(slot.release, vk::PipelineStageFlags::TRANSFER); }
            hal.add_signal_semaphore(slot.ready, None);
        }
        self.producer_queue.submit([encoder.finish()]);
        self.separate_copies += 1;
        (slot.state.consumer.clone(), slot.cold.consumer.clone())
    }
    /// No image was acquired after a merged copy. Consume ready and release
    /// the slot on the GPU, without attempting a draw or CPU completion wait.
    pub(crate) fn discard(&mut self) {
        if self.merged.take().is_some_and(|done| done.load(Ordering::Acquire)) {
            self.finish();
            self.consumer_queue.submit([self.consumer_device.create_command_encoder(
                &wgpu::CommandEncoderDescriptor { label: Some("release-undrawn-pose") }).finish()]);
        }
    }
    pub(crate) fn finish(&mut self) {
        let slot = &mut self.slots[self.next];
        unsafe {
            let hal = self.consumer_queue.as_hal::<Vulkan>().unwrap();
            hal.add_wait_binary_semaphore(slot.ready, vk::PipelineStageFlags::VERTEX_SHADER);
            hal.add_signal_semaphore(slot.release, None);
        }
        slot.used = true;
        self.next = (self.next + 1) % self.slots.len();
    }
}
impl Drop for RenderBridge { fn drop(&mut self) { self.clear(); eprintln!("render-copy-counts: merged={} separate={}", self.merged_copies, self.separate_copies); } }

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn secondary_tracked_device_has_independent_completion_and_lifetime() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new_with_secondary_queue(None)).unwrap();
        let secondary = gpu.secondary_queue.as_ref().unwrap().clone();
        let (render_device, render_queue) = secondary.create_tracked_device(&gpu.adapter).unwrap();
        assert!(secondary.create_tracked_device(&gpu.adapter).is_err());
        unsafe {
            assert_ne!(gpu.queue.as_hal::<Vulkan>().unwrap().as_raw(),
                render_queue.as_hal::<Vulkan>().unwrap().as_raw());
        }
        // Observe work through each device's own completion and mapping paths.
        for (device, queue, value) in [(&gpu.device, &gpu.queue, 17u32),
            (&render_device, &render_queue, 91u32)] {
            let source = device.create_buffer(&wgpu::BufferDescriptor {
                label: None, size: 4, usage: wgpu::BufferUsages::COPY_SRC | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            });
            let readback = device.create_buffer(&wgpu::BufferDescriptor {
                label: None, size: 4, usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
                mapped_at_creation: false,
            });
            queue.write_buffer(&source, 0, &value.to_ne_bytes());
            let mut encoder = device.create_command_encoder(&Default::default());
            encoder.copy_buffer_to_buffer(&source, 0, &readback, 0, 4);
            queue.submit([encoder.finish()]);
            let (tx, rx) = std::sync::mpsc::channel();
            readback.slice(..).map_async(wgpu::MapMode::Read, move |r| tx.send(r).unwrap());
            device.poll(wgpu::PollType::Wait).unwrap();
            rx.recv().unwrap().unwrap();
            assert_eq!(&*readback.slice(..).get_mapped_range(), &value.to_ne_bytes());
            readback.unmap();
        }
        drop(gpu);
        // The render wrapper keeps the primary VkDevice alive after its public
        // owner disappears. Submission must remain valid until render teardown.
        render_queue.submit([]);
        render_device.poll(wgpu::PollType::Wait).unwrap();
        drop(render_queue);
        drop(render_device);
        assert!(!secondary.tracked_owner.load(Ordering::Acquire));
    }

    #[test]
    fn tracked_queues_exchange_binary_dependencies_without_cpu_waits() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new_with_secondary_queue(None)).unwrap();
        let secondary = gpu.secondary_queue.as_ref().unwrap();
        let (render_device, render_queue) = secondary.create_tracked_device(&gpu.adapter).unwrap();
        unsafe {
            let raw = &secondary.raw;
            let ready = raw.create_semaphore(&vk::SemaphoreCreateInfo::default(), None).unwrap();
            let released = raw.create_semaphore(&vk::SemaphoreCreateInfo::default(), None).unwrap();
            for generation in 0..64 {
                // Each slot's previous reader releases it before producer reuse.
                // All submissions still go through wgpu's own tracking/relays.
                let producer = gpu.device.create_command_encoder(&Default::default()).finish();
                {
                    let hal = gpu.queue.as_hal::<Vulkan>().unwrap();
                    if generation > 0 {
                        hal.add_wait_binary_semaphore(released, vk::PipelineStageFlags::ALL_COMMANDS);
                    }
                    hal.add_signal_semaphore(ready, None);
                }
                gpu.queue.submit([producer]);
                let consumer = render_device.create_command_encoder(&Default::default()).finish();
                {
                    let hal = render_queue.as_hal::<Vulkan>().unwrap();
                    hal.add_wait_binary_semaphore(ready, vk::PipelineStageFlags::ALL_COMMANDS);
                    hal.add_signal_semaphore(released, None);
                }
                render_queue.submit([consumer]);
            }
            // Drain the final release, then observe both tracked completions.
            gpu.queue.as_hal::<Vulkan>().unwrap()
                .add_wait_binary_semaphore(released, vk::PipelineStageFlags::ALL_COMMANDS);
            gpu.queue.submit([gpu.device.create_command_encoder(&Default::default()).finish()]);
            gpu.device.poll(wgpu::PollType::Wait).unwrap();
            render_device.poll(wgpu::PollType::Wait).unwrap();
            raw.destroy_semaphore(ready, None);
            raw.destroy_semaphore(released, None);
        }
    }

    #[test]
    fn shared_pose_ring_preserves_tracked_generations_and_alias_lifetime() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new_with_secondary_queue(None)).unwrap();
        let secondary = gpu.secondary_queue.as_ref().unwrap();
        let (render_device, render_queue) = secondary.create_tracked_device(&gpu.adapter).unwrap();
        unsafe {
            let slots: Vec<_> = (0..2).map(|_| SharedPoseBuffer::new(&gpu.device, &gpu.queue, &render_device, 4).unwrap()).collect();
            assert_eq!(slots[0].size, 4);
            let raw = &secondary.raw;
            let ready: Vec<_> = (0..2).map(|_| raw.create_semaphore(&vk::SemaphoreCreateInfo::default(), None).unwrap()).collect();
            let release: Vec<_> = (0..2).map(|_| raw.create_semaphore(&vk::SemaphoreCreateInfo::default(), None).unwrap()).collect();
            let output = render_device.create_buffer(&wgpu::BufferDescriptor {
                label: None, size: 260, usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ, mapped_at_creation: false,
            });
            for generation in 0..64u32 {
                let slot = generation as usize % 2;
                gpu.queue.write_buffer(&slots[slot].producer, 0, &(generation + 1).to_ne_bytes());
                {
                    let hal = gpu.queue.as_hal::<Vulkan>().unwrap();
                    if generation >= 2 { hal.add_wait_binary_semaphore(release[slot], vk::PipelineStageFlags::ALL_COMMANDS); }
                    hal.add_signal_semaphore(ready[slot], None);
                }
                gpu.queue.submit([gpu.device.create_command_encoder(&Default::default()).finish()]);
                let mut encoder = render_device.create_command_encoder(&Default::default());
                encoder.copy_buffer_to_buffer(&slots[slot].consumer, 0, &output, generation as u64 * 4, 4);
                {
                    let hal = render_queue.as_hal::<Vulkan>().unwrap();
                    hal.add_wait_binary_semaphore(ready[slot], vk::PipelineStageFlags::ALL_COMMANDS);
                    hal.add_signal_semaphore(release[slot], None);
                }
                render_queue.submit([encoder.finish()]);
            }
            gpu.device.poll(wgpu::PollType::Wait).unwrap();
            render_device.poll(wgpu::PollType::Wait).unwrap();
            // Imported memory must survive destruction of the exporter wrapper.
            let surviving_alias = slots[1].consumer.clone();
            drop(slots);
            gpu.device.poll(wgpu::PollType::Wait).unwrap();
            let mut encoder = render_device.create_command_encoder(&Default::default());
            encoder.copy_buffer_to_buffer(&surviving_alias, 0, &output, 256, 4);
            render_queue.submit([encoder.finish()]);
            let (tx, rx) = std::sync::mpsc::channel();
            output.slice(..).map_async(wgpu::MapMode::Read, move |r| tx.send(r).unwrap());
            render_device.poll(wgpu::PollType::Wait).unwrap();
            rx.recv().unwrap().unwrap();
            let bytes = output.slice(..).get_mapped_range();
            let values: Vec<_> = bytes.chunks_exact(4).map(|b| u32::from_ne_bytes(b.try_into().unwrap())).collect();
            let mut expected: Vec<u32> = (1..=64).collect(); expected.push(64);
            assert_eq!(values, expected);
            drop(bytes); output.unmap();
            for semaphore in ready.into_iter().chain(release) { raw.destroy_semaphore(semaphore, None); }
        }
    }

    #[test]
    fn discarded_merged_poses_release_slots_for_reuse() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new_with_secondary_queue(None)).unwrap();
        let mut render = gpu.clone();
        (render.device, render.queue) = gpu.secondary_queue.as_ref().unwrap().create_tracked_device(&gpu.adapter).unwrap();
        let mut bridge = RenderBridge::new(&gpu, &render);
        let state = gpu.device.create_buffer(&wgpu::BufferDescriptor { label: None, size: std::mem::size_of::<crate::types::BodyStateGpu>() as u64,
            usage: wgpu::BufferUsages::COPY_SRC | wgpu::BufferUsages::COPY_DST, mapped_at_creation: false });
        let cold = gpu.device.create_buffer(&wgpu::BufferDescriptor { label: None, size: 64,
            usage: wgpu::BufferUsages::COPY_SRC | wgpu::BufferUsages::COPY_DST, mapped_at_creation: false });
        bridge.copy(&state, &cold, 1);
        bridge.finish();
        render.queue.submit([render.device.create_command_encoder(&Default::default()).finish()]);
        for generation in 0..64 {
            let slot = &bridge.slots[bridge.next];
            let submitted = Arc::new(AtomicBool::new(false));
            let request = RenderCopy { state: slot.state.producer.clone(), cold: slot.cold.producer.clone(),
                ready: slot.ready, release: slot.used.then_some(slot.release), submitted: submitted.clone() };
            let mut encoder = gpu.device.create_command_encoder(&Default::default());
            request.encode(&mut encoder, &gpu.queue, &state, &cold);
            gpu.queue.submit([encoder.finish()]);
            bridge.merged = Some(submitted);
            let before = bridge.next;
            bridge.discard();
            assert_eq!(bridge.next, (before + 1) % 2, "generation {generation}");
            bridge.discard(); // A second discard must not consume/release twice.
            assert_eq!(bridge.next, (before + 1) % 2);
        }
        gpu.device.poll(wgpu::PollType::Wait).unwrap();
        render.device.poll(wgpu::PollType::Wait).unwrap();
    }

    #[test]
    fn secondary_queue_handoff_preserves_every_generation() {
        let gpu = pollster::block_on(crate::sim::GpuDevice::new_with_secondary_queue(None)).unwrap();
        let secondary = gpu.secondary_queue.as_ref().unwrap();
        gpu.device.poll(wgpu::PollType::Wait).unwrap();
        unsafe {
            let hal = gpu.device.as_hal::<Vulkan>().unwrap();
            let raw = hal.raw_device();
            let primary = gpu.queue.as_hal::<Vulkan>().unwrap().as_raw();
            assert_ne!(primary, secondary.queue);
            let props = hal.shared_instance().raw_instance()
                .get_physical_device_memory_properties(hal.raw_physical_device());
            // Raw buffers are outside wgpu tracking. No concurrent wgpu queue
            // operations occur while this test directly uses the primary queue.
            let mut allocations = Vec::new();
            for bytes in [8u64, 256] {
                let buffer = raw.create_buffer(&vk::BufferCreateInfo::default().size(bytes)
                    .usage(vk::BufferUsageFlags::TRANSFER_SRC | vk::BufferUsageFlags::TRANSFER_DST)
                    .sharing_mode(vk::SharingMode::EXCLUSIVE), None).unwrap();
                let req = raw.get_buffer_memory_requirements(buffer);
                let mt = (0..props.memory_type_count).find(|&i| req.memory_type_bits & (1<<i) != 0
                    && props.memory_types[i as usize].property_flags.contains(
                        vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT)).unwrap();
                let memory = raw.allocate_memory(&vk::MemoryAllocateInfo::default()
                    .allocation_size(req.size).memory_type_index(mt), None).unwrap();
                raw.bind_buffer_memory(buffer, memory, 0).unwrap();
                allocations.push((buffer, memory));
            }
            let pool = raw.create_command_pool(&vk::CommandPoolCreateInfo::default()
                .queue_family_index(secondary.family), None).unwrap();
            let commands = raw.allocate_command_buffers(&vk::CommandBufferAllocateInfo::default()
                .command_pool(pool).command_buffer_count(128)).unwrap();
            let mut ready = Vec::new(); let mut free = Vec::new();
            for _ in 0..2 {
                ready.push(raw.create_semaphore(&vk::SemaphoreCreateInfo::default(), None).unwrap());
                free.push(raw.create_semaphore(&vk::SemaphoreCreateInfo::default(), None).unwrap());
            }
            let fence = raw.create_fence(&vk::FenceCreateInfo::default(), None).unwrap();
            for step in 0..64usize {
                let slot = step % 2;
                let producer = commands[2*step]; let consumer = commands[2*step+1];
                raw.begin_command_buffer(producer, &vk::CommandBufferBeginInfo::default()).unwrap();
                raw.cmd_fill_buffer(producer, allocations[0].0, (slot*4) as u64, 4, step as u32 + 1);
                raw.end_command_buffer(producer).unwrap();
                raw.begin_command_buffer(consumer, &vk::CommandBufferBeginInfo::default()).unwrap();
                raw.cmd_copy_buffer(consumer, allocations[0].0, allocations[1].0,
                    &[vk::BufferCopy { src_offset:(slot*4) as u64, dst_offset:(step*4) as u64, size:4 }]);
                if step == 63 {
                    raw.cmd_pipeline_barrier(consumer, vk::PipelineStageFlags::TRANSFER,
                        vk::PipelineStageFlags::HOST, vk::DependencyFlags::empty(),
                        &[vk::MemoryBarrier::default().src_access_mask(vk::AccessFlags::TRANSFER_WRITE)
                            .dst_access_mask(vk::AccessFlags::HOST_READ)], &[], &[]);
                }
                raw.end_command_buffer(consumer).unwrap();
                let produced = [ready[slot]]; let released = [free[slot]];
                let stage = [vk::PipelineStageFlags::TRANSFER];
                let pc = [producer]; let cc = [consumer];
                let mut ps = vk::SubmitInfo::default().command_buffers(&pc).signal_semaphores(&produced);
                if step >= 2 { ps = ps.wait_semaphores(&released).wait_dst_stage_mask(&stage); }
                secondary.submit(&[ps], vk::Fence::null()).unwrap();
                let cs = vk::SubmitInfo::default().wait_semaphores(&produced).wait_dst_stage_mask(&stage)
                    .command_buffers(&cc).signal_semaphores(&released);
                raw.queue_submit(primary, &[cs], if step==63 {fence} else {vk::Fence::null()}).unwrap();
            }
            // Only final observation waits on the CPU. GPU semaphores protect
            // each slot until the consumer has finished reading its generation.
            raw.wait_for_fences(&[fence], true, 10_000_000_000).unwrap();
            secondary.wait_idle().unwrap();
            raw.queue_wait_idle(primary).unwrap();
            let mapped = raw.map_memory(allocations[1].1, 0, 256, vk::MemoryMapFlags::empty()).unwrap();
            let values = std::slice::from_raw_parts(mapped.cast::<u32>(), 64).to_vec();
            raw.unmap_memory(allocations[1].1);
            raw.destroy_fence(fence, None);
            for sem in ready.into_iter().chain(free) { raw.destroy_semaphore(sem, None); }
            raw.destroy_command_pool(pool, None);
            for (buffer, memory) in allocations {
                raw.destroy_buffer(buffer, None); raw.free_memory(memory, None);
            }
            assert_eq!(values, (1..=64u32).collect::<Vec<_>>());
        }
    }
}
