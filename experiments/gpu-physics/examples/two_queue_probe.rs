//! Transport feasibility probe, not a physics or performance benchmark.
//! Two wgpu contexts share one VkDevice, distinct queues and two aliased slots.
//! All per-publication dependencies are GPU waits. Readback occurs only at end.
use ash::vk;
use std::io::Write;
use std::sync::atomic::{AtomicBool, Ordering};
use wgpu::{hal::api::Vulkan, util::DeviceExt};

static VALIDATION_ERROR: AtomicBool = AtomicBool::new(false);

fn wait(queue: &wgpu::Queue, semaphore: vk::Semaphore, value: u64) {
    // This example owns both queues on this thread. No concurrent native/wgpu submit.
    unsafe {
        let hal = queue.as_hal::<Vulkan>().unwrap();
        let semaphores = [semaphore];
        let values = [value];
        let stages = [vk::PipelineStageFlags::ALL_COMMANDS];
        let mut timeline =
            vk::TimelineSemaphoreSubmitInfo::default().wait_semaphore_values(&values);
        let submit = vk::SubmitInfo::default()
            .wait_semaphores(&semaphores)
            .wait_dst_stage_mask(&stages)
            .push_next(&mut timeline);
        hal.raw_device()
            .queue_submit(hal.as_raw(), &[submit], vk::Fence::null())
            .unwrap();
    }
}

fn signal(queue: &wgpu::Queue, semaphore: vk::Semaphore, value: u64) {
    unsafe {
        queue
            .as_hal::<Vulkan>()
            .unwrap()
            .add_signal_semaphore(semaphore, Some(value));
    }
}

fn main() {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info"))
        .format(|buf, record| {
            if record.level() == log::Level::Error {
                VALIDATION_ERROR.store(true, Ordering::Relaxed);
            }
            writeln!(
                buf,
                "{} {}: {}",
                record.level(),
                record.target(),
                record.args()
            )
        })
        .init();
    pollster::block_on(run());
    // run() has dropped both contexts before declaring success, so destruction
    // errors cannot be hidden behind an earlier successful contents comparison.
    assert!(
        !VALIDATION_ERROR.load(Ordering::Relaxed),
        "validation reported errors; see log"
    );
    println!("{{\"status\":\"pass\",\"rounds\":3,\"publications_per_round\":512,\"slots\":2,\"per_publication_cpu_waits\":0,\"performance_claim\":false,\"dedicated\":{}}}",std::env::args().any(|a|a=="--dedicated"));
}

async fn run() {
    let instance = wgpu::Instance::new(&wgpu::InstanceDescriptor {
        backends: wgpu::Backends::VULKAN,
        flags: wgpu::InstanceFlags::debugging(),
        ..Default::default()
    });
    let adapter = instance
        .request_adapter(&wgpu::RequestAdapterOptions::default())
        .await
        .unwrap();
    let desc = wgpu::DeviceDescriptor {
        label: Some("two-queue-probe"),
        required_features: wgpu::Features::empty(),
        required_limits: wgpu::Limits::default(),
        memory_hints: wgpu::MemoryHints::MemoryUsage,
        trace: wgpu::Trace::Off,
    };
    let dedicated=std::env::args().any(|a|a=="--dedicated");
    let (producer, pq, consumer, cq) = unsafe {
        let hal = adapter.as_hal::<Vulkan>().unwrap();
        let families = hal
            .shared_instance()
            .raw_instance()
            .get_physical_device_queue_family_properties(hal.raw_physical_device());
        assert!(families[0].queue_count >= 2);
        assert!(families[0]
            .queue_flags
            .contains(vk::QueueFlags::COMPUTE | vk::QueueFlags::GRAPHICS));
        let family=if dedicated {families.iter().position(|f|f.queue_flags.contains(vk::QueueFlags::COMPUTE)
            && !f.queue_flags.contains(vk::QueueFlags::GRAPHICS)).expect("compute family") as u32}else{0};
        let open = hal
            .open_with_callback(
                desc.required_features,
                &desc.memory_hints,
                Some(Box::new(move |args| {
                    if dedicated {
                        args.queue_create_infos.push(vk::DeviceQueueCreateInfo::default().queue_family_index(family).queue_priorities(&[1.0]));
                    }else{
                        args.queue_create_infos[0] = vk::DeviceQueueCreateInfo::default()
                            .queue_family_index(0).queue_priorities(&[1.0, 1.0]);
                    }
                })),
            )
            .unwrap();
        let raw = open.device.raw_device().clone();
        let extensions = open.device.enabled_device_extensions().to_vec();
        let (producer, pq) = adapter
            .create_device_from_hal::<Vulkan>(open, &desc)
            .unwrap();
        // Only context zero owns VkDevice. Context one keeps that owner alive.
        let owner = producer.clone();
        let second = hal
            .device_from_raw(
                raw,
                Some(Box::new(move || drop(owner))),
                &extensions,
                desc.required_features,
                &desc.memory_hints,
                family,
                if dedicated {0}else{1},
            )
            .unwrap();
        let (consumer, cq) = adapter
            .create_device_from_hal::<Vulkan>(second, &desc)
            .unwrap();
        if dedicated {(consumer,cq,producer,pq)}else{(producer,pq,consumer,cq)}
    };
    unsafe {
        assert_ne!(
            pq.as_hal::<Vulkan>().unwrap().as_raw(),
            cq.as_hal::<Vulkan>().unwrap().as_raw()
        );
    }
    // Multiple lifecycles include slot-size growth and skipped presentations.
    for (round, bytes) in [4096u64, 65536, 16384].into_iter().enumerate() {
        transfer_round(&producer, &pq, &consumer, &cq, bytes, round as u32);
    }
}

fn transfer_round(
    pd: &wgpu::Device,
    pq: &wgpu::Queue,
    cd: &wgpu::Device,
    cq: &wgpu::Queue,
    bytes: u64,
    round: u32,
) {
    const FRAMES: usize = 512;
    let words = bytes as usize / 4;
    let input: Vec<u32> = (0..FRAMES * words)
        .map(|i| (i as u32).wrapping_mul(747796405) ^ ((i / words) as u32).rotate_left(13) ^ round)
        .collect();
    let source = pd.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: Some("immutable-frame-patterns"),
        contents: bytemuck::cast_slice(&input),
        usage: wgpu::BufferUsages::COPY_SRC,
    });
    let output = cd.create_buffer(&wgpu::BufferDescriptor {
        label: Some("all-frame-results"),
        size: bytes * FRAMES as u64,
        usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
        mapped_at_creation: false,
    });
    let raw;
    let mut slots = Vec::new();
    let ready;
    let done;
    unsafe {
        let hal = pd.as_hal::<Vulkan>().unwrap();
        raw = hal.raw_device().clone();
        let producer_family=hal.queue_family_index();
        let consumer_family=cd.as_hal::<Vulkan>().unwrap().queue_family_index();
        let families=[producer_family,consumer_family];
        eprintln!("snapshot families: producer={producer_family}, consumer={consumer_family}");
        let props = hal
            .shared_instance()
            .raw_instance()
            .get_physical_device_memory_properties(hal.raw_physical_device());
        for _ in 0..2 {
            let mut info = vk::BufferCreateInfo::default()
                .size(bytes)
                .usage(vk::BufferUsageFlags::TRANSFER_SRC | vk::BufferUsageFlags::TRANSFER_DST)
                .sharing_mode(vk::SharingMode::EXCLUSIVE);
            if producer_family!=consumer_family {
                info=info.sharing_mode(vk::SharingMode::CONCURRENT).queue_family_indices(&families);
            }
            let a = raw.create_buffer(&info, None).unwrap();
            let b = raw.create_buffer(&info, None).unwrap();
            let mut dedicated_a = vk::MemoryDedicatedRequirements::default();
            let mut req_a = vk::MemoryRequirements2::default().push_next(&mut dedicated_a);
            raw.get_buffer_memory_requirements2(
                &vk::BufferMemoryRequirementsInfo2::default().buffer(a),
                &mut req_a,
            );
            let mut dedicated_b = vk::MemoryDedicatedRequirements::default();
            let mut req_b = vk::MemoryRequirements2::default().push_next(&mut dedicated_b);
            raw.get_buffer_memory_requirements2(
                &vk::BufferMemoryRequirementsInfo2::default().buffer(b),
                &mut req_b,
            );
            let req_a = req_a.memory_requirements;
            let req_b = req_b.memory_requirements;
            assert_eq!(dedicated_a.requires_dedicated_allocation, vk::FALSE);
            assert_eq!(dedicated_b.requires_dedicated_allocation, vk::FALSE);
            let bits = req_a.memory_type_bits & req_b.memory_type_bits;
            let memory_type = (0..props.memory_type_count)
                .find(|&i| {
                    bits & (1 << i) != 0
                        && props.memory_types[i as usize]
                            .property_flags
                            .contains(vk::MemoryPropertyFlags::DEVICE_LOCAL)
                })
                .unwrap();
            let memory = raw
                .allocate_memory(
                    &vk::MemoryAllocateInfo::default()
                        .allocation_size(req_a.size.max(req_b.size))
                        .memory_type_index(memory_type),
                    None,
                )
                .unwrap();
            raw.bind_buffer_memory(a, memory, 0).unwrap();
            raw.bind_buffer_memory(b, memory, 0).unwrap();
            // The public import contract requires initialized memory. This
            // startup-only wait happens before either buffer is exposed to core.
            let pool = raw
                .create_command_pool(
                    &vk::CommandPoolCreateInfo::default().queue_family_index(producer_family),
                    None,
                )
                .unwrap();
            let command = raw
                .allocate_command_buffers(
                    &vk::CommandBufferAllocateInfo::default()
                        .command_pool(pool)
                        .level(vk::CommandBufferLevel::PRIMARY)
                        .command_buffer_count(1),
                )
                .unwrap()[0];
            raw.begin_command_buffer(command, &vk::CommandBufferBeginInfo::default())
                .unwrap();
            raw.cmd_fill_buffer(command, a, 0, bytes, 0);
            raw.end_command_buffer(command).unwrap();
            let commands = [command];
            let init_fence = raw
                .create_fence(&vk::FenceCreateInfo::default(), None)
                .unwrap();
            raw.queue_submit(
                pq.as_hal::<Vulkan>().unwrap().as_raw(),
                &[vk::SubmitInfo::default().command_buffers(&commands)],
                init_fence,
            )
            .unwrap();
            raw.wait_for_fences(&[init_fence], true, 30_000_000_000)
                .unwrap();
            raw.destroy_fence(init_fence, None);
            raw.destroy_command_pool(pool, None);
            let desc = wgpu::BufferDescriptor {
                label: Some("shared-snapshot-alias"),
                size: bytes,
                usage: wgpu::BufferUsages::COPY_SRC | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            };
            // Distinct VkBuffer handles; neither wrapper owns the shared allocation.
            // Both alias the memory initialized above; frame copies then replace it.
            let a =
                pd.create_buffer_from_hal::<Vulkan>(wgpu::hal::vulkan::Buffer::from_raw(a), &desc);
            let b =
                cd.create_buffer_from_hal::<Vulkan>(wgpu::hal::vulkan::Buffer::from_raw(b), &desc);
            slots.push((a, b, memory));
        }
        let mut ty =
            vk::SemaphoreTypeCreateInfo::default().semaphore_type(vk::SemaphoreType::TIMELINE);
        let info = vk::SemaphoreCreateInfo::default().push_next(&mut ty);
        ready = raw.create_semaphore(&info, None).unwrap();
        done = raw.create_semaphore(&info, None).unwrap();
    }
    for frame in 0..FRAMES {
        let value = frame as u64 + 1;
        let (a, b, _) = &slots[frame % 2];
        if frame >= 2 {
            wait(pq, done, value - 2);
        }
        let mut encoder = pd.create_command_encoder(&Default::default());
        encoder.copy_buffer_to_buffer(&source, frame as u64 * bytes, a, 0, bytes);
        signal(pq, ready, value);
        pq.submit([encoder.finish()]);
        wait(cq, ready, value);
        let mut encoder = cd.create_command_encoder(&Default::default());
        if frame % 17 != 0 {
            encoder.copy_buffer_to_buffer(b, 0, &output, frame as u64 * bytes, bytes);
        } else {
            // An acquire failure/cancel must still release the published slot.
            encoder.clear_buffer(&output, frame as u64 * bytes, Some(bytes));
        }
        signal(cq, done, value);
        cq.submit([encoder.finish()]);
    }
    let (tx, rx) = std::sync::mpsc::channel();
    output
        .slice(..)
        .map_async(wgpu::MapMode::Read, move |result| tx.send(result).unwrap());
    cd.poll(wgpu::PollType::Wait).unwrap();
    rx.recv().unwrap().unwrap();
    {
        let mapping = output.slice(..).get_mapped_range();
        let actual: &[u32] = bytemuck::cast_slice(&mapping);
        for (i, &value) in actual.iter().enumerate() {
            let expected = if (i / words) % 17 == 0 { 0 } else { input[i] };
            assert_eq!(
                value,
                expected,
                "round {round}, frame {}, word {}",
                i / words,
                i % words
            );
        }
    }
    output.unmap();
    pd.poll(wgpu::PollType::Wait).unwrap();
    // No retained bind groups/command buffers exist. Drain both queues, destroy
    // both handles and run core cleanup before freeing their shared allocation.
    for (a, b, _) in &slots {
        a.destroy();
        b.destroy();
    }
    pd.poll(wgpu::PollType::Wait).unwrap();
    cd.poll(wgpu::PollType::Wait).unwrap();
    unsafe {
        for (_, _, memory) in slots {
            raw.free_memory(memory, None);
        }
        raw.destroy_semaphore(ready, None);
        raw.destroy_semaphore(done, None);
    }
    eprintln!(
        "round {round}: {FRAMES} publications, {bytes} bytes/slot, exact frame contents verified"
    );
}
