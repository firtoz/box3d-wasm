#[cfg(not(target_arch = "wasm32"))]
use std::sync::Arc;
use std::time::Instant;

use wgpu::util::DeviceExt;
use wgpu::{BindGroup, Buffer, Device, Queue, Surface, SurfaceConfiguration, TextureView};

use crate::api::{b3_world_finalize_render_state, b3_world_render_buffers, b3_world_step_gpu, WorldId};
use crate::mesh::{
    self, vertex_layout,
};
use crate::types::{DEFAULT_SUB_STEPS, FIXED_DT, GROUND_Y};

#[repr(C)]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
struct CameraUniform {
    view_proj: [[f32; 4]; 4],
    _pad: [u32; 48],
}

fn depth_format() -> wgpu::TextureFormat {
    wgpu::TextureFormat::Depth32Float
}

fn use_depth() -> bool {
    true
}

#[derive(Clone, Copy)]
struct OrbitCamera { target: glam::Vec3, radius: f32, yaw: f32, pitch: f32 }
impl Default for OrbitCamera {
    fn default() -> Self { Self { target: glam::Vec3::ZERO, radius: 65.0, yaw: 0.7, pitch: 0.65 } }
}
impl OrbitCamera {
    fn uniform(self, aspect: f32) -> CameraUniform {
        let direction = glam::Vec3::new(self.yaw.sin()*self.pitch.cos(), self.pitch.sin(), self.yaw.cos()*self.pitch.cos());
        let projection = glam::Mat4::perspective_rh(45_f32.to_radians(), aspect.max(0.01), 0.05, (self.radius*8.0).max(500.0));
        CameraUniform { view_proj: (projection * glam::Mat4::look_at_rh(self.target + self.radius*direction, self.target, glam::Vec3::Y)).to_cols_array_2d(), _pad:[0;48] }
    }
    fn fit(&mut self, lower: [f32;3], upper: [f32;3], aspect: f32) {
        let lower=glam::Vec3::from_array(lower);let upper=glam::Vec3::from_array(upper);
        self.target=(lower+upper)*0.5;
        let direction=glam::Vec3::new(self.yaw.sin()*self.pitch.cos(),self.pitch.sin(),self.yaw.cos()*self.pitch.cos());
        let right=glam::Vec3::Y.cross(direction).normalize();let up=direction.cross(right);
        let ty=22.5_f32.to_radians().tan();let tx=ty*aspect.max(0.01);
        let mut distance=1.0f32;
        for x in [lower.x,upper.x] {for y in [lower.y,upper.y] {for z in [lower.z,upper.z] {
            let p=glam::Vec3::new(x,y,z)-self.target;
            distance=distance.max(p.dot(direction)+(p.dot(right).abs()/tx).max(p.dot(up).abs()/ty));
        }}}
        self.radius=(distance*1.1).max(8.0);
    }
}

pub struct FrameTimings {
    pub total_ms: f32,
    pub physics_ms: f32,
    pub draw_ms: f32,
    pub present_ms: f32,
}

pub struct Renderer {
    loading_screen: Option<crate::loading_screen::LoadingScreen>,
    overlap:Option<OverlapState>,overlap_requested:bool,drain_only:bool,
    staged_source: Option<crate::sim::GpuDevice>,
    staged_buffers: Option<StagedBuffers>,
    #[cfg(feature = "native-command-cache")]
    render_bridge: Option<crate::native_async::RenderBridge>,
    #[cfg(feature = "native-command-cache")]
    late_acquire: bool,
    poll_idle_status: bool,
    #[cfg(target_os = "linux")]
    cpu: Option<crate::cpu_viewer::CpuViewer>,
    scene_draw: crate::scene_draw::SceneDraw,
    camera: OrbitCamera,
    camera_world: Option<WorldId>,
    surface: Surface<'static>,
    config: SurfaceConfiguration,
    camera_buf: Buffer,
    ground_vb: Buffer,
    ground_ib: Buffer,
    ground_index_count: u32,
    ground_pipeline: wgpu::RenderPipeline,
    camera_bg: BindGroup,
    shadow_view: TextureView,
    shadow_camera: Buffer,
    shadow_camera_bg: BindGroup,
    shadow_bg: BindGroup,
    depth_view: Option<TextureView>,
    depth_size: (u32, u32),
}

impl Renderer {
    #[cfg(feature = "native-command-cache")]
    pub(crate) fn set_render_bridge(&mut self, bridge: crate::native_async::RenderBridge) { self.render_bridge = Some(bridge); }
    pub fn new(
        device: &Device,
        adapter: &wgpu::Adapter,
        surface: Surface<'static>,
        size: (u32, u32),
    ) -> Self {
        let width = size.0.max(1);
        let height = size.1.max(1);
        let mut config = surface
            .get_default_config(adapter, width, height)
            .expect("no default surface config");
        if let Ok(mode)=std::env::var("GPU_PHYSICS_PRESENT_MODE") {
            config.present_mode=match mode.as_str() {
                "immediate"=>wgpu::PresentMode::Immediate,
                "fifo"=>wgpu::PresentMode::Fifo,
                _=>panic!("GPU_PHYSICS_PRESENT_MODE must be immediate or fifo"),
            };
            assert!(surface.get_capabilities(adapter).present_modes.contains(&config.present_mode),
                "requested GPU presentation mode is unsupported");
        }
        eprintln!("GPU presentation mode: {:?}, maximum frame latency: {}",
            config.present_mode,config.desired_maximum_frame_latency);
        surface.configure(device, &config);

        debug_assert_eq!(std::mem::size_of::<CameraUniform>(), 256);
        let vp = OrbitCamera::default().uniform(width as f32 / height as f32);
        let camera_buf = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("camera"),
            contents: bytemuck::bytes_of(&vp),
            usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
        });

        let ground = mesh::ground_quad(500.0, GROUND_Y);
        let (ground_vb, ground_ib, ground_index_count) =
            mesh::upload_mesh(device, &ground, "ground");

        let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("render"),
            source: wgpu::ShaderSource::Wgsl(include_str!("../shaders/window_ground.wgsl").into()),
        });

        let bgl = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: Some("render-bgl"),
            entries: &[wgpu::BindGroupLayoutEntry {
                binding: 0,
                visibility: wgpu::ShaderStages::VERTEX | wgpu::ShaderStages::FRAGMENT,
                ty: wgpu::BindingType::Buffer {
                    ty: wgpu::BufferBindingType::Uniform,
                    has_dynamic_offset: false,
                    min_binding_size: None,
                },
                count: None,
            }],
        });

        let shadow_view = device.create_texture(&wgpu::TextureDescriptor { label:Some("directional-shadow"),
            size:wgpu::Extent3d{width:2048,height:2048,depth_or_array_layers:1},mip_level_count:1,sample_count:1,
            dimension:wgpu::TextureDimension::D2,format:depth_format(),usage:wgpu::TextureUsages::RENDER_ATTACHMENT|wgpu::TextureUsages::TEXTURE_BINDING,view_formats:&[]
        }).create_view(&Default::default());
        let shadow_camera=device.create_buffer_init(&wgpu::util::BufferInitDescriptor{label:Some("shadow-camera"),contents:bytemuck::bytes_of(&vp),usage:wgpu::BufferUsages::UNIFORM|wgpu::BufferUsages::COPY_DST});
        let shadow_camera_bg=device.create_bind_group(&wgpu::BindGroupDescriptor{label:Some("shadow-camera"),layout:&bgl,entries:&[wgpu::BindGroupEntry{binding:0,resource:shadow_camera.as_entire_binding()}]});
        let shadow_layout=device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor{label:Some("shadow-sampling"),entries:&[
            wgpu::BindGroupLayoutEntry{binding:0,visibility:wgpu::ShaderStages::FRAGMENT,ty:wgpu::BindingType::Texture{sample_type:wgpu::TextureSampleType::Depth,view_dimension:wgpu::TextureViewDimension::D2,multisampled:false},count:None},
            wgpu::BindGroupLayoutEntry{binding:1,visibility:wgpu::ShaderStages::FRAGMENT,ty:wgpu::BindingType::Sampler(wgpu::SamplerBindingType::Comparison),count:None},
            wgpu::BindGroupLayoutEntry{binding:2,visibility:wgpu::ShaderStages::FRAGMENT,ty:wgpu::BindingType::Buffer{ty:wgpu::BufferBindingType::Uniform,has_dynamic_offset:false,min_binding_size:None},count:None}]});
        let shadow_sampler=device.create_sampler(&wgpu::SamplerDescriptor{compare:Some(wgpu::CompareFunction::LessEqual),mag_filter:wgpu::FilterMode::Linear,min_filter:wgpu::FilterMode::Linear,..Default::default()});
        let shadow_bg=device.create_bind_group(&wgpu::BindGroupDescriptor{label:Some("shadow-sampling"),layout:&shadow_layout,entries:&[
            wgpu::BindGroupEntry{binding:0,resource:wgpu::BindingResource::TextureView(&shadow_view)},wgpu::BindGroupEntry{binding:1,resource:wgpu::BindingResource::Sampler(&shadow_sampler)},wgpu::BindGroupEntry{binding:2,resource:shadow_camera.as_entire_binding()}]});
        let layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: Some("render-pl"),
            bind_group_layouts: &[&bgl, &shadow_layout],
            push_constant_ranges: &[],
        });

        let depth_stencil = if use_depth() {
            Some(wgpu::DepthStencilState {
                format: depth_format(),
                depth_write_enabled: true,
                depth_compare: wgpu::CompareFunction::Less,
                stencil: wgpu::StencilState::default(),
                bias: wgpu::DepthBiasState::default(),
            })
        } else {
            None
        };

        let color_target = [Some(wgpu::ColorTargetState {
            format: config.format,
            blend: Some(wgpu::BlendState::REPLACE),
            write_mask: wgpu::ColorWrites::ALL,
        })];

        let ground_pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("ground-pipeline"),
            layout: Some(&layout),
            vertex: wgpu::VertexState {
                module: &shader,
                entry_point: Some("vs_ground"),
                compilation_options: Default::default(),
                buffers: &[vertex_layout()],
            },
            fragment: Some(wgpu::FragmentState {
                module: &shader,
                entry_point: Some("fs_ground"),
                compilation_options: Default::default(),
                targets: &color_target,
            }),
            primitive: wgpu::PrimitiveState::default(),
            depth_stencil,
            multisample: wgpu::MultisampleState::default(),
            multiview: None,
            cache: None,
        });

        let camera_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: Some("render-bg"),
            layout: &bgl,
            entries: &[wgpu::BindGroupEntry {
                binding: 0,
                resource: camera_buf.as_entire_binding(),
            }],
        });
        let (depth_view, depth_size) = if use_depth() {
            let (view, sz) = make_depth(device, width, height);
            (Some(view), sz)
        } else {
            (None, (width, height))
        };

        let scene_draw = crate::scene_draw::SceneDraw::new(device, &bgl, config.format);
        Self {
            loading_screen: None,
            overlap:None,overlap_requested:std::env::var("GPU_PHYSICS_SPLIT_OVERLAP").as_deref()==Ok("1"),drain_only:false,
            staged_source:None, staged_buffers:None,
            #[cfg(feature = "native-command-cache")]
            render_bridge: None,
            #[cfg(feature = "native-command-cache")]
            late_acquire: std::env::var("GPU_PHYSICS_RENDER_LATE_ACQUIRE").as_deref() == Ok("1"),
            poll_idle_status: std::env::var("GPU_PHYSICS_IDLE").as_deref() != Ok("0"),
            #[cfg(target_os = "linux")]
            cpu: None,
            scene_draw,
            camera: OrbitCamera::default(),
            camera_world: None,
            surface,
            config,
            camera_buf,
            ground_vb,
            ground_ib,
            ground_index_count,
            ground_pipeline,
            camera_bg,
            shadow_view, shadow_camera, shadow_camera_bg, shadow_bg,
            depth_view,
            depth_size,
        }
    }

    pub(crate) fn loading_frame(&mut self, device: &Device, queue: &Queue, elapsed: f32, message: &str) -> Result<(), wgpu::SurfaceError> {
        let screen = self.loading_screen.get_or_insert_with(|| crate::loading_screen::LoadingScreen::new(device, self.config.format));
        let frame = self.surface.get_current_texture()?;
        let view = frame.texture.create_view(&Default::default());
        let mut encoder = device.create_command_encoder(&Default::default());
        screen.draw(device, &mut encoder, &view, (self.config.width,self.config.height), elapsed, message);
        queue.submit(Some(encoder.finish()));
        frame.present();
        Ok(())
    }

    pub fn resize(&mut self, device: &Device, queue: &Queue, size: (u32, u32)) {
        let width = size.0.max(1);
        let height = size.1.max(1);
        self.config.width = width;
        self.config.height = height;
        self.surface.configure(device, &self.config);
        upload_camera(
            device,
            queue,
            &self.camera_buf,
            self.camera.uniform(width as f32 / height as f32),
        );
        if use_depth() && self.depth_size != (width, height) {
            let (view, sz) = make_depth(device, width, height);
            self.depth_view = Some(view);
            self.depth_size = sz;
        }
    }

    pub fn orbit(&mut self, dx: f32, dy: f32) {
        self.camera.yaw += dx;
        self.camera.pitch = (self.camera.pitch + dy).clamp(0.05, 1.5);
    }
    pub fn zoom(&mut self, amount: f32) {
        self.camera.radius = (self.camera.radius * (-amount * 0.1).exp()).clamp(1.0, 10000.0);
    }

    pub fn configure_reference(&mut self, device: &Device, cfg: &crate::types::DemoConfig, world: WorldId, sleep: bool) {
        self.overlap=None;self.drain_only=false;
        self.overlap_requested=self.staged_source.is_some()
            && std::env::var("GPU_PHYSICS_SPLIT_OVERLAP").as_deref()==Ok("1")
            && cfg.contacts && !cfg.jacobi
            && matches!(cfg.scene,crate::types::DemoScene::FallingCubes|crate::types::DemoScene::MixedStacks|crate::types::DemoScene::Dominoes);
        crate::api::b3_world_enable_sleeping(world, sleep);
        #[cfg(target_os = "linux")]
        { self.cpu = crate::cpu_viewer::CpuViewer::from_env(device, cfg, world, sleep); }
        crate::api::b3_world_ensure_gpu(world);
    }
    pub fn physics_step(&self, world: WorldId) -> u64 {
        #[cfg(target_os = "linux")]
        if let Some(cpu) = &self.cpu { return cpu.steps; }
        crate::api::b3_world_physics_step(world)
    }
    pub(crate) fn is_cpu(&self) -> bool {
        #[cfg(target_os = "linux")]
        { return self.cpu.is_some(); }
        #[cfg(not(target_os = "linux"))]
        false
    }

    pub(crate) fn overlap_progress(&self)->Option<(u64,u64,usize)> {
        self.overlap.as_ref().map(|o|(o.published,o.displayed,o.ring.as_ref().map_or(0,|r|r.pending())))
    }
    pub fn drain_overlap(&mut self,device:&Device,queue:&Queue,world:WorldId)->Option<f32> {
        if !self.overlap_requested {return None;}
        let step=self.physics_step(world);let start=Instant::now();self.drain_only=true;
        self.frame(device,queue,world).expect("final overlap drain");self.drain_only=false;
        assert_eq!(self.physics_step(world),step);
        let overlap=self.overlap.as_ref().unwrap();assert_eq!(overlap.published,step);assert_eq!(overlap.displayed,step);
        if let Some(ring)=&overlap.ring {assert_eq!(ring.pending(),0);}
        let elapsed=start.elapsed().as_secs_f32()*1000.0;
        eprintln!("overlap-final: submitted={} displayed={} drain_ms={}",step,overlap.displayed,elapsed);Some(elapsed)
    }
    pub fn set_staged_source(&mut self, source: crate::sim::GpuDevice) { self.staged_source=Some(source); }
    pub fn frame(
        &mut self,
        device: &Device,
        queue: &Queue,
        world: WorldId,
    ) -> Result<FrameTimings, wgpu::SurfaceError> {
        let t0 = Instant::now();
        // Acquire before arming a merged producer signal: an acquire failure
        // must not leave an unconsumed ready semaphore behind.
        #[cfg(feature = "native-command-cache")]
        let mut early_output = if self.overlap_requested || (self.render_bridge.is_some() && !self.late_acquire) { Some(self.surface.get_current_texture()?) } else { None };
        #[cfg(target_os = "linux")]
        if self.overlap_requested && self.overlap.is_none() {
            let source=self.staged_source.as_ref().expect("overlap requires split adapter");
            let (state,cold,count)=b3_world_render_buffers(world).expect("seed render state");
            assert_eq!(self.physics_step(world),0,"seed must be actual step zero");
            let state_bytes=u64::from(count)*std::mem::size_of::<crate::types::BodyStateGpu>() as u64;
            let cold_bytes=u64::from(count)*std::mem::size_of::<crate::types::BodyColdGpu>() as u64;
            let mut ring=None;let mut cpu_buffers=Vec::new();
            if let Some(cpu)=&mut self.cpu {
                cpu.seed_render_state(queue);cpu_buffers.push(cpu.buffer.clone());
                cpu_buffers.push(device.create_buffer(&wgpu::BufferDescriptor{label:Some("CPU previous-step states"),size:state_bytes,usage:wgpu::BufferUsages::STORAGE|wgpu::BufferUsages::COPY_DST,mapped_at_creation:false}));
            } else {
                let mut transport=crate::staged_ring::StagedRing::new(&source.device,device,state_bytes,cold_bytes);
                transport.publish(&source.queue,&state,&cold,0).expect("seed snapshot");ring=Some(transport);
            }
            self.overlap=Some(OverlapState{world,count,ring,cpu_buffers,published:0,displayed:0});
        }
        let physics_start = Instant::now();
        #[cfg(feature = "native-command-cache")]
        if let Some(bridge) = &mut self.render_bridge { bridge.arm(world); }

        #[cfg(target_os = "linux")]
        if !self.drain_only {
            if let Some(cpu) = &mut self.cpu {
                if let Some(overlap)=&self.overlap {cpu.buffer=overlap.cpu_buffers[((cpu.steps+1)%2) as usize].clone();}
                cpu.advance(queue);
            }
        }
        if !self.is_cpu() && !self.drain_only {
            if self.camera_world!=Some(world)
                && std::env::var("GPU_PHYSICS_DEMAND_POSES").as_deref()==Ok("1") {
                crate::api::b3_world_set_automatic_pose_snapshots(world,false);
            }
            b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
        }
        #[cfg(feature = "native-command-cache")]
        if self.render_bridge.is_some() { crate::api::b3_world_cancel_render_copy(world); }
        let physics_ms = physics_start.elapsed().as_secs_f32() * 1000.0;

        let t_draw = Instant::now();
        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
            label: Some("draw"),
        });
        // Compatibility cohorts finalize here; eligible resident worlds draw
        // directly after GPU CCD in queue order, without a CPU body snapshot.
        if !self.is_cpu() { b3_world_finalize_render_state(world); }
        let Some((instance_state, instance_cold, _body_count)) = b3_world_render_buffers(world) else {
            return Ok(FrameTimings {
                total_ms: t0.elapsed().as_secs_f32() * 1000.0,
                physics_ms,
                draw_ms: 0.0,
                present_ms: 0.0,
            });
        };

        #[cfg(target_os = "linux")]
        let instance_state = self.cpu.as_ref().map(|cpu| cpu.buffer.clone()).unwrap_or(instance_state);
        if self.camera_world != Some(world) {
            if let Some((lower,upper)) = crate::api::b3_world_render_framing_bounds(world) {
                self.camera.fit(lower,upper,self.config.width as f32/self.config.height as f32);
            }
            self.camera_world = Some(world);
        }
        queue.write_buffer(&self.camera_buf, 0, bytemuck::bytes_of(&self.camera.uniform(self.config.width as f32/self.config.height as f32)));
        self.scene_draw.prepare(device, world);
        #[cfg(feature = "native-command-cache")]
        let output = match early_output.take().map(Ok).unwrap_or_else(|| self.surface.get_current_texture()) {
            Ok(output) => output,
            Err(error) => {
                if let Some(bridge) = &mut self.render_bridge { bridge.discard(); }
                return Err(error);
            }
        };
        #[cfg(not(feature = "native-command-cache"))]
        let output = self.surface.get_current_texture()?;
        #[cfg(feature = "native-command-cache")]
        let (instance_state, instance_cold) = if let Some(bridge) = &mut self.render_bridge {
            assert!(!self.cpu.is_some(), "secondary queue prototype requires GPU physics");
            bridge.copy(&instance_state, &instance_cold, _body_count)
        } else { (instance_state, instance_cold) };
        let step=self.physics_step(world);
        let (instance_state,instance_cold)=if self.overlap_requested && !self.is_cpu() {
            let source=self.staged_source.as_ref().unwrap();let overlap=self.overlap.as_mut().unwrap();
            assert_eq!((overlap.world,overlap.count),(world,_body_count));
            let ring=overlap.ring.as_mut().unwrap();
            if !self.drain_only {
                assert_eq!(step,overlap.published+1);
                ring.publish(&source.queue,&instance_state,&instance_cold,step).expect("publish current step");overlap.published=step;
            }
            let expected=if self.drain_only {step}else{step-1};
            let snapshot=ring.consume(queue,expected).expect("consume previous step");
            assert_eq!(snapshot.step,expected);overlap.displayed=snapshot.step;
            (snapshot.state,snapshot.cold)
        } else if let Some(source)=&self.staged_source {
            let cpu=self.is_cpu();
            if self.staged_buffers.as_ref().is_none_or(|b|b.world!=world || b.count!=_body_count) {
                self.staged_buffers=Some(StagedBuffers::new(source,device,world,_body_count));
            }
            let buffers=self.staged_buffers.as_mut().unwrap();
            buffers.transfer(source,device,queue,if cpu {None} else {Some(&instance_state)},&instance_cold);
            (if cpu {instance_state} else {buffers.state.clone()},buffers.cold.clone())
        } else {(instance_state,instance_cold)};
        let instance_state=if self.overlap_requested && self.is_cpu() {
            let overlap=self.overlap.as_mut().unwrap();assert_eq!((overlap.world,overlap.count),(world,_body_count));
            if !self.drain_only {assert_eq!(step,overlap.published+1);overlap.published=step;}
            overlap.displayed=if self.drain_only{step}else{step-1};
            overlap.cpu_buffers[(overlap.displayed%2) as usize].clone()
        } else {instance_state};
        let scene_binding = self.scene_draw.bind(device, &instance_state, &instance_cold);
        let extent=(self.camera.radius*0.7).max(12.0);
        let light=glam::Vec3::new(0.4,0.8,0.3).normalize();
        let light_matrix=glam::Mat4::orthographic_rh(-extent,extent,-extent,extent,0.1,extent*6.0)
            *glam::Mat4::look_at_rh(self.camera.target+light*extent*3.0,self.camera.target,glam::Vec3::Y);
        let light_uniform=CameraUniform{view_proj:light_matrix.to_cols_array_2d(),_pad:[0;48]};
        queue.write_buffer(&self.shadow_camera,0,bytemuck::bytes_of(&light_uniform));
        {
            let mut pass=encoder.begin_render_pass(&wgpu::RenderPassDescriptor{label:Some("directional-shadow"),color_attachments:&[],
                depth_stencil_attachment:Some(wgpu::RenderPassDepthStencilAttachment{view:&self.shadow_view,depth_ops:Some(wgpu::Operations{load:wgpu::LoadOp::Clear(1.0),store:wgpu::StoreOp::Store}),stencil_ops:None}),timestamp_writes:None,occlusion_query_set:None});
            pass.set_bind_group(0,&self.shadow_camera_bg,&[]);
            self.scene_draw.draw_shadow(&mut pass,&scene_binding);
        }


        let view = output
            .texture
            .create_view(&wgpu::TextureViewDescriptor::default());

        {
            let mut pass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
                label: Some("scene"),
                color_attachments: &[Some(wgpu::RenderPassColorAttachment {
                    view: &view,
                    resolve_target: None,
                    ops: wgpu::Operations {
                        load: wgpu::LoadOp::Clear(wgpu::Color {
                            r: 0.06,
                            g: 0.07,
                            b: 0.09,
                            a: 1.0,
                        }),
                        store: wgpu::StoreOp::Store,
                    },
                    depth_slice: None,
                })],
                depth_stencil_attachment: self.depth_view.as_ref().map(|depth| {
                    wgpu::RenderPassDepthStencilAttachment {
                        view: depth,
                        depth_ops: Some(wgpu::Operations {
                            load: wgpu::LoadOp::Clear(1.0),
                            store: wgpu::StoreOp::Store,
                        }),
                        stencil_ops: None,
                    }
                }),
                timestamp_writes: None,
                occlusion_query_set: None,
            });
            pass.set_bind_group(0, &self.camera_bg, &[]);
            pass.set_pipeline(&self.ground_pipeline);
            pass.set_bind_group(1, &self.shadow_bg, &[]);
            pass.set_vertex_buffer(0, self.ground_vb.slice(..));
            pass.set_index_buffer(self.ground_ib.slice(..), wgpu::IndexFormat::Uint32);
            pass.draw_indexed(0..self.ground_index_count, 0, 0..1);

            self.scene_draw.draw(&mut pass, &scene_binding);
        }

        #[cfg(feature = "native-command-cache")]
        if let Some(bridge) = &mut self.render_bridge { bridge.finish(); }
        queue.submit(Some(encoder.finish()));
        let draw_ms = t_draw.elapsed().as_secs_f32() * 1000.0;
        let t_present = Instant::now();
        output.present();
        let present_ms = t_present.elapsed().as_secs_f32() * 1000.0;
        // Drain a ready status after presentation so the single staging slot
        // can be reused by the next step. Never wait or download body state.
        // This work remains inside total_ms.
        #[cfg(not(target_arch = "wasm32"))]
        if self.poll_idle_status && !self.is_cpu() {
            let _ = crate::api::b3_world_contact_metrics(world, false);
        }
        Ok(FrameTimings {
            total_ms: t0.elapsed().as_secs_f32() * 1000.0,
            physics_ms,
            draw_ms,
            present_ms,
        })
    }
}

fn make_depth(device: &Device, width: u32, height: u32) -> (TextureView, (u32, u32)) {
    let tex = device.create_texture(&wgpu::TextureDescriptor {
        label: Some("depth"),
        size: wgpu::Extent3d {
            width,
            height,
            depth_or_array_layers: 1,
        },
        mip_level_count: 1,
        sample_count: 1,
        dimension: wgpu::TextureDimension::D2,
        format: depth_format(),
        usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
        view_formats: &[],
    });
    (
        tex.create_view(&wgpu::TextureViewDescriptor::default()),
        (width, height),
    )
}

fn upload_camera(_device: &Device, queue: &Queue, camera_buf: &Buffer, vp: CameraUniform) {
    queue.write_buffer(camera_buf, 0, bytemuck::bytes_of(&vp));
}

#[cfg(not(target_arch = "wasm32"))]
pub fn window_surface(
    instance: &wgpu::Instance,
    window: Arc<winit::window::Window>,
) -> Result<Surface<'static>, String> {
    instance
        .create_surface(window)
        .map_err(|e| format!("create_surface: {e}"))
}

#[cfg(test)]
mod camera_tests {
    use super::*;
    #[test]
    fn fit_keeps_mixed_stack_bounds_inside_wide_and_tall_viewports() {
        let lower=[-1.0,-1.0,-1.0];let upper=[58.0,3.0,43.0];
        for aspect in [0.5,1.0,16.0/9.0,2.5] {
            let mut camera=OrbitCamera::default();camera.fit(lower,upper,aspect);
            let matrix=glam::Mat4::from_cols_array_2d(&camera.uniform(aspect).view_proj);
            for x in [lower[0],upper[0]] {for y in [lower[1],upper[1]] {for z in [lower[2],upper[2]] {
                let p=matrix*glam::Vec4::new(x,y,z,1.0);let ndc=p.truncate()/p.w;
                assert!(ndc.x.abs()<1.0 && ndc.y.abs()<1.0 && (0.0..1.0).contains(&ndc.z),"aspect={aspect}, point={ndc:?}");
            }}}
        }
    }
}

// Diagnostic transport: current-state staging across adapters, never lagged poses.
// Named fixtures have immutable cold body metadata; a new world/count recreates it.
struct StagedBuffers {
    world: WorldId, count:u32, initialized:bool,
    readback:Buffer, state:Buffer, cold:Buffer, state_bytes:u64, cold_bytes:u64,
}
impl StagedBuffers {
    fn new(source:&crate::sim::GpuDevice, destination:&Device,world:WorldId,count:u32)->Self {
        let state_bytes=u64::from(count)*std::mem::size_of::<crate::types::BodyStateGpu>() as u64;
        let cold_bytes=u64::from(count)*std::mem::size_of::<crate::types::BodyColdGpu>() as u64;
        assert!(state_bytes>0 && cold_bytes>0);
        let readback=source.device.create_buffer(&wgpu::BufferDescriptor{label:Some("split adapter staging"),size:state_bytes+cold_bytes,usage:wgpu::BufferUsages::MAP_READ|wgpu::BufferUsages::COPY_DST,mapped_at_creation:false});
        let make=|size|destination.create_buffer(&wgpu::BufferDescriptor{label:Some("split adapter destination"),size,usage:wgpu::BufferUsages::STORAGE|wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::COPY_SRC,mapped_at_creation:false});
        Self{world,count,initialized:false,readback,state:make(state_bytes),cold:make(cold_bytes),state_bytes,cold_bytes}
    }
    fn transfer(&mut self,source:&crate::sim::GpuDevice,destination:&Device,queue:&Queue,state:Option<&Buffer>,cold:&Buffer) {
        if state.is_none() && self.initialized {return;}
        let mut encoder=source.device.create_command_encoder(&wgpu::CommandEncoderDescriptor{label:Some("split adapter current state")});
        if let Some(state)=state {encoder.copy_buffer_to_buffer(state,0,&self.readback,0,self.state_bytes);}
        if !self.initialized {encoder.copy_buffer_to_buffer(cold,0,&self.readback,self.state_bytes,self.cold_bytes);}
        source.queue.submit([encoder.finish()]);
        let (tx,rx)=std::sync::mpsc::channel();
        self.readback.slice(..).map_async(wgpu::MapMode::Read,move|result|{tx.send(result).unwrap();});
        source.device.poll(wgpu::PollType::Wait).expect("split adapter wait");
        rx.recv().unwrap().expect("split adapter map");
        let verify=std::env::var("GPU_PHYSICS_SPLIT_VERIFY").as_deref()==Ok("1") && !self.initialized;
        let mut expected=Vec::new();
        {
            let bytes=self.readback.slice(..).get_mapped_range();
            if verify {expected=bytes.to_vec();}
            if state.is_some() {queue.write_buffer(&self.state,0,&bytes[..self.state_bytes as usize]);}
            if !self.initialized {queue.write_buffer(&self.cold,0,&bytes[self.state_bytes as usize..]);}
        }
        self.readback.unmap();
        if verify {
            let readback=destination.create_buffer(&wgpu::BufferDescriptor{label:Some("split adapter verify"),size:self.state_bytes+self.cold_bytes,usage:wgpu::BufferUsages::MAP_READ|wgpu::BufferUsages::COPY_DST,mapped_at_creation:false});
            let mut enc=destination.create_command_encoder(&wgpu::CommandEncoderDescriptor{label:Some("split adapter verify")});
            if state.is_some() {enc.copy_buffer_to_buffer(&self.state,0,&readback,0,self.state_bytes);}
            enc.copy_buffer_to_buffer(&self.cold,0,&readback,self.state_bytes,self.cold_bytes);
            queue.submit([enc.finish()]);
            let (tx,rx)=std::sync::mpsc::channel();
            readback.slice(..).map_async(wgpu::MapMode::Read,move|r|tx.send(r).unwrap());
            destination.poll(wgpu::PollType::Wait).expect("verify wait");rx.recv().unwrap().expect("verify map");
            {let actual=readback.slice(..).get_mapped_range();let start=if state.is_some(){0}else{self.state_bytes as usize};assert_eq!(&actual[start..],&expected[start..],"cross-adapter bytes must be exact");}
            readback.unmap();eprintln!("split-adapter-byte-verify: passed state={} cold={}",if state.is_some(){self.state_bytes}else{0},self.cold_bytes);
        }
        self.initialized=true;
    }
}

struct OverlapState {
    world:WorldId,count:u32,ring:Option<crate::staged_ring::StagedRing>,cpu_buffers:Vec<Buffer>,published:u64,displayed:u64,
}
