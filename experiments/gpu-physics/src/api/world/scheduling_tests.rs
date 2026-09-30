//! Bounded schedule-selection behavior; no relaxed physics tolerances.
use super::*;
use crate::api::b3_default_world_def;

#[test]
fn mixed_topology_schedules_match_through_impact() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
    let checkpoints = [1, 65, 78, 79, 90, 150, 330];
    let run = |component, automatic| {
        let mut def = b3_default_world_def();
        def.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &def);
        with_world_mut_no_sync(world, |w| {
            w.component_tgs_requested = component;
            w.component_tgs_automatic = automatic;
        });
        crate::scenes::create_mixed_topology(world);
        let mut snapshots = Vec::new();
        for step in 1..=330 {
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            b3_world_gpu_wait(world);
            if checkpoints.contains(&step) {
                let states = pollster::block_on(b3_world_sync_from_gpu(world));
                assert_eq!(states.len(), 12290);
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
                if step >= 90 {
                    let mut groups: Vec<_> = states[2..4098].to_vec();
                    for b in &mut groups { b.pos[0] -= 150.0; }
                    assert!(crate::dump::mixed_stacks_quality_error(&groups, step).is_none());
                    let mut sizes = std::collections::BTreeMap::new();
                    for i in 2..states.len() { *sizes.entry(root(i)).or_insert(0usize) += 1; }
                    for i in 2..4098 { assert_eq!(sizes[&root(i)], 2, "independent pair must remain disjoint"); }
                    let pile_max = (4098..states.len()).map(|i| sizes[&root(i)]).max().unwrap();
                    assert!(pile_max >= 6144, "at least three quarters of the pile must connect: {pile_max}");
                }
                assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
                snapshots.push(states);
            }
        }
        b3_destroy_world(world);
        snapshots
    };
    let global = run(false, false);
    let component = run(true, false);
    let automatic = run(true, true);
    let repeat = run(true, true);
    for (label, actual, epsilon) in [("component", &component, 1e-5), ("automatic", &automatic, 1e-5), ("repeat", &repeat, 0.0)] {
        let expected = if label == "repeat" { &automatic } else { &global };
        let mut worst = 0.0f32;
        for (a,b) in actual.iter().zip(expected) {
            for (a,b) in a.iter().zip(b) {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite());
                    worst = worst.max((x-y).abs());
                }
            }
        }
        eprintln!("mixed topology {label} worst difference {worst}");
        assert!(if epsilon == 0.0 { worst == 0.0 } else { worst < epsilon });
    }
    if let Some(error) = pollster::block_on(gpu.device.pop_error_scope()) { panic!("{error}"); }
}

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


#[test]
fn mixed_topology_schedule_transitions_preserve_constraints() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let run = |transitions| {
        let mut def = b3_default_world_def(); def.enable_sleep = false;
        let world = b3_create_world(gpu.clone(), &def);
        crate::scenes::create_mixed_topology(world);
        with_world_mut_no_sync(world, |w| w.component_tgs_requested = false);
        let mut snapshots = Vec::new();
        for step in 1..=160 {
            if transitions && [91,101,111,121,131].contains(&step) {
                with_world_mut_no_sync(world, |w| {
                    w.component_tgs_requested = step != 101 && step != 131;
                    w.component_tgs_automatic = step == 111;
                });
            }
            b3_world_step_gpu(world, 1.0/60.0, 4); b3_world_gpu_wait(world);
            if [90,100,110,120,130,160].contains(&step) {
                if transitions {
                    if step == 100 || step == 130 { assert_eq!(b3_world_last_solver_dispatches(world), 2); }
                    else { assert!(b3_world_last_solver_dispatches(world) > 2); }
                }
                assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
                snapshots.push(pollster::block_on(b3_world_sync_from_gpu(world)));
            }
        }
        b3_destroy_world(world); snapshots
    };
    let global = run(false); let transitions = run(true); let repeat = run(true);
    for (actual,epsilon) in [(&transitions,1e-5),(&repeat,0.0)] {
        let expected = if epsilon==0.0 {&transitions} else {&global};
        for (a,b) in actual.iter().zip(expected) {
            for (a,b) in a.iter().zip(b) {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite());
                    assert!(if epsilon==0.0 {x==y} else {(x-y).abs()<epsilon});
                }
            }
        }
    }
}
