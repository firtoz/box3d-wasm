//! Offscreen 1280×720 H.264 clips for pre-commit visual comparison (30 fps CFR).

use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::process::{Command, Stdio};

use bytemuck::Zeroable;
use wgpu::util::DeviceExt;

use crate::api::{
    b3_destroy_world, b3_world_finalize_render_state, b3_world_render_buffers, b3_world_ensure_gpu,
    b3_world_step_gpu,
};
use crate::mesh::{
    self, camera_matrix, instance_cold_layout, instance_state_layout, vertex_layout,
};
use crate::scenes::build_demo_world;
use crate::sim::GpuDevice;
use crate::types::{BodyColdGpu, BodyStateGpu, DemoConfig, DEFAULT_SUB_STEPS, FIXED_DT, GROUND_Y};

pub const RECORD_WIDTH: u32 = 1280;
pub const RECORD_HEIGHT: u32 = 720;
pub const RECORD_FPS: u32 = 30;

const COPY_ALIGN: u32 = 256;

#[repr(C)]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
struct CameraUniform {
    view_proj: [[f32; 4]; 4],
    _pad: [u32; 48],
}

pub struct RecordConfig {
    pub demo: DemoConfig,
    pub frames: u32,
    pub path: PathBuf,
}

pub fn default_record_path(scene_slug: &str, frames: u32) -> PathBuf {
    let dir = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("recordings");
    dir.join(format!("{scene_slug}-{frames}f.mp4"))
}

pub async fn record_mp4(cfg: RecordConfig) -> Result<(), String> {
    which_ffmpeg()?;
    if let Some(parent) = cfg.path.parent() {
        std::fs::create_dir_all(parent).map_err(|e| e.to_string())?;
    }

    let gpu = GpuDevice::new(None).await?;
    let world = build_demo_world(gpu.clone(), &cfg.demo);
    b3_world_ensure_gpu(world);
    let scene = OffscreenScene::new(&gpu.device, RECORD_WIDTH, RECORD_HEIGHT);
    let padded_row = padded_bytes_per_row(RECORD_WIDTH);
    let staging = gpu.device.create_buffer(&wgpu::BufferDescriptor {
        label: Some("video-staging"),
        size: padded_row as u64 * RECORD_HEIGHT as u64,
        usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
        mapped_at_creation: false,
    });

    let mut ff = spawn_ffmpeg(&cfg.path, RECORD_WIDTH, RECORD_HEIGHT, RECORD_FPS)?;
    let stdin = ff.stdin.as_mut().ok_or("ffmpeg stdin")?;

    let n = cfg.frames.max(1);
    for step in 0..n {
        if step > 0 {
            b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
        }
        scene.draw(&gpu.device, &gpu.queue, world, &staging, padded_row)?;
        let rgba = read_rgba(
            &gpu.device,
            &staging,
            RECORD_WIDTH,
            RECORD_HEIGHT,
            padded_row,
        )
        .await?;
        stdin
            .write_all(&rgba)
            .map_err(|e| format!("ffmpeg write: {e}"))?;
        if step % 30 == 0 {
            eprintln!("record frame {step}/{n} → {}", cfg.path.display());
        }
    }

    drop(ff.stdin.take());
    let mut err = String::new();
    if let Some(mut e) = ff.stderr.take() {
        let _ = e.read_to_string(&mut err);
    }
    let status = ff.wait().map_err(|e| e.to_string())?;
    b3_destroy_world(world);
    if !status.success() {
        return Err(format!("ffmpeg exited {status}\n{err}"));
    }
    eprintln!(
        "wrote {} ({} frames @ {} fps)",
        cfg.path.display(),
        n,
        RECORD_FPS
    );
    Ok(())
}

/// Encode an oracle dump (`B3OR` BodyGpu frames) with the same camera as GPU `--mp4`.
pub async fn record_mp4_from_dump(dump_path: PathBuf, out_path: PathBuf) -> Result<(), String> {
    which_ffmpeg()?;
    if let Some(parent) = out_path.parent() {
        std::fs::create_dir_all(parent).map_err(|e| e.to_string())?;
    }
    let dump =
        std::fs::read(&dump_path).map_err(|e| format!("read {}: {e}", dump_path.display()))?;
    if dump.len() < 20 {
        return Err("oracle dump too short".into());
    }
    if &dump[0..4] != b"B3OR" {
        return Err("oracle dump magic: want B3OR".into());
    }
    let version = u32::from_le_bytes(dump[4..8].try_into().unwrap());
    if version != 1 {
        return Err(format!("oracle dump version {version}"));
    }
    let nframes = u32::from_le_bytes(dump[8..12].try_into().unwrap());
    let body_count = u32::from_le_bytes(dump[12..16].try_into().unwrap());
    let stride = u32::from_le_bytes(dump[16..20].try_into().unwrap());
    if !matches!(stride, 144 | 160) {
        return Err(format!("oracle dump stride {stride}, want 144 or 160"));
    }
    let header = 20usize;
    // The current 160-byte record appends island/sleep metadata; the
    // rendered pose/geometry prefix is unchanged from the 144-byte format.
    let frame_bytes = body_count as usize * stride as usize;
    let expect = header + nframes as usize * frame_bytes;
    if dump.len() != expect {
        return Err(format!(
            "oracle dump size {} want {expect} (frames={nframes} bodies={body_count})",
            dump.len()
        ));
    }

    let gpu = GpuDevice::new(None).await?;
    let scene = OffscreenScene::new(&gpu.device, RECORD_WIDTH, RECORD_HEIGHT);
    let padded_row = padded_bytes_per_row(RECORD_WIDTH);
    let staging = gpu.device.create_buffer(&wgpu::BufferDescriptor {
        label: Some("video-staging"),
        size: padded_row as u64 * RECORD_HEIGHT as u64,
        usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
        mapped_at_creation: false,
    });

    let mut ff = spawn_ffmpeg(&out_path, RECORD_WIDTH, RECORD_HEIGHT, RECORD_FPS)?;
    let stdin = ff.stdin.as_mut().ok_or("ffmpeg stdin")?;
    let n = nframes.max(1);
    for step in 0..n {
        let off = header + step as usize * frame_bytes;
        let slice = if frame_bytes == 0 {
            &[0u8; 144][..]
        } else {
            &dump[off..off + frame_bytes]
        };
        let oracle_body = |chunk: &[u8]| {
            let f =
                |offset: usize| f32::from_le_bytes(chunk[offset..offset + 4].try_into().unwrap());
            let u =
                |offset: usize| u32::from_le_bytes(chunk[offset..offset + 4].try_into().unwrap());
            (
                BodyStateGpu {
                    pos: [f(0), f(4), f(8)],
                    inv_mass: f(12),
                    vel: [f(16), f(20), f(24)],
                    flags: u(44),
                    rot: [f(48), f(52), f(56), f(60)],
                    omega: [0.0; 3],
                    sleep_velocity: 0.0,
                    dp: [0.0; 3],
                    sleep_time: 0.0,
                    dq: [0.0, 0.0, 0.0, 1.0],
                },
                BodyColdGpu {
                    half: [f(32), f(36), f(40)],
                    kind: u(28),
                    ..BodyColdGpu::zeroed()
                },
            )
        };
        let (states, cold): (Vec<_>, Vec<_>) = if frame_bytes == 0 {
            vec![oracle_body(&[0u8; 144])].into_iter().unzip()
        } else {
            slice.chunks_exact(stride as usize).map(oracle_body).unzip()
        };
        let instance_state = gpu
            .device
            .create_buffer_init(&wgpu::util::BufferInitDescriptor {
                label: Some("oracle-instance-state"),
                contents: bytemuck::cast_slice(&states),
                usage: wgpu::BufferUsages::VERTEX,
            });
        let instance_cold = gpu
            .device
            .create_buffer_init(&wgpu::util::BufferInitDescriptor {
                label: Some("oracle-instance-cold"),
                contents: bytemuck::cast_slice(&cold),
                usage: wgpu::BufferUsages::VERTEX,
            });
        scene.draw_instances(
            &gpu.device,
            &gpu.queue,
            &instance_state,
            &instance_cold,
            body_count.max(1),
            &staging,
            padded_row,
        )?;
        let rgba = read_rgba(
            &gpu.device,
            &staging,
            RECORD_WIDTH,
            RECORD_HEIGHT,
            padded_row,
        )
        .await?;
        stdin
            .write_all(&rgba)
            .map_err(|e| format!("ffmpeg write: {e}"))?;
        if step % 30 == 0 {
            eprintln!("replay frame {step}/{n} → {}", out_path.display());
        }
    }

    drop(ff.stdin.take());
    let mut err = String::new();
    if let Some(mut e) = ff.stderr.take() {
        let _ = e.read_to_string(&mut err);
    }
    let status = ff.wait().map_err(|e| e.to_string())?;
    if !status.success() {
        return Err(format!("ffmpeg exited {status}\n{err}"));
    }
    eprintln!(
        "wrote {} ({} frames @ {} fps)",
        out_path.display(),
        n,
        RECORD_FPS
    );
    Ok(())
}

fn which_ffmpeg() -> Result<(), String> {
    Command::new("ffmpeg")
        .arg("-version")
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .status()
        .map_err(|_| "ffmpeg not found on PATH".to_string())?
        .success()
        .then_some(())
        .ok_or_else(|| "ffmpeg not found on PATH".to_string())
}

fn spawn_ffmpeg(path: &Path, w: u32, h: u32, fps: u32) -> Result<std::process::Child, String> {
    let g = fps.max(1).to_string();
    let fps_s = fps.to_string();
    Command::new("ffmpeg")
        .args([
            "-y",
            "-f",
            "rawvideo",
            "-pix_fmt",
            "rgba",
            "-s",
            &format!("{w}x{h}"),
            "-framerate",
            &fps_s,
            "-i",
            "-",
            "-an",
            "-c:v",
            "libx264",
            "-pix_fmt",
            "yuv420p",
            "-preset",
            "medium",
            "-crf",
            "18",
            "-fps_mode",
            "cfr",
            "-r",
            &fps_s,
            "-g",
            &g,
            "-bf",
            "0",
            "-movflags",
            "+faststart",
        ])
        .arg(path)
        .stdin(Stdio::piped())
        .stdout(Stdio::null())
        .stderr(Stdio::piped())
        .spawn()
        .map_err(|e| format!("spawn ffmpeg: {e}"))
}

fn padded_bytes_per_row(width: u32) -> u32 {
    let unpadded = width * 4;
    unpadded.div_ceil(COPY_ALIGN) * COPY_ALIGN
}

struct OffscreenScene {
    color: wgpu::Texture,
    color_view: wgpu::TextureView,
    depth_view: wgpu::TextureView,
    camera_bg: wgpu::BindGroup,
    sphere_vb: wgpu::Buffer,
    sphere_ib: wgpu::Buffer,
    sphere_index_count: u32,
    box_vb: wgpu::Buffer,
    box_ib: wgpu::Buffer,
    box_index_count: u32,
    capsule_vb: wgpu::Buffer,
    capsule_ib: wgpu::Buffer,
    capsule_index_count: u32,
    ground_vb: wgpu::Buffer,
    ground_ib: wgpu::Buffer,
    ground_index_count: u32,
    sphere_pipeline: wgpu::RenderPipeline,
    box_pipeline: wgpu::RenderPipeline,
    capsule_pipeline: wgpu::RenderPipeline,
    ground_pipeline: wgpu::RenderPipeline,
}

impl OffscreenScene {
    fn new(device: &wgpu::Device, width: u32, height: u32) -> Self {
        let format = wgpu::TextureFormat::Rgba8Unorm;
        let color = device.create_texture(&wgpu::TextureDescriptor {
            label: Some("video-color"),
            size: wgpu::Extent3d {
                width,
                height,
                depth_or_array_layers: 1,
            },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format,
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT | wgpu::TextureUsages::COPY_SRC,
            view_formats: &[],
        });
        let color_view = color.create_view(&wgpu::TextureViewDescriptor::default());
        let depth = device.create_texture(&wgpu::TextureDescriptor {
            label: Some("video-depth"),
            size: wgpu::Extent3d {
                width,
                height,
                depth_or_array_layers: 1,
            },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Depth32Float,
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
            view_formats: &[],
        });
        let depth_view = depth.create_view(&wgpu::TextureViewDescriptor::default());

        let vp = CameraUniform {
            view_proj: camera_matrix(width as f32 / height as f32).to_cols_array_2d(),
            _pad: [0; 48],
        };
        debug_assert_eq!(std::mem::size_of::<CameraUniform>(), 256);
        let camera_buf = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("video-camera"),
            contents: bytemuck::bytes_of(&vp),
            usage: wgpu::BufferUsages::UNIFORM,
        });

        let sphere = mesh::unit_uv_sphere(16, 12);
        let cube = mesh::unit_cube();
        let capsule = mesh::unit_capsule_x(16, 6);
        let ground = mesh::ground_quad(50.0, GROUND_Y);
        let (sphere_vb, sphere_ib, sphere_index_count) =
            mesh::upload_mesh(device, &sphere, "video-sphere");
        let (box_vb, box_ib, box_index_count) = mesh::upload_mesh(device, &cube, "video-box");
        let (capsule_vb, capsule_ib, capsule_index_count) =
            mesh::upload_mesh(device, &capsule, "video-capsule");
        let (ground_vb, ground_ib, ground_index_count) =
            mesh::upload_mesh(device, &ground, "video-ground");

        let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("render"),
            source: wgpu::ShaderSource::Wgsl(include_str!("../shaders/render.wgsl").into()),
        });
        let bgl = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: Some("video-bgl"),
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
        let pl = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: Some("video-pl"),
            bind_group_layouts: &[&bgl],
            push_constant_ranges: &[],
        });
        let depth_stencil = Some(wgpu::DepthStencilState {
            format: wgpu::TextureFormat::Depth32Float,
            depth_write_enabled: true,
            depth_compare: wgpu::CompareFunction::Less,
            stencil: wgpu::StencilState::default(),
            bias: wgpu::DepthBiasState::default(),
        });
        let instance_buffers = [
            vertex_layout(),
            instance_state_layout(),
            instance_cold_layout(),
        ];
        let color_target = [Some(wgpu::ColorTargetState {
            format,
            blend: Some(wgpu::BlendState::REPLACE),
            write_mask: wgpu::ColorWrites::ALL,
        })];

        let make = |label: &str, vs: &str, buffers: &[wgpu::VertexBufferLayout]| {
            let fs = if vs == "vs_ground" {
                "fs_ground"
            } else {
                "fs_main"
            };
            let cull = if vs == "vs_ground" {
                None
            } else {
                Some(wgpu::Face::Back)
            };
            device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
                label: Some(label),
                layout: Some(&pl),
                vertex: wgpu::VertexState {
                    module: &shader,
                    entry_point: Some(vs),
                    compilation_options: Default::default(),
                    buffers,
                },
                fragment: Some(wgpu::FragmentState {
                    module: &shader,
                    entry_point: Some(fs),
                    compilation_options: Default::default(),
                    targets: &color_target,
                }),
                primitive: wgpu::PrimitiveState {
                    cull_mode: cull,
                    ..Default::default()
                },
                depth_stencil: depth_stencil.clone(),
                multisample: wgpu::MultisampleState::default(),
                multiview: None,
                cache: None,
            })
        };

        let camera_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: Some("video-bg"),
            layout: &bgl,
            entries: &[wgpu::BindGroupEntry {
                binding: 0,
                resource: camera_buf.as_entire_binding(),
            }],
        });

        Self {
            color,
            color_view,
            depth_view,
            camera_bg,
            sphere_vb,
            sphere_ib,
            sphere_index_count,
            box_vb,
            box_ib,
            box_index_count,
            capsule_vb,
            capsule_ib,
            capsule_index_count,
            ground_vb,
            ground_ib,
            ground_index_count,
            sphere_pipeline: make("v-sphere", "vs_sphere", &instance_buffers),
            box_pipeline: make("v-box", "vs_box", &instance_buffers),
            capsule_pipeline: make("v-capsule", "vs_capsule", &instance_buffers),
            ground_pipeline: make("v-ground", "vs_ground", &[vertex_layout()]),
        }
    }

    fn draw(
        &self,
        device: &wgpu::Device,
        queue: &wgpu::Queue,
        world: crate::api::WorldId,
        staging: &wgpu::Buffer,
        padded_row: u32,
    ) -> Result<(), String> {
        b3_world_finalize_render_state(world);
        let (instance_state, instance_cold, body_count) =
            b3_world_render_buffers(world).ok_or("no body buffers")?;
        self.draw_instances(
            device,
            queue,
            &instance_state,
            &instance_cold,
            body_count,
            staging,
            padded_row,
        )
    }

    fn draw_instances(
        &self,
        device: &wgpu::Device,
        queue: &wgpu::Queue,
        instance_state: &wgpu::Buffer,
        instance_cold: &wgpu::Buffer,
        body_count: u32,
        staging: &wgpu::Buffer,
        padded_row: u32,
    ) -> Result<(), String> {
        let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
            label: Some("video-draw"),
        });
        {
            let mut pass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
                label: Some("video-scene"),
                color_attachments: &[Some(wgpu::RenderPassColorAttachment {
                    view: &self.color_view,
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
                depth_stencil_attachment: Some(wgpu::RenderPassDepthStencilAttachment {
                    view: &self.depth_view,
                    depth_ops: Some(wgpu::Operations {
                        load: wgpu::LoadOp::Clear(1.0),
                        store: wgpu::StoreOp::Store,
                    }),
                    stencil_ops: None,
                }),
                timestamp_writes: None,
                occlusion_query_set: None,
            });
            pass.set_bind_group(0, &self.camera_bg, &[]);
            pass.set_pipeline(&self.ground_pipeline);
            pass.set_vertex_buffer(0, self.ground_vb.slice(..));
            pass.set_index_buffer(self.ground_ib.slice(..), wgpu::IndexFormat::Uint32);
            pass.draw_indexed(0..self.ground_index_count, 0, 0..1);

            pass.set_pipeline(&self.sphere_pipeline);
            pass.set_vertex_buffer(0, self.sphere_vb.slice(..));
            pass.set_vertex_buffer(1, instance_state.slice(..));
            pass.set_vertex_buffer(2, instance_cold.slice(..));
            pass.set_index_buffer(self.sphere_ib.slice(..), wgpu::IndexFormat::Uint32);
            pass.draw_indexed(0..self.sphere_index_count, 0, 0..body_count);

            pass.set_pipeline(&self.box_pipeline);
            pass.set_vertex_buffer(0, self.box_vb.slice(..));
            pass.set_vertex_buffer(1, instance_state.slice(..));
            pass.set_vertex_buffer(2, instance_cold.slice(..));
            pass.set_index_buffer(self.box_ib.slice(..), wgpu::IndexFormat::Uint32);
            pass.draw_indexed(0..self.box_index_count, 0, 0..body_count);

            pass.set_pipeline(&self.capsule_pipeline);
            pass.set_vertex_buffer(0, self.capsule_vb.slice(..));
            pass.set_vertex_buffer(1, instance_state.slice(..));
            pass.set_vertex_buffer(2, instance_cold.slice(..));
            pass.set_index_buffer(self.capsule_ib.slice(..), wgpu::IndexFormat::Uint32);
            pass.draw_indexed(0..self.capsule_index_count, 0, 0..body_count);
        }
        encoder.copy_texture_to_buffer(
            wgpu::TexelCopyTextureInfo {
                texture: &self.color,
                mip_level: 0,
                origin: wgpu::Origin3d::ZERO,
                aspect: wgpu::TextureAspect::All,
            },
            wgpu::TexelCopyBufferInfo {
                buffer: staging,
                layout: wgpu::TexelCopyBufferLayout {
                    offset: 0,
                    bytes_per_row: Some(padded_row),
                    rows_per_image: Some(RECORD_HEIGHT),
                },
            },
            wgpu::Extent3d {
                width: RECORD_WIDTH,
                height: RECORD_HEIGHT,
                depth_or_array_layers: 1,
            },
        );
        queue.submit(Some(encoder.finish()));
        Ok(())
    }
}

async fn read_rgba(
    device: &wgpu::Device,
    staging: &wgpu::Buffer,
    width: u32,
    height: u32,
    padded_row: u32,
) -> Result<Vec<u8>, String> {
    let size = padded_row as u64 * height as u64;
    let slice = staging.slice(..size);
    let (tx, rx) = std::sync::mpsc::channel();
    slice.map_async(wgpu::MapMode::Read, move |r| {
        let _ = tx.send(r);
    });
    device
        .poll(wgpu::PollType::wait())
        .map_err(|e| e.to_string())?;
    rx.recv()
        .map_err(|e| e.to_string())?
        .map_err(|e| format!("map: {e}"))?;
    let data = slice.get_mapped_range();
    let src_row = padded_row as usize;
    let dst_row = (width * 4) as usize;
    let mut out = vec![0u8; dst_row * height as usize];
    for y in 0..height as usize {
        let s = y * src_row;
        let d = y * dst_row;
        out[d..d + dst_row].copy_from_slice(&data[s..s + dst_row]);
    }
    drop(data);
    staging.unmap();
    Ok(out)
}
