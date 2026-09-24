# GPU physics experiment

An experimental Rust/WGSL rigid-body engine with a Box3D-compatible C interface
and native sample viewers. It is separate from the Box3D WASM package; the
upstream `box3d/` sources remain unchanged.

**Compatibility is incomplete.** Linux NVIDIA and AMD have local runtime coverage;
macOS and Windows build paths await CI and hardware verification. See
[status, work priorities and missing features](../../docs/gpu-physics.md).

## Build and run

Initialize the submodule with `git submodule update --init --recursive`. Install
Bun, Python 3.10+, Rust, CMake 3.24+ and a native C/C++ toolchain. Additional
platform dependencies are in the [backend guide](compiler/native-backend/README.md).
From the repository root:

```sh
bun run samples:cpu     # upstream Box3D
bun run samples:gpu     # experimental GPU engine
bun run samples:both    # independent CPU and GPU worlds, side by side
bun run samples:both --build-only
```

The launcher chooses a capable hardware adapter. Linux defaults to cached Vulkan
when its build prerequisites are available; macOS prefers Metal and Windows
prefers DX12. It does not force a GPU vendor or change driver selection.

```sh
GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0 bun run samples:both  # ordinary backend
GPU_PHYSICS_ADAPTER=amd bun run samples:gpu
GPU_PHYSICS_POWER_PREFERENCE=low bun run samples:gpu
```

These examples use POSIX shell syntax. In PowerShell, set an override with
`$env:GPU_PHYSICS_ADAPTER = 'amd'`, then run the Bun command.
See the [backend guide](compiler/native-backend/README.md) for strict overrides,
cache eligibility and build directories.

## Using the comparison viewer

CPU is on the left and GPU on the right. Both panes share the camera, sample
controller, settings, pause (`p`) and single-step controls. The bottom-left
checkbox switches to an overlapping view.

Click selects; **Ctrl + left-drag** grabs. Matching pane coordinates produce the
same ray, but each world uses its own hit, depth, anchor and motor joint. A miss
can leave one side ungrabbed. The other pane shows a matching cursor. Release,
focus loss and scene changes end the gesture.

Poses and simulation state stay independent. Warm starting is fixed on for GPU
worlds, so its toggle is replaced by a label.
Recording is unavailable in GPU/combined mode. The combined worker slider is
labelled CPU-only; GPU-only mode reports automatic scheduling.

The inspector is GPU-led; the shared sample controller means sample-specific
event/query logic is not two
independent application instances. Unsupported APIs can affect sample behavior;
consult the [limitations](../../docs/gpu-physics.md#missing-features-and-known-failures).

**GPU API / Shape Replacement** cycles one live shape through sphere, capsule,
box and cylinder geometry. IDs and metadata stay stable; alternating replacements
explicitly recompute body mass. The [replacement fixture and semantics](../../docs/gpu-physics.md#native-shape-replacement)
cover independent CPU/GPU state and rendering.

For sustained spawning, set expected peak body/shape counts in
`b3WorldDef.capacity` before creating the world, and destroy expired projectiles
so their slots can be reused. Automatic headroom and geometric growth handle
unplanned additions; compiled programs survive buffer growth. See the
[capacity and resource ownership notes](../../docs/gpu-physics.md#architecture-and-state-ownership).

GPU loading displays preparation stages while buffers and shaders are prepared.
Worlds do not step during loading. Driver compilation can delay shutdown until
worker teardown is safe. Pipeline caching can be disabled with
`GPU_PHYSICS_PIPELINE_CACHE=0` or relocated with `GPU_PHYSICS_PIPELINE_CACHE_DIR`.
Native precision shaders now parse and validate their shared source once per
process, then specialize individual entry points. Previously 110 entry points
reparsed the same source on each launch even with a warm driver cache. Two local
RTX 4070 Laptop warm sample launches now prepared in 1.51–1.76 s; first-launch
shader/driver work still takes longer. `GPU_PHYSICS_TRACE_PIPELINES=1` reports
preparation timings. `GPU_PHYSICS_VERIFY_SHADER_REUSE=1` recompiles independently
and checks byte-identical SPIR-V; use it for validation, not timing. All 110
startup entry points matched, and the four precision tests passed on NVIDIA and
AMD. Generated SPIR-V is still cached in memory; the disk cache holds driver
pipelines.

## Direct Rust viewer

From this directory:

```sh
cargo run --release -- --scene box-stack
cargo run --release -- --scene mixed-stacks --bodies 4096
```

Esc quits, `[` / `]` change scenes and `R` restarts. `--no-sleep` keeps bodies
active; normal interactive use allows sleep. `--mp4 output.mp4 --frames 300`
records a clip using FFmpeg. Run `cargo run --release -- --help` for scene options.

`--bodies` means dynamic box count for `mixed-stacks` and `falling-cubes`, sphere count for `spheres`,
pendulum count for `anchored-mechanisms`, link count for `joint-chain`, and ring
count for `dominoes`. The default Dominoes fixture has 30 rings.

## Falling-cube scaling benchmark

**RTX 4070 Laptop (8,188 MiB), Ryzen 9 8945HS, NVIDIA 610.57.04; 2026-09-24.**
The tiled implementation removes the previous 150,000-cube dispatch failure and
reduces large-world graph construction time without changing solver settings.
The matched before/after comparison below is complete. The replacement three-trial
CPU/GPU sweep, both renderers and final 200,000-cube qualification are in progress.

Three matched 100,000-cube trials on the RTX 4070 Laptop compare `533f14cd`
against preserved `1559a049` binaries. Median trial mean completed-step time
falls from **61.21 to 46.98 ms**, or **16.34 to 21.28 steps/s**: **30.3% higher
throughput**, exceeding the 20% target, and 23.2% less time per step. Trial means
range from 61.17–61.40 ms before and 46.80–47.07 ms after; median trial p50/p95
are 61.12/63.41 ms before and 43.53/59.33 ms after. Graph construction falls from
26.05 to 12.05 ms. Primary buffers increase from 1,739,759,836 to 1,837,715,104
bytes (93.4 MiB extra); this excludes driver allocations and renderer resources.
Trials alternate executable order, use the same global solver, 90 warmup plus
240 timed steps, four substeps, disabled sleep and normal desktop load. The
final 200,000-cube qualification and updated CPU/renderer sweep remain pending.

![Repeated 100k comparison: completed step, phases and memory](benchmarks/rtx4070-tiled-100k-2026-09-24.svg)

[Machine-readable comparison](benchmarks/rtx4070-tiled-100k-2026-09-24.json).
Local raw evidence is in `artifacts/dispatch-scaling/final-100k/`, including
per-step timings, power telemetry, binary hashes and the exact runner copies.

Production physics now uses shared Rust/WGSL tiled indexing in direct, indirect
and native cached dispatches, retaining the existing 1D layout below the boundary.
Padded radix workgroups are bounded, strided kernels cover the full grid, and the
component solver retains its existing 2D layout. Two host tests pass; an NVIDIA
fixture verifies exactly-once writes and untouched padding at 0, 1, 65,535, 65,536
and 65,537 workgroups across all four dispatch modes (20 cases). A production
lifecycle fixture with 131,075 shapes also passes with native command caches and
full replay enabled and disabled, covering callbacks, contact events, deletion,
remapping and slot reuse. Reproduce with
`./scripts/build-native-cache.sh test --release --lib dispatch -- --test-threads=1`.
An initial post-tiling trial completed **200,000 colliding cubes**: 7.52 completed
steps/s, 132.81 ms p50 and 136.67 ms p95, with 410,745 live contacts after 330 steps.
Capacity-loss, finite-position and ground-escape checks passed; all 200,000 dynamic
bodies remained awake. Primary simulation buffers used 3,478,816,732 bytes. This is
one diagnostic trial; repeated trials and application-renderer qualification are
still pending. This initial trial predates the clearing and graph-cache changes.

Contact clearing now tracks a monotonic high-water mark of allocated contact
slots, including mesh patches, and preserves it across buffer growth. It clears
that dirty range rather than reading every reserved contact slot; retirement does
not shrink the range, so old event keys are still cleared. Unused history starts
empty, and pair producers overwrite their live prefix without a capacity-wide
fill. Convex free-list construction also scans only the previous slot high-water
mark plus new root demand, selecting the same lowest free slots; mesh worlds keep
the full pool for extra child patches. A focused alternating convex/mesh fixture
checks this ordering and the retained mesh capacity. The final native-cache
library suite passes all 267 tests on NVIDIA, with both Vulkan drivers visible
for the cross-adapter lifetime test. Focused graph-cache opt-out and native-cache
opt-out lifecycle checks also pass. The repeated 100,000-cube comparison above includes these changes.
[Validation record](benchmarks/rtx4070-tiled-validation-2026-09-24.json) lists
the checked cases, native interaction results and source log hashes.

The `falling-cubes` workload compares real Box3D CPU physics with this GPU engine
in three paths: completed physics steps without rendering, the Sokol sample app,
and the direct Rust instanced renderer. Each CPU/GPU pair uses the same scene,
time step and renderer. The direct renderer reads GPU body buffers without the
sample app's CPU pose mirror and debug-draw construction. Its CPU counterpart
runs independent Box3D physics and uploads its own transforms.

The fixture uses 1 m cubes, one floor and up to ten layers. Layer spacing is
1.25 m with alternating ±0.125 m offsets; starting heights are 6–17.25 m. Counts
are dynamic cubes, excluding the floor. Gravity is −10 m/s², dt is 1/60 s with
four substeps, default materials, and sleeping is disabled. Measurements cover
steps 91–330 (1.5–5.5 simulated seconds), including landing and pile settling.
This measures an awake falling pile, not continuous spawning or sleeping-world
performance. The C viewers/oracle share a scene header; the Rust fixture uses
exact binary-fraction coordinates, checked against the CPU viewer at startup.

Benchmark windows ignore interactive key/mouse controls; normal viewers retain
them. The sweep runs three trials per path/count, reversing path order on alternate
trials. FPS is the reciprocal of the median **trial mean** frame/step time;
p50 and p95 milliseconds and trial ranges are retained. Standalone `--bench`
files leave the legacy `cpu_win_validated` flag false; CPU crossover claims here
come from the matched multi-path, three-trial sweep. App timings use full
frame cadence, including presentation and a final device drain in the direct
viewer. GPU physics timings wait for completion. Startup, allocation and shader
loading occur before the timed window. Sokol's physics submission time alone is
not completed GPU simulation time. Cameras frame the whole workload, with each
viewer's own camera, lighting, UI and shading; this is an application comparison,
not an isolated renderer microbenchmark.

The following real viewer captures use 1,000 cubes, four substeps and sleeping
disabled on the NVIDIA GPU, both at 2040×1148. They were captured separately,
outside timing runs, at different simulation steps. Sokol includes its debug UI;
the direct renderer uses its own camera and shading. The on-screen timing in the
Sokol preview is illustrative and is not a benchmark result.

| Sokol sample app | Direct instanced renderer |
|---|---|
| ![Sokol: 1,000 falling cubes](benchmarks/falling-cubes-sokol.png) | ![Direct renderer: 1,000 falling cubes](benchmarks/falling-cubes-direct.png) |

Each path stops independently at ≤10 FPS, invalid results, or a real capacity
limit. The default count schedule extends to 1,000,000 cubes, with each path
stopping at its own measured threshold or validation limit. Physics and the direct renderer use full-width
shape pairs and can test beyond 65,535 dynamic cubes, subject to device capacity
and validation. Native GPU metadata and CPU/GPU identity mappings use per-world
chunk tables. Both Sokol paths reserve at least `GPU_BENCH_CUBES + 1` debug
shapes and opaque instances per stream, then verify that all cubes plus the floor
reach each frame's renderer upload. `GPU_SAMPLES_SHAPE_CAPACITY` sets an explicit
startup reservation for other scenes; the ordinary default is 65,536. The pool
keeps stable pointers for the app lifetime. Transparent-stream limits are unchanged.
Falling Cubes fixes its draw distance at 1,000 m; the harness records saved renderer
settings and rejects changes to them during a sweep. Dropped
pairs/contacts, invalid bodies,
incomplete windows and mismatched framebuffers invalidate a measurement. The
physics/direct GPU paths check final completed state; Sokol's asynchronous
contact telemetry is checked throughout. Thresholds are brackets between tested
counts, not interpolated capacities or guarantees for other scenes.

Build before measuring, so compiler work does not compete with the benchmark:

```sh
# From experiments/gpu-physics, on the supported Linux native-cache path:
python3 scripts/run-native-samples.py cpu --build-only
python3 scripts/run-native-samples.py gpu --build-only
./scripts/build-native-cache.sh build --release
cmake --build oracle/build --target box3d_oracle -j 6
python3 scripts/check-viewer-cpu.py

# Real desktop display; these driver overrides match the measured NVIDIA laptop.
export __NV_PRIME_RENDER_OFFLOAD=1
export __GLX_VENDOR_LIBRARY_NAME=nvidia
export VK_DRIVER_FILES=/usr/share/vulkan/icd.d/nvidia_icd.json
python3 scripts/bench-falling-cubes.py artifacts/falling-cubes/my-machine \
  --adapter nvidia --gpu-solver global --timeout 900
# Plotting alone requires matplotlib; measurement uses the Python standard library.
python3 scripts/plot-falling-cubes.py artifacts/falling-cubes/my-machine \
  benchmarks/my-machine --title 'Exact CPU / GPU / driver'
python3 scripts/plot-falling-resources.py artifacts/falling-cubes/my-machine \
  benchmarks/my-machine-resources --title 'Exact GPU / driver'
```

`--require-idle` checks background load before each trial and stops without losing
completed trials if CPU use exceeds 15% or NVIDIA GPU use exceeds 10%. A busy
two-second reading is confirmed over a full ten-second CPU averaging window,
so a brief spike does not abort the sweep. GPU readings retain the 10% ceiling.
Each trial retains its idle-check evidence, and atomic checkpoint writes preserve completed
trials and terminal failures across interruption (`scripts/check-falling-resume.py`
exercises this recovery). Repeat the same command with `--resume` after interruption; it requires unchanged binaries, settings
and environment. The optional idle gate may be changed when resuming; the resume
history and individual trials record that policy. Without `--require-idle`, runs
proceed under normal desktop load. Use the repeated-trial ranges and saved load
telemetry to investigate unusually inconsistent results, retaining all valid trials.

For before/after completed-physics comparisons, preserve both executables and run
`scripts/compare-falling-binaries.py <global-solver-manifest.json> <new-output-dir>
--binary before=<preserved-path> --binary after=<new-path> --counts 100000 --trials 3`.
It uses one environment and measurement window, alternates executable order,
checks binary hashes, and retains raw samples, phase costs, allocations, power
snapshots and failed trials. It supplements the full CPU/renderer sweep above.

To isolate solver scheduling costs, `profile-falling-solvers.py` reuses a completed
sweep's binary and environment, verifies its binary hash, and compares component
TGS with global contact-color dispatches. It alternates variant order across three
trials and preserves raw phase timings and machine telemetry. Both variants retain
the falling-cube workload, timestep, four substeps, and disabled sleeping. These
are diagnostic comparisons; promoting a scheduling change still requires solver
agreement checks and the full CPU/renderer sweep.

```sh
python3 scripts/profile-falling-solvers.py artifacts/falling-cubes/my-machine/manifest.json \
  artifacts/solver-profile --counts 5000 15000
python3 scripts/plot-falling-solvers.py artifacts/solver-profile benchmarks/solver-profile \
  --title 'Exact GPU • solver scheduling investigation'
```

The plot rechecks raw-file hashes and trial means, displays completed-step trial
ranges and median trial p50/p95 markers, and separates broadphase, narrowphase, graph construction, preparation,
and solving. Phase bars use medians of per-trial means; their sum is not presented
as an exact decomposition of median completed-step time.

The scaling investigation found and fixed a concurrent island-union bug: an
unconditional atomic minimum could overwrite an earlier parent link and lose a
connection. Compare-and-exchange now attaches only a node that is still a root.
The 15,000-cube regression checks every dynamic contact's island membership and
compares repeated global solves, component solves, and batched graph construction
at steps 1, 65, 78, 79, 90, 150, and 330. All sampled positions, rotations, and
linear/angular velocities match exactly after the fix.

Dynamic coloring now fetches and publishes endpoints cooperatively in batches of
256 contacts, while retaining the reference greedy decision order. Its shared
cache holds at most 512 endpoints rather than the entire world. Worlds above
8,168 bodies select this path automatically; smaller worlds retain their existing
shared/memo scheduling. `GPU_PHYSICS_GRAPH_BATCHED=0` forces the scalar reference
and `=1` forces batching for controlled comparisons. With
`GPU_PHYSICS_GRAPH_MEMO=1`, the optional memo allocation now also supports large
worlds. Batches are anchored to 128-body ranges, with up to four cached chunks
of 256 edges per range, so changes in earlier ranges do not shift later cache
entries. A batch reuses colors only when its length, endpoints and incoming
dynamic-color masks match its cached inputs. Local color-list offsets are rebased against the current
counts; contacts and impulses are never cached. Changed batches run the original
ordered greedy walk. Allocation respects device buffer limits and falls back to
ordinary batching when unavailable; `GPU_PHYSICS_GRAPH_MEMO=0` disables it.
Focused tests cover hits, changed inputs, partial batches, reordered edges and
slot reuse, and the 15,000-cube impact comparison remains exact through step 330.
Global color solving can
still be selected with `GPU_PHYSICS_COMPONENT_TGS=0`; component TGS remains useful
for small independent islands but underutilizes the GPU on a large connected pile.
Spatial-hash insertion storage now grows geometrically with reserved shape
capacity (16 entries per reserved shape, rounded to a power of two, at least
1,024, also covering graph-prefix workspace). The three insertion arrays sit after the scene's fat bounds; their
runtime offsets avoid shader recompilation on growth. Oversized workloads still
report sticky capacity loss rather than silently dropping entries. Contact pools
now reserve 16 slots per reserved body, rounded to a power of two (minimum 256),
and pair capacity follows that reservation with a 65,536-entry floor. Runtime
scratch/hash offsets and chunked prefix scans replace the old fixed ceiling.
Growth preserves contact state and history, relocates occupied lists, and rebuilds
the hash index without compiling new shader variants. Capacity loss still
invalidates the run; unusually dense workloads may exhaust their reservation. A 30,000-cube first-step
regression records 100,305 spatial entries without capacity loss, exceeding the
old limit. The preceding capacity build passed all 260 NVIDIA library tests, including the insertion
boundary, full-width sorting and contact hashing, compaction beyond 65,536 entries,
contact-cache growth, and canonical coloring regressions.
The general static-contact sort now carries full contact indices and sorts on
32-bit body/color keys, removing its `(body << 16) | index` packing. It preserves
canonical contact order between its two stable sorts and retains four radix
passes per sort. Retained-contact lookup now stores two full-width identity words
per physical slot and publishes only the slot reference into the hash table.
A separate preparation dispatch makes both words visible before publication;
retirement leaves them intact until the next allocation phase. This uses the
existing hash allocation and also applies when growth or topology changes rebuild
the index. Grid pair generation now assigns each overlap one exact shared cell,
using insertion ordinals to distinguish cells even when their hash buckets collide.
Retained roots emit their existing pairs once; new overlaps append directly, so
pair generation no longer needs an online atomic packed-key deduplication table.
The independent all-pairs matrix agrees through cell collisions and mutations,
and raw append counts equal unique counts in those fixtures.
Broadphase pairs, retained-contact identities, previous-touching history,
callbacks, events, graph memoization, joint filters, and material lookup now
carry full 32-bit indices for both endpoints. Persistent contacts add an aligned
16-byte identity field; pair, history, and radix arrays use two words per entry.
Sorting preserves high-index-major order and skips unused high bytes: four
passes through 65,536 shapes, six through 16,777,216, then eight. Tests cover all
three widths and high-index contacts through callbacks, events, deletion/remapping,
and slot reuse. The Rust physics slot guards now reflect the public index and
graph-classification ranges rather than a 16-bit pair encoding. Device allocation
limits still apply. Native sample metadata uses stable-address chunks per world,
with direct-index geometry lookup and cleanup; independent CPU mappings also
check generations. `scripts/check-native-metadata.sh` exercises 100,001 entries,
callback growth, world isolation, reuse, and ownership under ASan/UBSan.
The batched pipeline is prepared at world initialization and reused across worlds
and capacity growth, so crossing the selection threshold does not compile a shader.

New completed-step raw runs include `allocations` for primary simulation buffers:
body state, the complete reserved scene heap (including geometry and materials),
contacts, joints, scratch, atomics, and fixed dispatch/query buffers. These are
allocated buffer bytes, excluding readback/timestamp staging, CCD resources,
renderers, pipelines, and driver overhead; they must not be labeled total VRAM.


For fresh CPU comparisons, `bench-falling-cubes.py --gpu-solver global` selects
the same global-color schedule and records the choice. `--gpu-binary PATH`
lets physics/direct runs use a preserved executable while work continues.
The default still selects component TGS; all comparisons must report this setting.

The [pre-tiling CPU/GPU sweep](benchmarks/rtx4070-complete-scene-2026-09-24.json)
and [raw archive](benchmarks/rtx4070-complete-scene-2026-09-24-raw.tar.gz) preserve
the `1559a049` baseline, including all three failed 150,000-cube dispatch attempts.
Earlier investigations retain their own binaries, settings and source snapshots:
[batched graph coloring](benchmarks/rtx4070-batched-graph-2026-09-24-raw.tar.gz),
[capacity expansion](benchmarks/rtx4070-capacity-qualification-2026-09-24-raw.tar.gz),
[full-width identities](benchmarks/rtx4070-full-width-2026-09-24-raw.tar.gz), and
[the initial solver diagnostic that exposed the island-union bug](benchmarks/rtx4070-solver-profile-2026-09-24-raw.tar.gz).
That last diagnostic failed correctness and is not a qualified performance result.

The comparison grid retains the real Box3D CPU column first and records the
current GPU implementation in `2026-09-24-tiled-stable-buckets`. All 19 affected
scenes have 300-frame CPU/GPU clips. [Recording identities and clip checks](benchmarks/rtx4070-tiled-recordings-2026-09-24.json)
verify that the GPU recording binary matches the measured binary and the CPU
oracle is unchanged. Ordinary scene recordings are separate from the no-sleep
benchmark window. Run `bun run compare` to inspect the grid.

The preserved executable SHA-256 identifies the measured program. Raw `git` and
`source_sha256` fields describe the runtime working tree, which can include
documentation edits during a sweep; they are not compiled-source fingerprints.
Engine source hashes, source patches and build logs accompany the raw evidence.

The runner records adapter/driver, CPU, VRAM, power/clock telemetry, framebuffer,
settings, commands and binary hashes. Keep the raw output directory. To compare
a second PC, repeat the same workload/window/worker count/resolution and retain
both datasets. Report measured throughput ratios and CPU differences; GPU model
names or advertised FLOPS alone do not predict this workload's scaling. Host
submission, memory traffic, contact graph work and rendering can each limit it.

## Validation and recordings

Run from this directory on Linux with the required GPU/build dependencies:

```sh
python3 scripts/test-native-portability.py
python3 scripts/test-sokol-capacity.py
cargo test --release --lib -- --test-threads=1
./scripts/correctness-gate.sh artifacts/correctness-review.json
./scripts/native-scene-gate.sh
./scripts/check-both-pointer.sh
./scripts/check-shape-replacement.sh
./scripts/check-native-metadata.sh # Linux; requires the built CPU oracle
./scripts/check-joint-separation.sh
./scripts/check-joint-reaction.sh
./scripts/check-substep-forces.sh
./scripts/check-spawn-stream.sh
python3 scripts/audit-native-api.py --require-complete
```

The API audit intentionally fails while coverage is incomplete. It selects the
same portable build directories as the launcher; `--gpu-build-dir` and
`--both-build-dir` select explicit builds. Use `--require-built` to reject missing
artifacts without requiring full API parity. Missing artifacts are reported as
unknown coverage. Symbol coverage does not certify semantics.
Several diagnostic scripts retain Linux/NVIDIA assumptions. Required checks
reported as skipped or unsupported do not count as passes.

The default Vulkan precision path passes normal and strict ten-drag checks on
Linux Radeon 780M and RTX 4070 Laptop, with independent histories and unchanged
tolerances. After `check-both-pointer.sh`, run the produced `both-drag` binary with
`--ground-strict` on each adapter using `scripts/native-samples-cache-env.sh`.
See [qualification results](../../docs/gpu-physics.md#ground-drag-qualification)
for exact commands, limits and remaining failures.

For drag phase traces, build the fixtures first with `check-both-pointer.sh`,
then select the same build and library. For the cached Linux build:

```sh
GPU_PHYSICS_ADAPTER=amd DRAG_PHASE_RANGE=60:240 ./scripts/check-drag-phases.sh \
  artifacts/drag-phases-amd native-samples/build-both-native-cache-portable \
  target/native-cache-build/release/libgpu_physics.a
```

The diagnostic returns failure when the original strict drag gate fails, while
retaining `result.log` and `phases.log`. Tracing can change scheduling and floating-point code generation, so confirm
findings with the uninstrumented fixture too. Without build/library arguments,
it uses the ordinary portable build produced with caching disabled. For joint/contact
boundaries, also set `DRAG_CONSTRAINT_TRACE=1` and
`GPU_PHYSICS_AB=phase-capture,mesh-candidates`; trace overflow invalidates the
diagnostic. CPU phases 100/101 bracket relaxed joint solves, 102/103 biased solves.
`BOTH_DRAG_TRACE_BODY=-1` traces all five cubes; `B <frame> <cube-index>` records
identify the following state/contact lines. Values 0–4 retain single-cube tracing.

For visual or solver changes, record the affected scenes before committing:

```sh
./scripts/record-box3d-oracle.sh
./scripts/record-snapshot.sh YYYY-MM-DD-label
bun run compare
```

To capture a measured build without rebuilding it, set `GPU_RECORD_BIN` to its
absolute executable path. `GPU_RECORD_ORACLE` similarly preserves a specific CPU
oracle for the first column. Both scripts write `recording-manifest.json` with
binary hashes, requested frame counts, and GPU environment settings. Use the
same solver environment as the run being illustrated, and record outside timing
runs so capture and encoding do not affect benchmark results.

The grid places the real Box3D CPU reference first, followed by GPU snapshots.
Reports and recordings are ignored local outputs under `artifacts/` and
`recordings/`; fresh clones must regenerate them. They are not distributed via
Git LFS. Historical investigations are retained in Git history and local archives.

## Source map

| Path | Purpose |
|---|---|
| `src/api/`, `src/c_abi.rs` | World API and exported bridge |
| `src/sim.rs`, `shaders/` | GPU scheduling and physics kernels |
| `c_abi/` | Native compatibility layer and reference fixtures |
| `native-samples/` | CPU, GPU and combined Sokol viewers |
| `oracle/` | Upstream Box3D reference executable |
| `compiler/` | Optional pinned Vulkan backend patches |
| `scripts/` | Builds, correctness checks, recordings and benchmarks |

Architecture and current limitations: [GPU physics notes](../../docs/gpu-physics.md).
Measured performance and reproduction: [native backend guide](compiler/native-backend/README.md).
