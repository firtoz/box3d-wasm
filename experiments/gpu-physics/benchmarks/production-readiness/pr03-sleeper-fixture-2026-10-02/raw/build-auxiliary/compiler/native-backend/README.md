# Native backends and command caching

The sample launcher selects an eligible adapter without forcing a vendor or
changing PRIME, GL or Vulkan driver environment variables. Existing overrides
are respected. See the [experiment guide](../../README.md) for basic commands
and [support status](../../../../docs/gpu-physics.md) for missing features.

## Upstream Sokol sample application

| Host | Default physics backend | Renderer | Additional build dependencies |
|---|---|---|---|
| Linux | Cached Vulkan when cache tools are available; ordinary wgpu otherwise | OpenGL | OpenGL/X11/Xi/Xcursor and GTK development packages; `bash`, `patch`, `flock` for caching |
| macOS | Ordinary wgpu, preferring Metal | Metal | Xcode command-line tools; LLVM `llvm-nm`/`llvm-objcopy` on PATH |
| Windows | Ordinary wgpu, preferring DX12 | D3D11 | Visual Studio C++ Build Tools, Windows SDK, matching Rust MSVC toolchain and LLVM archive tools on PATH |

All builds require Bun, Python 3.10+, Rust and CMake 3.24+. The native ABI targets
64-bit desktops. macOS/Windows paths still need CI and hardware verification.
Ordinary builds on those platforms do not require Bash.

From the repository root:

```sh
bun run samples:both
bun run samples:gpu --sample-name 'GPU Bench/Mixed Stacks 4096'
GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0 bun run samples:both
```

| Environment variable | Values and behavior |
|---|---|
| `GPU_PHYSICS_SAMPLES_NATIVE_CACHE` | `auto` (default), `0` (ordinary backend), `1` (require Linux Vulkan cache prerequisites; error otherwise) |
| `GPU_PHYSICS_ADAPTER` | Strict case-insensitive device-name substring or `amd`, `intel`, `nvidia`, `apple` |
| `GPU_PHYSICS_BACKEND` | `auto`, `vulkan`, `metal`, `dx12`; explicit Metal/DX12 disables automatic Vulkan caching |
| `GPU_PHYSICS_POWER_PREFERENCE` | `high` (default) prefers discrete hardware; `low` prefers integrated hardware |
| `GPU_PHYSICS_ALLOW_SOFTWARE` | `1` permits software adapters; otherwise they are rejected |
| `GPU_PHYSICS_PIPELINE_CACHE` | `0` disables persistent driver pipeline caching |
| `GPU_PHYSICS_PIPELINE_CACHE_DIR` | Overrides the platform user-cache directory |

Device selection checks solver limits and surface compatibility, then tries
eligible candidates if device creation fails. Explicit overrides do not silently
select another vendor/backend. Compute and graphics adapters may differ on
hybrid machines. This policy is not automatic performance tuning.

The Linux sample cache configuration enables eligible full replay, graph
memoization, pair/reset caches, component scheduling and GPU CCD. Small component
workgroups default to 16; this is a measured starting point, not a per-device
optimum. `GPU_PHYSICS_COMPONENT_TGS=auto` opts into the measured density-based
schedule choice; `0` forces global and `1` forces component. Launchers preserve
these overrides. See the [bounded desktop qualification](../../benchmarks/scheduling-2026-09-30/README.md);
second-machine qualification is pending. Resource and world eligibility checks still apply. Automatic pose
staging stays enabled; `GPU_PHYSICS_SAMPLES_DEMAND_POSES=1` is an experimental
option without a demonstrated application benefit. Normal launch retains sleep
and pacing.

Native shader startup reuses one validated Naga module per source before entry-point
specialization. This removes repeated full-source parsing outside the driver disk
cache without changing SPIR-V output. `GPU_PHYSICS_TRACE_PIPELINES=1` separates
source preparation, specialization/emission and pipeline setup; the slower
`GPU_PHYSICS_VERIFY_SHADER_REUSE=1` checks every generated module against fresh
compilation. Local validation covered all 110 startup variants and four precision
regressions on both NVIDIA and AMD. See the experiment README for measured warm
startup and the falling-cube benchmark.

Portable executables live in
`native-samples/build-{cpu,gpu,both}[-native-cache]-portable/bin`. Ordinary Rust
sample libraries use `target/samples`; cached libraries use
`target/native-cache-build`. `--build-only` compiles without opening a window.
`--build-info /absolute/output.json` also writes the binary/library paths and
GPU runtime environment after a successful build. The native scene gate uses
that manifest to freeze executables and preserve the launcher's cache defaults.
Use `GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0` with the gate for an ordinary-backend run.
The Linux/X11 gate uses the active display; without DISPLAY it uses
Xvfb when available, which may select software graphics. Check renderer logs.
The `external-c-shim` feature and `GPU_PORTABLE_API=ON` select one C definition
per API in generated source copies. The combined CPU reference uses unfiltered,
separately namespaced upstream sources. The Box3D checkout remains unchanged.

## Patched Vulkan build

Pinned Naga/wgpu 26 patches expose tracked command replay, repair SPIR-V layout
and Vulkan synchronization, and retain resources until submission completion.
`manifest.json` and the hook manifests check before/after source fingerprints.
Builds use private crates under `target/native-backend`, a separate manifest and
lockfile under `target/native-workspace`, and output to `target/native-cache-build`.
Cargo registry sources and the ordinary lockfile are untouched. Repeated
preparation preserves unchanged manifest timestamps so warm launches reuse the
compiled dependencies.

From `experiments/gpu-physics`:

```sh
./scripts/build-native-cache.sh
./scripts/run-native-cache.sh --scene mixed-stacks
./scripts/build-native-cache.sh test --release --lib --no-run
```

Ordinary `cargo build --release` does not enable these patches. Enabling the
cache feature without the patched backend is unsupported. Metal/DX12 have no
raw-command replay implementation.

The direct `run-native-cache.sh` launcher differs from the sample launcher:
full replay, graph memoization, pair and reset replay require explicit opt-in.
It enables demand-driven pose staging for the direct viewer. Supported small
component workgroup overrides are 8/16/32/64.

Full replay requires the component solver and excludes joints, meshes, phase
capture, multi-step timing capture and idle-step elision. Its key includes
simulation parameters, capacities, bind groups and scheduling choices. Resource
or CCD-state replacement invalidates it. Ineligible worlds record normally;
queue uploads (including per-step force/torque replacement and clearing),
completion tracking and readback destinations stay live.
Physics shader modules, layouts and compiled pipelines are shared per logical
device; growing a scene replaces mutable resources and invalidates command caches
without recreating those programs. Pipeline-cache persistence happens during
explicit loading and world/device teardown, outside the spawning path. World
body/shape capacity hints include default per-shape material storage. See the
[spawning guidance](../../../../docs/gpu-physics.md#architecture-and-state-ownership).
Fused per-body TGS remains off.

Use `GPU_PHYSICS_NATIVE_VALIDATE=1` with an installed Vulkan validation layer.
Inspect direct layer output as well as wgpu logs, whose callback can filter
messages.

## Measurements

The direct Rust renderer previously qualified on 30-ring Dominoes and 4,096-box
mixed stacks in five matched AC trials against eight-worker Box3D: completed-step
and application-frame p50 were at least 20% lower, with lower p95 in every pair.
This used NVIDIA physics, AMD rendering, equal one-step display delay, a
2040×1148 window, immediate presentation, no sleeping, dt=1/60, four substeps,
200 warmup and 1,000 measured steps. It does not qualify Sokol or other hardware.
Reproduce that configuration from the experiment directory:

```sh
./scripts/build-native-cache.sh
GPU_PHYSICS_FULL_REPLAY=1 GPU_PHYSICS_GRAPH_MEMO=1 \
GPU_PHYSICS_NATIVE_PAIR_CACHE=1 GPU_PHYSICS_NATIVE_RESET_CACHE=1 \
GPU_PHYSICS_SMALL_COMPONENT_WG=16 GPU_PHYSICS_IDLE=0 \
GPU_PHYSICS_PRESENT_MODE=immediate \
./scripts/run-split-adapter.sh --scene dominoes --bodies 30 --no-sleep
# Other workload: --scene mixed-stacks --bodies 4096
```

### Sokol application (2026-09-19)

This round ran on AC on the shared RTX 4070 Laptop GPU machine, with NVIDIA
OpenGL rendering and Vulkan physics. The final cache-integration matrix used
three interleaved trials per mode and scene, 2880×1686 measured framebuffers,
eight CPU workers, dt=1/60, four substeps, 200 warmup and 1,000 measured frames,
sleeping disabled and swap interval zero. Values below are medians of each
trial's whole-frame cadence percentile, in milliseconds; they are not pooled
percentiles or physics-only timings.

| Scene | Ordinary both p50 / p95 | Cached both p50 / p95 | Cached GPU-only p50 / p95 | CPU-only p50 / p95 |
|---|---:|---:|---:|---:|
| Dominoes (5,431 bodies) | 18.120 / 29.428 | 16.825 / 26.004 | 15.767 / 20.459 | 12.820 / 16.698 |
| Mixed Stacks (4,096 dynamic + 2 static) | 14.793 / 22.186 | 11.291 / 16.845 | 9.990 / 14.949 | 8.240 / 9.485 |

Cached both reduced the median p50/p95 by 7.1%/11.6% for Dominoes and
23.7%/24.1% for mixed stacks. Mixed stacks improved in all three paired trials;
Dominoes improved p50 in two of three (the other regressed 0.4%) and p95 in all
three. This is bounded evidence of an application improvement, with a less
stable Dominoes benefit. One-minute system load varied from 9.2 to 14.0. Earlier
five-trial matrices and all outliers remain in the local artifact set; they also
show a stronger mixed-stacks benefit and variable Dominoes performance.
**The GPU-only application did not achieve the 20% CPU advantage.** The earlier
split-adapter direct-renderer qualification does not transfer to Sokol.

Current-state full-mirror reads and GL drawing remain significant costs.
Demand-pose staging and attempts to move synchronization did not produce a
repeatable improvement and were not enabled. This round's gain comes from
cache integration, with no extra display delay or skipped CPU-world work.

Freeze all executables before running interleaved comparisons from the repository
root:

```sh
python3 experiments/gpu-physics/scripts/measure-samples-application.py /absolute/new-report-dir \
  --baseline-both /absolute/frozen-baseline-both \
  --candidate-both /absolute/frozen-candidate-both \
  --candidate-gpu /absolute/frozen-candidate-gpu \
  --cpu /absolute/frozen-cpu
```

This harness is Linux/NVIDIA-specific. It checks AC power, binary hashes, matching
framebuffer/body counts, capacity loss and submitted/completed/rendered steps;
it retains all trials and does not automatically certify a CPU win. Loading
frames are excluded. Whole-frame cadence is callback entry to next callback entry.

Raw reports are ignored local outputs: `artifacts/goal-replay-qualification/`,
`artifacts/goal-replay-final-gate/`, `artifacts/goal-samples-integration/final-verified/`
and `artifacts/portability/`. Fresh clones need regenerated results or a separately
published archive. The [status document](../../../../docs/gpu-physics.md) records
known failures and the remaining merge checks.
