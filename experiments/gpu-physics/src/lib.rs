#[cfg(all(feature = "replay-diagnostics", not(target_arch = "wasm32")))]
mod drag_replay;
#[cfg(not(target_arch = "wasm32"))]
mod native_precision;
mod loading;
mod pipeline_cache;
#[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
mod native_command_cache;
#[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
mod native_async;
pub mod adapter;
pub mod api;
pub mod dump;
pub mod mesh;
pub mod scenes;
pub mod sim;
pub mod trace;
pub mod types;

#[cfg(all(test, not(target_arch = "wasm32")))]
mod gpu_invariants;

#[cfg(not(target_arch = "wasm32"))]
pub mod c_abi;

#[cfg(not(target_arch = "wasm32"))]
pub mod pose_gl;

#[cfg(not(target_arch = "wasm32"))]
pub mod video;

#[cfg(not(target_arch = "wasm32"))]
pub mod headless;

#[cfg(not(target_arch = "wasm32"))]
pub mod app;
pub mod render;
pub mod render_scene;
mod scene_draw;

#[cfg(target_arch = "wasm32")]
pub mod web_loop;

use crate::types::{DemoConfig, DemoScene, DEFAULT_BODY_COUNT};

#[derive(Clone, Debug)]
pub struct Cli {
    pub headless: bool,
    pub self_test: bool,
    pub contacts: bool,
    pub body_count: u32,
    pub body_count_explicit: bool,
    pub scene: DemoScene,
    pub dump_path: Option<std::path::PathBuf>,
    pub record_path: Option<std::path::PathBuf>,
    pub metrics_path: Option<std::path::PathBuf>,
    pub bench_path: Option<std::path::PathBuf>,
    pub native_timeline_path: Option<std::path::PathBuf>,
    pub replay_mp4: Option<std::path::PathBuf>,
    pub compare_oracle: Option<std::path::PathBuf>,
    pub compare_trace: Option<std::path::PathBuf>,
    pub frames: u32,
    pub warmup: u32,
    pub metric_runs: u32,
    pub jacobi: bool,
    pub sleep: bool,
    pub epsilon: f32,
    pub verify: bool,
}

impl Cli {
    pub fn parse() -> Self {
        let mut headless = false;
        let mut self_test = false;
        let mut contacts = true;
        let mut body_count = DEFAULT_BODY_COUNT;
        let mut body_count_explicit = false;
        let mut scene = DemoScene::SingleBox;
        let mut dump_path = None;
        let mut record_path = None;
        let mut metrics_path = None;
        let mut bench_path = None;
        let mut native_timeline_path = None;
        let mut replay_mp4 = None;
        let mut compare_oracle = None;
        let mut compare_trace = None;
        let mut frames = 300u32;
        let mut warmup = 20u32;
        let mut metric_runs = 5u32;
        let mut jacobi = false;
        let mut sleep = true;
        let mut epsilon = 1e-4f32;
        let mut verify = false;
        let mut args = std::env::args().skip(1).peekable();
        while let Some(arg) = args.next() {
            match arg.as_str() {
                "--headless" => headless = true,
                "--self-test" => {
                    self_test = true;
                    headless = true;
                }
                "--verify" => {
                    verify = true;
                    headless = true;
                    self_test = true;
                }
                "--contacts" => contacts = true,
                "--no-contacts" => contacts = false,
                "--jacobi" => jacobi = true,
                "--sleep" => sleep = true,
                "--no-sleep" => sleep = false,
                "--scene" => {
                    let name = args.next().expect("--scene <name>");
                    scene = parse_scene(&name);
                }
                "--bodies" => {
                    body_count = args
                        .next()
                        .expect("--bodies N")
                        .parse()
                        .expect("bodies u32");
                    body_count_explicit = true;
                }
                "--dump" => {
                    dump_path = Some(args.next().expect("--dump PATH").into());
                    headless = true;
                }
                "--mp4" => {
                    headless = true;
                    if args.peek().is_some_and(|a| !a.starts_with('-')) {
                        record_path = Some(args.next().unwrap().into());
                    } else {
                        record_path = Some(std::path::PathBuf::new());
                    }
                }
                "--metrics" => {
                    headless = true;
                    metrics_path = Some(args.next().expect("--metrics PATH").into());
                }
                "--bench" => {
                    headless = true;
                    bench_path = Some(args.next().expect("--bench PATH").into());
                }
                "--native-timeline" => {
                    native_timeline_path =
                        Some(args.next().expect("--native-timeline PATH").into());
                }
                "--replay-mp4" => {
                    headless = true;
                    replay_mp4 = Some(args.next().expect("--replay-mp4 DUMP.bin").into());
                }
                "--compare-oracle" => {
                    headless = true;
                    compare_oracle = Some(args.next().expect("--compare-oracle PATH.bin").into());
                }
                "--compare-trace" => {
                    headless = true;
                    compare_trace = Some(args.next().expect("--compare-trace PATH.b3tr").into());
                }
                "--epsilon" => {
                    epsilon = args
                        .next()
                        .expect("--epsilon F")
                        .parse()
                        .expect("epsilon f32");
                }
                "--frames" => {
                    frames = args
                        .next()
                        .expect("--frames N")
                        .parse()
                        .expect("frames u32");
                }
                "--warmup" => {
                    warmup = args
                        .next()
                        .expect("--warmup N")
                        .parse()
                        .expect("warmup u32");
                }
                "--metric-runs" => {
                    metric_runs = args
                        .next()
                        .expect("--metric-runs N")
                        .parse()
                        .expect("metric runs u32");
                }
                "-h" | "--help" => {
                    print_help();
                    std::process::exit(0);
                }
                other => {
                    eprintln!("unknown arg {other}");
                    print_help();
                    std::process::exit(2);
                }
            }
        }
        Self {
            headless,
            self_test,
            contacts,
            body_count,
            body_count_explicit,
            scene,
            dump_path,
            record_path,
            metrics_path,
            bench_path,
            native_timeline_path,
            replay_mp4,
            compare_oracle,
            compare_trace,
            frames,
            warmup,
            metric_runs: metric_runs.max(1),
            jacobi,
            sleep,
            epsilon,
            verify,
        }
    }
}

fn print_help() {
    eprintln!(
        "\
gpu-physics — GPU rigid bodies (Box3D C is a visual/timing reference, not lock-step)

  cargo run --release --manifest-path experiments/gpu-physics/Cargo.toml
  cargo run --release --manifest-path experiments/gpu-physics/Cargo.toml -- --self-test
  cargo run --release --manifest-path experiments/gpu-physics/Cargo.toml -- --scene box-stack

  --headless          no window; step and optional dump
  --self-test         two headless runs, memcmp dumps (GPU-vs-GPU determinism)
  --bench PATH        completed-step + GPU-clock stages (rebuild first; not a CPU-win)
  --native-timeline PATH  windowed present/vsync/frame breakdown (needs a display)
  --verify            reserved; use scripts/correctness-gate.sh
  --no-sleep          keep dynamic bodies awake for solver diagnosis
  --dump PATH         write binary checkpoint dump
  --mp4 [PATH]        1280x720 H.264 clip of N world steps (pre-commit compare)
                      default PATH: recordings/<scene>-<frames>f.mp4
  --replay-mp4 DUMP   encode a Box3D CPU oracle dump (needs --mp4 PATH)
  --compare-oracle B3OR   GPU vs Box3D dump at checkpoints
  --compare-trace B3TR    GPU vs Box3D per-step bodies+contacts
  --epsilon F         compare tolerance (default 1e-4)
  --metrics PATH      physics-only ms/step + rest checkpoints for every scene
  --bench PATH        completed-step p50/p95 for --scene (use --scene dominoes)
  --frames N          world steps in the clip / metrics (default 300 ≈ 10 s @ 30 fps)
  --metric-runs N     repeated timing windows for metrics p50/p95 (default 5)
  --jacobi            Jacobi/XPBD-style contact solve (scale; default is colored TGS)
  --scene NAME        spheres | stack | single-box | box-stack | sphere-stack
                      | capsule-stack | revolute | weld | anchored-mechanisms
                      | joint-chain | pyramid | bounce | mixed | spinner | ramp
                      | high-resistance | mixed-stacks | falling-cubes | dominoes
  --bodies N          spheres: body count; mixed-stacks: dynamic boxes;
                      anchored-mechanisms / joint-chain: mechanism/link count;
                      dominoes: ring count (default 30). Default {DEFAULT_BODY_COUNT}
  --contacts / --no-contacts
  -h, --help

Window keys: [ previous sample, ] next sample, R restart, Esc quit.

Hybrid NVIDIA laptop: if adapter pick fails, run with
  __NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia \\
  VK_DRIVER_FILES=/usr/share/vulkan/icd.d/nvidia_icd.json \\"
    );
}

fn parse_scene(name: &str) -> DemoScene {
    match name {
        "spheres" => DemoScene::Spheres,
        "stack" => DemoScene::Stack,
        "single-box" => DemoScene::SingleBox,
        "box-stack" => DemoScene::BoxStack,
        "sphere-stack" => DemoScene::SphereStack,
        "capsule-stack" => DemoScene::CapsuleStack,
        "revolute" => DemoScene::Revolute,
        "weld" => DemoScene::Weld,
        "anchored-mechanisms" => DemoScene::AnchoredMechanisms,
        "joint-chain" => DemoScene::JointChain,
        "pyramid" => DemoScene::Pyramid,
        "bounce" => DemoScene::Bounce,
        "mixed" => DemoScene::Mixed,
        "spinner" => DemoScene::Spinner,
        "ramp" => DemoScene::Ramp,
        "dominoes" => DemoScene::Dominoes,
        "high-resistance" => DemoScene::HighResistance,
        "mixed-stacks" => DemoScene::MixedStacks,
        "falling-cubes" => DemoScene::FallingCubes,
        other => {
            eprintln!("unknown scene {other}");
            std::process::exit(2);
        }
    }
}

pub fn demo_config(cli: &Cli) -> DemoConfig {
    DemoConfig {
        body_count: cli.body_count,
        body_count_explicit: cli.body_count_explicit,
        contacts: cli.contacts,
        scene: cli.scene,
        jacobi: cli.jacobi,
    }
}

#[cfg(target_arch = "wasm32")]
use wasm_bindgen::prelude::*;

#[cfg(target_arch = "wasm32")]
#[wasm_bindgen]
pub async fn start() -> Result<(), JsValue> {
    console_error_panic_hook::set_once();
    let _ = console_log::init_with_level(log::Level::Info);
    crate::web_loop::run()
        .await
        .map_err(|e| JsValue::from_str(&e))
}

#[allow(dead_code)]
mod ccd;

#[cfg(target_os = "linux")]
mod cpu_viewer;

#[cfg(not(target_arch = "wasm32"))]
pub mod staged_ring;
