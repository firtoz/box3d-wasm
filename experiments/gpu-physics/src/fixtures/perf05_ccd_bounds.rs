
    #[test]
    fn perf05_static_convex_bound_preserves_toi() {
        let cases: serde_json::Value=serde_json::from_str(include_str!("../fixtures/perf05_ccd_bounds.json")).unwrap();
        fn a3(x:&serde_json::Value)->[f32;3] { std::array::from_fn(|i|f32::from_bits(x[i].as_u64().unwrap() as u32)) }
        fn a4(x:&serde_json::Value)->[f32;4] { std::array::from_fn(|i|f32::from_bits(x[i].as_u64().unwrap() as u32)) }
        fn xf(x:&serde_json::Value)->WorldTransform { WorldTransform {p:a3(&x[0]),q:a4(&x[1])} }
        fn shape(x:&serde_json::Value,suffix:&str,index:usize)->HostShape {
            let get=|key:&str|&x[format!("{}_{}",key,suffix)];
            let mut out=test_shape(get("kind").as_u64().unwrap() as u32,
                get("points").as_array().unwrap().iter().map(a3).collect(),
                f32::from_bits(x["radius"][index].as_u64().unwrap() as u32));
            out.hull_planes=arc_vec(get("planes").as_array().unwrap().iter().map(a4).collect());
            out.local_center=a3(get("center"));out.half_extents=a3(get("half"));out
        }
        fn old_bound(s:&HostShape,a:WorldTransform,b:WorldTransform)->Aabb {
            // Execute the actual old broad-bound operation, not a copy.
            let (lower_bound,upper_bound)=super::super::world::sweep_bounds(s,a,b);
            Aabb {lower_bound,upper_bound}
        }
        fn overlap(a:Aabb,b:Aabb)->bool {(0..3).all(|i|a.lower_bound[i]<=b.upper_bound[i]&&b.lower_bound[i]<=a.upper_bound[i])}
        fn enclose(s:&HostShape,a:WorldTransform,b:WorldTransform,bounds:Aabb)->usize {
            let mut count=0;
            for i in 0..=256 {
                // Execute the actual generic convex TOI transform/proxy path,
                // which normalizes endpoints before the quaternion blend.
                let proxy=shape_proxy(s,Some(interpolate_transform(a,b,i as f32/256.0)));
                for point in proxy.points {for axis in 0..3 {
                    assert!(point[axis]-s.radius>=bounds.lower_bound[axis]&&point[axis]+s.radius<=bounds.upper_bound[axis],"point enclosure failed at{axis} t{i}");
                }count+=1;}
            }count
        }
        let mut rejected=0;let mut points=0;let mut replayed=0;
        for x in cases.as_array().unwrap() {
            let a=shape(x,"a",0);let b=shape(x,"b",1);
            let a0=xf(&x["start_a"]);let a1=xf(&x["end_a"]);let b0=xf(&x["start_b"]);let b1=xf(&x["end_b"]);
            let actual=rigid_time_of_impact(&a,a0,a1,&b,b0,b1,f32::from_bits(x["max_fraction"].as_u64().unwrap() as u32));
            assert!(actual.is_none()&&x["hits"].as_array().unwrap().is_empty(),"actual captured miss changed");replayed+=1;
            let bound=convex_sweep_bounds(&a,a0,a1,[0.0;3]).unwrap();
            points+=enclose(&a,a0,a1,bound);
            assert!(overlap(old_bound(&a,a0,a1),old_bound(&b,b0,b1)));
            if !overlap(bound,old_bound(&b,b0,b1)) {rejected+=1;}
            eprintln!("PERF05BOUND,target={},keep={},lower={:?},upper={:?}",x["target"],overlap(bound,old_bound(&b,b0,b1)),bound.lower_bound.map(f32::to_bits),bound.upper_bound.map(f32::to_bits));
        }
        assert!(rejected>0&&replayed==41);
        // Actual generic TOI path enclosure at off-axis hull points, far world
        // coordinates, antipodal endpoints, small/large angles and norm defects.
        let hull=shape(&cases[1],"a",0);let mut controls=0;
        for axis in [V3::X,V3::Y,V3::Z] {for angle in [0.0,0.000001,0.01,1.2,std::f32::consts::PI] {for sign in [-1.0,1.0] {for norm in [0.999999,1.0,1.000001] {
            let a=WorldTransform {p:[10000.0,-20000.0,30000.0],q:[0.0,0.0,0.0,norm]};
            let b=WorldTransform {p:[10001.0,-19996.0,29998.0],q:(Quat::from_axis_angle(axis,angle)*sign).to_array().map(|x|x*norm)};
            points+=enclose(&hull,a,b,convex_sweep_bounds(&hull,a,b,[0.0;3]).unwrap());controls+=1;
        }}}}
        let floor=shape(&cases[0],"a",0);let cube=shape(&cases[0],"b",1);let ground=xf(&cases[0]["start_a"]);
        let tight=convex_sweep_bounds(&floor,ground,ground,[0.0;3]).unwrap();let mut positive=0;
        for (x,z) in [(0.0,0.0),(59.7,0.0),(-59.7,0.0),(0.0,59.7),(0.0,-59.7)] {
            let a=WorldTransform {p:[x,2.0,z],q:[0.0,0.0,0.0,1.0]};let b=WorldTransform {p:[x,-2.0,z],..a};
            let hit=rigid_time_of_impact(&floor,ground,ground,&cube,a,b,1.0).unwrap();
            assert!(hit.fraction>0.4&&hit.fraction<0.6&&overlap(tight,old_bound(&cube,a,b)));positive+=1;
        }
        let invalid=WorldTransform {q:[0.0;4],..ground};assert!(convex_sweep_bounds(&floor,invalid,ground,[0.0;3]).is_none());
        let empty=test_shape(crate::types::KIND_BOX,vec![],0.0);assert!(convex_sweep_bounds(&empty,ground,ground,[0.0;3]).is_none());
        eprintln!("PERF05PROBE,replayed={replayed},rejected={rejected},controls={controls},enclosed_points={points},positive_hits={positive},invalid_fallback=2");
    }
