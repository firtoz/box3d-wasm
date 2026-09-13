//! Seed contact patches directly to test solver ownership independently of narrowphase.
use super::*;
use crate::types::{
    ContactHotGpu, FLAG_STATIC, OVERFLOW_COLOR, PAIR_CAP, RADIX_BUCKETS, SCR_COLOR, SCR_PAIRS,
    SCR_RADIX_BASE,
};

#[derive(Clone, Copy, Debug)]
enum Path {
    Color,
    Wave,
    Overflow,
    Jacobi,
    Legacy,
}
const PATHS: [Path; 5] = [
    Path::Color,
    Path::Wave,
    Path::Overflow,
    Path::Jacobi,
    Path::Legacy,
];

fn make_sim(gpu: &GpuDevice, endpoint: u32) -> GpuSim {
    make_sim_sized(gpu,endpoint,2)
}

fn make_sim_sized(gpu: &GpuDevice, endpoint: u32, count:u32) -> GpuSim {
    let mut ground = BodyGpu::zeroed();
    ground.flags = FLAG_STATIC;
    ground.rot = [0.0, 0.0, 0.0, 1.0];
    ground.dq = ground.rot;
    ground.half = [0.5; 3];
    let mut body = ground;
    body.flags = 0;
    body.inv_mass = 1.0;
    body.inv_inertia = [1.0; 3];
    body.vel = [-2.0, -2.0, 0.0];
    match endpoint {
        1 => {
            ground.flags = 0;
            ground.inv_mass = 1.0;
            ground.inv_inertia = [1.0; 3];
        }
        2 => {
            ground.flags = crate::types::FLAG_KINEMATIC;
            ground.vel = [-0.5, 0.5, 0.0];
        }
        _ => {}
    }
    let caps = GpuSceneCaps::allocate(GpuSceneCaps::live(count, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 1, 1);
    let mut bodies=vec![ground;count as usize];bodies[1]=body;
    GpuSim::new(
        gpu,
        &bodies,
        &vec![[0.0; 3];count as usize],
        &vec![[0.0; 3];count as usize],
        &[],
        &[],
        &[],
        &[],
        &[],
        &[],
        &[],
        &[],
        &[],
        &[],
        &[],
        caps,
        true,
        1.0 / 60.0,
        [0.0; 3],
    )
    .unwrap()
}

fn patches() -> [ContactHotGpu; 2] {
    let mut root = ContactHotGpu::zeroed();
    root.a = 0;
    root.b = 1;
    root.count = 1;
    root.n = [0.0, 1.0, 0.0];
    root.restitution = 0.5;
    root.manifold_link = [2, 0, 2, 0];
    let mut child = root;
    child.n = [1.0, 0.0, 0.0];
    child.manifold_link = [0, 1, 0, 0];
    [root, child]
}

fn run(
    gpu: &GpuDevice,
    patches: Vec<ContactHotGpu>,
    path: Path,
    modes: &[u32],
) -> (Vec<BodyGpu>, LiveStepStats) {
    run_with_endpoint(gpu, patches, path, modes, 0)
}

fn run_with_endpoint(
    gpu: &GpuDevice,
    mut patches: Vec<ContactHotGpu>,
    path: Path,
    modes: &[u32],
    endpoint: u32,
) -> (Vec<BodyGpu>, LiveStepStats) {
    let mut sim = make_sim(gpu, endpoint);
    let color = if matches!(path, Path::Overflow) {
        OVERFLOW_COLOR
    } else {
        0
    };
    for patch in &mut patches {
        patch.color = color;
    }
    sim.queue
        .write_buffer(&sim.contacts, 0, bytemuck::cast_slice(&patches));
    // One active root, irrespective of how many patches it owns. This mirrors
    // the graph contract; child slots must never be independent body writers.
    let write = |word: u32, value: u32| {
        sim.queue.write_buffer(
            &sim.scratch,
            u64::from(word) * 4,
            bytemuck::bytes_of(&value),
        );
    };
    write(3, 1); // SCR_NCONTACTS
    write(SCR_PAIRS + 2 * PAIR_CAP, 0); // SCR_ACTIVE_CONTACT
    write(SCR_COLOR + color, 1);
    // Exact WGSL color_contact_base() layout (uses live body slots, not caps).
    let color_base = SCR_RADIX_BASE + RADIX_BUCKETS + 112 * sim.params.body_count + 192;
    write(color_base + color * sim.params.contact_capacity, 0);
    let mut enc = sim
        .device
        .create_command_encoder(&wgpu::CommandEncoderDescriptor {
            label: Some("manifold-chain-regression"),
        });
    sim.dispatch_n(&mut enc, &sim.prepare_contacts, 1, color, 0);
    for &mode in modes {
        let pipeline = match path {
            Path::Color => &sim.solve_color,
            Path::Wave => &sim.solve_color_wave_one_group,
            Path::Overflow => &sim.solve_overflow,
            Path::Jacobi => &sim.solve_jacobi,
            Path::Legacy if mode == 2 => &sim.warm_start,
            Path::Legacy => &sim.solve_contacts,
        };
        if matches!(path, Path::Jacobi) {
            sim.dispatch_n(&mut enc, &sim.jacobi_clear, 1, color, mode);
        }
        sim.dispatch_n(&mut enc, pipeline, 1, color, mode);
        if matches!(path, Path::Jacobi) {
            sim.dispatch_n(&mut enc, &sim.apply_jacobi, 1, color, mode);
        }
    }
    sim.queue.submit(Some(enc.finish()));
    let bodies = pollster::block_on(sim.read_bodies());
    let stats = pollster::block_on(sim.read_live_step_stats());
    (bodies, stats)
}

fn assert_velocity(bodies: &[BodyGpu], expect: [f32; 3], label: &str) {
    for axis in 0..3 {
        assert!(
            (bodies[1].vel[axis] - expect[axis]).abs() < 0.002,
            "{label}: {:?} expected {expect:?}",
            bodies[1].vel
        );
        assert_eq!(bodies[0].vel[axis], 0.0, "static endpoint changed: {label}");
    }
}

#[test]
fn manifold_chain_executes_every_patch_in_all_solver_paths() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    for path in PATHS {
        let (bodies, stats) = run(&gpu, patches().to_vec(), path, &[0]);
        assert_velocity(&bodies, [0.0; 3], &format!("{path:?}"));
        assert_eq!(stats.drops.contact_pairs_dropped, 0);
        let mut root = patches()[0];
        root.manifold_link = [0; 4];
        let (single, _) = run(&gpu, vec![root], path, &[0]);
        assert_velocity(&single, [-2.0, 0.0, 0.0], "singleton control");
    }
}

#[test]
fn manifold_chain_warm_start_and_restitution_visit_children_once() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    for path in PATHS {
        let mut warm = patches();
        for patch in &mut warm {
            patch.rb[0][3] = 2.0;
        }
        let (bodies, _) = run(&gpu, warm.to_vec(), path, &[2]);
        assert_velocity(&bodies, [0.0; 3], &format!("warm {path:?}"));
        let (bodies, _) = run(&gpu, patches().to_vec(), path, &[0, 3]);
        assert_velocity(&bodies, [1.0, 1.0, 0.0], &format!("restitution {path:?}"));
    }
}

#[test]
fn manifold_chain_rejects_corruption_before_applying_impulses() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    for corruption in 0..7 {
        let mut bad = patches();
        match corruption {
            0 => bad[1].manifold_link[1] = 7,        // wrong owner
            1 => bad[0].manifold_link[0] = u32::MAX, // invalid next slot
            2 => bad[1].manifold_link[0] = 1,        // cycle to root
            3 => bad[1].b = 0,                       // different body pair
            4 => bad[1].count = 5,                   // point array overflow
            5 => bad[1].count = 0,                   // empty child in active ownership chain
            6 => bad[0].manifold_link[2] = u32::MAX, // unbounded traversal
            _ => unreachable!(),
        }
        for path in PATHS {
            let (bodies, stats) = run(&gpu, bad.to_vec(), path, &[2, 0, 3]);
            assert_velocity(
                &bodies,
                [-2.0, -2.0, 0.0],
                &format!("bad {corruption} {path:?}"),
            );
            assert!(stats.drops.contact_pairs_dropped > 0);
            assert!(stats.sticky.contact_pairs_dropped > 0);
        }
    }
}

#[test]
fn manifold_chain_conserves_dynamic_momentum_and_respects_kinematics() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    for path in PATHS {
        for endpoint in [1, 2] {
            let (bodies, stats) =
                run_with_endpoint(&gpu, patches().to_vec(), path, &[1, 0], endpoint);
            assert_eq!(stats.drops.contact_pairs_dropped, 0);
            let expected = if endpoint == 1 {
                [-1.0, -1.0, 0.0]
            } else {
                [-0.5, 0.5, 0.0]
            };
            for body in &bodies[..2] {
                for axis in 0..3 {
                    assert!(
                        (body.vel[axis] - expected[axis]).abs() < 0.003,
                        "endpoint={endpoint} {path:?}: {:?}, expected {expected:?}",
                        body.vel
                    );
                }
            }
        }
    }
}

fn test_pipeline(sim: &GpuSim, entry: &str, suffix: &str) -> ComputePipeline {
    let module = sim
        .device
        .create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("manifold-pool-test"),
            source: wgpu::ShaderSource::Wgsl(format!("{PHYSICS_WGSL}\n{suffix}").into()),
        });
    let bgl = sim.solve_color.get_bind_group_layout(0);
    let layout = sim
        .device
        .create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: Some("manifold-pool-test"),
            bind_group_layouts: &[&bgl],
            push_constant_ranges: &[],
        });
    make_compute_cached(&sim.device, &layout, &module, entry, None)
}

fn prepare_pool(sim: &mut GpuSim, missing: u32, mesh: bool) {
    sim.params.mesh_triangle_count = u32::from(mesh);
    sim.flush_params();
    sim.queue
        .write_buffer(&sim.scratch, 54 * 4, bytemuck::bytes_of(&missing));
    let mut enc = sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc, &sim.write_alloc_indirects, 1, 0, 0);
    sim.copy_radix_indirect(&mut enc);
    {
        let mut pass = enc.begin_compute_pass(&Default::default());
        pass.set_bind_group(0, sim.ping_bg(), &[GpuSim::pass_lut_offset(0, 0) as u32]);
        pass.set_pipeline(&sim.alloc_free_histogram);
        pass.dispatch_workgroups_indirect(&sim.indirect, GpuSim::indirect_radix_offset());
        pass.set_pipeline(&sim.alloc_free_bases);
        pass.dispatch_workgroups(1, 1, 1);
        pass.set_pipeline(&sim.alloc_free_scatter);
        pass.dispatch_workgroups_indirect(&sim.indirect, GpuSim::indirect_radix_offset());
    }
    sim.queue.submit(Some(enc.finish()));
}

#[test]
fn manifold_pool_allocates_uniquely_after_primary_reservations_and_reports_exhaustion() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    sim.queue
        .write_buffer(&sim.contacts, 0, bytemuck::cast_slice(&patches()));
    prepare_pool(&mut sim, 3, true);
    let pipeline = test_pipeline(
        &sim,
        "test_allocate_patches",
        r#"
        @compute @workgroup_size(64)
        fn test_allocate_patches(@builtin(global_invocation_id) gid: vec3<u32>) {
            if (gid.x >= params.contact_capacity) { return; }
            let slot = allocate_manifold_slot(0u);
            scratch[SCR_ACTIVE_CONTACT + gid.x] = slot;
            if (slot != EMPTY) {
                var c = empty_contact();
                c.a = 0u; c.b = 1u; c.count = 1u;
                c.n = vec3<f32>(0.0, 1.0, 0.0);
                c.manifold_link.y = 1u;
                store_contact(slot, c);
            }
        }
    "#,
    );
    let mut enc = sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(
        &mut enc,
        &pipeline,
        sim.params.contact_capacity.div_ceil(64),
        0,
        0,
    );
    sim.queue.submit(Some(enc.finish()));
    let contacts = pollster::block_on(sim.read_contacts());
    let words = pollster::block_on(
        sim.read_scratch_prefix(SCR_PAIRS + 2 * PAIR_CAP + sim.params.contact_capacity),
    );
    let out = &words[(SCR_PAIRS + 2 * PAIR_CAP) as usize..];
    let unique: std::collections::BTreeSet<_> = out
        .iter()
        .copied()
        .filter(|&slot| slot != u32::MAX)
        .collect();
    let expected: std::collections::BTreeSet<_> = (5..sim.params.contact_capacity).collect();
    assert_eq!(unique, expected);
    assert_eq!(
        out.iter().filter(|&&slot| slot != u32::MAX).count(),
        unique.len(),
        "duplicate allocation"
    );
    assert_eq!(contacts[0].manifold_link, [2, 0, 2, 0]);
    assert_eq!(
        contacts[1].manifold_link,
        [0, 1, 0, 0],
        "old warm-start patch overwritten"
    );
    for slot in 2..5 {
        assert_eq!(
            contacts[slot].a,
            u32::MAX,
            "primary reservation overwritten"
        );
    }
    let stats = pollster::block_on(sim.read_live_step_stats());
    assert_eq!(stats.drops.contact_pairs_dropped, 5);
    assert_eq!(stats.sticky.contact_pairs_dropped, 5);
}

#[test]
fn manifold_pool_is_available_for_mesh_patch_growth_without_new_pairs() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    sim.queue
        .write_buffer(&sim.contacts, 0, bytemuck::cast_slice(&patches()));
    prepare_pool(&mut sim, 0, false);
    let words = pollster::block_on(sim.read_scratch_prefix(55));
    assert_eq!(
        words[48], 0,
        "non-mesh singleton fast path should skip free scan"
    );
    assert_eq!(words[53], 0);
    prepare_pool(&mut sim, 0, true);
    let words = pollster::block_on(sim.read_scratch_prefix(55));
    assert_eq!(words[48], sim.params.contact_capacity.div_ceil(256));
    assert_eq!(words[53], sim.params.contact_capacity - 2);
}

#[test]
fn manifold_pool_retires_deleted_body_chains_but_preserves_malformed_chains() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    for corrupt in [false, true] {
        let mut sim = make_sim(&gpu, 0);
        let mut old = patches();
        for c in &mut old {
            c.a = 900;
            c.b = 901;
        }
        if corrupt {
            old[1].manifold_link[1] = 8;
        }
        sim.queue
            .write_buffer(&sim.contacts, 0, bytemuck::cast_slice(&old));
        sim.queue
            .write_buffer(&sim.scratch, 5 * 4, bytemuck::bytes_of(&1u32));
        sim.queue.write_buffer(
            &sim.scratch,
            u64::from(SCR_PAIRS + 3 * PAIR_CAP) * 4,
            bytemuck::bytes_of(&0u32),
        );
        let mut enc = sim.device.create_command_encoder(&Default::default());
        sim.dispatch_n(&mut enc, &sim.retire_stale_contacts, 1, 0, 0);
        sim.queue.submit(Some(enc.finish()));
        let contacts = pollster::block_on(sim.read_contacts());
        let stats = pollster::block_on(sim.read_live_step_stats());
        if corrupt {
            assert_eq!(contacts[0].a, 900);
            assert_eq!(
                contacts[1].a, 900,
                "partial retirement of invalid ownership"
            );
            assert!(stats.sticky.contact_pairs_dropped > 0);
        } else {
            assert_eq!(contacts[0].a, u32::MAX);
            assert_eq!(contacts[1].a, u32::MAX);
            assert_eq!(contacts[0].manifold_link, [0; 4]);
            assert_eq!(contacts[1].manifold_link, [0; 4]);
            assert_eq!(stats.drops.contact_pairs_dropped, 0);
            prepare_pool(&mut sim, 0, true);
            let words = pollster::block_on(sim.read_scratch_prefix(55));
            assert_eq!(
                words[53], sim.params.contact_capacity,
                "retired slots unavailable on next pass"
            );
        }
    }
}

#[test]
fn manifold_finalization_uses_selected_patch_history_not_pair_root() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    let mut old = patches();
    old[0].rb[0][3] = 2.0;
    old[1].rb[0][3] = 5.0;
    old[0].friction_impulse = [1.0, 2.0];
    old[1].friction_impulse = [3.0, 4.0];
    sim.queue
        .write_buffer(&sim.contacts, 0, bytemuck::cast_slice(&old));
    let pipeline = test_pipeline(
        &sim,
        "test_finalize_history",
        r#"
        @compute @workgroup_size(64)
        fn test_finalize_history(@builtin(local_invocation_index) lid: u32) {
            if (lid == 0u) { publish_contact_slot(0u, 0u); }
            storageBarrier(); workgroupBarrier();
            if (lid >= 3u) { return; }
            var previous = empty_contact();
            if (lid < 2u) { previous = load_contact(lid); }
            var c = empty_contact();
            c.a = 0u; c.b = 1u; c.count = 1u;
            c.n = select(vec3<f32>(0.0, 1.0, 0.0), vec3<f32>(1.0, 0.0, 0.0), lid == 1u);
            finish_manifold_from_previous(&c, load_body(0u), load_body(1u), 0u, 1u, previous);
            store_contact(2u + lid, c);
        }
    "#,
    );
    let mut enc = sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc, &pipeline, 1, 0, 0);
    sim.queue.submit(Some(enc.finish()));
    let contacts = pollster::block_on(sim.read_contacts());
    for (index, impulse) in [2.0, 5.0, 0.0].into_iter().enumerate() {
        assert_eq!(
            contacts[2 + index].rb0[3],
            impulse,
            "wrong patch history for {index}"
        );
    }
    assert_eq!(contacts[2].friction_impulse, [1.0, 2.0]);
    assert_eq!(contacts[3].friction_impulse, [3.0, 4.0]);
    assert_eq!(contacts[4].friction_impulse, [0.0, 0.0]);
}

#[test]
fn mesh_point_history_requires_full_triangle_and_feature_identity() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    let pipeline = test_pipeline(
        &sim,
        "test_triangle_history",
        r#"
        @compute @workgroup_size(1)
        fn test_triangle_history() {
            var old = empty_contact();
            old.a = 0u; old.b = 1u; old.count = 4u;
            old.n = vec3<f32>(0.0, 1.0, 0.0);
            old.point_triangles = vec4<u32>(1u, 65537u, 70001u, 90001u);
            for (var i = 0u; i < 4u; i++) {
                set_point(&old, i, vec4<f32>(0.0), vec4<f32>(0.0, 0.0, 0.0, f32(i + 2u)));
                set_feat_at(&old, i, 0u); // zero is a valid mesh feature id
            }
            var c = old;
            c.point_triangles = vec4<u32>(65537u, 1u, 65537u, 42u);
            let matched = match_previous_points(c, old);
            let impulse = matched.impulses;
            c.lifecycle.y = matched.persisted;
            c.rb0.w = impulse.x; c.rb1.w = impulse.y; c.rb2.w = impulse.z; c.rb3.w = impulse.w;
            store_contact(0u, c);
            // Same triangle but a different feature must not use proximity fallback.
            c = old;
            for (var i = 0u; i < 4u; i++) { set_feat_at(&c, i, 99u); }
            let unmatched = match_previous_points(c, old);
            let mismatch = unmatched.impulses;
            c.lifecycle.y = unmatched.persisted;
            c.rb0.w = mismatch.x; c.rb1.w = mismatch.y; c.rb2.w = mismatch.z; c.rb3.w = mismatch.w;
            store_contact(1u, c);
            old.rb0.w = 0.0; old.rb1.w = 0.0; old.rb2.w = 0.0; old.rb3.w = 0.0;
            c = old;
            let zero_match = match_previous_points(c, old);
            c.lifecycle.y = zero_match.persisted;
            store_contact(2u, c);
        }
    "#,
    );
    let mut enc = sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc, &pipeline, 1, 0, 0);
    sim.queue.submit(Some(enc.finish()));
    let contacts = pollster::block_on(sim.read_contacts());
    let impulses = |c: &ContactGpu| [c.rb0[3], c.rb1[3], c.rb2[3], c.rb3[3]];
    assert_eq!(impulses(&contacts[0]), [3.0, 2.0, 0.0, 0.0]);
    assert_eq!(impulses(&contacts[1]), [0.0; 4]);
    assert_eq!(std::array::from_fn::<_, 4, _>(|i| contacts[0].point_persisted(i)), [true,true,false,false]);
    assert!((0..4).all(|i| !contacts[1].point_persisted(i)));
    assert_eq!(impulses(&contacts[2]), [0.0;4]);
    assert!((0..4).all(|i| contacts[2].point_persisted(i)), "zero impulse is still a persisted point");
    assert!(!contacts[2].point_persisted(usize::MAX));
    assert_eq!(
        contacts[0].point_triangles,
        [65537, 1, 65537, 42],
        "GPU/host triangle layout or identity truncation"
    );
}

#[test]
fn mesh_point_reduction_carries_triangle_and_feature_with_retained_witness() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    let pipeline = test_pipeline(
        &sim,
        "test_triangle_reduction",
        r#"
        @compute @workgroup_size(1)
        fn test_triangle_reduction() {
            var c = empty_contact(); c.a = 0u; c.b = 1u;
            c.n = vec3<f32>(0.0, 1.0, 0.0);
            let points = array<vec3<f32>, 5>(vec3<f32>(-1.0,0.0,-1.0), vec3<f32>(1.0,0.0,-1.0),
                vec3<f32>(1.0,0.0,1.0), vec3<f32>(-1.0,0.0,1.0), vec3<f32>(0.0));
            for (var i = 0u; i < 5u; i++) {
                let depth = select(0.0, -1.0, i == 4u);
                append_mesh_point_with_identity(&c, vec4<f32>(points[i], depth), vec4<f32>(points[i], 0.0),
                    c.n, 70000u + i, 1000u + i);
            }
            store_contact(0u, c);
        }
    "#,
    );
    let mut enc = sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc, &pipeline, 1, 0, 0);
    sim.queue.submit(Some(enc.finish()));
    let contacts = pollster::block_on(sim.read_contacts());
    let c = &contacts[0];
    assert_eq!(c.count, 4);
    assert!(
        c.point_triangles.contains(&70004),
        "deepest witness discarded"
    );
    let points = [
        [-1.0, 0.0, -1.0],
        [1.0, 0.0, -1.0],
        [1.0, 0.0, 1.0],
        [-1.0, 0.0, 1.0],
        [0.0; 3],
    ];
    let anchors = [c.ra0, c.ra1, c.ra2, c.ra3];
    let features = [
        c._tail[6],
        c._tail[7],
        c._pad_ca.to_bits(),
        c._pad_cb.to_bits(),
    ];
    for i in 0..4 {
        let source = (c.point_triangles[i] - 70000) as usize;
        assert!(source < 5);
        assert_eq!(&anchors[i][..3], &points[source]);
        assert_eq!(features[i], 1000 + source as u32);
    }
}

#[test]
fn mesh_clipping_preserves_incoming_and_outgoing_feature_edges() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    let pipeline = test_pipeline(
        &sim,
        "test_mesh_clip_features",
        r#"
        @compute @workgroup_size(1)
        fn test_mesh_clip_features() {
            var polygon: ClipPolygon; polygon.count = 4u;
            polygon.points[0] = vec3<f32>(-1.0,0.0,-1.0);
            polygon.points[1] = vec3<f32>(1.0,0.0,-1.0);
            polygon.points[2] = vec3<f32>(1.0,0.0,1.0);
            polygon.points[3] = vec3<f32>(-1.0,0.0,1.0);
            for (var i = 0u; i < 4u; i++) { polygon.features[i] = pack_feature(1u, (i+3u)%4u, 1u, i); }
            let clipped = clip_polygon_with_features(polygon, vec3<f32>(1.0,0.0,0.0), 0.0, 9u, 0u);
            var c = empty_contact(); c.a = 0u; c.b = 1u; c.count = clipped.count;
            for (var i = 0u; i < clipped.count; i++) {
                set_point(&c, i, vec4<f32>(clipped.points[i], 0.0), vec4<f32>(0.0));
                set_feat_at(&c, i, clipped.features[i]);
            }
            store_contact(0u, c);
        }
    "#,
    );
    let mut enc = sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc, &pipeline, 1, 0, 0);
    sim.queue.submit(Some(enc.finish()));
    let contacts = pollster::block_on(sim.read_contacts());
    let c = &contacts[0];
    assert_eq!(c.count, 4);
    let pack = |o1: u32, i1: u32, o2: u32, i2: u32| (o1 << 24) | (i1 << 16) | (o2 << 8) | i2;
    assert_eq!(
        [
            c._tail[6],
            c._tail[7],
            c._pad_ca.to_bits(),
            c._pad_cb.to_bits()
        ],
        [
            pack(1, 3, 1, 0),
            pack(1, 0, 0, 9),
            pack(0, 9, 1, 2),
            pack(1, 2, 1, 3)
        ]
    );
    assert_eq!(&c.ra1[..3], &[0.0, 0.0, -1.0]);
    assert_eq!(&c.ra2[..3], &[0.0, 0.0, 1.0]);
}

#[test]
fn generated_mesh_list_publishes_root_and_children() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    prepare_pool(&mut sim, 1, true);
    let pipeline = test_pipeline(&sim, "test_mesh_publication", r#"
        @compute @workgroup_size(1)
        fn test_mesh_publication() {
            var first = empty_contact(); first.a = 0u; first.b = 1u; first.count = 1u;
            first.n = vec3<f32>(0.0,1.0,0.0); first.manifold_link = vec4<u32>(3u,1u,0u,1u);
            var second = first; second.n = vec3<f32>(1.0,0.0,0.0); second.manifold_link.x = 0u;
            var list = MeshPatchList(EMPTY, EMPTY, 0u);
            var shape: Shape;
            append_mesh_patch(&list, 0u, first, shape, load_body(0u));
            append_mesh_patch(&list, 0u, empty_contact(), shape, load_body(0u));
            append_mesh_patch(&list, 0u, second, shape, load_body(0u));
            let result = finalize_mesh_patch_list(list,0u,load_body(0u),load_body(1u),0u,1u);
            store_pair(0u,65536u,result,0u,1u,load_body(0u),load_body(1u));
        }
    "#);
    let mut enc = sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc,&pipeline,1,0,0); sim.queue.submit(Some(enc.finish()));
    let contacts = pollster::block_on(sim.read_contacts());
    assert_eq!(contacts[0].count,1,"root {:?}",contacts[0]);
    assert_eq!(contacts[0].manifold_link,[3,0,2,1]);
    assert_eq!(contacts[1].a,u32::MAX);
    assert_eq!(contacts[2].manifold_link,[0,1,0,1]);
    assert_eq!(contacts[2].lifecycle[1],3);
}

#[test]
fn callback_veto_validates_complete_manifold_ownership() {
    let mut contacts = vec![ContactGpu::empty(); 3];
    contacts[0].a=1; contacts[0].b=2; contacts[0].count=1;
    contacts[0].manifold_link=[3,0,2,0];
    contacts[2].a=1; contacts[2].b=2; contacts[2].count=1;
    contacts[2].manifold_link=[0,1,0,0];
    assert_eq!(callback_disabled_members(&[(0,5)],&contacts).unwrap(),vec![(0,5),(2,1)]);
    contacts[2].manifold_link[1]=2;
    assert!(callback_disabled_members(&[(0,5)],&contacts).is_err());
    contacts[2].manifold_link[1]=1;
    contacts[2].manifold_link[0]=3;
    assert!(callback_disabled_members(&[(0,5)],&contacts).is_err());
    contacts[2].manifold_link[0]=0;
    contacts[2].count=0;
    assert!(callback_disabled_members(&[(0,5)],&contacts).is_err());
}

#[test]
fn gear_mesh_reduction_keeps_both_native_penetrating_witnesses() {
    // Accepted, unique candidates from the identical-state native Gear Lift
    // rock impact. The former deepest-first reduction discarded point 1.
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    let pipeline = test_pipeline(&sim, "test_gear_reduction", r#"
        @compute @workgroup_size(1)
        fn test_gear_reduction() {
            var c = empty_contact(); c.a = 0u; c.b = 1u;
            c.n = vec3<f32>(0.0, 1.0, 0.0);
            let points = array<vec4<f32>,5>(
                vec4<f32>(-1.9323504,2.8985815,-0.9133200,-0.05971861),
                vec4<f32>(-1.9261181,2.9189296,-0.8577611,-0.039370537),
                vec4<f32>(-1.9532518,3.0111532,-0.6526684,0.052853107),
                vec4<f32>(-2.2156858,3.0421884,-0.7973329,0.08388829),
                vec4<f32>(-1.8998071,3.0048330,-0.6232074,0.04653287));
            for (var i=0u; i<5u; i++) {
                append_mesh_point_with_identity(&c, points[i], vec4<f32>(points[i].xyz,0.0),
                    c.n, 100u+i, 200u+i);
            }
            store_contact(0u,c);
        }
    "#);
    let mut enc = sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc, &pipeline, 1, 0, 0);
    sim.queue.submit(Some(enc.finish()));
    let contacts = pollster::block_on(sim.read_contacts());
    let c = &contacts[0];
    assert_eq!(c.count, 4);
    let mut identities = c.point_triangles;
    identities.sort();
    assert_eq!(identities, [100,101,103,104], "native coverage and both penetrating witnesses");
    assert_eq!([c.ra0,c.ra1,c.ra2,c.ra3].iter().filter(|p| p[3]<0.0).count(),2);
}

#[test]
fn body_extra_reads_use_capacity_when_live_slot_span_changes() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    let capacity = sim.params.shape_base_u32 / 24;
    assert!(capacity > 2, "fixture requires spare allocated body slots");
    let extra = [0.125f32,0.25,0.5,0.0625,0.3,0.6,1.2,0.0];
    sim.queue.write_buffer(&sim.body_cold, u64::from(16*capacity+8)*4, bytemuck::cast_slice(&extra));
    let pipeline = test_pipeline(&sim,"test_body_extra",r#"
        @compute @workgroup_size(1)
        fn test_body_extra() {
            let body=load_body(1u);
            let extra=body_extra_offset(1u);
            var c=empty_contact();c.a=0u;c.b=1u;c.count=1u;
            c.ra0=vec4<f32>(body.inv_inertia_offdiag,scene_f32(extra+3u));
            c.rb0=vec4<f32>(scene_f32(extra+4u),scene_f32(extra+5u),scene_f32(extra+6u),0.0);
            store_contact(0u,c);
        }
    "#);
    for live in [2,capacity-1,capacity] {
        sim.params.body_count=live;
        sim.flush_params();
        let mut enc=sim.device.create_command_encoder(&Default::default());
        sim.dispatch_n(&mut enc,&pipeline,1,0,0);
        sim.queue.submit(Some(enc.finish()));
        let contacts=pollster::block_on(sim.read_contacts());
        assert_eq!(contacts[0].ra0,[0.125,0.25,0.5,0.0625],"live={live}");
        assert_eq!(contacts[0].rb0,[0.3,0.6,1.2,0.0],"live={live}");
    }
}


#[test]
fn convex_rolling_uses_child_inner_radius_not_body_bounds() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    // Two children of one body have identical bounds but different inner radii.
    // Native contact.c uses quarter innerRadius; mesh_contact.c uses it whole.
    for (slot, radius) in [(0u64, 0.2f32), (1, 0.6)] {
        let mut shape = crate::types::ShapeGpu::zeroed();
        shape.body_index = 1;
        shape.kind = crate::types::KIND_CONVEX_HULL;
        shape.half = [2.0, 3.0, 4.0];
        shape.inner_radius = radius;
        sim.queue.write_buffer(
            &sim.body_cold,
            u64::from(sim.params.shape_base_u32) * 4 + slot * core::mem::size_of::<crate::types::ShapeGpu>() as u64,
            bytemuck::bytes_of(&shape),
        );
    }
    let pipeline = test_pipeline(&sim, "test_child_rolling", r#"
        @compute @workgroup_size(1)
        fn test_child_rolling() {
            var c = empty_contact(); c.a = 0u; c.b = 1u; c.count = 1u;
            c.ra0 = vec4<f32>(rolling_radius(load_collider(0u)),
                rolling_radius(load_collider(1u)),
                mesh_rolling_radius(load_shape(0u)), mesh_rolling_radius(load_shape(1u)));
            var rounded = load_collider(0u);
            rounded.kind = KIND_SPHERE; rounded.half.x = 0.8;
            let sphere = rolling_radius(rounded);
            rounded.kind = KIND_CAPSULE;
            let capsule = rolling_radius(rounded);
            rounded.kind = KIND_BOX; rounded.half = vec3<f32>(2.0, 3.0, 4.0);
            c.rb0 = vec4<f32>(sphere, capsule, rolling_radius(rounded), 0.0);
            store_contact(0u, c);
        }
    "#);
    let mut enc = sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc, &pipeline, 1, 0, 0);
    sim.queue.submit(Some(enc.finish()));
    let contacts = pollster::block_on(sim.read_contacts());
    assert_eq!(contacts[0].ra0, [0.05, 0.15, 0.2, 0.6]);
    assert_eq!(contacts[0].rb0, [0.8, 0.8, 0.5, 0.0]);
}

#[test]
fn joint_pair_retirement_owns_children_and_preserves_other_roots() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    for corrupt in [false, true] {
        let mut sim = make_sim(&gpu, 0);
        let mut contacts = patches().to_vec();
        if corrupt { contacts[1].manifold_link[1] = 8; }
        let mut unrelated = contacts[0];
        unrelated.a = 77; unrelated.b = 78;
        unrelated.manifold_link = [0; 4];
        contacts.push(unrelated);
        sim.queue.write_buffer(&sim.contacts, 0, bytemuck::cast_slice(&contacts));
        sim.queue.write_buffer(&sim.scratch, u64::from(crate::types::SCR_UNIQUE_N) * 4, bytemuck::bytes_of(&2u32));
        sim.queue.write_buffer(&sim.scratch, u64::from(SCR_PAIRS + 2 * PAIR_CAP) * 4, bytemuck::cast_slice(&[0u32, 2]));
        sim.retire_body_pair_contacts(1, 0); // Endpoint order must not matter.
        let after = pollster::block_on(sim.read_contacts());
        let stats = pollster::block_on(sim.read_live_step_stats());
        assert_eq!((after[2].a, after[2].b), (77, 78));
        if corrupt {
            assert_eq!((after[0].a, after[1].a), (0, 0));
            assert!(stats.sticky.contact_pairs_dropped > 0);
            assert_ne!(stats.contact_drop_reasons & (1 << 1), 0);
        } else {
            assert_eq!((after[0].a, after[1].a), (u32::MAX, u32::MAX));
            assert_eq!(stats.sticky.contact_pairs_dropped, 0);
        }
    }
}

#[test]
fn contact_metrics_distinguish_candidates_roots_and_patch_slots_with_step_identity() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu,0);
    assert!(sim.contact_metrics(false).is_none());
    let seed = test_pipeline(&sim,"test_metrics",r#"
        @compute @workgroup_size(1)
        fn test_metrics() {
            store_contact(0u,empty_contact()); store_contact(1u,empty_contact());
            let mode = atomicLoad(&query[70u]);
            let live = mode != 0u;
            let count = select(3u,130u,mode == 2u);
            scratch[SCR_UNIQUE_N] = select(0u,count,live);
            for (var i=0u; i<130u; i++) { scratch[SCR_ACTIVE_CONTACT+i] = EMPTY; }
            if (live) { scratch[SCR_ACTIVE_CONTACT+count-1u] = 0u; }
            if (live) {
                var root = empty_contact(); root.a=0u; root.b=1u; root.count=1u;
                root.manifold_link=vec4<u32>(2u,0u,2u,0u);
                root.lifecycle.y=CONTACT_TOUCHING;
                var child=root; child.manifold_link=vec4<u32>(0u,1u,0u,0u);
                store_contact(0u,root); store_contact(1u,child);
            }
        }
    "#);
    let submit = |sim: &mut GpuSim,step:u64,live:u32| {
        sim.restore_physics_step(step);
        sim.queue.write_buffer(&sim.query,70*4,bytemuck::bytes_of(&live));
        let mut enc=sim.device.create_command_encoder(&Default::default());
        sim.dispatch_n(&mut enc,&seed,1,0,0);
        sim.dispatch_n(&mut enc,&sim.begin_occupied_contacts,1,0,0);
        sim.dispatch_n(&mut enc,&sim.collect_occupied_contacts,3,0,0);
        sim.dispatch_n(&mut enc,&sim.finish_occupied_contacts,1,0,0);
        sim.encode_contact_status(&mut enc);
        sim.record_submit(sim.queue.submit(Some(enc.finish())));
        sim.map_contact_status();
    };
    let first=(1u64<<32)+11;
    submit(&mut sim,first,1);
    // Keep the status slot occupied across another submit. Its data and ID
    // must remain paired, even if a newer completed GPU state is now empty.
    submit(&mut sim,first+1,0);
    let old=sim.contact_metrics(true).unwrap();
    assert_eq!(old,ContactMetrics {step:first,topology_revision:0,state_revision:0,capacity_loss:false,candidate_pairs:3,allocated_roots:1,
        allocated_manifold_slots:2,touching_roots:1,non_sensor_roots:0});
    assert_eq!(sim.physics_step(),first+1);
    assert_eq!(sim.pose_snapshot_maps(),0);
    submit(&mut sim,first+2,0);
    assert_eq!(sim.contact_metrics(true).unwrap(),ContactMetrics {step:first+2,..ContactMetrics::default()});
    let contacts=pollster::block_on(sim.read_contacts());
    assert!(contacts.iter().all(|c|c.a==u32::MAX));
    submit(&mut sim,first+3,2);
    assert_eq!(sim.contact_metrics(true).unwrap(),ContactMetrics {step:first+3,topology_revision:0,state_revision:0,
        candidate_pairs:130,allocated_roots:1,allocated_manifold_slots:2,touching_roots:1,non_sensor_roots:0,capacity_loss:false});
    let actual=pollster::block_on(sim.read_contacts());
    assert_eq!(actual.iter().filter(|c|c.a!=u32::MAX).count(),2);
    assert_eq!(actual.iter().filter(|c|c.a!=u32::MAX && c.manifold_link[1]==0).count(),1);
}

#[test]
fn dynamic_greedy_colors_match_reference_with_holes_and_overflow() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
    for (shared,span,edge_count) in [(false,32,193),(true,32,127),(true,32,128),(true,32,193),(true,7975,193),(true,7976,193),(false,8168,193),(true,8168,193),(true,8169,193)] {
    let mut sim = make_sim_sized(&gpu, 0,span);
    sim.graph_shared_requested=shared;
    assert_eq!(sim.shared_graph_eligible(),shared && span<=8168);
    sim.params.body_count = span;
    sim.upload_pass_lut();
    let seed = test_pipeline(&sim, "seed_greedy_graph", &r#"
        @compute @workgroup_size(1)
        fn seed_greedy_graph() {
            for (var b=0u; b<params.body_count; b++) {
                atomicStore(&atom[ATOM_JACOBI+b],select(0u,(1u<<20u)|(1u<<22u),b%3u==0u));
            }
            for (var c=0u; c<24u; c++) {atomicStore(&atom[atom_graph_color()+c],0u);}
            for (var i=0u; i<EDGE_COUNT; i++) {
                var a=(i*13u)%32u;
                var b=(a+1u+i%17u)%32u;
                if (i<24u) {a=0u;b=i+1u;}
                var c=empty_contact();c.a=a+params.body_count-32u;c.b=b+params.body_count-32u;c.count=1u;
                store_contact(i,c);
                scratch[SCR_ACTIVE_CONTACT+i]=i;
                scratch[SCR_NEXT_OCCUPIED+i]=i;
            }
            scratch[SCR_DYN_DYN_N]=EDGE_COUNT;
            scratch[SCR_NCONTACTS]=EDGE_COUNT;
        }
    "#.replace("EDGE_COUNT", &format!("{edge_count}u")));
    let mut enc=sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc,&seed,1,0,1);
    sim.dispatch_n(&mut enc,sim.dynamic_graph_pipeline(),1,0,1);
    sim.queue.submit(Some(enc.finish()));
    let contacts=pollster::block_on(sim.read_contacts());
    let mut masks: Vec<u32>=(0..span).map(|b|if b%3==0 {(1<<20)|(1<<22)} else {0}).collect();
    let mut overflow=0;
    let mut counts=[0u32;24];
    for c in contacts.iter().take(edge_count) {
        let (a,b)=(c.a as usize,c.b as usize);
        let expected=(0..20).find(|color|(masks[a]|masks[b])&(1<<color)==0).unwrap_or(OVERFLOW_COLOR);
        assert_eq!(c.color,expected,"edge {a}-{b}");
        assert_eq!(c.lifecycle[2],counts[expected as usize],"per-color order changed");
        counts[expected as usize]+=1;
        if expected==OVERFLOW_COLOR {overflow+=1;} else {masks[a]|=1<<expected;masks[b]|=1<<expected;}
    }
    assert!(overflow>=4,"fixture must exercise all dynamic colors and overflow");
    let mut enc=sim.device.create_command_encoder(&Default::default());
    let copy_masks=test_pipeline(&sim,"copy_graph_masks",&r#"
        @compute @workgroup_size(64)
        fn copy_graph_masks(@builtin(global_invocation_id) gid:vec3<u32>) {
            if (gid.x<params.body_count) {scratch[gid.x]=atomicLoad(&atom[ATOM_JACOBI+gid.x]);}
            if (gid.x<EDGE_COUNT) {
                let c=contacts[gid.x];let local=contact_persistent[gid.x].lifecycle.z;
                scratch[params.body_count+gid.x]=scratch[color_contact_base()+c.color*params.contact_capacity+local];
            }
            if (gid.x<24u) {scratch[params.body_count+EDGE_COUNT+gid.x]=atomicLoad(&atom[atom_graph_color()+gid.x]);}
        }
    "#.replace("EDGE_COUNT",&format!("{edge_count}u")));
    sim.dispatch_n(&mut enc,&copy_masks,span.max(edge_count as u32).div_ceil(64),0,1);
    sim.queue.submit(Some(enc.finish()));
    masks.extend(0..edge_count as u32);
    masks.extend(counts);
    assert_eq!(pollster::block_on(sim.read_scratch_prefix(masks.len() as u32)),masks);
    assert_eq!(sim.shared_graph_initialized(),shared && span<=8168);
    if shared && span<=8168 {
        sim.params.body_count=8169;
        assert!(!sim.shared_graph_eligible());
        assert!(std::ptr::eq(sim.dynamic_graph_pipeline(),&sim.graph_assign_dynamic));
    }

    }
    if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
}

#[test]
fn paired_graph_compaction_preserves_both_ordered_lists() {
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
    gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
    let mut sim=make_sim(&gpu,0);
    let seed=test_pipeline(&sim,"seed_paired_compact",r#"
        @compute @workgroup_size(256)
        fn seed_paired_compact(@builtin(global_invocation_id) gid:vec3<u32>) {
            let i=gid.x;
            if (i<scratch[SCR_UNIQUE_N]) {
                let mode=scratch[58u];
                let kind=select(i%4u,mode,mode<3u);
                scratch[SCR_RADIX_HIST+i]=(i<<2u)|kind;
                scratch[SCR_OCCUPIED_CONTACT+i]=0xa5000000u+i;
            }
            if (i<2u*RADIX_GROUPS) {scratch[SCR_INS_CELL+i]=0xdeadbeefu;}
        }
    "#);
    for n in [0u32,1,255,256,257,511,512,513,PAIR_CAP,17,0] {
        for mode in [0u32,1,2,3] {
            let expect_static:Vec<u32>=(0..n).filter(|i|if mode<3 {mode==1}else{i%4==1}).collect();
            let expect_dynamic:Vec<u32>=(0..n).filter(|i|if mode<3 {mode==2}else{i%4==2}).collect();
            {
                sim.queue.write_buffer(&sim.scratch,u64::from(crate::types::SCR_UNIQUE_N)*4,bytemuck::bytes_of(&n));
                sim.queue.write_buffer(&sim.scratch,58*4,bytemuck::bytes_of(&mode));
                let mut enc=sim.device.create_command_encoder(&Default::default());
                sim.dispatch_n(&mut enc,&seed,n.max(512).div_ceil(256),0,1);
                let groups=n.div_ceil(256).max(1);
                let p=sim.paired_graph_pipelines();
                sim.dispatch_n(&mut enc,&p[0],groups,0,1);
                sim.dispatch_n(&mut enc,&p[1],1,0,1);
                sim.dispatch_n(&mut enc,&p[2],groups,0,1);
                sim.queue.submit(Some(enc.finish()));
                let words=pollster::block_on(sim.read_scratch_prefix(crate::types::SCR_RADIX_OUT+n));
                assert_eq!(words[57] as usize,expect_static.len(),"static count n={n} mode={mode}");
                assert_eq!(words[56] as usize,expect_dynamic.len());
                let start=crate::types::SCR_RADIX_OUT as usize;
                assert_eq!(&words[start..start+expect_static.len()],expect_static.as_slice());
                let start=crate::types::SCR_NEXT_OCCUPIED as usize;
                assert_eq!(&words[start..start+expect_dynamic.len()],expect_dynamic.as_slice());
                let previous=crate::types::SCR_OCCUPIED_CONTACT as usize;
                for i in 0..n as usize {assert_eq!(words[previous+i],0xa5000000+i as u32,
                    "graph compaction overwrote previous occupied root {i}");}
            }
        }
    }
    if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
}


#[test]
fn single_manifold_rejects_corrupt_ownership_and_points() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    for corruption in 0..4 {
        let mut bad = patches()[0];
        bad.manifold_link = [0; 4];
        match corruption {
            0 => bad.manifold_link[1] = 7,
            1 => bad.manifold_link[0] = 1,
            2 => bad.count = 5,
            3 => bad.b = bad.a,
            _ => unreachable!(),
        }
        for path in PATHS {
            let (bodies, stats) = run(&gpu, vec![bad], path, &[2, 0, 3]);
            assert_velocity(&bodies, [-2.0, -2.0, 0.0],
                &format!("single bad {corruption} {path:?}"));
            assert!(stats.drops.contact_pairs_dropped > 0);
        }
    }
}

#[test]
fn small_component_rejects_bad_chains_before_integration() {
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
    gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
    for corruption in 0..6 {
        let mut sim=make_sim(&gpu,0);
        // Production enables this path through the setter, which grows and
        // rebinds the component metadata buffer. Seed that same allocation.
        sim.set_component_tgs(true);
        let mut chain=patches().to_vec();
        match corruption {
            0=>chain[0].manifold_link[1]=7,
            1=>chain[1].manifold_link[0]=1,
            2=>chain[1].count=5,
            3=>chain[1].b=chain[1].a,
            4=>chain[0].manifold_link[0]=sim.params.contact_capacity+1,
            5=>{}, // Valid chain proves the seeded component actually executes.
            _=>unreachable!(),
        }
        sim.queue.write_buffer(&sim.contacts,0,bytemuck::cast_slice(&chain));
        let setup=test_pipeline(&sim,"seed_small_component",r#"
@compute @workgroup_size(1)
fn seed_small_component() {
    atomicStore(&atom[atom_island_label()],EMPTY);
    atomicStore(&atom[atom_island_label()+1u],1u);
    atomicStore(&query[260u],0u);
    let m=component_meta(1u);
    atomicStore(&query[m],1u);atomicStore(&query[m+1u],1u);
    atomicStore(&query[m+2u],0u);atomicStore(&query[m+3u],0u);
    atomicStore(&query[component_bodies()],1u);
    atomicStore(&query[component_contacts()],0u);
    atomicStore(&query[component_contacts()+1u],0u);
}
"#);
        let mut enc=sim.device.create_command_encoder(&wgpu::CommandEncoderDescriptor{label:Some("bad-small-component")});
        sim.dispatch_n(&mut enc,&setup,1,0,1);
        // Deliberately bypass prepare: the component must guard its own chains.
        sim.dispatch_n(&mut enc,&sim.solve_complete_components,1,0,1);
        sim.queue.submit(Some(enc.finish()));
        let bodies=pollster::block_on(sim.read_bodies());
        let stats=pollster::block_on(sim.read_live_step_stats());
        if corruption == 5 {
            assert!(bodies[1].dp[0] < 0.0, "valid component did not integrate");
            assert_eq!(stats.drops.contact_pairs_dropped, 0, "valid chain rejected");
        } else {
            assert_velocity(&bodies,[-2.0,-2.0,0.0],&format!("bad small component {corruption}"));
            assert_eq!(bodies[1].dp,[0.0;3],"invalid component integrated");
            assert!(stats.drops.contact_pairs_dropped>0,"bad chain {corruption} was not reported");
        }
    }
    if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()){panic!("{error}");}
}

#[test]
fn large_component_rejects_bad_chains_before_integration() {
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
    gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
    for corruption in 0..6 {
        let mut sim=make_sim(&gpu,0);
        // Production enables this path through the setter, which grows and
        // rebinds the component metadata buffer. Seed that same allocation.
        sim.set_component_tgs(true);
        let mut chain=patches().to_vec();
        match corruption {
            0=>chain[0].manifold_link[1]=7,
            1=>chain[1].manifold_link[0]=1,
            2=>chain[1].count=5,
            3=>chain[1].b=chain[1].a,
            4=>chain[0].manifold_link[0]=sim.params.contact_capacity+1,
            5=>{}, // Valid chain proves the seeded component actually executes.
            _=>unreachable!(),
        }
        sim.queue.write_buffer(&sim.contacts,0,bytemuck::cast_slice(&chain));
        let setup=test_pipeline(&sim,"seed_large_component",r#"
@compute @workgroup_size(1)
fn seed_large_component() {
    atomicStore(&atom[atom_island_label()],EMPTY);
    atomicStore(&atom[atom_island_label()+1u],1u);
    atomicStore(&query[260u],0u);
    atomicStore(&query[256u],1u);
    atomicStore(&query[component_large_roots()],1u);
    atomicStore(&query[component_color_counts()+24u],1u);
    atomicStore(&query[component_color_starts()+24u],0u);
    let m=component_meta(1u);
    atomicStore(&query[m],1u);atomicStore(&query[m+1u],1u);
    atomicStore(&query[m+2u],0u);atomicStore(&query[m+3u],0u);
    atomicStore(&query[component_bodies()],1u);
    atomicStore(&query[component_contacts()],0u);
    atomicStore(&query[component_contacts()+1u],0u);
}
"#);
        let mut enc=sim.device.create_command_encoder(&wgpu::CommandEncoderDescriptor{label:Some("bad-small-component")});
        sim.dispatch_n(&mut enc,&setup,1,0,1);
        // Deliberately bypass prepare: the component must guard its own chains.
        sim.dispatch_n(&mut enc,&sim.solve_large_components,1,0,1);
        sim.queue.submit(Some(enc.finish()));
        let bodies=pollster::block_on(sim.read_bodies());
        let stats=pollster::block_on(sim.read_live_step_stats());
        if corruption == 5 {
            assert!(bodies[1].dp[0] < 0.0, "valid component did not integrate");
            assert_eq!(stats.drops.contact_pairs_dropped, 0, "valid chain rejected");
        } else {
            assert_velocity(&bodies,[-2.0,-2.0,0.0],&format!("bad small component {corruption}"));
            assert_eq!(bodies[1].dp,[0.0;3],"invalid component integrated");
            assert!(stats.drops.contact_pairs_dropped>0,"bad chain {corruption} was not reported");
        }
    }
    if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()){panic!("{error}");}
}

#[test]
fn occupied_retirement_keeps_input_until_publication() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    let mut shapes = [ShapeGpu::zeroed(); 2];
    for (i, shape) in shapes.iter_mut().enumerate() {
        shape.body_index = i as u32;
        shape.half = [0.5; 3];
        shape.category_bits_lo = 1;
        shape.mask_bits_lo = u32::MAX;
    }
    sim.params.shape_count = 2;
    sim.queue.write_buffer(&sim.body_cold, u64::from(sim.params.shape_base_u32)*4,
        bytemuck::cast_slice(&shapes));
    sim.upload_pass_lut();
    let seed = test_pipeline(&sim, "seed_occupied_retirement", r#"
        @compute @workgroup_size(64)
        fn seed_occupied_retirement(@builtin(global_invocation_id) gid: vec3<u32>) {
            let i = gid.x;
            if (i >= 512u) { return; }
            var c = empty_contact();
            c.a = select(900u, 0u, i % 2u == 0u);
            c.b = select(901u, 1u, i % 2u == 0u);
            c.count = 1u;
            c.lifecycle.w = 1u << 16u;
            store_contact(i, c);
            scratch[SCR_OCCUPIED_CONTACT + i] = 511u - i;
            scratch[SCR_CONTACT_MARK + i] = select(0u, 1u, i == 0u);
            if (i == 0u) {
                scratch[SCR_OCCUPIED_N] = 512u;
                scratch[SCR_UNIQUE_N] = 1u;
                scratch[SCR_ACTIVE_CONTACT] = 0u;
            }
        }
    "#);
    let mut enc=sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc,&seed,8,0,0);
    sim.dispatch_n(&mut enc,&sim.begin_occupied_contacts,1,0,0);
    sim.dispatch_n(&mut enc,&sim.retire_stale_contacts,8,0,0);
    sim.queue.submit(Some(enc.finish()));
    let words=pollster::block_on(sim.read_scratch_prefix(crate::types::SCR_OCCUPIED_CONTACT+512));
    let start=crate::types::SCR_OCCUPIED_CONTACT as usize;
    for i in 0..512 {assert_eq!(words[start+i],511-i as u32,
        "parallel retirement overwrote its input at {i}");}
    let mut enc=sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc,&sim.collect_occupied_contacts,1,0,0);
    sim.dispatch_n(&mut enc,&sim.finish_occupied_contacts,1,0,0);
    sim.queue.submit(Some(enc.finish()));
    let words=pollster::block_on(sim.read_scratch_prefix(crate::types::SCR_OCCUPIED_CONTACT+512));
    assert_eq!(words[crate::types::SCR_OCCUPIED_N as usize],256);
    let mut actual=words[start..start+256].to_vec();actual.sort_unstable();
    assert_eq!(actual,(0..512u32).step_by(2).collect::<Vec<_>>(),
        "each surviving root must be published exactly once, including the active root");
    assert_eq!(pollster::block_on(sim.read_live_step_stats()).sticky.contact_pairs_dropped,0);
}

#[test]
fn hull_reduction_preserves_small_polygons_and_spreads_large_ones() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut sim = make_sim(&gpu, 0);
    let pipeline = test_pipeline(&sim, "test_hull_reduction", r#"
        @compute @workgroup_size(1)
        fn test_hull_reduction() {
            var body = load_body(1u);
            body.pos = vec3<f32>(0.0);
            body.rot = vec4<f32>(0.0, 0.0, 0.0, 1.0);
            var polygon: ClipPolygon;
            polygon.count = 4u;
            polygon.points[0] = vec3<f32>(0.0, 0.0, 0.0);
            polygon.points[1] = vec3<f32>(1.0, 0.0, 0.0);
            polygon.points[2] = vec3<f32>(1.1, 0.0, 0.2);
            polygon.points[3] = vec3<f32>(0.2, 0.0, 1.0);
            let n = vec3<f32>(0.0, 1.0, 0.0);
            let ref_point = vec3<f32>(0.0, 0.01, 0.0);
            var c = empty_contact(); c.a = 0u; c.b = 1u; c.count = 1u;
            c.ra0 = vec4<f32>(reduce_hull_polygon(polygon, body, n, ref_point));
            polygon.count = 8u;
            for (var i = 0u; i < 8u; i++) {
                let angle = f32(i) * 0.7853981633974483;
                polygon.points[i] = vec3<f32>(cos(angle), 0.0, sin(angle));
            }
            c.rb0 = vec4<f32>(reduce_hull_polygon(polygon, body, n, ref_point));
            store_contact(0u, c);
        }
    "#);
    let mut enc = sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc, &pipeline, 1, 0, 0);
    sim.queue.submit(Some(enc.finish()));
    let contacts = pollster::block_on(sim.read_contacts());
    assert_eq!(contacts[0].ra0, [0.0, 1.0, 2.0, 3.0]);
    let indices = contacts[0].rb0;
    for (i, &index) in indices.iter().enumerate() {
        assert!(index >= 0.0 && index < 8.0 && index.fract() == 0.0);
        assert!(!indices[..i].contains(&index), "duplicate retained polygon vertex");
    }
    let mut angles: Vec<_> = indices.iter().map(|i| i * std::f32::consts::FRAC_PI_4).collect();
    angles.sort_by(f32::total_cmp);
    let area = 0.5 * (0..4).map(|i| (angles[(i+1)%4] - angles[i]).sin()).sum::<f32>();
    assert!(area > 1.6, "retained polygon must cover the contact patch: {area}");
}

#[test]
fn hull_triangle_incident_face_matches_native_support_edge_choice() {
    // Generated from Box3D b3FindIncidentFace using the Gear Lift rock hull.
    // Global normal alignment chooses face 6; the support-edge witness chooses 7.
    mod fixture { include!("fixtures/hull_incident_face.rs"); }
    let points = fixture::POINTS;
    let planes = fixture::PLANES;
    let topology = fixture::TOPOLOGY;
    let normal = fixture::NORMAL;
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let mut body = BodyGpu::zeroed(); body.rot=[0.0,0.0,0.0,1.0];body.dq=body.rot;
    body.kind=crate::types::KIND_CONVEX_HULL;body.inv_mass=1.0;body.inv_inertia=[1.0;3];
    let mut shape = crate::types::ShapeGpu::zeroed();shape.kind=body.kind;
    shape.topology_counts=points.len() as u32 | ((planes.len() as u32)<<8) | ((topology.len() as u32)<<24);
    let caps=GpuSceneCaps::allocate(GpuSceneCaps::live(1,1,points.len() as u32,planes.len() as u32,0,topology.len() as u32,0,0,0,0,0),1,1);
    let mut sim=GpuSim::new(&gpu,&[body],&[[0.0;3]],&[[0.0;3]],&[shape],&points,&planes,&[],&topology,&[],&[],&[],&[],&[],&[],caps,true,1.0/60.0,[0.0;3]).unwrap();
    let shader=format!(r#"
        @compute @workgroup_size(1)
        fn test_incident_face() {{
            let shape=load_shape(0u); let body=load_body(0u);
            let n=vec3<f32>({:?},{:?},{:?});
            var c=empty_contact();c.count=1u;
            c.ra0=vec4<f32>(f32(hull_triangle_incident_face(shape,body,n)),f32(best_hull_face(shape,body,-n)),0.0,0.0);
            store_contact(0u,c);
        }}
    "#,normal[0],normal[1],normal[2]);
    let pipeline=test_pipeline(&sim,"test_incident_face",&shader);
    let mut enc=sim.device.create_command_encoder(&Default::default());
    sim.dispatch_n(&mut enc,&pipeline,1,0,0);sim.queue.submit(Some(enc.finish()));
    let contacts=pollster::block_on(sim.read_contacts());
    assert_eq!(contacts[0].ra0[0],fixture::EXPECTED);
    assert_eq!(contacts[0].ra0[1],fixture::OLD);
    assert_ne!(contacts[0].ra0[0],contacts[0].ra0[1]);
}
