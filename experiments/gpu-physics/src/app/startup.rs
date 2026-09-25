use super::*;
use std::sync::{mpsc, atomic::{AtomicU32, Ordering}};
use std::time::Instant;

type DeviceResult = Result<(GpuDevice, wgpu::Surface<'static>), String>;
struct PreparedWorld { world: Option<WorldId>, scene_ms: f64, gpu_ms: f64 }
impl Drop for PreparedWorld {
    fn drop(&mut self) { if let Some(world)=self.world.take() { b3_destroy_world(world); } }
}
struct Graphics { gpu: GpuDevice, render_gpu: GpuDevice, renderer: Renderer }
pub(super) struct Startup {
    window: Arc<Window>, config: DemoConfig, sleep: bool, started: Instant,
    device_rx: Option<mpsc::Receiver<DeviceResult>>, world_rx: Option<mpsc::Receiver<Result<PreparedWorld,String>>>,
    graphics: Option<Graphics>, phase: Arc<AtomicU32>, device_ms: f64,
    first_loading_ms: Option<f64>, loading_frames: u32, last_draw: Instant,
}
pub(super) struct StartupMetrics {
    started: Instant, device_ms:f64, scene_ms:f64, gpu_ms:f64,
    first_loading_ms:Option<f64>, loading_frames:u32, count:u32,
}
impl StartupMetrics {
    pub(super) fn presented(self, framebuffer: (u32,u32)) {
        let total=self.started.elapsed().as_secs_f64()*1000.;
        eprintln!("startup: scene={:.3} ms gpu={:.3} ms first-scene-frame={:.3} ms loading-frames={}",self.scene_ms,self.gpu_ms,total,self.loading_frames);
        if let Some(path)=std::env::var_os("GPU_PHYSICS_STARTUP_REPORT") {
            let first=self.first_loading_ms.map(|v|v.to_string()).unwrap_or_else(||"null".into());
            let json=format!("{{\"schema\":\"startup-v1\",\"mode\":\"direct-renderer\",\"dynamic_cubes\":{},\"device_ms\":{},\"scene_ms\":{},\"gpu_prepare_ms\":{},\"first_frame_ms\":{},\"first_loading_frame_ms\":{},\"loading_frames\":{},\"framebuffer\":[{},{}]}}\n",
                self.count,self.device_ms,self.scene_ms,self.gpu_ms,total,first,self.loading_frames,framebuffer.0,framebuffer.1);
            if let Err(e)=std::fs::write(path,json) { eprintln!("startup report: {e}"); }
        }
    }
}
impl Startup {
    pub(super) fn new(event_loop:&ActiveEventLoop,config:DemoConfig,sleep:bool)->Self {
        let started=Instant::now();
        let attrs=Window::default_attributes().with_title("Preparing simulation: starting graphics device")
            .with_inner_size(winit::dpi::LogicalSize::new(
                std::env::var("GPU_BENCH_WIDTH").ok().and_then(|v|v.parse::<f64>().ok()).unwrap_or(1280.),
                std::env::var("GPU_BENCH_HEIGHT").ok().and_then(|v|v.parse::<f64>().ok()).unwrap_or(720.)));
        let window=Arc::new(event_loop.create_window(attrs).expect("window"));
        let instance=GpuDevice::instance_new();
        let surface=window_surface(&instance,window.clone()).expect("surface");
        let (tx,rx)=mpsc::channel();
        std::thread::spawn(move || {
            let result=block_on(viewer_device(instance,&surface)).map(|gpu|(gpu,surface));
            let _=tx.send(result);
        });
        Self {window,config,sleep,started,device_rx:Some(rx),world_rx:None,graphics:None,
            phase:Arc::new(AtomicU32::new(0)),device_ms:0.,first_loading_ms:None,loading_frames:0,last_draw:started}
    }
    pub(super) fn restart(old: Running, config: DemoConfig, sleep: bool) -> Self {
        let started=Instant::now();
        let Running{window,gpu,render_gpu,renderer,world,..}=old;
        window.set_title("Preparing simulation: restarting scene");
        // Release the previous surface before configuring its replacement.
        drop(renderer); drop(render_gpu);
        let surface=window_surface(&gpu.instance,window.clone()).expect("replacement surface");
        let (tx,rx)=mpsc::channel();
        std::thread::spawn(move || {
            b3_destroy_world(world);
            let _=tx.send(Ok((gpu,surface)));
        });
        Self {window,config,sleep,started,device_rx:Some(rx),world_rx:None,graphics:None,
            phase:Arc::new(AtomicU32::new(0)),device_ms:0.,first_loading_ms:None,loading_frames:0,last_draw:started}
    }
    pub(super) fn poll(&mut self)->Result<Option<Running>,String> {
        if let Some(rx)=&self.device_rx {
            match rx.try_recv() {
                Ok(result)=>{
                    self.device_rx=None;
                    let (gpu,surface)=result?;
                    self.device_ms=self.started.elapsed().as_secs_f64()*1000.;
                    let render_gpu=render_device(&gpu,&surface,&self.config);
                    let size=self.window.inner_size();
                    let mut renderer=Renderer::new(&render_gpu.device,&render_gpu.adapter,surface,(size.width,size.height));
                    #[cfg(feature="native-command-cache")]
                    if gpu.secondary_queue.is_some() { renderer.set_render_bridge(crate::native_async::RenderBridge::new(&gpu,&render_gpu)); }
                    if split_scene_enabled(&self.config) { renderer.set_staged_source(gpu.clone()); }
                    self.graphics=Some(Graphics{gpu:gpu.clone(),render_gpu,renderer});
                    // Present a real loading frame before starting expensive world work.
                    self.draw();
                    let (tx,rx)=mpsc::channel();self.world_rx=Some(rx);
                    let cfg=self.config;let sleep=self.sleep;let phase=self.phase.clone();
                    std::thread::spawn(move || {
                        let begin=Instant::now();
                        let world=build_demo_world(gpu,&cfg);
                        let scene_ms=begin.elapsed().as_secs_f64()*1000.;
                        let mut ready=PreparedWorld{world:Some(world),scene_ms,gpu_ms:0.};
                        phase.store(1,Ordering::Release);
                        let begin=Instant::now();
                        crate::api::b3_world_enable_sleeping(world,sleep);
                        crate::api::b3_world_ensure_gpu(world);
                        crate::api::b3_world_prepare_collision(world);
                        crate::api::b3_world_gpu_wait(world);
                        ready.gpu_ms=begin.elapsed().as_secs_f64()*1000.;
                        let failure=crate::api::b3_world_gpu_fail(world);
                        let result=if failure.is_null() {Ok(ready)} else {
                            Err(unsafe{std::ffi::CStr::from_ptr(failure)}.to_string_lossy().into_owned())
                        };
                        // A closed window drops the receiver; PreparedWorld then retires the world.
                        let _=tx.send(result);
                    });
                },
                Err(mpsc::TryRecvError::Empty)=>{},
                Err(mpsc::TryRecvError::Disconnected)=>return Err("graphics initialization worker stopped".into()),
            }
        }
        if let Some(rx)=&self.world_rx {
            match rx.try_recv() {
                Ok(result)=>{
                    let mut ready=result?;
                    let mut graphics=self.graphics.take().unwrap();
                    let world=ready.world.unwrap();
                    graphics.renderer.configure_reference(&graphics.render_gpu.device,&self.config,world,self.sleep);
                    let size=self.window.inner_size();
                    eprintln!("matched-window-start: {}x{}",size.width,size.height);
                    return Ok(Some(Running{window:self.window.clone(),gpu:graphics.gpu,render_gpu:graphics.render_gpu,
                        renderer:graphics.renderer,world:ready.world.take().unwrap(),last_ms:0.,
                        startup:Some(StartupMetrics{started:self.started,device_ms:self.device_ms,scene_ms:ready.scene_ms,
                            gpu_ms:ready.gpu_ms,first_loading_ms:self.first_loading_ms,loading_frames:self.loading_frames,count:self.config.body_count})}));
                },
                Err(mpsc::TryRecvError::Empty)=>{},
                Err(mpsc::TryRecvError::Disconnected)=>return Err("scene initialization worker stopped".into()),
            }
        }
        if self.last_draw.elapsed() >= std::time::Duration::from_millis(16) { self.window.request_redraw(); }
        Ok(None)
    }
    fn draw(&mut self) {
        self.last_draw=Instant::now();
        let message=if self.phase.load(Ordering::Acquire)==0 {"CREATING SCENE"} else {"PREPARING GPU"};
        self.window.set_title(&format!("Preparing simulation: {message} ({:.1}s)",self.started.elapsed().as_secs_f32()));
        let size=self.window.inner_size();if size.width==0 || size.height==0 {return;}
        if let Some(g)=&mut self.graphics {
            match g.renderer.loading_frame(&g.render_gpu.device,&g.render_gpu.queue,self.started.elapsed().as_secs_f32(),message) {
                Ok(())=>{self.loading_frames+=1;self.first_loading_ms.get_or_insert(self.started.elapsed().as_secs_f64()*1000.);},
                Err(wgpu::SurfaceError::Lost|wgpu::SurfaceError::Outdated)=>g.renderer.resize(&g.render_gpu.device,&g.render_gpu.queue,(size.width,size.height)),
                Err(e)=>log::warn!("loading surface: {e}"),
            }
        }
    }
    pub(super) fn event(&mut self,event_loop:&ActiveEventLoop,event:&WindowEvent) {
        match event {
            WindowEvent::CloseRequested=>event_loop.exit(),
            WindowEvent::KeyboardInput{event:KeyEvent{physical_key:PhysicalKey::Code(KeyCode::Escape),..},..}=>event_loop.exit(),
            WindowEvent::Resized(size)=>if let Some(g)=&mut self.graphics {
                g.renderer.resize(&g.render_gpu.device,&g.render_gpu.queue,(size.width,size.height));
            },
            WindowEvent::RedrawRequested=>self.draw(), _=>{},
        }
    }
}
