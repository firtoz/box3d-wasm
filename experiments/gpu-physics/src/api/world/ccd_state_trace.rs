//! Capture the actual separate CCD buffers, excluding unused vector lanes.
use super::*;
pub(super) fn capture(scene:Option<crate::ccd::DiagnosticCcdScene>) -> Result<Value,String> {
    let Some(s)=scene else {return Ok(Value::Null);};
    if s.config.len()!=5 || s.config[0] as usize!=s.bodies.len() || s.start.len()!=s.bodies.len() {
        return Err("CCD body/config count mismatch".into());
    }
    let indices=|first:u32,count:u32,limit:usize| -> Result<(),String> {
        let end=first.checked_add(count).ok_or("CCD index span overflow")? as usize;
        let slice=s.indices.get(first as usize..end).ok_or("CCD index span out of range")?;
        if slice.iter().any(|&i|i as usize>=limit) {return Err("CCD index reference out of range".into());}Ok(())
    };
    indices(s.config[1],s.config[2],s.shapes.len())?;
    indices(s.config[3],s.config[4].checked_mul(2).ok_or("CCD joint count overflow")?,s.bodies.len())?;
    let points=s.points.iter().map(|p|[p[0],p[1],p[2]].trace()).collect::<Result<Vec<_>,String>>()?;
    let bodies=s.bodies.iter().map(|b| {
        indices(b.first,b.count,s.shapes.len())?;
        Ok(json!({"first":b.first,"count":b.count,"min_extent":b.min_extent.trace()?,"max_extent":b.max_extent.trace()?,
            "center":([b.center[0],b.center[1],b.center[2]].trace()?)}))
    }).collect::<Result<Vec<_>,String>>()?;
    let shapes=s.shapes.iter().map(|shape| {
        if shape.body as usize>=s.bodies.len() || shape.count==0 || shape.first.checked_add(shape.count).is_none_or(|end|end as usize>s.points.len()) {
            return Err("CCD shape reference out of range".into());
        }
        Ok(json!(fields!(shape;first,count,radius,body,group,category,mask,sweep_radius)))
    }).collect::<Result<Vec<_>,String>>()?;
    let start=s.start.iter().map(|b|Ok(json!(fields!(b;pos,rot,origin,origin_valid)))).collect::<Result<Vec<_>,String>>()?;
    Ok(json!({"config":s.config,"points":points,"bodies":bodies,"shapes":shapes,"indices":s.indices,"start":start}))
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn ccd_capture_rejects_invalid_geometry_relationships() {
        use crate::ccd::{DiagnosticCcdScene,ConvexBody,ConvexShape};
        let scene=|| DiagnosticCcdScene {
            points:vec![[0.0;4]],
            shapes:vec![ConvexShape{first:0,count:1,radius:0.5,unused:0,body:0,group:0,category:[1,0],mask:[u32::MAX;2],sweep_radius:0.5,unused2:0}],
            bodies:vec![ConvexBody{first:0,count:1,min_extent:0.5,max_extent:0.5,center:[0.0;4]}],
            indices:vec![0,0],config:vec![1,1,1,2,0],start:vec![crate::types::BodyStateGpu::zeroed()],
        };
        assert!(capture(Some(scene())).is_ok());
        let mut bad=scene();bad.config[0]=2;assert!(capture(Some(bad)).is_err());
        let mut bad=scene();bad.shapes[0].first=u32::MAX;assert!(capture(Some(bad)).is_err());
        let mut bad=scene();bad.shapes[0].body=1;assert!(capture(Some(bad)).is_err());
        let mut bad=scene();bad.indices[0]=1;assert!(capture(Some(bad)).is_err());
        let mut bad=scene();bad.config[4]=u32::MAX;assert!(capture(Some(bad)).is_err());
    }
    #[test]
    fn trace_observes_device_only_convex_ccd_geometry() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();bd.position=[0.0,-0.5,0.0];
        b3_create_hull_shape(b3_create_body(world,&bd),&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(2.0,0.5,2.0));
        bd.body_type=BodyType::Dynamic;bd.position=[0.0,2.0,0.0];
        b3_create_hull_shape(b3_create_body(world,&bd),&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        with_world_mut_no_sync(world,|w|w.gpu_ccd_requested=true);
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
        {
            let mut worlds=lock_worlds();let sim=slot_mut(&mut worlds,world).unwrap().sim.as_mut().unwrap();
            let before=capture(sim.read_diagnostic_ccd_scene()).unwrap();
            assert_eq!(before["bodies"].as_array().unwrap().len(),2);
            assert_eq!(before["shapes"].as_array().unwrap().len(),2);
            assert!(!before["points"].as_array().unwrap().is_empty());
            sim.overwrite_diagnostic_ccd_point_test();
            let after=capture(sim.read_diagnostic_ccd_scene()).unwrap();
            assert_ne!(before["points"],after["points"],"device-only CCD geometry edit must be captured");
            assert_eq!(after["points"][0],json!(["3e000000","00000000","00000000"]),"unused NaN lane must be ignored");
        }
        b3_destroy_world(world);
    }
}
