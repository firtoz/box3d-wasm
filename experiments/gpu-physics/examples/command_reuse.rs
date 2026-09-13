//! Isolated encode/replay feasibility, not physics or a CPU-engine benchmark.
use ash::vk;
use std::{
    io::Write,
    sync::atomic::{AtomicBool, Ordering},
    time::Instant,
};
use wgpu::{hal::api::Vulkan, util::DeviceExt};
const DISPATCHES: u32 = 96;
const STEPS: u32 = 150;
const COMPUTE: &str = r#"@group(0) @binding(0) var<storage,read_write> result:array<u32>;
@compute @workgroup_size(64) fn main(@builtin(local_invocation_index) lane:u32) {
 result[lane] = result[lane] + lane + 1u;
}"#;
include!("command_cache_raw.rs.inc");
static ERROR: AtomicBool = AtomicBool::new(false);
fn summary(mut a: Vec<f64>) -> String {
    a.sort_by(f64::total_cmp);
    format!(
        "{{\"p50_ms\":{},\"p95_ms\":{}}}",
        a[a.len() / 2],
        a[a.len() * 95 / 100]
    )
}
fn check(a: &[u32]) {
    for (i, &v) in a.iter().enumerate() {
        assert_eq!(v, (i as u32 + 1) * DISPATCHES * STEPS);
    }
}
fn main() {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info"))
        .format(|b, r| {
            if r.level() == log::Level::Error {
                ERROR.store(true, Ordering::Relaxed);
            }
            writeln!(b, "{} {}: {}", r.level(), r.target(), r.args())
        })
        .init();
    pollster::block_on(run());
    assert!(!ERROR.load(Ordering::Relaxed), "validation errors");
}
async fn run() {
    let instance = wgpu::Instance::new(&wgpu::InstanceDescriptor {
        backends: wgpu::Backends::VULKAN,
        flags: if std::env::var_os("QUEUE_PROBE_VALIDATE").is_some() {
            wgpu::InstanceFlags::debugging()
        } else {
            wgpu::InstanceFlags::empty()
        },
        ..Default::default()
    });
    let (adapter, _) = gpu_physics::adapter::pick_adapter(&instance, None)
        .await
        .unwrap();
    let (device, queue) = adapter
        .request_device(&wgpu::DeviceDescriptor::default())
        .await
        .unwrap();
    let family = unsafe { device.as_hal::<Vulkan>().unwrap().queue_family_index() };
    let mut rows = Vec::new();
    for mode in ["raw-record", "raw-reuse"] {
        let c = unsafe { RawCompute::new(&device, &queue, family) };
        let mut timeline =
            vk::SemaphoreTypeCreateInfo::default().semaphore_type(vk::SemaphoreType::TIMELINE);
        let sem = unsafe {
            c.raw
                .create_semaphore(
                    &vk::SemaphoreCreateInfo::default().push_next(&mut timeline),
                    None,
                )
                .unwrap()
        };
        let mut encode = Vec::new();
        let mut completed = Vec::new();
        for step in 0..STEPS {
            let start = Instant::now();
            unsafe {
                if mode == "raw-record" {
                    c.record();
                }
                let enc = start.elapsed().as_secs_f64() * 1000.;
                c.submit(sem, step as u64 + 1);
                c.finish();
                if step >= 30 {
                    encode.push(enc);
                    completed.push(start.elapsed().as_secs_f64() * 1000.);
                }
            }
        }
        check(&unsafe { c.values() });
        unsafe {
            c.raw.destroy_semaphore(sem, None);
        }
        rows.push(format!(
            "{{\"mode\":\"{mode}\",\"encode\":{},\"completed\":{}}}",
            summary(encode),
            summary(completed)
        ));
    }
    let buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: None,
        contents: &[0; 256],
        usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC,
    });
    let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
        label: None,
        source: wgpu::ShaderSource::Wgsl(COMPUTE.into()),
    });
    let pipeline = device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
        label: None,
        layout: None,
        module: &shader,
        entry_point: Some("main"),
        compilation_options: Default::default(),
        cache: None,
    });
    let bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: None,
        layout: &pipeline.get_bind_group_layout(0),
        entries: &[wgpu::BindGroupEntry {
            binding: 0,
            resource: buffer.as_entire_binding(),
        }],
    });
    let mut encode = Vec::new();
    let mut completed = Vec::new();
    for step in 0..STEPS {
        let start = Instant::now();
        let mut enc = device.create_command_encoder(&Default::default());
        {
            let mut pass = enc.begin_compute_pass(&Default::default());
            pass.set_pipeline(&pipeline);
            pass.set_bind_group(0, &bg, &[]);
            for _ in 0..DISPATCHES {
                pass.dispatch_workgroups(1, 1, 1);
            }
        }
        let command = enc.finish();
        let time = start.elapsed().as_secs_f64() * 1000.;
        queue.submit([command]);
        device.poll(wgpu::PollType::Wait).unwrap();
        if step >= 30 {
            encode.push(time);
            completed.push(start.elapsed().as_secs_f64() * 1000.);
        }
    }
    let read = device.create_buffer(&wgpu::BufferDescriptor {
        label: None,
        size: 256,
        usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
        mapped_at_creation: false,
    });
    let mut enc = device.create_command_encoder(&Default::default());
    enc.copy_buffer_to_buffer(&buffer, 0, &read, 0, 256);
    queue.submit([enc.finish()]);
    let (tx, rx) = std::sync::mpsc::channel();
    read.slice(..)
        .map_async(wgpu::MapMode::Read, move |r| tx.send(r).unwrap());
    device.poll(wgpu::PollType::Wait).unwrap();
    rx.recv().unwrap().unwrap();
    check(bytemuck::cast_slice(&read.slice(..).get_mapped_range()));
    read.unmap();
    rows.push(format!(
        "{{\"mode\":\"wgpu-record\",\"encode\":{},\"completed\":{}}}",
        summary(encode),
        summary(completed)
    ));
    println!("{{\"status\":\"pass\",\"dispatches\":{DISPATCHES},\"steps\":{STEPS},\"rows\":[{}],\"physics_benchmark\":false,\"cpu_win_validated\":false}}",rows.join(","));
}
