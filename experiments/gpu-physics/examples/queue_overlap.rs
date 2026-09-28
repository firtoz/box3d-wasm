//! Queue hardware capability test, NOT a native-frame benchmark.
//! Existing scene draw code, frozen poses, and synthetic barrier-heavy compute.
//! Run: queue_overlap [single|universal|dedicated|serial] [mixed|dominoes]
use ash::vk;
use std::{io::Write, sync::atomic::{AtomicBool, Ordering}};
use wgpu::{hal::api::Vulkan, util::DeviceExt};
pub use gpu_physics::{api, mesh, render_scene};
#[path = "../src/scene_draw.rs"]
mod scene_draw;

include!("raw_compute.rs.inc");

static ERROR: AtomicBool = AtomicBool::new(false);
const COMPUTE: &str = r#"
@group(0) @binding(0) var<storage,read_write> result:array<u32>;
var<workgroup> lanes:array<u32,64>;
@compute @workgroup_size(64) fn main(@builtin(local_invocation_index) lane:u32) {
  var x=lane+1u;
  for(var i=0u;i<4096u;i++) {
    lanes[lane]=x;
    workgroupBarrier();
    x=(x^(lanes[(lane+1u)%64u]>>3u))*1664525u+1013904223u;
    workgroupBarrier();
  }
  result[lane]=x;
}
"#;

fn main() {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("warn"))
        .format(|b,r| {if r.level()==log::Level::Error {ERROR.store(true,Ordering::Relaxed);}
            writeln!(b,"{} {}: {}",r.level(),r.target(),r.args())}).init();
    let result=pollster::block_on(run());
    assert!(!ERROR.load(Ordering::Relaxed),"validation errors");
    println!("{result}");
}

pub(crate) async fn devices(mode:&str)->(gpu_physics::sim::GpuDevice,wgpu::Device,wgpu::Queue,u32) {
    let validate=std::env::var_os("QUEUE_PROBE_VALIDATE").is_some();
    let instance=wgpu::Instance::new(&wgpu::InstanceDescriptor{backends:wgpu::Backends::VULKAN,
        flags:if validate {wgpu::InstanceFlags::debugging()}else{wgpu::InstanceFlags::empty()},..Default::default()});
    let (adapter,report)=gpu_physics::adapter::pick_adapter(&instance,None).await.unwrap();
    let mut features=wgpu::Features::TIMESTAMP_QUERY|wgpu::Features::TIMESTAMP_QUERY_INSIDE_ENCODERS;
    if adapter.features().contains(wgpu::Features::SUBGROUP) {features|=wgpu::Features::SUBGROUP;}
    let supported=adapter.limits();
    let desc=wgpu::DeviceDescriptor{label:Some("overlap-probe"),required_features:features,
        required_limits:wgpu::Limits{max_storage_buffers_per_shader_stage:10,
            max_storage_buffer_binding_size:supported.max_storage_buffer_binding_size,
            max_buffer_size:supported.max_buffer_size,
            max_compute_workgroup_storage_size:supported.max_compute_workgroup_storage_size.min(32768),
            max_compute_workgroup_size_x:supported.max_compute_workgroup_size_x,
            max_compute_invocations_per_workgroup:supported.max_compute_invocations_per_workgroup,..Default::default()},
        memory_hints:wgpu::MemoryHints::MemoryUsage,trace:wgpu::Trace::Off};
    let (gd,gq,pd,pq,family)=unsafe {
        let hal=adapter.as_hal::<Vulkan>().unwrap();
        let families=hal.shared_instance().raw_instance().get_physical_device_queue_family_properties(hal.raw_physical_device());
        assert!(families[0].queue_flags.contains(vk::QueueFlags::GRAPHICS|vk::QueueFlags::COMPUTE));
        let family=if mode=="dedicated" || mode=="serial" {
            families.iter().position(|f| f.queue_count>0 && f.queue_flags.contains(vk::QueueFlags::COMPUTE)
                && !f.queue_flags.contains(vk::QueueFlags::GRAPHICS)).expect("no dedicated compute family") as u32
        }else{0};
        if family==0 && mode!="single" {assert!(families[0].queue_count>=2);}
        assert_eq!(families[0].timestamp_valid_bits,64);
        assert_eq!(families[family as usize].timestamp_valid_bits,64);
        let extensions=hal.shared_instance().raw_instance().enumerate_device_extension_properties(hal.raw_physical_device()).unwrap();
        let calibration=[ash::khr::calibrated_timestamps::NAME,ash::ext::calibrated_timestamps::NAME].into_iter()
            .find(|&name|extensions.iter().any(|e|std::ffi::CStr::from_ptr(e.extension_name.as_ptr())==name))
            .expect("calibrated timestamps required for cross-queue intervals");
        let single=mode=="single";
        let open=hal.open_with_callback(features,&desc.memory_hints,Some(Box::new(move |args| {
            if !args.extensions.contains(&calibration) {args.extensions.push(calibration);}
            if family!=0 {args.queue_create_infos.push(vk::DeviceQueueCreateInfo::default().queue_family_index(family).queue_priorities(&[1.0]));}
            else if !single {args.queue_create_infos[0]=vk::DeviceQueueCreateInfo::default().queue_family_index(0).queue_priorities(&[1.0,1.0]);}
        }))).unwrap();
        let raw=open.device.raw_device().clone();let extensions=open.device.enabled_device_extensions().to_vec();
        let (gd,gq)=adapter.create_device_from_hal::<Vulkan>(open,&desc).unwrap();
        let (pd,pq)=if single {(gd.clone(),gq.clone())} else {
            let owner=gd.clone();
            let open=hal.device_from_raw(raw,Some(Box::new(move||drop(owner))),&extensions,features,&desc.memory_hints,
                family,if family==0 {1}else{0}).unwrap();
            adapter.create_device_from_hal::<Vulkan>(open,&desc).unwrap()
        };
        (gd,gq,pd,pq,family)
    };
    (gpu_physics::sim::GpuDevice{instance,adapter,device:pd,queue:pq,report,timestamp_queries:true,
        subgroups:features.contains(wgpu::Features::SUBGROUP)},gd,gq,family)
}

fn read(d:&wgpu::Device,q:&wgpu::Queue,source:&wgpu::Buffer,size:u64)->Vec<u8> {
    let buffer=d.create_buffer(&wgpu::BufferDescriptor{label:Some("probe-readback"),size,
        usage:wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::MAP_READ,mapped_at_creation:false});
    let mut e=d.create_command_encoder(&Default::default());e.copy_buffer_to_buffer(source,0,&buffer,0,size);q.submit([e.finish()]);
    let (tx,rx)=std::sync::mpsc::channel();buffer.slice(..).map_async(wgpu::MapMode::Read,move|r|tx.send(r).unwrap());
    d.poll(wgpu::PollType::Wait).unwrap();rx.recv().unwrap().unwrap();
    let bytes=buffer.slice(..).get_mapped_range().to_vec();buffer.unmap();bytes
}

struct Timer{query:wgpu::QuerySet,buffer:wgpu::Buffer}
impl Timer {
    fn new(d:&wgpu::Device)->Self {Self{query:d.create_query_set(&wgpu::QuerySetDescriptor{label:Some("interval"),ty:wgpu::QueryType::Timestamp,count:2}),
        buffer:d.create_buffer(&wgpu::BufferDescriptor{label:Some("interval"),size:256,usage:wgpu::BufferUsages::QUERY_RESOLVE|wgpu::BufferUsages::COPY_SRC,mapped_at_creation:false})}}
    fn resolve(&self,e:&mut wgpu::CommandEncoder){e.resolve_query_set(&self.query,0..2,&self.buffer,0);}
    fn get(&self,d:&wgpu::Device,q:&wgpu::Queue)->[u64;2]{let bytes=read(d,q,&self.buffer,16);[u64::from_ne_bytes(bytes[..8].try_into().unwrap()),u64::from_ne_bytes(bytes[8..].try_into().unwrap())]}
}

async fn run()->String {
    let args:Vec<_>=std::env::args().collect();let mode=args.get(1).map(String::as_str).unwrap_or("single");
    assert!(["single","universal","dedicated","serial"].contains(&mode));
    let scene=args.get(2).map(String::as_str).unwrap_or("mixed");assert!(["mixed","dominoes"].contains(&scene));
    let (gpu,gd,gq,family)=devices(mode).await;
    let pd=&gpu.device;let pq=&gpu.queue;
    let world=gpu_physics::scenes::build_demo_world(gpu.clone(),&gpu_physics::types::DemoConfig{
        body_count:600,body_count_explicit:false,contacts:true,jacobi:false,
        scene:if scene=="mixed"{gpu_physics::types::DemoScene::MixedStacks}else{gpu_physics::types::DemoScene::Dominoes}});
    // Host scene construction provides initial transforms without compiling a
    // physics pipeline. This probe intentionally measures synthetic compute.
    let shapes=api::b3_world_render_scene(world,None).unwrap().shapes;
    let count=shapes.iter().map(|s|s.body_slot+1).max().unwrap();
    let mut hot=vec![0f32;count as usize*28];let mut cold=vec![0f32;count as usize*16];
    for shape in &shapes {
        let slot=shape.body_slot as usize;
        let (origin,rotation)=api::b3_body_get_transform(shape.body);
        let mass=api::b3_body_get_mass_data(shape.body);
        let com=glam::Vec3::from_array(origin)+glam::Quat::from_array(rotation)*glam::Vec3::from_array(mass.center);
        hot[slot*28..slot*28+3].copy_from_slice(&com.to_array());
        hot[slot*28+3]=if mass.mass>0.0 {1.0/mass.mass}else{0.0};
        hot[slot*28+7]=f32::from_bits(match api::b3_body_get_type(shape.body) {
            api::BodyType::Static=>1,api::BodyType::Kinematic=>16,api::BodyType::Dynamic=>0});
        hot[slot*28+8..slot*28+12].copy_from_slice(&rotation);
        cold[slot*16+13..slot*16+16].copy_from_slice(&mass.center);
    }
    let hot=bytemuck::cast_slice(&hot);let cold=bytemuck::cast_slice(&cold);
    let upload=|label,bytes:&[u8],usage|gd.create_buffer_init(&wgpu::util::BufferInitDescriptor{label:Some(label),contents:bytes,usage});
    let hot=upload("frozen-hot",&hot,wgpu::BufferUsages::STORAGE);let cold=upload("frozen-cold",&cold,wgpu::BufferUsages::STORAGE);
    let (lower,upper)=api::b3_world_render_framing_bounds(world).unwrap();let lower=glam::Vec3::from_array(lower);let upper=glam::Vec3::from_array(upper);
    let target=(lower+upper)*0.5;let radius=(upper-lower).length()*1.1;
    let view=glam::Mat4::look_at_rh(target+glam::Vec3::new(0.5,0.7,0.6).normalize()*radius,target,glam::Vec3::Y);
    let projection=glam::Mat4::perspective_rh(45f32.to_radians(),1920.0/1080.0,0.05,radius*8.0);
    let camera=upload("camera",bytemuck::cast_slice(&(projection*view).to_cols_array()),wgpu::BufferUsages::UNIFORM);
    let layout=gd.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor{label:Some("camera"),entries:&[wgpu::BindGroupLayoutEntry{binding:0,visibility:wgpu::ShaderStages::VERTEX,
        ty:wgpu::BindingType::Buffer{ty:wgpu::BufferBindingType::Uniform,has_dynamic_offset:false,min_binding_size:None},count:None}]});
    let camera_bg=gd.create_bind_group(&wgpu::BindGroupDescriptor{label:Some("camera"),layout:&layout,entries:&[wgpu::BindGroupEntry{binding:0,resource:camera.as_entire_binding()}]});
    let mut draw=scene_draw::SceneDraw::new(&gd,&layout,wgpu::TextureFormat::Rgba8Unorm);draw.prepare(&gd,world);let bodies=draw.bind(&gd,&hot,&cold);
    let texture=|format,usage|gd.create_texture(&wgpu::TextureDescriptor{label:Some("offscreen"),size:wgpu::Extent3d{width:1920,height:1080,depth_or_array_layers:1},mip_level_count:1,sample_count:1,
        dimension:wgpu::TextureDimension::D2,format,usage,view_formats:&[]});
    let color=texture(wgpu::TextureFormat::Rgba8Unorm,wgpu::TextureUsages::RENDER_ATTACHMENT|wgpu::TextureUsages::COPY_SRC);let view=color.create_view(&Default::default());
    let depth=texture(wgpu::TextureFormat::Depth32Float,wgpu::TextureUsages::RENDER_ATTACHMENT).create_view(&Default::default());
    let compute=unsafe{RawCompute::new(pd,pq,family)};
    let gt=Timer::new(&gd);let period=pq.get_timestamp_period() as f64;
    assert_eq!(pq.get_timestamp_period(),gq.get_timestamp_period());
    let semaphore=unsafe{let hal=pd.as_hal::<Vulkan>().unwrap();let mut ty=vk::SemaphoreTypeCreateInfo::default().semaphore_type(vk::SemaphoreType::TIMELINE);
        hal.raw_device().create_semaphore(&vk::SemaphoreCreateInfo::default().push_next(&mut ty),None).unwrap()};
    let mut rows=Vec::new();
    for iteration in 0..110 {
        let mut ge=gd.create_command_encoder(&Default::default());
        {let mut pass=ge.begin_render_pass(&wgpu::RenderPassDescriptor{label:Some("existing-scene-draw"),
            color_attachments:&[Some(wgpu::RenderPassColorAttachment{view:&view,resolve_target:None,depth_slice:None,ops:wgpu::Operations{load:wgpu::LoadOp::Clear(wgpu::Color::BLACK),store:wgpu::StoreOp::Store}})],
            depth_stencil_attachment:Some(wgpu::RenderPassDepthStencilAttachment{view:&depth,depth_ops:Some(wgpu::Operations{load:wgpu::LoadOp::Clear(1.0),store:wgpu::StoreOp::Store}),stencil_ops:None}),
            timestamp_writes:Some(wgpu::RenderPassTimestampWrites{query_set:&gt.query,beginning_of_pass_write_index:Some(0),end_of_pass_write_index:Some(1)}),occlusion_query_set:None});
            pass.set_bind_group(0,&camera_bg,&[]);draw.draw(&mut pass,&bodies);}
        gt.resolve(&mut ge);let ge=ge.finish();
        let host_start=std::time::Instant::now();
        unsafe{compute.submit(semaphore,iteration+1);}
        if mode=="serial" || mode=="single" {unsafe{let hal=gq.as_hal::<Vulkan>().unwrap();let sems=[semaphore];let values=[iteration+1];let stages=[vk::PipelineStageFlags::ALL_COMMANDS];
            let mut time=vk::TimelineSemaphoreSubmitInfo::default().wait_semaphore_values(&values);
            hal.raw_device().queue_submit(hal.as_raw(),&[vk::SubmitInfo::default().wait_semaphores(&sems).wait_dst_stage_mask(&stages).push_next(&mut time)],vk::Fence::null()).unwrap();}}
        gq.submit([ge]);let p=unsafe{compute.finish()};gd.poll(wgpu::PollType::Wait).unwrap();
        let host_completed_ms=host_start.elapsed().as_secs_f64()*1000.0;
        let g=gt.get(&gd,&gq);assert!(p[1]>p[0] && g[1]>g[0]);
        if iteration>=10 {let overlap=p[1].min(g[1]).saturating_sub(p[0].max(g[0]));
            if mode=="serial" || mode=="single" {assert_eq!(overlap,0,"serialized control must not overlap");}
            rows.push(format!("[{}, {}, {}, {}, {:.6}, {:.6}, {:.6}, {:.6}, {:.6}]",p[0],p[1],g[0],g[1],
                (p[1]-p[0]) as f64*period/1e6,(g[1]-g[0]) as f64*period/1e6,overlap as f64*period/1e6,(p[1].max(g[1])-p[0].min(g[0])) as f64*period/1e6,host_completed_ms));}
    }
    let actual=unsafe{compute.values()};let mut expected:[u32;64]=std::array::from_fn(|i|i as u32+1);
    for _ in 0..4096 {let old=expected;for i in 0..64 {expected[i]=(old[i]^(old[(i+1)%64]>>3)).wrapping_mul(1664525).wrapping_add(1013904223);}}
    assert_eq!(actual,expected,"compute output");
    let image=gd.create_buffer(&wgpu::BufferDescriptor{label:Some("image-check"),size:1920*1080*4,usage:wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::MAP_READ,mapped_at_creation:false});
    let mut e=gd.create_command_encoder(&Default::default());e.copy_texture_to_buffer(color.as_image_copy(),wgpu::TexelCopyBufferInfo{buffer:&image,layout:wgpu::TexelCopyBufferLayout{offset:0,bytes_per_row:Some(1920*4),rows_per_image:Some(1080)}},wgpu::Extent3d{width:1920,height:1080,depth_or_array_layers:1});gq.submit([e.finish()]);
    let (tx,rx)=std::sync::mpsc::channel();image.slice(..).map_async(wgpu::MapMode::Read,move|r|tx.send(r).unwrap());gd.poll(wgpu::PollType::Wait).unwrap();rx.recv().unwrap().unwrap();
    let mapping=image.slice(..).get_mapped_range();let visible=mapping.chunks_exact(4).filter(|p|p[0]!=0||p[1]!=0||p[2]!=0).count();assert!(visible>1000,"empty graphics workload");
    let hash=mapping.iter().fold(14695981039346656037u64,|h,&b|(h^b as u64).wrapping_mul(1099511628211));drop(mapping);image.unmap();
    unsafe{pd.as_hal::<Vulkan>().unwrap().raw_device().destroy_semaphore(semaphore,None);}
    drop(compute);
    api::b3_destroy_world(world);
    format!("{{\"status\":\"pass\",\"mode\":\"{mode}\",\"scene\":\"{scene}\",\"compute_family\":{family},\"calibrated_timestamps\":true,\"visible_pixels\":{visible},\"image_hash\":\"{hash:016x}\",\"physics_workload\":false,\"native_frame_benchmark\":false,\"columns\":[\"p_begin\",\"p_end\",\"g_begin\",\"g_end\",\"compute_ms\",\"graphics_ms\",\"overlap_ms\",\"envelope_ms\",\"host_completed_ms\"],\"rows\":[{}]}}",rows.join(","))
}
