// Compare actual saved C-boundary casts and old BVH candidates against the
// production narrowphase. The candidate changes selection, not cast arithmetic.
fn legacy_candidates_cast(
    shape: &HostShape,
    transform: WorldTransform,
    moving: &Proxy,
    translation: V3,
    max_fraction: f32,
    can_encroach: bool,
) -> CastOutput {
    let start = aabb_for_proxy(moving);
    let end = Aabb {
        lower_bound: (v(start.lower_bound) + max_fraction * translation).to_array(),
        upper_bound: (v(start.upper_bound) + max_fraction * translation).to_array(),
    };
    let indices = mesh_candidates(shape, transform, union_aabb(start, end));
    mesh_shape_cast_indices(
        shape,
        transform,
        moving,
        translation,
        max_fraction,
        can_encroach,
        indices,
    )
}
#[test]
fn perf02_actual_frozen_casts_and_transformed_controls() {
    let root=std::path::Path::new("benchmarks/production-readiness/pr09-repairs-2026-10-02/PERF02/cast-cost-trace-no-json-dependency");
    let j: serde_json::Value =
        serde_json::from_str(&std::fs::read_to_string(root.join("actual-mesh.json")).unwrap())
            .unwrap();
    let arrays3 = |x: &serde_json::Value| {
        x.as_array()
            .unwrap()
            .iter()
            .map(|a| std::array::from_fn(|i| a[i].as_f64().unwrap() as f32))
            .collect::<Vec<[f32; 3]>>()
    };
    let mut mesh = test_shape(KIND_MESH, arrays3(&j["points"]), 0.0);
    mesh.mesh_triangles = arc_vec(
        j["triangles"]
            .as_array()
            .unwrap()
            .iter()
            .map(|a| std::array::from_fn(|i| a[i].as_u64().unwrap() as u32))
            .collect(),
    );
    mesh.mesh_triangle_ids = arc_vec(
        j["triangle_ids"]
            .as_array()
            .unwrap()
            .iter()
            .map(|x| x.as_i64().unwrap() as i32)
            .collect(),
    );
    mesh.mesh_nodes = arc_vec(
        j["nodes"]
            .as_array()
            .unwrap()
            .iter()
            .map(|n| super::super::world::MeshNode {
                lower: std::array::from_fn(|i| n[0][i].as_f64().unwrap() as f32),
                upper: std::array::from_fn(|i| n[1][i].as_f64().unwrap() as f32),
                data: n[2].as_u64().unwrap() as u32,
                triangle_offset: n[3].as_u64().unwrap() as u32,
            })
            .collect(),
    );
    let xf = WorldTransform {
        p: std::array::from_fn(|i| j["body_origin"][i].as_f64().unwrap() as f32),
        q: std::array::from_fn(|i| j["body_rotation"][i].as_f64().unwrap() as f32),
    };
    let inputs: serde_json::Value = serde_json::from_str(
        &std::fs::read_to_string(root.join("actual-inputs-outputs.json")).unwrap(),
    )
    .unwrap();
    let mut old_count = 0usize;
    let mut new_count = 0usize;
    let mut hits = 0usize;
    for row in inputs.as_array().unwrap() {
        let p = V3::from_array(std::array::from_fn(|i| {
            row["origin"][i].as_f64().unwrap() as f32
        }));
        let query = Proxy {
            points: vec![p],
            radius: row["radius"].as_f64().unwrap() as f32,
        };
        let delta = V3::from_array(std::array::from_fn(|i| {
            row["translation"][i].as_f64().unwrap() as f32
        }));
        let before = legacy_candidates_cast(&mesh, xf, &query, delta, 1.0, false);
        let after = mesh_shape_cast(&mesh, xf, &query, delta, 1.0, false);
        assert_eq!(
            (
                before.hit,
                before.fraction,
                before.point,
                before.normal,
                before.triangle_index,
                before.material_index
            ),
            (
                after.hit,
                after.fraction,
                after.point,
                after.normal,
                after.triangle_index,
                after.material_index
            ),
            "cast {}",
            row["index"]
        );
        assert_eq!(before.hit, !row["expected"].is_null());
        if before.hit {
            hits += 1;
            assert_eq!(
                before.fraction,
                row["expected"]["fraction"].as_f64().unwrap() as f32
            );
            assert_eq!(
                before.point,
                std::array::from_fn(|i| row["expected"]["p"][i].as_f64().unwrap() as f32)
            );
            assert_eq!(
                before.normal,
                std::array::from_fn(|i| row["expected"]["n"][i].as_f64().unwrap() as f32)
            );
            assert_eq!(
                before.triangle_index,
                row["expected"]["triangle"].as_i64().unwrap() as i32
            );
        }
        let start = aabb_for_proxy(&query);
        let end = Aabb {
            lower_bound: (v(start.lower_bound) + delta).to_array(),
            upper_bound: (v(start.upper_bound) + delta).to_array(),
        };
        old_count += mesh_candidates(&mesh, xf, union_aabb(start, end)).len();
        new_count += mesh_cast_sweep::mesh_cast_candidates(&mesh, xf, &query, delta, 1.0).len();
    }
    // Actual affine instance/body transforms, reflected/nonuniform scale, offset
    // and multi-point proxies, zero components, clipped fractions and tiny radii.
    let mut controls = 0usize;
    for angle in [0.0, 0.6, 3.1415925] {
        for scale in [[1.0; 3], [-1.0, 0.5, 2.0], [2.0, 1.5, 0.5]] {
            for radius in [0.0, 0.1, 1.0] {
                for clipped in [0.0, 0.1, 1.0] {
                    mesh.mesh_instance = Some(MeshInstance {
                        position: [-25.0, 2.0, -25.0],
                        rotation: Quat::from_rotation_y(angle).to_array(),
                        scale,
                    });
                    let transform = WorldTransform {
                        p: [1000.0, -10.0, 2000.0],
                        q: Quat::from_rotation_z(angle * 0.5).to_array(),
                    };
                    let p = transform_point(
                        transform.p,
                        transform.q,
                        mesh.mesh_instance.unwrap().point([10.0, 20.0, 10.0]),
                    );
                    let moving = Proxy {
                        points: vec![p, p + V3::new(0.5, 1.0, -0.3)],
                        radius,
                    };
                    let delta = Quat::from_array(transform.q) * V3::new(0.0, -100.0, 0.0);
                    let a =
                        legacy_candidates_cast(&mesh, transform, &moving, delta, clipped, false);
                    let b = mesh_shape_cast(&mesh, transform, &moving, delta, clipped, false);
                    assert_eq!(
                        (a.hit, a.fraction, a.point, a.normal, a.triangle_index),
                        (b.hit, b.fraction, b.point, b.normal, b.triangle_index)
                    );
                    controls += 1;
                }
            }
        }
    }
    println!("PERF02_PROOF actual_inputs={} actual_hits={} old_candidates={} new_candidates={} transformed_controls={}",inputs.as_array().unwrap().len(),hits,old_count,new_count,controls);
}
