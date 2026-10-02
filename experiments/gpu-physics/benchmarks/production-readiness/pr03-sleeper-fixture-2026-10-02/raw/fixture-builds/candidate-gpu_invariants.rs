//! GPU regression coverage for unique-pair compact and graph coloring.

use crate::api::{
    b3_body_apply_angular_impulse, b3_body_apply_mass_from_shapes, b3_body_get_angular_velocity,
    b3_body_get_local_center, b3_body_get_mass_data,
    b3_body_get_position, b3_body_get_rotation, b3_create_body, b3_create_capsule_shape,
    b3_create_convex_hull_shape, b3_create_height_field_shape, b3_create_hull_shape,
    b3_create_mesh_shape, b3_create_world, b3_default_body_def, b3_default_shape_def,
    b3_default_world_def, b3_destroy_shape, b3_destroy_world, b3_make_box_hull, b3_make_quat_from_axis_angle,
    b3_replace_mesh_shape, b3_world_contact_event_ptrs, b3_world_enable_continuous,
    b3_world_enable_sleeping, b3_world_ensure_gpu, b3_world_gpu_wait, b3_world_gpu_wait_with_mirror,
    b3_world_prepare_pose_snapshot, b3_world_step, b3_world_sync_contacts, b3_world_step_gpu,
    b3_world_set_diagnostic_flags, b3_world_physics_step,
    b3_world_last_solver_dispatches, b3_world_last_static_sort_dispatches, b3_world_last_joint_dispatches,
    b3_world_live_step_stats, b3_world_clear_capacity_status, b3_world_physics_invalid,
    b3_body_set_linear_velocity, b3_body_get_linear_velocity, b3_body_is_awake,
    b3_body_set_mass_data, b3_body_set_transform, b3_world_set_custom_filter_callback, BodyType,
    Capsule, ConvexHull, Filter, MassData, MeshNode, MotionLocks, Sphere, B3_DEG_TO_RAD, ShapeId,
    b3_create_revolute_joint, b3_create_sphere_shape, b3_create_weld_joint, b3_create_prismatic_joint,
    b3_create_wheel_joint, b3_create_spherical_joint, b3_create_distance_joint, b3_create_parallel_joint,
    b3_create_motor_joint,
    b3_default_revolute_joint_def, b3_default_weld_joint_def, b3_default_prismatic_joint_def,
    b3_default_wheel_joint_def, b3_default_spherical_joint_def, b3_default_distance_joint_def,
    b3_default_parallel_joint_def, b3_default_motor_joint_def, b3_destroy_body, b3_body_set_awake, b3_shape_get_aabb,
    b3_world_gpu_fail, b3_world_cast_ray_closest, b3_default_query_filter,
};
use crate::scenes::{create_ground, create_high_resistance, create_revolute};
use crate::sim::{GpuDevice, GpuSim};
use crate::types::{
    pair_hash_mix, BodyGpu, BroadphaseStats, GpuSceneCaps, CONTACT_HASH_CAP, PAIR_CAP,
    SCR_COLOR, SCR_UNIQUE_N,     OVERFLOW_COLOR, DIAG_FORCE_CAPACITY_LOSS,
    DIAG_GENERAL_SOLVER, DIAG_FORCE_GENERAL_STATIC_SORT, DIAG_RECOMPUTE_TOPOLOGY,
    FLAG_DISABLED, FLAG_KINEMATIC, FLAG_STATIC, gpu_is_non_dynamic,
    DIAG_SERIAL_JOINTS, DIAG_PARALLEL_JOINTS, DIAG_DISABLE_RECYCLING, SPECULATIVE_DISTANCE,
};

fn cpu_unique(sorted_pairs: &[u32]) -> Vec<u32> {
    let mut out = Vec::new();
    for (i, &p) in sorted_pairs.iter().enumerate() {
        if p == u32::MAX {
            continue;
        }
        if i == 0 || p != sorted_pairs[i - 1] {
            out.push(p);
        }
    }
    out
}

fn dummy_sim(gpu: &GpuDevice) -> GpuSim {
    let body = BodyGpu::zeroed();
    let caps = GpuSceneCaps::allocate(
        GpuSceneCaps::live(1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
        1,
        1,
    );
    GpuSim::new(
        gpu,
        &[body],
        &[[0.0; 3]],
        &[[0.0; 3]],
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
        [0.0, -10.0, 0.0],
    )
    .expect("create GpuSim")
}

fn compact_case(gpu: &GpuDevice, pairs: &[u32]) {
    let expect = cpu_unique(pairs);
    let mut sim = dummy_sim(gpu);
    let (unique, raw, compacted) = pollster::block_on(sim.debug_compact_unique(&pairs.iter().map(|&v| if v==u32::MAX {u64::MAX} else {u64::from(v)}).collect::<Vec<_>>(), 0x00FF_FF00));
    assert_eq!(raw, expect.len() as u32, "unbounded unique count");
    assert_eq!(unique, expect.len() as u32, "bounded unique count");
    assert!(unique <= PAIR_CAP);
    assert_eq!(&compacted[..unique as usize], expect.iter().map(|&v|u64::from(v)).collect::<Vec<_>>().as_slice());
}

#[test]
fn compact_unique_matches_cpu_reference_with_stale_radix() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    compact_case(&gpu, &[]);
    compact_case(&gpu, &[1]);
    compact_case(&gpu, &[7, 7, 7, 7]);
    compact_case(&gpu, &(0..255).collect::<Vec<_>>());
    compact_case(&gpu, &(0..256).collect::<Vec<_>>());
    compact_case(&gpu, &(0..257).collect::<Vec<_>>());
    let mut dups = Vec::new();
    for i in 0..400u32 {
        dups.push(i / 3);
    }
    compact_case(&gpu, &dups);
    compact_case(&gpu, &(0..10).collect::<Vec<_>>());
    compact_case(&gpu, &(0..300).collect::<Vec<_>>());
    compact_case(&gpu, &(0..2048).collect::<Vec<_>>());
    compact_case(&gpu, &(0..8192).collect::<Vec<_>>());
    compact_case(&gpu, &(0..PAIR_CAP).collect::<Vec<_>>());
}

#[test]
fn box_contacts_stay_bounded_and_conflict_free() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 20.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    let shape = b3_default_shape_def();
    let hull = b3_make_box_hull(0.5, 0.5, 0.5);
    let mut ids = Vec::new();
    for i in 0..6 {
        def.position = [0.0, 0.5 + i as f32 * 1.05, 0.0];
        let body = b3_create_body(world, &def);
        b3_create_hull_shape(body, &shape, &hull);
        ids.push(body);
    }
    b3_world_ensure_gpu(world);
    for _ in 0..120 {
        b3_world_step(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let mut keys = std::collections::BTreeSet::new();
    let mut per_color: Vec<Vec<(u32, u32)>> = vec![Vec::new(); 24];
    for c in &contacts {
        if c.a == u32::MAX || c.b == u32::MAX || c.count == 0 {
            continue;
        }
        let key = (c.a.min(c.b), c.a.max(c.b));
        assert!(
            keys.insert(key),
            "duplicate dense contact for body pair {key:?}"
        );
        assert!(c.count > 0, "touching contact has zero points");
        if c.color < 24 {
            per_color[c.color as usize].push((c.a, c.b));
        }
    }
    for (col, list) in per_color.iter().enumerate() {
        if col == OVERFLOW_COLOR as usize {
            continue;
        }
        for (i, (a, b)) in list.iter().enumerate() {
            for (c, d) in list.iter().skip(i + 1) {
                let share_dyn = (*a != 0 && (*a == *c || a == d)) || (*b != 0 && (b == c || b == d));
                assert!(
                    !share_dyn,
                    "color {col} shares a dynamic body between {a}-{b} and {c}-{d}"
                );
            }
        }
    }
    assert!(
        keys.len() >= 6,
        "expected stacked boxes to keep ground/neighbor contacts, got {}",
        keys.len()
    );
    b3_destroy_world(world);
}

#[test]
fn pair_hash_mix_spreads_ground_keys() {
    let cap = PAIR_CAP;
    let mut buckets = vec![0u32; cap as usize];
    for i in 1..5431u32 {
        let packed = i;
        buckets[(pair_hash_mix(packed) & (cap - 1)) as usize] += 1;
    }
    let occupied = buckets.iter().filter(|c| **c > 0).count();
    let max = *buckets.iter().max().unwrap();
    assert!(
        occupied > 2000,
        "ground-style keys collapsed into {occupied} buckets, max {max}"
    );
    assert!(max < 20, "ground-style probe chains too long: max {max}");

    let hi_cap = CONTACT_HASH_CAP;
    let mut hi = vec![0u32; hi_cap as usize];
    for i in 1..5431u32 {
        let packed = i;
        hi[(pair_hash_mix(packed) & (hi_cap - 1)) as usize] += 1;
    }
    let hi_max = *hi.iter().max().unwrap();
    assert!(hi_max < 12, "contact-hash chains too long: max {hi_max}");
}

#[test]
fn pair_hash_mix_spreads_constant_high_half() {
    let cap = PAIR_CAP;
    let mut buckets = vec![0u32; 256];
    for i in 0..4000u32 {
        let packed = (i << 16) | 7;
        buckets[(pair_hash_mix(packed) & (cap - 1)) as usize % 256] += 1;
    }
    let max = *buckets.iter().max().unwrap();
    assert!(max < 80, "high-half keys clustered: max {max}");
}

#[test]
fn pair_hash_mix_sequential_and_random_pairs() {
    let cap = PAIR_CAP;
    let mut buckets = vec![0u32; 1024];
    for i in 0..8000u32 {
        let packed = ((i + 1) << 16) | i;
        buckets[(pair_hash_mix(packed) & (cap - 1)) as usize % 1024] += 1;
    }
    let max = *buckets.iter().max().unwrap();
    assert!(max < 40, "sequential pairs clustered: max {max}");

    let mut x = 0x1234_5678u32;
    buckets.fill(0);
    for _ in 0..8000u32 {
        x = x.wrapping_mul(1664525).wrapping_add(1013904223);
        let a = x & 0xffff;
        let b = (x >> 16) & 0xffff;
        let packed = a.max(b) << 16 | a.min(b);
        buckets[(pair_hash_mix(packed) & (cap - 1)) as usize % 1024] += 1;
    }
    let max = *buckets.iter().max().unwrap();
    assert!(max < 40, "random pairs clustered: max {max}");
}

#[test]
fn pair_hash_table_survives_collisions_and_tombstones() {
    const CAP: usize = 64;
    let mut keys = vec![u32::MAX; CAP];
    let mix = |k: u32| pair_hash_mix(k) as usize & (CAP - 1);
    let insert = |keys: &mut [u32], key: u32| {
        let mut slot = mix(key);
        let mut tomb = None;
        for _ in 0..CAP {
            if keys[slot] == key {
                return true;
            }
            if keys[slot] == u32::MAX - 1 && tomb.is_none() {
                tomb = Some(slot);
            }
            if keys[slot] == u32::MAX {
                keys[tomb.unwrap_or(slot)] = key;
                return true;
            }
            slot = (slot + 1) % CAP;
        }
        false
    };
    let lookup = |keys: &[u32], key: u32| {
        let mut slot = mix(key);
        for _ in 0..CAP {
            if keys[slot] == key {
                return true;
            }
            if keys[slot] == u32::MAX {
                return false;
            }
            slot = (slot + 1) % CAP;
        }
        false
    };
    let retire = |keys: &mut [u32], key: u32| {
        let mut slot = mix(key);
        for _ in 0..CAP {
            if keys[slot] == key {
                keys[slot] = u32::MAX - 1;
                return;
            }
            if keys[slot] == u32::MAX {
                return;
            }
            slot = (slot + 1) % CAP;
        }
    };
    let mut live = Vec::new();
    for i in 0..40u32 {
        let key = (i << 16) | 0;
        assert!(insert(&mut keys, key));
        live.push(key);
    }
    for key in live.iter().take(15).copied() {
        retire(&mut keys, key);
        assert!(!lookup(&keys, key));
    }
    for i in 100..120u32 {
        let key = (i << 16) | 0;
        assert!(insert(&mut keys, key));
        assert!(lookup(&keys, key));
    }
    for key in live.iter().skip(15).copied() {
        assert!(lookup(&keys, key), "live key lost after tombstone reuse {key}");
    }
}

#[test]
fn box_stack_reuses_slots_on_same_world() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 20.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    let shape = b3_default_shape_def();
    let hull = b3_make_box_hull(0.5, 0.5, 0.5);
    for i in 0..4 {
        def.position = [0.0, 0.5 + i as f32 * 1.05, 0.0];
        let body = b3_create_body(world, &def);
        b3_create_hull_shape(body, &shape, &hull);
    }
    b3_world_ensure_gpu(world);
    for wave in 0..3 {
        for _ in 0..80 {
            b3_world_step(world, 1.0 / 60.0, 4);
        }
        b3_world_gpu_wait(world);
        let live = pollster::block_on(b3_world_sync_contacts(world))
            .iter()
            .filter(|c| c.a != u32::MAX && c.count > 0)
            .count();
        assert!(
            live >= 4,
            "wave {wave} dropped stacked contacts, live={live}"
        );
    }
    b3_destroy_world(world);
}

#[test]
fn scratch_layout_matches_shaders() {
    assert_eq!(SCR_UNIQUE_N, 2);
    assert_eq!(SCR_COLOR, 16);
    assert!(crate::types::SCR_PAIRS > crate::types::SCR_HASH);
    assert!(crate::types::SCR_RADIX_BASE > crate::types::SCR_RADIX_HIST);
    let _ = BroadphaseStats::default();
}

#[test]
fn pose_snapshot_is_not_remapped_per_getter() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 20.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    let shape = b3_default_shape_def();
    let hull = b3_make_box_hull(0.5, 0.5, 0.5);
    let mut ids = Vec::new();
    for i in 0..6 {
        def.position = [0.0, 0.5 + i as f32 * 1.05, 0.0];
        let body = b3_create_body(world, &def);
        b3_create_hull_shape(body, &shape, &hull);
        ids.push(body);
    }
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let copies = crate::api::b3_world_pose_snapshot_copies(world);
    let kicks = crate::api::b3_world_pose_snapshot_kicks(world);
    for _ in 0..4000 {
        for id in &ids {
            let _ = std::hint::black_box(b3_body_get_position(*id));
            let _ = std::hint::black_box(b3_body_get_rotation(*id));
        }
    }
    assert_eq!(
        crate::api::b3_world_pose_snapshot_copies(world),
        copies,
        "getters remapped the pose snapshot"
    );
    assert_eq!(crate::api::b3_world_pose_snapshot_kicks(world), kicks);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    assert_eq!(
        crate::api::b3_world_pose_snapshot_copies(world),
        copies + 1,
        "one step must refresh the snapshot once"
    );
    b3_destroy_world(world);
}

#[test]
fn setter_survives_stale_pose_snapshot() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 20.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 10.0, 0.0];
    let body = b3_create_body(world, &def);
    let shape = b3_default_shape_def();
    let hull = b3_make_box_hull(0.5, 0.5, 0.5);
    b3_create_hull_shape(body, &shape, &hull);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    crate::api::b3_body_set_transform(body, [100.0, 20.0, 0.0], [0.0, 0.0, 0.0, 1.0]);
    b3_world_gpu_wait(world);
    let pos = b3_body_get_position(body);
    assert!(
        (pos[0] - 100.0).abs() < 1e-3 && (pos[1] - 20.0).abs() < 1e-3,
        "stale snapshot overwrote setter: {pos:?}"
    );
    b3_destroy_world(world);
}

#[test]
fn pose_snapshot_maps_once_per_kick() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 4.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 1.0, 0.0];
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let maps = crate::api::b3_world_pose_snapshot_maps(world);
    for _ in 0..64 {
        let _ = b3_body_get_position(body);
        let _ = b3_body_get_rotation(body);
    }
    assert_eq!(crate::api::b3_world_pose_snapshot_maps(world), maps);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    assert_eq!(crate::api::b3_world_pose_snapshot_maps(world), maps + 1);
    b3_destroy_world(world);
}

#[test]
fn resting_box_stays_on_ground() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 20.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    for _ in 0..240 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    let pos = b3_body_get_position(body);
    let vel = crate::api::b3_body_get_linear_velocity(body);
    let speed = (vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]).sqrt();
    assert!(pos[1] > 0.4 && pos[1] < 0.7, "rest height {pos:?}");
    assert!(speed < 0.4, "rest speed {speed}");
    b3_destroy_world(world);
}

#[test]
fn friction_slows_sliding_box() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let run = |friction: f32| {
        let world = b3_create_world(gpu.clone(), &b3_default_world_def());
        create_ground(world, 40.0);
        let mut def = b3_default_body_def();
        def.body_type = BodyType::Dynamic;
        def.position = [0.0, 0.5, 0.0];
        def.linear_velocity = [4.0, 0.0, 0.0];
        let body = b3_create_body(world, &def);
        let mut shape = b3_default_shape_def();
        shape.friction = friction;
        b3_create_hull_shape(body, &shape, &b3_make_box_hull(0.5, 0.5, 0.5));
        b3_world_ensure_gpu(world);
        for _ in 0..120 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
        }
        b3_world_gpu_wait(world);
        let vx = crate::api::b3_body_get_linear_velocity(body)[0];
        b3_destroy_world(world);
        vx
    };
    let with_f = run(0.6);
    let no_f = run(0.0);
    assert!(
        with_f < no_f * 0.85 || with_f.abs() < 0.5,
        "friction {with_f} vs {no_f}"
    );
}

#[test]
fn pause_draws_match_continuous_steps() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let a = b3_create_world(gpu.clone(), &def);
    let b = b3_create_world(gpu, &def);
    create_ground(a, 20.0);
    create_ground(b, 20.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 2.0, 0.0];
    let hull = b3_make_box_hull(0.5, 0.5, 0.5);
    let ba = b3_create_body(a, &body_def);
    let bb = b3_create_body(b, &body_def);
    b3_create_hull_shape(ba, &b3_default_shape_def(), &hull);
    b3_create_hull_shape(bb, &b3_default_shape_def(), &hull);
    b3_world_ensure_gpu(a);
    b3_world_ensure_gpu(b);
    for _ in 0..40 {
        b3_world_step_gpu(a, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(a);
    for _ in 0..40 {
        b3_world_step_gpu(b, 1.0 / 60.0, 4);
        b3_world_gpu_wait(b);
        b3_world_prepare_pose_snapshot(b);
        let _ = b3_body_get_position(bb);
        let _ = b3_body_get_rotation(bb);
    }
    let pa = b3_body_get_position(ba);
    let pb = b3_body_get_position(bb);
    let err = (pa[0] - pb[0]).abs() + (pa[1] - pb[1]).abs() + (pa[2] - pb[2]).abs();
    assert!(err < 1e-4, "pause vs continuous {pa:?} vs {pb:?}");
    b3_destroy_world(a);
    b3_destroy_world(b);
}

#[test]
fn destroy_and_reuse_slots() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 20.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    let hull = b3_make_box_hull(0.4, 0.4, 0.4);
    let mut live = Vec::new();
    for i in 0..8 {
        def.position = [i as f32, 1.0, 0.0];
        let id = b3_create_body(world, &def);
        b3_create_hull_shape(id, &b3_default_shape_def(), &hull);
        live.push(id);
    }
    b3_world_ensure_gpu(world);
    for _ in 0..10 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    for id in live.drain(0..4) {
        crate::api::b3_destroy_body(id);
    }
    for i in 0..4 {
        def.position = [i as f32 * 0.5, 3.0, 1.0];
        let id = b3_create_body(world, &def);
        b3_create_hull_shape(id, &b3_default_shape_def(), &hull);
        live.push(id);
    }
    for _ in 0..30 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    assert!(contacts.iter().any(|c| c.count > 0 && c.a != u32::MAX));
    b3_destroy_world(world);
}

#[test]
fn pose_snapshot_grows_when_bodies_are_added() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 20.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 1.0, 0.0];
    let hull = b3_make_box_hull(0.4, 0.4, 0.4);
    let first = b3_create_body(world, &def);
    b3_create_hull_shape(first, &b3_default_shape_def(), &hull);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let _ = b3_body_get_position(first);
    for i in 0..4 {
        def.position = [i as f32 + 1.0, 1.0, 0.0];
        let id = b3_create_body(world, &def);
        b3_create_hull_shape(id, &b3_default_shape_def(), &hull);
    }
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let pos = b3_body_get_position(first);
    assert!(pos[1].is_finite(), "grown pose snapshot panicked or returned NaN");
    b3_destroy_world(world);
}

#[test]
fn pose_snapshot_keeps_identity_with_unfilled_holes() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    create_ground(world, 40.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let hull = b3_make_box_hull(0.4, 0.4, 0.4);
    let mut ids = Vec::new();
    for x in [0.0f32, 10.0, 20.0] {
        body_def.position = [x, 1.0, 0.0];
        let id = b3_create_body(world, &body_def);
        b3_create_hull_shape(id, &b3_default_shape_def(), &hull);
        ids.push(id);
    }
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let before: Vec<[f32; 3]> = ids.iter().map(|id| b3_body_get_position(*id)).collect();
    assert!((before[2][0] - 20.0).abs() < 0.5, "setup last body {:?}", before[2]);
    crate::api::b3_destroy_body(ids[1]);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let last = b3_body_get_position(ids[2]);
    let first = b3_body_get_position(ids[0]);
    assert!(
        (last[0] - 20.0).abs() < 1.0,
        "hole remapped last body to {last:?} (first {first:?})"
    );
    assert!((first[0] - 0.0).abs() < 1.0, "first body drifted to {first:?}");
    let v = crate::api::b3_body_get_linear_velocity(ids[2]);
    assert!(v.iter().all(|c| c.is_finite()), "last velocity {v:?}");
    b3_destroy_world(world);
}

#[test]
fn pose_snapshot_keeps_identity_with_first_slot_hole() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    create_ground(world, 40.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let hull = b3_make_box_hull(0.4, 0.4, 0.4);
    let mut ids = Vec::new();
    for x in [0.0f32, 8.0, 16.0] {
        body_def.position = [x, 1.0, 0.0];
        let id = b3_create_body(world, &body_def);
        b3_create_hull_shape(id, &b3_default_shape_def(), &hull);
        ids.push(id);
    }
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    crate::api::b3_destroy_body(ids[0]);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let mid = b3_body_get_position(ids[1]);
    let last = b3_body_get_position(ids[2]);
    assert!((mid[0] - 8.0).abs() < 1.0, "mid after first-hole {mid:?}");
    assert!((last[0] - 16.0).abs() < 1.0, "last after first-hole {last:?}");
    b3_destroy_world(world);
}

#[test]
fn pose_snapshot_keeps_identity_with_multiple_holes() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    create_ground(world, 50.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let hull = b3_make_box_hull(0.4, 0.4, 0.4);
    let mut ids = Vec::new();
    for x in [0.0f32, 6.0, 12.0, 18.0, 24.0] {
        body_def.position = [x, 1.0, 0.0];
        let id = b3_create_body(world, &body_def);
        b3_create_hull_shape(id, &b3_default_shape_def(), &hull);
        ids.push(id);
    }
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    crate::api::b3_destroy_body(ids[0]);
    crate::api::b3_destroy_body(ids[2]);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let last = b3_body_get_position(ids[4]);
    let mid = b3_body_get_position(ids[3]);
    assert!((last[0] - 24.0).abs() < 1.0, "multi-hole last {last:?}");
    assert!((mid[0] - 18.0).abs() < 1.0, "multi-hole mid {mid:?}");
    b3_destroy_world(world);
}

#[test]
fn pose_snapshot_compound_survives_neighbor_hole() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    create_ground(world, 40.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let hull = b3_make_box_hull(0.3, 0.3, 0.3);
    body_def.position = [0.0, 1.0, 0.0];
    let a = b3_create_body(world, &body_def);
    b3_create_hull_shape(a, &b3_default_shape_def(), &hull);
    body_def.position = [15.0, 1.2, 0.0];
    let b = b3_create_body(world, &body_def);
    b3_create_hull_shape(b, &b3_default_shape_def(), &hull);
    b3_create_hull_shape(b, &b3_default_shape_def(), &b3_make_box_hull(0.2, 0.6, 0.2));
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    crate::api::b3_destroy_body(a);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let pos = b3_body_get_position(b);
    assert!((pos[0] - 15.0).abs() < 1.5, "compound after hole {pos:?}");
    b3_destroy_world(world);
}

#[test]
fn pose_snapshot_reuse_does_not_apply_stale_occupant() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    create_ground(world, 40.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let hull = b3_make_box_hull(0.4, 0.4, 0.4);
    body_def.position = [0.0, 1.0, 0.0];
    let a = b3_create_body(world, &body_def);
    b3_create_hull_shape(a, &b3_default_shape_def(), &hull);
    body_def.position = [12.0, 1.0, 0.0];
    let b = b3_create_body(world, &body_def);
    b3_create_hull_shape(b, &b3_default_shape_def(), &hull);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let _ = b3_body_get_position(a);
    crate::api::b3_destroy_body(a);
    body_def.position = [30.0, 4.0, 0.0];
    let c = b3_create_body(world, &body_def);
    b3_create_hull_shape(c, &b3_default_shape_def(), &hull);
    let pos = b3_body_get_position(c);
    assert!(
        (pos[0] - 30.0).abs() < 0.1 && (pos[1] - 4.0).abs() < 0.1,
        "reuse applied stale snapshot {pos:?}"
    );
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let after = b3_body_get_position(c);
    assert!(
        (after[0] - 30.0).abs() < 2.0,
        "reused slot lost identity after step {after:?}"
    );
    b3_destroy_world(world);
}

#[test]
fn pause_draws_match_continuous_with_contacts_and_holes() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let a = b3_create_world(gpu.clone(), &def);
    let b = b3_create_world(gpu, &def);
    create_ground(a, 20.0);
    create_ground(b, 20.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let hull = b3_make_box_hull(0.5, 0.5, 0.5);
    let mut ids_a = Vec::new();
    let mut ids_b = Vec::new();
    for i in 0..3 {
        body_def.position = [0.0, 0.5 + i as f32 * 1.05, 0.0];
        let ba = b3_create_body(a, &body_def);
        let bb = b3_create_body(b, &body_def);
        b3_create_hull_shape(ba, &b3_default_shape_def(), &hull);
        b3_create_hull_shape(bb, &b3_default_shape_def(), &hull);
        ids_a.push(ba);
        ids_b.push(bb);
    }
    b3_world_ensure_gpu(a);
    b3_world_ensure_gpu(b);
    for _ in 0..20 {
        b3_world_step_gpu(a, 1.0 / 60.0, 4);
        b3_world_step_gpu(b, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(a);
    b3_world_gpu_wait(b);
    crate::api::b3_destroy_body(ids_a[1]);
    crate::api::b3_destroy_body(ids_b[1]);
    for _ in 0..20 {
        b3_world_step_gpu(a, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(a);
    for _ in 0..20 {
        b3_world_step_gpu(b, 1.0 / 60.0, 4);
        b3_world_gpu_wait(b);
        b3_world_prepare_pose_snapshot(b);
        let _ = b3_body_get_position(ids_b[0]);
        let _ = b3_body_get_position(ids_b[2]);
    }
    b3_world_prepare_pose_snapshot(a);
    let pa = b3_body_get_position(ids_a[2]);
    let pb = b3_body_get_position(ids_b[2]);
    let err = (pa[0] - pb[0]).abs() + (pa[1] - pb[1]).abs() + (pa[2] - pb[2]).abs();
    assert!(err < 5e-3, "pause+hole vs continuous {pa:?} vs {pb:?}");
    b3_destroy_world(a);
    b3_destroy_world(b);
}

#[test]
fn duration_ms_from_ns_is_exact_at_long_uptime() {
    let ms = |a: u64, b: u64| (b.saturating_sub(a) as f64 * 1e-6) as f32;
    let long = 139_000_000_000_000u64;
    assert!((ms(long, long + 250_000) - 0.25).abs() < 1e-4);
    assert!((ms(1_000, 2_000) - 0.001).abs() < 1e-6);
    let bad = (long as f32 + 0.25) - (long as f32);
    assert!(bad < 0.01, "float32 uptime subtraction must not be used");
}

#[test]
fn fused_islands_remain_disabled() {
    assert!(!crate::types::ENABLE_FUSED_ISLANDS);
}

#[test]
fn restitution_drop_rebounds() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let run = |restitution: f32| {
        let mut def = b3_default_world_def();
        def.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &def);
        create_ground(world, 20.0);
        let mut body_def = b3_default_body_def();
        body_def.body_type = BodyType::Dynamic;
        body_def.position = [0.0, 4.0, 0.0];
        let body = b3_create_body(world, &body_def);
        let mut shape = b3_default_shape_def();
        shape.restitution = restitution;
        b3_create_hull_shape(body, &shape, &b3_make_box_hull(0.5, 0.5, 0.5));
        b3_world_ensure_gpu(world);
        let mut peak_vy = 0.0f32;
        for _ in 0..90 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            b3_world_gpu_wait(world);
            let vy = crate::api::b3_body_get_linear_velocity(body)[1];
            peak_vy = peak_vy.max(vy);
        }
        b3_destroy_world(world);
        peak_vy
    };
    let bounce = run(0.8);
    let inelastic = run(0.0);
    assert!(
        bounce > inelastic + 0.4,
        "restitution peak vy {bounce} vs {inelastic}"
    );
}

#[test]
fn pause_draws_then_impulse_then_step() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    create_ground(world, 20.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &body_def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    for _ in 0..30 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    for _ in 0..20 {
        b3_world_prepare_pose_snapshot(world);
        let _ = b3_body_get_position(body);
    }
    crate::api::b3_body_set_linear_velocity(body, [4.0, 0.0, 0.0]);
    let vx = crate::api::b3_body_get_linear_velocity(body)[0];
    assert!(vx > 3.0, "velocity setter lost after paused draws: {vx}");
    let before = b3_body_get_position(body);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    let after = b3_body_get_position(body);
    assert!(
        (after[0] - before[0]).abs() > 1e-4,
        "velocity after paused draws did not move body {before:?} -> {after:?}"
    );
    b3_destroy_world(world);
}

#[test]
fn multiple_submits_consume_one_snapshot() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 4.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 1.0, 0.0];
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let maps = crate::api::b3_world_pose_snapshot_maps(world);
    for _ in 0..16 {
        let _ = b3_body_get_position(body);
    }
    assert_eq!(crate::api::b3_world_pose_snapshot_maps(world), maps);
    b3_destroy_world(world);
}

include!("fixtures/full_mass_lifecycle.rs");

#[test]
fn capsule_full_mass_survives_edits_and_compounds() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let mut bd = b3_default_body_def(); bd.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &bd);
    let mut sd = b3_default_shape_def(); sd.density = 731.0;
    let mut cap = Capsule { center1: [0.023719,0.006008,-0.039068], center2: [-0.064492,-0.004664,-0.424718], radius: 0.09 };
    let shape = b3_create_capsule_shape(body, &sd, &cap);
    let check = |case: usize| {
        let d = b3_body_get_mass_data(body);
        let mut values = vec![d.mass]; values.extend(d.center); values.extend(d.inertia.into_iter().flatten());
        assert_eq!(values.iter().map(|v|v.to_bits()).collect::<Vec<_>>(), FULL_MASS_LIFECYCLE[case].map(f32::to_bits), "native full mass case {case}");
    };
    check(0);
    b3_world_ensure_gpu(world);
    crate::api::b3_shape_set_density(shape, 1000.0, true); check(1);
    b3_world_ensure_gpu(world);
    cap.center2 = [0.21,0.31,0.12];
    crate::api::b3_shape_set_capsule(shape, &cap);
    b3_body_apply_mass_from_shapes(body); check(2);
    let second = b3_create_capsule_shape(body, &sd, &cap); check(3);
    b3_destroy_shape(second, true); check(4);
    let explicit = b3_body_get_mass_data(body);
    b3_body_set_mass_data(body, explicit);
    check(4);
    b3_destroy_world(world);
}

#[test]
fn capsule_mass_data_matches_compute_helper() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &def);
    let mut shape = b3_default_shape_def();
    shape.density = 1.0;
    b3_create_capsule_shape(
        body,
        &shape,
        &Capsule {
            center1: [0.0, -1.0, 0.0],
            center2: [0.0, 1.0, 0.0],
            radius: 0.5,
        },
    );
    let mass = b3_body_get_mass_data(body);
    assert!((mass.mass - 2.0943951).abs() < 1e-5);
    assert!((mass.inertia[1][1] - 0.24870942).abs() < 1e-5);
    assert!((mass.inertia[0][0] - 1.39408174).abs() < 1e-4);
    b3_destroy_world(world);
}

#[test]
fn capsule_body_center_matches_native_inverse_mass_scaling() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &def);
    b3_create_capsule_shape(body, &b3_default_shape_def(), &Capsule {
        center1: [0.023719,0.006008,-0.039068],
        center2: [-0.064492,-0.004664,-0.424718], radius: 0.09,
    });
    let mass = b3_body_get_mass_data(body);
    // Independent CPU Falling Ragdolls left thigh, before the first step.
    assert_eq!(mass.mass.to_bits(), 13.1243343_f32.to_bits());
    for (actual, expected) in mass.center.into_iter().zip([-0.0203865003_f32,0.000671999936,-0.231893003]) {
        assert_eq!(actual.to_bits(), expected.to_bits(), "native body center");
    }
    b3_destroy_world(world);
}

#[test]
fn capsule_compound_inertia_applies_parallel_axis_once() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &def);
    let mut shape = b3_default_shape_def();
    shape.density = 1.0;
    let cap = Capsule {
        center1: [0.0, 0.0, 0.0],
        center2: [0.0, 0.0, 0.0],
        radius: 0.5,
    };
    b3_create_capsule_shape(
        body,
        &shape,
        &Capsule {
            center1: [0.0, 1.0, 0.0],
            center2: [0.0, 1.0, 0.0],
            radius: 0.5,
        },
    );
    b3_create_capsule_shape(
        body,
        &shape,
        &Capsule {
            center1: [0.0, -1.0, 0.0],
            center2: [0.0, -1.0, 0.0],
            radius: 0.5,
        },
    );
    let one = crate::types::compute_capsule_mass(cap.center1, cap.center2, 0.5, 1.0);
    let mass = b3_body_get_mass_data(body);
    assert!((mass.mass - 2.0 * one.mass).abs() < 1e-4);
    let ixx = 2.0 * (one.inertia[0] + one.mass * 1.0);
    let iyy = 2.0 * one.inertia[1];
    assert!((mass.inertia[0][0] - ixx).abs() < 1e-3, "ixx {}", mass.inertia[0][0]);
    assert!((mass.inertia[1][1] - iyy).abs() < 1e-3, "iyy {}", mass.inertia[1][1]);
    b3_destroy_world(world);
}

#[test]
fn capsule_angular_impulse_uses_axial_inertia() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wdef = b3_default_world_def();
    wdef.enable_sleep = false;
    let world = b3_create_world(gpu, &wdef);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.gravity_scale = 0.0;
    let body = b3_create_body(world, &def);
    let mut shape = b3_default_shape_def();
    shape.density = 1.0;
    b3_create_capsule_shape(
        body,
        &shape,
        &Capsule {
            center1: [0.0, -1.0, 0.0],
            center2: [0.0, 1.0, 0.0],
            radius: 0.5,
        },
    );
    b3_body_apply_angular_impulse(body, [0.0, 0.24870942, 0.0], true);
    b3_world_ensure_gpu(world);
    b3_world_gpu_wait(world);
    let w = b3_body_get_angular_velocity(body);
    assert!(
        (w[1] - 1.0).abs() < 0.05,
        "axial impulse should yield ~1 rad/s, got {w:?}"
    );
    b3_destroy_world(world);
}

#[test]
fn high_resistance_creation_order_and_pause_do_not_change_rest() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let a = b3_create_world(gpu.clone(), &def);
    let b = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(a, false);
    b3_world_enable_sleeping(b, false);
    create_high_resistance(a);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.rotation = b3_make_quat_from_axis_angle([0.0, 0.0, 1.0], 30.0 * B3_DEG_TO_RAD);
    let capsule = Capsule {
        center1: [0.0, -1.0, 0.0],
        center2: [0.0, 1.0, 0.0],
        radius: 0.5,
    };
    let mut tracked = None;
    for index in 0..10 {
        body_def.position = [-22.0 + 5.0 * index as f32, 1.5, 0.0];
        let body = b3_create_body(b, &body_def);
        if index == 0 {
            tracked = Some(body);
        }
        let mut shape_def = b3_default_shape_def();
        shape_def.rolling_resistance = 0.2 * index as f32;
        b3_create_capsule_shape(body, &shape_def, &capsule);
    }
    create_ground(b, 50.0);
    b3_world_ensure_gpu(a);
    b3_world_ensure_gpu(b);
    let tracked = tracked.expect("capsule 0");
    for _ in 0..400 {
        b3_world_step_gpu(a, 1.0 / 60.0, 4);
        b3_world_step_gpu(b, 1.0 / 60.0, 4);
        b3_world_gpu_wait(b);
        b3_world_prepare_pose_snapshot(b);
        let _ = b3_body_get_position(tracked);
    }
    b3_world_gpu_wait(a);
    let pa = pollster::block_on(crate::api::b3_world_sync_from_gpu(a));
    let pb = pollster::block_on(crate::api::b3_world_sync_from_gpu(b));
    assert!(crate::dump::high_resistance_quality_error(&pa, 400).is_none());
    let mut dyn_a: Vec<_> = pa.iter().filter(|b| b.inv_mass > 0.0).collect();
    let mut dyn_b: Vec<_> = pb.iter().filter(|b| b.inv_mass > 0.0).collect();
    dyn_a.sort_by(|x, y| x.pos[0].partial_cmp(&y.pos[0]).unwrap());
    dyn_b.sort_by(|x, y| x.pos[0].partial_cmp(&y.pos[0]).unwrap());
    for (x, y) in dyn_a.iter().zip(dyn_b.iter()) {
        let err = (x.pos[1] - y.pos[1]).abs();
        assert!(err < 0.05, "creation-order y {} vs {}", x.pos[1], y.pos[1]);
    }
    b3_destroy_world(a);
    b3_destroy_world(b);
}

#[test]
fn empty_capsule_pairs_keep_populated_reference_order() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    for (kind_a, kind_b, expected) in [
        (BodyType::Dynamic, BodyType::Dynamic, (1, 0)),
        (BodyType::Static, BodyType::Dynamic, (0, 1)),
        (BodyType::Dynamic, BodyType::Static, (1, 0)),
        (BodyType::Kinematic, BodyType::Dynamic, (0, 1)),
    ] {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0; 3];
        wd.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &wd);
        let mut bodies = Vec::new();
        for (index, kind) in [kind_a, kind_b].into_iter().enumerate() {
            let mut bd = b3_default_body_def();
            bd.body_type = kind;
            bd.position = [index as f32 * 0.43, 0.0, 0.0];
            bd.enable_sleep = false;
            let body = b3_create_body(world, &bd);
            b3_create_capsule_shape(body, &b3_default_shape_def(), &Capsule {
                center1: [0.0, -0.5, 0.0], center2: [0.0, 0.5, 0.0], radius: 0.2,
            });
            bodies.push(body);
        }
        for step in 0..2 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            let contacts = pollster::block_on(b3_world_sync_contacts(world));
            let ghost = contacts.iter().find(|c| c.a != u32::MAX).expect("broadphase capsule pair");
            assert_eq!(ghost.count, 0, "separated capsules, step {step}");
            assert_eq!((ghost.a, ghost.b), expected, "empty pair, step {step}");
            if step == 1 { assert_ne!(ghost.lifecycle[1] & 4, 0, "empty pair should recycle"); }
        }
        b3_body_set_transform(bodies[1], [0.39, 0.0, 0.0], [0.0, 0.0, 0.0, 1.0]);
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
        let contacts = pollster::block_on(b3_world_sync_contacts(world));
        let contact = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0).expect("touching capsule pair");
        assert_eq!((contact.a, contact.b), expected, "populated pair changed reference body");
        b3_destroy_world(world);
    }
}

#[test]
fn recycled_contact_separation_matches_fresh_at_rest() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    create_ground(world, 20.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 1.0, 0.0];
    let body = b3_create_body(world, &body_def);
    b3_create_capsule_shape(
        body,
        &b3_default_shape_def(),
        &Capsule {
            center1: [0.0, -0.5, 0.0],
            center2: [0.0, 1.0, 0.0],
            radius: 0.5,
        },
    );
    b3_world_ensure_gpu(world);
    for _ in 0..180 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    let contacts = pollster::block_on(crate::api::b3_world_sync_contacts(world));
    let sep0 = contacts
        .iter()
        .filter(|c| c.count > 0 && c.a != u32::MAX)
        .map(|c| c.ra0[3])
        .fold(0.0f32, |a, s| a.max(s.abs()));
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    let contacts = pollster::block_on(crate::api::b3_world_sync_contacts(world));
    let sep1 = contacts
        .iter()
        .filter(|c| c.count > 0 && c.a != u32::MAX)
        .map(|c| c.ra0[3])
        .fold(0.0f32, |a, s| a.max(s.abs()));
    assert!(
        (sep1 - sep0).abs() < 5e-3,
        "zero-motion recycle jumped separation {sep0} -> {sep1}"
    );
    b3_destroy_world(world);
}

#[test]
fn mixed_stacks_stay_in_two_layers() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    crate::scenes::create_mixed_stacks(world, 600);
    b3_world_ensure_gpu(world);
    for _ in 0..120 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    let bodies = pollster::block_on(crate::api::b3_world_sync_from_gpu(world));
    let err = crate::dump::mixed_stacks_quality_error(&bodies, 120);
    assert!(err.is_none(), "{}", err.unwrap_or_default());
    let stats = pollster::block_on(b3_world_live_step_stats(world)).expect("stats");
    assert!(!stats.capacity_loss(), "{}", stats.sticky.loss_detail());
    b3_destroy_world(world);
}

#[test]
fn graph_body_id_is_not_truncated_to_15_bits() {
    let body = 32768u32;
    let packed = 1u32 | (body << 2);
    assert_eq!(packed >> 2, body);
    assert_ne!((packed >> 2) & 0x7fff, body);
}

#[test]
fn body_slots_cross_old_16bit_boundary() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    for i in 0..65_538 {
        let id = b3_create_body(world, &b3_default_body_def());
        assert_ne!(id.index1, 0, "slot {i} refused early");
    }
    let next = b3_create_body(world, &b3_default_body_def());
    assert_eq!(next.index1, 65_539);
    b3_destroy_world(world);
}

#[test]
fn sticky_capacity_loss_survives_later_clean_steps() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 10.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_enable_sleeping(world, false);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_set_diagnostic_flags(world, DIAG_FORCE_CAPACITY_LOSS);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let fail_step = b3_world_physics_step(world);
    b3_world_set_diagnostic_flags(world, 0);
    for _ in 0..3 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    let failure = b3_world_gpu_fail(world);
    assert!(!failure.is_null());
    assert!(unsafe { std::ffi::CStr::from_ptr(failure) }.to_str().unwrap().contains("contact_reasons=0x40"), "async status must retain contact cause bits");
    let stats = pollster::block_on(b3_world_live_step_stats(world)).expect("stats");
    assert!(stats.capacity_loss(), "sticky loss must remain after clean steps");
    assert_eq!(stats.first_fail_step, fail_step as u32);
    assert!(stats.sticky.pair_candidates_dropped > 0);
    assert!(stats.sticky.cell_inserts_dropped > 0);
    assert!(stats.sticky.contact_pairs_dropped > 0);
    assert_ne!(stats.contact_drop_reasons & (1 << 6), 0, "forced-loss cause must survive status harvest");
    assert!(stats.sticky.hash_traversals_dropped > 0);
    b3_world_clear_capacity_status(world);
    b3_destroy_world(world);
}

#[test]
fn physics_step_id_advances_when_timestamp_ring_is_full() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 10.0);
    b3_world_ensure_gpu(world);
    for _ in 0..5 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    assert_eq!(b3_world_physics_step(world), 5);
    b3_world_step_gpu(world, 0.0, 4);
    assert_eq!(b3_world_physics_step(world), 5, "dt=0 must not advance the world step");
    b3_world_gpu_wait(world);
    let ts = crate::api::b3_world_last_timestamp_step(world);
    assert!(ts <= 5 && ts > 0, "timestamp sample {ts} must be a submitted physics step");
    b3_destroy_world(world);
}

#[test]
fn high_resistance_uses_one_group_wave_only() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    // Test one-workgroup color waves, not the alternative component solver.
    crate::api::set_component_tgs_requested_for_test(world,false);
    create_high_resistance(world);
    b3_world_enable_sleeping(world, false);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert_eq!(b3_world_last_solver_dispatches(world), 13);
    assert_eq!(b3_world_last_static_sort_dispatches(world), 0);
    b3_world_set_diagnostic_flags(world, DIAG_GENERAL_SOLVER);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert_eq!(b3_world_last_solver_dispatches(world), 325);
    b3_world_set_diagnostic_flags(world, 0);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert_eq!(b3_world_last_solver_dispatches(world), 13);
    let mut extra = b3_default_body_def();
    extra.body_type = BodyType::Dynamic;
    extra.position = [20.0, 1.5, 0.0];
    let body = b3_create_body(world, &extra);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert_eq!(
        b3_world_last_solver_dispatches(world),
        325,
        "worlds past the pair bound keep the 24-color encode plus fused prefix"
    );
    b3_destroy_world(world);
}

#[test]
fn gpu_schedule_uses_body_slot_32768() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.capacity.static_body_count = 40000;
    def.capacity.dynamic_body_count = 8;
    def.capacity.static_shape_count = 8;
    def.capacity.dynamic_shape_count = 8;
    let world = b3_create_world(gpu, &def);
    create_ground(world, 20.0);
    let mut body_def = b3_default_body_def();
    for _ in 1..32768 {
        let id = b3_create_body(world, &body_def);
        assert_ne!(id.index1, 0);
    }
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 0.5, 0.0];
    let cube = b3_create_body(world, &body_def);
    assert_eq!(cube.index1, 32769);
    b3_create_hull_shape(cube, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_enable_sleeping(world, false);
    b3_world_ensure_gpu(world);
    for _ in 0..8 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    let pos = b3_body_get_position(cube);
    assert!(pos[1] > 0.4 && pos[1] < 0.7, "slot 32768 cube y={}", pos[1]);
    let cons = pollster::block_on(b3_world_sync_contacts(world));
    let hit = cons.iter().any(|c| c.a == 32768 || c.b == 32768 || c.a == 32768 - 1 || c.b == 32768 - 1);
    assert!(hit, "expected a contact involving GPU body slot 32768");
    b3_destroy_world(world);
}

#[test]
fn high_resistance_sleeper_wakes_on_velocity() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    // Reuse a world slot so the fixture cannot accidentally rely on generation 1.
    let prior = b3_create_world(gpu.clone(), &b3_default_world_def());
    create_high_resistance(prior);
    let stale = crate::api::b3_world_dynamic_body_ids(prior).into_iter()
        .find(|body| body.index1 == 7).expect("prior capsule");
    b3_destroy_world(prior);
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_high_resistance(world);
    let sleeper = crate::api::b3_world_dynamic_body_ids(world).into_iter()
        .find(|body| body.index1 == 7).expect("live sleeper capsule");
    assert_eq!(world.index1, prior.index1, "exercise world-slot reuse");
    assert!(sleeper.generation > stale.generation, "reused child epoch must advance");
    assert!(crate::api::b3_body_is_valid(sleeper), "fixture needs a valid live body");
    assert!(!crate::api::b3_body_is_valid(stale), "retired capsule must remain invalid");
    b3_world_enable_sleeping(world, true);
    b3_world_ensure_gpu(world);
    for _ in 0..400 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    assert!(crate::api::b3_body_is_valid(sleeper), "sleeper must still be valid after settling");
    assert!(!b3_body_is_awake(sleeper), "fixture must be asleep before testing wakeup");
    let settled_velocity = b3_body_get_linear_velocity(sleeper);
    b3_body_set_linear_velocity(stale, [0.0, 4.0, 0.0]);
    assert_eq!(b3_body_get_linear_velocity(sleeper), settled_velocity, "stale setter must not change live velocity");
    assert!(!b3_body_is_awake(sleeper), "stale setter must not wake live capsule");
    eprintln!("sleeper fixture: stale={:?} live={:?} valid=true asleep=true", stale, sleeper);
    b3_body_set_linear_velocity(sleeper, [0.0, 4.0, 0.0]);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait_with_mirror(world);
    assert!(b3_body_is_awake(sleeper), "impulse must wake a sleeping capsule");
    b3_destroy_world(world);
}

fn overlapping_compound_world(gpu: GpuDevice) -> (crate::api::WorldId, [crate::api::BodyId; 3]) {
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    let mut ground_def = b3_default_body_def();
    ground_def.position = [0.0, -1.0, 0.0];
    let ground = b3_create_body(world, &ground_def);
    b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(20.0, 1.0, 20.0));
    let cube = b3_make_box_hull(0.5, 0.5, 0.5);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let mut ids = [crate::api::b3_null_body_id(); 3];
    for (i, x) in [0.0, 3.0, 6.0].iter().enumerate() {
        body_def.position = [*x, 0.5, 0.0];
        ids[i] = b3_create_body(world, &body_def);
        b3_create_hull_shape(ids[i], &b3_default_shape_def(), &cube);
        b3_create_hull_shape(ids[i], &b3_default_shape_def(), &cube);
    }
    (world, ids)
}

fn assert_compounds_rest(world: crate::api::WorldId, ids: [crate::api::BodyId; 3], label: &str) {
    b3_world_gpu_wait_with_mirror(world);
    for id in ids {
        let p = b3_body_get_position(id);
        assert!(
            (p[1] - 0.5).abs() < 0.05,
            "{label} compound y={} expected ~0.5",
            p[1]
        );
        assert!(p[0].abs() < 8.0 && p[2].abs() < 1.0, "{label} xz drift {:?}", p);
    }
}

#[test]
fn overlapping_compound_children_stay_on_ground() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let (world, ids) = overlapping_compound_world(gpu);
    // Keep this general-radix regression independent of bounded-sort overrides.
    b3_world_set_diagnostic_flags(world, 0);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert!(
        b3_world_last_static_sort_dispatches(world) > 0,
        "compound degree>1 must encode the general static sort"
    );
    for _ in 0..119 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    assert_compounds_rest(world, ids, "default");
    b3_destroy_world(world);
}

#[test]
fn overlapping_compound_matches_force_general_static_sort() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let (world, ids) = overlapping_compound_world(gpu);
    b3_world_set_diagnostic_flags(world, DIAG_FORCE_GENERAL_STATIC_SORT);
    b3_world_ensure_gpu(world);
    for _ in 0..120 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    assert!(b3_world_last_static_sort_dispatches(world) > 0);
    assert_compounds_rest(world, ids, "force-general-static-sort");
    b3_destroy_world(world);
}

#[test]
fn offset_compound_children_stay_supported() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    let mut ground_def = b3_default_body_def();
    ground_def.position = [0.0, -1.0, 0.0];
    let ground = b3_create_body(world, &ground_def);
    b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(20.0, 1.0, 20.0));
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &body_def);
    let cube = b3_make_box_hull(0.5, 0.5, 0.5);
    b3_create_hull_shape(body, &b3_default_shape_def(), &cube);
    let mut offset = cube;
    offset.center = [1.1, 0.0, 0.0];
    b3_create_hull_shape(body, &b3_default_shape_def(), &offset);
    b3_world_ensure_gpu(world);
    for _ in 0..120 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let p = b3_body_get_position(body);
    assert!(p[1] > 0.4 && p[1] < 0.7, "offset compound fell y={}", p[1]);
    b3_destroy_world(world);
}

#[test]
fn kinematic_support_keeps_dynamic_on_platform() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    let mut kin_def = b3_default_body_def();
    kin_def.body_type = BodyType::Kinematic;
    kin_def.position = [0.0, -1.0, 0.0];
    let platform = b3_create_body(world, &kin_def);
    b3_create_hull_shape(platform, &b3_default_shape_def(), &b3_make_box_hull(4.0, 1.0, 4.0));
    let mut dyn_def = b3_default_body_def();
    dyn_def.body_type = BodyType::Dynamic;
    dyn_def.position = [0.0, 0.5, 0.0];
    let cube = b3_create_body(world, &dyn_def);
    b3_create_hull_shape(cube, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert!(
        b3_world_last_static_sort_dispatches(world) > 0
            || b3_world_last_solver_dispatches(world) > 0,
        "kinematic endpoints must not use the static_shapes<=1 shortcut"
    );
    for _ in 0..60 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let p = b3_body_get_position(cube);
    assert!(p[1] > 0.4 && p[1] < 0.7, "kinematic platform drop y={}", p[1]);
    b3_destroy_world(world);
}

#[test]
fn adding_second_child_after_degree_one_path_stays_supported() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    // This fixture checks restoration of the general static radix path.
    // Bounded-static-sort has its own coverage and may validly avoid that radix.
    b3_world_set_diagnostic_flags(world, 0);
    b3_world_enable_sleeping(world, false);
    create_ground(world, 20.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &body_def);
    let cube = b3_make_box_hull(0.5, 0.5, 0.5);
    b3_create_hull_shape(body, &b3_default_shape_def(), &cube);
    b3_world_ensure_gpu(world);
    for _ in 0..30 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    assert_eq!(b3_world_last_static_sort_dispatches(world), 0);
    b3_create_hull_shape(body, &b3_default_shape_def(), &cube);
    b3_world_ensure_gpu(world);
    for _ in 0..90 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    assert!(b3_world_last_static_sort_dispatches(world) > 0);
    b3_world_gpu_wait_with_mirror(world);
    let p = b3_body_get_position(body);
    assert!(p[1] > 0.4 && p[1] < 0.8, "grown compound y={}", p[1]);
    b3_destroy_world(world);
}

#[test]
fn revolute_keeps_one_group_contact_waves() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_revolute(world);
    b3_world_enable_sleeping(world, false);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    // Jointed worlds share graph colors. A small world executes each complete
    // wave in one workgroup; the diagnostic dispatches all 24 colors separately.
    assert_eq!(b3_world_last_solver_dispatches(world), 4 * 3 + 1);
    assert_eq!(b3_world_last_joint_dispatches(world), 4 * 3);
    b3_world_set_diagnostic_flags(world, DIAG_GENERAL_SOLVER);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert_eq!(b3_world_last_solver_dispatches(world), (4 * 3 + 1) * 24);
    assert_eq!(b3_world_last_joint_dispatches(world), 4 * 3 * 24);
    b3_destroy_world(world);
}

#[test]
fn dt_zero_is_a_full_physics_noop() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 10.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_enable_sleeping(world, false);
    b3_world_ensure_gpu(world);
    for _ in 0..10 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let pos = b3_body_get_position(body);
    let vel = b3_body_get_linear_velocity(body);
    let step = b3_world_physics_step(world);
    b3_world_step_gpu(world, 0.0, 4);
    b3_world_gpu_wait_with_mirror(world);
    assert_eq!(b3_world_physics_step(world), step);
    let pos1 = b3_body_get_position(body);
    let vel1 = b3_body_get_linear_velocity(body);
    assert_eq!(pos, pos1);
    assert_eq!(vel, vel1);
    b3_destroy_world(world);
}

unsafe extern "C" fn accept_all_pairs(_: ShapeId, _: ShapeId, _: *mut std::ffi::c_void) -> bool {
    true
}

#[test]
fn noop_custom_filter_matches_ordinary_stepping() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let a = b3_create_world(gpu.clone(), &def);
    let b = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(a, false);
    b3_world_enable_sleeping(b, false);
    create_ground(a, 10.0);
    create_ground(b, 10.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 0.5, 0.0];
    let cube = b3_make_box_hull(0.5, 0.5, 0.5);
    let ba = b3_create_body(a, &body_def);
    let bb = b3_create_body(b, &body_def);
    b3_create_hull_shape(ba, &b3_default_shape_def(), &cube);
    b3_create_hull_shape(bb, &b3_default_shape_def(), &cube);
    b3_world_set_custom_filter_callback(b, Some(accept_all_pairs), std::ptr::null_mut());
    for _ in 0..20 {
        b3_world_step_gpu(a, 1.0 / 60.0, 4);
        b3_world_step_gpu(b, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(a);
    b3_world_gpu_wait_with_mirror(b);
    let pa = b3_body_get_position(ba);
    let pb = b3_body_get_position(bb);
    assert!((pa[1] - pb[1]).abs() < 1e-4, "callback y {} vs {}", pa[1], pb[1]);
    assert_eq!(b3_world_physics_step(a), b3_world_physics_step(b));
    b3_destroy_world(a);
    b3_destroy_world(b);
}

#[test]
fn capacity_loss_stops_later_physics_submissions() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 10.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_enable_sleeping(world, false);
    b3_world_ensure_gpu(world);
    b3_world_set_diagnostic_flags(world, DIAG_FORCE_CAPACITY_LOSS);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    let fail_step = b3_world_physics_step(world);
    b3_world_set_diagnostic_flags(world, 0);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    assert!(b3_world_physics_invalid(world));
    assert_eq!(b3_world_physics_step(world), fail_step);
    b3_world_clear_capacity_status(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    assert!(
        b3_world_physics_invalid(world),
        "clearing sticky counters must not rehabilitate invalid physics"
    );
    b3_destroy_world(world);
}

#[test]
fn host_non_dynamic_matches_shader_ownership() {
    assert!(gpu_is_non_dynamic(FLAG_STATIC));
    assert!(gpu_is_non_dynamic(FLAG_KINEMATIC));
    assert!(gpu_is_non_dynamic(FLAG_DISABLED));
    assert!(!gpu_is_non_dynamic(0));
}

fn assert_live_contacts_scheduled_once(world: crate::api::WorldId) {
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let mut keys = std::collections::BTreeSet::new();
    let mut per_color: Vec<Vec<(u32, u32)>> = vec![Vec::new(); 24];
    let mut live = 0u32;
    for c in &contacts {
        if c.a == u32::MAX || c.b == u32::MAX || c.count == 0 {
            continue;
        }
        live += 1;
        let key = (c.a.min(c.b), c.a.max(c.b), c.pair_key());
        assert!(keys.insert(key), "duplicate scheduled contact {key:?}");
        if c.color < 24 {
            per_color[c.color as usize].push((c.a, c.b));
        }
    }
    for (col, list) in per_color.iter().enumerate() {
        if col == OVERFLOW_COLOR as usize {
            continue;
        }
        for (i, (a, b)) in list.iter().enumerate() {
            for (c, d) in list.iter().skip(i + 1) {
                let share_dyn = (*a != 0 && (*a == *c || a == d)) || (*b != 0 && (b == c || b == d));
                assert!(!share_dyn, "color {col} shares a writable body between {a}-{b} and {c}-{d}");
            }
        }
    }
    assert!(live >= 1, "expected at least one live contact");
}

#[test]
fn overlapping_compounds_schedule_each_contact_once() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let (world, ids) = overlapping_compound_world(gpu);
    b3_world_ensure_gpu(world);
    for _ in 0..30 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    assert_live_contacts_scheduled_once(world);
    assert_compounds_rest(world, ids, "schedule");
    b3_destroy_world(world);
}

#[test]
fn two_kinematic_supports_without_static() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    // Compare general-radix behavior independently of bounded-sort overrides.
    b3_world_set_diagnostic_flags(world, 0);
    b3_world_enable_sleeping(world, false);
    let mut kin = b3_default_body_def();
    kin.body_type = BodyType::Kinematic;
    kin.position = [-1.5, -1.0, 0.0];
    let a = b3_create_body(world, &kin);
    kin.position = [1.5, -1.0, 0.0];
    let b = b3_create_body(world, &kin);
    let slab = b3_make_box_hull(2.0, 1.0, 2.0);
    b3_create_hull_shape(a, &b3_default_shape_def(), &slab);
    b3_create_hull_shape(b, &b3_default_shape_def(), &slab);
    let mut dyn_def = b3_default_body_def();
    dyn_def.body_type = BodyType::Dynamic;
    dyn_def.position = [0.0, 0.5, 0.0];
    let cube = b3_create_body(world, &dyn_def);
    b3_create_hull_shape(cube, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert!(
        b3_world_last_static_sort_dispatches(world) > 0,
        "two kinematic supports make D_b*N > 1"
    );
    for _ in 0..60 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let p = b3_body_get_position(cube);
    assert!(p[1] > 0.4 && p[1] < 0.8, "two-kinematic y={}", p[1]);
    b3_destroy_world(world);
}

#[test]
fn zero_mass_dynamic_support_remains_dynamic_for_sort() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    let mut support_def = b3_default_body_def();
    support_def.body_type = BodyType::Dynamic;
    support_def.position = [0.0, -1.0, 0.0];
    let support = b3_create_body(world, &support_def);
    b3_create_hull_shape(support, &b3_default_shape_def(), &b3_make_box_hull(4.0, 1.0, 4.0));
    b3_body_set_mass_data(support, MassData {
        mass: 0.0,
        center: [0.0; 3],
        inertia: [[0.0; 3]; 3],
    });
    let mut dyn_def = b3_default_body_def();
    dyn_def.body_type = BodyType::Dynamic;
    dyn_def.position = [0.0, 0.5, 0.0];
    let cube = b3_create_body(world, &dyn_def);
    b3_create_hull_shape(cube, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    for _ in 0..40 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let p = b3_body_get_position(cube);
    assert!(p[1] > 0.35, "zero-mass support drop y={}", p[1]);
    b3_destroy_world(world);
}

#[test]
fn flat_mesh_does_not_create_pairs_above_native_fat_bounds() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.gravity = [0.0; 3];
    let world = b3_create_world(gpu, &def);
    let ground = b3_create_body(world, &b3_default_body_def());
    b3_create_mesh_shape(ground, &b3_default_shape_def(),
        &[[-1.0,0.0,-1.0], [1.0,0.0,-1.0], [1.0,0.0,1.0], [-1.0,0.0,1.0]],
        &[[0,2,1],[0,3,2]], &[], &[], &[], [1.0;3]);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0,0.14,0.0];
    let body = b3_create_body(world, &body_def);
    b3_create_sphere_shape(body, &b3_default_shape_def(), &Sphere { center:[0.0;3], radius:0.05 });
    b3_world_step_gpu(world, 1.0/60.0, 4);
    // Native flat terrain ends at y=.04 including speculative + static margin.
    // The sphere's fat lower bound is .06375: no broadphase pair should exist.
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    assert!(contacts.iter().all(|c| c.a == u32::MAX), "artificial mesh thickness created a pair");
    b3_body_set_transform(body, [0.0,0.10,0.0], [0.0,0.0,0.0,1.0]);
    b3_world_step_gpu(world, 1.0/60.0, 4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    assert!(contacts.iter().any(|c| c.a != u32::MAX), "actual fat-bound overlap must create a pair");
    b3_destroy_world(world);
}

#[test]
fn mesh_ground_does_not_skip_general_static_sort() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    let mut ground_def = b3_default_body_def();
    ground_def.position = [0.0, -1.0, 0.0];
    let ground = b3_create_body(world, &ground_def);
    let verts = [
        [-4.0, 0.0, -4.0],
        [4.0, 0.0, -4.0],
        [4.0, 0.0, 4.0],
        [-4.0, 0.0, 4.0],
    ];
    let tris = [[0u32, 1, 2], [0, 2, 3]];
    b3_create_mesh_shape(
        ground,
        &b3_default_shape_def(),
        &verts,
        &tris,
        &[],
        &[],
        &[] as &[MeshNode],
        [1.0, 1.0, 1.0],
    );
    let mut dyn_def = b3_default_body_def();
    dyn_def.body_type = BodyType::Dynamic;
    dyn_def.position = [0.0, 0.5, 0.0];
    let cube = b3_create_body(world, &dyn_def);
    b3_create_hull_shape(cube, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert!(
        b3_world_last_static_sort_dispatches(world) > 0,
        "mesh contacts are excluded from the degree-one skip proof"
    );
    b3_destroy_world(world);
}

#[test]
fn topology_recompute_matches_cached_compound_classification() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    // Compare general-radix behavior independently of bounded-sort overrides.
    b3_world_set_diagnostic_flags(world, 0);
    b3_world_enable_sleeping(world, false);
    create_ground(world, 20.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &body_def);
    let cube = b3_make_box_hull(0.5, 0.5, 0.5);
    b3_create_hull_shape(body, &b3_default_shape_def(), &cube);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert_eq!(b3_world_last_static_sort_dispatches(world), 0);
    b3_create_hull_shape(body, &b3_default_shape_def(), &cube);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let cached = b3_world_last_static_sort_dispatches(world);
    b3_world_set_diagnostic_flags(world, DIAG_RECOMPUTE_TOPOLOGY);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert_eq!(
        cached,
        b3_world_last_static_sort_dispatches(world),
        "cached topology must match the always-recompute oracle after a compound grow"
    );
    assert!(cached > 0);
    b3_destroy_world(world);
}

#[test]
fn rotated_compound_stays_supported() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    create_ground(world, 20.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 0.8, 0.0];
    body_def.rotation = b3_make_quat_from_axis_angle([0.0, 1.0, 0.0], 0.4);
    let body = b3_create_body(world, &body_def);
    let cube = b3_make_box_hull(0.5, 0.5, 0.5);
    b3_create_hull_shape(body, &b3_default_shape_def(), &cube);
    let mut offset = cube;
    offset.center = [0.0, 0.0, 1.1];
    b3_create_hull_shape(body, &b3_default_shape_def(), &offset);
    b3_world_ensure_gpu(world);
    for _ in 0..90 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let p = b3_body_get_position(body);
    assert!(p[1] > 0.3, "rotated compound y={}", p[1]);
    b3_destroy_world(world);
}

fn joint_scan_disables(joints: &[crate::types::JointGpu], a: u32, b: u32) -> bool {
    joints.iter().any(|j| {
        if j.kind == crate::types::JOINT_NONE {
            return false;
        }
        let pair = (j.a == a && j.b == b) || (j.a == b && j.b == a);
        pair && (j.flags & crate::types::JOINT_COLLIDE_CONNECTED == 0)
    })
}

fn joint_table_disables(table: &[u32], a: u32, b: u32) -> Option<bool> {
    let key_a = a.min(b);
    let key_b = a.max(b);
    let mut h = pair_hash_mix(pair_hash_mix(key_a) ^ key_b);
    let cap = crate::types::JOINT_FILTER_CAP;
    for _ in 0..crate::types::JOINT_FILTER_PROBE {
        let idx = (h & (cap - 1)) as usize;
        let stored = table[idx * 3];
        if stored == u32::MAX {
            return Some(false);
        }
        if stored == key_a && table[idx * 3 + 1] == key_b {
            return Some(table[idx * 3 + 2] != 0);
        }
        h = h.wrapping_add(1);
    }
    None
}

#[test]
fn joint_filter_table_matches_scan_oracle() {
    use crate::types::{
        build_joint_filter_table, JointGpu, JOINT_COLLIDE_CONNECTED, JOINT_FILTER, JOINT_REVOLUTE,
    };
    let mut joints = vec![JointGpu::zeroed(); 8];
    joints[0].a = 3;
    joints[0].b = 9;
    joints[0].kind = JOINT_REVOLUTE;
    joints[0].flags = 0;
    joints[1].a = 9;
    joints[1].b = 3;
    joints[1].kind = JOINT_FILTER;
    joints[1].flags = JOINT_COLLIDE_CONNECTED;
    joints[2].a = 4;
    joints[2].b = 5;
    joints[2].kind = JOINT_REVOLUTE;
    joints[2].flags = JOINT_COLLIDE_CONNECTED;
    joints[3].a = 100;
    joints[3].b = 200;
    joints[3].kind = JOINT_REVOLUTE;
    joints[3].flags = 0;
    let (table, overflow) = build_joint_filter_table(&joints);
    assert!(!overflow);
    for a in 0..16u32 {
        for b in 0..16u32 {
            let scan = joint_scan_disables(&joints, a, b);
            let look = joint_table_disables(&table, a, b).expect("probe overflow");
            assert_eq!(scan, look, "pair {a},{b}");
        }
    }
    assert_eq!(joint_table_disables(&table, 100, 200), Some(true));
    assert_eq!(joint_table_disables(&table, 4, 5), Some(false));
    joints[0].flags = JOINT_COLLIDE_CONNECTED;
    let (table, _) = build_joint_filter_table(&joints);
    assert_eq!(joint_table_disables(&table, 3, 9), Some(false), "enabled filter joint cannot veto");
}

fn pendulum_on_ground(
    world: crate::api::WorldId,
    ground: crate::api::BodyId,
    x: f32,
    swapped: bool,
) -> crate::api::BodyId {
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [x, 4.0, 0.0];
    let body = b3_create_body(world, &body_def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.25, 1.0, 0.25));
    let mut joint = b3_default_revolute_joint_def();
    if swapped {
        joint.body_a = body;
        joint.body_b = ground;
        joint.local_anchor_a = [0.0, 1.0, 0.0];
        joint.local_anchor_b = [x, 6.0, 0.0];
    } else {
        joint.body_a = ground;
        joint.body_b = body;
        joint.local_anchor_a = [x, 6.0, 0.0];
        joint.local_anchor_b = [0.0, 1.0, 0.0];
    }
    b3_create_revolute_joint(world, &joint);
    body
}

fn assert_exclusive_writable_owners(world: crate::api::WorldId) {
    let owners = crate::api::b3_world_joint_writable_owners(world);
    let mut seen = std::collections::HashMap::<u32, u32>::new();
    for (body, root) in owners {
        if let Some(prev) = seen.insert(body, root) {
            panic!("writable body {body} owned by {prev} and {root}");
        }
    }
}

fn poses_of(ids: &[crate::api::BodyId]) -> Vec<([f32; 3], [f32; 4])> {
    ids.iter()
        .map(|id| (b3_body_get_position(*id), b3_body_get_rotation(*id)))
        .collect()
}

#[test]
fn shared_static_ground_joints_are_exclusive_and_match_serial() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let run = |flags: u32| {
        let world = b3_create_world(gpu.clone(), &b3_default_world_def());
        b3_world_enable_sleeping(world, false);
        b3_world_set_diagnostic_flags(world, flags);
        let mut ground_def = b3_default_body_def();
        ground_def.position = [0.0, -1.0, 0.0];
        let ground = b3_create_body(world, &ground_def);
        b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(80.0, 1.0, 4.0));
        let mut ids = Vec::new();
        for i in 0..80 {
            ids.push(pendulum_on_ground(world, ground, -40.0 + i as f32, i % 2 == 0));
        }
        let mut kin_def = b3_default_body_def();
        kin_def.body_type = BodyType::Kinematic;
        kin_def.position = [0.0, 8.0, 2.0];
        let kinematic = b3_create_body(world, &kin_def);
        let mut dyn_def = b3_default_body_def();
        dyn_def.body_type = BodyType::Dynamic;
        dyn_def.position = [0.0, 6.0, 2.0];
        let hanging = b3_create_body(world, &dyn_def);
        b3_create_hull_shape(hanging, &b3_default_shape_def(), &b3_make_box_hull(0.25, 0.5, 0.25));
        let mut weld = b3_default_weld_joint_def();
        weld.body_a = kinematic;
        weld.body_b = hanging;
        weld.local_anchor_a = [0.0, -1.0, 0.0];
        weld.local_anchor_b = [0.0, 0.5, 0.0];
        b3_create_weld_joint(world, &weld);
        ids.push(hanging);

        let mut zero_def = b3_default_body_def();
        zero_def.body_type = BodyType::Dynamic;
        zero_def.position = [4.0, 5.0, 2.0];
        let zero = b3_create_body(world, &zero_def);
        b3_create_hull_shape(zero, &b3_default_shape_def(), &b3_make_box_hull(0.25, 0.5, 0.25));
        b3_body_set_mass_data(
            zero,
            MassData {
                mass: 0.0,
                center: [0.0; 3],
                inertia: [[0.0; 3]; 3],
            },
        );
        let mut slider_def = b3_default_body_def();
        slider_def.body_type = BodyType::Dynamic;
        slider_def.position = [4.0, 3.0, 2.0];
        let slider = b3_create_body(world, &slider_def);
        b3_create_hull_shape(slider, &b3_default_shape_def(), &b3_make_box_hull(0.25, 0.5, 0.25));
        let mut prism = b3_default_prismatic_joint_def();
        prism.body_a = zero;
        prism.body_b = slider;
        prism.local_anchor_a = [0.0, -1.0, 0.0];
        prism.local_anchor_b = [0.0, 0.5, 0.0];
        prism.local_axis_a = [0.0, 1.0, 0.0];
        b3_create_prismatic_joint(world, &prism);
        ids.push(slider);

        let mut wheel_def = b3_default_body_def();
        wheel_def.body_type = BodyType::Dynamic;
        wheel_def.position = [-4.0, 2.0, 2.0];
        let wheel_body = b3_create_body(world, &wheel_def);
        b3_create_hull_shape(
            wheel_body,
            &b3_default_shape_def(),
            &b3_make_box_hull(0.3, 0.3, 0.3),
        );
        let mut wheel = b3_default_wheel_joint_def();
        wheel.body_a = ground;
        wheel.body_b = wheel_body;
        wheel.local_anchor_a = [-4.0, 2.0, 2.0];
        wheel.local_anchor_b = [0.0, 0.0, 0.0];
        b3_create_wheel_joint(world, &wheel);
        ids.push(wheel_body);

        b3_world_ensure_gpu(world);
        for _ in 0..30 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
        }
        b3_world_gpu_wait_with_mirror(world);
        assert_exclusive_writable_owners(world);
        let ground_p = b3_body_get_position(ground);
        let kin_p = b3_body_get_position(kinematic);
        let poses = poses_of(&ids);
        b3_destroy_world(world);
        (ground_p, kin_p, poses)
    };
    let serial = run(DIAG_SERIAL_JOINTS);
    let parallel = run(DIAG_PARALLEL_JOINTS);
    assert!((serial.0[1] + 1.0).abs() < 1e-4, "static ground moved {:?}", serial.0);
    assert!((parallel.0[1] + 1.0).abs() < 1e-4, "static ground moved {:?}", parallel.0);
    assert!((serial.1[1] - 8.0).abs() < 1e-3, "kinematic moved {:?}", serial.1);
    assert!((parallel.1[1] - 8.0).abs() < 1e-3, "kinematic moved {:?}", parallel.1);
    assert_eq!(serial.2.len(), parallel.2.len());
    for (i, (a, b)) in serial.2.iter().zip(parallel.2.iter()).enumerate() {
        let dp = (a.0[0] - b.0[0]).abs() + (a.0[1] - b.0[1]).abs() + (a.0[2] - b.0[2]).abs();
        let dq = (a.1[0] - b.1[0]).abs()
            + (a.1[1] - b.1[1]).abs()
            + (a.1[2] - b.1[2]).abs()
            + (a.1[3] - b.1[3]).abs();
        assert!(dp < 0.05 && dq < 0.05, "joint {i} serial vs parallel pos {dp} quat {dq}");
    }
}

#[test]
fn joint_chain_stays_one_writable_component() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    b3_world_enable_sleeping(world, false);
    b3_world_set_diagnostic_flags(world, DIAG_PARALLEL_JOINTS);
    let mut ground_def = b3_default_body_def();
    ground_def.position = [0.0, -1.0, 0.0];
    let ground = b3_create_body(world, &ground_def);
    let mut prev = ground;
    let mut last = ground;
    for i in 0..24 {
        let mut def = b3_default_body_def();
        def.body_type = BodyType::Dynamic;
        def.position = [0.0, 0.5 + i as f32, 0.0];
        last = b3_create_body(world, &def);
        b3_create_hull_shape(last, &b3_default_shape_def(), &b3_make_box_hull(0.25, 0.4, 0.25));
        let mut joint = b3_default_revolute_joint_def();
        joint.body_a = prev;
        joint.body_b = last;
        joint.local_anchor_a = [0.0, 0.4, 0.0];
        joint.local_anchor_b = [0.0, -0.4, 0.0];
        b3_create_revolute_joint(world, &joint);
        prev = last;
    }
    b3_world_ensure_gpu(world);
    for _ in 0..20 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let owners = crate::api::b3_world_joint_writable_owners(world);
    let roots: std::collections::HashSet<u32> = owners.iter().map(|(_, r)| *r).collect();
    assert_eq!(roots.len(), 1, "connected chain must stay one writable component: {roots:?}");
    let (ok, lists) = crate::api::b3_world_gpu_joint_lists(world);
    assert!(ok);
    assert_eq!(lists.len(), 1);
    assert!(roots.contains(&lists[0].0), "list readback must preserve the component root");
    // The anchored joint gets color 22; alternating dynamic links get 0 and 1.
    // This must differ from creation order while retaining one writable owner.
    let expected: Vec<u32> = (1..24).step_by(2)
        .chain((2..24).step_by(2)).chain(std::iter::once(0)).collect();
    assert_eq!(lists[0].1, expected, "joint list must follow CPU graph priority");
    let p = b3_body_get_position(last);
    assert!(p[1].is_finite() && p[1] > -2.0, "chain end {p:?}");
    b3_destroy_world(world);
}

#[test]
fn joint_sparse_slot_reuse_and_wake_keep_exclusive_owners() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    b3_world_set_diagnostic_flags(world, DIAG_PARALLEL_JOINTS);
    let mut ground_def = b3_default_body_def();
    ground_def.position = [0.0, -1.0, 0.0];
    let ground = b3_create_body(world, &ground_def);
    let a = pendulum_on_ground(world, ground, -1.0, false);
    let b = pendulum_on_ground(world, ground, 1.0, true);
    b3_world_ensure_gpu(world);
    for _ in 0..10 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_destroy_body(b);
    let c = pendulum_on_ground(world, ground, 2.0, false);
    b3_world_ensure_gpu(world);
    b3_world_enable_sleeping(world, true);
    for _ in 0..40 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_body_set_awake(c, true);
    for _ in 0..10 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    assert_exclusive_writable_owners(world);
    let pa = b3_body_get_position(a);
    let pc = b3_body_get_position(c);
    assert!(pa[1].is_finite() && pc[1].is_finite());
    b3_destroy_world(world);
}

#[test]
fn pose_snapshot_getter_latency_is_not_a_copy_gate() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 8.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 1.0, 0.0];
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    b3_world_prepare_pose_snapshot(world);
    let copies = crate::api::b3_world_pose_snapshot_copies(world);
    let t0 = std::time::Instant::now();
    for _ in 0..2000 {
        let _ = std::hint::black_box(b3_body_get_position(body));
    }
    let _ms = t0.elapsed().as_secs_f64() * 1000.0;
    assert_eq!(crate::api::b3_world_pose_snapshot_copies(world), copies);
    b3_destroy_world(world);
}

#[test]
fn parameterized_scale_fixtures_realize_counts() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let check = |scene: crate::types::DemoScene, n: u32, bodies: i32, joints: i32| {
        let world = crate::scenes::build_demo_world(
            gpu.clone(),
            &crate::types::DemoConfig {
                body_count: n,
                body_count_explicit: true,
                contacts: true,
                scene,
                jacobi: false,
            },
        );
        let (b, _s, j) = crate::api::b3_world_counts(world);
        assert_eq!(b, bodies, "{} bodies for {n}", scene.slug());
        assert_eq!(j, joints, "{} joints for {n}", scene.slug());
        b3_destroy_world(world);
    };
    check(crate::types::DemoScene::AnchoredMechanisms, 8, 9, 8);
    check(crate::types::DemoScene::AnchoredMechanisms, 64, 65, 64);
    check(crate::types::DemoScene::JointChain, 8, 9, 8);
    check(crate::types::DemoScene::JointChain, 32, 33, 32);
    check(crate::types::DemoScene::MixedStacks, 64, 66, 0);
    check(crate::types::DemoScene::Spheres, 16, 17, 0);

    let world = crate::scenes::build_demo_world(
        gpu,
        &crate::types::DemoConfig {
            body_count: 8,
            body_count_explicit: true,
            contacts: true,
            scene: crate::types::DemoScene::AnchoredMechanisms,
            jacobi: false,
        },
    );
    b3_world_enable_sleeping(world, false);
    b3_world_ensure_gpu(world);
    for _ in 0..20 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    let bodies = pollster::block_on(crate::api::b3_world_sync_from_gpu(world));
    assert!(
        crate::dump::physics_quality_error(
            crate::types::DemoScene::AnchoredMechanisms,
            &bodies,
            20
        )
        .is_none()
    );
    let owners = crate::api::b3_world_joint_writable_owners(world);
    let mut roots: Vec<u32> = owners.iter().map(|(_, root)| *root).collect();
    roots.sort_unstable();
    roots.dedup();
    assert_eq!(roots.len(), 8, "independent pendulums must stay separate writable components");
    b3_destroy_world(world);
}

#[test]
fn completed_step_is_independent_of_timestamp_ring() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 8.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 1.0, 0.0];
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert_eq!(b3_world_physics_step(world), 1);
    b3_world_gpu_wait(world);
    let completed = crate::api::b3_world_completed_step(world).expect("completed known after wait");
    assert_eq!(completed, 1);
    b3_destroy_world(world);
}

#[test]
fn gpu_closest_ray_matches_cpu_scan_on_boxes() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 20.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    for _ in 0..8 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    let filter = crate::api::b3_default_query_filter();
    let origin = [0.0, 8.0, 0.0];
    let translation = [0.0, -10.0, 0.0];
    let gpu_hit = unsafe { crate::api::b3_world_cast_ray_closest(world, origin, translation, filter) };
    let mut cpu = crate::api::RayResult::default();
    unsafe {
        crate::api::b3_world_cast_ray(
            world,
            origin,
            translation,
            filter,
            Some(cpu_closest_callback),
            &mut cpu as *mut _ as *mut core::ffi::c_void,
        );
    }
    assert!(gpu_hit.hit, "gpu ray missed the box");
    assert!(cpu.hit, "cpu ray missed the box");
    assert_eq!(gpu_hit.shape_id.index1, cpu.shape_id.index1);
    assert!(
        (gpu_hit.fraction - cpu.fraction).abs() < 0.05,
        "gpu {} cpu {}",
        gpu_hit.fraction,
        cpu.fraction
    );
    b3_destroy_world(world);
}

#[test]
fn gpu_closest_ray_miss_mask_teleport_and_compound() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 20.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &def);
    let mut box_def = b3_default_shape_def();
    box_def.filter = Filter {
        category_bits: 0x8,
        mask_bits: u64::MAX,
        group_index: 0,
    };
    let box_shape = b3_create_hull_shape(body, &box_def, &b3_make_box_hull(0.5, 0.5, 0.5));
    def.position = [4.0, 0.5, 0.0];
    let compound = b3_create_body(world, &def);
    let mut left = b3_default_shape_def();
    left.filter = Filter {
        category_bits: 0x2,
        mask_bits: u64::MAX,
        group_index: 0,
    };
    let mut left_hull = b3_make_box_hull(0.25, 0.5, 0.25);
    left_hull.center = [-0.5, 0.0, 0.0];
    b3_create_hull_shape(compound, &left, &left_hull);
    let mut right = b3_default_shape_def();
    right.filter = Filter {
        category_bits: 0x4,
        mask_bits: u64::MAX,
        group_index: 0,
    };
    let mut right_hull = b3_make_box_hull(0.25, 0.5, 0.25);
    right_hull.center = [0.5, 0.0, 0.0];
    b3_create_hull_shape(compound, &right, &right_hull);
    def.position = [-4.0, 0.5, 0.0];
    let sphere_body = b3_create_body(world, &def);
    b3_create_sphere_shape(
        sphere_body,
        &b3_default_shape_def(),
        &Sphere {
            center: [0.0; 3],
            radius: 0.5,
        },
    );
    b3_world_ensure_gpu(world);
    for _ in 0..4 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let filter = crate::api::b3_default_query_filter();
    let miss = unsafe {
        crate::api::b3_world_cast_ray_closest(world, [50.0, 8.0, 50.0], [0.0, -1.0, 0.0], filter)
    };
    assert!(!miss.hit, "empty sky must miss");

    let mut masked = filter;
    masked.mask_bits = 0;
    let blocked = unsafe {
        crate::api::b3_world_cast_ray_closest(world, [0.0, 8.0, 0.0], [0.0, -10.0, 0.0], masked)
    };
    assert!(!blocked.hit, "zero mask must reject every shape");

    let mut cat8 = filter;
    cat8.mask_bits = 0x8;
    let only_box = unsafe {
        crate::api::b3_world_cast_ray_closest(world, [0.0, 8.0, 0.0], [0.0, -10.0, 0.0], cat8)
    };
    assert!(only_box.hit);
    assert_eq!(only_box.shape_id.index1, box_shape.index1);

    b3_body_set_transform(body, [12.0, 0.5, 0.0], [0.0, 0.0, 0.0, 1.0]);
    let teleported = unsafe {
        crate::api::b3_world_cast_ray_closest(world, [12.0, 8.0, 0.0], [0.0, -10.0, 0.0], filter)
    };
    assert!(teleported.hit, "GPU ray must see host SetTransform without a pose mirror");
    assert_eq!(teleported.shape_id.index1, box_shape.index1);

    let mut cat2 = filter;
    cat2.mask_bits = 0x2;
    let left_hit = unsafe {
        crate::api::b3_world_cast_ray_closest(world, [3.5, 8.0, 0.0], [0.0, -10.0, 0.0], cat2)
    };
    assert!(left_hit.hit, "compound child with matching mask");
    let sphere_hit = unsafe {
        crate::api::b3_world_cast_ray_closest(world, [-4.0, 8.0, 0.0], [0.0, -10.0, 0.0], filter)
    };
    assert!(sphere_hit.hit);
    let profile = crate::api::b3_world_last_query_profile(world);
    let state_bytes = core::mem::size_of::<crate::types::BodyStateGpu>() as u64;
    assert_eq!(
        profile.copied_bytes,
        64 + state_bytes,
        "teleport upload plus the 64-byte result, not a scene mirror"
    );
    assert_eq!(profile.overflow, 0);
    b3_destroy_world(world);
}

unsafe extern "C" fn cpu_closest_callback(
    shape_id: crate::api::ShapeId,
    point: crate::api::Vec3,
    normal: crate::api::Vec3,
    fraction: f32,
    user_material_id: u64,
    triangle_index: i32,
    child_index: i32,
    context: *mut core::ffi::c_void,
) -> f32 {
    if fraction == 0.0 {
        return -1.0;
    }
    let result = &mut *(context as *mut crate::api::RayResult);
    result.shape_id = shape_id;
    result.point = point.into();
    result.normal = normal.into();
    result.user_material_id = user_material_id;
    result.fraction = fraction;
    result.triangle_index = triangle_index;
    result.child_index = child_index;
    result.hit = true;
    fraction
}

fn query_ray(
    world: crate::api::WorldId,
    origin: [f32; 3],
    translation: [f32; 3],
) -> crate::api::RayResult {
    let filter = crate::api::b3_default_query_filter();
    unsafe { crate::api::b3_world_cast_ray_closest(world, origin, translation, filter) }
}

fn cpu_oracle_ray(
    world: crate::api::WorldId,
    origin: [f32; 3],
    translation: [f32; 3],
) -> crate::api::RayResult {
    let filter = crate::api::b3_default_query_filter();
    let mut cpu = crate::api::RayResult::default();
    unsafe {
        crate::api::b3_world_cast_ray(
            world,
            origin,
            translation,
            filter,
            Some(cpu_closest_callback),
            &mut cpu as *mut _ as *mut core::ffi::c_void,
        );
    }
    cpu
}

fn finite3(v: [f32; 3]) -> bool {
    v.iter().all(|x| x.is_finite())
}

fn box_as_convex(h: [f32; 3]) -> ConvexHull {
    let hx = h[0];
    let hy = h[1];
    let hz = h[2];
    ConvexHull {
        points: vec![
            [-hx, -hy, -hz],
            [hx, -hy, -hz],
            [hx, hy, -hz],
            [-hx, hy, -hz],
            [-hx, -hy, hz],
            [hx, -hy, hz],
            [hx, hy, hz],
            [-hx, hy, hz],
        ],
        planes: vec![
            [1.0, 0.0, 0.0, hx],
            [-1.0, 0.0, 0.0, hx],
            [0.0, 1.0, 0.0, hy],
            [0.0, -1.0, 0.0, hy],
            [0.0, 0.0, 1.0, hz],
            [0.0, 0.0, -1.0, hz],
        ],
        edge_directions: vec![[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]],
        half_edges: Vec::new(),
        half_extents: h,
        aabb_center: [0.0; 3],
        center: [0.0; 3],
        inner_radius: hx.min(hy).min(hz) * 0.1,
        volume: 8.0 * hx * hy * hz,
        central_inertia: [1.0; 6],
    }
}

fn assert_same_hit(gpu: &crate::api::RayResult, cpu: &crate::api::RayResult, label: &str) {
    assert_eq!(gpu.hit, cpu.hit, "{label} hit mismatch");
    if !gpu.hit {
        return;
    }
    assert_eq!(gpu.shape_id.index1, cpu.shape_id.index1, "{label} shape");
    assert!(
        (gpu.fraction - cpu.fraction).abs() < 0.02,
        "{label} fraction gpu={} cpu={}",
        gpu.fraction,
        cpu.fraction
    );
    assert!(finite3(gpu.point) && finite3(gpu.normal), "{label} non-finite");
    let n = (gpu.normal[0] * gpu.normal[0] + gpu.normal[1] * gpu.normal[1] + gpu.normal[2] * gpu.normal[2])
        .sqrt();
    assert!((n - 1.0).abs() < 0.05, "{label} normal length {n}");
}

#[test]
fn gpu_closest_ray_winner_is_one_shape_on_ties() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let body = b3_create_body(world, &b3_default_body_def());
    let mut a = b3_default_shape_def();
    a.user_material_id = 11;
    let mut b = b3_default_shape_def();
    b.user_material_id = 22;
    let first = b3_create_hull_shape(body, &a, &b3_make_box_hull(0.5, 0.5, 0.5));
    let second = b3_create_hull_shape(body, &b, &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    let mut ids = Vec::new();
    for _ in 0..8 {
        let hit = query_ray(world, [0.0, 4.0, 0.0], [0.0, -8.0, 0.0]);
        assert!(hit.hit);
        assert!(finite3(hit.normal));
        ids.push((hit.shape_id.index1, hit.user_material_id, hit.fraction));
    }
    assert!(ids.iter().all(|row| *row == ids[0]), "tie result must be one writer: {ids:?}");
    assert_eq!(ids[0].0, first.index1.min(second.index1));
    assert_eq!(ids[0].1, if ids[0].0 == first.index1 { 11 } else { 22 });

    b3_destroy_shape(first, true);
    let after_destroy = query_ray(world, [0.0, 4.0, 0.0], [0.0, -8.0, 0.0]);
    assert!(after_destroy.hit);
    assert_eq!(after_destroy.shape_id.index1, second.index1);
    assert_eq!(after_destroy.user_material_id, 22);
    let reused = b3_create_hull_shape(body, &a, &b3_make_box_hull(0.5, 0.5, 0.5));
    let after = query_ray(world, [0.0, 4.0, 0.0], [0.0, -8.0, 0.0]);
    assert!(after.hit);
    assert!(after.shape_id.index1 == reused.index1 || after.shape_id.index1 == second.index1);
    b3_destroy_world(world);
}

#[test]
fn gpu_closest_ray_capsule_and_box_normals() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let cap_body = b3_create_body(world, &{
        let mut def = b3_default_body_def();
        def.position = [30.0, 0.0, 0.0];
        def
    });
    b3_create_capsule_shape(
        cap_body,
        &b3_default_shape_def(),
        &Capsule {
            center1: [0.0, -1.0, 0.0],
            center2: [0.0, 1.0, 0.0],
            radius: 0.5,
        },
    );
    let box_body = b3_create_body(world, &b3_default_body_def());
    b3_create_hull_shape(box_body, &b3_default_shape_def(), &b3_make_box_hull(1.0, 1.0, 1.0));
    b3_world_ensure_gpu(world);

    let cap = query_ray(world, [30.0, 10.0, 0.0], [0.0, -20.0, 0.0]);
    assert!(cap.hit, "axis-parallel capsule must hit the far cap");
    assert!((cap.fraction - 0.425).abs() < 1e-3, "fraction {}", cap.fraction);
    assert!((cap.point[1] - 1.5).abs() < 1e-3, "point {:?}", cap.point);

    let faces = [
        ([3.0, 0.0, 0.0], [-6.0, 0.0, 0.0], [1.0, 0.0, 0.0], [1.0, 0.0, 0.0]),
        ([-3.0, 0.0, 0.0], [6.0, 0.0, 0.0], [-1.0, 0.0, 0.0], [-1.0, 0.0, 0.0]),
        ([0.0, 3.0, 0.0], [0.0, -6.0, 0.0], [0.0, 1.0, 0.0], [0.0, 1.0, 0.0]),
        ([0.0, -3.0, 0.0], [0.0, 6.0, 0.0], [0.0, -1.0, 0.0], [0.0, -1.0, 0.0]),
        ([0.0, 0.0, 3.0], [0.0, 0.0, -6.0], [0.0, 0.0, 1.0], [0.0, 0.0, 1.0]),
        ([0.0, 0.0, -3.0], [0.0, 0.0, 6.0], [0.0, 0.0, -1.0], [0.0, 0.0, -1.0]),
    ];
    for (origin, translation, expect_p, expect_n) in faces {
        let hit = query_ray(world, origin, translation);
        assert!(hit.hit, "box face from {origin:?}");
        assert!((hit.point[0] - expect_p[0]).abs() < 1e-3);
        assert!((hit.point[1] - expect_p[1]).abs() < 1e-3);
        assert!((hit.point[2] - expect_p[2]).abs() < 1e-3);
        assert!((hit.normal[0] - expect_n[0]).abs() < 1e-3);
        assert!((hit.normal[1] - expect_n[1]).abs() < 1e-3);
        assert!((hit.normal[2] - expect_n[2]).abs() < 1e-3);
    }

    let rot_body = b3_create_body(world, &{
        let mut def = b3_default_body_def();
        def.position = [8.0, 0.0, 0.0];
        def.rotation = b3_make_quat_from_axis_angle([0.0, 0.0, 1.0], 0.5 * std::f32::consts::PI);
        def
    });
    b3_create_hull_shape(rot_body, &b3_default_shape_def(), &b3_make_box_hull(1.0, 1.0, 1.0));
    let rotated = query_ray(world, [8.0, 4.0, 0.0], [0.0, -8.0, 0.0]);
    assert!(rotated.hit);
    assert!(finite3(rotated.normal));

    let inside = query_ray(world, [0.0, 0.0, 0.0], [0.0, -1.0, 0.0]);
    assert!(!inside.hit, "inside origin is a miss for closest-hit");
    let zero = query_ray(world, [3.0, 0.0, 0.0], [0.0, 0.0, 0.0]);
    assert!(!zero.hit);
    let nan = query_ray(world, [f32::NAN, 0.0, 0.0], [0.0, -1.0, 0.0]);
    assert!(!nan.hit);
    b3_destroy_world(world);
}

#[test]
fn gpu_closest_ray_mesh_height_and_oracle_primitives() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let ground = b3_create_body(world, &b3_default_body_def());
    let mesh = b3_create_mesh_shape(
        ground,
        &b3_default_shape_def(),
        &[
            [-2.0, 0.0, -2.0],
            [2.0, 0.0, -2.0],
            [2.0, 0.0, 2.0],
            [-2.0, 0.0, 2.0],
        ],
        &[[0, 2, 1], [0, 3, 2]],
        &[],
        &[],
        &[] as &[MeshNode],
        [1.0, 1.0, 1.0],
    );
    let mut buried_def = b3_default_body_def();
    buried_def.position = [0.0, -2.0, 0.0];
    let buried = b3_create_body(world, &buried_def);
    b3_create_hull_shape(buried, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    let mut box_def = b3_default_body_def();
    box_def.position = [0.0, 3.0, 0.0];
    let box_body = b3_create_body(world, &box_def);
    let box_shape = b3_create_hull_shape(box_body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);

    let occluded = query_ray(world, [0.0, 8.0, 0.0], [0.0, -12.0, 0.0]);
    assert!(occluded.hit);
    assert_eq!(occluded.shape_id.index1, box_shape.index1, "primitive in front of mesh");

    b3_body_set_transform(box_body, [8.0, 3.0, 0.0], [0.0, 0.0, 0.0, 1.0]);
    let mesh_hit = query_ray(world, [0.0, 8.0, 0.0], [0.0, -12.0, 0.0]);
    assert!(mesh_hit.hit);
    assert_eq!(mesh_hit.shape_id.index1, mesh.index1, "mesh must occlude the buried box");
    assert!(mesh_hit.triangle_index >= 0);

    b3_destroy_shape(mesh, false);
    let mut patch_def = b3_default_body_def();
    patch_def.position = [5.0, 0.0, 0.0];
    let patch_body = b3_create_body(world, &patch_def);
    let replacement = b3_create_mesh_shape(
        patch_body,
        &b3_default_shape_def(),
        &[
            [-0.5, 0.0, -0.5],
            [0.5, 0.0, -0.5],
            [0.5, 0.0, 0.5],
            [-0.5, 0.0, 0.5],
        ],
        &[[0, 2, 1], [0, 3, 2]],
        &[],
        &[],
        &[] as &[MeshNode],
        [1.0, 1.0, 1.0],
    );
    assert!(b3_replace_mesh_shape(
        replacement,
        &[
            [-0.5, 0.0, -0.5],
            [0.5, 0.0, -0.5],
            [0.5, 0.0, 0.5],
            [-0.5, 0.0, 0.5],
        ],
        &[[0, 2, 1], [0, 3, 2]],
        &[],
        &[],
        &[] as &[MeshNode],
        [1.0, 1.0, 1.0],
    ));
    b3_world_ensure_gpu(world);
    let through = query_ray(world, [0.0, 8.0, 0.0], [0.0, -12.0, 0.0]);
    assert!(through.hit);
    assert_ne!(
        through.shape_id.index1, replacement.index1,
        "replacement mesh must leave the origin open"
    );
    let moved = query_ray(world, [5.0, 8.0, 0.0], [0.0, -12.0, 0.0]);
    assert!(moved.hit);
    assert_eq!(moved.shape_id.index1, replacement.index1);

    let mut flip_def = b3_default_body_def();
    flip_def.position = [-8.0, 0.0, 0.0];
    let flip_body = b3_create_body(world, &flip_def);
    let flipped = b3_create_mesh_shape(
        flip_body,
        &b3_default_shape_def(),
        &[
            [-0.5, 0.0, -0.5],
            [0.5, 0.0, -0.5],
            [0.5, 0.0, 0.5],
            [-0.5, 0.0, 0.5],
        ],
        &[[0, 1, 2], [0, 2, 1], [0, 2, 3], [0, 3, 2]],
        &[],
        &[],
        &[] as &[MeshNode],
        [-1.0, 1.0, 1.0],
    );
    let neg = query_ray(world, [-8.0, 4.0, 0.0], [0.0, -8.0, 0.0]);
    assert!(neg.hit, "negative scale mesh");
    assert_eq!(neg.shape_id.index1, flipped.index1);

    let mut hf_def = b3_default_body_def();
    hf_def.position = [20.0, 0.0, 20.0];
    let hf_body = b3_create_body(world, &hf_def);
    let heights = [0u16, 0, 0, 0];
    let hf = b3_create_height_field_shape(
        hf_body,
        &b3_default_shape_def(),
        &heights,
        2,
        2,
        0.0,
        1.0,
        [1.0, 1.0, 1.0],
        &[0],
        &[0, 0],
        false,
    );
    let hf_hit = query_ray(world, [20.5, 4.0, 20.5], [0.0, -8.0, 0.0]);
    assert!(hf_hit.hit);
    assert_eq!(hf_hit.shape_id.index1, hf.index1);

    let sph_body = b3_create_body(world, &{
        let mut def = b3_default_body_def();
        def.position = [12.0, 0.0, 0.0];
        def
    });
    b3_create_sphere_shape(
        sph_body,
        &b3_default_shape_def(),
        &Sphere {
            center: [0.0; 3],
            radius: 0.5,
        },
    );
    let hull_body = b3_create_body(world, &{
        let mut def = b3_default_body_def();
        def.position = [15.0, 0.0, 0.0];
        def
    });
    b3_create_convex_hull_shape(hull_body, &b3_default_shape_def(), &box_as_convex([0.5, 0.5, 0.5]));
    b3_world_ensure_gpu(world);
    for origin in [[12.0, 4.0, 0.0], [15.0, 4.0, 0.0], [0.0, 4.0, 0.0]] {
        let gpu_hit = query_ray(world, origin, [0.0, -8.0, 0.0]);
        let cpu_hit = cpu_oracle_ray(world, origin, [0.0, -8.0, 0.0]);
        assert_same_hit(&gpu_hit, &cpu_hit, &format!("oracle at {origin:?}"));
    }
    let tangent = query_ray(world, [12.5, 4.0, 0.0], [0.0, -8.0, 0.0]);
    let cpu_t = cpu_oracle_ray(world, [12.5, 4.0, 0.0], [0.0, -8.0, 0.0]);
    assert_eq!(tangent.hit, cpu_t.hit);
    b3_destroy_world(world);
}

#[test]
fn gpu_closest_ray_teleport_does_not_rewind_neighbors() {
    let gpu_ref = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let gpu_q = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let make = |gpu: GpuDevice| {
        let world = b3_create_world(gpu, &b3_default_world_def());
        b3_world_enable_sleeping(world, false);
        create_ground(world, 20.0);
        let mut def = b3_default_body_def();
        def.body_type = BodyType::Dynamic;
        def.position = [0.0, 4.0, 0.0];
        let a = b3_create_body(world, &def);
        b3_create_hull_shape(a, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
        def.position = [4.0, 4.0, 0.0];
        let b = b3_create_body(world, &def);
        b3_create_hull_shape(b, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
        b3_world_ensure_gpu(world);
        (world, a, b)
    };
    let (ref_world, ref_a, _) = make(gpu_ref);
    let (q_world, q_a, q_b) = make(gpu_q);
    for _ in 0..6 {
        b3_world_step_gpu(ref_world, 1.0 / 60.0, 4);
        b3_world_step_gpu(q_world, 1.0 / 60.0, 4);
    }
    b3_body_set_transform(q_b, [12.0, 4.0, 0.0], [0.0, 0.0, 0.0, 1.0]);
    let _ = query_ray(q_world, [12.0, 8.0, 0.0], [0.0, -10.0, 0.0]);
    b3_world_step_gpu(ref_world, 1.0 / 60.0, 4);
    b3_world_step_gpu(q_world, 1.0 / 60.0, 4);
    b3_world_gpu_wait_with_mirror(ref_world);
    b3_world_gpu_wait_with_mirror(q_world);
    let pa = b3_body_get_position(ref_a);
    let pb = b3_body_get_position(q_a);
    assert!(
        (pa[1] - pb[1]).abs() < 0.05,
        "query/teleport of B must not rewind A: {pa:?} vs {pb:?}"
    );
    b3_destroy_world(ref_world);
    b3_destroy_world(q_world);
}

#[test]
fn gpu_closest_ray_ccd_and_events_independent_of_picks() {
    let gpu_none = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let gpu_many = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let gpu_delay = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let setup = |gpu: GpuDevice| {
        let world = b3_create_world(gpu, &b3_default_world_def());
        b3_world_enable_sleeping(world, false);
        b3_world_enable_continuous(world, true);
        create_ground(world, 20.0);
        let mut def = b3_default_body_def();
        def.body_type = BodyType::Dynamic;
        def.is_bullet = true;
        def.position = [0.0, 6.0, 0.0];
        let body = b3_create_body(world, &def);
        let mut shape = b3_default_shape_def();
        shape.enable_contact_events = true;
        b3_create_sphere_shape(
            body,
            &shape,
            &Sphere {
                center: [0.0; 3],
                radius: 0.25,
            },
        );
        b3_body_set_linear_velocity(body, [0.0, -40.0, 0.0]);
        b3_world_ensure_gpu(world);
        (world, body)
    };
    let (none_w, none_b) = setup(gpu_none);
    let (many_w, many_b) = setup(gpu_many);
    let (delay_w, delay_b) = setup(gpu_delay);
    b3_world_step_gpu(none_w, 1.0 / 60.0, 4);
    b3_world_step_gpu(many_w, 1.0 / 60.0, 4);
    b3_world_step_gpu(delay_w, 1.0 / 60.0, 4);
    for _ in 0..4 {
        let _ = query_ray(many_w, [0.0, 8.0, 0.0], [0.0, -12.0, 0.0]);
    }
    b3_world_gpu_wait_with_mirror(none_w);
    b3_world_gpu_wait_with_mirror(many_w);
    let _ = query_ray(delay_w, [0.0, 8.0, 0.0], [0.0, -12.0, 0.0]);
    b3_world_gpu_wait_with_mirror(delay_w);
    let pn = b3_body_get_position(none_b);
    let pm = b3_body_get_position(many_b);
    let pd = b3_body_get_position(delay_b);
    assert!((pn[1] - pm[1]).abs() < 0.08, "many queries {pm:?} vs none {pn:?}");
    assert!((pn[1] - pd[1]).abs() < 0.08, "delayed harvest {pd:?} vs none {pn:?}");
    let events = |id| {
        let (_b, n, _, _, _, _) = b3_world_contact_event_ptrs(id);
        n
    };
    assert_eq!(events(none_w), events(many_w));
    assert_eq!(events(none_w), events(delay_w));
    b3_destroy_world(none_w);
    b3_destroy_world(many_w);
    b3_destroy_world(delay_w);
}

#[test]
fn completed_ccd_correction_survives_staged_pose_reads() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_ground(world, 10.0);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0,0.8,0.0];
    def.is_bullet = true;
    let body = b3_create_body(world, &def);
    b3_create_sphere_shape(body, &b3_default_shape_def(), &Sphere { center: [0.0;3], radius: 0.25 });
    b3_body_set_linear_velocity(body, [0.0,-80.0,0.0]);
    b3_world_step_gpu(world, 1.0/60.0, 4);
    b3_world_gpu_wait_with_mirror(world);
    let corrected = crate::api::b3_body_get_world_center(body);
    assert!(corrected[1] > 0.2, "CCD failed to clamp: {corrected:?}");
    let maps = crate::api::b3_world_pose_snapshot_maps(world);
    for _ in 0..4 {
        b3_world_prepare_pose_snapshot(world);
        assert_eq!(b3_body_get_position(body), corrected, "pre-CCD staged pose overwrote corrected state");
    }
    assert_eq!(crate::api::b3_world_pose_snapshot_maps(world), maps);
    b3_destroy_world(world);
}

#[test]
fn gpu_joint_lists_are_exclusive_and_ordered() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    b3_world_enable_sleeping(world, false);
    let mut ground_def = b3_default_body_def();
    ground_def.position = [0.0, -1.0, 0.0];
    let ground = b3_create_body(world, &ground_def);
    b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(40.0, 1.0, 4.0));
    for i in 0..24 {
        pendulum_on_ground(world, ground, -12.0 + i as f32, i % 2 == 0);
    }
    b3_world_ensure_gpu(world);
    for _ in 0..4 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait(world);
    assert_exclusive_writable_owners(world);
    let (ok, lists) = crate::api::b3_world_gpu_joint_lists(world);
    assert!(ok, "joint list construction must succeed");
    let mut seen = std::collections::HashSet::new();
    for (_root, joints) in &lists {
        let mut last = None;
        for j in joints {
            assert!(seen.insert(*j), "joint {j} listed twice");
            if let Some(prev) = last {
                assert!(*j > prev, "component list must keep original joint order");
            }
            last = Some(*j);
        }
    }
    assert_eq!(seen.len(), 24, "every live joint must appear once, got {}", seen.len());
    b3_destroy_world(world);
}

#[test]
fn joint_contact_wave_matches_general_colors_with_more_than_64_components() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let run = |flags| {
        let world = b3_create_world(gpu.clone(), &b3_default_world_def());
        b3_world_enable_sleeping(world, false);
        b3_world_set_diagnostic_flags(world, flags);
        let ground = create_ground(world, 100.0);
        let mut bodies = Vec::new();
        for i in 0..80 {
            let x = -79.0 + 2.0 * i as f32;
            for z in [0.0, 1.1] {
                let mut bd = b3_default_body_def();
                bd.body_type = BodyType::Dynamic;
                bd.position = [x, 0.45, z];
                let body = b3_create_body(world, &bd);
                b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.4, 0.5, 0.4));
                bodies.push(body);
            }
            let mut joint = b3_default_spherical_joint_def();
            joint.body_a = bodies[bodies.len() - 2];
            joint.body_b = bodies[bodies.len() - 1];
            joint.local_anchor_a = [0.0, 0.0, 0.55];
            joint.local_anchor_b = [0.0, 0.0, -0.55];
            b3_create_spherical_joint(world, &joint);
            joint.body_b = joint.body_a;
            joint.body_a = ground;
            joint.local_anchor_a = [x, 1.45, 0.0];
            joint.local_anchor_b = [0.0; 3];
            joint.collide_connected = true;
            b3_create_spherical_joint(world, &joint);
        }
        for _ in 0..10 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
        }
        b3_world_gpu_wait_with_mirror(world);
        assert_exclusive_writable_owners(world);
        let (ok, lists) = crate::api::b3_world_gpu_joint_lists(world);
        assert!(ok);
        assert_eq!(lists.len(), 80);
        let contacts = pollster::block_on(b3_world_sync_contacts(world));
        let mut counts = [0; 24];
        for c in contacts.iter().filter(|c| c.a != u32::MAX && c.count > 0) {
            // Ground joint owns color 22 on the first body of each pair. Its
            // ground contact must use 21; the second body's contact gets 22.
            let dynamic = if c.a == 0 { c.b } else { c.a };
            let expected = if dynamic % 2 == 1 { 21 } else { 22 };
            assert_eq!(c.color, expected, "contact conflicts with joint color");
            counts[c.color as usize] += 1;
        }
        assert!(counts[21] > 64 && counts[22] > 64, "exercise strided contact waves: {counts:?}");
        let result = poses_of(&bodies);
        b3_destroy_world(world);
        result
    };
    let wave = run(0);
    let general = run(DIAG_GENERAL_SOLVER);
    for (a, b) in wave.iter().zip(&general) {
        for (x, y) in a.0.iter().chain(&a.1).zip(b.0.iter().chain(&b.1)) {
            assert!(x.is_finite() && y.is_finite() && (x-y).abs() < 1e-5, "wave {a:?}, general {b:?}");
        }
    }
}

#[test]
fn getters_match_aabb_after_step_without_wait() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let timestamps = gpu.timestamp_queries;
    let world = b3_create_world(gpu, &b3_default_world_def());
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 5.0, 0.0];
    let body = b3_create_body(world, &body_def);
    let shape = b3_create_sphere_shape(
        body,
        &b3_default_shape_def(),
        &Sphere {
            center: [0.0; 3],
            radius: 0.25,
        },
    );
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let pos = b3_body_get_position(body);
    if timestamps {
        assert_eq!(crate::api::b3_world_last_timestamp_step(world), 1,
            "completed mirror must publish clocks even when drawing skips the consumed pose snapshot");
        assert!(crate::api::b3_world_last_device_ms(world) > 0.0);
    }
    let aabb = b3_shape_get_aabb(shape);
    let aabb_y = 0.5 * (aabb.lower_bound[1] + aabb.upper_bound[1]);
    assert!(pos[1] < 5.0, "body should have fallen, y={}", pos[1]);
    assert!(
        (aabb_y - pos[1]).abs() < 1e-5,
        "GetPosition y={} AABB center y={}",
        pos[1],
        aabb_y
    );
    b3_destroy_world(world);
}

#[test]
fn paired_spherical_joints_stay_finite() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    let mut ground_def = b3_default_body_def();
    ground_def.position = [0.0, -1.0, 0.0];
    let ground = b3_create_body(world, &ground_def);
    b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(20.0, 1.0, 4.0));
    let mut shape = b3_default_shape_def();
    shape.density = 20.0;
    let mut ids = Vec::new();
    for i in 0..6 {
        let mut body_def = b3_default_body_def();
        body_def.body_type = BodyType::Dynamic;
        body_def.position = [i as f32 * 1.0, 4.0, 0.0];
        let body = b3_create_body(world, &body_def);
        b3_create_hull_shape(body, &shape, &b3_make_box_hull(0.5, 0.1, 0.4));
        ids.push(body);
    }
    for pair in ids.windows(2) {
        for z in [-0.35, 0.35] {
            let mut joint = b3_default_spherical_joint_def();
            joint.body_a = pair[0];
            joint.body_b = pair[1];
            joint.local_anchor_a = [0.5, 0.0, z];
            joint.local_anchor_b = [-0.5, 0.0, z];
            joint.hertz = 1000.0;
            joint.enable_spring = true;
            joint.spring_hertz = 2.0;
            joint.spring_damping = 1.0;
            b3_create_spherical_joint(world, &joint);
        }
    }
    b3_world_ensure_gpu(world);
    for _ in 0..120 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    let pos = b3_body_get_position(ids[3]);
    assert!(pos.iter().all(|v| v.is_finite()), "{pos:?}");
    assert!(pos[1] > -2.0 && pos[1] < 12.0, "bridge exploded {:?}", pos);
    let (ok, lists) = crate::api::b3_world_gpu_joint_lists(world);
    assert!(ok, "paired joints must fit compact lists");
    let listed: usize = lists.iter().map(|(_, j)| j.len()).sum();
    assert_eq!(listed, 10);
    b3_destroy_world(world);
}

#[test]
fn prismatic_blocks_transverse_velocity_at_zero_position_error() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.gravity = [0.0; 3];
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    let a = b3_create_body(world, &b3_default_body_def());
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    let b = b3_create_body(world, &bd);
    b3_create_hull_shape(b, &b3_default_shape_def(), &b3_make_box_hull(0.2, 0.3, 0.4));
    let mut joint = b3_default_prismatic_joint_def();
    joint.body_a = a;
    joint.body_b = b;
    b3_create_prismatic_joint(world, &joint);
    b3_body_set_linear_velocity(b, [0.0, 3.0, 4.0]);
    b3_world_step_gpu(world, 1.0 / 60.0, 1);
    b3_world_gpu_wait_with_mirror(world);
    let p = b3_body_get_position(b);
    let v = b3_body_get_linear_velocity(b);
    b3_destroy_world(world);
    // The constraint must act before integration even with coincident anchors.
    // Joint softness permits a small displacement, not a free ballistic step.
    assert!(p[1].abs() < 0.01 && p[2].abs() < 0.01, "transverse displacement {p:?}, velocity {v:?}");
}

#[test]
fn prismatic_locks_relative_rotation() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    let mut a_def = b3_default_body_def();
    a_def.body_type = BodyType::Static;
    a_def.position = [0.0, 2.0, 0.0];
    let a = b3_create_body(world, &a_def);
    b3_create_hull_shape(a, &b3_default_shape_def(), &b3_make_box_hull(0.2, 0.2, 0.2));
    let mut b_def = b3_default_body_def();
    b_def.body_type = BodyType::Dynamic;
    b_def.position = [0.0, 1.0, 0.0];
    let b = b3_create_body(world, &b_def);
    b3_create_hull_shape(b, &b3_default_shape_def(), &b3_make_box_hull(0.2, 0.2, 0.2));
    let mut prism = b3_default_prismatic_joint_def();
    prism.body_a = a;
    prism.body_b = b;
    prism.local_anchor_a = [0.0, 0.0, 0.0];
    prism.local_anchor_b = [0.0, 0.0, 0.0];
    prism.local_axis_a = [0.0, 1.0, 0.0];
    b3_create_prismatic_joint(world, &prism);
    b3_body_set_linear_velocity(b, [0.4, 0.0, 0.0]);
    b3_world_ensure_gpu(world);
    for _ in 0..60 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    let rot = b3_body_get_rotation(b);
    let twist = rot[0].abs() + rot[2].abs();
    assert!(twist < 0.15, "prismatic spun {:?}", rot);
    let pos = b3_body_get_position(b);
    assert!(pos[0].abs() < 0.2, "slide axis leaked {:?}", pos);
    b3_destroy_world(world);
}

#[test]
fn locked_hull_rests_on_height_field() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    let ground = b3_create_body(world, &b3_default_body_def());
    let heights = [0u16; 25];
    b3_create_height_field_shape(
        ground,
        &b3_default_shape_def(),
        &heights,
        5,
        5,
        0.0,
        1.0,
        [2.0, 1.0, 2.0],
        &[0; 16],
        &[0; 32],
        false,
    );
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [2.0, 3.0, 2.0];
    body_def.motion_locks = MotionLocks {
        linear_x: false,
        linear_y: false,
        linear_z: false,
        angular_x: true,
        angular_y: true,
        angular_z: true,
    };
    let body = b3_create_body(world, &body_def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.4, 0.9, 0.4));
    b3_world_ensure_gpu(world);
    for _ in 0..180 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    let pos = b3_body_get_position(body);
    assert!(pos[1] > 0.5, "fell through height field {:?}", pos);
    assert!(pos[1] < 4.0, "{:?}", pos);
    b3_destroy_world(world);
}

#[test]
fn kinematic_mesh_supports_dynamic_box() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    let mut mesh_def = b3_default_body_def();
    mesh_def.body_type = BodyType::Kinematic;
    mesh_def.position = [0.0, 0.0, 0.0];
    let mesh_body = b3_create_body(world, &mesh_def);
    let heights = [0u16; 9];
    b3_create_height_field_shape(
        mesh_body,
        &b3_default_shape_def(),
        &heights,
        3,
        3,
        0.0,
        1.0,
        [4.0, 1.0, 4.0],
        &[0; 4],
        &[0; 8],
        false,
    );
    let mut box_def = b3_default_body_def();
    box_def.body_type = BodyType::Dynamic;
    box_def.position = [2.0, 2.0, 2.0];
    let cube = b3_create_body(world, &box_def);
    b3_create_hull_shape(cube, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    for _ in 0..120 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    let pos = b3_body_get_position(cube);
    assert!(pos[1] > 0.3, "fell through kinematic height field {:?}", pos);
    b3_destroy_world(world);
}

#[test]
fn hit_event_flag_does_not_change_impact_pose() {
    let gpu_a = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let gpu_b = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let setup = |gpu, hits: bool| {
        let mut def = b3_default_world_def();
        def.enable_sleep = false;
        let world = b3_create_world(gpu, &def);
        let ground = b3_create_body(world, &b3_default_body_def());
        b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(8.0, 0.5, 8.0));
        let mut body_def = b3_default_body_def();
        body_def.body_type = BodyType::Dynamic;
        body_def.position = [0.0, 6.0, 0.0];
        let body = b3_create_body(world, &body_def);
        let mut shape = b3_default_shape_def();
        shape.enable_hit_events = hits;
        shape.restitution = 0.8;
        b3_create_sphere_shape(
            body,
            &shape,
            &Sphere {
                center: [0.0; 3],
                radius: 0.5,
            },
        );
        b3_world_enable_continuous(world, true);
        b3_world_ensure_gpu(world);
        (world, body)
    };
    let (wa, ba) = setup(gpu_a, false);
    let (wb, bb) = setup(gpu_b, true);
    for _ in 0..90 {
        b3_world_step_gpu(wa, 1.0 / 60.0, 4);
        b3_world_step_gpu(wb, 1.0 / 60.0, 4);
    }
    let pa = b3_body_get_position(ba);
    let pb = b3_body_get_position(bb);
    assert!((pa[1] - pb[1]).abs() < 0.05, "hit events changed response {pa:?} vs {pb:?}");
    let _ = b3_world_contact_event_ptrs(wb);
    let pc = b3_body_get_position(bb);
    assert!((pb[1] - pc[1]).abs() < 1e-5, "event harvest changed pose");
    b3_destroy_world(wa);
    b3_destroy_world(wb);
}

#[test]
fn unsupported_scene_sets_fail_without_abort() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let err = GpuSceneCaps::live(8, 100, 64, 64, 64, 64, 64, 64, 64, u32::MAX, 0)
        .validate_allocation(7);
    assert!(err.is_err(), "{err:?}");
    assert!(b3_world_gpu_fail(world).is_null());
    b3_destroy_world(world);
}

#[test]
fn locked_hull_rests_on_triangle_mesh() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    let ground = b3_create_body(world, &b3_default_body_def());
    let vertices = [
        [-2.0, 0.0, -2.0],
        [2.0, 0.0, -2.0],
        [2.0, 0.0, 2.0],
        [-2.0, 0.0, 2.0],
    ];
    let triangles = [[0u32, 2, 1], [0, 3, 2]];
    b3_create_mesh_shape(
        ground,
        &b3_default_shape_def(),
        &vertices,
        &triangles,
        &[],
        &[],
        &[],
        [1.0, 1.0, 1.0],
    );
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 3.5, 0.0];
    body_def.motion_locks = MotionLocks {
        linear_x: false,
        linear_y: false,
        linear_z: false,
        angular_x: true,
        angular_y: true,
        angular_z: true,
    };
    let body = b3_create_body(world, &body_def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.25, 1.0, 0.25));
    b3_world_enable_continuous(world, true);
    b3_world_ensure_gpu(world);
    for _ in 0..180 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    let pos = b3_body_get_position(body);
    assert!(pos[1] > 0.7, "fell through triangle mesh {:?}", pos);
    assert!(pos[1] < 4.0, "{:?}", pos);
    b3_destroy_world(world);
}

#[test]
fn parallel_joint_holds_chassis_upright() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    let ground = b3_create_body(world, &b3_default_body_def());
    b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(8.0, 0.5, 8.0));
    let mut chassis_def = b3_default_body_def();
    chassis_def.body_type = BodyType::Dynamic;
    chassis_def.position = [0.0, 2.5, 0.0];
    let chassis = b3_create_body(world, &chassis_def);
    let mut shape = b3_default_shape_def();
    shape.density = 0.5;
    b3_create_hull_shape(chassis, &shape, &b3_make_box_hull(2.0, 0.5, 1.0));
    let mut parallel = b3_default_parallel_joint_def();
    parallel.body_a = ground;
    parallel.body_b = chassis;
    parallel.hertz = 0.5;
    parallel.damping = 1.0;
    parallel.collide_connected = true;
    b3_create_parallel_joint(world, &parallel);
    b3_body_set_linear_velocity(chassis, [0.0, 0.0, 2.0]);
    b3_world_ensure_gpu(world);
    for _ in 0..120 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    let rot = b3_body_get_rotation(chassis);
    let tilt = rot[0].abs() + rot[2].abs();
    assert!(tilt < 0.25, "chassis rolled over {:?}", rot);
    let pos = b3_body_get_position(chassis);
    assert!(pos[1] > 0.5 && pos[1] < 6.0, "{:?}", pos);
    b3_destroy_world(world);
}

#[test]
fn distance_joint_box_stays_bounded() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    let ground = b3_create_body(world, &b3_default_body_def());
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [-12.5, 10.0, 0.0];
    let body = b3_create_body(world, &body_def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(1.0, 1.0, 0.5));
    let mut joint = b3_default_distance_joint_def();
    joint.body_a = ground;
    joint.body_b = body;
    joint.local_anchor_a = [-12.5, 13.0, 0.0];
    joint.local_anchor_b = [0.0, 1.0, 0.0];
    joint.length = 2.0;
    joint.collide_connected = true;
    b3_create_distance_joint(world, &joint);
    b3_world_ensure_gpu(world);
    for _ in 0..120 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    let pos = b3_body_get_position(body);
    assert!(pos.iter().all(|v| v.is_finite()), "{pos:?}");
    assert!(pos[1] > 0.0 && pos[1] < 20.0, "distance joint exploded {:?}", pos);
    b3_destroy_world(world);
}

#[test]
fn picking_does_not_change_completed_pose() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    let ground = b3_create_body(world, &b3_default_body_def());
    b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(8.0, 0.5, 8.0));
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 4.0, 0.0];
    let body = b3_create_body(world, &body_def);
    b3_create_sphere_shape(
        body,
        &b3_default_shape_def(),
        &Sphere {
            center: [0.0; 3],
            radius: 0.4,
        },
    );
    b3_world_enable_continuous(world, true);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let before = b3_body_get_position(body);
    unsafe {
        let _ = b3_world_cast_ray_closest(
            world,
            [0.0, 10.0, 0.0],
            [0.0, -20.0, 0.0],
            b3_default_query_filter(),
        );
        let _ = b3_world_cast_ray_closest(
            world,
            [0.0, 10.0, 0.0],
            [0.0, -20.0, 0.0],
            b3_default_query_filter(),
        );
    }
    let after = b3_body_get_position(body);
    assert!(
        (before[1] - after[1]).abs() < 1e-5,
        "pick changed pose {before:?} vs {after:?}"
    );
    b3_destroy_world(world);
}

fn world_origin_anchor(body: crate::api::BodyId, origin_anchor: [f32; 3]) -> [f32; 3] {
    crate::api::b3_body_get_world_point(body, origin_anchor)
}

fn offset_dynamic(world: crate::api::WorldId, position: [f32; 3]) -> crate::api::BodyId {
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = position;
    let body = b3_create_body(world, &def);
    let mut hull = b3_make_box_hull(0.25, 0.5, 0.25);
    hull.center = [0.4, 0.0, 0.0];
    b3_create_hull_shape(body, &b3_default_shape_def(), &hull);
    b3_body_apply_mass_from_shapes(body);
    body
}

fn joint_anchor_error(a: crate::api::BodyId, b: crate::api::BodyId, anchor_a: [f32; 3], anchor_b: [f32; 3]) -> f32 {
    let pa = world_origin_anchor(a, anchor_a);
    let pb = world_origin_anchor(b, anchor_b);
    let d = [pa[0] - pb[0], pa[1] - pb[1], pa[2] - pb[2]];
    (d[0] * d[0] + d[1] * d[1] + d[2] * d[2]).sqrt()
}

#[test]
fn stored_joint_anchors_stay_origin_relative_after_mass() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    let mut ground_def = b3_default_body_def();
    ground_def.position = [0.0, 2.0, 0.0];
    let ground = b3_create_body(world, &ground_def);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 2.0, 0.0];
    let body = b3_create_body(world, &body_def);
    let mut joint = b3_default_revolute_joint_def();
    joint.body_a = ground;
    joint.body_b = body;
    joint.local_anchor_a = [0.0, 0.0, 0.0];
    joint.local_anchor_b = [0.0, 0.0, 0.0];
    b3_create_revolute_joint(world, &joint);
    let before = joint_anchor_error(ground, body, [0.0; 3], [0.0; 3]);
    let mut hull = b3_make_box_hull(0.5, 0.25, 0.25);
    hull.center = [0.6, 0.0, 0.0];
    b3_create_hull_shape(body, &b3_default_shape_def(), &hull);
    b3_body_apply_mass_from_shapes(body);
    let center = b3_body_get_local_center(body);
    assert!(center[0].abs() > 0.2, "expected offset COM {center:?}");
    let after = joint_anchor_error(ground, body, [0.0; 3], [0.0; 3]);
    assert!(
        after < 1e-4 && after <= before + 1e-4,
        "world anchors drifted after mass: before={before} after={after} center={center:?}"
    );
    b3_destroy_world(world);
}

fn offset_joint_case<F>(kind: &str, swapped: bool, build: F)
where
    F: FnOnce(crate::api::WorldId, crate::api::BodyId, crate::api::BodyId, [f32; 3], [f32; 3]),
{
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    let a = offset_dynamic(world, [-0.5, 3.0, 0.0]);
    let b = offset_dynamic(world, [0.5, 3.0, 0.0]);
    let anchor_a = [0.5, 0.0, 0.0];
    let anchor_b = [-0.5, 0.0, 0.0];
    if swapped {
        build(world, b, a, anchor_b, anchor_a);
    } else {
        build(world, a, b, anchor_a, anchor_b);
    }
    let initial = if swapped {
        joint_anchor_error(b, a, anchor_b, anchor_a)
    } else {
        joint_anchor_error(a, b, anchor_a, anchor_b)
    };
    assert!(initial < 0.05, "{kind} initial lever {initial}");
    b3_world_ensure_gpu(world);
    for _ in 0..60 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let pa = b3_body_get_position(a);
    let pb = b3_body_get_position(b);
    assert!(
        pa.iter().chain(pb.iter()).all(|x| x.is_finite()) && pa[1].abs() < 40.0 && pb[1].abs() < 40.0,
        "{kind} exploded a={pa:?} b={pb:?}"
    );
    b3_destroy_world(world);
}

#[test]
fn offset_com_joints_keep_world_anchors() {
    offset_joint_case("revolute", false, |world, a, b, aa, ab| {
        let mut joint = b3_default_revolute_joint_def();
        joint.body_a = a;
        joint.body_b = b;
        joint.local_anchor_a = aa;
        joint.local_anchor_b = ab;
        b3_create_revolute_joint(world, &joint);
    });
    offset_joint_case("revolute-swapped", true, |world, a, b, aa, ab| {
        let mut joint = b3_default_revolute_joint_def();
        joint.body_a = a;
        joint.body_b = b;
        joint.local_anchor_a = aa;
        joint.local_anchor_b = ab;
        b3_create_revolute_joint(world, &joint);
    });
    offset_joint_case("spherical", false, |world, a, b, aa, ab| {
        let mut joint = b3_default_spherical_joint_def();
        joint.body_a = a;
        joint.body_b = b;
        joint.local_anchor_a = aa;
        joint.local_anchor_b = ab;
        b3_create_spherical_joint(world, &joint);
    });
    offset_joint_case("weld", false, |world, a, b, aa, ab| {
        let mut joint = b3_default_weld_joint_def();
        joint.body_a = a;
        joint.body_b = b;
        joint.local_anchor_a = aa;
        joint.local_anchor_b = ab;
        b3_create_weld_joint(world, &joint);
    });
    offset_joint_case("distance", false, |world, a, b, aa, ab| {
        let mut joint = b3_default_distance_joint_def();
        joint.body_a = a;
        joint.body_b = b;
        joint.local_anchor_a = aa;
        joint.local_anchor_b = ab;
        joint.length = 1.0;
        b3_create_distance_joint(world, &joint);
    });
    offset_joint_case("prismatic", false, |world, a, b, aa, ab| {
        let mut joint = b3_default_prismatic_joint_def();
        joint.body_a = a;
        joint.body_b = b;
        joint.local_anchor_a = aa;
        joint.local_anchor_b = ab;
        joint.local_axis_a = [1.0, 0.0, 0.0];
        b3_create_prismatic_joint(world, &joint);
    });
    offset_joint_case("wheel", false, |world, a, b, aa, ab| {
        let mut joint = b3_default_wheel_joint_def();
        joint.body_a = a;
        joint.body_b = b;
        joint.local_anchor_a = aa;
        joint.local_anchor_b = ab;
        b3_create_wheel_joint(world, &joint);
    });
    offset_joint_case("motor", false, |world, a, b, aa, ab| {
        let mut joint = b3_default_motor_joint_def();
        joint.body_a = a;
        joint.body_b = b;
        joint.local_anchor_a = aa;
        joint.local_anchor_b = ab;
        b3_create_motor_joint(world, &joint);
    });
    offset_joint_case("parallel", false, |world, a, b, _aa, _ab| {
        let mut joint = b3_default_parallel_joint_def();
        joint.body_a = a;
        joint.body_b = b;
        b3_create_parallel_joint(world, &joint);
    });
}

#[test]
fn wheel_prepared_and_current_levers_agree_at_identity_dq() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    def.gravity = [0.0, 0.0, 0.0];
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    let a = offset_dynamic(world, [0.0, 2.0, 0.0]);
    let b = offset_dynamic(world, [1.0, 2.0, 0.0]);
    let mut joint = b3_default_wheel_joint_def();
    joint.body_a = a;
    joint.body_b = b;
    joint.local_anchor_a = [0.5, 0.0, 0.0];
    joint.local_anchor_b = [-0.5, 0.0, 0.0];
    b3_create_wheel_joint(world, &joint);
    let before = joint_anchor_error(a, b, [0.5, 0.0, 0.0], [-0.5, 0.0, 0.0]);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait_with_mirror(world);
    let after = joint_anchor_error(a, b, [0.5, 0.0, 0.0], [-0.5, 0.0, 0.0]);
    assert!(
        after < 0.08,
        "identity-dq wheel levers diverged before={before} after={after}"
    );
    b3_destroy_world(world);
}

fn contact_world_sep(contact: &crate::types::ContactGpu, pos_a: [f32; 3], pos_b: [f32; 3]) -> Vec<f32> {
    let ras = [contact.ra0, contact.ra1, contact.ra2, contact.ra3];
    let rbs = [contact.rb0, contact.rb1, contact.rb2, contact.rb3];
    let n = [contact.nx, contact.ny, contact.nz];
    (0..contact.count.min(4) as usize)
        .map(|i| {
            let packed = ras[i][3]
                + (rbs[i][0] - ras[i][0]) * n[0]
                + (rbs[i][1] - ras[i][1]) * n[1]
                + (rbs[i][2] - ras[i][2]) * n[2];
            let geometric = (pos_b[0] + rbs[i][0] - pos_a[0] - ras[i][0]) * n[0]
                + (pos_b[1] + rbs[i][1] - pos_a[1] - ras[i][1]) * n[1]
                + (pos_b[2] + rbs[i][2] - pos_a[2] - ras[i][2]) * n[2];
            (packed - geometric).abs()
        })
        .collect()
}

#[test]
fn offset_shape_contact_is_rebased_exactly_once() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    let world = b3_create_world(gpu, &wd);
    let ground = b3_create_body(world, &b3_default_body_def());
    let mut floor = b3_make_box_hull(8.0, 1.0, 8.0);
    floor.center = [0.0, -1.0, 0.0];
    b3_create_hull_shape(ground, &b3_default_shape_def(), &floor);
    let mut counterweight = floor;
    counterweight.center = [20.0, 1.0, 0.0];
    b3_create_hull_shape(ground, &b3_default_shape_def(), &counterweight);
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    bd.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &bd);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let c = contacts.iter().find(|c| c.a == 0 && c.b == 1 && c.count > 0).expect("floor contact");
    // Both anchors are the common world contact point on y=0. A separation
    // equality alone cannot detect a double shift of anchors AND packed base.
    for (ra, rb) in [c.ra0,c.ra1,c.ra2,c.ra3].iter().zip([c.rb0,c.rb1,c.rb2,c.rb3]).take(c.count as usize) {
        assert!(ra[1].abs() < 0.001, "floor anchor shifted twice: {ra:?}");
        assert!((0.5 + rb[1]).abs() < 0.001, "box anchor: {rb:?}");
    }
    b3_destroy_world(world);
}

#[test]
fn manifold_rebase_preserves_world_separation() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    b3_world_enable_sleeping(world, false);
    let mut ground_def = b3_default_body_def();
    ground_def.position = [0.0, -1.0, 0.0];
    let ground = b3_create_body(world, &ground_def);
    b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(8.0, 1.0, 8.0));
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &body_def);
    let cube = b3_make_box_hull(0.5, 0.5, 0.5);
    b3_create_hull_shape(body, &b3_default_shape_def(), &cube);
    let mut offset = cube;
    offset.center = [0.8, 0.0, 0.0];
    b3_create_hull_shape(body, &b3_default_shape_def(), &offset);
    b3_world_ensure_gpu(world);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait_with_mirror(world);
    let poses = [
        b3_body_get_position(ground),
        b3_body_get_position(body),
    ];
    let fresh = pollster::block_on(b3_world_sync_contacts(world));
    let live: Vec<_> = fresh
        .iter()
        .filter(|c| c.a != u32::MAX && c.count > 0)
        .collect();
    assert!(!live.is_empty(), "expected offset-compound contacts");
    for c in &live {
        let pa = poses.get(c.a as usize).copied().unwrap_or(poses[0]);
        let pb = poses.get(c.b as usize).copied().unwrap_or(poses[1]);
        for err in contact_world_sep(c, pa, pb) {
            assert!(err < 1e-4, "fresh packed vs geometric sep {err}");
        }
    }
    for _ in 0..8 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let recycled = pollster::block_on(b3_world_sync_contacts(world));
    let poses = [
        b3_body_get_position(ground),
        b3_body_get_position(body),
    ];
    for c in recycled.iter().filter(|c| c.a != u32::MAX && c.count > 0) {
        let pa = poses.get(c.a as usize).copied().unwrap_or(poses[0]);
        let pb = poses.get(c.b as usize).copied().unwrap_or(poses[1]);
        for err in contact_world_sep(c, pa, pb) {
            assert!(err < 2e-3, "recycled packed vs geometric sep {err}");
        }
    }
    b3_world_set_diagnostic_flags(world, DIAG_DISABLE_RECYCLING);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait_with_mirror(world);
    let forced = pollster::block_on(b3_world_sync_contacts(world));
    let poses = [
        b3_body_get_position(ground),
        b3_body_get_position(body),
    ];
    for c in forced.iter().filter(|c| c.a != u32::MAX && c.count > 0) {
        let pa = poses.get(c.a as usize).copied().unwrap_or(poses[0]);
        let pb = poses.get(c.b as usize).copied().unwrap_or(poses[1]);
        for err in contact_world_sep(c, pa, pb) {
            assert!(err < 1e-4, "force-fresh packed vs geometric sep {err}");
        }
    }
    let _ = SPECULATIVE_DISTANCE;
    b3_destroy_world(world);
}

fn prism_hull(extra_ring: bool, permute: bool) -> ConvexHull {
    let mut points = vec![
        [-0.4, -0.4, -0.4],
        [0.4, -0.4, -0.4],
        [0.4, 0.4, -0.4],
        [-0.4, 0.4, -0.4],
        [-0.4, -0.4, 0.4],
        [0.4, -0.4, 0.4],
        [0.4, 0.4, 0.4],
        [-0.4, 0.4, 0.4],
    ];
    if extra_ring {
        points.extend_from_slice(&[
            [0.0, -0.4, 0.0],
            [0.0, 0.4, 0.0],
            [0.55, 0.0, 0.0],
            [-0.55, 0.0, 0.0],
        ]);
    }
    if permute {
        points.reverse();
    }
    ConvexHull {
        points,
        planes: vec![
            [1.0, 0.0, 0.0, 0.55],
            [-1.0, 0.0, 0.0, 0.55],
            [0.0, 1.0, 0.0, 0.4],
            [0.0, -1.0, 0.0, 0.4],
            [0.0, 0.0, 1.0, 0.4],
            [0.0, 0.0, -1.0, 0.4],
        ],
        edge_directions: vec![[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]],
        half_edges: Vec::new(),
        half_extents: [0.55, 0.4, 0.4],
        aabb_center: [0.0; 3],
        center: [0.0; 3],
        inner_radius: 0.3,
        volume: 1.0,
        central_inertia: [1.0; 6],
    }
}

#[test]
fn mesh_contacts_use_all_hull_vertices_not_first_eight() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let run = |permute: bool| {
        let mut def = b3_default_world_def();
        def.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &def);
        b3_world_enable_sleeping(world, false);
        let ground = b3_create_body(world, &b3_default_body_def());
        let verts = [
            [-3.0, 0.0, -3.0],
            [3.0, 0.0, -3.0],
            [3.0, 0.0, 3.0],
            [-3.0, 0.0, 3.0],
        ];
        let tris = [[0u32, 2, 1], [0, 3, 2]];
        b3_create_mesh_shape(
            ground,
            &b3_default_shape_def(),
            &verts,
            &tris,
            &[],
            &[],
            &[] as &[MeshNode],
            [1.0, 1.0, 1.0],
        );
        let mut body_def = b3_default_body_def();
        body_def.body_type = BodyType::Dynamic;
        body_def.position = [0.0, 1.2, 0.0];
        let body = b3_create_body(world, &body_def);
        b3_create_convex_hull_shape(body, &b3_default_shape_def(), &prism_hull(true, permute));
        b3_world_ensure_gpu(world);
        for _ in 0..90 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
        }
        b3_world_gpu_wait_with_mirror(world);
        let p = b3_body_get_position(body);
        let contacts = pollster::block_on(b3_world_sync_contacts(world));
        let n = contacts.iter().filter(|c| c.a != u32::MAX && c.count > 0).count();
        b3_destroy_world(world);
        (p, n)
    };
    let (plain, n0) = run(false);
    let (perm, n1) = run(true);
    assert!(plain[1] > 0.2 && plain[1] < 1.2, "12-point hull fell {:?}", plain);
    assert!(perm[1] > 0.2 && perm[1] < 1.2, "permuted hull fell {:?}", perm);
    assert!(n0 > 0 && n1 > 0, "mesh hull contacts missing {n0} {n1}");
}

unsafe extern "C" fn sticky_friction(_a: f32, _ia: u64, _b: f32, _ib: u64) -> f32 {
    0.11
}

#[test]
fn box_mesh_support_inside_face_without_corner_hits() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let ground = b3_create_body(world, &b3_default_body_def());
    b3_create_mesh_shape(ground, &b3_default_shape_def(),
        &[[-0.2, 0.0, -0.2], [0.0, 0.0, 0.2], [0.2, 0.0, -0.2]],
        &[[0, 1, 2]], &[], &[], &[] as &[MeshNode], [1.0; 3]);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 0.51, 0.0];
    def.enable_sleep = false;
    def.motion_locks.angular_x = true;
    def.motion_locks.angular_y = true;
    def.motion_locks.angular_z = true;
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    for _ in 0..90 { b3_world_step_gpu(world, 1.0 / 60.0, 4); }
    b3_world_gpu_wait_with_mirror(world);
    let y = b3_body_get_position(body)[1];
    b3_destroy_world(world);
    assert!((y - 0.5).abs() < 0.02, "box missed interior mesh support: y={y}");
}

#[test]
fn tentative_mesh_edge_cannot_hide_confirmed_floor() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let ground = b3_create_body(world, &b3_default_body_def());
    // The first coplanar triangle ends just behind the box's rear side.
    // Its OBB reference face produces a shallow sideways candidate; the next
    // triangles contain the actual floor under the body. Force that order.
    b3_create_mesh_shape(ground, &b3_default_shape_def(),
        &[[-2.0,0.0,-2.0],[-2.0,0.0,-0.2],[2.0,0.0,-0.2],
          [-2.0,0.0,2.0],[2.0,0.0,2.0]],
        &[[0,1,2],[1,3,4],[1,4,2]], &[], &[],
        &[MeshNode { lower: [-2.0,0.0,-2.0], upper: [2.0,0.0,2.0], data: (3<<2)|3, triangle_offset: 0 }], [1.0;3]);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0,0.5,0.2032];
    def.motion_locks.angular_x = true;
    def.motion_locks.angular_y = true;
    def.motion_locks.angular_z = true;
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.4,0.5,0.4));
    b3_world_step_gpu(world, 1.0/60.0, 4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    // Executed Box3D fixture: c_abi/mesh_patch_normals_reference.cpp case 0.
    // CPU retains both the floor and the exposed triangle-side patch.
    let live: Vec<_> = contacts.iter().filter(|c| c.a != u32::MAX && c.count > 0).collect();
    assert_eq!(live.len(), 2, "expected CPU floor and side patches: {live:?}");
    assert!(live.iter().any(|c| c.ny > 0.99), "tentative edge hid floor: {live:?}");
    assert!(live.iter().any(|c| c.nz > 0.99), "missing CPU side patch: {live:?}");
    b3_destroy_world(world);
}

#[test]
fn rejected_mesh_wall_cannot_replace_floor_manifold_normal() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let ground = b3_create_body(world, &b3_default_body_def());
    // One leaf fixes visitation order: the floor fills the manifold before a
    // deeper side-facing triangle below its surface is considered.
    b3_create_mesh_shape(ground, &b3_default_shape_def(),
        &[[-2.0,0.0,-2.0],[-2.0,0.0,2.0],[2.0,0.0,2.0],[2.0,0.0,-2.0],
          [0.25,-1.0,-2.0],[0.25,0.0,2.0],[0.25,0.0,-2.0]],
        &[[0,1,2],[0,2,3],[4,5,6]], &[], &[],
        &[MeshNode { lower: [-2.0,-1.0,-2.0], upper: [2.0,0.0,2.0], data: (3<<2)|3, triangle_offset: 0 }],
        [1.0; 3]);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 0.49, 0.0];
    def.enable_sleep = false;
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let live: Vec<_> = contacts.iter().filter(|c| c.a != u32::MAX && c.count > 0).collect();
    // Box3D retains two clusters: triangle normals differ even though both
    // collision normals point up. See mesh_patch_normals_reference.cpp case 1.
    assert_eq!(live.len(), 2, "expected two CPU clusters: {live:?}");
    assert!(live.iter().all(|c| c.ny > 0.99), "wall changed floor basis: {live:?}");
    b3_destroy_world(world);
}

#[test]
fn box_spanning_chamfer_uses_box_face_axis() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let ground = b3_create_body(world, &b3_default_body_def());
    b3_create_mesh_shape(ground, &b3_default_shape_def(),
        &[[-0.5,-0.15,-1.0],[-0.25,0.0,1.0],[-0.25,0.0,-1.0]],
        &[[0,1,2]], &[], &[], &[] as &[MeshNode], [1.0;3]);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0,0.49,0.0];
    def.enable_sleep = false;
    def.motion_locks.angular_x = true;
    def.motion_locks.angular_y = true;
    def.motion_locks.angular_z = true;
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5,0.5,0.5));
    b3_body_set_linear_velocity(body, [9.0,0.0,0.0]);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let contact = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0).expect("chamfer contact");
    assert!(contact.ny > 0.99, "triangle face incorrectly chosen over box face: {contact:?}");
    assert!(b3_body_get_linear_velocity(body)[1] < 0.1);
    b3_destroy_world(world);
}

#[test]
fn later_mesh_bvh_offsets_are_nodes_not_packed_vectors() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let mut dynamic = Vec::new();
    for x in [-5.0, 5.0, 15.0] {
        let ground = b3_create_body(world, &b3_default_body_def());
        let vertices = [[x-2.0,0.0,-2.0],[x-2.0,0.0,2.0],[x+2.0,0.0,2.0],[x+2.0,0.0,-2.0]];
        b3_create_mesh_shape(ground, &b3_default_shape_def(), &vertices,
            &[[0,1,2],[0,2,3]], &[], &[], &[] as &[MeshNode], [1.0;3]);
        let mut def = b3_default_body_def();
        def.body_type = BodyType::Dynamic;
        def.position = [x,0.6,0.0];
        let body = b3_create_body(world, &def);
        b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5,0.5,0.5));
        dynamic.push(body);
    }
    for _ in 0..90 { b3_world_step_gpu(world, 1.0/60.0, 4); }
    b3_world_gpu_wait_with_mirror(world);
    for body in dynamic {
        let y = b3_body_get_position(body)[1];
        assert!((y - 0.5).abs() < 0.02, "mesh for body {} lost support: {y}", body.index1);
    }
    b3_destroy_world(world);
}

#[test]
fn health_scan_uses_body_origin_for_offset_mass() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [0.0, 2.0, 0.0];
    let body = b3_create_body(world, &def);
    b3_create_capsule_shape(body, &b3_default_shape_def(), &Capsule {
        center1: [0.0, 40.0, 0.0], center2: [0.0, 42.0, 0.0], radius: 0.5,
    });
    let health = crate::api::b3_world_health_scan(world);
    assert!((health.min_y - 2.0).abs() < 1e-5, "COM leaked into health: {health:?}");
    assert!((health.max_y - b3_body_get_position(body)[1]).abs() < 1e-5);
    b3_destroy_world(world);
}

#[test]
fn material_callback_capacity_invalidates_existing_world() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    // 90 fits the count bound but deterministically exceeds 32 probes at pair
    // (43,43); 91 exceeds the count bound. Neither may silently restore defaults.
    for count in [90, 91] {
    let world = b3_create_world(gpu.clone(), &b3_default_world_def());
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    for _ in 1..count {
        b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    }
    crate::api::b3_world_set_friction_callback(world, Some(sticky_friction));
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    assert!(crate::api::b3_world_physics_invalid(world));
    assert!(!crate::api::b3_world_gpu_fail(world).is_null());
    b3_destroy_world(world);
    }
}

unsafe extern "C" fn sticky_restitution(_a: f32, _ia: u64, _b: f32, _ib: u64) -> f32 {
    0.42
}

#[test]
fn linear_materials_use_geometric_mix_and_bounded_callbacks() {
    assert!(crate::api::callback_mix_pairs_supported(64));
    assert!(crate::api::callback_mix_pairs_supported(90));
    assert!(!crate::api::callback_mix_pairs_supported(91));
    assert!(!crate::api::callback_mix_pairs_supported(52_500));
    let geometric = crate::api::mix_friction(None, 0.4, 1, 0.9, 2);
    assert!((geometric - (0.4 * 0.9_f32).sqrt()).abs() < 1e-6);
    let custom = crate::api::mix_friction(Some(sticky_friction), 0.4, 11, 0.9, 22);
    assert!((custom - 0.11).abs() < 1e-6);
    let rest = crate::api::mix_restitution(Some(sticky_restitution), 0.0, 1, 0.2, 2);
    assert!((rest - 0.42).abs() < 1e-6);
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    crate::api::b3_world_set_friction_callback(world, Some(sticky_friction));
    crate::api::b3_world_set_restitution_callback(world, Some(sticky_restitution));
    let ground = b3_create_body(world, &b3_default_body_def());
    let mut mat = b3_default_shape_def();
    mat.friction = 0.4;
    mat.restitution = 0.1;
    mat.user_material_id = 7;
    b3_create_hull_shape(ground, &mat, &b3_make_box_hull(4.0, 0.5, 4.0));
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 1.0, 0.0];
    let body = b3_create_body(world, &body_def);
    let mut box_def = b3_default_shape_def();
    box_def.friction = 0.9;
    box_def.user_material_id = 8;
    b3_create_hull_shape(body, &box_def, &b3_make_box_hull(0.5, 0.5, 0.5));
    b3_world_ensure_gpu(world);
    for _ in 0..20 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
    }
    b3_world_gpu_wait_with_mirror(world);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let live: Vec<_> = contacts
        .iter()
        .filter(|c| c.a != u32::MAX && c.count > 0)
        .collect();
    assert!(!live.is_empty());
    assert!(
        live.iter().any(|c| (c.friction - 0.11).abs() < 0.08),
        "contact friction {:?} expected host callback 0.11",
        live.iter().map(|c| c.friction).collect::<Vec<_>>()
    );
    let health = crate::api::b3_world_health_scan(world);
    assert!(health.dynamic_count >= 1);
    assert_eq!(health.nan_count, 0);
    b3_destroy_world(world);
}

#[test]
fn mesh_face_retains_separated_points_for_rotating_impact() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.enable_sleep = false;
    let world = b3_create_world(gpu, &wd);
    let ground = b3_create_body(world, &b3_default_body_def());
    b3_create_mesh_shape(ground, &b3_default_shape_def(),
        &[[-4.0,0.279481441,-9.0],[-4.0,0.451831192,-8.0],
          [-3.0,0.279481351,-8.0],[-3.0,0.172873959,-9.0]],
        &[[0,1,2],[2,3,0]], &[], &[], &[] as &[MeshNode], [1.0;3]);
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    bd.position = [-3.53273058,0.575324297,-8.47112465];
    bd.rotation = [-0.576700628,-0.544579446,-0.599283516,0.10820777];
    bd.linear_velocity = [0.509750605,-9.38941956,-0.513962209];
    bd.angular_velocity = [-1.19869685,1.00277746,-3.39283967];
    let body = b3_create_body(world, &bd);
    let mut shape_def = b3_default_shape_def();
    shape_def.rolling_resistance = 0.1;
    b3_create_hull_shape(body, &shape_def, &b3_make_box_hull(0.02,0.2,0.04));
    for _ in 0..2 {
        b3_world_step_gpu(world, 1.0/60.0, 4);
        b3_world_gpu_wait_with_mirror(world);
    }
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let live = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0).expect("impact contact");
    let v = b3_body_get_linear_velocity(body);
    let w = b3_body_get_angular_velocity(body);
    b3_destroy_world(world);
    // Real Box3D retains the full face even though only one point starts
    // within 0.02 m. Its other vertices engage during this rotating impact.
    assert_eq!(live.count, 4, "clipped face was reduced to its nearest vertex");
    assert!(v[1] > -3.0 && w[2] < 0.0, "wrong impact response: v={v:?} w={w:?}");
}

#[test]
fn mesh_drop_thin_box_contact_at_captured_pose() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let ground = b3_create_body(world, &b3_default_body_def());
    b3_create_mesh_shape(ground, &b3_default_shape_def(),
        &[[-4.0,0.279481441,-9.0],[-4.0,0.451831192,-8.0],
          [-3.0,0.279481351,-8.0],[-3.0,0.172873959,-9.0]],
        &[[0,1,2],[2,3,0]], &[], &[], &[] as &[MeshNode], [1.0;3]);
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    def.position = [-3.55281305,0.294953436,-8.53022671];
    def.rotation = [-0.504609466,-0.783914804,-0.358624011,0.0472847596];
    def.enable_sleep = false;
    let body = b3_create_body(world, &def);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.02,0.2,0.04));
    b3_world_step_gpu(world, 1.0/60.0, 4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let live: Vec<_> = contacts.iter().filter(|c| c.a != u32::MAX && c.count > 0).collect();
    assert!(!live.is_empty(), "captured overlapping thin box has no mesh support");
    assert!(live.iter().any(|c| c.ny > 0.5), "no upward contact: {live:?}");
    b3_destroy_world(world);
}

#[test]
fn body_contact_recycling_setting_reaches_gpu_for_either_endpoint() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    assert!(b3_default_body_def().enable_contact_recycling);
    for via_abi in [false, true] {
    for disabled in [None, Some(0), Some(1)] {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0; 3];
        wd.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &wd);
        for endpoint in 0..2 {
            let mut bd = b3_default_body_def();
            bd.body_type = if endpoint == 0 { BodyType::Static } else { BodyType::Dynamic };
            bd.position = [endpoint as f32 * 2.0, 0.0, 0.0];
            bd.enable_sleep = false;
            bd.enable_contact_recycling = disabled != Some(endpoint);
            let body = if via_abi {
                crate::c_abi::gpu_b3_create_body(world,
                    if endpoint == 0 { 0 } else { 2 }, bd.position[0], 0.0, 0.0,
                    0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0,
                    if bd.enable_contact_recycling { 0 } else { crate::types::FLAG_DISABLE_CONTACT_RECYCLING })
            } else { b3_create_body(world, &bd) };
            b3_create_sphere_shape(body, &b3_default_shape_def(), &Sphere { center: [0.0;3], radius: 1.0 });
        }
        let mut recycled = false;
        for _ in 0..4 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            let contacts = pollster::block_on(b3_world_sync_contacts(world));
            let contact = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0).expect("sphere contact");
            recycled |= contact.lifecycle[1] & 4 != 0;
        }
        b3_destroy_world(world);
        assert_eq!(recycled, disabled.is_none(), "recycling setting ignored for {disabled:?}");
    }
    }
}

#[test]
fn revolute_substep_warm_start_matches_cpu() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    wd.enable_sleep = false;
    wd.enable_continuous = false;
    let world = b3_create_world(gpu, &wd);
    let ground = b3_create_body(world, &b3_default_body_def());
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    bd.rotation = [0.0998334166, 0.0, 0.0, 0.995004177];
    bd.angular_velocity = [0.2, 0.7, 5.0];
    let body = b3_create_body(world, &bd);
    b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.2, 0.3, 0.4));
    let mut jd = b3_default_revolute_joint_def();
    jd.body_a = ground;
    jd.body_b = body;
    b3_create_revolute_joint(world, &jd);
    // Independent Box3D oracle: rotating anisotropic box with initial hinge tilt.
    // Later substeps must warm start on the last solve's perpendicular axes.
    let expected = [
        [0.0310692787, 0.000106125604, 0.041259259, 0.998665333, 0.00681114197, -0.152240261, 4.90252972],
        [0.00966917723, 0.0000330051407, 0.0820644796, 0.996580184, 0.00404787064, -0.0471853167, 4.89763641],
        [0.0030093519, 0.0000102607291, 0.122665122, 0.992443562, 0.00185754895, -0.0146196187, 4.8970685],
    ];
    for (step, reference) in expected.iter().enumerate() {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
        b3_world_gpu_wait_with_mirror(world);
        let q = b3_body_get_rotation(body);
        let omega = b3_body_get_angular_velocity(body);
        for (component, (&actual, &target)) in q.iter().chain(omega.iter()).zip(reference).enumerate() {
            assert!((actual - target).abs() < 1e-4,
                "step {} component {component}: {actual} vs {target}", step + 1);
        }
    }
    b3_destroy_world(world);
}

#[test]
fn rotated_revolute_frame_corrects_tilt_instead_of_amplifying_it() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    for yaw in [0.0_f32, 0.7, std::f32::consts::FRAC_PI_2] {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0;3];
        wd.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &wd);
        let ground = b3_create_body(world, &b3_default_body_def());
        let frame = glam::Quat::from_rotation_z(yaw);
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.enable_sleep = false;
        bd.rotation = (frame * glam::Quat::from_rotation_x(0.2)).to_array();
        let body = b3_create_body(world, &bd);
        b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5,0.5,0.5));
        let mut jd = b3_default_revolute_joint_def();
        jd.body_a = ground;
        jd.body_b = body;
        jd.local_rotation_a = frame.to_array();
        b3_create_revolute_joint(world, &jd);
        for _ in 0..30 { b3_world_step_gpu(world, 1.0/60.0, 4); }
        b3_world_gpu_wait_with_mirror(world);
        let q = glam::Quat::from_array(b3_body_get_rotation(body));
        let tilt = (q * glam::Vec3::Z).z.clamp(-1.0, 1.0).acos();
        let relative = frame.conjugate() * q;
        let twist = 2.0 * relative.z.atan2(relative.w).abs();
        eprintln!("yaw={yaw} tilt={tilt} twist={twist} q={q:?}");
        b3_destroy_world(world);
        assert!(tilt < 0.01, "revolute frame yaw={yaw} amplified/retained initial 0.2rad tilt: {tilt}");
        assert!(twist < 0.01, "tilt correction injected free-axis rotation at yaw={yaw}: {twist}");
    }
}

#[test]
fn runtime_recycling_toggle_recomputes_existing_contacts() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    for endpoint in 0..2 {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0;3];
        wd.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &wd);
        let mut bodies = Vec::new();
        for i in 0..2 {
            let mut bd = b3_default_body_def();
            bd.body_type = if i == 0 { BodyType::Static } else { BodyType::Dynamic };
            bd.position = [i as f32 * 2.0, 0.0, 0.0];
            bd.enable_sleep = false;
            let body = b3_create_body(world, &bd);
            b3_create_sphere_shape(body, &b3_default_shape_def(), &Sphere { center: [0.0;3], radius: 1.0 });
            bodies.push(body);
        }
        let body = bodies[endpoint];
        for enabled in [true, false, true, false, true] {
            // Toggle after submitting: the setter must not restore an older pose.
            b3_world_step_gpu(world, 1.0/60.0, 4);
            crate::c_abi::gpu_b3_body_enable_contact_recycling(body, enabled);
            assert_eq!(crate::c_abi::gpu_b3_body_is_contact_recycling_enabled(body), enabled);
            for _ in 0..3 {
                b3_world_step_gpu(world, 1.0/60.0, 4);
                let contacts = pollster::block_on(b3_world_sync_contacts(world));
                let c = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0).expect("existing contact");
                assert_eq!(c.lifecycle[1] & 4 != 0, enabled, "endpoint={endpoint}, enabled={enabled}");
            }
            b3_world_prepare_pose_snapshot(world);
            assert_eq!(crate::c_abi::gpu_b3_body_is_contact_recycling_enabled(body), enabled, "stale snapshot restored flag");
            assert!(b3_body_get_position(bodies[1])[0] > 1.99);
        }
        b3_destroy_world(world);
    }
}

#[test]
fn fast_mesh_refresh_preserves_slow_and_nonmesh_recycling() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    for compound in [false, true] {
    for (mesh, continuous, speed) in [(true, true, 0.2), (true, false, 0.2),
                                     (true, true, 0.0), (false, true, 0.2)] {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0; 3];
        wd.enable_sleep = false;
        wd.enable_continuous = continuous;
        let world = b3_create_world(gpu.clone(), &wd);
        let ground = b3_create_body(world, &b3_default_body_def());
        let mut sd = b3_default_shape_def();
        sd.friction = 0.0;
        if mesh {
            b3_create_mesh_shape(ground, &sd,
                &[[-10.0,0.0,-10.0],[-10.0,0.0,10.0],[10.0,0.0,10.0],[10.0,0.0,-10.0]],
                &[[0,1,2],[2,3,0]], &[], &[], &[] as &[MeshNode], [1.0;3]);
        } else {
            b3_create_hull_shape(ground, &sd, &b3_make_box_hull(10.0,0.5,10.0));
        }
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        // Keep the same positive 0.1 mm solver gap in both cases. Native mesh
        // rest offset is one slop; placing the tiny sphere at 0.0051 would now
        // start it deeply inside that offset and accelerate the "slow" control.
        bd.position = [0.0, if mesh { 0.0051 + crate::types::LINEAR_SLOP } else { 0.5051 }, 0.0];
        bd.linear_velocity = [speed,0.0,0.0];
        let body = b3_create_body(world, &bd);
        if compound {
            // A larger first child must not hide the thin support child's min extent.
            b3_create_sphere_shape(body, &sd, &Sphere { center: [0.0,2.0,0.0], radius: 0.5 });
        }
        b3_create_sphere_shape(body, &sd, &Sphere { center: [0.0;3], radius: 0.005 });
        for step in 0..2 {
            b3_world_step_gpu(world, 1.0/60.0, 4);
            let contacts = pollster::block_on(b3_world_sync_contacts(world));
            let c = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0).expect("support contact");
            if step == 1 {
                let expected = !(mesh && continuous && speed > 0.0);
                assert_eq!(c.lifecycle[1] & 4 != 0, expected,
                    "compound={compound} mesh={mesh} continuous={continuous} speed={speed}");
            }
        }
        // Turning continuous off must immediately stop applying the veto,
        // even though the preceding completed state still carried FAST.
        b3_world_enable_continuous(world, false);
        b3_world_step_gpu(world, 1.0/60.0, 4);
        let contacts = pollster::block_on(b3_world_sync_contacts(world));
        assert!(contacts.iter().any(|c| c.a != u32::MAX && c.count > 0 && c.lifecycle[1] & 4 != 0));
        b3_destroy_world(world);
    }
    }
}

#[test]
fn contact_hit_identity_survives_a_body_slot_hole() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    for sphere in [false, true] {
    let mut outcomes = Vec::new();
    for hole in [false, true] {
        let mut wd = b3_default_world_def();
        wd.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &wd);
        let removed = if hole { Some(b3_create_body(world, &b3_default_body_def())) } else { None };
        let ground = b3_create_body(world, &b3_default_body_def());
        let ground_shape = b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(8.0,0.5,8.0));
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.position = [0.0,1.05,0.0];
        bd.linear_velocity = [0.0,-5.0,0.0];
        let body = b3_create_body(world, &bd);
        let mut sd = b3_default_shape_def();
        sd.enable_hit_events = true;
        sd.enable_contact_events = true;
        let shape = if sphere {
            b3_create_sphere_shape(body, &sd, &Sphere { center:[0.0;3], radius:0.5 })
        } else {
            b3_create_hull_shape(body, &sd, &b3_make_box_hull(0.5,0.5,0.5))
        };
        let (expected_a, expected_b) = (ground_shape, shape); // CPU contact.c: hull is primary.
        if let Some(removed) = removed { b3_destroy_body(removed); }
        let mut hits = Vec::new();
        for _ in 0..8 {
            b3_world_step_gpu(world,1.0/60.0,4);
            b3_world_gpu_wait_with_mirror(world);
            let (begin, begin_count, _, _, hit, hit_count) = b3_world_contact_event_ptrs(world);
            for e in unsafe { std::slice::from_raw_parts(begin, begin_count as usize) } {
                assert_eq!(e.shape_id_a, expected_a);
                assert_eq!(e.shape_id_b, expected_b);
            }
            for e in unsafe { std::slice::from_raw_parts(hit, hit_count as usize) } {
                assert_eq!(e.shape_id_a, expected_a);
                assert_eq!(e.shape_id_b, expected_b);
                assert!(e.normal[1] > 0.99);
                hits.push((e.point,e.approach_speed));
            }
        }
        b3_destroy_world(world);
        outcomes.push(hits);
    }
    assert!(!outcomes[0].is_empty(), "fixture did not produce a hit");
    assert_eq!(outcomes[0].len(), outcomes[1].len(), "slot hole lost hits");
    for (a,b) in outcomes[0].iter().zip(&outcomes[1]) {
        for axis in 0..3 { assert!((a.0[axis]-b.0[axis]).abs()<1e-5); }
        assert!((a.1-b.1).abs()<1e-5);
    }
    }
}

#[test]
fn mesh_hull_backside_uses_cpu_linear_slop() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    for (height, should_touch) in [(-0.002, true), (-0.006, false)] {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0;3];
        wd.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &wd);
        let ground = b3_create_body(world, &b3_default_body_def());
        b3_create_mesh_shape(ground, &b3_default_shape_def(),
            &[[-5.0,0.0,-5.0],[-5.0,0.0,5.0],[5.0,0.0,5.0],[5.0,0.0,-5.0]],
            &[[0,1,2],[2,3,0]], &[], &[], &[] as &[MeshNode], [1.0;3]);
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.position = [0.0,height,0.0];
        let body = b3_create_body(world, &bd);
        b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.1,0.02,0.1));
        b3_world_step_gpu(world, 1.0/60.0, 4);
        let contacts = pollster::block_on(b3_world_sync_contacts(world));
        let touching = contacts.iter().any(|c| c.a != u32::MAX && c.count>0 && c.ny>0.99);
        b3_destroy_world(world);
        assert_eq!(touching, should_touch, "hull center y={height}");
    }
}

#[test]
fn compound_events_and_callbacks_use_public_shapes_with_slot_holes() {
    use crate::api::{b3_create_compound_parent, b3_shape_attach_compound_child, b3_world_set_pre_solve_callback};
    unsafe extern "C" fn accept(a: ShapeId, b: ShapeId, context: *mut std::ffi::c_void) -> bool {
        unsafe { &mut *(context as *mut Vec<(ShapeId, ShapeId)>) }.push((a,b));
        true
    }
    unsafe extern "C" fn pre_accept(a: ShapeId, b: ShapeId, _: crate::api::Vec3, _: crate::api::Vec3, context: *mut std::ffi::c_void) -> bool {
        unsafe { &mut *(context as *mut Vec<(ShapeId, ShapeId)>) }.push((a,b));
        true
    }
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let removed = b3_create_body(world, &b3_default_body_def());
    let ground = b3_create_body(world, &b3_default_body_def());
    let mut sd = b3_default_shape_def();
    sd.enable_contact_events = true;
    sd.enable_custom_filtering = true;
    sd.enable_pre_solve_events = true;
    let parent = b3_create_compound_parent(ground, &sd);
    assert_ne!(parent.index1, 0);
    let mut children = Vec::new();
    for _ in 0..2 {
        let child = b3_create_hull_shape(ground, &sd, &b3_make_box_hull(10.0,0.5,10.0));
        assert!(b3_shape_attach_compound_child(parent,child));
        children.push(child);
    }
    let mut shapes = Vec::new();
    for x in [0.0,4.0] {
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.position = [x,1.0,0.0];
        let body = b3_create_body(world,&bd);
        shapes.push(b3_create_hull_shape(body,&sd,&b3_make_box_hull(0.5,0.5,0.5)));
    }
    b3_destroy_body(removed);
    let mut callbacks: Vec<(ShapeId,ShapeId)> = Vec::new();
    b3_world_set_custom_filter_callback(world,Some(accept), &mut callbacks as *mut _ as *mut std::ffi::c_void);
    b3_world_step_gpu(world,1.0/60.0,4);
    b3_world_gpu_wait_with_mirror(world);
    assert_eq!(callbacks.len(),4,"one custom-filter decision per native child contact");
    for &(a,b) in &callbacks {
        assert!((a==parent && shapes.contains(&b)) || (b==parent && shapes.contains(&a)), "hidden/wrong callback shapes {a:?} {b:?}");
    }
    let (begin,count,_,end_count,_,_) = b3_world_contact_event_ptrs(world);
    assert_eq!(count,4,"two child contacts per public shape pair, matching CPU");
    assert_eq!(end_count,0);
    let first = unsafe {std::slice::from_raw_parts(begin,count as usize)}.to_vec();
    for e in &first {
        assert_eq!(e.shape_id_a,parent);
        assert!(shapes.contains(&e.shape_id_b));
    }
    assert_ne!(first[0].contact_id,first[1].contact_id);
    let mut contact_data = [crate::api::ContactData::default(); 4];
    assert_eq!(crate::api::b3_shape_get_contact_data(parent, &mut contact_data), 4);
    for data in &contact_data[..4] {
        assert_eq!(data.manifold_count, 1, "native child contacts must remain separate entries");
        assert!(first.iter().any(|e| e.contact_id == data.contact_id));
        assert!(crate::api::b3_contact_is_valid(data.contact_id));
    }

    let mut pre_callbacks: Vec<(ShapeId,ShapeId)> = Vec::new();
    b3_world_set_pre_solve_callback(world,Some(pre_accept), &mut pre_callbacks as *mut _ as *mut std::ffi::c_void);
    b3_destroy_shape(children[0],false);
    b3_world_step_gpu(world,1.0/60.0,4);
    b3_world_gpu_wait_with_mirror(world);
    let (_,begin_count,_,end_count,_,_) = b3_world_contact_event_ptrs(world);
    assert_eq!(begin_count,0,"remaining child must preserve public contact");
    assert_eq!(end_count,2,"removing a child ends its two native contacts");
    assert_eq!(crate::api::b3_shape_get_contact_data(parent, &mut contact_data), 2);
    for data in &contact_data[..2] {
        assert_eq!(data.manifold_count, 1);
        assert!(first.iter().any(|e| e.contact_id == data.contact_id), "surviving public pair changed ID");
    }

    assert_eq!(pre_callbacks.len(),2,"one pre-solve decision per public pair");
    for &(a,b) in &pre_callbacks {
        assert!((a==parent && shapes.contains(&b)) || (b==parent && shapes.contains(&a)));
    }
    b3_world_set_custom_filter_callback(world,None,std::ptr::null_mut());
    b3_world_step_gpu(world,1.0/60.0,4);
    b3_world_gpu_wait_with_mirror(world);
    let (_,begin_count,_,end_count,_,_) = b3_world_contact_event_ptrs(world);
    assert_eq!((begin_count,end_count),(0,0));
    b3_destroy_shape(children[1],false);
    b3_world_step_gpu(world,1.0/60.0,4);
    b3_world_gpu_wait_with_mirror(world);
    let (_,begin_count,end,end_count,_,_) = b3_world_contact_event_ptrs(world);
    assert_eq!((begin_count,end_count),(0,2), "last child ends each public pair once");
    for e in unsafe {std::slice::from_raw_parts(end,end_count as usize)} {
        assert!(first.iter().any(|b| b.contact_id==e.contact_id && b.shape_id_a==e.shape_id_a && b.shape_id_b==e.shape_id_b));
    }
    b3_destroy_world(world);
}

#[test]
fn pre_solve_veto_disables_all_mesh_patches_and_can_reenable_at_rest() {
    use crate::api::b3_world_set_pre_solve_callback;
    struct Toggle { accept: bool, calls: usize, locked_capacity: i32 }
    unsafe extern "C" fn callback(a: ShapeId, _: ShapeId, _: crate::api::Vec3, _: crate::api::Vec3, raw: *mut std::ffi::c_void) -> bool {
        let state = unsafe { &mut *(raw as *mut Toggle) };
        state.locked_capacity = crate::api::b3_shape_get_contact_capacity(a);
        state.calls += 1;
        state.accept
    }
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0;3];
    wd.enable_continuous = false;
    wd.enable_sleep = false;
    let world = b3_create_world(gpu,&wd);
    let ground = b3_create_body(world,&b3_default_body_def());
    let mut sd = b3_default_shape_def();
    sd.enable_pre_solve_events = true;
    sd.enable_contact_events = true;
    b3_create_mesh_shape(ground,&sd,
        &[[-2.0,0.0,-2.0],[-2.0,0.0,2.0],[2.0,0.0,2.0],[2.0,0.0,-2.0],
          [0.25,-1.0,-2.0],[0.25,0.0,2.0],[0.25,0.0,-2.0]],
        &[[0,1,2],[0,2,3],[4,5,6]],&[],&[],
        &[MeshNode {lower:[-2.0,-1.0,-2.0],upper:[2.0,0.0,2.0],data:(3<<2)|3,triangle_offset:0}],[1.0;3]);
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    bd.position = [0.0,0.49,0.0];
    let body = b3_create_body(world,&bd);
    b3_create_hull_shape(body,&sd,&b3_make_box_hull(0.5,0.5,0.5));
    let mut toggle = Toggle {accept:false,calls:0,locked_capacity:-1};
    b3_world_set_pre_solve_callback(world,Some(callback),&mut toggle as *mut _ as *mut std::ffi::c_void);
    b3_world_step_gpu(world,1.0/60.0,4);
    b3_world_gpu_wait_with_mirror(world);
    assert_eq!(toggle.calls,1);
    assert_eq!(toggle.locked_capacity,0,"contact query must not deadlock in pre-solve");
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    assert!(contacts.iter().all(|c| c.count==0),"veto left live child manifolds");
    let (_,begin,_,end,_,hit)=b3_world_contact_event_ptrs(world);
    assert_eq!((begin,end,hit),(0,0,0));
    assert_eq!(b3_body_get_position(body),bd.position);
    toggle.accept = true;
    b3_world_step_gpu(world,1.0/60.0,4);
    b3_world_gpu_wait_with_mirror(world);
    assert_eq!(toggle.calls,2,"veto was incorrectly recycled as a geometric miss");
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    assert_eq!(contacts.iter().filter(|c|c.a!=u32::MAX && c.count>0).count(),2);
    let (_,begin,_,end,_,_)=b3_world_contact_event_ptrs(world);
    assert_eq!((begin,end),(1,0));
    let mut data = [crate::api::ContactData::default(); 2];
    assert_eq!(crate::api::b3_body_get_contact_data(body, &mut data), 1);
    assert_eq!(data[0].manifold_count, 2, "mesh patches must remain in one public pair");
    let manifolds = unsafe { std::slice::from_raw_parts(data[0].manifolds, 2) };
    assert!(manifolds.iter().all(|m| m.point_count > 0));
    let mut actual_normals: Vec<_> = manifolds.iter().map(|m| m.normal).collect();
    let mut expected_normals: Vec<_> = contacts.iter().filter(|c|c.a!=u32::MAX && c.count>0)
        .map(|c| [c.nx,c.ny,c.nz]).collect();
    let order = |a: &[f32;3], b: &[f32;3]| a[0].total_cmp(&b[0]).then(a[1].total_cmp(&b[1])).then(a[2].total_cmp(&b[2]));
    actual_normals.sort_by(order); expected_normals.sort_by(order);
    assert_eq!(actual_normals, expected_normals, "API lost or replaced a mesh patch");
    let (begin_ptr, _, _, _, _, _) = b3_world_contact_event_ptrs(world);
    assert_eq!(unsafe { (*begin_ptr).contact_id }, data[0].contact_id);
    let stats = pollster::block_on(b3_world_live_step_stats(world)).expect("stats");
    assert_eq!(stats.first_fail_step,0);
    b3_destroy_world(world);
}

#[test]
fn late_event_read_does_not_invent_a_new_contact_begin() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def(); wd.gravity=[0.0;3];
    let world=b3_create_world(gpu,&wd);
    let ground=b3_create_body(world,&b3_default_body_def());
    let mut sd=b3_default_shape_def(); sd.enable_contact_events=true;
    b3_create_hull_shape(ground,&sd,&b3_make_box_hull(4.0,0.5,4.0));
    let mut bd=b3_default_body_def(); bd.body_type=BodyType::Dynamic; bd.position=[0.0,1.0,0.0];
    let body=b3_create_body(world,&bd);
    b3_create_hull_shape(body,&sd,&b3_make_box_hull(0.5,0.5,0.5));
    // Do not harvest the first step's events. CPU event buffers describe only
    // the latest step; a persistent contact is not a new begin in step two.
    b3_world_step_gpu(world,1.0/60.0,4);
    b3_world_step_gpu(world,1.0/60.0,4);
    b3_world_gpu_wait_with_mirror(world);
    let (_,begin,_,end,_,_)=b3_world_contact_event_ptrs(world);
    assert_eq!((begin,end),(0,0));
    assert!(pollster::block_on(b3_world_sync_contacts(world)).iter().any(|c|c.count>0));
    b3_destroy_world(world);
}

#[test]
fn point_persistence_survives_publication_recycling_and_zero_impulses() {
    use crate::api::b3_body_enable_contact_recycling;
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def(); wd.gravity=[0.0;3]; wd.enable_sleep=false;
    let world=b3_create_world(gpu,&wd);
    let ground=b3_create_body(world,&b3_default_body_def());
    let sd=b3_default_shape_def();
    b3_create_hull_shape(ground,&sd,&b3_make_box_hull(4.0,0.5,4.0));
    let mut bd=b3_default_body_def(); bd.body_type=BodyType::Dynamic; bd.position=[0.0,1.0,0.0];
    let body=b3_create_body(world,&bd);
    b3_create_hull_shape(body,&sd,&b3_make_box_hull(0.5,0.5,0.5));
    for step in 0..3 {
        if step==2 { b3_body_enable_contact_recycling(body,false); }
        b3_world_step_gpu(world,1.0/60.0,4);
        b3_world_gpu_wait_with_mirror(world);
        let contacts=pollster::block_on(b3_world_sync_contacts(world));
        let c=contacts.iter().find(|c|c.a!=u32::MAX && c.count>0).expect("contact");
        assert_eq!(c.count,4);
        assert_eq!([c.rb0[3],c.rb1[3],c.rb2[3],c.rb3[3]],[0.0;4]);
        assert!((0..4).all(|i|c.point_persisted(i)==(step>0)), "step {step}: flags {}",c.lifecycle[1]);
        assert_eq!(c.lifecycle[1] & 4 != 0,step==1,"recycling fixture");
        let manifold = crate::api::contact_data::decode_manifold(c).expect("native manifold");
        assert_eq!(manifold.point_count, 4);
        for point in &manifold.points {
            assert_eq!(point.persisted, step > 0);
            assert_eq!(point.triangle_index, -1);
            assert!(point.separation.abs() < 1e-6);
            assert!(point.base_separation.abs() < 1e-6);
            assert_eq!(point.normal_impulse, 0.0);
            assert_eq!(point.total_normal_impulse, 0.0);
            assert_eq!(point.normal_velocity, 0.0);
        }

    }
    b3_destroy_world(world);
}


#[test]
fn contact_api_caches_data_without_events_and_invalidates_reused_world_ids() {
    use crate::api::{ContactData, b3_body_get_contact_capacity, b3_body_get_contact_data,
        b3_shape_get_contact_capacity, b3_shape_get_contact_data, b3_contact_is_valid, b3_contact_get_data};
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def(); wd.gravity = [0.0; 3]; wd.enable_continuous = false;
    let world = b3_create_world(gpu.clone(), &wd);
    let hole = b3_create_body(world, &b3_default_body_def());
    let ground = b3_create_body(world, &b3_default_body_def());
    let sd = b3_default_shape_def();
    b3_create_hull_shape(ground, &sd, &b3_make_box_hull(4.0, 0.5, 4.0));
    let mut bd = b3_default_body_def(); bd.body_type = BodyType::Dynamic; bd.position = [0.0, 1.0, 0.0];
    let body = b3_create_body(world, &bd);
    let shape = b3_create_hull_shape(body, &sd, &b3_make_box_hull(0.5,0.5,0.5));
    b3_destroy_body(hole);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    let before = crate::api::contact_snapshot_copies(world);
    assert_eq!(b3_body_get_contact_capacity(body), 1);
    let mut data = [ContactData::default(); 2];
    assert_eq!(b3_body_get_contact_data(body, &mut data), 1);
    let id = data[0].contact_id;
    let ptr = data[0].manifolds;
    assert!(!ptr.is_null());
    assert_eq!(unsafe { (*ptr).point_count }, 4);
    let copies = crate::api::contact_snapshot_copies(world);
    assert_eq!(copies, before + 1);
    for _ in 0..100 {
        assert_eq!(b3_shape_get_contact_capacity(shape), 1);
        assert_eq!(b3_shape_get_contact_data(shape, &mut data), 1);
        assert!(b3_contact_is_valid(id));
        assert_eq!(b3_contact_get_data(id).manifolds, ptr);
    }
    assert_eq!(crate::api::contact_snapshot_copies(world), copies);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    b3_world_gpu_wait(world);
    assert_eq!(crate::api::contact_snapshot_copies(world), copies, "ordinary step downloaded contact data");
    assert!(b3_contact_is_valid(id));
    assert_eq!(crate::api::contact_snapshot_copies(world), copies + 1);
    b3_destroy_body(body);
    assert!(!b3_contact_is_valid(id));
    b3_destroy_world(world);
    let replacement_world = b3_create_world(gpu, &wd);
    let ground = b3_create_body(replacement_world, &b3_default_body_def());
    b3_create_hull_shape(ground, &sd, &b3_make_box_hull(4.0,0.5,4.0));
    let body = b3_create_body(replacement_world, &bd);
    b3_create_hull_shape(body, &sd, &b3_make_box_hull(0.5,0.5,0.5));
    b3_world_step_gpu(replacement_world, 1.0/60.0, 4);
    assert_eq!(b3_body_get_contact_data(body, &mut data), 1);
    assert_ne!(data[0].contact_id, id);
    assert!(!b3_contact_is_valid(id));
    b3_destroy_world(replacement_world);
}

#[test]
fn baked_compound_ray_returns_parent_and_stable_child_ordinal() {
    use crate::api::{b3_create_compound_parent, b3_shape_attach_compound_child};
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let body = b3_create_body(world, &b3_default_body_def());
    let def = b3_default_shape_def();
    let parent = b3_create_compound_parent(body, &def);
    let mut children = Vec::new();
    for x in [-3.0, 3.0] {
        let mut hull = b3_make_box_hull(0.5, 0.5, 0.5);
        hull.center = [x, 0.0, 0.0];
        let child = b3_create_hull_shape(body, &def, &hull);
        assert!(b3_shape_attach_compound_child(parent, child));
        children.push(child);
    }
    let ray = |x| unsafe {
        crate::api::b3_world_cast_ray_closest(world, [x, 4.0, 0.0], [0.0, -8.0, 0.0], crate::api::b3_default_query_filter())
    };
    assert!(!ray(0.0).hit, "compound gap is not geometry");
    for (x, ordinal) in [(-3.0, 0), (3.0, 1)] {
        let hit = ray(x);
        assert!(hit.hit);
        assert_eq!(hit.shape_id, parent, "hidden child must not escape the API");
        assert_eq!(hit.child_index, ordinal);
    }
    b3_destroy_shape(children[0], false);
    let hit = ray(3.0);
    assert!(hit.hit);
    assert_eq!(hit.shape_id, parent);
    assert_eq!(hit.child_index, 1, "deleting a sibling must not renumber children");
    let mut hull = b3_make_box_hull(0.5, 0.5, 0.5);
    hull.center = [-3.0, 0.0, 0.0];
    let replacement = b3_create_hull_shape(body, &def, &hull);
    assert!(b3_shape_attach_compound_child(parent, replacement));
    assert_eq!(ray(-3.0).child_index, 2, "new child must not reuse retired ordinal");
    b3_destroy_world(world);
}

#[test]
fn collision_pipeline_tracks_mesh_add_remove() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def = b3_default_world_def();
    def.enable_sleep = false;
    let world = b3_create_world(gpu, &def);
    let ground = b3_create_body(world, &b3_default_body_def());
    // Keep two collider slots live so the initial primitive path really runs.
    let mut far_def = b3_default_body_def();
    far_def.position = [20.0, 0.0, 0.0];
    let far = b3_create_body(world, &far_def);
    let far_shape = b3_create_hull_shape(far, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 1.0, 0.0];
    body_def.enable_sleep = false;
    let ball = b3_create_body(world, &body_def);
    b3_create_sphere_shape(ball, &b3_default_shape_def(), &Sphere { center: [0.0; 3], radius: 0.5 });
    b3_world_step(world, 1.0 / 60.0, 4);
    let vertices = [[-4.0, 0.0, -4.0], [4.0, 0.0, -4.0], [4.0, 0.0, 4.0], [-4.0, 0.0, 4.0]];
    let mesh = b3_create_mesh_shape(ground, &b3_default_shape_def(), &vertices,
        &[[0, 2, 1], [0, 3, 2]], &[], &[], &[], [1.0; 3]);
    for _ in 0..120 { b3_world_step_gpu(world, 1.0 / 60.0, 4); }
    b3_world_gpu_wait_with_mirror(world);
    let y = b3_body_get_position(ball)[1];
    assert!((y - 0.5).abs() < 0.02, "mesh introduced after primitive pipeline: {y}");
    assert!(!b3_world_physics_invalid(world));
    b3_destroy_shape(mesh, false);
    b3_body_set_transform(ground, [0.0, -0.05, 0.0], [0.0, 0.0, 0.0, 1.0]);
    let slab = b3_create_hull_shape(ground, &b3_default_shape_def(), &b3_make_box_hull(4.0, 0.05, 4.0));
    for _ in 0..120 { b3_world_step_gpu(world, 1.0 / 60.0, 4); }
    b3_world_gpu_wait_with_mirror(world);
    let y = b3_body_get_position(ball)[1];
    assert!((y - 0.5).abs() < 0.02, "primitive pipeline after removing mesh: {y}");
    assert!(!b3_world_physics_invalid(world));
    // Removing both supports leaves one collider: collision dispatch is skipped,
    // but old contact chains must still retire and the body must fall freely.
    b3_destroy_shape(slab, false);
    b3_destroy_shape(far_shape, false);
    for _ in 0..60 { b3_world_step_gpu(world, 1.0 / 60.0, 4); }
    b3_world_gpu_wait_with_mirror(world);
    assert!(b3_body_get_position(ball)[1] < -1.0);
    assert!(b3_body_get_linear_velocity(ball)[1] < -5.0);
    assert!(!b3_world_physics_invalid(world));
    b3_destroy_world(world);
}


#[test]
fn bounded_static_sort_handles_two_children_and_restores_general_for_three() {
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
    let (world,ids)=overlapping_compound_world(gpu);
    b3_world_set_diagnostic_flags(world,crate::types::DIAG_BOUNDED_STATIC_SORT);
    for _ in 0..60 {b3_world_step_gpu(world,1.0/60.0,4);}
    assert_eq!(b3_world_last_static_sort_dispatches(world),0);
    assert_compounds_rest(world,ids,"bounded degree two");
    b3_create_hull_shape(ids[0],&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
    for _ in 0..60 {b3_world_step_gpu(world,1.0/60.0,4);}
    assert!(b3_world_last_static_sort_dispatches(world)>0);
    assert_compounds_rest(world,ids,"restored general degree three");
    b3_destroy_world(world);
}

#[cfg(feature = "native-command-cache")]
#[test]
fn full_physics_replay_timestamps_remain_live_and_partition_device_time() {
    use crate::api::*;
    let gpu=pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut def=b3_default_world_def();def.enable_sleep=false;
    let world=b3_create_world(gpu,&def);crate::scenes::create_mixed_stacks(world,600);
    b3_world_ensure_gpu(world);
    for step in 1..=16 {
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait(world);
        assert_eq!(b3_world_last_timestamp_step(world),step);
        let c=b3_world_last_collide_ms(world);let p=b3_world_last_prepare_ms(world);
        let s=b3_world_last_solve_ms(world);let i=b3_world_last_integrate_ms(world);let d=b3_world_last_device_ms(world);
        assert!([c,p,s,i,d].into_iter().all(|x|x.is_finite() && x>0.0));
        assert!([c,p,s,i].into_iter().all(|x|x<=d));
        assert!((c+p+s+i-d).abs()<=d*1e-5+1e-5,"step {step}: {c} + {p} + {s} + {i} != {d}");
    }
    assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
    b3_destroy_world(world);
}


#[test]
fn mass_queries_preserve_shape_and_explicit_mass() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    let mut def = b3_default_body_def();
    def.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &def);
    let mut shape = b3_default_shape_def();
    shape.density = 1000.0;
    let id = b3_create_hull_shape(body, &shape, &b3_make_box_hull(0.5, 0.5, 0.5));
    // Inverse mass 0.001 rounds back to 999.99994; public mass must stay 1000.
    let check = |expected: f32| {
        assert_eq!(crate::api::b3_body_get_mass(body).to_bits(), expected.to_bits());
        assert_eq!(b3_body_get_mass_data(body).mass.to_bits(), expected.to_bits());
    };
    check(1000.0);
    b3_body_apply_mass_from_shapes(body);
    check(1000.0);
    crate::api::b3_shape_set_density(id, 2000.0, true);
    check(2000.0);
    let mut mass = b3_body_get_mass_data(body);
    mass.mass = 1000.0;
    b3_body_set_mass_data(body, mass);
    check(1000.0);
    crate::api::b3_body_set_type(body, BodyType::Kinematic);
    check(0.0);
    crate::api::b3_body_set_type(body, BodyType::Dynamic);
    check(2000.0);
    b3_destroy_shape(id, true);
    check(0.0);
    b3_destroy_world(world);
}

#[test]
fn full_width_shape_pairs_preserve_callbacks_events_and_remapping() {
    shape_pair_lifecycle_at_capacity(65_536);
}

#[test]
fn tiled_physics_dispatch_preserves_callbacks_events_and_remapping() {
    // Cross the 65,535-workgroup boundary in capacity-wide 64-lane kernels.
    shape_pair_lifecycle_at_capacity(131_072);
}

fn shape_pair_lifecycle_at_capacity(filler_count: u32) {
    use crate::api::*;
    use std::sync::atomic::{AtomicI32,Ordering};
    static MAX_FILTER_ID:AtomicI32=AtomicI32::new(0);
    unsafe extern "C" fn accept(a:ShapeId,b:ShapeId,_:*mut std::ffi::c_void)->bool {
        MAX_FILTER_ID.fetch_max(a.index1.max(b.index1),Ordering::SeqCst);true
    }
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
    let world=b3_create_world(gpu,&b3_default_world_def());
    b3_world_enable_sleeping(world,false);
    let mut bd=b3_default_body_def();bd.position=[0.0,-0.5,0.0];
    let ground=b3_create_body(world,&bd);
    let mut events=b3_default_shape_def();events.enable_contact_events=true;events.enable_custom_filtering=true;
    let ground_shape=b3_create_hull_shape(ground,&events,&b3_make_box_hull(10.0,0.5,10.0));
    let filler_def=b3_default_shape_def();let mut filler=b3_null_body_id();
    for i in 0..filler_count {
        bd.position=[20.0+(i%256) as f32*4.0,5.0,20.0+(i/256) as f32*4.0];
        let body=b3_create_body(world,&bd);
        if i==0 {filler=body;}
        assert_ne!(b3_create_sphere_shape(body,&filler_def,&Sphere{center:[0.0;3],radius:0.1}).index1,0);
    }
    bd.body_type=BodyType::Dynamic;bd.position=[0.0,0.45,0.0];
    let low=b3_create_body(world,&bd);
    let low_shape=b3_create_sphere_shape(low,&events,&Sphere{center:[0.0;3],radius:0.5});
    bd.position=[0.0,1.35,0.0];
    let high=b3_create_body(world,&bd);
    let high_shape=b3_create_sphere_shape(high,&events,&Sphere{center:[0.0;3],radius:0.5});
    assert!(low.index1>65_536 && low_shape.index1>65_536 && high_shape.index1>65_536);
    b3_world_set_custom_filter_callback(world,Some(accept),std::ptr::null_mut());
    let events_now=|| {
        let (begin,n,end,m,_,_)=b3_world_contact_event_ptrs(world);
        let starts=if n==0 {Vec::new()} else {unsafe{std::slice::from_raw_parts(begin,n as usize)}.iter().map(|e|(e.shape_id_a,e.shape_id_b)).collect()};
        let ends=if m==0 {Vec::new()} else {unsafe{std::slice::from_raw_parts(end,m as usize)}.iter().map(|e|(e.shape_id_a,e.shape_id_b)).collect()};
        (starts,ends)
    };
    let contains=|pairs:&Vec<(ShapeId,ShapeId)>,a:ShapeId,b:ShapeId|pairs.iter().any(|&(x,y)| (x.index1==a.index1&&x.generation==a.generation&&y.index1==b.index1&&y.generation==b.generation)||(y.index1==a.index1&&y.generation==a.generation&&x.index1==b.index1&&x.generation==b.generation));
    b3_world_step_gpu(world,1.0/60.0,4);
    let (begins,ends)=events_now();
    assert!(contains(&begins,ground_shape,low_shape));assert!(contains(&begins,low_shape,high_shape));assert!(ends.is_empty());
    assert!(MAX_FILTER_ID.load(Ordering::SeqCst)>65_536);
    b3_destroy_body(filler); // Dense physical shape indices shift; public IDs stay.
    b3_world_step_gpu(world,1.0/60.0,4);
    let (begins,ends)=events_now();assert!(begins.is_empty()&&ends.is_empty(),"remap must preserve both retained contacts");
    b3_body_set_transform(high,[8.0,3.0,0.0],[0.0,0.0,0.0,1.0]);
    b3_world_step_gpu(world,1.0/60.0,4);
    let (_,ends)=events_now();assert!(contains(&ends,low_shape,high_shape));
    b3_destroy_body(high);bd.position=[0.0,1.35,0.0];
    let replacement=b3_create_body(world,&bd);
    let replacement_shape=b3_create_sphere_shape(replacement,&events,&Sphere{center:[0.0;3],radius:0.5});
    assert_eq!(replacement.index1,high.index1);assert_ne!(replacement.generation,high.generation);
    b3_world_step_gpu(world,1.0/60.0,4);
    let (begins,_)=events_now();assert!(contains(&begins,low_shape,replacement_shape));
    assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
    assert!(!b3_world_physics_invalid(world));
    b3_destroy_world(world);
}

#[test]
fn capsule_contact_endpoint_is_translation_invariant() {
    // The Falling Ragdolls spine pair has almost parallel axes. At x=8.25,
    // world-space endpoint rounding previously flipped the torque lever arm.
    // Independent Box3D C gives +0.06 m for both anchors at every translation.
    for x in [0.0, 7.5, 8.25, -7.5] {
        let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0; 3];
        let world = b3_create_world(gpu, &wd);
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.position = [x, 16.113505, -0.03481];
        bd.rotation = [0.739973, 0.0, 0.0, 0.672637];
        let a = b3_create_body(world, &bd);
        let sd = b3_default_shape_def();
        b3_create_capsule_shape(a, &sd, &Capsule {
            center1: [0.06, 0.0, -0.052264],
            center2: [-0.06, 0.0, -0.052264], radius: 0.12,
        });
        bd.position = [x, 16.31043, -0.028232];
        bd.rotation = [0.669856, 0.000001, -0.000001, 0.742491];
        let b = b3_create_body(world, &bd);
        b3_create_capsule_shape(b, &sd, &Capsule {
            center1: [0.11, -0.039753, -0.13],
            center2: [-0.11, -0.039753, -0.13], radius: 0.145,
        });
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
        b3_world_gpu_wait_with_mirror(world);
        let mut data = [crate::api::ContactData::default(); 2];
        assert_eq!(crate::api::b3_body_get_contact_data(a, &mut data), 1);
        assert_eq!(data[0].manifold_count, 1);
        let manifold = unsafe { &*data[0].manifolds };
        assert_eq!(manifold.point_count, 1);
        let point = manifold.points[0];
        for anchor in [point.anchor_a, point.anchor_b] {
            assert!((anchor[0] - 0.06).abs() < 1e-5,
                "x={x}: wrong capsule endpoint {anchor:?}");
        }
        assert!((point.separation - 0.00995803).abs() < 2e-6);
        b3_destroy_world(world);
    }
}

#[test]
fn jointed_contact_keeps_color_when_another_contact_stops() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    wd.enable_sleep = false;
    let world = b3_create_world(gpu, &wd);
    let ground = b3_create_body(world, &b3_default_body_def());
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    let center = b3_create_body(world, &bd);
    let sphere = Sphere { center: [0.0; 3], radius: 1.0 };
    let sd = b3_default_shape_def();
    b3_create_sphere_shape(center, &sd, &sphere);
    bd.position = [1.99, 0.0, 0.0];
    let right = b3_create_body(world, &bd);
    b3_create_sphere_shape(right, &sd, &sphere);
    bd.position = [-1.99, 0.0, 0.0];
    let left = b3_create_body(world, &bd);
    b3_create_sphere_shape(left, &sd, &sphere);
    let mut joint = b3_default_spherical_joint_def();
    joint.body_a = ground;
    joint.body_b = center;
    b3_create_spherical_joint(world, &joint);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let active: Vec<_> = contacts.iter().filter(|c| c.a != u32::MAX && c.count > 0).collect();
    assert_eq!(active.len(), 2);
    let later = active.iter().find(|c| c.color == 1).expect("second-colored contact");
    let staying = if later.a == (left.index1 - 1) as u32 || later.b == (left.index1 - 1) as u32 { left } else { right };
    let leaving = if staying.index1 == left.index1 { right } else { left };
    let color = later.color;
    let direction = b3_body_get_position(leaving)[0].signum();
    b3_body_set_linear_velocity(leaving, [100.0 * direction, 0.0, 0.0]);
    for step in 0..3 {
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
        let cs = pollster::block_on(b3_world_sync_contacts(world));
        let contact = cs.iter().find(|c| c.count > 0 && c.a != u32::MAX
            && (c.a == (staying.index1 - 1) as u32 || c.b == (staying.index1 - 1) as u32))
            .expect("staying contact after separation");
        assert_eq!(contact.color, color, "surviving contact recolored at step {step}");
    }
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let active: Vec<_> = contacts.iter().filter(|c| c.a != u32::MAX && c.count > 0).collect();
    assert_eq!(active.len(), 1);
    assert_eq!(active[0].color, color, "surviving contact changed solve priority");
    b3_destroy_world(world);
}

#[test]
fn capsule_parallel_threshold_matches_native_float_product() {
    // Native b3CollideCapsules gives two points at this float32 boundary.
    // 0.05f * 0.05f is one ULP greater than the rounded literal 0.0025f.
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    wd.enable_sleep = false;
    let world = b3_create_world(gpu, &wd);
    b3_world_enable_continuous(world, false);
    let mut bd = b3_default_body_def();
    let a = b3_create_body(world, &bd);
    let sd = b3_default_shape_def();
    b3_create_capsule_shape(a, &sd, &Capsule {
        center1: [-0.5, 0.0, 0.0], center2: [0.5, 0.0, 0.0], radius: 0.1,
    });
    bd.body_type = BodyType::Dynamic;
    bd.enable_contact_recycling = false;
    let b = b3_create_body(world, &bd);
    b3_create_capsule_shape(b, &sd, &Capsule {
        center1: [0.0, 0.05, 0.0],
        center2: [f32::from_bits(0x3f7fae07), 0.05 + f32::from_bits(0x3d10d0b3), f32::from_bits(0x3d10d0d0)],
        radius: 0.1,
    });
    b3_world_step_gpu(world, 1.0 / 60.0, 1);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let c = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0).expect("capsule contact");
    let count = c.count;
    b3_destroy_world(world);
    assert_eq!(count, 2, "native boundary manifold must retain both clipped points");
}

#[test]
fn capsule_feature_ids_survive_single_and_clipped_manifold_transitions() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    wd.enable_sleep = false;
    let world = b3_create_world(gpu, &wd);
    b3_world_enable_continuous(world, false);
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    bd.enable_contact_recycling = false;
    let short = b3_create_body(world, &bd);
    let sd = b3_default_shape_def();
    b3_create_capsule_shape(short, &sd, &Capsule {
        center1: [-0.5,0.0,0.0], center2: [0.5,0.0,0.0], radius: 0.2,
    });
    bd.position = [0.0,0.35,0.0];
    let long = b3_create_body(world, &bd);
    b3_create_capsule_shape(long, &sd, &Capsule {
        center1: [-0.7,0.0,0.0], center2: [0.7,0.0,0.0], radius: 0.2,
    });
    let expected = [vec![(0,false),(0x00010001,false)], vec![(0,true)],
                    vec![(0,true),(0x00010001,false)]];
    for (step, angle) in [0.0, -0.2, 0.0].into_iter().enumerate() {
        b3_body_set_transform(short, [0.0;3], [0.0,0.0,0.0,1.0]);
        b3_body_set_transform(long, [0.0,0.35,0.0], b3_make_quat_from_axis_angle([0.0,0.0,1.0], angle));
        for body in [short,long] {
            b3_body_set_linear_velocity(body, [0.0;3]);
            crate::api::b3_body_set_angular_velocity(body, [0.0;3]);
        }
        b3_world_step_gpu(world, 1.0/60.0, 4);
        let contacts = pollster::block_on(b3_world_sync_contacts(world));
        let c = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0).expect("capsule contact");
        let manifold = crate::api::contact_data::decode_manifold(c).unwrap();
        let mut actual: Vec<_> = manifold.points[..manifold.point_count as usize].iter()
            .map(|p|(p.feature_id,p.persisted)).collect();
        actual.sort();
        assert_eq!(actual, expected[step], "feature identity transition {step}");
    }
    b3_destroy_world(world);
}

#[test]
fn capsule_rolling_and_twist_friction_match_cpu() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    wd.enable_sleep = false;
    let world = b3_create_world(gpu, &wd);
    b3_world_enable_continuous(world, false);
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    bd.enable_contact_recycling = false;
    bd.linear_velocity = [0.4, 0.2, -0.3];
    bd.angular_velocity = [2.0, -1.0, 3.0];
    let a = b3_create_body(world, &bd);
    let mut sd = b3_default_shape_def();
    sd.rolling_resistance = 0.2;
    b3_create_capsule_shape(a, &sd, &Capsule {
        center1: [-0.5,0.0,0.0], center2: [0.5,0.0,0.0], radius: 0.2,
    });
    bd.position = [0.0,0.35,0.0];
    bd.linear_velocity = [-0.3, -0.5, 0.4];
    bd.angular_velocity = [-3.0, 2.0, -1.0];
    let b = b3_create_body(world, &bd);
    b3_create_capsule_shape(b, &sd, &Capsule {
        center1: [-0.7,0.0,0.0], center2: [0.7,0.0,0.0], radius: 0.2,
    });
    // Independent Box3D C, same two capsules, two steps with four substeps.
    // Concurrent rolling/twist friction makes Gauss-Seidel phase order observable.
    let expected_steps = [
        [
            [0.12671411, -0.456347167, -0.105784342, 3.52523422, 0.57609266, 1.01008356],
            [-0.0923027173, -0.00117617473, 0.252396047, -1.47855854, 1.31995833, 0.242672667],
        ],
        [
            [0.12671411, -0.456347108, -0.105784342, 3.52395248, 0.62885797, 0.982259154],
            [-0.0923027173, -0.00117617473, 0.252396047, -1.47760618, 1.3150624, 0.272714734],
        ],
    ];
    for expected in expected_steps {
        b3_world_step_gpu(world, 1.0/60.0, 4);
        b3_world_gpu_wait_with_mirror(world);
        for (body, expected) in [a,b].into_iter().zip(expected) {
            let v = b3_body_get_linear_velocity(body);
            let w = b3_body_get_angular_velocity(body);
            let actual = [v[0],v[1],v[2],w[0],w[1],w[2]];
            for i in 0..6 {
                assert!((actual[i]-expected[i]).abs() < 1e-4,
                    "rolling/twist response: actual={actual:?} CPU={expected:?}");
            }
        }
    }
    b3_destroy_world(world);
}

#[test]
fn capsule_original_endpoints_match_cpu_contact_normal() {
    // Captured CPU frame-2 thigh/spine inputs. Reconstructing the asymmetric
    // endpoints through a midpoint/axis changes the normal by one float ulp.
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    wd.enable_sleep = false;
    wd.enable_continuous = false;
    let world = b3_create_world(gpu, &wd);
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    bd.enable_contact_recycling = false;
    bd.position = [-6.74983692,16.1078739,-7.53534126];
    bd.rotation = [0.740263045,0.000472915184,0.000308163959,0.672317207];
    let b = b3_create_body(world, &bd);
    let sd = b3_default_shape_def();
    b3_create_capsule_shape(b, &sd, &Capsule {
        center1: [0.0599999987,0.0,-0.0522640012],
        center2: [-0.0599999987,0.0,-0.0522640012], radius: 0.119999997,
    });
    bd.position = [-6.81319523,15.9843035,-7.53504276];
    bd.rotation = [-0.700930178,0.0899335667,-0.0780423284,0.703219891];
    let a = b3_create_body(world, &bd);
    let capsule = Capsule {
        center1: [-0.0237189997,0.00600800011,-0.0390679985],
        center2: [0.0644920021,-0.00466400012,-0.424717993], radius: 0.0900000036,
    };
    let shape = b3_create_capsule_shape(a, &sd, &capsule);
    let actual = crate::api::b3_shape_get_capsule(shape);
    assert_eq!(actual.center1, capsule.center1);
    assert_eq!(actual.center2, capsule.center2);
    assert!(crate::api::b3_shape_set_capsule(shape, &capsule));
    let actual = crate::api::b3_shape_get_capsule(shape);
    assert_eq!(actual.center1, capsule.center1);
    assert_eq!(actual.center2, capsule.center2);
    b3_world_step_gpu(world,1.0/60.0,4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let c = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0).expect("contact");
    let sign = if c.a == (a.index1-1) as u32 { 1.0 } else { -1.0 };
    for (actual, expected) in [c.nx,c.ny,c.nz].into_iter()
        .zip([0.168884620,0.984409750,0.0491481274]) {
        assert!((sign*actual-expected).abs()<2e-8,
            "CPU contact normal: {} vs {expected}",sign*actual);
    }
    b3_destroy_world(world);
}

#[test]
fn capsule_friction_rotating_normal_matches_cpu() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.enable_sleep = false;
    wd.enable_continuous = false;
    let world = b3_create_world(gpu, &wd);
    let ground = b3_create_body(world, &b3_default_body_def());
    let mut sd = b3_default_shape_def();
    sd.rolling_resistance = 0.2;
    b3_create_capsule_shape(ground, &sd, &Capsule {
        center1: [-0.5,0.0,0.0], center2: [0.5,0.0,0.0], radius: 0.2,
    });
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    bd.enable_contact_recycling = false;
    bd.position = [0.1,0.38,0.02];
    bd.rotation = [0.0,0.0,0.074929707,0.997188818];
    bd.linear_velocity = [0.4,-0.5,0.3];
    bd.angular_velocity = [2.0,-1.0,3.0];
    let body = b3_create_body(world, &bd);
    b3_create_capsule_shape(body, &sd, &Capsule {
        center1: [-0.4,0.0,0.0], center2: [0.4,0.0,0.0], radius: 0.2,
    });
    // Independent Box3D C checkpoints. Sliding/rotating capsules change the
    // contact normal, so cached friction must be projected onto its new basis.
    // At step 7 the manifold grows from one point to two. The static capsule
    // must be the reference segment; reversing it changes clipping and torque.
    let expected = [
        [0.0729924738, 0.033056438, 0.408481181, 0.992330968, -0.436167717, -0.346350908],
        [0.0729924738, -0.133610263, 0.408481181, 0.992664516, -0.440705508, -0.33956039],
        [0.102183118, -0.229524747, 0.404058933, 1.05939186, -0.44662568, -0.521404445],
        [0.13933146, -0.315533042, 0.405064017, 1.09358132, -0.442227125, -0.720146716],
        [0.174168766, -0.40222317, 0.407256573, 1.13306844, -0.441703856, -0.920345724],
        [0.205887839, -0.490173399, 0.410348147, 1.17827082, -0.445246816, -1.12249553],
        [-0.000993938185, -0.035963621, 0.377171814, 1.92905891, -0.00116696942, 0.00494829472],
        [-0.00540184509, -0.0525989383, 0.375118017, 1.91705549, -0.00480221678, 0.0269327909],
        [-0.00850363728, -0.0648762658, 0.374413282, 1.91505635, -0.00818460807, 0.042076353],
        [-0.0104090041, -0.0749290287, 0.375255466, 1.92311394, -0.0108003793, 0.0511876307],
        [-0.0116243809, -0.0839491636, 0.377723187, 1.94111419, -0.012947442, 0.0568599589],
        [-0.0124602811, -0.0926799178, 0.381848454, 1.96895432, -0.0148413312, 0.0606477037],
    ];
    for (step, reference) in expected.iter().enumerate() {
        b3_world_step_gpu(world,1.0/60.0,4);
        b3_world_gpu_wait_with_mirror(world);
        let v = b3_body_get_linear_velocity(body);
        let w = b3_body_get_angular_velocity(body);
        for (component, (&actual, &target)) in v.iter().chain(w.iter()).zip(reference).enumerate() {
            assert!((actual-target).abs()<1e-4,
                "step {} component {component}: {actual} vs {target}",step+1);
        }
    }
    b3_destroy_world(world);
}

#[test]
fn capsule_mesh_contact_uses_capsule_local_frame() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    for offset in [0.0, 1024.0] {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0; 3];
        wd.enable_sleep = false;
        wd.enable_continuous = false;
        let world = b3_create_world(gpu.clone(), &wd);
        let mut ground_def = b3_default_body_def();
        ground_def.position = [offset, 0.0, offset];
        let ground = b3_create_body(world, &ground_def);
        let mut sd = b3_default_shape_def();
        sd.rolling_resistance = 0.2;
        b3_create_mesh_shape(ground, &sd,
            &[[-10.0,0.0,-10.0],[0.0,0.0,10.0],[10.0,0.0,-10.0]],
            &[[0,1,2]], &[], &[], &[] as &[MeshNode], [1.0;3]);
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.enable_contact_recycling = false;
        bd.position = [offset + 0.25,0.5,offset + 0.25];
        bd.rotation = [-0.700930178,0.0899335667,-0.0780423284,0.703219891];
        bd.linear_velocity = [0.4,-0.5,-0.3];
        bd.angular_velocity = [2.0,-1.0,3.0];
        let body = b3_create_body(world, &bd);
        b3_create_capsule_shape(body, &sd, &Capsule {
            center1: [-0.0237189997,0.00600800011,-0.0390679985],
            center2: [0.0644920021,-0.00466400012,-0.424717993], radius: 0.0900000036,
        });
        b3_world_step_gpu(world,1.0/60.0,4);
        b3_world_gpu_wait_with_mirror(world);
        // Independent C oracle: triangle rotated into the original capsule frame,
        // then the manifold normal rotated back with the body matrix.
        let contacts = pollster::block_on(b3_world_sync_contacts(world));
        let c = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0).expect("mesh contact");
        assert_eq!(c.count,2);
        for (actual,expected) in [c.nx,c.ny,c.nz].into_iter().zip([0.0,0.99999994,-6.98491931e-9]) {
            assert!((actual-expected).abs()<2e-8,"mesh normal {actual} vs CPU {expected}");
        }
        // CPU constructs these COM anchors without a world-space round trip.
        // Translating the entire scene must not discard the small lever arms.
        for (anchor, expected) in [c.rb0, c.rb1].into_iter().zip([
            [0.0025437735, -0.0810171813, -0.00434703194],
            [-0.00254378468, -0.278829932, 0.00434701517],
        ]) {
            for axis in 0..3 {
                assert!((anchor[axis] - expected[axis]).abs() < 1e-7,
                    "mesh anchor at offset {offset}: {anchor:?} vs CPU {expected:?}");
            }
        }
        let v = b3_body_get_linear_velocity(body);
        let w = b3_body_get_angular_velocity(body);
        // Use an independent CPU run at each translation: body integration itself
        // also rounds at large coordinates, even though the initial anchors agree.
        let expected = if offset == 0.0 {
            [0.788099527,0.0157151818,-0.582600594,-0.768641174,-0.832056761,-0.89757359]
        } else {
            [0.788165808,0.0157152414,-0.582630157,-0.768363476,-0.832071841,-0.897239208]
        };
        for (actual,expected) in v.into_iter().chain(w).zip(expected) {
            assert!((actual-expected).abs()<1e-4,"mesh velocity {actual} vs CPU {expected}");
        }
        b3_destroy_world(world);
    }
}

#[test]
fn capsule_mesh_friction_uses_cpu_scalar_order() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    for (extent, half_length, expected) in [
        (10.0,0.5,[-0.00234478712,0.30840981,-0.113943845,-0.576931,-0.0000495237182,0.0118713146]),
        (1.0,3.0,[-0.522853732,1.3622992,-0.0668234229,-0.338079035,-0.809676886,2.64401054]),
    ] {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0; 3];
        wd.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &wd);
        b3_world_enable_continuous(world, false);
        let ground = b3_create_body(world, &b3_default_body_def());
        let mut sd = b3_default_shape_def();
        sd.rolling_resistance = 0.2;
        b3_create_mesh_shape(ground, &sd,
            &[[-extent,0.0,-extent],[0.0,0.0,extent],[extent,0.0,-extent]],
            &[[0,1,2]], &[], &[], &[] as &[MeshNode], [1.0;3]);
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.enable_contact_recycling = false;
        bd.position = [0.0,0.195,0.0];
        bd.linear_velocity = [0.4,-0.5,-0.3];
        bd.angular_velocity = [2.0,-1.0,3.0];
        let body = b3_create_body(world, &bd);
        b3_create_capsule_shape(body, &sd, &Capsule {
            center1: [-half_length,0.0,0.0], center2: [half_length,0.0,0.0], radius: 0.2,
        });
        b3_world_step_gpu(world, 1.0/60.0, 4);
        b3_world_gpu_wait_with_mirror(world);
        let v = b3_body_get_linear_velocity(body);
        let w = b3_body_get_angular_velocity(body);
        let actual = [v[0],v[1],v[2],w[0],w[1],w[2]];
        // Independent Box3D C, one step. Mesh friction has a different solve
        // order from the SIMD convex path. The second case also requires clipping
        // both endpoints: neither original capsule end lies over the triangle.
        for i in 0..6 {
            assert!((actual[i]-expected[i]).abs() < 1e-4,
                "mesh friction response: actual={actual:?} CPU={expected:?}");
        }
        b3_destroy_world(world);
    }
}

#[test]
fn capsule_proxy_retires_separated_parallel_pair() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    for axis in 0..3 {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0; 3];
        wd.enable_sleep = false;
        wd.enable_continuous = false;
        let world = b3_create_world(gpu.clone(), &wd);
        let mut endpoint = [0.0; 3];
        endpoint[axis] = 1.0;
        let capsule = Capsule { center1: endpoint.map(|x| -x), center2: endpoint, radius: 0.1 };
        let sd = b3_default_shape_def();
        let mut bd = b3_default_body_def();
        let fixed = b3_create_body(world, &bd);
        b3_create_capsule_shape(fixed, &sd, &capsule);
        bd.body_type = BodyType::Dynamic;
        bd.position[(axis + 1) % 3] = 0.19;
        let moving = b3_create_body(world, &bd);
        b3_create_capsule_shape(moving, &sd, &capsule);
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
        b3_world_gpu_wait_with_mirror(world);
        let contacts = pollster::block_on(b3_world_sync_contacts(world));
        assert!(contacts.iter().any(|c| c.a != u32::MAX && c.count > 0), "initial capsule contact");
        // Farther than the endpoint-based fat bounds, but still inside the
        // old surrounding-sphere bounds. Retire the pair, not just its points.
        let mut position = [0.0; 3];
        position[(axis + 1) % 3] = 0.5;
        b3_body_set_transform(moving, position, [0.0, 0.0, 0.0, 1.0]);
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
        b3_world_gpu_wait_with_mirror(world);
        let contacts = pollster::block_on(b3_world_sync_contacts(world));
        assert!(contacts.iter().all(|c| c.a == u32::MAX), "separated capsule pair retained on axis {axis}");
        b3_destroy_world(world);
    }
}

#[test]
fn ccd_proxy_padding_tracks_no_hit_and_impact() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    for mesh in [false, true] {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0; 3];
        wd.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &wd);
        let mut bd = b3_default_body_def();
        bd.position = [0.0, -0.5, 0.0];
        let ground = b3_create_body(world, &bd);
        let sd = b3_default_shape_def();
        if mesh {
            b3_create_mesh_shape(ground, &sd,
                &[[-10.0,0.5,-10.0],[0.0,0.5,10.0],[10.0,0.5,-10.0]],
                &[[0,1,2]], &[], &[], &[] as &[MeshNode], [1.0;3]);
        } else {
            b3_create_hull_shape(ground, &sd, &b3_make_box_hull(10.0, 0.5, 10.0));
        }
        bd.body_type = BodyType::Dynamic;
        bd.position = [0.0, 5.0, 0.0];
        bd.linear_velocity = [0.0, -100.0, 0.0];
        let body = b3_create_body(world, &bd);
        b3_create_sphere_shape(body, &sd, &Sphere { center: [0.0;3], radius: 0.1 });
        let mut hit = false;
        for step in 0..4 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            b3_world_gpu_wait_with_mirror(world);
            let states = pollster::block_on(crate::api::b3_world_sync_from_gpu(world));
            let moving = states.iter().find(|b| b.inv_mass > 0.0).unwrap();
            if step == 0 {
                assert_ne!(moving.flags & crate::types::FLAG_CCD_NO_HIT, 0, "fast unobstructed step");
            }
            if moving.pos[1] < 0.2 {
                assert_eq!(moving.flags & crate::types::FLAG_CCD_NO_HIT, 0, "impact must restore padded proxy bounds");
                hit = true;
                break;
            }
        }
        assert!(hit, "CCD impact fixture mesh={mesh}");
        b3_destroy_world(world);
    }
}

#[test]
fn jointed_contact_start_precedes_later_id_retirement() {
    contact_transition_fixture(false);
}

#[test]
fn jointed_contact_transition_survives_buffer_growth() {
    contact_transition_fixture(true);
}

fn contact_transition_fixture(grow: bool) {
    // Native reference: two ghost pairs get IDs left=0, right=1. Right starts
    // on step 2 (color 0). On step 3 left starts before right retires, so left
    // must get color 1 even though color 0 is free at the end of the step.
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    wd.enable_sleep = false;
    let world = b3_create_world(gpu, &wd);
    let ground = b3_create_body(world, &b3_default_body_def());
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    let center = b3_create_body(world, &bd);
    let sphere = Sphere { center: [0.0; 3], radius: 1.0 };
    let sd = b3_default_shape_def();
    b3_create_sphere_shape(center, &sd, &sphere);
    bd.position = [2.03, 0.0, 0.0];
    let right = b3_create_body(world, &bd);
    b3_create_sphere_shape(right, &sd, &sphere);
    bd.position = [-2.03, 0.0, 0.0];
    let left = b3_create_body(world, &bd);
    b3_create_sphere_shape(left, &sd, &sphere);
    let mut joint = b3_default_spherical_joint_def();
    joint.body_a = ground;
    joint.body_b = center;
    b3_create_spherical_joint(world, &joint);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let cs = pollster::block_on(b3_world_sync_contacts(world));
    assert_eq!(cs.iter().filter(|c| c.a != u32::MAX).count(), 2);
    assert!(cs.iter().all(|c| c.count == 0));
    let identity = [0.0, 0.0, 0.0, 1.0];
    b3_body_set_transform(right, [1.99, 0.0, 0.0], identity);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let cs = pollster::block_on(b3_world_sync_contacts(world));
    assert_eq!(cs.iter().find(|c| c.a != u32::MAX && c.count > 0).unwrap().color, 0);
    if grow {
        let mut filler = b3_default_body_def();
        filler.position = [1000.0, 1000.0, 1000.0];
        for _ in 0..300 { b3_create_body(world, &filler); }
    }
    for (body, position) in [(center, [0.0; 3]), (right, [2.3, 0.0, 0.0]), (left, [-1.99, 0.0, 0.0])] {
        b3_body_set_transform(body, position, identity);
        b3_body_set_linear_velocity(body, [0.0; 3]);
    }
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let cs = pollster::block_on(b3_world_sync_contacts(world));
    let active: Vec<_> = cs.iter().filter(|c| c.a != u32::MAX && c.count > 0).collect();
    assert_eq!(active.len(), 1);
    assert_eq!(active[0].color, 1, "later contact retired before earlier contact started");
    b3_destroy_world(world);
}

#[test]
fn jointed_contact_inherits_color_when_first_joint_is_added() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    wd.enable_sleep = false;
    let world = b3_create_world(gpu, &wd);
    let floor = b3_create_body(world, &b3_default_body_def());
    b3_create_hull_shape(floor, &b3_default_shape_def(), &b3_make_box_hull(5.0, 0.5, 5.0));
    let mut bd = b3_default_body_def();
    bd.position = [0.0, 1.49, 0.0];
    let anchor = b3_create_body(world, &bd);
    bd.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &bd);
    b3_create_sphere_shape(body, &b3_default_shape_def(), &Sphere { center: [0.0; 3], radius: 1.0 });
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let before = pollster::block_on(b3_world_sync_contacts(world));
    let old_color = before.iter().find(|c| c.a != u32::MAX && c.count > 0).unwrap().color;
    assert_eq!(old_color, 22);
    let mut joint = b3_default_spherical_joint_def();
    joint.body_a = anchor;
    joint.body_b = body;
    b3_create_spherical_joint(world, &joint);
    b3_world_step_gpu(world, 1.0 / 60.0, 4);
    let after = pollster::block_on(b3_world_sync_contacts(world));
    let contact = after.iter().find(|c| c.a != u32::MAX && c.count > 0).unwrap();
    assert_eq!(contact.color, old_color, "new joint stole a surviving contact's color");
    b3_destroy_world(world);
}

#[test]
fn capsule_mesh_solves_face_patch_before_tentative_edge() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    wd.enable_sleep = false;
    wd.enable_continuous = false;
    let world = b3_create_world(gpu, &wd);
    let ground = b3_create_body(world, &b3_default_body_def());
    // Native frame-122 head contact, expressed in the capsule frame. Force
    // BVH visitation of the tentative edge before the accepted triangle face.
    b3_create_mesh_shape(ground, &b3_default_shape_def(),
        &[[-0.139201164,-0.302850783,-0.394349217],
          [-0.00188159943,0.92638427,0.835102201],
          [-0.255115032,0.689032912,1.01095104],
          [-0.0233283043,-0.079226017,-0.690914631]],
        &[[0,1,2],[1,0,3]], &[116,116], &[],
        &[MeshNode { lower: [-1.0;3], upper: [2.0;3], data: (2<<2)|3, triangle_offset: 0 }], [1.0;3]);
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &bd);
    b3_create_capsule_shape(body, &b3_default_shape_def(), &Capsule {
        center1: [-0.000001,0.016892,-0.05869],
        center2: [0.0,-0.003629,-0.115072], radius: 0.0975,
    });
    b3_world_step_gpu(world, 1.0/60.0, 4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let root = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0 && c.manifold_link[1] == 0).expect("root");
    assert_eq!(root.manifold_link[2], 2, "both native patches must survive");
    assert_eq!(root.count, 2, "face patch must precede the one-point tentative edge");
    let child = &contacts[(root.manifold_link[0]-1) as usize];
    assert_eq!(child.count, 1);
    for (actual, expected) in [root.nx,root.ny,root.nz].into_iter().zip([0.948055923,-0.271576464,0.165638655]) {
        assert!((actual-expected).abs()<1e-5);
    }
    b3_destroy_world(world);
}

#[test]
fn capsule_mesh_preserves_distinct_seam_witnesses() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0; 3];
    wd.enable_sleep = false;
    wd.enable_continuous = false;
    let world = b3_create_world(gpu, &wd);
    let ground = b3_create_body(world, &b3_default_body_def());
    b3_create_mesh_shape(ground, &b3_default_shape_def(),
        &[[-1.0,0.0,-1.0],[-1.0,0.0,1.0],[1.0,0.0,1.0],[1.0,0.0,-1.0]],
        &[[0,1,2],[0,2,3]], &[], &[],
        &[MeshNode { lower: [-1.0,0.0,-1.0], upper: [1.0,0.0,1.0], data: (2<<2)|3, triangle_offset: 0 }], [1.0;3]);
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &bd);
    b3_create_capsule_shape(body, &b3_default_shape_def(), &Capsule {
        center1: [-0.3,0.1,0.0], center2: [0.3,0.1,0.0], radius: 0.1,
    });
    b3_world_step_gpu(world, 1.0/60.0, 4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let root = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0 && c.manifold_link[1] == 0).expect("root");
    // Independent Box3D oracle retains both triangle witnesses at x=0,
    // following the two extreme points. Proximity is not contact identity.
    assert_eq!(root.count, 4, "each triangle contributes its seam witness");
    assert_eq!(root.point_triangles, [1,2,2,1]);
    for (anchor, x) in [root.rb0, root.rb1, root.rb2, root.rb3].into_iter().zip([-0.3,0.3,0.0,0.0]) {
        for (actual, expected) in anchor[..3].iter().copied().zip([x,-0.099999994,0.0]) {
            assert!((actual-expected).abs()<1e-7, "seam anchor {anchor:?}, expected x={x}");
        }
    }
    b3_destroy_world(world);
}

#[test]
fn capsule_mesh_reduces_complete_cluster() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let mut wd = b3_default_world_def();
    wd.gravity = [0.0;3];
    wd.enable_sleep = false;
    wd.enable_continuous = false;
    let world = b3_create_world(gpu, &wd);
    let ground = b3_create_body(world, &b3_default_body_def());
    let mut vertices = Vec::new();
    let mut triangles = Vec::new();
    for i in 0..9 {
        let x = -0.8_f32 + 0.2_f32 * i as f32;
        vertices.extend([[x,0.0,-1.0],[x,0.0,1.0]]);
    }
    for i in 0..8 {
        let a = 2*i;
        triangles.extend([[a,a+1,a+3],[a,a+3,a+2]]);
    }
    b3_create_mesh_shape(ground, &b3_default_shape_def(), &vertices, &triangles, &[], &[],
        &[MeshNode { lower: [-1.0,0.0,-1.0], upper: [1.0,0.0,1.0], data: (16<<2)|3, triangle_offset: 0 }], [1.0;3]);
    let mut bd = b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &bd);
    b3_create_capsule_shape(body, &b3_default_shape_def(), &Capsule {
        center1: [-0.7,0.1,0.0], center2: [0.7,0.1,0.0], radius: 0.1,
    });
    b3_world_step_gpu(world, 1.0/60.0, 4);
    let contacts = pollster::block_on(b3_world_sync_contacts(world));
    let root = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0 && c.manifold_link[1] == 0).expect("root");
    assert_eq!(root.count,4);
    // Independent native culling across all 16 triangles, including the
    // swap-removal tie order. Streaming four-point reduction loses this order.
    assert_eq!(root.point_triangles, [1,15,16,15]);
    for (anchor,x) in [root.rb0,root.rb1,root.rb2,root.rb3].into_iter().zip([-0.699999988,0.699999988,0.699999988,0.599999964]) {
        for (actual,expected) in anchor[..3].iter().copied().zip([x,-0.099999994,0.0]) {
            assert!((actual-expected).abs()<1e-7, "cluster anchor {anchor:?}, expected x={x}");
        }
    }
    b3_destroy_world(world);
}

#[test]
fn capsule_mesh_keeps_face_admitted_by_closest_distance() {
    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    // Native frame-139 triangle/capsule inputs. Closest-distance admission
    // succeeds although both clipped face points exceed speculative distance.
    let cases = [
        ([0.142406002,0.0393919982,0.261092007],
         [[0.403065205,0.173336983,0.869106531],[-0.421206474,0.247315764,-0.795042038],[-0.252500057,-0.0960446596,-0.877142191]],
         [-0.794671535,-0.479471982,0.372295231],
         [[0.0515942313,0.0311298296,-0.0241713542],[0.0752515569,0.036841277,0.0250684209]]),
        ([-0.142406002,0.0393919982,0.261092007],
         [[-0.0255892277,0.433032513,0.336993933],[-0.749417782,-1.16999626,-0.163224697],[-0.65510273,-1.05237508,-0.521910429]],
         [0.896159589,-0.433792561,0.0933913663],
         [[-0.149668753,0.0567847863,0.130711406],[-0.180530399,0.0614212416,0.223727062]]),
    ];
    for (endpoint, vertices, normal, points) in cases {
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0;3];
        wd.enable_sleep = false;
        wd.enable_continuous = false;
        let world = b3_create_world(gpu.clone(), &wd);
        let ground = b3_create_body(world, &b3_default_body_def());
        b3_create_mesh_shape(ground, &b3_default_shape_def(), &vertices, &[[0,1,2]], &[116], &[],
            &[MeshNode { lower: [-2.0;3], upper: [2.0;3], data: (1<<2)|3, triangle_offset: 0 }], [1.0;3]);
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        let body = b3_create_body(world, &bd);
        b3_create_capsule_shape(body, &b3_default_shape_def(), &Capsule {
            center1: [0.0;3], center2: endpoint, radius: 0.05,
        });
        b3_world_step_gpu(world, 1.0/60.0, 4);
        let contacts = pollster::block_on(b3_world_sync_contacts(world));
        let root = contacts.iter().find(|c| c.a != u32::MAX && c.count > 0 && c.manifold_link[1] == 0).expect("accepted capsule face");
        assert_eq!(root.count,2);
        for (actual,expected) in [root.nx,root.ny,root.nz].into_iter().zip(normal) {
            assert!((actual-expected).abs()<1e-6);
        }
        for (anchor,point) in [root.rb0,root.rb1].into_iter().zip(points) {
            for axis in 0..3 {
                assert!((anchor[axis]-(point[axis]-0.5*endpoint[axis])).abs()<1e-6,
                    "face anchor {anchor:?}, native point {point:?}");
            }
        }
        b3_destroy_world(world);
    }
}
