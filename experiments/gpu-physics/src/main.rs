fn main() {
    #[cfg(not(target_arch = "wasm32"))]
    {
        env_logger::Builder::from_env(
            env_logger::Env::default()
                .default_filter_or("gpu_physics=info,wgpu=warn,wgpu_hal=error"),
        )
        .init();
        let cli = gpu_physics::Cli::parse();
        if let Some(path) = cli.metrics_path.clone() {
            if let Err(e) = pollster::block_on(gpu_physics::headless::write_snapshot_metrics(
                path,
                cli.frames,
                cli.contacts,
                cli.body_count,
                cli.metric_runs,
            )) {
                eprintln!("{e}");
                std::process::exit(1);
            }
        } else if let Some(path) = cli.bench_path.clone() {
            if let Err(e) = pollster::block_on(gpu_physics::headless::write_completed_step_bench(
                path,
                cli.scene,
                cli.body_count,
                cli.warmup,
                cli.frames,
                cli.metric_runs,
                cli.sleep,
                cli.contacts,
                cli.body_count_explicit,
            )) {
                eprintln!("{e}");
                std::process::exit(1);
            }
        } else if let Some(path) = cli.native_timeline_path.clone() {
            if let Err(e) = gpu_physics::app::run_native_timeline(
                gpu_physics::demo_config(&cli),
                cli.sleep,
                cli.warmup,
                cli.frames,
                cli.metric_runs,
                path,
            ) {
                eprintln!("{e}");
                std::process::exit(1);
            }
        } else if let Some(dump) = cli.compare_trace.clone() {
            if let Err(e) = pollster::block_on(gpu_physics::headless::compare_trace_dump(
                dump,
                cli.scene,
                cli.contacts,
                cli.body_count,
                cli.epsilon,
                cli.frames,
            )) {
                eprintln!("{e}");
                std::process::exit(1);
            }
        } else if let Some(dump) = cli.compare_oracle.clone() {
            if let Err(e) = pollster::block_on(gpu_physics::headless::compare_oracle_dump(
                dump,
                cli.scene,
                cli.contacts,
                cli.body_count,
                cli.epsilon,
                cli.sleep,
            )) {
                eprintln!("{e}");
                std::process::exit(1);
            }
        } else if let Some(dump) = cli.replay_mp4.clone() {
            let Some(path) = cli
                .record_path
                .clone()
                .filter(|p| !p.as_os_str().is_empty())
            else {
                eprintln!("--replay-mp4 needs --mp4 PATH");
                std::process::exit(2);
            };
            if let Err(e) = pollster::block_on(gpu_physics::video::record_mp4_from_dump(dump, path))
            {
                eprintln!("{e}");
                std::process::exit(1);
            }
        } else if let Some(mut path) = cli.record_path.clone() {
            if path.as_os_str().is_empty() {
                path = gpu_physics::video::default_record_path(cli.scene.slug(), cli.frames);
            }
            let cfg = gpu_physics::video::RecordConfig {
                demo: gpu_physics::demo_config(&cli),
                frames: cli.frames,
                path,
            };
            if let Err(e) = pollster::block_on(gpu_physics::video::record_mp4(cfg)) {
                eprintln!("{e}");
                std::process::exit(1);
            }
        } else if cli.headless || cli.self_test {
            let cfg = gpu_physics::headless::HeadlessConfig {
                body_count: cli.body_count,
                body_count_explicit: cli.body_count_explicit,
                contacts: cli.contacts,
                scene: cli.scene,
                max_step: cli.frames,
                dump_path: cli.dump_path,
                self_test: cli.self_test,
                jacobi: cli.jacobi,
                sleep: cli.sleep,
            };
            if let Err(e) = pollster::block_on(gpu_physics::headless::run_headless(cfg)) {
                eprintln!("{e}");
                std::process::exit(1);
            }
        } else if let Err(e) = gpu_physics::app::run_window(gpu_physics::demo_config(&cli), cli.sleep) {
            eprintln!("{e}");
            std::process::exit(1);
        }
    }
}
