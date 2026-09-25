mod startup;
use startup::{Startup, StartupMetrics};
use std::future::Future;
use std::sync::Arc;

use winit::application::ApplicationHandler;
use winit::event::{KeyEvent, WindowEvent};
use winit::event_loop::{ActiveEventLoop, ControlFlow, EventLoop};
use winit::keyboard::{KeyCode, PhysicalKey};
use winit::window::{Window, WindowId};

use crate::api::{b3_destroy_world, b3_world_counts, b3_world_gpu_report_name, WorldId};
use crate::render::{window_surface, Renderer};
use crate::scenes::build_demo_world;
use crate::sim::GpuDevice;
use crate::types::{DemoConfig, DemoScene};

async fn viewer_device(instance: wgpu::Instance, surface: &wgpu::Surface<'_>) -> Result<GpuDevice, String> {
    let secondary = std::env::var("GPU_PHYSICS_RENDER_QUEUE").as_deref() == Ok("1");
    #[cfg(not(feature = "native-command-cache"))]
    if secondary { return Err("secondary rendering requires native backend".into()); }
    GpuDevice::from_instance_mode(instance, if std::env::var("GPU_PHYSICS_SPLIT_ADAPTER").as_deref()==Ok("1") {None} else {Some(surface)}, secondary).await
}
fn split_scene_enabled(config:&DemoConfig)->bool {
    std::env::var("GPU_PHYSICS_SPLIT_ADAPTER").as_deref()==Ok("1")
        && config.contacts && !config.jacobi
        && matches!(config.scene,DemoScene::FallingCubes|DemoScene::MixedStacks|DemoScene::Dominoes)
}
fn render_device(gpu: &GpuDevice, surface: &wgpu::Surface<'_>,config:&DemoConfig) -> GpuDevice {
    if split_scene_enabled(config) {
        let render=block_on(GpuDevice::from_instance(gpu.instance.clone(),Some(surface))).expect("display-local renderer");
        assert_eq!(gpu.report.vendor,0x10de); assert_eq!(render.report.vendor,0x1002);
        eprintln!("split-adapter: physics={} render={}",gpu.report.name,render.report.name);
        return render;
    }
    let mut render = gpu.clone();
    #[cfg(feature = "native-command-cache")]
    if let Some(queue) = &gpu.secondary_queue {
        let (device, queue) = queue.create_tracked_device(&gpu.adapter).expect("tracked rendering device");
        render.device = device; render.queue = queue;
    }
    render
}

struct Running {
    window: Arc<Window>,
    gpu: GpuDevice,
    render_gpu: GpuDevice,
    world: WorldId,
    renderer: Renderer,
    last_ms: f32,
    startup: Option<StartupMetrics>,
}

pub struct DemoApp {
    lifecycle_stage:u32,lifecycle_frames:u32,
    config: DemoConfig,
    sleep: bool,
    running: Option<Running>,
    startup: Option<Startup>,
}

impl DemoApp {
    pub fn new(config: DemoConfig, sleep: bool) -> Self {
        Self {
            lifecycle_stage:0,lifecycle_frames:0,
            config,
            sleep,
            running: None,
            startup: None,
        }
    }

    fn check_lifecycle_frame(&mut self,event_loop:&ActiveEventLoop) {
        if std::env::var("GPU_PHYSICS_OVERLAP_LIFECYCLE").as_deref()!=Ok("1") {return;}
        assert!(std::env::var_os("GPU_PHYSICS_CPU_REFERENCE").is_none());
        self.lifecycle_frames+=1;
        if self.lifecycle_frames<8 {return;}
        let running=self.running.as_mut().unwrap();
        let step=running.renderer.physics_step(running.world);
        let expected_step=if self.lifecycle_stage==2 {16}else{8};
        assert_eq!(step,expected_step);
        let eligible=split_scene_enabled(&self.config);
        assert_eq!(running.render_gpu.report.vendor,if eligible {0x1002}else{0x10de});
        if eligible {assert_eq!(running.renderer.overlap_progress(),Some((step,step-1,1)));}
        else {assert!(running.renderer.overlap_progress().is_none());}
        eprintln!("overlap-lifecycle-stage: stage={} scene={} step={} eligible={}",self.lifecycle_stage,self.config.scene.slug(),step,eligible);
        match self.lifecycle_stage {
            0=>self.set_scene(DemoScene::MixedStacks),
            1=>running.renderer.resize(&running.render_gpu.device,&running.render_gpu.queue,(960,540)),
            2=>self.set_scene(DemoScene::BoxStack),
            3=>self.set_scene(DemoScene::MixedStacks),
            4=>{
                running.renderer.drain_overlap(&running.render_gpu.device,&running.render_gpu.queue,running.world).expect("drain");
                assert_eq!(running.renderer.overlap_progress(),Some((8,8,0)));
                eprintln!("overlap-lifecycle: pass restart resize eligible-ineligible-eligible final-drain");event_loop.exit();
            },
            _=>unreachable!(),
        }
        self.lifecycle_stage+=1;self.lifecycle_frames=0;
    }
    fn set_scene(&mut self, scene: DemoScene) {
        self.config.scene = scene;
        let Some(old)=self.running.take() else {return;};
        self.startup=Some(Startup::restart(old,self.config,self.sleep));
        eprintln!(
            "scene {} ({}/{})",
            scene.slug(),
            scene_ordinal(scene),
            DemoScene::ALL.len()
        );
    }
}

fn scene_ordinal(scene: DemoScene) -> usize {
    DemoScene::ALL.iter().position(|s| *s == scene).unwrap_or(0) + 1
}

fn window_title(cfg: &DemoConfig, gpu_name: &str, bodies: u32, ms: f32) -> String {
    format!(
        "gpu-physics | {}/{} {} | {} | {bodies} bodies | contacts={} | viewer {ms:.2} ms | arrows orbit  wheel zoom  [ ] cycle  R restart",
        scene_ordinal(cfg.scene),
        DemoScene::ALL.len(),
        cfg.scene.slug(),
        gpu_name,
        cfg.contacts
    )
}

impl ApplicationHandler for DemoApp {
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {
        if self.running.is_none() && self.startup.is_none() {
            self.startup=Some(Startup::new(event_loop,self.config,self.sleep));
        }
    }

    fn window_event(&mut self, event_loop: &ActiveEventLoop, _id: WindowId, event: WindowEvent) {
        if let Some(startup)=&mut self.startup { startup.event(event_loop,&event);return; }
        let Some(running) = self.running.as_mut() else { return; };
        match event {
            WindowEvent::CloseRequested => event_loop.exit(),
            WindowEvent::KeyboardInput {
                event:
                    KeyEvent {
                        physical_key: PhysicalKey::Code(code),
                        state: winit::event::ElementState::Pressed,
                        repeat: false,
                        ..
                    },
                ..
            } => match code {
                KeyCode::Escape => event_loop.exit(),
                KeyCode::ArrowLeft => running.renderer.orbit(-0.08, 0.0),
                KeyCode::ArrowRight => running.renderer.orbit(0.08, 0.0),
                KeyCode::ArrowUp => running.renderer.orbit(0.0, 0.06),
                KeyCode::ArrowDown => running.renderer.orbit(0.0, -0.06),
                KeyCode::BracketRight => {
                    let next = self.config.scene.next();
                    self.set_scene(next);
                }
                KeyCode::BracketLeft => {
                    let prev = self.config.scene.prev();
                    self.set_scene(prev);
                }
                KeyCode::KeyR => {
                    let scene = self.config.scene;
                    self.set_scene(scene);
                }
                _ => {}
            },
            WindowEvent::MouseWheel { delta, .. } => {
                let amount = match delta { winit::event::MouseScrollDelta::LineDelta(_,y) => y,
                    winit::event::MouseScrollDelta::PixelDelta(p) => p.y as f32 / 40.0 };
                running.renderer.zoom(amount);
            }
            WindowEvent::Resized(size) => {
                running.renderer.resize(
                    &running.render_gpu.device,
                    &running.render_gpu.queue,
                    (size.width, size.height),
                );
            }
            WindowEvent::RedrawRequested => {
                match running
                    .renderer
                    .frame(&running.render_gpu.device, &running.render_gpu.queue, running.world)
                {
                    Ok(timings) => {
                        if let Some(startup)=running.startup.take() {
                            startup.presented((running.window.inner_size().width,running.window.inner_size().height));
                            if std::env::var("GPU_PHYSICS_STARTUP_EXIT").as_deref()==Ok("1") {
                                event_loop.exit(); return;
                            }
                        }
                        running.last_ms = timings.total_ms;
                        running.window.set_title(&window_title(
                            &self.config,
                            &b3_world_gpu_report_name(running.world),
                            b3_world_counts(running.world).0.max(0) as u32,
                            timings.total_ms,
                        ));
                        self.check_lifecycle_frame(event_loop);
                    }
                    Err(wgpu::SurfaceError::Lost | wgpu::SurfaceError::Outdated) => {
                        running.renderer.resize(
                            &running.render_gpu.device,
                            &running.render_gpu.queue,
                            (
                                running.window.inner_size().width,
                                running.window.inner_size().height,
                            ),
                        );
                    }
                    Err(wgpu::SurfaceError::OutOfMemory) => event_loop.exit(),
                    Err(e) => log::warn!("surface: {e}"),
                }
            }
            _ => {}
        }
    }

    fn about_to_wait(&mut self, event_loop: &ActiveEventLoop) {
        if let Some(startup)=&mut self.startup {
            event_loop.set_control_flow(ControlFlow::WaitUntil(std::time::Instant::now()+std::time::Duration::from_millis(16)));
            match startup.poll() {
                Ok(Some(running))=>{self.running=Some(running);self.startup=None;event_loop.set_control_flow(ControlFlow::Poll);},
                Ok(None)=>{},
                Err(error)=>{eprintln!("startup: {error}");event_loop.exit();},
            }
        }
        if let Some(running) = &self.running {
            running.window.request_redraw();
        }
    }
}

impl Drop for DemoApp {
    fn drop(&mut self) {
        if let Some(running) = self.running.take() {
            b3_destroy_world(running.world);
        }
    }
}

pub fn run_window(config: DemoConfig, sleep: bool) -> Result<(), String> {
    let event_loop = EventLoop::new().map_err(|e| e.to_string())?;
    event_loop.set_control_flow(ControlFlow::Poll);
    let mut app = DemoApp::new(config, sleep);
    event_loop.run_app(&mut app).map_err(|e| e.to_string())
}

struct TimelineApp {
    config: DemoConfig,
    sleep: bool,
    warmup: u32,
    timed: u32,
    remaining_warmup: u32,
    remaining_timed: u32,
    path: std::path::PathBuf,
    running: Option<Running>,
    startup: Option<Startup>,
    physics_ms: Vec<f32>,
    draw_ms: Vec<f32>,
    present_ms: Vec<f32>,
    total_ms: Vec<f32>,
    last_step: u64,
    zero_solver_frames: u32,
    last_completed: Option<std::time::Instant>,
    cadence_ms: Vec<f32>,
}

impl ApplicationHandler for TimelineApp {
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {
        if self.running.is_none() && self.startup.is_none() {
            self.startup=Some(Startup::new(event_loop,self.config,self.sleep));
        }
    }

    fn window_event(&mut self, event_loop: &ActiveEventLoop, _id: WindowId, event: WindowEvent) {
        if let Some(startup)=&mut self.startup { startup.event(event_loop,&event);return; }
        let Some(running) = self.running.as_mut() else { return; };
        match event {
            WindowEvent::CloseRequested => event_loop.exit(),
            WindowEvent::Resized(size) => {
                running.renderer.resize(
                    &running.render_gpu.device,
                    &running.render_gpu.queue,
                    (size.width, size.height),
                );
            }
            WindowEvent::RedrawRequested => {
                match running
                    .renderer
                    .frame(&running.render_gpu.device, &running.render_gpu.queue, running.world)
                {
                    Ok(timings) => {
                        if let Some(startup)=running.startup.take() { startup.presented((running.window.inner_size().width,running.window.inner_size().height)); }
                        let completed = std::time::Instant::now();
                        let cadence = self.last_completed.replace(completed)
                            .map(|previous| completed.duration_since(previous).as_secs_f32()*1000.0);
                        if self.remaining_warmup > 0 {
                            self.remaining_warmup -= 1;
                        } else if self.remaining_timed > 0 {
                            self.cadence_ms.push(cadence.expect("diagnostic requires warmup"));
                            self.physics_ms.push(timings.physics_ms);
                            self.draw_ms.push(timings.draw_ms);
                            self.present_ms.push(timings.present_ms);
                            self.total_ms.push(timings.total_ms);
                            self.last_step = running.renderer.physics_step(running.world);
                            if !running.renderer.is_cpu() && crate::api::b3_world_last_solver_dispatches(running.world) == 0 {
                                self.zero_solver_frames += 1;
                            }
                            self.remaining_timed -= 1;
                            if self.remaining_timed == 0 {
                                if let Some(drain)=running.renderer.drain_overlap(&running.render_gpu.device,&running.render_gpu.queue,running.world) {
                                    // Include final drain work in the last measured cadence interval.
                                    *self.cadence_ms.last_mut().unwrap()+=drain;
                                    *self.total_ms.last_mut().unwrap()+=drain;
                                }
                                // Bound the measured window at completed device work, including
                                // queued final render/physics work; correctness reads happen later.
                                let drain_start = std::time::Instant::now();
                                running.render_gpu.device.poll(wgpu::PollType::wait()).expect("benchmark render drain");
                                if !running.renderer.is_cpu() { crate::api::b3_world_gpu_wait(running.world); }
                                let drain_ms = drain_start.elapsed().as_secs_f32() * 1000.0;
                                *self.cadence_ms.last_mut().unwrap() += drain_ms;
                                *self.total_ms.last_mut().unwrap() += drain_ms;
                                let size=running.window.inner_size();
                                eprintln!("matched-window-end: {}x{}",size.width,size.height);
                                eprintln!("pose-staging-kicks: {}",crate::api::b3_world_pose_snapshot_kicks(running.world));
                                event_loop.exit();
                            }
                        }
                    }
                    Err(wgpu::SurfaceError::Lost | wgpu::SurfaceError::Outdated) => {
                        running.renderer.resize(
                            &running.render_gpu.device,
                            &running.render_gpu.queue,
                            (
                                running.window.inner_size().width,
                                running.window.inner_size().height,
                            ),
                        );
                    }
                    Err(wgpu::SurfaceError::OutOfMemory) => event_loop.exit(),
                    Err(e) => log::warn!("surface: {e}"),
                }
            }
            _ => {}
        }
    }

    fn about_to_wait(&mut self, event_loop: &ActiveEventLoop) {
        if let Some(startup)=&mut self.startup {
            event_loop.set_control_flow(ControlFlow::WaitUntil(std::time::Instant::now()+std::time::Duration::from_millis(16)));
            match startup.poll() {
                Ok(Some(running))=>{self.running=Some(running);self.startup=None;event_loop.set_control_flow(ControlFlow::Poll);},
                Ok(None)=>{},
                Err(error)=>{eprintln!("startup: {error}");event_loop.exit();},
            }
        }
        if let Some(running) = &self.running {
            running.window.request_redraw();
        }
    }
}

impl Drop for TimelineApp {
    fn drop(&mut self) {
        if let Some(running) = self.running.take() {
            if self.config.scene == DemoScene::FallingCubes && !running.renderer.is_cpu() {
                crate::api::b3_world_gpu_wait(running.world);
                let stats = block_on(crate::api::b3_world_live_step_stats(running.world)).expect("benchmark GPU stats");
                assert!(!stats.capacity_loss(), "benchmark capacity loss: {}", stats.sticky.loss_detail());
                let bodies = block_on(crate::api::b3_world_sync_from_gpu(running.world));
                assert_eq!(bodies.len(), self.config.body_count as usize + 1);
                assert!(bodies.iter().all(|b| b.pos.iter().all(|v| v.is_finite()) && b.pos[1] >= -1.01), "invalid benchmark bodies");
                assert!(stats.narrowphase_pairs > 0, "benchmark has no contact pairs");
            }
            b3_destroy_world(running.world);
        }
        if self.total_ms.is_empty() {
            return;
        }
        fn p50(v: &[f32]) -> f32 {
            let mut s = v.to_vec();
            s.sort_by(|a, b| a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal));
            s[s.len() / 2]
        }
        fn p95(v: &[f32]) -> f32 {
            let mut s = v.to_vec();
            s.sort_by(|a, b| a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal));
            s[((s.len() as f32 * 0.95) as usize).min(s.len() - 1)]
        }
        assert_eq!(self.cadence_ms.len(), self.total_ms.len());
        let cadence_json = format!("{{\"diagnostic_only\":true,\"samples\":{},\"p50_ms\":{},\"p95_ms\":{},\"sum_ms\":{}}}",
            self.cadence_ms.len(), p50(&self.cadence_ms), p95(&self.cadence_ms), self.cadence_ms.iter().map(|v|f64::from(*v)).sum::<f64>());
        std::fs::write(self.path.with_extension("cadence.json"), cadence_json).expect("cadence output");
        let engine = if std::env::var_os("GPU_PHYSICS_CPU_REFERENCE").is_some() { "box3d-cpu" } else { "gpu" };
        let workers: u32 = if engine == "box3d-cpu" { std::env::var("GPU_PHYSICS_CPU_WORKERS").unwrap_or_else(|_| "4".into()).parse().unwrap() } else { 0 };
        let json = format!(
            "{{\n  \"scene\":\"{}\",\n  \"engine\":\"{}\",\n  \"cpu_workers\":{},\n  \"sleep\":{},\n  \"warmup\":{},\n  \"timed\":{},\n  \"physics_step\":{},\n  \"zero_solver_frames\":{},\n  \"cpu_win_validated\":false,\n  \"frame_total\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"physics_submit\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"draw_encode\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"present\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"notes\":\"present is wgpu Surface::present elapsed on this thread; vsync wait may be inside present or the next get_current_texture. Do not subtract aggregate medians.\"\n}}\n",
            self.config.scene.slug(),
            engine,
            workers,
            self.sleep,
            self.warmup,
            self.timed,
            self.last_step,
            self.zero_solver_frames,
            p50(&self.total_ms),
            p95(&self.total_ms),
            p50(&self.physics_ms),
            p95(&self.physics_ms),
            p50(&self.draw_ms),
            p95(&self.draw_ms),
            p50(&self.present_ms),
            p95(&self.present_ms),
        );
        if let Some(parent) = self.path.parent() {
            let _ = std::fs::create_dir_all(parent);
        }
        if let Err(err) = std::fs::write(&self.path, json) {
            eprintln!("native-timeline write {}: {err}", self.path.display());
        }
    }
}

pub fn run_native_timeline(
    config: DemoConfig,
    sleep: bool,
    warmup: u32,
    frames: u32,
    _runs: u32,
    path: std::path::PathBuf,
) -> Result<(), String> {
    let timed = frames.max(1);
    let event_loop = EventLoop::new().map_err(|e| e.to_string())?;
    event_loop.set_control_flow(ControlFlow::Poll);
    let mut app = TimelineApp {
        config,
        sleep,
        warmup,
        timed,
        remaining_warmup: warmup,
        remaining_timed: timed,
        path,
        running: None,
        startup: None,
        physics_ms: Vec::new(),
        draw_ms: Vec::new(),
        present_ms: Vec::new(),
        total_ms: Vec::new(),
        last_step: 0,
        zero_solver_frames: 0,
        last_completed: None,
        cadence_ms: Vec::new(),
    };
    event_loop.run_app(&mut app).map_err(|e| e.to_string())
}

fn block_on<T>(f: impl Future<Output = T>) -> T {
    pollster::block_on(f)
}
