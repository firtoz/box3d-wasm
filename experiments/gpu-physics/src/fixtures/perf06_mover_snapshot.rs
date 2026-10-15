
    #[test]
    fn perf06_mover_snapshot_preserves_public_groups_and_casts() {
        let mut shapes: Vec<_> = (0..1536).map(|i| {
            let mut s = test_shape(KIND_SPHERE, vec![[0.0; 3]], 0.5);
            s.id = ShapeId {index1:i+1, world0:7, generation:3};
            s.public_id = ShapeId {index1:9001+(i/512), world0:7, generation:8};
            s.child_index=i%512;
            s.body_origin=[3.0*(i%512) as f32, 2.0*(i/512) as f32, 0.0]; s
        }).collect();
        let mut index=QueryIndex::rebuild(shapes.clone());
        // Include misses between children, touching/encroaching sweeps, zero
        // translation, positive hits, reverse casts, and full-compound sweeps.
        for (p,t) in [([-2.,0.,0.],[7.,0.,0.]),([2.,0.,0.],[0.,0.01,0.]),
            ([0.,3.,0.],[0.,-4.,0.]),([4.,0.,0.],[-5.,0.,0.]),
            ([0.,0.,0.],[0.;3]),([-2.,0.,0.],[1600.,0.,0.])] {
            let moving=proxy(&[p],0.3);let mut end=moving.clone();end.points[0]+=v(t);
            let coarse=union_aabb(aabb_for_proxy(&moving),aabb_for_proxy(&end));
            let owned=index.snapshot_mover(coarse,union_aabb(query_candidate_bounds(&moving),query_candidate_bounds(&end)));
            let expected:Vec<_>=index.groups.iter().filter(|g|aabb_overlaps(coarse,group_bounds(&index.shapes,g)))
                .map(|g|index.shapes[g[0]].public_id).collect();
            assert_eq!(owned.iter().map(|g|g.shape.public_id).collect::<Vec<_>>(),expected);
            for g in &owned {assert!(g.children.windows(2).all(|s|s[0].child_index<s[1].child_index));}
            let solve=|ss:Vec<&HostShape>| {let mut f=1.0;
                for s in ss {let out=shape_cast(&shape_proxy(s,None),&moving,v(t),f,true);
                    if out.hit && out.fraction>0.0 && out.fraction<f {f=out.fraction;}}
                f.to_bits()};
            assert_eq!(solve(owned.iter().flat_map(|g|g.children.iter()).collect()),
                solve(index.groups.iter().filter(|g|aabb_overlaps(coarse,group_bounds(&index.shapes,g)))
                    .flat_map(|g|g.iter().map(|&i|&index.shapes[i])).collect()));
        }
        let hollow=Aabb{lower_bound:[1.1,-0.1,-0.1],upper_bound:[1.4,0.1,0.1]};
        let owned=index.snapshot_mover(hollow,hollow);
        assert_eq!(owned.len(),1);assert!(owned[0].children.is_empty());
        // Refits must update the cached public broad-phase bounds, while prior
        // owned callback metadata/geometry remain valid after replacement.
        for s in &mut index.shapes {s.body_origin[0]+=10000.0;}index.refit();
        assert!(index.snapshot_mover(hollow,hollow).is_empty());
        drop(index);assert_eq!(owned[0].shape.public_id.world0,7);
        assert_eq!(owned[0].shape.body_origin,[0.;3]);
        shapes.clear();
        eprintln!("PERF06 snapshot:1536children/3publicgroups; hit/miss/boundary/filtergroup/order/refit/owned lifetime PASS");
    }
