# GPU physics experiment

The [experiment](../experiments/gpu-physics/README.md) implements rigid-body physics
in Rust/WGSL and exposes part of Box3D's native C API. It does not replace the
Box3D WASM package or modify the upstream engine. Native samples can run either
engine or show two independent worlds side by side.

## Support status

| Area | Status |
|---|---|
| Linux NVIDIA GeForce RTX 4070 Laptop GPU | Native CPU/GPU/combined builds and normal/strict retained-history drag checks pass |
| Linux AMD Radeon 780M | Builds, picking and normal/strict retained-history drag checks pass; convex-sweep regression remains |
| macOS / Windows | Build paths and CI matrix added; CI results and physical-GPU runtime validation are still required |
| Intel / Apple hardware | Adapter selection permits capable devices; hardware correctness and performance are unverified |
| Experimental browser target | Does not currently compile; native transport, ABI layout and world-storage assumptions need work |
| Box3D WASM package | Separate implementation; unchanged by the experiment |
| Native API parity | Incomplete; linkable symbols include placeholders and CPU-only comparison passthroughs |
| Performance | Cached Sokol improves over ordinary Sokol on two measured workloads; GPU-only Sokol remains slower than CPU-only |

Platform setup, adapter overrides and cache policy are in the
[backend guide](../experiments/gpu-physics/compiler/native-backend/README.md).
Automatic selection ranks capable devices; it does not benchmark every adapter
or tune the solver for each device.
The [falling-cube benchmark](../experiments/gpu-physics/README.md#falling-cube-scaling-benchmark)
compares completed physics, Sokol frame cadence and the direct renderer under the
same scene settings, recording exact hardware and frame-time distributions.

## Work priorities

1. **Completed: ground-drag agreement.** Both tested GPUs pass the independent
   ten-drag sequence at unchanged normal and strict thresholds. Solver/contact
   regressions pass; the separate AMD sweep and secondary-queue failures below
   remain. These results do not establish general GPU parity.
2. **Next: non-recording native APIs.** Shape replacement, joint separation and reaction
   queries and substep force/torque integration are implemented with the precision
   limits below. Next implement warm-start and speculative-contact controls, then
   world diagnostics and review the remaining worker-count/static-tree API semantics.
   Review CPU-only comparison wrappers alongside each API. Each completed API needs a sample or focused fixture
   exercising independent CPU and GPU behavior, including mutation and lifetime
   cases where relevant.
3. **Queued: GPU engine in WASM/WebGPU.** Repair the experimental browser build
   and add browser runtime tests. Start with Firefox on the development machine:
   WebGPU is reported available there, but unavailable in the local Chromium,
   Brave and Helium installations. Verify adapter/device creation when testing.

The existing AMD convex-sweep mismatch and secondary-queue restrictions are
documented limitations, not current work priorities. Keep reporting their test
failures without weakening assertions; they do not displace the API and browser
work above.

Recording/replay, further application optimization, and Windows/macOS/other-GPU
portability are deferred. The requested falling-cube scaling benchmark and repeated
shader-parsing startup fix are covered in the experiment README. Runtime testing so far is Linux-only on the RTX 4070
Laptop GPU and Radeon 780M. Other platforms/devices still need implementation
where incomplete and build/runtime verification. GPU-only Sokol remains slower
than CPU-only in the measured workloads; optimization is deferred until after
correctness and non-recording APIs.

## Missing features and known failures

The portable-build audit on 2026-09-20 identifies **36 stub definitions and 10
additional placeholders**; **36 concern recording/replay**. It also lists 13
CPU-only comparison wrappers for semantic review, with no additional missing
linked symbols in the selected build. Full native compatibility must not be inferred from scene checks.

| Gap | User-visible consequence / remaining work |
|---|---|
| Joint query limits | Wheel angular separation is unimplemented upstream; its release fallback is zero. Reaction precision limits are described below. |
| World controls | Warm-start and speculative-contact toggles do not control GPU behavior. Worker-count APIs are placeholders rather than GPU scheduling controls. |
| World diagnostics | Profile/max-capacity APIs return placeholders; memory/bounds dump and static-tree rebuild helpers are incomplete. Public `contactCount` is not implemented by the GPU world counter. |
| Recording/replay | Native recording creation, storage, file I/O, playback, seeking and query-history APIs are placeholders. Diagnostic state replay is not an implementation of these APIs. |
| Combined viewer coverage | Some generated wrappers call only CPU APIs. Audit each remaining wrapper before claiming that controls or diagnostics affect/report both worlds. |
| AMD convex sweeps | A rotating capsule reports a GPU hit where the CPU conservative-advancement reference reports no hit. Cause undiagnosed; investigation is deferred. |
| AMD secondary queues | The experimental backend requires two graphics/compute queues in family zero; this AMD device exposes one. Its compute-only queues in another family are unsupported by this path; broader queue-family support is deferred. |
| Browser target | Fix native-only transport/pose dependencies, pointer-size ABI assertions and global world storage before advertising WebGPU support. |

Run `python3 experiments/gpu-physics/scripts/audit-native-api.py --require-complete`
for the function inventory. The audit selects portable builds using the launcher
cache policy; explicit `--gpu-build-dir` and `--both-build-dir` override it.
`--require-built` rejects absent archives or generated wrappers. Missing artifacts
produce unknown coverage rather than a successful empty inventory. CI checks
that builds supply these inputs, without requiring the unfinished APIs to pass.

### Native joint reaction queries

`b3Joint_GetConstraintForce` and `b3Joint_GetConstraintTorque` read the GPU's
completed joint impulses and body poses for all nine joint types. Values use
upstream inverse-substep scaling, including zero output after a zero-duration
step. Sleeping joints retain their impulses and prepared frames. Frame edits
use the same mixture of current and prepared transforms as the native getters.
The combined wrappers return GPU results; the fixture queries the CPU world
independently and checks that a CPU-only step cannot change the GPU result.

The implementation preserves upstream diagnostic conventions: revolute torque
includes both axial terms, wheel force uses the lower suspension limit value,
and prismatic/wheel force adds the upper-limit term. Supporting solver fixes
restore parallel-joint warm starting and softness, both wheel spin-axis warm
start terms, retention of sleeping joint impulses, and preservation of explicit body sleep
through the automatic sleep pass. Distance reaction history
uses existing fields unused by that joint's solver; the GPU layout is unchanged.

`./scripts/check-joint-reaction.sh` checks all joint types with four configurations,
including rotations, springs, limits, motors, an offset anchor, changing timesteps
and substep counts, sleep, frame edits, distance parameter edits, zero-duration
steps and destruction. It asserts nonzero coverage for each supported reaction.
The main fixture uses unit density and a per-component gate of
`1e-5 * max(1, abs(cpu), abs(gpu))`; its result is not general solver parity.
Existing GPU parameter setters wake bodies differently from native, so the
retained-history case explicitly puts both worlds back to sleep.

All four Linux Vulkan runs (AMD/NVIDIA, native sample caching on/off) pass
**2,856 scalar comparisons each**. Both GPUs also pass the normal and strict
ten-drag checks at unchanged maxima. Logs and binary/source hashes are in
`experiments/gpu-physics/artifacts/joint-reaction/`.

The fixture also retains strict failing probes:

- `GPU_REACTION_KIND=5 GPU_REACTION_DENSITY=1000`: high-density revolute precision.
- `GPU_REACTION_KIND=4 GPU_REACTION_OFFSET_STRESS=1`: rotated, offset prismatic
  precision (the unrotated offset case is in the main fixture).

Run these environment settings with the script above. They retain the same
strict gate and must not be counted as passes. The main fixture and probes use
independent GPU/CPU state; no CPU result is substituted for a GPU reaction.

### Native external force integration

Body force and torque accumulators are uploaded for the next positive-duration
step and integrated on every substep, using upstream damping/gravity order and
step-start world inertia. This replaces the previous whole-step impulse applied
before substeps. Zero-duration steps preserve pending loads. Sleeping, static and
kinematic bodies retain the existing API eligibility rules; impulses remain
immediate operations.

The per-body scene extension grows from 32 to 64 bytes to hold force and torque;
no storage binding or body pose layout changes. Queue uploads clear previously
loaded slots before the next positive-duration step, including when no new force arrives. A step
with a bound load cannot take the resident shortcut until that clearing upload
has occurred. Idle proofs are invalidated on load changes; scene rebuilds start
with zero loads. Body destruction/reuse cannot inherit an old load.

`./scripts/check-substep-forces.sh` compares independent CPU/GPU positions,
rotations and velocities for 1/2/4/8 substeps, gravity, damping, rotated anisotropic
inertia, accumulated and off-center forces, sleep/wake, zero-duration steps,
clearing, rebuilds, neighboring bodies across holes and slot reuse. All four
AMD/NVIDIA cache-on/off runs pass **3,744 scalar comparisons each**, retaining
the `1e-5` component gate. The motor
reaction probe `GPU_REACTION_KIND=1 GPU_REACTION_APPLIED_TORQUE=1` with
`./scripts/check-joint-reaction.sh` is now a regression check for the fixed
substep timing and passes **312 comparisons** in each of the same four modes.
The separate precision probes above remain limitations.
Local logs and source/binary hashes are in
`experiments/gpu-physics/artifacts/substep-forces/`.

### Native joint separation queries

`b3Joint_GetLinearSeparation` and `b3Joint_GetAngularSeparation` read completed
GPU body poses and current joint settings. Combined-view getters return GPU
results; fixtures query the separately namespaced CPU engine independently.
The implementation follows upstream `joint.c`, including its body-rotation
convention for angular error and its single perpendicular component for slider
linear error. Local anchor positions affect linear separation; local frame
rotations do not replace the body rotations used by these upstream diagnostics.
Spring/limit settings and weld softness determine which errors count. Angles
use Box3D's deterministic `b3Atan2`, including quaternion-polarity behavior.

Linear queries cover all nine native joint types. Angular queries cover the eight
upstream implementations. **Wheel angular separation is not implemented by
upstream** (debug assertion, release zero); the GPU returns zero without claiming
an angular wheel diagnostic. These getters do not change solver arithmetic.

The focused fixture covers configured poses, spring/limit combinations and edits,
nontrivial local frames, offset centers of mass, quaternion polarity, destroyed
joints, sleeping bodies and pending-step reads, at the unchanged `1e-5` tolerance:

```sh
# From experiments/gpu-physics
GPU_PHYSICS_ADAPTER=amd ./scripts/check-joint-separation.sh artifacts/joint-separation/cached
GPU_PHYSICS_ADAPTER=nvidia ./scripts/check-joint-separation.sh artifacts/joint-separation/cached
GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0 GPU_PHYSICS_ADAPTER=amd \
  ./scripts/check-joint-separation.sh artifacts/joint-separation/ordinary
GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0 GPU_PHYSICS_ADAPTER=nvidia \
  ./scripts/check-joint-separation.sh artifacts/joint-separation/ordinary
```

On 2026-09-20 all four adapter/cache combinations pass 2,686 independent
comparisons each, plus a CPU-only mutation check proving the combined getters
remain GPU-backed. The native API audit has no additional missing linked symbols.
Local evidence is in `experiments/gpu-physics/artifacts/joint-separation/`.
These are Linux Vulkan checks; wheel angular separation and other platforms
remain outside the implemented/qualified scope.

### Native shape replacement

`b3Shape_SetSphere`, `SetCapsule` and `SetHull` replace GPU geometry in place.
They retain shape IDs, material/filter/event settings and user data. Shape mass
data changes immediately; body mass, center of mass, inertia and stored CCD
extents change only when `b3Body_ApplyMassFromShapes` is called. Retiring touching
contacts wakes their owners and connected sleeping islands; an isolated sleeping body stays asleep. An identical hull is a no-op,
including input from `b3Shape_GetHull`. The bridge owns complete hull copies,
including the face data and padding used by upstream's content comparison.

Replacement retires affected GPU contact roots and public contact handles,
updates query/CCD geometry and invalidates cached scene uploads. Unrelated
contact histories remain intact. Public shape/body AABB queries include the
native 0.02 m speculative margin. Both comparison engines execute the mutations
independently, and both renderers rebuild replaced geometry.

Run **GPU API / Shape Replacement** in any native viewer. Its shared input
schedule cycles through an offset sphere, capsule, box and cylinder every 180
steps, with explicit mass recomputation on alternate replacements. The focused
fixture checks metadata, queries, mass policy, motion, sleeping contacts, hull
aliasing, debug geometry destruction/recreation and geometry ownership:

```sh
# From experiments/gpu-physics; defaults to the native-cache backend on Linux.
GPU_PHYSICS_ADAPTER=amd ./scripts/check-shape-replacement.sh artifacts/shape-replacement/cached
GPU_PHYSICS_ADAPTER=nvidia ./scripts/check-shape-replacement.sh artifacts/shape-replacement/cached
GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0 GPU_PHYSICS_ADAPTER=amd \
  ./scripts/check-shape-replacement.sh artifacts/shape-replacement/ordinary
GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0 GPU_PHYSICS_ADAPTER=nvidia \
  ./scripts/check-shape-replacement.sh artifacts/shape-replacement/ordinary
```

On 2026-09-20 the fixture passes on AMD Radeon 780M and NVIDIA RTX 4070 Laptop,
both with native-cache and with cache opt-out. It retains the existing `1e-5`
motion tolerance and checks that unrelated sleeping contacts survive replacement.
CPU and combined AMD viewer health scans complete 720 measured steps without
NaNs or exploded bodies. The free-running contact scene can diverge in transient
capsule tipping time; this is not a long-running pose-parity gate. A diagnostic
reproduces the initial 180-step sphere motion exactly through the existing
destroy/create path in each engine, establishing divergence before the capsule
phase. Local logs, scene captures and source/binary hashes are
in `experiments/gpu-physics/artifacts/shape-replacement/RESULTS.md`. These are
Linux Vulkan correctness checks; other platforms and performance remain unqualified.
The comparison grid includes a `shape-replacement` row made from the recorded
CPU/GPU panes. This native sample is not hosted by `record-snapshot.sh` or the
Rust oracle replay renderer; its CPU pane runs real Box3D C.

### Ground-drag qualification

The default native-cache path passes all ten drags on both Linux Vulkan adapters,
including release/re-grab with retained histories. Each world performs its own
picking and joint creation; CPU state is never copied into the GPU simulation.
Both GPUs produced these maxima on 2026-09-20:

| Measurement | Observed | Unchanged limit |
|---|---:|---:|
| Held position difference | 0.002881 m | 0.005 m |
| Held quaternion chord | 0.005064 | 0.01 |
| Held velocity difference | 0.047432 m/s | 0.1 m/s |
| Settled velocity difference | 0 m/s | 0.02 m/s |
| All-frame position difference | 0.006028 m | 0.025 m |
| All-frame quaternion chord | 0.005973 | 0.025 |
| Strict all-frame velocity difference | 0.230405 m/s | 0.5 m/s |

Reproduce from `experiments/gpu-physics` (Linux cached backend):

```sh
GPU_PHYSICS_ADAPTER=amd ./scripts/check-both-pointer.sh artifacts/drag-check
source scripts/native-samples-cache-env.sh
GPU_PHYSICS_ADAPTER=amd artifacts/drag-check/both-drag --ground-strict
GPU_PHYSICS_ADAPTER=nvidia artifacts/drag-check/both-drag --ground-strict
```

No experimental precision flag is required. The first command also checks picking
and isolated dragging. Local qualification logs are under
`artifacts/drag-agreement/default-*`. Diagnostic CPU-state replay remains a
first-divergence tool, not evidence of independent simulation agreement.

The broader AMD library suite exposes a separate convex-sweep failure: case 20 of
`gpu_convex_sweeps_match_cpu_conservative_advancement` reports a GPU hit at
fraction 0.115237534 where the CPU reports no hit. It reproduces in the earlier
2026-09-19 test binary; the sweep shader and test are unchanged by the drag fixes.

Gear Lift's current native gate measures geometric support, penetration duration,
floor crossings and joint behavior over 1,200 steps. Its earlier chaotic rock
trajectory mismatch is not a bitwise-parity guarantee. Village has bounded
600-step sphere-drop and scene-transition coverage, not exhaustive interaction
coverage. Neither scene establishes full native API compatibility.

## Architecture and state ownership

Native Vulkan physics preserves explicit scalar arithmetic order with SPIR-V
`NoContraction` decorations, including explicit FMA residuals used for corrected
division and square roots. Other backends retain the WGSL path and still need
equivalent numerical validation. Compiled modules are cached by shader source,
entry point and exact override values; no device-specific solver constants are used.

The drag investigation found several rounding differences that accumulated across
contacts: fused vector operations, approximate reciprocals, square-root double
rounding, and different matrix and impulse evaluation order. Box clipping now
keeps the native local-coordinate calculation order and preserves original contact
separations for recycling instead of reconstructing them from solver offsets.
Mass queries retain the original mass instead of inverting its rounded reciprocal,
so independently created pointer joints receive matching inputs. These changes
preserve independent simulations and the existing comparison thresholds.

| Layer | Responsibility |
|---|---|
| Upstream C helpers | Hull cooking, geometry utilities and the independent CPU reference |
| Rust | World/ID lifetimes, uploads, callbacks, host queries, events and CCD fallback |
| WGSL | Broadphase, contact generation, graph scheduling, constraint solving, integration and closest-hit queries |
| Native sample bridge | Box3D C interface, independent comparison worlds and current-state drawing |

Body/shape/joint slots retain generation-checked identities. Capacity growth must
preserve live contacts and constraint history; allocation or contact-capacity
loss must be reported, not silently interpreted as a successful simulation.

Native `b3WorldDef.capacity` body/shape hints now reach the GPU world in both
GPU-only and combined builds. Set the expected peak **simultaneously live** counts
before `b3CreateWorld`; include scenery and gameplay objects, not total lifetime
spawns. Shape hints also reserve the default per-shape material entries. For
example, a level expecting 4,096 live projectile bodies/shapes in addition to its
scenery can add 4,096 to `dynamicBodyCount` and `dynamicShapeCount`. Destroy expired
projectiles to return their generation-checked slots to the pool. The hints are
advisory; exceeding them grows capacity geometrically, rather than dropping spawns.
`contactCount` is still not a configurable GPU contact allocation budget.

Without larger hints, initial body/shape/material allocations include bounded
headroom (25% of the live count, clamped to 64–256 spare entries, subject to the
existing minimum capacities). Geometry has separate capacities: adding new hull
or mesh data can still cause growth. Mutations are collected into the scene upload
on the next step/query; create a batch before requesting completed-state reads.

Compiled physics programs and layouts belong to the logical GPU device and are
shared across its worlds and buffer reallocations. Mutable buffers, bindings,
contacts and command replays remain world-owned. Graph memo offsets are derived
from reserved capacity at runtime, so growth does not compile a capacity-specific
shader. Pipeline-cache files are saved during explicit loading or world/device
teardown, not while creating a runtime pipeline or growing a scene. Allocation
and scene uploads are still synchronous; these changes remove the repeated
shader/cache work, not every cost of a large topology edit or bullet CCD.

The Shift-click regression runner calls the actual sample handler:
`experiments/gpu-physics/scripts/check-shoot-latency.py` takes a prebuilt native
binary, adapter and output directory, and uses the launcher's runtime environment.
It checks that each shot adds one body, the current step is drawn and no contact
capacity loss is reported. `check-spawn-stream.sh` compares independent CPU/GPU
worlds through 4,096 launches, up to 1,024 live bullets, automatic growth, explicit
reservations and expired-slot reuse at the unchanged `1e-5` scalar tolerance.
The fixture executable also accepts a peak-live argument; `spawn-stream 4096`
checks 16,384 launches with 4,096 live bullets, both automatic and reserved.

Local Linux qualification on both GPUs covers three scripted shots each in Box
Stack, Dominoes, Mixed Stacks 4096, Mesh/Grid, Revolute and Village. Dominoes'
first-shot physics step fell from 400 to 12 ms on NVIDIA and 522 to 14 ms on AMD.
The 4,096-live stream passes 22,044,672 independent scalar comparisons per adapter;
normal/strict drag results are unchanged. This does not establish 60-fps gameplay:
Village still takes about 92–94 ms on later topology-changing shot steps and is
slow between shots too. Evidence and complete frame timings are in
`experiments/gpu-physics/artifacts/shoot-stall/`.

Stepping submits GPU work. Synchronous public getters and queries finalize the
required step, including pending CCD, before returning current state. Events are
harvested once; unread events expire at the next step. Callbacks use owned query
snapshots so nested queries do not hold the world lock. Mutations invalidate
cached state and upload affected data.

The resident GPU path and convex GPU CCD are eligibility-based optimizations.
Unsupported geometry, callbacks or other excluded world features retain the
ordinary path. Sleep dispatch elision requires a valid zero-active proof across
an unchanged eligible submission chain. Command replay must retain resources
through completion and invalidate on relevant parameter/resource changes.

The combined viewer never copies CPU poses or hits into GPU simulation. Both
panes draw the same completed step. Its shared sample controller and GPU-led
inspector remain limitations when interpreting sample-specific behavior.
Linux GL pose imports require matching Vulkan/OpenGL device UUIDs; otherwise
the viewer uses current CPU pose mirrors. Other platforms use mirrors directly.
The separate Linux split-adapter experiment displays N-1 while simulating N;
its CPU control uses the same delay. Those results do not describe Sokol.

GPU contact metrics describe the contact-scheduling phase and carry step and
state revisions. Unknown or stale snapshots must remain labelled. Candidate
pairs, allocated roots, manifold slots and touching roots are different counts;
none should be presented as the unimplemented native public contact counter.

## Validation and merge requirements

Merging as an **experimental native engine** does not require completing every
Box3D API or proving a speedup on every GPU. It does require accurate limitations,
reproducible builds and a current regression result. A production-compatible
replacement would additionally require closing the API and physics gaps above.

The 2026-09-20 native-cache release library suite reports **249/249 passed on
NVIDIA** and **243/249 passed on AMD**. One AMD failure is the pre-existing
convex-sweep case above; five secondary-queue tests fail because RADV exposes
fewer than the two required graphics/compute queues in family zero. Local
`vulkaninfo` reports one such queue on AMD versus 16 on NVIDIA; AMD also has four
compute-only queues in family one, which the current same-family backend does
not support. These five tests fail during device creation, before exercising
queue handoff or pose lifetime. They remain failures, not passes or silently
skipped checks. Both GPUs pass the solver/contact,
mass-query, command-replay, pose-staging and event regressions. The six native
tooling tests also pass. Normal and strict ten-drag checks passed during the
substep-force and spawning qualifications with the unchanged maxima above. Current force
evidence is in `experiments/gpu-physics/artifacts/substep-forces/`. Spawning,
capacity growth and current library regression evidence is in `artifacts/shoot-stall/`;
joint-query evidence is in `artifacts/joint-reaction/`. Prior replacement and
drag evidence remains in `artifacts/shape-replacement/` and
`artifacts/drag-agreement/` within the experiment.

Earlier merge-review checks covered TypeScript, lint (with warnings), the demo
build, a two-restart Junkyard smoke, cached/ordinary native builds and artifact
audits. The 12-case native scene matrix included 1,200-step Gear Lift, 600-step
Village and a Village → Bounce House switch, using NVIDIA physics and AMD graphics.
Those results predate the drag fixes; they are not current scene-matrix or
cross-platform qualification. Logs are in
`experiments/gpu-physics/artifacts/merge-review/`. WASM was not rebuilt.

Before merging:

- Verify clean Linux CPU/GPU/combined builds with and without caching.
  Windows/macOS CI and hardware verification are deferred; retain Linux-only
  runtime claims until that work is complete. The CI matrix remains available
  for future platform validation.
- Run the complete correctness gate and cached-backend lifecycle/CCD/event
  tests on the final code. The earlier 58-case correctness result predates the
  portability changes, and the 12-case native matrix predates the drag fixes.
  Refresh both before merging and retain failures/skips with source identity.
  The native scene gate now freezes binaries from the portable launcher and
  records/applies its runtime configuration; cache opt-out uses the same gate.
- Exercise the final launch path: default cache, cache opt-out, AMD selection,
  hybrid graphics fallback, repeated scene changes, dragging, pause/single-step,
  sleep/wake and shutdown during loading. Preserve independent step identities.
- Verify the original WASM/demo workflow still builds and typechecks. The
  2026-09-19 review passed the demo build, workspace typechecking and lint
  (with warnings); it did not rebuild WASM. The subsequent cleanup repaired
  Junkyard checks to use native object counts and all remaining bridge pools.
  Full TypeScript checking and a two-restart Junkyard smoke now pass.
- Review public C placeholders and generated CPU-only wrappers. Unsupported
  controls must not be presented as functioning GPU features. GPU warm-start
  and recording controls are now unavailable; the combined worker slider is
  explicitly CPU-only. Sample-specific controls still need a compatibility audit.
- Publish a compact test summary with the PR. Raw captures are ignored local
  files; archive any evidence needed by reviewers outside the checkout.

The large initial engine addition also needs a code review focused on GPU buffer
bounds, ID reuse, C/Rust layouts, callback reentrancy, command-cache lifetimes and
failure cleanup. Build success alone cannot establish these properties.

Benchmark reports must distinguish device time, completed physics time and whole
application cadence. Compare equal scenes, CPU worker counts, framebuffer sizes,
pacing, sleep settings and display delay; retain all interleaved trials. The
[current measurements](../experiments/gpu-physics/compiler/native-backend/README.md#measurements)
include their scope and do not establish a general CPU advantage.
