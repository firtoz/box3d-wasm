//! Remaining public event payloads and sensor history, without user pointers.
use super::*;

pub(super) fn capture(w: &WorldInner, world0: u16) -> Result<Value,String> {
    let shape = |id:ShapeId| json!({"identity":id.index1,"world_matches":id.world0==world0,
        "live":id.index1>0 && w.shapes.get(id.index1 as usize-1).and_then(Option::as_ref).is_some_and(|s|s.generation==id.generation)});
    let pair = |a,b|json!([shape(a),shape(b)]);
    let mut overlaps:Vec<_>=w.sensor_overlaps.iter().collect();overlaps.sort_unstable_by_key(|(s,_)|**s);
    let moves=w.body_move_events.iter().map(|e| {
        let body=e.body_id;
        let live=w.bodies.get(body.index1.saturating_sub(1) as usize).and_then(Option::as_ref)
            .filter(|b|body.index1>0 && body.world0==world0 && b.generation==body.generation);
        Ok(json!({"body":{"slot":body.index1,"generation":body.generation,"world_matches":body.world0==world0,
            "identity":live.map(|b|b.creation_ordinal)},"position":e.transform.p.trace()?,
            "rotation":e.transform.q.trace()?,"fell_asleep":e.fell_asleep}))
    }).collect::<Result<Vec<_>,String>>()?;
    let joints=w.joint_events.iter().map(|e| {
        let id=e.joint_id;
        json!({"identity":id.index1,"world_matches":id.world0==world0,
            "live":id.index1>0 && w.joint_meta.get(id.index1 as usize-1).and_then(Option::as_ref)
                .is_some_and(|j|j.generation==id.generation)})
    }).collect::<Vec<_>>();
    Ok(json!({"sensor_overlaps":overlaps.into_iter().map(|(s,visitors)|json!({"sensor":s,"visitors":visitors.iter().copied().map(shape).collect::<Vec<_>>()})).collect::<Vec<_>>(),
        "sensor_begins":w.sensor_begin_events.iter().map(|e|pair(e.sensor_shape_id,e.visitor_shape_id)).collect::<Vec<_>>(),
        "sensor_ends":w.sensor_end_events.iter().map(|e|pair(e.sensor_shape_id,e.visitor_shape_id)).collect::<Vec<_>>(),
        "deferred_sensor_ends":w.deferred_sensor_end_events.iter().map(|e|pair(e.sensor_shape_id,e.visitor_shape_id)).collect::<Vec<_>>(),
        "continuous_sensor_hits":w.continuous_sensor_hits.iter().map(|&(a,b)|pair(a,b)).collect::<Vec<_>>(),
        "body_moves":moves,"joint_events":joints}))
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn trace_tracks_continuous_sensor_crossing_and_clear() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let mut wd=crate::api::b3_default_world_def();wd.gravity=[0.0;3];
        let world=b3_create_world(gpu,&wd);
        let anchor=b3_create_body(world,&crate::api::b3_default_body_def());
        let mut sd=crate::api::b3_default_shape_def();sd.is_sensor=true;sd.enable_sensor_events=true;
        let sensor=b3_create_hull_shape(anchor,&sd,&crate::api::b3_make_box_hull(0.01,1.0,1.0));
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        bd.position=[-1.0,0.0,0.0];bd.linear_velocity=[120.0,0.0,0.0];bd.is_bullet=true;
        let body=b3_create_body(world,&bd);sd.is_sensor=false;
        let visitor=b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.05,0.05,0.05));
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
        assert!(b3_body_get_position(body)[0]>0.5,"a sensor must not stop the crossing body");
        {
            let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
            let hit=capture(w,world.index1).unwrap();
            assert_eq!(hit["continuous_sensor_hits"].as_array().unwrap().len(),1);
            assert_eq!(hit["continuous_sensor_hits"][0][0]["identity"],json!(sensor.index1));
            assert_eq!(hit["continuous_sensor_hits"][0][1]["identity"],json!(visitor.index1));
            assert_eq!(hit["sensor_begins"],hit["continuous_sensor_hits"]);
            assert!(hit["sensor_ends"].as_array().unwrap().is_empty());
        }
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
        {
            let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
            let after=capture(w,world.index1).unwrap();
            assert!(after["continuous_sensor_hits"].as_array().unwrap().is_empty());
            assert!(after["sensor_begins"].as_array().unwrap().is_empty());
            assert_eq!(after["sensor_ends"].as_array().unwrap().len(),1);
        }
        b3_destroy_world(world);
    }
    #[test]
    fn trace_tracks_sensor_deletion_and_motion_joint_events() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let anchor=b3_create_body(world,&crate::api::b3_default_body_def());
        let mut sd=crate::api::b3_default_shape_def();sd.is_sensor=true;sd.enable_sensor_events=true;
        let sensor=b3_create_hull_shape(anchor,&sd,&crate::api::b3_make_box_hull(2.0,2.0,2.0));
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[1.0,0.0,0.0];
        let body=b3_create_body(world,&bd);sd.is_sensor=false;
        let visitor=b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.25,0.25,0.25));
        let mut jd=crate::api::b3_default_distance_joint_def();jd.body_a=anchor;jd.body_b=body;
        jd.collide_connected=true;jd.force_threshold=0.0;
        let joint=b3_create_distance_joint(world,&jd);
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
        {
            let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
            let before=capture(w,world.index1).unwrap();
            assert_eq!(before["sensor_begins"].as_array().unwrap().len(),1);
            assert_eq!(before["sensor_begins"][0][0]["identity"],json!(sensor.index1));
            assert_eq!(before["sensor_begins"][0][1]["identity"],json!(visitor.index1));
            assert_eq!(before["sensor_overlaps"][0]["visitors"].as_array().unwrap().len(),1);
            assert_eq!(before["body_moves"].as_array().unwrap().len(),1);
            assert_eq!(before["joint_events"][0]["identity"],json!(joint.index1));
            let saved=w.body_move_events[0].transform.p[0];w.body_move_events[0].transform.p[0]=f32::NAN;
            assert!(capture(w,world.index1).is_err(),"nonfinite event transform must fail capture");
            w.body_move_events[0].transform.p[0]=saved;
        }
        b3_destroy_shape(sensor,false);
        {
            let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
            let pending=capture(w,world.index1).unwrap();
            assert_eq!(pending["deferred_sensor_ends"].as_array().unwrap().len(),1);
            assert_eq!(pending["deferred_sensor_ends"][0][0]["live"],json!(false));
        }
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
        {
            let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
            let after=capture(w,world.index1).unwrap();
            assert_eq!(after["sensor_ends"].as_array().unwrap().len(),1);
            assert!(after["deferred_sensor_ends"].as_array().unwrap().is_empty());
            assert!(after["sensor_overlaps"].as_array().unwrap().is_empty());
        }
        b3_destroy_world(world);
    }
}
