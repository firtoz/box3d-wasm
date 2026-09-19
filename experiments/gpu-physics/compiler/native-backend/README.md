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
