//! Host configuration and acceleration state consumed by later queries/steps.
use super::*;

pub(super) fn capture(w:&WorldInner, world0:u16) -> Result<Value,String> {
    let def=&w.def;
    let mut definition=fields!(def;gravity,hit_event_threshold,contact_hertz,contact_damping_ratio,contact_speed,maximum_linear_speed,restitution_threshold);
    definition.insert("enable_sleep".into(),json!(def.enable_sleep));
    definition.insert("enable_continuous".into(),json!(def.enable_continuous));
    definition.insert("capacity".into(),json!(fields!(&def.capacity;static_shape_count,dynamic_shape_count,static_body_count,dynamic_body_count,contact_count)));
    // Only pos/rot/origin are read from this snapshot by host CCD. Keep physical
    // slot order and length, including holes, because indexing/length gates CCD.
    let starts=w.step_start_bodies.iter().map(|b|Ok(json!(fields!(b;pos,rot,origin,origin_valid))))
        .collect::<Result<Vec<_>,String>>()?;
    let topology=w.cached_topology.map(|t|json!({"shape_count":t.shape_count,"mesh_shapes":t.mesh_shapes,"joints":t.joints,
        "non_dynamic_shapes":t.non_dynamic_shapes,"skip_general_static_sort":t.skip_general_static_sort,
        "static_degree_two_proof":t.static_degree_two_proof,"one_group_pair_ok":t.one_group_pair_ok}));
    let capabilities=w.scene_capabilities.map(|c|json!({"pending_forces":c.pending_forces,"shape_events":c.shape_events,
        "ccd_shapes":c.ccd_shapes,"component_bodies":c.component_bodies,"has_bullet":c.has_bullet}));
    let mirrors=w.bodies.iter().map(|slot| {
        let Some(host)=slot else {return Ok(Value::Null);};
        let b=&host.gpu;
        Ok(json!({"identity":host.creation_ordinal,"host_epoch":host.host_epoch,"generation":host.generation,"has_shape":host.has_shape,
            "gpu":fields!(b;pos,inv_mass,vel,kind,half,flags,rot,omega,restitution,inv_inertia,friction,gravity_scale,
                linear_damping,angular_damping,rolling,dp,sleep_time,dq,island_id,sleep_velocity,sleep_threshold,origin,origin_valid)}))
    }).collect::<Result<Vec<_>,String>>()?;
    let shape_id=|id:ShapeId|json!({"identity":id.index1,"world_matches":id.world0==world0,
        "live":id.index1>0 && w.shapes.get(id.index1 as usize-1).and_then(Option::as_ref).is_some_and(|s|s.generation==id.generation)});
    let query=if let Some(index)=&w.query_index {
        let mut geometries=Vec::new();let mut geometry_ids=HashMap::new();
        let shapes=index.shapes.iter().map(|s| {
            let mesh_nodes=s.mesh_nodes.iter().map(|n|Ok(json!(fields!(n;lower,upper,data,triangle_offset))))
                .collect::<Result<Vec<_>,String>>()?;
            let geometry=json!({"points":s.local_points.trace()?,"planes":s.hull_planes.trace()?,"topology":s.hull_topology.trace()?,
                "triangles":s.mesh_triangles.trace()?,"triangle_ids":s.mesh_triangle_ids.trace()?,"mesh_nodes":mesh_nodes});
            let encoded=serde_json::to_string(&geometry).map_err(|e|e.to_string())?;
            let next=geometries.len();let geometry_id=*geometry_ids.entry(encoded).or_insert_with(||{geometries.push(geometry);next});
            let instance=match s.mesh_instance {Some(i)=>json!(fields!(i;position,rotation,scale)),None=>Value::Null};
            Ok(json!({"id":shape_id(s.id),"public_id":shape_id(s.public_id),"child_index":s.child_index,
                "compound_material_indices":s.compound_material_indices,"kind":s.kind,"geometry":geometry_id,
                "body_origin":s.body_origin.trace()?,"body_rotation":s.body_rotation.trace()?,"local_center":s.local_center.trace()?,
                "radius":s.radius.trace()?,"half_extents":s.half_extents.trace()?,"mesh_instance":instance,
                "filter":[s.filter.category_bits,s.filter.mask_bits],"group_index":s.filter.group_index,
                "friction":s.friction.trace()?,"restitution":s.restitution.trace()?,"user_material_id":s.user_material_id}))
        }).collect::<Result<Vec<_>,String>>()?;
        let nodes=index.diagnostic_nodes().into_iter().map(|(lower,upper,left,right)|Ok(json!({
            "lower":lower.trace()?,"upper":upper.trace()?,"left":left,"right":right}))).collect::<Result<Vec<_>,String>>()?;
        json!({"topology":index.topology,"state":index.state,"shapes":shapes,"geometries":geometries,"groups":index.groups.as_ref(),"nodes":nodes})
    } else {Value::Null};
    Ok(json!({"body_mirrors":mirrors,"snapshot_epoch":snapshot_epoch(w),"definition":definition,"step_start_bodies":starts,"cached_topology":topology,"scene_capabilities":capabilities,"query_index":query,
        "last_substep_h":w.last_substep_h.trace()?,"reaction_inv_h":w.reaction_inv_h.trace()?,"contact_recycle_distance":w.contact_recycle_distance.trace()?,
        "enable_contacts":w.enable_contacts,"jacobi":w.jacobi,"pose_export_live":w.pose_export_live,"physics_invalid":w.physics_invalid,
        "gpu_failure":w.gpu_fail.as_ref().map(|s|s.to_string_lossy().into_owned()),"gpu_mirror_stale":w.gpu_mirror_stale,
        "diagnostic_flags_override":w.diagnostic_flags_override,"component_tgs_requested":w.component_tgs_requested,"gpu_ccd_requested":w.gpu_ccd_requested,
        "gpu_resident_requested":w.gpu_resident_requested,"automatic_pose_snapshots":w.automatic_pose_snapshots,"gpu_idle_requested":w.gpu_idle_requested,
        "gpu_ccd_cache_key":w.gpu_ccd_cache_key,
        // Code identity and callback-owned context are external inputs. Pointer
        // addresses are incidental; presence is still meaningful world state.
        "callbacks_present":{"custom_filter":w.custom_filter_callback.is_some(),"pre_solve":w.pre_solve_callback.is_some(),
            "friction":w.friction_callback.is_some(),"restitution":w.restitution_callback.is_some()}}))
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn trace_captures_host_configuration_query_cache_and_ccd_history() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[0.0,3.0,0.0];
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        {
            let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
            // Exercise host CCD history on either compiled backend.
            w.gpu_resident_requested=false;w.gpu_ccd_requested=false;
        }
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
        {
            let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
            refresh_query_index(w,world.index1);
            let baseline=capture(w,world.index1).unwrap();
            assert_eq!(baseline["step_start_bodies"].as_array().unwrap().len(),1);
            assert_eq!(baseline["query_index"]["shapes"].as_array().unwrap().len(),1);
            assert!(!baseline["query_index"]["nodes"].as_array().unwrap().is_empty());
            assert!(!baseline["query_index"]["geometries"][0]["points"].as_array().unwrap().is_empty());
            let velocity=w.bodies[0].as_ref().unwrap().gpu.vel;
            w.bodies[0].as_mut().unwrap().gpu.vel[0]+=0.5;
            assert_ne!(baseline["body_mirrors"],capture(w,world.index1).unwrap()["body_mirrors"]);
            w.bodies[0].as_mut().unwrap().gpu.vel=velocity;
            let threshold=w.def.hit_event_threshold;w.def.hit_event_threshold+=1.0;
            assert_ne!(baseline["definition"],capture(w,world.index1).unwrap()["definition"]);
            w.def.hit_event_threshold=threshold;
            let start=w.step_start_bodies[0].pos;w.step_start_bodies[0].pos[0]+=0.25;
            assert_ne!(baseline["step_start_bodies"],capture(w,world.index1).unwrap()["step_start_bodies"]);
            w.step_start_bodies[0].pos=start;
            let old_points=w.query_index.as_ref().unwrap().shapes[0].local_points.clone();
            Arc::make_mut(&mut w.query_index.as_mut().unwrap().shapes[0].local_points)[0][0]+=0.125;
            assert_ne!(baseline["query_index"]["geometries"],capture(w,world.index1).unwrap()["query_index"]["geometries"]);
            w.query_index.as_mut().unwrap().shapes[0].local_points=old_points;
            let saved=w.cached_topology.unwrap();
            w.cached_topology.as_mut().unwrap().one_group_pair_ok=!saved.one_group_pair_ok;
            assert_ne!(baseline["cached_topology"],capture(w,world.index1).unwrap()["cached_topology"]);
            w.cached_topology=Some(saved);
            w.step_start_bodies[0].rot[0]=f32::NAN;
            assert!(capture(w,world.index1).is_err());
        }
        b3_destroy_world(world);
    }
}
