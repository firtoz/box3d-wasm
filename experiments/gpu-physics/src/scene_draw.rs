//! Topology-cached instanced geometry with GPU body-state vertex fetches.
use std::collections::HashMap;
use std::ops::Range;
use glam::{Quat, Vec3};
use wgpu::util::DeviceExt;
use crate::{api, mesh::{self, CpuMesh, MeshVertex}, render_scene::{RenderGeometry, RenderSceneKey, RenderShape}};

#[repr(C)]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
struct Instance { body: u32, color: u32, p: [f32;3], q: [f32;4], scale: [f32;3] }
struct Batch { indices: Range<u32>, instances: Range<u32> }
struct Geometry { vertices: Vec<MeshVertex>, indices: Vec<u32>, instances: Vec<Instance>, batches: Vec<Batch> }

fn shape_mesh(shape: &RenderShape) -> (CpuMesh, Instance) {
    // Graph classification keeps slots below bit 30; bit 31 carries sensor metadata.
    let mut inst = Instance { body: shape.body_slot | (u32::from(shape.is_sensor) << 31), color: shape.custom_color,
        p: [0.0;3], q: [0.0,0.0,0.0,1.0], scale: [1.0;3] };
    let mesh = match &shape.geometry {
        RenderGeometry::Sphere {center, radius} => {
            inst.p=*center; inst.scale=[*radius;3]; mesh::unit_uv_sphere(16,12)
        }
        RenderGeometry::Box {center, half} => {inst.p=*center;inst.scale=*half;mesh::unit_cube()}
        RenderGeometry::Capsule {center, half_axis, radius} => {
            inst.p=*center;
            let axis=Vec3::from_array(*half_axis); let length=axis.length();
            if length>0.0 {inst.q=Quat::from_rotation_arc(Vec3::X,axis/length).to_array();}
            let mut mesh=mesh::unit_capsule_x(16,6);
            for v in &mut mesh.vertices {
                let x=v.position[0];
                v.position=[if x.abs()<=1.0 {x*length} else {x.signum()*length+(x-x.signum())*radius},v.position[1]*radius,v.position[2]*radius];
            }
            mesh
        }
        RenderGeometry::Hull {points, planes, ..} => {
            let center=points.iter().map(|p|Vec3::from_array(*p)).sum::<Vec3>()/points.len().max(1) as f32;
            inst.p=center.to_array();
            let mut mesh=CpuMesh{vertices:Vec::new(),indices:Vec::new()};
            for plane in planes.iter() {
                let n=Vec3::new(plane[0],plane[1],plane[2]);
                let extent=points.iter().map(|p|Vec3::from_array(*p).length()).fold(1.0f32,f32::max);
                let mut face:Vec<Vec3>=points.iter().map(|p|Vec3::from_array(*p))
                    .filter(|p|(n.dot(*p)-plane[3]).abs()<=1e-5*extent).collect();
                if face.len()<3 {continue;}
                let c=face.iter().copied().sum::<Vec3>()/face.len() as f32;
                let u=n.any_orthonormal_vector();let v=n.cross(u);
                face.sort_by(|a,b| {let a=*a-c;let b=*b-c;a.dot(v).atan2(a.dot(u)).total_cmp(&b.dot(v).atan2(b.dot(u)))});
                let base=mesh.vertices.len() as u32;
                mesh.vertices.extend(face.iter().map(|p|MeshVertex{position:(*p-center).to_array(),normal:n.to_array()}));
                for i in 1..face.len()-1 {mesh.indices.extend_from_slice(&[base,base+i as u32,base+i as u32+1]);}
            }
            mesh
        }
        RenderGeometry::Mesh {vertices,triangles,position,rotation,scale} => {
            inst.p=*position;inst.q=*rotation;inst.scale=*scale;
            let mut mesh=CpuMesh{vertices:Vec::new(),indices:Vec::new()};
            for t in triangles.iter() {
                let a=Vec3::from_array(vertices[t[0] as usize]);let b=Vec3::from_array(vertices[t[1] as usize]);let c=Vec3::from_array(vertices[t[2] as usize]);
                let n=(b-a).cross(c-a).normalize_or_zero();let base=mesh.vertices.len() as u32;
                mesh.vertices.extend([a,b,c].map(|p|MeshVertex{position:p.to_array(),normal:n.to_array()}));
                mesh.indices.extend_from_slice(&[base,base+1,base+2]);
            }
            mesh
        }
    };
    (mesh,inst)
}

fn build(shapes: &[RenderShape]) -> Geometry {
    let mut meshes:Vec<(CpuMesh,Vec<Instance>)>=Vec::new();
    let mut keys:HashMap<Vec<u8>,usize>=HashMap::new();
    for shape in shapes {
        let (mesh,inst)=shape_mesh(shape);
        if mesh.indices.is_empty() {continue;}
        let mut key=bytemuck::cast_slice(&mesh.vertices).to_vec();
        key.extend_from_slice(bytemuck::cast_slice(&mesh.indices));
        if let Some(&i)=keys.get(&key) {meshes[i].1.push(inst);} else {
            keys.insert(key,meshes.len());meshes.push((mesh,vec![inst]));
        }
    }
    let mut out=Geometry{vertices:Vec::new(),indices:Vec::new(),instances:Vec::new(),batches:Vec::new()};
    for (mesh,instances) in meshes {
        let vbase=u32::try_from(out.vertices.len()).expect("render vertex capacity");
        let istart=u32::try_from(out.indices.len()).expect("render index capacity");
        let first=u32::try_from(out.instances.len()).expect("render instance capacity");
        out.indices.extend(mesh.indices.iter().map(|i|vbase.checked_add(*i).expect("render index capacity")));
        out.vertices.extend(mesh.vertices);out.instances.extend(instances);
        out.batches.push(Batch{indices:istart..u32::try_from(out.indices.len()).unwrap(),instances:first..u32::try_from(out.instances.len()).unwrap()});
    }
    out
}

pub struct SceneDraw {
    key:Option<RenderSceneKey>, pipeline:wgpu::RenderPipeline, shadow_pipeline:wgpu::RenderPipeline, layout:wgpu::BindGroupLayout,
    buffers:Option<(wgpu::Buffer,wgpu::Buffer,wgpu::Buffer)>, batches:Vec<Batch>,
}
impl SceneDraw {
    pub fn new(device:&wgpu::Device,camera:&wgpu::BindGroupLayout,format:wgpu::TextureFormat)->Self {
        let entries=[0,1].map(|binding|wgpu::BindGroupLayoutEntry{binding,visibility:wgpu::ShaderStages::VERTEX,
            ty:wgpu::BindingType::Buffer{ty:wgpu::BufferBindingType::Storage{read_only:true},has_dynamic_offset:false,min_binding_size:None},count:None});
        let layout=device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor{label:Some("scene-bodies"),entries:&entries});
        let pl=device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor{label:Some("scene-draw"),bind_group_layouts:&[camera,&layout],push_constant_ranges:&[]});
        let shader=device.create_shader_module(wgpu::ShaderModuleDescriptor{label:Some("scene-draw"),source:wgpu::ShaderSource::Wgsl(include_str!("../shaders/render_scene.wgsl").into())});
        let attrs=wgpu::vertex_attr_array![2=>Uint32,3=>Uint32,4=>Float32x3,5=>Float32x4,6=>Float32x3];
        let buffers=[mesh::vertex_layout(),wgpu::VertexBufferLayout{array_stride:std::mem::size_of::<Instance>() as u64,step_mode:wgpu::VertexStepMode::Instance,attributes:&attrs}];
        let pipeline=device.create_render_pipeline(&wgpu::RenderPipelineDescriptor{label:Some("scene-draw"),layout:Some(&pl),
            vertex:wgpu::VertexState{module:&shader,entry_point:Some("vs_main"),compilation_options:Default::default(),buffers:&buffers},
            fragment:Some(wgpu::FragmentState{module:&shader,entry_point:Some("fs_main"),compilation_options:Default::default(),targets:&[Some(wgpu::ColorTargetState{format,blend:None,write_mask:wgpu::ColorWrites::ALL})]}),
            primitive:wgpu::PrimitiveState{cull_mode:None,..Default::default()},
            depth_stencil:Some(wgpu::DepthStencilState{format:wgpu::TextureFormat::Depth32Float,depth_write_enabled:true,depth_compare:wgpu::CompareFunction::Less,stencil:Default::default(),bias:Default::default()}),
            multisample:Default::default(),multiview:None,cache:None});
        let shadow_pipeline=device.create_render_pipeline(&wgpu::RenderPipelineDescriptor{label:Some("scene-shadow"),layout:Some(&pl),
            vertex:wgpu::VertexState{module:&shader,entry_point:Some("vs_main"),compilation_options:Default::default(),buffers:&buffers},fragment:None,
            primitive:wgpu::PrimitiveState{cull_mode:None,..Default::default()},
            depth_stencil:Some(wgpu::DepthStencilState{format:wgpu::TextureFormat::Depth32Float,depth_write_enabled:true,depth_compare:wgpu::CompareFunction::Less,stencil:Default::default(),bias:wgpu::DepthBiasState{constant:2,slope_scale:2.0,clamp:0.0}}),
            multisample:Default::default(),multiview:None,cache:None});
        Self{key:None,pipeline,shadow_pipeline,layout,buffers:None,batches:Vec::new()}
    }
    pub fn prepare(&mut self,device:&wgpu::Device,world:api::WorldId) {
        let Some(scene)=api::b3_world_render_scene(world,self.key) else{return;};
        let geometry=build(&scene.shapes);self.key=Some(scene.key);self.batches=geometry.batches;
        if self.batches.is_empty(){self.buffers=None;return;}
        let upload=|label,bytes,usage|device.create_buffer_init(&wgpu::util::BufferInitDescriptor{label:Some(label),contents:bytes,usage});
        self.buffers=Some((upload("scene-vertices",bytemuck::cast_slice(&geometry.vertices),wgpu::BufferUsages::VERTEX),
            upload("scene-indices",bytemuck::cast_slice(&geometry.indices),wgpu::BufferUsages::INDEX),
            upload("scene-instances",bytemuck::cast_slice(&geometry.instances),wgpu::BufferUsages::VERTEX)));
    }
    pub fn bind(&self,device:&wgpu::Device,state:&wgpu::Buffer,cold:&wgpu::Buffer)->wgpu::BindGroup {
        device.create_bind_group(&wgpu::BindGroupDescriptor{label:Some("scene-current-bodies"),layout:&self.layout,
            entries:&[wgpu::BindGroupEntry{binding:0,resource:state.as_entire_binding()},wgpu::BindGroupEntry{binding:1,resource:cold.as_entire_binding()}]})
    }
    pub fn draw_shadow<'a>(&'a self,pass:&mut wgpu::RenderPass<'a>,binding:&'a wgpu::BindGroup) {
        let Some((v,i,instances))=&self.buffers else{return;};
        pass.set_pipeline(&self.shadow_pipeline);pass.set_bind_group(1,binding,&[]);
        pass.set_vertex_buffer(0,v.slice(..));pass.set_vertex_buffer(1,instances.slice(..));pass.set_index_buffer(i.slice(..),wgpu::IndexFormat::Uint32);
        for batch in &self.batches {pass.draw_indexed(batch.indices.clone(),0,batch.instances.clone());}
    }
    pub fn draw<'a>(&'a self,pass:&mut wgpu::RenderPass<'a>,binding:&'a wgpu::BindGroup) {
        let Some((v,i,instances))=&self.buffers else{return;};
        pass.set_pipeline(&self.pipeline);pass.set_bind_group(1,binding,&[]);
        pass.set_vertex_buffer(0,v.slice(..));pass.set_vertex_buffer(1,instances.slice(..));pass.set_index_buffer(i.slice(..),wgpu::IndexFormat::Uint32);
        for batch in &self.batches {pass.draw_indexed(batch.indices.clone(),0,batch.instances.clone());}
    }
}

#[cfg(all(test, not(target_arch="wasm32")))]
mod tests {
    use super::*;
    use api::*;
    #[test]
    fn gpu_draw_uses_com_offsets_and_sparse_body_slots() {
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).unwrap();
        let world=b3_create_world(gpu.clone(),&b3_default_world_def());
        let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[-0.5,0.0,0.5];
        let a=b3_create_body(world,&bd);let hole=b3_create_body(world,&bd);let b=b3_create_body(world,&bd);
        let sd=b3_default_shape_def();
        b3_create_sphere_shape(a,&sd,&Sphere{center:[0.6,0.0,0.0],radius:0.1});
        b3_create_sphere_shape(b,&sd,&Sphere{center:[0.0;3],radius:0.1});
        b3_destroy_body(hole);b3_world_ensure_gpu(world);
        let (state,cold,count)=b3_world_render_buffers(world).unwrap();assert_eq!(count,3);
        let device=&gpu.device;
        let camera_layout=device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor{label:None,entries:&[wgpu::BindGroupLayoutEntry{binding:0,visibility:wgpu::ShaderStages::VERTEX,ty:wgpu::BindingType::Buffer{ty:wgpu::BufferBindingType::Uniform,has_dynamic_offset:false,min_binding_size:None},count:None}]});
        let camera=device.create_buffer_init(&wgpu::util::BufferInitDescriptor{label:None,contents:bytemuck::cast_slice(&glam::Mat4::IDENTITY.to_cols_array()),usage:wgpu::BufferUsages::UNIFORM});
        let camera_bg=device.create_bind_group(&wgpu::BindGroupDescriptor{label:None,layout:&camera_layout,entries:&[wgpu::BindGroupEntry{binding:0,resource:camera.as_entire_binding()}]});
        let mut draw=SceneDraw::new(device,&camera_layout,wgpu::TextureFormat::Rgba8Unorm);draw.prepare(device,world);
        assert_eq!(draw.batches.len(),1,"identical sphere geometry should share one instanced draw");
        let binding=draw.bind(device,&state,&cold);
        let texture=|format,usage|device.create_texture(&wgpu::TextureDescriptor{label:None,size:wgpu::Extent3d{width:128,height:128,depth_or_array_layers:1},mip_level_count:1,sample_count:1,dimension:wgpu::TextureDimension::D2,format,usage,view_formats:&[]});
        let color=texture(wgpu::TextureFormat::Rgba8Unorm,wgpu::TextureUsages::RENDER_ATTACHMENT|wgpu::TextureUsages::COPY_SRC);
        let depth=texture(wgpu::TextureFormat::Depth32Float,wgpu::TextureUsages::RENDER_ATTACHMENT);
        let cv=color.create_view(&Default::default());let dv=depth.create_view(&Default::default());
        let staging=device.create_buffer(&wgpu::BufferDescriptor{label:None,size:128*128*4,usage:wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::MAP_READ,mapped_at_creation:false});
        let mut enc=device.create_command_encoder(&Default::default());
        {
            let mut pass=enc.begin_render_pass(&wgpu::RenderPassDescriptor{label:None,color_attachments:&[Some(wgpu::RenderPassColorAttachment{view:&cv,resolve_target:None,ops:wgpu::Operations{load:wgpu::LoadOp::Clear(wgpu::Color::BLACK),store:wgpu::StoreOp::Store},depth_slice:None})],depth_stencil_attachment:Some(wgpu::RenderPassDepthStencilAttachment{view:&dv,depth_ops:Some(wgpu::Operations{load:wgpu::LoadOp::Clear(1.0),store:wgpu::StoreOp::Store}),stencil_ops:None}),timestamp_writes:None,occlusion_query_set:None});
            pass.set_bind_group(0,&camera_bg,&[]);draw.draw(&mut pass,&binding);
        }
        enc.copy_texture_to_buffer(color.as_image_copy(),wgpu::TexelCopyBufferInfo{buffer:&staging,layout:wgpu::TexelCopyBufferLayout{offset:0,bytes_per_row:Some(512),rows_per_image:Some(128)}},wgpu::Extent3d{width:128,height:128,depth_or_array_layers:1});
        let shadow=texture(wgpu::TextureFormat::Depth32Float,wgpu::TextureUsages::RENDER_ATTACHMENT|wgpu::TextureUsages::COPY_SRC);
        let shadow_view=shadow.create_view(&Default::default());
        let shadow_staging=device.create_buffer(&wgpu::BufferDescriptor{label:None,size:128*128*4,usage:wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::MAP_READ,mapped_at_creation:false});
        {
            let mut pass=enc.begin_render_pass(&wgpu::RenderPassDescriptor{label:Some("shadow-test"),color_attachments:&[],depth_stencil_attachment:Some(wgpu::RenderPassDepthStencilAttachment{view:&shadow_view,depth_ops:Some(wgpu::Operations{load:wgpu::LoadOp::Clear(1.0),store:wgpu::StoreOp::Store}),stencil_ops:None}),timestamp_writes:None,occlusion_query_set:None});
            pass.set_bind_group(0,&camera_bg,&[]);draw.draw_shadow(&mut pass,&binding);
        }
        let mut source=shadow.as_image_copy();source.aspect=wgpu::TextureAspect::DepthOnly;
        enc.copy_texture_to_buffer(source,wgpu::TexelCopyBufferInfo{buffer:&shadow_staging,layout:wgpu::TexelCopyBufferLayout{offset:0,bytes_per_row:Some(512),rows_per_image:Some(128)}},wgpu::Extent3d{width:128,height:128,depth_or_array_layers:1});
        gpu.queue.submit(Some(enc.finish()));
        let (tx,rx)=std::sync::mpsc::channel();staging.slice(..).map_async(wgpu::MapMode::Read,move|r|{tx.send(r).unwrap();});crate::sim::poll_until_idle(device);rx.recv().unwrap().unwrap();
        let data=staging.slice(..).get_mapped_range();
        let lit=|x:usize,y:usize|data[(y*128+x)*4..(y*128+x)*4+3].iter().any(|c|*c>0);
        assert!(lit(70,64),"offset sphere must appear at body origin + local center, not COM + local center");
        assert!(lit(32,64),"live body after a deleted slot must render");
        assert!(!lit(108,64),"COM offset must not be applied twice");
        drop(data);staging.unmap();
        let (tx,rx)=std::sync::mpsc::channel();shadow_staging.slice(..).map_async(wgpu::MapMode::Read,move|r|{tx.send(r).unwrap();});crate::sim::poll_until_idle(device);rx.recv().unwrap().unwrap();
        let depth_data=shadow_staging.slice(..).get_mapped_range();let depths:&[f32]=bytemuck::cast_slice(&depth_data);
        assert!(depths[64*128+70]<0.6 && depths[64*128+32]<0.6,"both sparse body slots must cast into the shadow map");
        assert_eq!(depths[64*128+108],1.0,"no phantom COM-offset shadow");
        drop(depth_data);shadow_staging.unmap();b3_destroy_world(world);
    }
}
