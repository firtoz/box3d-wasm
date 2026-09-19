# Opt-in Vulkan command cache backend

These pinned Naga 26 / wgpu 26 patches expose the raw handles needed to replay
compute command buffers, correct Vulkan queue stage masks and atomic semantics,
and connect the Rust SPIR-V layout repair in `../spirv-layout`.
The separately fingerprinted `queue-hooks.patch` supports binary semaphore waits
on tracked submissions for the experimental second queue. It preserves the
backend relay ordering; callers own dependency lifetime and submit serialization.
They are copied from the measured native-cache prototype; diagnostic shader
substitutions and rejected solver experiments are not included.

`physics-replay.patch` adds owned Vulkan primary command buffers and capture of
the tracked physics command sequence. Its separate before/after fingerprints
are checked after the compiler and queue hooks. A cached sequence retains its
command pools and resources through submission completion; discarding an encoder
does not execute it. Buffer initialization and state transitions remain tracked.
Queue uploads, completion identity, timestamp resolution and per-frame readback
destinations remain live on every submission.

Full-sequence reuse is opt-in with `GPU_PHYSICS_FULL_REPLAY=1`. It requires the
component solver and excludes joints, meshes, phase capture, multi-step timing
capture and idle-step elision. The key includes simulation parameters, capacities,
bind-group identity and scheduling choices; replacing CCD state invalidates it.
An ineligible or changed world records the ordinary sequence. Setting this option
alone is not a CPU-win claim; the qualified configuration is specified below.

From `experiments/gpu-physics`:

```sh
cargo fetch
./scripts/build-native-cache.sh
./scripts/run-native-cache.sh --scene mixed-stacks
# Compile the same focused library tests against the patched backend:
./scripts/build-native-cache.sh test --release --lib --no-run
```

The build requires Python 3, `patch`, `flock`, Rust and the ordinary native build
dependencies. It prepares private crates in `target/native-backend`, checks
before/after SHA-256 fingerprints, and uses a separate manifest and lockfile in
`target/native-workspace`. The build output is `target/native-cache-build`.
The workspace links current experiment sources and the clean Box3D checkout;
it does not copy or modify the Box3D engine. Cargo registry files and the ordinary
`Cargo.lock` are untouched. Repeat preparation is supported. A changed pinned
source fails preparation; use a fresh output directory after updating patches.

`cargo build --release` remains the ordinary build. Enabling the cache feature
without the patched backend is unsupported. The feature selects Vulkan only;
there is no Metal/DX12 raw-command implementation. The launcher enables the Rust
layout repair, cached passes, component solver, bounded pair matrix, GPU CCD and
demand pose staging. Pair-matrix eligibility and ordinary dispatch fallbacks
remain checked by the engine. Graph memoization defaults off; set
`GPU_PHYSICS_GRAPH_MEMO=1` explicitly for its tested mixed-stacks configuration.
The launcher defaults to workgroup size 16 and honors an explicit
`GPU_PHYSICS_SMALL_COMPONENT_WG` (supported sizes: 8/16/32/64).
It does not change power settings, presentation mode or sleep policy.

The explicit configuration below qualified on 30-ring Dominoes and the corrected
4,096-body mixed-stack fixture in five matched AC trials against eight-worker
Box3D. Both completed-step and application-frame p50 were at least 20% lower;
GPU p95 was lower in every pair. Rendering used the same 2040×1148 window,
immediate presentation and bounded one-step display delay for both engines.
The benchmark used 200 warmup and 1,000 timed steps, no sleeping, dt=1/60 and
four substeps. This is not qualification for Sokol, WASM, tiny scenes, different
hardware or arbitrary settings. Generic benchmark outputs continue to default
`cpu_win_validated` to false; the scoped completion audit records the qualified run.

Run the qualified native path from the experiment directory:

```sh
./scripts/build-native-cache.sh
GPU_PHYSICS_FULL_REPLAY=1 GPU_PHYSICS_GRAPH_MEMO=1 \
GPU_PHYSICS_NATIVE_PAIR_CACHE=1 GPU_PHYSICS_NATIVE_RESET_CACHE=1 \
GPU_PHYSICS_SMALL_COMPONENT_WG=16 GPU_PHYSICS_IDLE=0 \
GPU_PHYSICS_PRESENT_MODE=immediate \
./scripts/run-split-adapter.sh --scene dominoes --bodies 30 --no-sleep
# Use --scene mixed-stacks --bodies 4096 for the other qualified workload.
```

Fused per-body TGS remains off. Native validation uses
`GPU_PHYSICS_NATIVE_VALIDATE=1` and an
installed Vulkan validation layer; inspect the direct layer log as well as wgpu
output because the backend callback can filter diagnostics.

## Upstream Sokol sample application

From the repository root, GPU and combined samples choose a backend automatically:

```sh
bun run samples:both
bun run samples:gpu --sample-name 'GPU Bench/Mixed Stacks 4096'
# Explicit opt-out for debugging or ordinary-backend comparisons:
GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0 bun run samples:both
```

The Bun entry point locates Python 3.10+ and invokes a platform-neutral launcher.
It no longer sets a GPU vendor, PRIME offload, GL vendor or Vulkan ICD path.
Existing driver-selection environment variables are respected.

| Host | Automatic physics backend | Sample renderer |
|---|---|---|
| Linux | Cached Vulkan when `bash`, `patch` and `flock` are available; ordinary wgpu otherwise | OpenGL through the system driver |
| macOS | Ordinary wgpu, preferring Metal | Metal |
| Windows | Ordinary wgpu, preferring DX12 | D3D11 |

`GPU_PHYSICS_SAMPLES_NATIVE_CACHE=auto` is the default. `0` always selects the
ordinary backend; `1` requires the Linux Vulkan cache build and reports an error
when its prerequisites are absent. Selecting Metal/DX12 explicitly also disables
automatic Vulkan caching. Raw command replay is not implemented for Metal/DX12.
The Linux cache configuration enables full replay, graph memoization, pair/reset
caches, component scheduling and eligible GPU CCD. Existing eligibility and
resource-invalidation checks remain authoritative. Cache flags and the small
component workgroup size remain overridable; size 16 is a measured starting
point, not a per-device optimum.

Adapter selection checks solver limits and window compatibility before ranking
hardware. Discrete GPUs are preferred, with integrated GPUs next; there is no
NVIDIA/model requirement. Device-creation failures try the next eligible
candidate before any world exists. Overrides are strict filters, so a requested
missing device produces a diagnostic rather than silently choosing another GPU:

```sh
GPU_PHYSICS_ADAPTER=amd bun run samples:both
GPU_PHYSICS_ADAPTER='780m' bun run samples:gpu
GPU_PHYSICS_POWER_PREFERENCE=low bun run samples:gpu
GPU_PHYSICS_BACKEND=vulkan bun run samples:gpu
```

`GPU_PHYSICS_ADAPTER` accepts a case-insensitive name substring or vendor name
(`amd`, `intel`, `nvidia`, `apple`). `GPU_PHYSICS_BACKEND` accepts `auto`, `vulkan`,
`metal`, or `dx12`; power preference accepts `high` or `low`. CPU/software
adapters require `GPU_PHYSICS_ALLOW_SOFTWARE=1` and are logged as such. Devices
below the solver's limits report an actionable error; GPU mode never silently
becomes CPU Box3D. The Sokol graphics adapter follows the OS/driver selection,
which can differ from the compute adapter on a hybrid machine. Normal selection
is a capability/power heuristic, not an automatic performance benchmark.

The native ABI targets 64-bit desktops. Build prerequisites are Bun, Python
3.10+, Rust, CMake 3.24+, a native C/C++
toolchain and graphics drivers. Linux also needs OpenGL/X11 development packages
and the GTK development package used by the file dialog. macOS needs Xcode
command-line tools and LLVM's `llvm-nm`/`llvm-objcopy` on PATH (for example the
Homebrew LLVM bin directory). Windows needs Visual Studio C++ Build Tools with
a Windows SDK, Rust's matching MSVC toolchain and LLVM archive tools on PATH.
In PowerShell, set overrides with `$env:GPU_PHYSICS_ADAPTER = 'amd'` before
running the same Bun command. No Bash or Unix `nproc` is needed on macOS/Windows
for ordinary sample builds.

Builds use `native-samples/build-{cpu,gpu,both}[-native-cache]-portable`, with
ordinary Rust sample libraries in `target/samples` and cached ones in
`target/native-cache-build`. The `external-c-shim` Cargo feature lets CMake
select one definition of each public C API from generated copies rather than
relying on GNU duplicate-symbol/wrap linker options. The combined viewer's CPU
oracle is compiled from unfiltered upstream sources and namespaced separately.
The Box3D checkout stays unchanged. Existing Linux diagnostic fixtures can keep
the legacy CMake path; the launcher selects `GPU_PORTABLE_API=ON` explicitly.

Pose FD sharing is optional and Linux-only. The Vulkan exporting device UUID
must match a device UUID reported by OpenGL before an import is attempted. This
prevents unsafe cross-adapter imports on hybrid systems. Unknown UUIDs, missing
extensions and failed imports use current CPU pose mirrors without repeated
import attempts; other platforms use those mirrors directly. The GL buffer is
created before named storage is assigned, as required by
[OpenGL direct state access](https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf).
Physics worlds and step identities remain independent.

`bun run samples:both --build-only` builds without opening a window. The native
samples CI workflow builds all three modes on Linux, macOS and Windows; a build
job is not a physical-GPU correctness or performance test. Local validation here
covers Linux NVIDIA and AMD hardware. macOS/Windows execution and Intel hardware
still need testing on those systems before claiming verified runtime support.

The local portability checks passed adapter-policy tests, source-selection tests,
NVIDIA pointer/drag and C API settings fixtures, AMD picking and isolated dragging,
six AMD CCD tests, and AMD combined / NVIDIA ordinary-GPU health runs with matching
step identities. Automatic hybrid selection and AMD scene switching pass; matching
AMD UUID imports succeed, while different-device imports are skipped safely. All
three sample modes build on Linux.
The AMD ground-contact drag fixture **fails its unchanged normal tolerances**:
held position 0.007997 m (limit 0.005), quaternion chord 0.011881 (limit 0.01),
and held velocity 0.420311 m/s (limit 0.1). Cache-enabled and cache-disabled runs
produce identical results. Settling passes; this is an unresolved cross-driver
physics-comparison limitation, not a cache regression or a reason to substitute
CPU state. NVIDIA retains its prior normal pass and strict peak-velocity failure.
Raw local evidence is in ignored `artifacts/portability/`. The full library and
native correctness matrices have not been requalified across vendors.

An extra browser-target compile check exposed existing experimental WebGPU
build gaps (native-only staged transport and pose methods, pointer-size ABI
assertions and global world storage). Browser support is outside this desktop
port; no Box3D WASM binary was changed or browser compatibility claimed.

Automatic pose staging remains enabled by default. The experimental
`GPU_PHYSICS_SAMPLES_DEMAND_POSES=1` selects current-state mirrors on demand for
the C ABI world. It avoids the automatic pose-only staging submission that sample
queries and drawing otherwise supersede with a full mirror. Set it to `0` for the
same-executable control. Getters, queries, events and host CCD retain synchronous
current-state behavior. Both panes retain their own physics state, hit and anchor;
there is no extra display delay or asynchronous result-age policy.

The timing qualification below explicitly used NVIDIA OpenGL rendering and
NVIDIA Vulkan physics on this laptop, before automatic adapter selection. Do not apply the split-adapter renderer
results to this configuration. The sample list includes `GPU Bench/Mixed Stacks
4096`, with the same two-layer layout and enlarged supports as the 4,096-body
native fixture; `Mixed Stacks 600` retains its original layout.

Freeze baseline and candidate executables before timing. The application harness
runs alternating trials with AC checks, hardware/load snapshots, binary hashes,
unchanged frame timing, and per-frame completion/body-count checks:

```sh
python3 experiments/gpu-physics/scripts/measure-samples-application.py /absolute/new-report-dir \
  --baseline-both /absolute/frozen-baseline-both \
  --candidate-both /absolute/frozen-candidate-both \
  --candidate-gpu /absolute/frozen-candidate-gpu \
  --cpu /absolute/frozen-cpu
```

It measures Dominoes and Mixed Stacks 4096 with eight CPU workers, four substeps,
200 warmup / 1,000 measured steps, no sleeping and verified swap interval zero.
All trials and outliers are retained. Every measured frame must have the same
framebuffer dimensions across variants; submitted, completed and rendered step
identities must agree. For a staging-only experiment, add `--demand-poses 1
--native-control /absolute/frozen-candidate-both --staging-only`. GPU-only versus CPU-only
results are reported separately from the combined application. The harness does
not certify a CPU win automatically. Loading frames are excluded, as in the
existing Sokol harness; cadence remains callback-entry to next callback-entry.

### Sokol integration measurements (2026-09-19)

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

Profiling the cached application with the existing GPU/sidebar timers identified
full-mirror fetches of about 2.48 ms (Dominoes) and 1.78 ms (mixed stacks), after
the explicit physics completion wait. GPU compute was about 2.98/0.92 ms, while
OpenGL GPU drawing was about 8.30/6.63 ms. These are separate diagnostic runs,
not additive components of the table's median cadence. Picking requires a
current mirror for CCD/query semantics; omitting that synchronization moved the
wait into drawing and worsened cadence, so it was reverted. Disabling automatic
pose staging had less than 1% p50 effect in five closely alternating pairs and
remains optional, off by default. Moving Sokol GL to the AMD adapter or skipping
the unused GL pose import also failed their timing screens and were reverted.
A final three-pair screen queued mirror copies before the physics wait to remove
one CPU/GPU round trip. Dominoes p50/p95 changed from 16.703/23.274 to
18.058/23.470 ms; mixed stacks changed from 11.351/18.084 to
13.011/15.562 ms. Its p50 regressions rejected that change too; the patch and all
runs remain in `single-wait.patch` and `single-wait-pairs/`. No additional
synchronization optimization is claimed from these unsuccessful screens.

Local raw evidence is under ignored
`artifacts/goal-samples-integration/`: `final-verified/` contains the table's raw
frames, binary hashes, environments, hardware snapshots and all trial summaries;
`interleaved/`, `staging-pairs/`, `final-single/`, `profile-summary.json`, and
`unused-import-results.json` retain earlier trials and diagnostics. Freeze
executables before rebuilding: `--baseline-native` compares two cached binaries
with identical runtime options, while the default baseline uses the ordinary
backend. The harness rejects battery runs and mismatched geometry, missing
steps, stale rendered poses, reported GPU failures and contact capacity loss.

Validation of the retained integration passed 20 focused patched-backend library
tests: demand-pose lifecycle (4), full replay equivalence/lifecycle (3), primary
replay upload/ownership (1), convex CCD (6), joint events (3), current-state ray
queries with CCD/events (1), compound callback/event IDs with slot holes (1), and
sleep/wake velocity behavior (1). The full 229-test baseline and 58-case gate
from the previous qualification were not rerun in this bounded round.

```sh
# From experiments/gpu-physics:
GPU_PHYSICS_SAMPLES_NATIVE_CACHE=1 ./scripts/check-both-pointer.sh artifacts/cached-pointer
source scripts/native-samples-cache-env.sh
./scripts/build-native-cache.sh test --release --lib demand_pose_staging_tests -- --test-threads=1
./scripts/build-native-cache.sh test --release --lib full_physics_replay -- --test-threads=1
./scripts/build-native-cache.sh test --release --lib convex_ccd_integration_tests -- --test-threads=1
```

Application checks passed independent hit depths and one-sided misses, equivalent
pane rays, 600 held steps, ten release/re-grab cycles each for isolated and
ground-contact dragging, pause with ten single steps, sleeping, and a switch
from Mixed Stacks 4096 to Revolute. Demand staging passed the same pointer/drag
fixtures. The unchanged strict ground-drag velocity check still fails:
**0.982809 m/s versus the 0.5 m/s limit**. Position, orientation, held velocity and
settling checks pass; this existing failure remains nonblocking and unrelaxed.
Raw outputs and commands are in `interaction-results.json`, `focused-tests.json`
and `ccd-events-tests.json` beside the timing artifacts.

The measured improvement is the cached application configuration, not a
new asynchronous query or pose path. Current-state full mirrors and NVIDIA GL
work remain significant costs. Both worlds still simulate independently;
completion and rendered-step identities are checked rather than hiding work or
reusing stale poses. Normal launch retains sleeping and pacing; no-sleep and
unpaced operation are benchmark options only.
