# GPU physics experiment

The [experiment](../experiments/gpu-physics/README.md) implements rigid-body physics
in Rust/WGSL and exposes part of Box3D's native C API. It does not replace the
Box3D WASM package or modify the upstream engine. Native samples can run either
engine or show two independent worlds side by side.

## Support status

| Area | Status |
|---|---|
| Linux NVIDIA GeForce RTX 4070 Laptop GPU | Native CPU/GPU/combined builds and normal/strict retained-history drag checks pass |
| Linux AMD Radeon 780M | Earlier native builds, picking and normal/strict retained-history drag checks pass; the current large-world scaling changes are qualified on NVIDIA. The AMD convex-sweep regression remains. |
| macOS / Windows | Build paths and CI matrix added; CI results and physical-GPU runtime validation are still required |
| Intel / Apple hardware | Adapter selection permits capable devices; hardware correctness and performance are unverified |
| Experimental browser target | Does not currently compile; native transport, ABI layout and world-storage assumptions need work |
| Box3D WASM package | Separate implementation; unchanged by the experiment |
| Native API parity | Incomplete; linkable symbols include placeholders and CPU-only comparison passthroughs |
| Performance | RTX 4070 Laptop solver-writeback follow-up versus `0bbf5f63`: median completed-step time falls 7.7% at 2k cubes and 1.5% at 5k, below the 20% target. The 100k and 200k cases improve 10.7% and 7.9%, with every paired large-scene trial faster. In the matched sweep, the first sampled GPU/CPU physics crossover moves from 20k to 15k cubes. Small-scene timings remain variable; this is not a universal speedup or crossover threshold. See the experiment README for dated charts, renderer measurements, raw evidence and earlier benchmark history. |

The solver-writeback change stores only contact impulses during solving; it
preserves geometry, prepared coefficients, arithmetic and scheduling. A controlled
dispatch-versus-workgroup probe supports retaining the existing occupancy-based
schedule. Narrow body writes and shader specialization did not yield reliable
additional completed-step gains and remain rejected experiments. The native-cache
suite passes 276 tests, plus nine cache-disabled checks; the timing probe passes
separately. Box3D and the WASM package are unchanged.

The earlier connected-scene scheduling follow-up compares against `c46b81e2` on the same
RTX 4070 Laptop. The repeated physics sweep measures about 2%, 15% and 20% lower
median completed-step time at 1k, 2k and 5k falling cubes; the 25% target was not
reached, and the 1k result is noisy. Dense small worlds now choose batched graph
memoization using an asynchronous contact-density hint; sparse worlds keep the
shared path. The initial 100k slowdown was investigated with balanced three-way
repeats and did not consistently reproduce; the report retains both rounds.
See the [experiment benchmark history](../experiments/gpu-physics/README.md#falling-cube-scaling-benchmark)
for the final charts, exact configurations, variability and evidence.

The [scaling follow-up](../experiments/gpu-physics/README.md#falling-cube-scaling-benchmark)
fixes a concurrent island-link loss and adds batched contact coloring above 8,168
bodies. With global color solving, the 15,000-cube completed step is about 11.9 ms
in repeated diagnostic runs. Runtime insertion/pair/contact reservations now
allow the larger runs above. Full-width shape identities and growable native
metadata are implemented, and the Sokol opaque renderer reservation scales with
the benchmark count. Production direct, indirect and native cached dispatch now
share tiled indexing beyond the one-dimensional workgroup limit. Boundary and
lifecycle tests pass with native caches enabled and disabled. Dirty contact-range
clearing and stable body-range graph-color reuse reduce work without changing
ordered greedy assignments; the final native-cache library suite passes 267 tests.
The earlier complete renderer sweep stopped at a software dispatch limit rather
than a measured GPU memory or frame-rate ceiling. The replacement sweep reaches
frame-rate stops on all six paths, with no rejected trials; VRAM exhaustion remains
unmeasured.

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
portability are deferred. The completed falling-cube scaling benchmark and repeated
shader-parsing startup fix are documented in the experiment README. Runtime testing so far is Linux-only on the RTX 4070
Laptop GPU and Radeon 780M. Other platforms/devices still need implementation
where incomplete and build/runtime verification. The CPU/GPU crossover depends on body count and renderer; the experiment README
records measured ranges rather than a general claim that one engine is faster.

## Missing features and known failures

The portable-build audit on 2026-09-20 identifies **36 stub definitions and 10
additional placeholders**; **36 concern recording/replay**. It also lists 13
CPU-only comparison wrappers for semantic review, with no additional missing
linked symbols in the selected build. Full native compatibility must not be inferred from scene checks.

| Gap | User-visible consequence / remaining work |
|---|---|
| Joint query limits | Wheel angular separation is unimplemented upstream; its release fallback is zero. Reaction precision limits are described below. |
| Falling-cube GPU capacity | The archived sweep hit the old 65,536 spatial-insertion limit at 20,000 cubes. Insertion buffers now scale with reserved shape capacity; the static-contact sort also no longer packs body/index into 16-bit halves. Pair/contact buffers and their prefix scans now scale with reserved body capacity. Pair/history storage and callbacks/events now use full-width endpoints; canonical cell ownership removes online packed-key deduplication. High-index collision, event, remap, and reuse regressions pass. Native sample C metadata now uses per-world growable chunks and passes high-index ownership/callback regressions; Sokol debug/opaque renderer reservations now follow benchmark size, and both engines must upload every cube and the floor. The sample fixes a saved draw-distance culling issue. Updated full-scene app measurements replace the earlier unvalidated Sokol curve. Collision-heavy physics and direct rendering are qualified through 200,000 cubes across three 330-step trials each; Sokol stops below 10 FPS at 150,000. The former tiled-dispatch boundary is resolved. The 200,000-cube physics buffers occupy 3.42 GiB, excluding renderer/driver resources; this is not a measured memory ceiling. |
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

Each CPU body also owns an ordered list of its live shape slots. Bounds and mass
recomputation visit that body's shapes rather than scanning the world. This removes
the quadratic attachment cost for independent single-shape bodies. Repeatedly
recomputing one growing compound still visits its own children; it is not constant
time. Deletion removes ownership entries, and reused bodies start with an empty list.

The direct and native Sokol GPU viewers create scenes and prepare physics shaders
on a worker while their host event loops draw loading status. No simulation steps
run during this preparation. The direct viewer can first draw once its graphics
device is ready; Sokol can draw before the physics device initializes. Closing Sokol
may wait for an in-flight driver operation while continuing to draw closing status.
On a Sokol scene switch, cached renderer meshes are retired on the host thread
before the worker destroys the old physics world. Mouse-capture changes are also
applied by the host. These operations require the window/graphics context and
must not run from scene constructors or destructors on the worker.
Startup measurements and their cache/timer boundaries are documented in the
[experiment README](../experiments/gpu-physics/README.md), including the completed
96-launch cold/warm comparison, the 270-test library suite and repeated steady-state
comparisons. The startup fix shows no consistent steady-state regression in those
runs. All 19 recorded scenes match the baseline GPU's 300 decoded frames each;
the [portable evidence archive](../experiments/gpu-physics/benchmarks/rtx4070-startup-2026-09-25-raw.tar.gz)
passes fresh-extraction hash and raw-record validation.

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

With the native command-cache build and `GPU_PHYSICS_FULL_REPLAY=1`, command
replay covers eligible global color schedules as well as component schedules.
`GPU_PHYSICS_GLOBAL_REPLAY=0` restores the previous global recording path.
Replay keys include the color-grouping prefix, and captured counters preserve
the actual solver dispatch count. Joints, mesh triangles, phase capture, active
metric capture and the eligible sleep-elision path keep their ordinary recording
paths. This is command reuse, not simulation-state playback: contacts and body
state are still computed on the GPU each step. Small-scene timing and the separate
adaptive-color configuration, dated charts and reproducible raw evidence are
documented in the [experiment README](../experiments/gpu-physics/README.md).

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
An explicit GPU completion wait refreshes the small status record if its
asynchronous slot still describes an older step, so completed capacity failures
are reported even after a burst of queued steps. Nonblocking metric reads may
still return an older snapshot; the wait does not download the body mirror.

## Validation and merge requirements

The paired-drop scene is `Determinism / Falling Ragdolls` (four pairs);
`Benchmark / Rain` uses the same human builder (three per group in release,
two in debug). The GPU spherical-joint twist Jacobian now matches upstream:
`coneAxis + tan(halfSwing) * cross(swingAxis, coneAxis)`. Previously the scalar
multiplied the cone axis instead, disabling twist limits at zero swing and
applying them about the wrong axis at other angles. An aligned joint spinning
at 5 rad/s should stop at its 0.2-radian limit on step 3; the old GPU kept spinning.

Joint creation order was a second source of drift: the GPU solved adjacent
bones consecutively while the CPU solved constraints by graph color. GPU
component lists now follow the CPU's overflow-first, ascending-color priority,
including color reservations for filter joints. TGS contacts reserve colors
from the same body masks as joints and solve in the same color wave. Small
worlds (at most 256 bodies) use a single workgroup with barriers between colors;
larger worlds dispatch each color separately. The serial-joint diagnostic keeps
its creation-order walk. GPU colors are rebuilt each step, whereas the CPU
retains colors for existing constraints; contact insertion order and mesh
manifold differences can still produce different impact trajectories.

From `experiments/gpu-physics`, run `./scripts/check-ragdoll-reference.sh`.
Set `GPU_RAGDOLL_NATIVE_CACHE=1` to exercise the native viewer's cached Vulkan
path instead of ordinary wgpu. It compares both twist directions with aligned
and tilted frames against the independent C engine at `1e-5`, isolates joint
dynamics by disabling collisions for 60 steps, then runs the exact upstream
eight-ragdoll drop for 600 steps. The collision-free position error is at most
0.013 mm on NVIDIA, down from roughly 4 mm before matching joint order.

For the full drop, the largest position difference on step 1 falls from 3.97 mm
to 0.097 mm. RMS position error during mesh impact (step 88) falls from
12.6 mm to 4.5 mm, and at step 143 from 0.490 m to 0.362 m (26% lower). This is
not trajectory parity: the worst individual position error at step 143 rises
from 0.978 m to 1.071 m, and later contact-sensitive poses still differ.
The maximum position difference across steps 0–60 is 5.13 mm (previously
8.07 mm), including self-contact in the first ragdoll pair.

The corrected GPU drop peaks at 76 rad/s (CPU: 102) and settles during steps
480–600 with a maximum joint separation of 1.58 mm. Before the twist-Jacobian
fix it peaked at 158 rad/s and still reached 19 rad/s in that final window.
The gate checks initial state agreement, collision-free joint dynamics,
pre-impact position differences, transient joint stretch, and settling
separately, and reports max/RMS errors at impact checkpoints. It qualifies
this fixture, not the complete Rain benchmark. Ordinary and native cached GPU
traces match exactly for all six cases. The GPU regression suite also
checks color order in a connected chain and equality between single-workgroup
and general dispatch with 80 jointed pairs sharing static ground.

The subsequent visual-agreement goal is stricter than those stability results:
`validate-ragdoll-reference.py` now checks every frame from 0 through 600 for
position RMS <= 1 cm / maximum <= 5 cm and quaternion angular RMS <= 1 degree /
maximum <= 5 degrees. It normalizes quaternion inputs and treats q and -q as
the same orientation. `frame-errors.json` records all frames, and `result.json`
reports visual failure separately from stability. The shared-color baseline
fails first at step 88 (17.32 degrees on body 43), so its earlier stability pass
does not satisfy this goal.

An identical-state replay from CPU step 87 identified a further CCD mismatch:
convex sweeps interpolated the body origin instead of the center of mass.
CCD proxies now use COM-relative coordinates in both host and GPU convex
paths. The offset rotating-capsule regression compares physically identical
centered and offset geometry on both paths. Solid mesh sweeps also use the
upstream centroid-plane early-out, while sensor sweeps retain crossings.
These fixes do not establish visual parity: the full ragdoll trace still first
fails the angular target at step 88. Current diagnosis artifacts are under
`experiments/gpu-physics/artifacts/ragdoll-impact-goal`; the updated 600-step
trace is under `artifacts/ragdoll-com-sweep-reference` in that experiment.
A translated two-capsule fixture isolated the next mismatch: world-space
endpoint rounding changed the spine contact from +6 cm to -6 cm at x=8.25 m,
reversing its torque lever arm. Capsule-capsule narrow phase now works in
body A coordinates and rotates local contact lever arms back to world axes.
The segment-distance degeneracy thresholds also match Box3D. The translated
fixture agrees with the independent C oracle endpoint at x=0, 7.5, 8.25, and
-7.5 m; `capsule_contact_endpoint_is_translation_invariant` locks this down.
The contact-reuse check now compares body-origin relative transforms and
uses the uploaded whole-body motion bounds instead of the first shape's
render dimensions. Equal-type capsule manifolds use later-proxy-first order,
matching Box3D's initial dynamic-proxy pairs. This fixes a frame-2 discrepancy
where GPU retained the frame-1 thigh-to-spine normal until impact. The oracle
fixture now compares the first spine's complete frame-2 contact set at a
1e-4 normal tolerance; maximum normal error is 1.26e-5.

Jointed TGS preserves surviving contact colors before assigning new or
conflicting contacts. The collision stage carries the previous solve color
through graph rebuilding; freeing a lower color no longer changes another
contact's Gauss-Seidel priority. The 21 joint-filtered tests pass, including
a contact-separation regression that also passes with the native cache.

An initial graph trace found 14 of 24 ragdoll contact colors differed from CPU.
A diagnostic using CPU's first-step colors isolated that ordering effect.
Production now derives the order independently: `broadphase_order.rs` ports
Box3D's insertion-only AABB tree and assigns leaf ranks from the scene's fat
bounds. A first-step GPU heap sort reproduces proxy/query creation priority
before assigning jointed contact colors. The tree runs only during initial
setup of jointed worlds with supported dynamic convex shapes; compound or
mesh dynamics retain the existing ordering. Shape metadata is 192 bytes.
No recorded CPU colors or simulation states enter the production calculation.
The tree matches all 112 ragdoll leaf ranks and independent C fixtures covering
rotations, symmetric costs, and duplicate centers.

The independent production trace matches all 10,192 states from the ordering
diagnostic through frame 90. Ordinary/native cached GPU traces match exactly
for all six fixture cases in `artifacts/ragdoll-tree-sorted[-native]-reference`.
Collision-free precision and stability pass. First-step maximum position
error is 5.8 micrometers, down from 97 micrometers. Every visual target passes
through frame 89; frame 90 first fails at 5.54 degrees maximum / 1.39 degrees
RMS orientation error. Frame 143 position error is 0.109 m RMS / 0.274 m maximum.
This does not establish full visual parity: frame 600 still has 0.973 m RMS /
2.198 m maximum position error. At frame 90, CPU reduces a pelvis/spine capsule
manifold to one point while GPU retains two; earlier state differences and
later contact ordering remain under diagnosis.

Identical-state replay from CPU frame 89 produces the same one-point capsule
manifold on both engines, so that independent-run contact-count difference
comes from earlier state drift. Capsule contacts now retain Box3D's clipped
endpoint feature IDs and warm-start only exact ID matches, replacing synthetic
quadrant IDs and the proximity fallback for this shape pair. An independent C
fixture and `capsule_feature_ids_survive_single_and_clipped_manifold_transitions`
cover the two-point → one-point → two-point transition. All ten capsule tests
pass. The six ordinary/native traces in
`artifacts/ragdoll-capsule-features[-native]-reference` match exactly; this fix
still first fails the visual gate at frame 90, with frame-143 position error
0.120 m RMS / 0.274 m maximum and frame-600 error 0.816 m RMS / 2.181 m maximum.

Convex friction now solves rolling resistance before central twist, as in
Box3D's SIMD solver, and includes its epsilon in the rolling clamp denominator.
Mesh and overflow contacts retain twist-before-rolling and the scalar clamp.
Contact preparation caches the solver family in `prepared_softness.w`; the
hot solve load deliberately omits persistent shape IDs. The independent
two-step spinning/sliding capsule fixture
`capsule_rolling_and_twist_friction_match_cpu` passes at 1e-4 velocity tolerance,
and all 11 capsule-filtered tests pass. Ordinary/native cached traces match
exactly for all six cases in
`artifacts/ragdoll-friction-order[-native]-reference`. Stability and
collision-free checks pass, but the visual gate still fails first at frame 90
(5.529 degrees maximum / 1.386 degrees RMS). Frame 143 position error is
0.120 m RMS / 0.276 m maximum; frame 600 is 0.807 m RMS / 2.167 m maximum.
This corrects a solver difference without resolving the earliest remaining
ragdoll drift. All 24 contact colors match CPU through frame 3. Cold replay
from identical CPU frame-2 state has 6.1 micrometers maximum position error
on its first step, growing to 0.231 mm on its second; accumulated solver state
and phase-level differences remain under investigation.

Warm starting follows CPU's normal → tangent → twist → rolling order.
An earlier capsule-only change retained the two tangent coordinates when a
manifold refreshed. That interpretation was incorrect and has been removed:
CPU stores the friction impulse as a world vector between steps, then projects
it onto the new tangent basis during preparation (`contact_solver.c`). Historical
`artifacts/ragdoll-friction-coordinates[-native]-reference` traces therefore do
not validate correct friction transport, even though their stability and
collision-free checks passed.

A phase trace isolates the first larger error jump to frame 3, substep 2,
right-thigh/spine contact, biased solve. The independent CPU computes separation
+2.98023224e-8 m and takes the speculative branch; GPU rounds to zero and takes
the soft branch (mass scale 0.9422793, impulse scale 0.057720676). Angular velocity
error grows from roughly 0.0002 to 0.095 rad/s in that operation. Temporary GPU
tracing reproduces the production state trace exactly and has been removed;
CPU tracing used a separate instrumented copy of the native contact solver.
Artifacts are `ragdoll-impact-goal/phase-*` and `point-*`. No branch tolerance
was introduced. Capsule narrow-phase separations are now retained through COM
conversion, and normal construction uses rounded reciprocal multiplication.
These precision corrections do not yet remove the frame-3 branch mismatch.

A separate independent mesh-contact fixture exposed an extra centre sample:
GPU generated three points for a capsule over one triangle, versus CPU's two.
Capsule/triangle contacts now clip the segment to the triangle face and use
segment/triangle witnesses and edge axes for edge and deep-overlap cases.
This replaces endpoint/centre sampling. Contact preparation reads the actual
persistent shape IDs to select scalar mesh versus SIMD convex friction; the
hot contact load has no valid shape IDs. All 12 capsule-filtered tests pass
on ordinary and native cached GPU builds,
including CPU velocity comparisons at 1e-4 for an interior face contact and a
long capsule with both endpoints outside the triangle. All six ordinary/native
cached GPU traces match exactly in
`artifacts/ragdoll-capsule-mesh-clipping[-native]-reference` and pass stability and
collision-free gates, but first fails visual agreement at frame 90 (5.5345°
maximum / 1.3923° RMS orientation). Frame 143 is 0.1124 m RMS / 0.2748 m maximum
position error; frame 600 is 1.5166 m RMS / 3.5293 m maximum. Correct local
contact behavior has not yet produced full-run trajectory agreement.

Capsule anchors now follow CPU body-origin transforms and quaternion-derived
matrix rotation before conversion to COM lever arms. All 12 capsule tests pass
with this arithmetic. Spherical joints now compose prepared frames with delta
rotations without extra normalization, rotate prepared lever arms, group relative
displacements before adding the initial centre difference, and combine warm-start
torques before applying inverse inertia. All 21 joint-filtered tests pass.
The ordinary independent run in
`artifacts/ragdoll-spherical-arithmetic-reference` still first fails at frame 90
(5.5369° maximum / 1.3986° RMS orientation); frame 143 is 0.1083 m RMS /
0.2804 m maximum position error and frame 600 is 1.4958 m RMS / 3.5068 m maximum.
Collision-free and stability gates pass. These changes improve correspondence
with CPU operations but do not resolve the trajectory divergence; native cached
validation of this latest arithmetic is pending.

Frame-1 phase tracing then isolated a larger error in the right knee's second
substep warm start: angular velocity changes by approximately 0.033 rad/s relative
to CPU despite closely matched incoming state. CPU retains the perpendicular
axes updated by the preceding revolute solve; GPU had recomputed start-of-frame
axes for every warm start. Revolute joints now cache those updated axes between
substeps and reinitialize them at each world step. The existing joint layout
reuses the weld vectors (unused by revolute joints) and a former padding word
for the preparation epoch, without increasing GPU storage stride.

The tilted, spinning anisotropic-box regression
`revolute_substep_warm_start_matches_cpu` compares three independent CPU
checkpoints at 1e-4. It passes with the fix and fails with the previous axis
behavior (first-step quaternion z 0.040950254 versus CPU 0.041259259). All 21
existing joint-filtered tests also pass. The ordinary full run in
`artifacts/ragdoll-revolute-axis-cache-reference` reduces first-frame maximum
orientation error from 0.0017607° to 0.0000473°, but still first fails the visual
gate at frame 90 (5.3899° maximum / 1.1151° RMS orientation). Frame 143 is
0.0996 m RMS / 0.2232 m maximum position error; frame 600 is 0.7398 m RMS /
1.6132 m maximum. Frame 3 still contains a contact-related error jump.
Temporary phase instrumentation reproduced the production frame-1 state
exactly and has been removed. Artifacts are `ragdoll-impact-goal/firstphase-*`
and `warmphase-*`. The new hinge oracle test also passes on the native cached
build. All six GPU traces match exactly between ordinary and native cached
builds (`artifacts/ragdoll-revolute-axis-cache-native-reference`), including
the same full-run visual failure and passing stability/collision-free gates.

The remaining frame-3 contact jump persists after the hinge fix: for thigh/spine
body pair 23/16, CPU computes +2.98e-8 m separation and GPU computes zero on the
second biased substep. The resulting incremental normal impulses differ by
approximately 0.214 N·s. Pair 9/2 first takes different branches one substep later.
The `ragdoll-impact-goal/posthinge-point-*` diagnostic reproduces the production
state trace exactly; no separation epsilon or branch override was added.

Revolute frame composition, prepared-lever rotation, combined warm-start torque,
and relative point-constraint arithmetic now also follow CPU operation order.
The three revolute-filtered tests pass, including the independent CPU fixture.
The dispatch-count test now checks the shared-color wave architecture already
used by production: 13 complete contact waves / 12 joint waves for the small
workgroup path, and 24 separate color dispatches per wave in general mode.
The independent ordinary run in
`artifacts/ragdoll-revolute-arithmetic-reference` passes stability and improves
collision-free maximum position error to 2.96e-6 m. It still first fails visual
agreement at frame 90 (5.3919° maximum / 1.1127° RMS orientation); frame 143 is
0.0993 m RMS / 0.2289 m maximum position error, and frame 600 is 0.7973 m RMS /
1.5852 m maximum. All six ordinary/native cached GPU traces match exactly
(`artifacts/ragdoll-revolute-arithmetic-native-reference`).

Contact API snapshots around frames 87–90 reproduce the production state trace
exactly (`ragdoll-impact-goal/impact-contacts-*`). They show no missing manifold
pairs or differing point counts at frames 88–89; differing counts appear by
frame 90. An identical-state diagnostic replay from CPU frame 87 isolates a
separate CCD problem: after one step, position error is 1.203 mm RMS / 7.624 mm
maximum despite close velocities. Disabling continuous collision detection in
both diagnostic runs reduces that first-step error to 1.057e-6 m RMS /
2.935e-6 m maximum. Later cold replay steps still diverge, so this does not prove
CCD is the only remaining cause. Artifacts are `impact-replay-*` and
`impact-replay-noccd-*`; state injection and disabling CCD are confined to these
ignored diagnostic programs, never production or independent acceptance runs.
The host mesh CCD previously used bounded conservative advancement, whereas
Box3D uses a cached separating-axis root search. A native TOI trace for the worst
replay body finds fraction 0.197007388, accepting distance 0.07111994 within the
native tolerance around target 0.07; old GPU-side CCD continued to fraction
0.24536002. Mesh CCD now uses a Rust separating-axis root search with cached
GJK witnesses, vertex/edge/face separation functions, alternating bisection and
false position, native iteration limits, and the existing centroid fallback.
Convex-only CCD retains its separately validated bounded advancement path.

The isolated GPU impact now returns fraction 0.19701311; maximum position errors
across the first three replay steps are 3.45e-6 m, 0.000152 m, and 0.000462 m.
`mesh_ccd_rotating_capsule_matches_native_root_tolerance` checks the captured
native sweep at 2e-5 fraction tolerance, including a translated copy. All 16
query tests pass on ordinary and native cached builds, including glancing mesh motion, centroid fallback,
and degenerate witness normals. All six ordinary/native cached independent
traces match exactly in `artifacts/ragdoll-mesh-root-toi[-native]-reference`.
Stability and collision-free gates pass; visual agreement still first fails at
frame 90 (1.0277° RMS orientation, with 4.9798° maximum). Frame 88 maximum
position error is 0.951 mm, frame 143 is 0.07455 m RMS / 0.24105 m maximum, and
frame 600 is 0.38683 m RMS / 0.98604 m maximum. This fixes the identified CCD
overshoot, but does not establish full-run agreement.

Post-CCD replay phase tracing (`ragdoll-impact-goal/replayphase-*`) finds a
contact warm-start mismatch before the spine joints amplify it. The CPU trace
was rebuilt with full float output precision before comparison; both diagnostic
state traces reproduce their uninstrumented counterparts exactly. Restoring
world-vector friction reprojection for refreshed capsules reduces the three-step
impact replay's maximum position errors to 3.45e-6, 1.19e-5, and 2.46e-5 m;
second-step maximum velocity-component error drops from 0.3106 to 0.00636.
The independent `artifacts/ragdoll-friction-reprojection-reference` run still
fails first at frame 90 (5.0223° maximum / 1.2574° RMS orientation). Frame 143
position error is 0.08321 m RMS / 0.25313 m maximum; frame 600 is 0.40722 m RMS /
1.01155 m maximum. This fixes the isolated transport error but does not improve
every checkpoint in the independent trajectory. The subsequent ordinary/native
comparison below also validates this friction correction on both paths.

Extending the rotating-capsule CPU regression to 12 steps exposed a separate
reference-segment error at step 7, when the manifold expands from one point to
two. The GPU always put the later capsule first, including static/dynamic
pairs. CPU broad-phase queries put the static or kinematic shape first in these
pairs. Reversing the reference segment changed clipping: the observed CPU
normal had x approximately zero, while the GPU normal had x=0.01222, and the
two points appeared in the opposite order. The GPU now distinguishes anchored
pairs from initially moving dynamic pairs. The 12-step velocity regression
passes at the unchanged 1e-4 tolerance; before the fix its step-7 x velocity
was -0.0091197 versus CPU -0.00099394. Diagnostic contact traces are in
`artifacts/ragdoll-impact-goal/capsule-friction-{cpu,gpu}.log` (pre-fix GPU).
The corrected C-ABI fixture has maximum velocity-component error 6.0e-7 across
all 12 steps (`capsule-friction-order-gpu.{txt,log}`). All 14 capsule-filtered
tests pass on both ordinary and native cached GPU builds. All six full-gate
GPU traces in `artifacts/ragdoll-capsule-reference-order[-native]` match each
other byte-for-byte and match the previous friction-reprojection traces.
Stability and collision-free checks pass; visual agreement still fails at
frame 90 with the errors reported above. This anchored-capsule correction
therefore fixes the isolated regression but does not reduce the remaining
Falling Ragdolls trajectory error.

The subsequent `ragdoll-impact-goal/separation-*` trace captures the prepared
normal/anchors/base, rotated anchors, delta positions, and delta rotations for
the frame-3 thigh/spine contacts. CPU and GPU diagnostic state dumps exactly
match their respective uninstrumented traces. The mismatch is already present
in the prepared geometry, before substep displacement or rotation. At the
second biased solve, CPU separation +2.98e-8 versus GPU zero changes the
incremental impulse by about 0.214 N·s; no branch epsilon was introduced.

An isolated replay of the CPU frame-2 capsule inputs identifies a lossy geometry
round trip: the GPU reconstructed endpoints from their midpoint and half-axis.
Reconstructing those endpoints on CPU reproduces the GPU's normal and anchor
differences, including normal y=0.984409690 versus original CPU 0.984409750.
The same discrepancy occurs at the original world location and after moving
the pair near the origin, ruling out world translation as its cause in this
fixture. Original capsule endpoints now occupy two entries in the shared
geometry point buffer. Capsule contact generation, host query geometry,
initial bounds, and GPU capsule queries read them directly; midpoint/axis
remain available for the existing proxy and rendering paths. Geometry setters
also replace these endpoints. The regression
`capsule_original_endpoints_match_cpu_contact_normal` checks both endpoint
preservation and the captured CPU normal. Evidence is in
`ragdoll-impact-goal/capsule-input-*`. All 15 capsule-filtered tests pass on
ordinary and native cached GPU builds; the geometry-replacement regression
also passes. Offset Kinematic and Prismatic pass their unchanged gates.

The independent `artifacts/ragdoll-original-capsule-endpoints[-native]` runs
produce identical GPU traces for all six cases and pass stability/collision-free
checks. The frame-3 maximum position jump falls from 2.392e-4 to 4.8e-7 m;
maximum velocity-component error falls from 0.03945 to 2.09e-5. Frame-60 maximum
position error falls from 9.686e-4 to 2.622e-5 m. The later impact divergence
remains: visual agreement first fails at frame 90 (1.2520° RMS / 5.0223° maximum
orientation); frame 143 position error is 0.08431 m RMS / 0.25314 m maximum,
and frame 600 is 0.40257 m RMS / 1.01204 m maximum. The early improvement does
not satisfy the 600-frame acceptance targets.

Mesh/capsule generation now also uses the original capsule endpoints and
transforms the triangle into the capsule body frame, matching native
`mesh_contact.c`. The regression
`capsule_mesh_contact_uses_capsule_local_frame` checks a captured CPU contact
normal and one-step velocities for an asymmetric, rotated capsule. All 16
capsule-filtered tests pass in ordinary and native cached builds. The
independent `artifacts/ragdoll-capsule-mesh-local[-native]` runs have byte-identical
GPU traces for all six cases and pass stability/collision-free checks. Visual
agreement still first fails at frame 90 (1.2520° RMS / 5.0220° maximum
orientation). Frame 143 position error is 0.08432 m RMS / 0.25314 m maximum;
frame 600 is 0.40258 m RMS / 1.02910 m maximum. This coordinate-frame correction
alone does not resolve the impact divergence.

The frame-89 graph trace identifies four different contact colors: CPU handles
stopped/started contacts in persistent contact-ID order, whereas GPU releases
all stopped colors before assigning new ones. A temporary diagnostic override
matched those four colors; the trace confirmed it took effect, but all 600
frames remained byte-identical because those contacts applied zero normal
impulse. The override was removed. This lifecycle difference is real but is
not the cause of the current frame-90 jump.

A CPU geometry replay establishes the immediate cause of that jump. Feeding
independent frame-89 CPU and GPU poses to the same native `b3CollideCapsules`
code gives different manifold sizes for pelvis/spine pairs in rows 42/44,
70/72, and 98/100. With CPU poses, squared normalized-axis cross products are
0.0025014095, 0.00249971589, and 0.0025014095; with GPU poses they are
0.00249867048, 0.00250161253, and 0.00249860762. These straddle the native
parallelism threshold 0.00250000018 and reproduce the observed one-point versus
two-point choices. Thus incoming pose history, rather than a different
collision predicate on identical inputs, explains these three branch changes.
The next target is numerical error before this impact; changing the threshold
would not repair that error. Diagnostic sources and traces are under
`artifacts/ragdoll-impact-goal/capsule-threshold-*`, `mesh-local-graph-*`,
`impact-lifecycle-*`, and `color-causal-*`.

Mesh/capsule contact anchors now also follow native's body-relative
construction: rotate the local witness, add the difference between body
origins for the mesh anchor, then subtract each rotated local center. A
transient marker prevents finalization from shifting these COM anchors again.
The translated variant of `capsule_mesh_contact_uses_capsule_local_frame`
checks both anchors against CPU within 1e-7 m at offsets 0 and 1024 m, with
separate CPU velocity references for each location. The old implementation
fails this test: its translated first anchor's Z coordinate is -0.004272461
instead of CPU's -0.004347032. The corrected implementation passes, as do
all 16 capsule-filtered tests in ordinary and native cached builds.

The independent `artifacts/ragdoll-mesh-anchor[-native]` comparisons have
byte-identical GPU traces for all six cases and pass stability/collision-free
checks, but still first fail visual agreement at frame 90 (1.2520° RMS / 5.0221°
maximum orientation). Frame 143 position error is 0.08431 m RMS / 0.25310 m
maximum; frame 600 is 0.40239 m RMS / 1.02525 m maximum. The identical-state
frame-87 replay is mixed rather than materially improved: step-1 maximum
position error is 2.90e-6 m and maximum velocity-component error is 0.002291;
step-3 errors are 2.48e-5 m and 0.01170. Preserving these anchors fixes a
verified precision defect, but does not resolve the earlier solver/history
errors that trigger the frame-90 manifold changes.

The identical-state frame-87 phase trace (`ragdoll-impact-goal/impactphase-*`)
records matching operation sequences for pelvis/spine body IDs 44 and 46.
Initial warm-start states match after parsing both logs back to float32;
errors grow during subsequent joint solves, including the fourth substep's
biased solve. Temporary tracing was removed after capture.

GPU twist, swing, and wheel steering angles now use Box3D's deterministic
`b3Atan2` minimax polynomial instead of WGSL's built-in `atan2`. This is a
behavioral difference, not just platform rounding: for inputs (1,1), native
returns 0.785425782 radians. The polynomial retains native scalar evaluation
order and uses the rounded division helper. The native precision regression
checks 14 independent CPU bit patterns across axes, quadrants, and (0,0).
All four precision tests and all 21 joint-filtered tests pass on ordinary
and native cached GPU paths. The three revolute tests, wheel lever regression,
and unchanged Offset Kinematic/Prismatic precision gates also pass on the
ordinary path.

In the frame-87 replay, the spine at sample row 44 has maximum angular-velocity
component errors of 4.577e-5, 2.966e-4, and 1.889e-4 rad/s over its three steps,
down from 3.1281e-4, 2.3823e-3, and 6.008e-4 before the angle correction.
Across all bodies, maximum velocity-component error at replay step 2 falls
from 0.006548 to 0.003557; step 3 falls from 0.01170 to 0.009068. This isolation
result is not independent-run acceptance. The full
`artifacts/ragdoll-native-atan[-native]` comparisons produce byte-identical
GPU traces for all six cases and pass stability/collision-free checks, but
still first fail at frame 90 (1.2519° RMS / 5.0225° maximum
orientation). Frame 143 position error is 0.08470 m RMS / 0.25335 m maximum;
frame 600 is 0.40065 m RMS / 0.97313 m maximum. The remaining accumulated
history error still changes the pelvis/spine manifold branches.

The initial-frame trace (`ragdoll-impact-goal/seedphase-*`) now follows the
first two humans' spine-01 bodies, engine IDs 2 and 16 (sample rows 1 and 15).
All 120 recorded operation keys match per body. Warm-start states agree;
small differences first appear in the first biased joint solve, before these
bodies' contacts apply impulses. The explicit `body-map` trace avoids confusing
sample indices with engine IDs; each ground precedes its pair of ragdolls.

Initial mass-data comparison identifies another setup difference. Capsule mass
now follows native volume-then-density multiplication and quaternion/matrix
rotation of the central inertia, instead of the algebraically simplified
axisymmetric tensor. These differ in float32, including small off-diagonal
terms for axis-aligned capsules. `capsule_mass_matches_native_float32_rotation`
checks four independent CPU reference sets (pelvis, oblique thigh, antiparallel
axis, sphere limit) at non-unit density. Body mass aggregation also rounds
inverse mass once before scaling the weighted center. The first-shape fast
path preserves that calculation and its parallel-axis correction rather than
cancelling the mass factors. The regression
`capsule_body_center_matches_native_inverse_mass_scaling` checks the native
ragdoll thigh's center exactly. Both paths pass all 18 capsule-filtered tests.

The intermediate inertia-only run `artifacts/ragdoll-capsule-mass` first failed
at frame 106, but this is not the final result: correcting the first-shape
center changes subsequent branch choices again. Current independent runs in
`artifacts/ragdoll-capsule-native-mass[-native]` have byte-identical GPU traces
for all six cases, pass stability/collision-free checks, and first fail visual
agreement at frame 90 (1.0221° RMS / 5.0232° maximum orientation). Frame 143
position error is 0.07262 m RMS / 0.25318 m maximum; frame 600 is 0.36662 m RMS /
1.06262 m maximum. Inertia (24 comparisons), density, Offset Kinematic, and
Prismatic reference checks pass. Rain, ground dragging, performance, and
current recordings remain pending.

The final initial-state diagnostic matches all 1,456 sampled values across 112
bodies: mass, local center, six stored inertia components, and world center.
Expanding it to all nine matrix components reveals 80 remaining discrepancies
in the other three inertia components, up to 1.863e-9. Native float32 matrix
rotation is not perfectly symmetric, whereas the GPU's six-component storage
forces symmetry. Evidence is in `ragdoll-impact-goal/mass-initial-final-*` and
`mass-full-matrix-*`. This is a concrete remaining representation difference
for follow-up, not yet a demonstrated cause of the remaining visual failure.

Offset Kinematic and Prismatic pass their existing 1e-5/300-step and
1e-4/120-step gates with these changes. Rain, ground dragging, performance
measurements, and comparison recordings remain pending. No visual-agreement
completion is claimed.

The Offset Kinematic regression checks explicit zero-mass center overrides on
static and kinematic bodies. `b3Body_SetMassData` now accepts those overrides,
matching upstream; previously the GPU ignored them and rotated the offset box
about its body origin instead of its center. From `experiments/gpu-physics`, run
`./scripts/check-offset-kinematic-reference.sh` for the independent CPU/GPU
comparison. It checks 903 states over 300 steps per case, including a moving
kinematic body's center change after 150 GPU steps, with absolute tolerance
`1e-5` on mass, center, pose and velocity. This qualifies the sample's zero-mass
center override, not all mass-data semantics.

Pre-commit native viewer captures for Offset Kinematic, Falling Ragdolls and
Rain are registered in the local comparison grid's corresponding rows. CPU
panes are in `000-box3d-cpu`; the latest GPU panes are in
`2026-09-26-ragdoll-checkpoint` (the previous captures remain in
`2026-09-25-physics-fixes`). The checkpoint captures cover 300 steps for Offset
Kinematic and Falling Ragdolls and 60 steps for Rain. These are wall-time captures
with software OpenGL rendering, not performance measurements. Rain is a shortened
capture of the initial drops, not a completed benchmark run. The Rust demo
recorder does not host these native scenes. Capture reports and binary identity
are in `experiments/gpu-physics/artifacts/ragdoll-checkpoint-capture/`. The same
checkpoint also records all 19 Rust demo scenes and their real Box3D CPU oracle
clips at 120 frames, with the CPU column pinned first. These short recordings
are pre-commit comparisons, not the outstanding full qualification.

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
