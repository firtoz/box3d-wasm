use std::path::PathBuf;
use std::time::Instant;

use crate::api::{
    b3_destroy_world, b3_world_begin_timing, b3_world_body_count, b3_world_enable_sleeping,
    b3_world_finish_timing, b3_world_gpu_allocations, b3_world_gpu_report_name, b3_world_gpu_wait,
    b3_world_gpu_wait_with_mirror, b3_world_step_gpu, b3_world_sync_contacts,
    b3_world_draw_items, DrawItemC,
    b3_world_sync_from_gpu, b3_world_sync_phase_words, b3_world_live_step_stats,
};
use crate::dump::{
    box_stack_column_error, box_stack_rest_error, dumps_equal, encode_run, first_mismatch,
    max_body_error, oracle_frame, physics_quality_error, read_b3or, rest_json_fields,
    should_capture_state, should_dump, single_box_rest_error,
};
use crate::scenes::build_demo_world;
use crate::sim::GpuDevice;
use crate::trace::{
    append_phase_probe, begin_phase_probe, diff_contacts, dump_matched_contacts, from_gpu_contact,
    from_gpu_contact_unfiltered, gpu_contact_diagnostics, read_trace,
};
use crate::types::{
    ContactGpu, DemoConfig, DemoScene, DEFAULT_SUB_STEPS, DUMP_CHECKPOINTS, FIXED_DT, FLAG_SLEEP,
};

pub struct HeadlessConfig {
    pub body_count: u32,
    pub body_count_explicit: bool,
    pub contacts: bool,
    pub scene: DemoScene,
    pub max_step: u32,
    pub dump_path: Option<PathBuf>,
    pub self_test: bool,
    pub jacobi: bool,
    pub sleep: bool,
}

fn make_demo(
    scene: DemoScene,
    body_count: u32,
    contacts: bool,
    jacobi: bool,
    body_count_explicit: bool,
) -> DemoConfig {
    DemoConfig {
        body_count,
        body_count_explicit,
        contacts,
        scene,
        jacobi,
    }
}

pub async fn run_once(cfg: &HeadlessConfig) -> Result<(Vec<u8>, f32), String> {
    let gpu = GpuDevice::new(None).await?;
    let demo = make_demo(
        cfg.scene,
        cfg.body_count,
        cfg.contacts,
        cfg.jacobi,
        cfg.body_count_explicit,
    );
    let world = build_demo_world(gpu, &demo);
    b3_world_enable_sleeping(world, cfg.sleep);
    let mut checkpoints = Vec::new();
    let capture_phases = std::env::var("GPU_PHYSICS_AB")
        .unwrap_or_default()
        .split(',')
        .any(|flag| flag.trim() == "phase-capture");
    let mut phase_probe = begin_phase_probe(b3_world_body_count(world), 18);

    let t0 = Instant::now();
    for step in 0..=cfg.max_step {
        if should_capture_state(cfg.scene, step, cfg.max_step) {
            let bodies = b3_world_sync_from_gpu(world).await;
            if let Some(err) = physics_quality_error(cfg.scene, &bodies, step) {
                b3_destroy_world(world);
                return Err(err);
            }
            if cfg.scene == DemoScene::SingleBox {
                if let Some(err) = single_box_rest_error(&bodies) {
                    eprintln!("single-box rest |y-0.5|={err:.3e} at step {step}");
                }
            }
            if matches!(
                cfg.scene,
                DemoScene::BoxStack | DemoScene::SphereStack | DemoScene::CapsuleStack
            ) {
                if let Some((bottom, gap)) = box_stack_rest_error(&bodies) {
                    eprintln!(
                        "{} rest bottom_err={bottom:.4} max_|gap-1|={gap:.4} at step {step}",
                        cfg.scene.slug()
                    );
                }
                if cfg.scene == DemoScene::BoxStack {
                    if let Some((order_gap, fallen, xz)) = box_stack_column_error(&bodies) {
                        let dyns: Vec<_> = bodies.iter().filter(|b| b.inv_mass > 0.0).collect();
                        let speed = dyns
                            .iter()
                            .map(|b| {
                                (b.vel[0] * b.vel[0] + b.vel[1] * b.vel[1] + b.vel[2] * b.vel[2])
                                    .sqrt()
                            })
                            .fold(0.0f32, f32::max);
                        eprintln!(
                            "  column order_gap={order_gap:.4} fallen={fallen} xz={xz:.4} n={} max_speed={speed:.4}",
                            dyns.len()
                        );
                    }
                }
            }
            checkpoints.push((step, bodies));
            if let Some(stats) = b3_world_live_step_stats(world).await {
                if stats.capacity_loss() {
                    b3_destroy_world(world);
                    return Err(format!(
                        "capacity loss at step {step}: {}",
                        stats.sticky.loss_detail()
                    ));
                }
            }
        }
        if step == cfg.max_step {
            break;
        }
        b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
        let resulting_step = step + 1;
        if capture_phases && (13..=30).contains(&resulting_step) {
            let words = b3_world_sync_phase_words(world).await;
            append_phase_probe(&mut phase_probe, resulting_step, &words);
        }
    }
    b3_world_gpu_wait(world);
    if let Some(stats) = b3_world_live_step_stats(world).await {
        if stats.capacity_loss() {
            b3_destroy_world(world);
            return Err(format!(
                "capacity loss at end: {}",
                stats.drops.loss_detail()
            ));
        }
    }
    let ms = t0.elapsed().as_secs_f32() * 1000.0;
    let blob = encode_run(&checkpoints);
    if cfg.scene == DemoScene::SingleBox {
        if let Some((step, bodies)) = checkpoints.last() {
            if let Some(err) = single_box_rest_error(bodies) {
                if err > 1e-2 {
                    b3_destroy_world(world);
                    return Err(format!(
                        "single-box rest |y-0.5|={err:.3e} at step {step} (want < 1cm vs Box3D COM 0.5)"
                    ));
                }
            }
            if *step >= 60 {
                let sleeping = bodies
                    .iter()
                    .filter(|body| body.inv_mass > 0.0)
                    .all(|body| body.flags & FLAG_SLEEP != 0);
                if !sleeping {
                    b3_destroy_world(world);
                    return Err(format!(
                        "single-box dynamic body is still awake at step {step} (expected FLAG_SLEEP)"
                    ));
                }
            }
        }
    }
    if capture_phases {
        let path = std::env::var_os("GPU_PHASE_PROBE_PATH")
            .map(PathBuf::from)
            .unwrap_or_else(|| PathBuf::from("phase-probe.b3pr"));
        std::fs::write(&path, &phase_probe).map_err(|error| error.to_string())?;
        println!("wrote phase probe {}", path.display());
    }
    b3_destroy_world(world);
    Ok((blob, ms))
}

pub async fn run_headless(cfg: HeadlessConfig) -> Result<(), String> {
    if cfg.self_test {
        let (a, ms_a) = run_once(&cfg).await?;
        let (b, ms_b) = run_once(&cfg).await?;
        if dumps_equal(&a, &b) {
            println!(
                "self-test PASS: GPU-vs-GPU dumps identical ({} bytes, checkpoints {:?}, scene={:?})",
                a.len(),
                DUMP_CHECKPOINTS,
                cfg.scene
            );
            println!(
                "adapter run1+run2 wall {:.2} ms / {:.2} ms for {} steps, contacts={}",
                ms_a, ms_b, cfg.max_step, cfg.contacts
            );
            if let Some(path) = &cfg.dump_path {
                std::fs::write(path, &a).map_err(|e| e.to_string())?;
                println!("wrote {path:?}");
            }
            Ok(())
        } else {
            let at = first_mismatch(&a, &b).unwrap_or(0);
            Err(format!(
                "self-test FAIL: dumps differ at byte {at} (len {} vs {})",
                a.len(),
                b.len()
            ))
        }
    } else {
        let (blob, ms) = run_once(&cfg).await?;
        println!(
            "headless {} steps, scene={:?}, contacts={}, wall {:.2} ms, dump {} bytes",
            cfg.max_step,
            cfg.scene,
            cfg.contacts,
            ms,
            blob.len()
        );
        if let Some(path) = &cfg.dump_path {
            std::fs::write(path, &blob).map_err(|e| e.to_string())?;
            println!("wrote {path:?}");
        }
        Ok(())
    }
}

const METRICS_WARMUP: u32 = 60;
const SPHERE_SCALING_COUNTS: [u32; 4] = [64, 256, 1024, 4096];

fn json_escape(s: &str) -> String {
    s.replace('\\', "\\\\").replace('"', "\\\"")
}

#[derive(Clone, Debug)]
struct MetricRun {
    wall_ms_per_step: f64,
    timing: crate::sim::TimingWindow,
}

#[derive(Clone, Debug)]
struct SceneMeasurement {
    adapter: String,
    bodies: u32,
    allocations: crate::sim::AllocationStats,
    runs: Vec<MetricRun>,
}

async fn measure_timing_runs(
    gpu: GpuDevice,
    demo: &DemoConfig,
    warmup_steps: u32,
    timed_steps: u32,
    run_count: u32,
) -> Result<SceneMeasurement, String> {
    let mut runs = Vec::new();
    let mut adapter = String::new();
    let mut bodies = 0;
    let mut allocations = crate::sim::AllocationStats::default();
    for _ in 0..run_count {
        let world = build_demo_world(gpu.clone(), demo);
        crate::api::b3_world_ensure_gpu(world);
        for _ in 0..warmup_steps {
            b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
        }
        b3_world_gpu_wait(world);
        if adapter.is_empty() {
            adapter = b3_world_gpu_report_name(world);
            bodies = b3_world_body_count(world);
            allocations = b3_world_gpu_allocations(world).unwrap_or_default();
        }
        b3_world_begin_timing(world, timed_steps);
        let t0 = Instant::now();
        for _ in 0..timed_steps {
            b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
        }
        b3_world_gpu_wait(world);
        let wall_ms_per_step = t0.elapsed().as_secs_f64() * 1000.0 / f64::from(timed_steps);
        let timing = b3_world_finish_timing(world)
            .await
            .ok_or_else(|| "timing window produced no samples".to_string())?;
        runs.push(MetricRun {
            wall_ms_per_step,
            timing,
        });
        b3_destroy_world(world);
    }
    Ok(SceneMeasurement {
        adapter,
        bodies,
        allocations,
        runs,
    })
}

async fn measure_rest(gpu: GpuDevice, demo: &DemoConfig, timed_steps: u32) -> Vec<String> {
    let world = build_demo_world(gpu, demo);
    crate::api::b3_world_ensure_gpu(world);
    let mut rest_json = Vec::new();
    for step in 0..=timed_steps {
        if should_dump(step) {
            let bodies = b3_world_sync_from_gpu(world).await;
            let mut fields = vec![format!("\"step\":{step}")];
            fields.extend(rest_json_fields(demo.scene, &bodies));
            rest_json.push(format!("{{{}}}", fields.join(",")));
        }
        if step == timed_steps {
            break;
        }
        b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
    }
    b3_destroy_world(world);
    rest_json
}

fn percentile(values: &[f64], percentile: f64) -> f64 {
    if values.is_empty() {
        return 0.0;
    }
    let mut sorted = values.to_vec();
    sorted.sort_by(f64::total_cmp);
    let rank = (percentile.clamp(0.0, 1.0) * sorted.len() as f64).ceil() as usize;
    sorted[rank.saturating_sub(1).min(sorted.len() - 1)]
}

fn mean_f64(values: &[f64]) -> f64 {
    if values.is_empty() {
        0.0
    } else {
        values.iter().sum::<f64>() / values.len() as f64
    }
}

fn sphere_scaling_window(body_count: u32, requested_steps: u32) -> (u32, u32) {
    match body_count {
        0..=256 => (METRICS_WARMUP, requested_steps),
        257..=1024 => (10, requested_steps.min(30)),
        _ => (1, requested_steps.min(1)),
    }
}

fn optional_mean(values: impl Iterator<Item = Option<f64>>) -> Option<f64> {
    let values: Vec<f64> = values.flatten().collect();
    (!values.is_empty()).then(|| mean_f64(&values))
}

fn optional_summary_json(values: impl Iterator<Item = Option<f64>>) -> String {
    let values: Vec<f64> = values.flatten().collect();
    if values.is_empty() {
        return "null".to_string();
    }
    format!(
        "{{\"mean_ms\":{:.4},\"p50_ms\":{:.4},\"p95_ms\":{:.4}}}",
        mean_f64(&values),
        percentile(&values, 0.50),
        percentile(&values, 0.95)
    )
}

fn measurement_json(measurement: &SceneMeasurement, rest: Option<&[String]>) -> String {
    let wall: Vec<f64> = measurement
        .runs
        .iter()
        .map(|run| run.wall_ms_per_step)
        .collect();
    let encode: Vec<f64> = measurement
        .runs
        .iter()
        .map(|run| run.timing.cpu_encode_ms)
        .collect();
    let submit: Vec<f64> = measurement
        .runs
        .iter()
        .map(|run| run.timing.cpu_submit_ms)
        .collect();
    let p50 = percentile(&wall, 0.50);
    let p95 = percentile(&wall, 0.95);
    let gpu_collide = optional_mean(measurement.runs.iter().map(|run| run.timing.gpu_collide_ms));
    let gpu_solve = optional_mean(measurement.runs.iter().map(|run| run.timing.gpu_solve_ms));
    let gpu_integrate = optional_mean(
        measurement
            .runs
            .iter()
            .map(|run| run.timing.gpu_integrate_ms),
    );
    let gpu_fields = match (gpu_collide, gpu_solve, gpu_integrate) {
        (Some(collide), Some(solve), Some(integrate)) => format!(
            ",\n      \"gpu_collide_ms\": {collide:.4},\n      \"gpu_solve_ms\": {solve:.4},\n      \"gpu_integrate_ms\": {integrate:.4}"
        ),
        _ => String::new(),
    };
    let samples = wall
        .iter()
        .map(|value| format!("{value:.4}"))
        .collect::<Vec<_>>()
        .join(",");
    let workload = |avg: fn(&MetricRun) -> f64, peak: fn(&MetricRun) -> u32| {
        let averages: Vec<f64> = measurement.runs.iter().map(avg).collect();
        let max_peak = measurement.runs.iter().map(peak).max().unwrap_or(0);
        format!("{{\"avg\":{:.2},\"peak\":{max_peak}}}", mean_f64(&averages))
    };
    let allocations = measurement.allocations;
    let rest_field = rest
        .map(|items| format!(",\n      \"rest\": [{}]", items.join(", ")))
        .unwrap_or_default();
    format!(
        "{{\n      \"bodies\": {},\n      \"wall_ms\": {:.3},\n      \"ms_per_step\": {p50:.4},\n      \"p95_ms_per_step\": {p95:.4}{gpu_fields},\n      \"timing\": {{\"runs\":{},\"samples_ms_per_step\":[{samples}],\"p50_ms_per_step\":{p50:.4},\"p95_ms_per_step\":{p95:.4},\"cpu_encode\":{{\"mean_ms\":{:.4},\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\"cpu_submit\":{{\"mean_ms\":{:.4},\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\"gpu_collide\":{},\"gpu_solve\":{},\"gpu_integrate\":{}}},\n      \"workload\": {{\"candidate_pairs\":{},\"cell_inserts\":{},\"unique_pairs\":{},\"narrowphase_pairs\":{},\"occupied_contact_slots\":{},\"capacity_drops\":{}}},\n      \"allocations\": {{\"body_bytes\":{},\"shape_bytes\":{},\"contact_bytes\":{},\"joint_bytes\":{},\"scratch_bytes\":{},\"atom_bytes\":{},\"total_bytes\":{}}}{rest_field}\n    }}",
        measurement.bodies,
        p50 * measurement.runs.first().map_or(0.0, |run| f64::from(run.timing.steps)),
        measurement.runs.len(),
        mean_f64(&encode),
        percentile(&encode, 0.50),
        percentile(&encode, 0.95),
        mean_f64(&submit),
        percentile(&submit, 0.50),
        percentile(&submit, 0.95),
        optional_summary_json(measurement.runs.iter().map(|run| run.timing.gpu_collide_ms)),
        optional_summary_json(measurement.runs.iter().map(|run| run.timing.gpu_solve_ms)),
        optional_summary_json(measurement.runs.iter().map(|run| run.timing.gpu_integrate_ms)),
        workload(
            |run| run.timing.workload.candidate_pairs_avg,
            |run| run.timing.workload.candidate_pairs_peak
        ),
        workload(
            |run| run.timing.workload.cell_inserts_avg,
            |run| run.timing.workload.cell_inserts_peak
        ),
        workload(
            |run| run.timing.workload.unique_pairs_avg,
            |run| run.timing.workload.unique_pairs_peak
        ),
        workload(
            |run| run.timing.workload.narrowphase_pairs_avg,
            |run| run.timing.workload.narrowphase_pairs_peak
        ),
        workload(
            |run| run.timing.workload.occupied_contact_slots_avg,
            |run| run.timing.workload.occupied_contact_slots_peak
        ),
        workload(
            |run| run.timing.workload.capacity_drops_avg,
            |run| run.timing.workload.capacity_drops_peak
        ),
        allocations.body_bytes,
        allocations.shape_bytes,
        allocations.contact_bytes,
        allocations.joint_bytes,
        allocations.scratch_bytes,
        allocations.atom_bytes,
        allocations.total_bytes,
    )
}

#[cfg(test)]
mod metrics_tests {
    use super::{percentile, sphere_scaling_window};

    #[test]
    fn repeated_timing_percentiles_use_nearest_rank() {
        let values = [5.0, 1.0, 4.0, 2.0, 3.0];
        assert_eq!(percentile(&values, 0.50), 3.0);
        assert_eq!(percentile(&values, 0.95), 5.0);
        assert_eq!(percentile(&values, 0.0), 1.0);
        assert_eq!(percentile(&[], 0.95), 0.0);
    }

    #[test]
    fn scaling_windows_bound_expensive_workloads() {
        assert_eq!(sphere_scaling_window(64, 300), (60, 300));
        assert_eq!(sphere_scaling_window(1024, 300), (10, 30));
        assert_eq!(sphere_scaling_window(4096, 300), (1, 1));
    }
}

/// Physics-only `ms_per_step` plus rest checkpoints. One object covering every demo scene.
pub async fn write_snapshot_metrics(
    path: PathBuf,
    timed_steps: u32,
    contacts: bool,
    body_count: u32,
    timing_runs: u32,
) -> Result<(), String> {
    let steps = timed_steps.max(1);
    let timing_runs = timing_runs.max(1);
    let gpu = GpuDevice::new(None).await?;
    let mut adapter = String::new();
    let mut scene_objs = Vec::new();
    for scene in DemoScene::ALL {
        let skip = std::env::var("SKIP_SCENES").unwrap_or_default();
        if skip.split(',').any(|s| s == scene.slug()) {
            eprintln!("metrics {} skipped (SKIP_SCENES)", scene.slug());
            continue;
        }
        let demo = make_demo(scene, body_count, contacts, false, false);
        eprintln!(
            "metrics {} ({} runs, warmup {METRICS_WARMUP}, time {steps})",
            scene.slug(),
            timing_runs
        );
        let measurement =
            measure_timing_runs(gpu.clone(), &demo, METRICS_WARMUP, steps, timing_runs).await?;
        if adapter.is_empty() {
            adapter = measurement.adapter.clone();
        }
        let rest = measure_rest(gpu.clone(), &demo, steps).await;
        let wall: Vec<f64> = measurement
            .runs
            .iter()
            .map(|run| run.wall_ms_per_step)
            .collect();
        let p50 = percentile(&wall, 0.50);
        let p95 = percentile(&wall, 0.95);
        scene_objs.push(format!(
            "    \"{}\": {}",
            scene.slug(),
            measurement_json(&measurement, Some(&rest))
        ));
        eprintln!(
            "  {} bodies={} p50={p50:.3} p95={p95:.3} ms/step",
            scene.slug(),
            measurement.bodies
        );
    }
    let mut scaling = Vec::new();
    if !std::env::var("SKIP_SCENES")
        .unwrap_or_default()
        .split(',')
        .any(|scene| scene == DemoScene::Spheres.slug())
    {
        let scaling_max = std::env::var("GPU_SCALING_MAX")
            .ok()
            .and_then(|value| value.parse::<u32>().ok())
            .unwrap_or(u32::MAX);
        for body_count in SPHERE_SCALING_COUNTS
            .into_iter()
            .filter(|body_count| *body_count <= scaling_max)
        {
            let (scaling_warmup, scaling_steps) = sphere_scaling_window(body_count, steps);
            let demo = make_demo(DemoScene::Spheres, body_count, contacts, false, true);
            eprintln!(
                "scaling spheres/{body_count} ({} runs, warmup {scaling_warmup}, time {scaling_steps})",
                timing_runs,
            );
            // Large fixed-capacity buffers from earlier scenes may remain pending in
            // wgpu's resource recycler. Keep each scale point isolated so 4096 bodies
            // measures the workload rather than accumulated device pressure.
            let scaling_gpu = GpuDevice::new(None).await?;
            let measurement = measure_timing_runs(
                scaling_gpu,
                &demo,
                scaling_warmup,
                scaling_steps,
                timing_runs,
            )
            .await?;
            scaling.push(format!(
                "      {{\"dynamic_bodies\":{body_count},\"warmup_steps\":{scaling_warmup},\"timed_steps\":{scaling_steps},\"metrics\":{}}}",
                measurement_json(&measurement, None)
            ));
        }
    }
    let version = std::env::var("GPU_COMPARE_VERSION")
        .ok()
        .filter(|s| !s.is_empty());
    let version_field = version
        .as_ref()
        .map(|v| format!("  \"version\": \"{}\",\n", json_escape(v)))
        .unwrap_or_default();
    let json = format!(
        "{{\n{version_field}  \"adapter\": \"{}\",\n  \"sub_steps\": {},\n  \"warmup_steps\": {METRICS_WARMUP},\n  \"timed_steps\": {steps},\n  \"timing_runs\": {},\n  \"scenes\": {{\n{}\n  }},\n  \"scaling\": {{\n    \"spheres\": [\n{}\n    ]\n  }}\n}}\n",
        json_escape(&adapter),
        DEFAULT_SUB_STEPS,
        timing_runs,
        scene_objs.join(",\n"),
        scaling.join(",\n")
    );
    if let Some(parent) = path.parent() {
        std::fs::create_dir_all(parent).map_err(|e| e.to_string())?;
    }
    std::fs::write(&path, json).map_err(|e| e.to_string())?;
    eprintln!("wrote {}", path.display());
    Ok(())
}

/// Compare GPU vs a Box3D `B3OR` dump at checkpoints (default epsilon 1e-4).
#[allow(dead_code)]
pub struct OracleTol {
    pub pos: f32,
    pub vel: f32,
    pub quat: f32,
    pub omega: f32,
}

pub async fn compare_oracle_dump(
    dump_path: PathBuf,
    scene: DemoScene,
    contacts: bool,
    body_count: u32,
    body_count_explicit: bool,
    epsilon: f32,
    sleep: bool,
) -> Result<(), String> {
    compare_oracle_dump_with_tol(
        dump_path,
        scene,
        contacts,
        body_count,
        body_count_explicit,
        OracleTol {
            pos: epsilon,
            vel: epsilon,
            quat: epsilon,
            omega: epsilon,
        },
        sleep,
    )
    .await
}

pub async fn compare_oracle_dump_with_tol(
    dump_path: PathBuf,
    scene: DemoScene,
    contacts: bool,
    body_count: u32,
    body_count_explicit: bool,
    tol: OracleTol,
    sleep: bool,
) -> Result<(), String> {
    let (frames, nbody, cpu_bodies) = read_b3or(&dump_path)?;
    if frames == 0 || nbody == 0 {
        return Err(format!(
            "{} is empty (frames={frames} bodies={nbody})",
            dump_path.display()
        ));
    }
    let checkpoints: Vec<u32> = if scene == DemoScene::HighResistance {
        let mut steps: Vec<u32> = (300..=600).collect();
        let mut s = 900u32;
        while s < frames {
            steps.push(s);
            s += 300;
        }
        if frames > 0 {
            steps.push(frames - 1);
        }
        steps.sort_unstable();
        steps.dedup();
        steps
    } else {
        let mut steps: Vec<u32> = DUMP_CHECKPOINTS
            .iter()
            .copied()
            .filter(|&s| s < frames)
            .collect();
        if frames > 0 {
            steps.push(frames - 1);
        }
        steps.sort_unstable();
        steps.dedup();
        steps
    };
    let gpu_dev = GpuDevice::new(None).await?;
    let demo = make_demo(scene, body_count, contacts, false, body_count_explicit);
    let world = build_demo_world(gpu_dev, &demo);
    crate::api::b3_world_enable_sleeping(world, sleep);
    crate::api::b3_world_ensure_gpu(world);
    let gpu_n = b3_world_body_count(world);
    if gpu_n != nbody {
        b3_destroy_world(world);
        return Err(format!(
            "body count GPU {gpu_n} vs oracle {nbody} ({})",
            scene.slug()
        ));
    }
    let mut worst = (0.0f32, 0u32, "pos");
    let mut lines = Vec::new();
    let mut prev = 0u32;
    let mut comparisons = 0u32;
    let mut y_peak: Vec<(f32, f32)> = vec![(f32::MAX, f32::MIN); nbody as usize];
    for step in checkpoints {
        if step >= frames {
            b3_destroy_world(world);
            return Err(format!(
                "requested frame {step} missing (dump has {frames} frames)"
            ));
        }
        if step > prev {
            for _ in prev..step {
                b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
            }
        }
        prev = step;
        let gpu = b3_world_sync_from_gpu(world).await;
        let Some(cpu) = oracle_frame(&cpu_bodies, step, nbody) else {
            b3_destroy_world(world);
            return Err(format!("oracle frame {step} missing"));
        };
        if gpu.len() != cpu.len() {
            b3_destroy_world(world);
            return Err(format!(
                "step {step}: GPU {} bodies vs oracle {}",
                gpu.len(),
                cpu.len()
            ));
        }
        let Some((pos, vel, quat, omega)) = max_body_error(&gpu, cpu) else {
            b3_destroy_world(world);
            return Err(format!("step {step}: body compare failed"));
        };
        if let Some(err) = physics_quality_error(scene, &gpu, step) {
            b3_destroy_world(world);
            return Err(err);
        }
        if scene == DemoScene::HighResistance && (300..=600).contains(&step) {
            for (i, b) in gpu.iter().enumerate() {
                y_peak[i].0 = y_peak[i].0.min(b.pos[1]);
                y_peak[i].1 = y_peak[i].1.max(b.pos[1]);
            }
            if let Some(err) = crate::dump::high_resistance_quality_error(&gpu, step) {
                b3_destroy_world(world);
                return Err(err);
            }
            for (i, (g, c)) in gpu.iter().zip(cpu.iter()).enumerate() {
                if g.inv_mass <= 0.0 {
                    continue;
                }
                let dy = (g.pos[1] - c.pos[1]).abs();
                if dy > 0.002 {
                    b3_destroy_world(world);
                    return Err(format!(
                        "high-resistance body {i} y GPU {} vs CPU {} at {step}",
                        g.pos[1], c.pos[1]
                    ));
                }
                let mut dq = (g.rot[0] - c.rot[0]).abs()
                    + (g.rot[1] - c.rot[1]).abs()
                    + (g.rot[2] - c.rot[2]).abs()
                    + (g.rot[3] - c.rot[3]).abs();
                let dq_neg = (g.rot[0] + c.rot[0]).abs()
                    + (g.rot[1] + c.rot[1]).abs()
                    + (g.rot[2] + c.rot[2]).abs()
                    + (g.rot[3] + c.rot[3]).abs();
                dq = dq.min(dq_neg);
                if dq > 0.02 {
                    b3_destroy_world(world);
                    return Err(format!(
                        "high-resistance body {i} quat GPU {:?} vs CPU {:?} at {step}",
                        g.rot, c.rot
                    ));
                }
            }
        }
        comparisons += 1;
        lines.push(format!(
            "  step {step}: max |Δpos|={pos:.3e} |Δvel|={vel:.3e} |Δq|={quat:.3e} |Δω|={omega:.3e}"
        ));
        for (v, name) in [(pos, "pos"), (vel, "vel"), (quat, "q"), (omega, "omega")] {
            if v > worst.0 {
                worst = (v, step, name);
            }
        }
        let over = (pos > tol.pos)
            || (vel > tol.vel)
            || (quat > tol.quat)
            || (omega > tol.omega);
        if over && scene != DemoScene::HighResistance && scene != DemoScene::MixedStacks {
            b3_destroy_world(world);
            return Err(format!(
                "{} exceeds tolerance at step {step} pos={pos:.3e} vel={vel:.3e} q={quat:.3e} ω={omega:.3e}",
                scene.slug()
            ));
        }
        if scene == DemoScene::MixedStacks && step >= 60 {
            for (i, (g, c)) in gpu.iter().zip(cpu.iter()).enumerate() {
                if g.inv_mass <= 0.0 {
                    continue;
                }
                if (g.pos[1] - c.pos[1]).abs() > 0.002 {
                    b3_destroy_world(world);
                    return Err(format!(
                        "mixed-stacks body {i} y GPU {} vs CPU {} at {step}",
                        g.pos[1], c.pos[1]
                    ));
                }
            }
        }
        if scene == DemoScene::HighResistance && (300..=600).contains(&step) {
            if vel > 0.01 || omega > 0.01 {
                b3_destroy_world(world);
                return Err(format!(
                    "high-resistance speed at step {step} vel={vel:.3e} ω={omega:.3e}"
                ));
            }
        }
    }
    if scene == DemoScene::HighResistance {
        for (i, (lo, hi)) in y_peak.iter().enumerate() {
            if *hi < *lo {
                continue;
            }
            if hi - lo > 0.002 {
                b3_destroy_world(world);
                return Err(format!(
                    "high-resistance body {i} COM-y peak-to-peak {}",
                    hi - lo
                ));
            }
        }
    }
    b3_destroy_world(world);
    println!("compare {} vs {}", scene.slug(), dump_path.display());
    for l in &lines {
        println!("{l}");
    }
    println!(
        "  worst {} at step {} = {:.3e} (pos {} vel {} q {} ω {})",
        worst.2, worst.1, worst.0, tol.pos, tol.vel, tol.quat, tol.omega
    );
    if scene == DemoScene::HighResistance {
        println!(
            "  high-resistance: COM-y 0.002 m, support, speed; quaternion is component L1 not geodesic angle; generic --epsilon is not applied to |Δpos|"
        );
    }
    if scene == DemoScene::MixedStacks {
        println!(
            "  mixed-stacks: COM-y vs CPU 0.002 m, layer/support/xz/speed; generic --epsilon is not applied to |Δpos|"
        );
    }
    if comparisons == 0 {
        return Err("zero comparisons".into());
    }
    Ok(())
}

fn dump_gpu_solver_pairs(raw: &[ContactGpu]) {
    println!("    GPU solver pairs (all, including speculative):");
    for c in raw {
        if let Some((m, smin)) = from_gpu_contact_unfiltered(c) {
            println!(
                "      ({},{}) pts={} n=({:.3},{:.3},{:.3}) smin={smin:.4e}",
                m.a, m.b, m.count, m.n[0], m.n[1], m.n[2]
            );
        }
    }
}

fn dump_gpu_contact_diagnostics(raw: &[ContactGpu]) {
    println!("    GPU contact/cache/graph diagnostics:");
    for d in gpu_contact_diagnostics(raw) {
        println!(
            "      slot={} gen={} flags={:#x} order={} pair=({},{}) color={} pts={} sat={}:{}:{} cache_sep={:.5e}",
            d.slot,
            d.generation,
            d.lifecycle_flags,
            d.local_order,
            d.pair.0,
            d.pair.1,
            d.color,
            d.point_count,
            d.sat_type,
            d.sat_index_a,
            d.sat_index_b,
            d.cache_separation
        );
        match &d.manifold {
            Ok(manifold) => {
                for (i, point) in manifold.points.iter().take(manifold.point_count as usize).enumerate() {
                    println!("        native_p{i} sep={:.5e} cached_sep={:.5e} total_jn={:.5e} vn={:.5e} persisted={}",
                        point.separation, point.base_separation, point.total_normal_impulse,
                        point.normal_velocity, point.persisted);
                }
            }
            Err(error) => println!("        invalid native manifold: {error}"),
        }
        for i in 0..d.point_count as usize {
            println!(
                "        p{i} feature={:#010x} triangle={:?} base={:.5e} jn={:.5e} nm={:.5e} lever={:.5e}",
                d.point_ids[i],
                d.point_triangles[i].checked_sub(1),
                d.base_separations[i],
                d.normal_impulses[i],
                d.prepared_normal_mass[i],
                d.prepared_lever_arm[i]
            );
        }
        println!(
            "        friction=({:.5e},{:.5e}) twist={:.5e} rolling=({:.5e},{:.5e},{:.5e}) tangent_inv=({:.5e},{:.5e};{:.5e},{:.5e}) soft=({:.5e},{:.5e},{:.5e})",
            d.friction_impulse[0],
            d.friction_impulse[1],
            d.twist_impulse,
            d.rolling_impulse[0],
            d.rolling_impulse[1],
            d.rolling_impulse[2],
            d.prepared_tangent_inv[0],
            d.prepared_tangent_inv[1],
            d.prepared_tangent_inv[2],
            d.prepared_tangent_inv[3],
            d.prepared_softness[0],
            d.prepared_softness[1],
            d.prepared_softness[2]
        );
    }
}

/// Walk every world step in a `B3TR` dump. Reports the first mismatch and selected focus frames.
#[allow(dead_code)]
pub async fn compare_trace_dump(
    dump_path: PathBuf,
    scene: DemoScene,
    contacts: bool,
    body_count: u32,
    epsilon: f32,
    until: u32,
) -> Result<(), String> {
    let trace = read_trace(&dump_path)?;
    let gpu_dev = GpuDevice::new(None).await?;
    let demo = make_demo(scene, body_count, contacts, false, false);
    let world = build_demo_world(gpu_dev, &demo);
    crate::api::b3_world_ensure_gpu(world);
    let gpu_n = b3_world_body_count(world);
    if gpu_n != trace.body_count {
        b3_destroy_world(world);
        return Err(format!(
            "body count GPU {gpu_n} vs trace {}",
            trace.body_count
        ));
    }
    let last = until.min(trace.frames.saturating_sub(1));
    println!(
        "trace {} vs {} (steps 0..={last}, epsilon {epsilon:.1e})",
        scene.slug(),
        dump_path.display()
    );
    let focus_steps = [
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 50, 73, 81, 99,
        100, 200, 300,
    ];
    let mut first_mismatch: Option<String> = None;
    for step in 0..=last {
        if step > 0 {
            b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
        }
        let gpu_bodies = b3_world_sync_from_gpu(world).await;
        let gpu_raw = b3_world_sync_contacts(world).await;
        let mut gpu_cons: Vec<_> = gpu_raw.iter().filter_map(from_gpu_contact).collect();
        gpu_cons.sort_by_key(|c| (c.a, c.b));
        let cpu = &trace.data[step as usize];
        if gpu_bodies.len() != cpu.bodies.len() {
            b3_destroy_world(world);
            return Err(format!(
                "step {step}: body len GPU {} vs CPU {}",
                gpu_bodies.len(),
                cpu.bodies.len()
            ));
        }
        let Some((pos, vel, quat, omega)) = max_body_error(&gpu_bodies, &cpu.bodies) else {
            b3_destroy_world(world);
            return Err(format!("step {step}: body compare failed"));
        };
        let clines = diff_contacts(&cpu.contacts, &gpu_cons, epsilon);
        println!(
            "  step {step}: |Δpos|={pos:.3e} |Δvel|={vel:.3e} |Δq|={quat:.3e} |Δω|={omega:.3e} contacts CPU {} GPU {}",
            cpu.contacts.len(),
            gpu_cons.len()
        );
        let contact_mismatch = !clines.is_empty();
        let body_mismatch = pos > epsilon || quat > epsilon || omega > epsilon;
        let is_first = first_mismatch.is_none() && (contact_mismatch || body_mismatch);
        let is_focus = focus_steps.contains(&step);
        if is_focus {
            dump_gpu_contact_diagnostics(&gpu_raw);
        }
        if is_first {
            first_mismatch = Some(if contact_mismatch {
                format!(
                    "{} constraint mismatch at step {step} ({} diffs)",
                    scene.slug(),
                    clines.len()
                )
            } else {
                format!(
                    "{} body mismatch at step {step} pos={pos:.3e} vel={vel:.3e} q={quat:.3e}",
                    scene.slug()
                )
            });
        }
        if contact_mismatch && (is_first || is_focus) {
            for l in &clines {
                println!("{l}");
            }
            for (i, (g, c)) in gpu_bodies.iter().zip(cpu.bodies.iter()).enumerate() {
                println!(
                    "    body[{i}] GPU pos ({:.5},{:.5},{:.5}) rot ({:.5},{:.5},{:.5},{:.5}) flags={:#x}",
                    g.pos[0], g.pos[1], g.pos[2], g.rot[0], g.rot[1], g.rot[2], g.rot[3], g.flags
                );
                println!(
                    "           CPU pos ({:.5},{:.5},{:.5}) rot ({:.5},{:.5},{:.5},{:.5})",
                    c.pos[0], c.pos[1], c.pos[2], c.rot[0], c.rot[1], c.rot[2], c.rot[3]
                );
            }
            dump_gpu_solver_pairs(&gpu_raw);
            let raw_n = gpu_raw
                .iter()
                .filter(|c| c.a != u32::MAX && c.count > 0)
                .count();
            let ghost_n = gpu_raw
                .iter()
                .filter(|c| c.a != u32::MAX && c.count == 0)
                .count();
            println!(
                "    gpu contact slots used={raw_n} ghosts={ghost_n} total={}",
                gpu_raw.len()
            );
        }
        if body_mismatch && (is_first || is_focus) {
            for (i, (g, c)) in gpu_bodies.iter().zip(cpu.bodies.iter()).enumerate() {
                let dp = (g.pos[0] - c.pos[0])
                    .abs()
                    .max((g.pos[1] - c.pos[1]).abs())
                    .max((g.pos[2] - c.pos[2]).abs());
                if dp > epsilon {
                    println!(
                        "    body[{i}] pos GPU ({:.5},{:.5},{:.5}) CPU ({:.5},{:.5},{:.5})",
                        g.pos[0], g.pos[1], g.pos[2], c.pos[0], c.pos[1], c.pos[2]
                    );
                }
            }
            dump_matched_contacts(&cpu.contacts, &gpu_cons);
            dump_gpu_solver_pairs(&gpu_raw);
        }
        if vel > epsilon && !body_mismatch && !contact_mismatch {
            println!("    warn |Δvel|={vel:.3e} (contacts+pos still within {epsilon:.1e})");
        }
    }
    b3_destroy_world(world);
    if let Some(err) = first_mismatch {
        return Err(err);
    }
    println!("trace {} locked through step {last}", scene.slug());
    Ok(())
}

/// Completed-step latency (submit + device wait, no CPU mirror) plus named GPU stages.
pub async fn write_completed_step_bench(
    path: PathBuf,
    scene: DemoScene,
    body_count: u32,
    warmup: u32,
    timed_steps: u32,
    run_count: u32,
    sleep: bool,
    contacts: bool,
    body_count_explicit: bool,
) -> Result<(), String> {
    let timed = timed_steps.max(1);
    let runs = run_count.max(1);
    let fingerprint = build_fingerprint();
    let mut run_p50 = Vec::new();
    let mut run_p95 = Vec::new();
    let mut all_step = Vec::new();
    let mut collide = Vec::new();
    let mut solve = Vec::new();
    let mut broadphase = Vec::new();
    let mut narrowphase = Vec::new();
    let mut graph = Vec::new();
    let mut prepare = Vec::new();
    let mut device = Vec::new();
    let mut encode = Vec::new();
    let mut adapter = String::new();
    let mut bodies = 0u32;
    let mut unique_peak = 0u32;
    let mut raw_runs = String::new();
    let mut cpu_trials = Vec::new();
    let scale = crate::types::scene_scale_count(scene, body_count, body_count_explicit);
    for run in 0..runs {
        if run % 2 == 0 {
            cpu_trials.push(maybe_cpu_oracle(scene, scale, warmup, timed, sleep));
        }
        let gpu = GpuDevice::new(None).await?;
        let demo = make_demo(scene, body_count, contacts, false, body_count_explicit);
        let world = build_demo_world(gpu, &demo);
        b3_world_enable_sleeping(world, sleep);
        crate::api::b3_world_ensure_gpu(world);
        if adapter.is_empty() {
            adapter = b3_world_gpu_report_name(world);
            bodies = b3_world_body_count(world);
        }
        for _ in 0..warmup {
            b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
        }
        b3_world_gpu_wait(world);
        // Honor exactly the requested window on both GPU and CPU. Sleeping
        // being enabled must not silently advance only the GPU by extra steps.
        let bodies_now = crate::api::b3_world_sync_from_gpu(world).await;
        let awake_after_warmup = bodies_now.iter()
            .filter(|b| b.inv_mass > 0.0 && (b.flags & FLAG_SLEEP) == 0).count();
        let settled = sleep && awake_after_warmup == 0;
        let settle_wait = 0u32;
        let sleep_window = if !sleep {
            "no-sleep"
        } else if settled {
            "settled-sleep"
        } else if warmup >= 200 {
            "late-sleep-enabled-not-settled"
        } else {
            "early-sleep-enabled"
        };
        let mut step_ms = Vec::new();
        let mut run_collide = Vec::new();
        let mut run_solve = Vec::new();
        let mut run_bp = Vec::new();
        let mut run_bp_stages: Vec<String> = Vec::new();
        let mut run_np = Vec::new();
        let mut run_graph = Vec::new();
        let mut run_prepare = Vec::new();
        let mut run_device = Vec::new();
        let mut run_encode = Vec::new();
        for _ in 0..timed {
            let t0 = Instant::now();
            b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
            b3_world_gpu_wait(world);
            step_ms.push(t0.elapsed().as_secs_f64() * 1000.0);
            run_collide.push(crate::api::b3_world_last_collide_ms(world) as f64);
            run_solve.push(crate::api::b3_world_last_solve_ms(world) as f64);
            run_bp.push(crate::api::b3_world_last_broadphase_ms(world) as f64);
            #[cfg(all(feature="native-command-cache",not(target_arch="wasm32")))]
            if let Some(stages)=crate::api::b3_world_completed_broadphase_profile_ms(world) {
                assert_eq!(stages.len(),7);
                run_bp_stages.push(format!("[{}]",fmt_f64(&stages)));
            }
            run_np.push(crate::api::b3_world_last_narrowphase_ms(world) as f64);
            run_graph.push(crate::api::b3_world_last_graph_ms(world) as f64);
            run_prepare.push(crate::api::b3_world_last_prepare_ms(world) as f64);
            run_device.push(crate::api::b3_world_last_device_ms(world) as f64);
            run_encode.push(crate::api::b3_world_last_encode_ms(world) as f64);
        }
        if let Some(stats) = b3_world_live_step_stats(world).await {
            if stats.capacity_loss() {
                b3_destroy_world(world);
                return Err(format!(
                    "capacity loss during bench: {}",
                    stats.sticky.loss_detail()
                ));
            }
            unique_peak = unique_peak.max(stats.narrowphase_pairs);
        }
        let cons = b3_world_sync_contacts(world).await;
        let live = cons
            .iter()
            .filter(|c| c.a != u32::MAX && c.count > 0)
            .count() as u32;
        unique_peak = unique_peak.max(live);
        let bodies_now = b3_world_sync_from_gpu(world).await;
        if scene == DemoScene::FallingCubes {
            if live == 0 || bodies_now.iter().any(|b| !b.pos.iter().all(|v| v.is_finite()) || b.pos[1] < -1.01) {
                b3_destroy_world(world);
                return Err("falling-cubes validation failed: no contacts, nonfinite or escaped body".into());
            }
        }
        let awake = bodies_now
            .iter()
            .filter(|b| b.inv_mass > 0.0 && (b.flags & FLAG_SLEEP) == 0)
            .count();
        collide.extend_from_slice(&run_collide);
        solve.extend_from_slice(&run_solve);
        broadphase.extend_from_slice(&run_bp);
        narrowphase.extend_from_slice(&run_np);
        graph.extend_from_slice(&run_graph);
        prepare.extend_from_slice(&run_prepare);
        device.extend_from_slice(&run_device);
        encode.extend_from_slice(&run_encode);
        all_step.extend_from_slice(&step_ms);
        run_p50.push(percentile(&step_ms, 0.50));
        run_p95.push(percentile(&step_ms, 0.95));
        if !raw_runs.is_empty() {
            raw_runs.push(',');
        }
        let allocations = b3_world_gpu_allocations(world)
            .ok_or("completed-step benchmark has no GPU allocation record")?;
        let allocation_json = format!(
            "{{\"scope\":\"primary_simulation_buffers\",\"body_bytes\":{},\"shape_geometry_material_bytes\":{},\"contact_bytes\":{},\"joint_bytes\":{},\"scratch_bytes\":{},\"atom_bytes\":{},\"fixed_bytes\":{},\"total_bytes\":{}}}",
            allocations.body_bytes, allocations.shape_bytes, allocations.contact_bytes,
            allocations.joint_bytes, allocations.scratch_bytes, allocations.atom_bytes,
            allocations.fixed_bytes, allocations.total_bytes,
        );
        let bp_profile_json=if run_bp_stages.is_empty() {"null".to_string()} else {
            format!("{{\"stages\":[\"clear\",\"static_setup\",\"static_pairs\",\"spatial_insert\",\"dynamic_pairs\",\"sort\",\"compact\"],\"samples_ms\":[{}]}}",run_bp_stages.join(","))
        };
        raw_runs.push_str(&format!(
            "{{\"run\":{},\"allocations\":{allocation_json},\"broadphase_profile\":{bp_profile_json},\"physics_step\":{},\"solver_dispatches\":{},\"static_sort_dispatches\":{},\"joint_dispatches\":{},\"encode_commands\":{},\"completed_step_ms\":[{}],\"encode_ms\":[{}],\"broadphase_ms\":[{}],\"narrowphase_ms\":[{}],\"graph_ms\":[{}],\"prepare_ms\":[{}],\"collide_ms\":[{}],\"solve_ms\":[{}],\"device_ms\":[{}],\"live_contacts\":{live},\"awake_dynamic\":{awake},\"awake_after_warmup\":{awake_w},\"settled\":{settled},\"settle_wait_steps\":{settle_wait},\"sleep_window\":\"{sleep_window}\"}}",
            run + 1,
            crate::api::b3_world_physics_step(world),
            crate::api::b3_world_last_solver_dispatches(world),
            crate::api::b3_world_last_static_sort_dispatches(world),
            crate::api::b3_world_last_joint_dispatches(world),
            crate::api::b3_world_last_encode_commands(world),
            fmt_f64(&step_ms),
            fmt_f64(&run_encode),
            fmt_f64(&run_bp),
            fmt_f64(&run_np),
            fmt_f64(&run_graph),
            fmt_f64(&run_prepare),
            fmt_f64(&run_collide),
            fmt_f64(&run_solve),
            fmt_f64(&run_device),
            awake_w = awake_after_warmup,
            settled = if settled { "true" } else { "false" },
            settle_wait = settle_wait,
            sleep_window = sleep_window,
        ));
        eprintln!(
            "bench {} run {} bodies={bodies} completed-step p50={:.3} p95={:.3} ms bp={:.3} np={:.3} graph={:.3}",
            scene.slug(),
            run + 1,
            run_p50.last().copied().unwrap_or(0.0),
            run_p95.last().copied().unwrap_or(0.0),
            percentile(&run_bp, 0.50),
            percentile(&run_np, 0.50),
            percentile(&run_graph, 0.50),
        );
        b3_destroy_world(world);
        if run % 2 == 1 {
            cpu_trials.push(maybe_cpu_oracle(scene, scale, warmup, timed, sleep));
        }
    }
    let mut mirror_ms = Vec::new();
    let mut getter_ms = Vec::new();
    {
        let gpu = GpuDevice::new(None).await?;
        let world = build_demo_world(
            gpu,
            &make_demo(scene, body_count, contacts, false, body_count_explicit),
        );
        b3_world_enable_sleeping(world, sleep);
        crate::api::b3_world_ensure_gpu(world);
        for _ in 0..warmup {
            b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
        }
        b3_world_gpu_wait(world);
        let n = crate::api::b3_world_body_count(world) as usize;
        let mut items = vec![
            DrawItemC {
                pos: [0.0; 3],
                kind: 0,
                rot: [0.0, 0.0, 0.0, 1.0],
                half: [0.0; 3],
                axis: [0.0, 1.0, 0.0],
                flags: 0,
            };
            n.max(1)
        ];
        for _ in 0..timed {
            let t0 = Instant::now();
            b3_world_step_gpu(world, FIXED_DT, DEFAULT_SUB_STEPS);
            b3_world_gpu_wait_with_mirror(world);
            mirror_ms.push(t0.elapsed().as_secs_f64() * 1000.0);
            crate::api::b3_world_prepare_pose_snapshot(world);
            let t_get = Instant::now();
            let _ = b3_world_draw_items(world, &mut items);
            getter_ms.push(t_get.elapsed().as_secs_f64() * 1000.0);
        }
        b3_destroy_world(world);
    }
    let cpu = format!("[{}]", cpu_trials.join(","));
    let json = format!(
        "{{\n  \"scene\":\"{}\",\n  \"adapter\":\"{}\",\n  \"git\":\"{}\",\n  \"dirty\":{},\n  \"source_sha256\":\"{}\",\n  \"profile\":\"release\",\n  \"fused_islands\":false,\n  \"cpu_win_validated\":false,\n  \"bodies\":{bodies},\n  \"sub_steps\":{},\n  \"dt\":{},\n  \"warmup_steps\":{warmup},\n  \"timed_steps\":{timed},\n  \"runs\":{runs},\n  \"sleep\":{sleep},\n  \"timing_path\":\"device_wait_no_mirror\",\n  \"completed_step\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"step_plus_host_mirror\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"gpu_broadphase_ms\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"gpu_narrowphase_ms\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"gpu_graph_ms\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"gpu_prepare_ms\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"gpu_collide_ms\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"gpu_solve_ms\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"gpu_device_ms\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"gpu_encode_ms\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"host_draw_list_ms\":{{\"p50_ms\":{:.4},\"p95_ms\":{:.4}}},\n  \"live_contacts_final\":{unique_peak},\n  \"cpu_trial_count\":{cpu_n},\n  \"raw_runs\":[{raw_runs}],\n  \"cpu_trials\":{cpu},\n  \"notes\":\"completed-step is queue submit plus device wait without CPU pose mirror. step_plus_host_mirror is step+wait+CPU pose mirror, not Sokol present. host_draw_list_ms is CPU draw-item packing after the snapshot; Sokol present is only in the native/Rust window. Physics pair identities use two 32-bit shape indices; native sample C metadata limits are tracked separately. Fused TGS is disabled. cpu_win_validated is false until matched completed-step AND native application comparisons pass. live_contacts_final is the last observed unique-contact count, not a sticky peak. CPU trials are isolated per-run temp dirs. Historical fused-path JSON is not current evidence.\"\n}}\n",
        scene.slug(),
        json_escape(&adapter),
        json_escape(&fingerprint.0),
        fingerprint.1,
        json_escape(&fingerprint.2),
        DEFAULT_SUB_STEPS,
        FIXED_DT,
        percentile(&all_step, 0.50),
        percentile(&all_step, 0.95),
        percentile(&mirror_ms, 0.50),
        percentile(&mirror_ms, 0.95),
        percentile(&broadphase, 0.50),
        percentile(&broadphase, 0.95),
        percentile(&narrowphase, 0.50),
        percentile(&narrowphase, 0.95),
        percentile(&graph, 0.50),
        percentile(&graph, 0.95),
        percentile(&prepare, 0.50),
        percentile(&prepare, 0.95),
        percentile(&collide, 0.50),
        percentile(&collide, 0.95),
        percentile(&solve, 0.50),
        percentile(&solve, 0.95),
        percentile(&device, 0.50),
        percentile(&device, 0.95),
        percentile(&encode, 0.50),
        percentile(&encode, 0.95),
        percentile(&getter_ms, 0.50),
        percentile(&getter_ms, 0.95),
        cpu_n = cpu_trials.len(),
    );
    if let Some(parent) = path.parent() {
        std::fs::create_dir_all(parent).map_err(|e| e.to_string())?;
    }
    std::fs::write(&path, json).map_err(|e| e.to_string())?;
    eprintln!("wrote {}", path.display());
    Ok(())
}

fn fmt_f64(values: &[f64]) -> String {
    values
        .iter()
        .map(|v| format!("{v:.4}"))
        .collect::<Vec<_>>()
        .join(",")
}

fn build_fingerprint() -> (String, bool, String) {
    let rev = std::process::Command::new("git")
        .args(["rev-parse", "--short", "HEAD"])
        .output()
        .ok()
        .and_then(|o| String::from_utf8(o.stdout).ok())
        .unwrap_or_else(|| "unknown".into());
    let dirty = std::process::Command::new("git")
        .args(["status", "--porcelain"])
        .output()
        .ok()
        .map(|o| !o.stdout.is_empty())
        .unwrap_or(true);
    let root = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    let hash = std::process::Command::new("bash")
        .args([
            "-lc",
            "find src shaders c_abi scripts Cargo.toml Cargo.lock -type f ! -name '*.json' | sort | xargs sha256sum | sha256sum",
        ])
        .current_dir(&root)
        .output()
        .ok()
        .and_then(|o| String::from_utf8(o.stdout).ok())
        .map(|s| s.split_whitespace().next().unwrap_or("unknown").to_string())
        .unwrap_or_else(|| "unknown".into());
    (rev.trim().to_string(), dirty, hash)
}

fn maybe_cpu_oracle(
    scene: DemoScene,
    body_count: u32,
    warmup: u32,
    frames: u32,
    sleep: bool,
) -> String {
    let bin = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("oracle/build/box3d_oracle");
    if !bin.exists() {
        return "{\"available\":false,\"reason\":\"oracle/build/box3d_oracle missing\"}".into();
    }
    let dir = std::env::temp_dir().join(format!(
        "gpu-physics-cpu-{}-{}",
        std::process::id(),
        std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .map(|d| d.as_nanos())
            .unwrap_or(0)
    ));
    let _ = std::fs::create_dir_all(&dir);
    let tmp = dir.join("cpu.json");
    let mut args = vec![
        "--scene".into(),
        scene.slug().to_string(),
        "--bodies".into(),
        body_count.to_string(),
        "--warmup".into(),
        warmup.to_string(),
        "--frames".into(),
        frames.to_string(),
        "--metrics".into(),
        tmp.to_str().unwrap_or("/tmp/gpu-physics-cpu-bench.json").to_string(),
    ];
    if !sleep {
        args.push("--no-sleep".into());
    }
    if let Ok(workers) = std::env::var("GPU_PHYSICS_CPU_WORKERS") {
        args.extend(["--workers".into(), workers]);
    }
    let status = std::process::Command::new(&bin).args(&args).status();
    let out = match status {
        Ok(s) if s.success() => std::fs::read_to_string(&tmp)
            .unwrap_or_else(|_| "{\"available\":false,\"reason\":\"read failed\"}".into()),
        _ => "{\"available\":false,\"reason\":\"oracle failed\"}".into(),
    };
    let _ = std::fs::remove_dir_all(&dir);
    out
}

/// Isolated initialization timing: no CPU oracle, renderer or simulation steps.
/// Application time to first frame is measured separately in each viewer.
pub async fn write_startup_bench(path: PathBuf, demo: DemoConfig) -> Result<(), String> {
    let started = Instant::now();
    let gpu = GpuDevice::new(None).await?;
    let adapter = gpu.report.name.clone();
    let device_ms = started.elapsed().as_secs_f64() * 1000.0;
    let scene_started = Instant::now();
    let world = build_demo_world(gpu, &demo);
    let scene_ms = scene_started.elapsed().as_secs_f64() * 1000.0;
    let prepare_started = Instant::now();
    crate::api::b3_world_ensure_gpu(world);
    crate::api::b3_world_prepare_collision(world);
    b3_world_gpu_wait(world);
    let failure = crate::api::b3_world_gpu_fail(world);
    if !failure.is_null() {
        let message = unsafe { std::ffi::CStr::from_ptr(failure) }.to_string_lossy().into_owned();
        b3_destroy_world(world);
        return Err(message);
    }
    let gpu_prepare_ms = prepare_started.elapsed().as_secs_f64() * 1000.0;
    let total_ms = started.elapsed().as_secs_f64() * 1000.0;
    let result = format!(
        "{{\n  \"schema\": \"startup-v1\", \"mode\": \"initialization-only\",\n  \"adapter\": \"{}\", \"scene\": \"{}\",\n  \"dynamic_cubes\": {}, \"bodies\": {},\n  \"device_ms\": {device_ms}, \"scene_ms\": {scene_ms},\n  \"gpu_prepare_ms\": {gpu_prepare_ms}, \"total_ms\": {total_ms},\n  \"first_frame_ms\": null\n}}\n",
        json_escape(&adapter), demo.scene.slug(), demo.body_count, b3_world_body_count(world),
    );
    b3_destroy_world(world);
    std::fs::write(path, result).map_err(|e|e.to_string())
}
