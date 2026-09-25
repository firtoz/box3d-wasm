//! Small loading overlay independent of physics resources and shader compilation.
use wgpu::util::DeviceExt;

pub(crate) struct LoadingScreen { pipeline: wgpu::RenderPipeline }
impl LoadingScreen {
    pub(crate) fn new(device: &wgpu::Device, format: wgpu::TextureFormat) -> Self {
        let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("loading screen"), source: wgpu::ShaderSource::Wgsl(r#"
struct Out { @builtin(position) position: vec4<f32>, @location(0) color: vec3<f32> }
@vertex fn vs(@location(0) p: vec2<f32>, @location(1) color: vec3<f32>) -> Out {
    var o: Out; o.position=vec4<f32>(p,0.0,1.0); o.color=color; return o;
}
@fragment fn fs(i: Out) -> @location(0) vec4<f32> { return vec4<f32>(i.color,1.0); }
"#.into()),
        });
        let pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("loading screen"), layout: None,
            vertex: wgpu::VertexState { module: &shader, entry_point: Some("vs"),
                compilation_options: Default::default(), buffers: &[wgpu::VertexBufferLayout {
                    array_stride: 20, step_mode: wgpu::VertexStepMode::Vertex,
                    attributes: &wgpu::vertex_attr_array![0 => Float32x2, 1 => Float32x3],
                }] },
            fragment: Some(wgpu::FragmentState { module: &shader, entry_point: Some("fs"),
                compilation_options: Default::default(), targets: &[Some(format.into())] }),
            primitive: Default::default(), depth_stencil: None, multisample: Default::default(),
            multiview: None, cache: None,
        });
        Self { pipeline }
    }
    pub(crate) fn draw(&self, device: &wgpu::Device, encoder: &mut wgpu::CommandEncoder,
        view: &wgpu::TextureView, size: (u32,u32), elapsed: f32, message: &str) {
        let (width,height)=(size.0 as f32,size.1 as f32);
        let scale=(width/300.0).min(3.0).max(1.0);
        let mut vertices: Vec<[f32;5]>=Vec::new();
        let mut rect=|x:f32,y:f32,w:f32,h:f32,c:[f32;3]| {
            for (dx,dy) in [(0.,0.),(1.,0.),(0.,1.),(0.,1.),(1.,0.),(1.,1.)] {
                vertices.push([2.*(x+dx*w)/width-1.,1.-2.*(y+dy*h)/height,c[0],c[1],c[2]]);
            }
        };
        for (line,text) in ["PREPARING SIMULATION",message,"PLEASE WAIT"].iter().enumerate() {
            let x=(width-text.len() as f32*6.*scale)/2.;
            let y=height/2.-50.*scale+line as f32*18.*scale;
            for (i,ch) in text.chars().enumerate() {
                for (row,bits) in glyph(ch).into_iter().enumerate() {
                    for col in 0..5 { if bits & (1<<(4-col)) != 0 {
                        rect(x+(i*6+col) as f32*scale,y+row as f32*scale,scale,scale,[0.65,0.82,0.95]);
                    }}
                }
            }
        }
        let bar_width=(width*0.5).min(450.);
        let x=(width-bar_width)/2.; let y=height/2.+20.*scale;
        rect(x,y,bar_width,4.*scale,[0.12,0.19,0.26]);
        rect(x+(elapsed*0.8).sin().mul_add(0.5,0.5)*(bar_width-40.*scale),y,40.*scale,4.*scale,[0.25,0.65,0.95]);
        let buffer=device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("loading glyphs"), contents: bytemuck::cast_slice(&vertices), usage: wgpu::BufferUsages::VERTEX,
        });
        let mut pass=encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
            label: Some("loading screen"), color_attachments: &[Some(wgpu::RenderPassColorAttachment {
                view, depth_slice: None, resolve_target: None, ops: wgpu::Operations {
                    load: wgpu::LoadOp::Clear(wgpu::Color {r:0.025,g:0.035,b:0.055,a:1.}), store: wgpu::StoreOp::Store,
                },
            })], depth_stencil_attachment: None, timestamp_writes: None, occlusion_query_set: None,
        });
        pass.set_pipeline(&self.pipeline);pass.set_vertex_buffer(0,buffer.slice(..));pass.draw(0..vertices.len() as u32,0..1);
    }
}
fn glyph(c:char)->[u8;7] {
    match c {
        'A'=>[14,17,17,31,17,17,17], 'C'=>[14,17,16,16,16,17,14],
        'D'=>[30,17,17,17,17,17,30], 'E'=>[31,16,16,30,16,16,31],
        'G'=>[14,17,16,23,17,17,15], 'I'=>[31,4,4,4,4,4,31],
        'L'=>[16,16,16,16,16,16,31], 'M'=>[17,27,21,21,17,17,17],
        'N'=>[17,25,25,21,19,19,17], 'O'=>[14,17,17,17,17,17,14],
        'P'=>[30,17,17,30,16,16,16], 'R'=>[30,17,17,30,20,18,17],
        'S'=>[15,16,16,14,1,1,30], 'T'=>[31,4,4,4,4,4,4],
        'U'=>[17,17,17,17,17,17,14], 'W'=>[17,17,17,21,21,27,17],
        _=>[0;7],
    }
}

#[cfg(all(test,not(target_arch="wasm32")))]
mod tests {
    use super::*;
    #[test]
    fn loading_overlay_renders_without_a_physics_world() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).unwrap();
        let size=wgpu::Extent3d{width:960,height:540,depth_or_array_layers:1};
        let texture=gpu.device.create_texture(&wgpu::TextureDescriptor {
            label:None,size,mip_level_count:1,sample_count:1,dimension:wgpu::TextureDimension::D2,
            format:wgpu::TextureFormat::Rgba8Unorm,usage:wgpu::TextureUsages::RENDER_ATTACHMENT|wgpu::TextureUsages::COPY_SRC,view_formats:&[],
        });
        let staging=gpu.device.create_buffer(&wgpu::BufferDescriptor{label:None,size:960*540*4,
            usage:wgpu::BufferUsages::MAP_READ|wgpu::BufferUsages::COPY_DST,mapped_at_creation:false});
        let screen=LoadingScreen::new(&gpu.device,wgpu::TextureFormat::Rgba8Unorm);
        let mut encoder=gpu.device.create_command_encoder(&Default::default());
        screen.draw(&gpu.device,&mut encoder,&texture.create_view(&Default::default()),(960,540),1.0,"PREPARING GPU");
        encoder.copy_texture_to_buffer(wgpu::TexelCopyTextureInfo{texture:&texture,mip_level:0,origin:wgpu::Origin3d::ZERO,aspect:wgpu::TextureAspect::All},
            wgpu::TexelCopyBufferInfo{buffer:&staging,layout:wgpu::TexelCopyBufferLayout{offset:0,bytes_per_row:Some(3840),rows_per_image:Some(540)}},size);
        gpu.queue.submit(Some(encoder.finish()));
        let (tx,rx)=std::sync::mpsc::channel();
        staging.slice(..).map_async(wgpu::MapMode::Read,move |r|{let _=tx.send(r);});
        gpu.device.poll(wgpu::PollType::wait()).unwrap();rx.recv().unwrap().unwrap();
        let bytes=staging.slice(..).get_mapped_range();
        assert!(bytes.chunks_exact(4).filter(|p|p[0]>100).count()>1000,"loading text must be visible without scene resources");
        if let Some(path)=std::env::var_os("GPU_PHYSICS_LOADING_CAPTURE") {
            let mut ppm=b"P6\n960 540\n255\n".to_vec();
            for pixel in bytes.chunks_exact(4) {ppm.extend_from_slice(&pixel[..3]);}
            std::fs::write(path,ppm).unwrap();
        }
    }
}
