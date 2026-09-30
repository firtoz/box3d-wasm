//! Bounded schedule-selection behavior; no relaxed physics tolerances.
use super::*;
use crate::api::b3_default_world_def;

#[test]
fn automatic_schedule_preserves_independent_groups() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
    let run = |component, automatic| {
        let mut def = b3_default_world_def();
        def.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &def);
        with_world_mut_no_sync(world, |w| {
            w.component_tgs_requested = component;
            w.component_tgs_automatic = automatic;
        });
        crate::scenes::create_mixed_stacks(world, 4096);
        let mut snapshots = Vec::new();
        for step in 1..=330 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            b3_world_gpu_wait(world);
            if [1, 90, 150, 330].contains(&step) {
                let states = pollster::block_on(b3_world_sync_from_gpu(world));
                if step >= 90 {
                    assert!(crate::dump::mixed_stacks_quality_error(&states, step).is_none());
                    if component { assert_eq!(b3_world_last_solver_dispatches(world), 2); }
                    else { assert!(b3_world_last_solver_dispatches(world) > 2); }
                }
                let root = |mut i: usize| loop {
                    let parent = states[i].island_id as usize;
                    if parent == i { break i; }
                    assert!(parent < i);
                    i = parent;
                };
                let contacts = with_world_mut_no_sync(world, |w| {
                    pollster::block_on(w.sim.as_mut().unwrap().read_contacts())
                }).unwrap();
                for c in contacts.iter().filter(|c| c.count > 0 && c.a != u32::MAX) {
                    let (a,b) = (c.a as usize, c.b as usize);
                    if states[a].inv_mass > 0.0 && states[b].inv_mass > 0.0 {
                        assert_eq!(root(a), root(b), "contact crosses islands at step {step}");
                    }
                }
                assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
                snapshots.push(states);
            }
        }
        b3_destroy_world(world);
        snapshots
    };
    let global = run(false, false);
    let automatic = run(true, true);
    let repeat = run(true, true);
    let forced = run(true, false);
    for (label, states, epsilon) in [("global", &global, 1e-5), ("repeat", &repeat, 0.0), ("component", &forced, 0.0)] {
        let mut worst = 0.0f32;
        for (actual, expected) in states.iter().zip(&automatic) {
            assert_eq!(actual.len(), expected.len());
            for (a,b) in actual.iter().zip(expected) {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite());
                    worst = worst.max((x-y).abs());
                }
            }
        }
        eprintln!("independent 4096 bodies {label} max state difference {worst}");
        assert!(if epsilon == 0.0 { worst == 0.0 } else { worst < epsilon });
    }
    if let Some(error) = pollster::block_on(gpu.device.pop_error_scope()) { panic!("{error}"); }
}

#[test]
fn automatic_schedule_rechecks_topology_and_explicit_overrides() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let cfg = crate::types::DemoConfig {
        scene: crate::types::DemoScene::FallingCubes,
        body_count: 15000, body_count_explicit: true, contacts: true, jacobi: false,
    };
    let world = crate::scenes::build_demo_world(gpu, &cfg);
    b3_world_enable_sleeping(world, false);
    with_world_mut_no_sync(world, |w| {
        w.component_tgs_requested = true;
        w.component_tgs_automatic = true;
    });
    let step = || { b3_world_step_gpu(world, 1.0 / 60.0, 4); b3_world_gpu_wait(world); };
    for _ in 0..90 { step(); }
    assert!(b3_world_last_solver_dispatches(world) > 2, "dense hint must select global");
    with_world_mut_no_sync(world, |w| w.component_tgs_automatic = false);
    step();
    assert_eq!(b3_world_last_solver_dispatches(world), 2, "explicit component override");
    with_world_mut_no_sync(world, |w| w.component_tgs_requested = false);
    step();
    assert!(b3_world_last_solver_dispatches(world) > 2, "explicit global override");
    with_world_mut_no_sync(world, |w| {
        w.component_tgs_requested = true;
        w.component_tgs_automatic = true;
    });
    let mut bd = crate::api::b3_default_body_def();
    bd.body_type = BodyType::Dynamic;
    bd.position = [100.0, 100.0, 0.0];
    let added = b3_create_body(world, &bd);
    crate::api::b3_create_hull_shape(added, &crate::api::b3_default_shape_def(), &crate::api::b3_make_cube_hull(0.5));
    step();
    assert_eq!(b3_world_last_solver_dispatches(world), 2, "topology upload discards old hint");
    step();
    assert!(b3_world_last_solver_dispatches(world) > 2, "new completed status restores global");
    let states = pollster::block_on(b3_world_sync_from_gpu(world));
    assert_eq!(states.len(), 15002);
    assert!(states.iter().flat_map(|b| b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)).all(|v| v.is_finite()));
    assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
    b3_destroy_world(world);
}

#[test]
fn automatic_dense_schedule_repeats_exactly() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let checkpoints = [1, 65, 78, 79, 90, 150, 330];
    let run = || {
        let cfg = crate::types::DemoConfig {
            scene: crate::types::DemoScene::FallingCubes,
            body_count: 15000, body_count_explicit: true, contacts: true, jacobi: false,
        };
        let world = crate::scenes::build_demo_world(gpu.clone(), &cfg);
        with_world_mut_no_sync(world, |w| {
            w.component_tgs_requested = true;
            w.component_tgs_automatic = true;
        });
        b3_world_enable_sleeping(world, false);
        let mut snapshots = Vec::new();
        for step in 1..=330 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            b3_world_gpu_wait(world);
            if checkpoints.contains(&step) {
                snapshots.push(pollster::block_on(b3_world_sync_from_gpu(world)));
                assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
            }
        }
        assert!(b3_world_last_solver_dispatches(world) > 2, "dense auto must actually select global");
        b3_destroy_world(world);
        snapshots
    };
    let first = run();
    let repeat = run();
    for (checkpoint, (actual, expected)) in first.iter().zip(&repeat).enumerate() {
        assert_eq!(actual.len(), expected.len());
        for (a,b) in actual.iter().zip(expected) {
            for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                assert!(x.is_finite() && y.is_finite());
                assert_eq!(x, y, "automatic repeat at step {}", checkpoints[checkpoint]);
            }
        }
    }
}
