use super::*;
use crate::api::*;
use crate::types::BodyStateGpu;

fn close<const N: usize>(a: [f32; N], b: [f32; N]) {
    for i in 0..N { assert!((a[i]-b[i]).abs() <= 1e-5, "component{i}: {a:?} != {b:?}"); }
}

fn read_pose_words(gpu: &GpuDevice, buffer: &wgpu::Buffer, count: usize) -> Vec<BodyStateGpu> {
    let size=(count*std::mem::size_of::<BodyStateGpu>()) as u64;
    let staging=gpu.device.create_buffer(&wgpu::BufferDescriptor {
        label:Some("coordinate-frame-pose-check"),size,
        usage:wgpu::BufferUsages::MAP_READ|wgpu::BufferUsages::COPY_DST,mapped_at_creation:false,
    });
    let mut encoder=gpu.device.create_command_encoder(&wgpu::CommandEncoderDescriptor::default());
    encoder.copy_buffer_to_buffer(buffer,0,&staging,0,size);gpu.queue.submit(Some(encoder.finish()));
    let slice=staging.slice(..);let(tx,rx)=std::sync::mpsc::channel();
    slice.map_async(wgpu::MapMode::Read,move|result|{tx.send(result).unwrap();});
    crate::sim::poll_until_idle(&gpu.device);rx.recv().unwrap().unwrap();
    let bytes=slice.get_mapped_range();let values=bytemuck::cast_slice::<u8,BodyStateGpu>(&bytes).to_vec();
    drop(bytes);staging.unmap();values
}

fn check_publication(w: WorldId) -> Vec<BodyStateGpu> {
    b3_world_gpu_wait_with_mirror(w);
    with_world(w,|world| {
        let sim=world.sim.as_ref().unwrap();
        let local=read_pose_words(&world.gpu,sim.current_bodies_buffer(),world.bodies.len());
        let published=read_pose_words(&world.gpu,sim.render_bodies_buffer(),world.bodies.len());
        for (slot,(a,b)) in local.iter().zip(&published).enumerate() {
            let mut expected=*a;expected.pos=global_point(world,expected.pos);
            if expected.origin_valid!=0 {expected.origin=global_point(world,expected.origin);}
            assert_eq!(bytemuck::bytes_of(&expected),bytemuck::bytes_of(b),"28-word pose publication slot{slot}");
            if let Some(cpu)=world.bodies[slot].as_ref() {close(cpu.gpu.pos,a.pos);}
        }
        local
    }).unwrap()
}

#[test]
fn far_pose_publication_mirror_edits_capacity_reuse_and_recreation() {
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();let mut reference=Vec::new();
    for (case,offset) in [[0.0;3],[1e6,0.0,-1e6]].into_iter().enumerate() {
        let(w,b,s)=free_world(gpu.clone(),offset);
        b3_body_set_linear_velocity(b,[-0.559923053,0.745658755,-0.541612029]);
        b3_body_set_angular_velocity(b,[0.1,0.2,0.3]);b3_world_ensure_gpu(w);
        check_publication(w);
        for frame in 1..=50 {
            if frame==11 {
                b3_body_set_transform(b,translated(offset,[0.75,3.0,1.0]),[0.0,0.0,0.0,1.0]);
                b3_world_ensure_gpu(w);let states=check_publication(w);
                assert_eq!(states[(b.index1-1) as usize].origin_valid,1);
            }
            if frame==21 {
                let old=with_world_no_sync(w,|world|world.sim.as_ref().unwrap().caps.bodies).unwrap();
                assert!(old<=1024,"bounded capacity fixture");
                for _ in 0..old {let mut bd=b3_default_body_def();bd.position=offset;b3_create_body(w,&bd);}
                b3_world_ensure_gpu(w);
                assert!(with_world_no_sync(w,|world|world.sim.as_ref().unwrap().caps.bodies).unwrap()>old);
                check_publication(w);
            }
            b3_world_step_gpu(w,1.0/60.0,4);let states=check_publication(w);
            let words:Vec<u8>=bytemuck::bytes_of(&states[(b.index1-1) as usize]).to_vec();
            if case==0 {reference.push(words);}else {assert_eq!(words,reference[frame-1],"internal precision frame{frame}");}
            assert!(b3_body_is_valid(b)&&b3_shape_is_valid(s));assert!(!b3_world_physics_invalid(w));
        }
        b3_destroy_body(b);let mut bd=b3_default_body_def();bd.position=translated(offset,[1.0,4.0,1.0]);
        let replacement=b3_create_body(w,&bd);assert_eq!(b.index1,replacement.index1);
        assert_ne!(b.generation,replacement.generation);b3_world_ensure_gpu(w);check_publication(w);
        close(b3_body_get_position(replacement),bd.position);b3_destroy_world(w);
        let fresh=b3_create_world(gpu.clone(),&b3_default_world_def());
        let body=b3_create_body(fresh,&b3_default_body_def());close(b3_body_get_position(body),[0.0;3]);
        with_world_no_sync(fresh,|world|assert_eq!(world.coordinate_origin,[0.0;3]));b3_destroy_world(fresh);
    }
}

#[test]
fn far_ccd_landing_preserves_original_support_limits() {
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
    for offset in [[0.0;3],[1e6,0.0,-1e6]] {
        let mut wd=b3_default_world_def();wd.gravity=[0.0;3];wd.enable_sleep=false;
        let w=b3_create_world(gpu.clone(),&wd);with_world_mut_no_sync(w,|world|world.gpu_ccd_requested=false);
        let mut bd=b3_default_body_def();bd.position=translated(offset,[0.0,-0.5,0.0]);
        let ground=b3_create_body(w,&bd);let sd=b3_default_shape_def();
        b3_create_hull_shape(ground,&sd,&b3_make_box_hull(20.0,0.5,20.0));
        bd.body_type=BodyType::Dynamic;bd.position=translated(offset,[0.0,0.522,0.0]);
        let b=b3_create_body(w,&bd);b3_create_hull_shape(b,&sd,&b3_make_box_hull(0.5,0.5,0.5));
        b3_body_set_linear_velocity(b,[0.0,-2.66667,0.0]);b3_world_step(w,1.0/60.0,4);
        let landing=b3_body_get_position(b)[1];assert!((landing-0.505).abs()<0.001,"landing{landing}");
        b3_world_step(w,1.0/60.0,4);let next=b3_body_get_position(b)[1];
        assert!(next>=0.495,"support{next}");assert!(!b3_world_physics_invalid(w));b3_destroy_world(w);
    }
}

#[test]
fn far_pre_solve_and_hit_events_publish_world_points() {
    unsafe extern "C" fn observe(_:ShapeId,_:ShapeId,point:Vec3,normal:Vec3,context:*mut std::ffi::c_void)->bool {
        let rows=unsafe{&mut *(context as *mut Vec<(Vec3,Vec3)>)};rows.push((point,normal));true
    }
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
    for offset in [[0.0;3],[1e6,0.0,-1e6]] {
        let w=b3_create_world(gpu.clone(),&b3_default_world_def());
        let mut bd=b3_default_body_def();bd.position=translated(offset,[0.0,-0.5,0.0]);
        let ground=b3_create_body(w,&bd);let mut sd=b3_default_shape_def();
        sd.enable_hit_events=true;sd.enable_pre_solve_events=true;
        let floor=b3_create_hull_shape(ground,&sd,&b3_make_box_hull(20.0,0.5,20.0));
        bd.body_type=BodyType::Dynamic;bd.position=translated(offset,[0.0,1.5,0.0]);
        let body=b3_create_body(w,&bd);let shape=b3_create_hull_shape(body,&sd,&b3_make_box_hull(0.5,0.5,0.5));
        let mut rows:Vec<(Vec3,Vec3)>=Vec::new();
        b3_world_set_pre_solve_callback(w,Some(observe),&mut rows as *mut _ as *mut std::ffi::c_void);
        let mut hits=0;
        for _ in 0..60 {
            b3_world_step(w,1.0/60.0,4);let(_,_,_,_,ptr,count)=b3_world_contact_event_ptrs(w);
            for e in unsafe{std::slice::from_raw_parts(ptr,count as usize)} {
                assert!((e.shape_id_a==floor&&e.shape_id_b==shape)||(e.shape_id_a==shape&&e.shape_id_b==floor));
                assert!((e.point[0]-offset[0]).abs()<=1.0&&(e.point[2]-offset[2]).abs()<=1.0);
                assert!(e.point.iter().all(|v|v.is_finite()));hits+=1;
            }
        }
        assert!(hits>0,"actual hit events required");assert!(!rows.is_empty(),"actual pre-solve required");
        for (point,normal) in rows {
            let point:[f32;3]=point.into();let normal:[f32;3]=normal.into();
            assert!((point[0]-offset[0]).abs()<=1.0&&(point[2]-offset[2]).abs()<=1.0,"pre-solve offset{offset:?} point{point:?} normal{normal:?}");
            assert!(point.iter().chain(normal.iter()).all(|v|v.is_finite()));
            assert!((normal.iter().map(|v|v*v).sum::<f32>()-1.0).abs()<=1e-5);
        }
        assert!(!b3_world_physics_invalid(w));b3_world_set_pre_solve_callback(w,None,std::ptr::null_mut());b3_destroy_world(w);
    }
}
fn translated(offset: [f32;3], local: [f32;3]) -> [f32;3] {
    std::array::from_fn(|i| offset[i]+local[i])
}
fn free_world(gpu: GpuDevice, offset: [f32;3]) -> (WorldId, BodyId, ShapeId) {
    let mut wd=b3_default_world_def(); wd.gravity=[0.0;3];
    let w=b3_create_world(gpu,&wd);
    let mut bd=b3_default_body_def(); bd.position=offset;
    b3_create_body(w,&bd); // first static body fixes the coordinate frame
    bd.body_type=BodyType::Dynamic; bd.enable_sleep=false;
    bd.position=translated(offset,[0.5,3.0,1.0]);
    let b=b3_create_body(w,&bd);
    let s=b3_create_sphere_shape(b,&b3_default_shape_def(),&Sphere{center:[0.25,0.0,0.0],radius:0.5});
    (w,b,s)
}

#[test]
fn far_public_points_queries_forces_targets_explosions_and_identity() {
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut reference=None;
    for offset in [[0.0;3],[1e6,0.0,-1e6]] {
        let (w,b,s)=free_world(gpu.clone(),offset);
        let origin=translated(offset,[0.5,3.0,1.0]);
        let center=translated(offset,[0.75,3.0,1.0]);
        close(b3_body_get_position(b),origin);
        close(b3_body_get_world_center(b),center);
        close(b3_body_get_world_point(b,[1.0,0.0,0.0]),translated(offset,[1.5,3.0,1.0]));
        close(b3_body_get_local_point(b,translated(offset,[1.5,3.0,1.0])),[1.0,0.0,0.0]);
        let bounds=b3_shape_get_aabb(s);
        close(bounds.lower_bound,translated(offset,[0.23,2.48,0.48]));
        close(bounds.upper_bound,translated(offset,[1.27,3.52,1.52]));
        let hit=unsafe{b3_world_cast_ray_closest(w,translated(offset,[0.75,5.0,1.0]),[0.0,-4.0,0.0],b3_default_query_filter())};
        assert!(hit.hit);assert_eq!(hit.shape_id,s);
        assert!((hit.fraction-0.375).abs()<=1e-5);
        close(hit.point,translated(offset,[0.75,3.5,1.0]));close(hit.normal,[0.0,1.0,0.0]);
        close(b3_body_get_local_point(b,hit.point),[0.25,0.5,0.0]); // actual picking anchor
        close(b3_shape_get_closest_point(s,translated(offset,[0.75,5.0,1.0])),hit.point);
        let m=b3_body_get_mass(b);
        b3_body_apply_linear_impulse(b,[m,0.0,0.0],translated(offset,[0.75,3.5,1.0]),true);
        close(b3_body_get_linear_velocity(b),[1.0,0.0,0.0]);
        close(b3_body_get_angular_velocity(b),[0.0,0.0,-5.0]); // solid sphere I=0.1m
        close(b3_body_get_local_point_velocity(b,[0.0;3]),[1.0,1.25,0.0]);
        close(b3_body_get_world_point_velocity(b,origin),[1.0,1.25,0.0]);
        b3_body_set_target_transform(b,translated(offset,[0.75,3.0,1.0]),[0.0,0.0,0.0,1.0],1.0,true);
        close(b3_body_get_linear_velocity(b),[0.25,0.0,0.0]);close(b3_body_get_angular_velocity(b),[0.0;3]);
        b3_body_set_linear_velocity(b,[0.0;3]);
        b3_body_apply_force(b,[6.0*m,0.0,0.0],translated(offset,[0.75,3.5,1.0]),true);
        b3_world_step(w,1.0/60.0,4);
        close(b3_body_get_linear_velocity(b),[0.1,0.0,0.0]);
        close(b3_body_get_angular_velocity(b),[0.0,0.0,-0.5]);
        let (ptr,count)=b3_world_body_event_ptrs(w);
        let events=unsafe{std::slice::from_raw_parts(ptr,count as usize)};
        let e=events.iter().find(|e|e.body_id==b).expect("move event");
        close(e.transform.p,b3_body_get_position(b));
        b3_body_set_transform(b,origin,[0.0,0.0,0.0,1.0]);
        b3_body_set_linear_velocity(b,[0.0;3]);b3_body_set_angular_velocity(b,[0.0;3]);
        let mut explosion=b3_default_explosion_def();
        explosion.position=translated(offset,[0.0,3.0,1.0]);explosion.radius=2.0;explosion.impulse_per_area=100.0;
        b3_world_explode(w,&explosion);
        let outcome=(b3_body_get_linear_velocity(b),b3_body_get_angular_velocity(b));
        assert!(outcome.0[0]>0.0);
        if let Some((v,omega))=reference {close(outcome.0,v);close(outcome.1,omega);} else {reference=Some(outcome);}
        b3_destroy_body(b);assert!(!b3_body_is_valid(b));assert!(!b3_shape_is_valid(s));
        assert_eq!(b3_body_get_position(b),[0.0;3],"invalid handle must not return offset");
        let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=origin;
        let replacement=b3_create_body(w,&bd);
        assert_eq!(replacement.index1,b.index1);assert_ne!(replacement.generation,b.generation);
        close(b3_body_get_position(replacement),origin);
        b3_destroy_world(w);assert!(!b3_body_is_valid(replacement));
    }
}

#[test]
fn moving_far_public_pose_ray_and_center_impulse_are_consistent() {
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
    for offset in [[0.0;3],[1e6,0.0,-1e6]] {
        let(w,b,s)=free_world(gpu.clone(),offset);
        b3_body_set_linear_velocity(b,[0.54,0.0,0.0]);b3_world_step(w,1.0/60.0,4);
        let position=b3_body_get_position(b);let center=b3_body_get_world_center(b);
        let origin=translated(offset,[0.75,5.0,1.0]);let direction=[0.0,-4.0,0.0];
        let gpu_hit=unsafe{b3_world_cast_ray_closest(w,origin,direction,b3_default_query_filter())};
        let host_hit=b3_shape_ray_cast(s,origin,direction);
        let local=b3_body_get_local_point(b,position);
        let internal=with_world(w,|world|body_ref(world,b).unwrap().gpu.pos).unwrap();
        let mass=b3_body_get_mass(b);b3_body_apply_linear_impulse(b,[0.0,mass,0.0],center,true);
        let omega=b3_body_get_angular_velocity(b);
        println!("movingfar offset{offset:?} internalCOM{internal:?} publicorigin{position:?} publicCOM{center:?} localOfPublicOrigin{local:?} gpuRay{gpu_hit:?} hostRay{host_hit:?} omegaAfterCenterImpulse{omega:?}");
        assert!(gpu_hit.hit&&host_hit.hit);assert_eq!(gpu_hit.shape_id,s);
        let ray_error=(gpu_hit.fraction-host_hit.fraction).abs();
        let normal_error=(0..3).map(|i|(gpu_hit.normal[i]-host_hit.normal[i]).abs()).fold(0.0f32,f32::max);
        let local_error=local.iter().map(|v|v.abs()).fold(0.0f32,f32::max);
        let center_error=omega.iter().map(|v|v.abs()).fold(0.0f32,f32::max);
        // A ray must leave the existing solver scratch ABI safe for the next step.
        b3_body_set_angular_velocity(b,[0.0;3]);
        b3_body_apply_force(b,[0.0,mass*6.0,0.0],b3_body_get_world_center(b),true);
        b3_world_step(w,1.0/60.0,4);
        let force_omega=b3_body_get_angular_velocity(b);
        close(force_omega,[0.0;3]);
        close(b3_body_get_world_point(b,[0.0;3]),b3_body_get_position(b));
        close(b3_body_get_local_point(b,b3_body_get_position(b)),[0.0;3]);
        let g=unsafe{b3_world_cast_ray_closest(w,origin,direction,b3_default_query_filter())};
        let h=b3_shape_ray_cast(s,origin,direction);
        assert!(g.hit&&h.hit);assert_eq!(g.shape_id,s);close(g.normal,h.normal);
        assert!((g.fraction-h.fraction).abs()<=1e-5);
        println!("movingfar afterNextStep forceOmega{force_omega:?} gpuRay{g:?} hostRay{h:?}");
        b3_destroy_world(w);
        assert!(ray_error<=1e-5&&normal_error<=1e-5&&local_error<=1e-5&&center_error<=1e-5,
            "moving public boundary rayfraction{ray_error} normal{normal_error} localorigin{local_error} centertorque{center_error}");
    }
}
