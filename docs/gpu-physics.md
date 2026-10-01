# GPU physics experiment

The [experiment](../experiments/gpu-physics/README.md) implements rigid-body physics
in Rust/WGSL and exposes part of Box3D's native C API. It does not replace the
Box3D WASM package or modify the upstream engine. Native samples can run either
engine or show two independent worlds side by side.

## Support status

The [GPU solver qualification](gpu-solver-qualification.md) tracks correctness,
stability and same-device repeatability with efficient GPU ordering. It separates
physical behavior from exact CPU trajectory agreement and lists unverified areas.

The current desktop candidate enables paired normal-contact solving only with
`GPU_PHYSICS_LIVE_CONTACT_ORDER=0`. It passes the original contact-island,
support, friction and restitution checks in five fresh processes per backend;
default and CPU-compatible contact solving remain unchanged. Rain, the remaining
capability checks, performance and recordings still prevent qualification or a
default-mode change. See the qualification report for frozen builds and scope.

Frozen mesh-contact regression inputs are kept in
`experiments/gpu-physics/c_abi/fixtures/frozen-mesh`. The
`scripts/check-frozen-mesh-reference.py` runner compares prebuilt CPU and GPU
probes at identical poses and records hashes and exits; see the qualification
report for its command and the distinction from full-scene physics checks.

| Area | Status |
|---|---|
| Linux NVIDIA GeForce RTX 4070 Laptop GPU | Native CPU/GPU/combined builds and normal/strict retained-history drag checks pass |
| Linux AMD Radeon 780M | Earlier native builds, picking and normal/strict retained-history drag checks pass; the current large-world scaling changes are qualified on NVIDIA. The AMD convex-sweep regression remains. |
| macOS / Windows | Build paths and CI matrix added; CI results and physical-GPU runtime validation are still required |
| Intel / Apple hardware | Adapter selection permits capable devices; hardware correctness and performance are unverified |
| Experimental browser target | Does not currently compile; native transport, ABI layout and world-storage assumptions need work |
| Box3D WASM package | Separate implementation; unchanged by the experiment |
| Native API parity | Incomplete; linkable symbols include placeholders and CPU-only comparison passthroughs |
| Falling Ragdolls precision | Independent 600-step, 112-body runs pass every-frame targets on ordinary and native cached NVIDIA paths: worst position RMS/max 2.39/14.92 mm; orientation RMS/max 0.764/4.342 degrees. Collision-free, joint stability, Offset, Prismatic and strict dragging checks pass. Synchronized recordings are complete. Rain passes 600-step health/recycling checks but fails CPU-relative joint-error screening; see the validation details below. The warm ragdoll fixture costs 32.2% more wall time than its recorded earlier baseline. |
| Performance | New bounded desktop scheduling: opt-in automatic matches global at 50k cubes and reduces independent-group mean step time 57.18% against global, with p95 limits passing on both fixtures. Laptop validation of this policy is pending. Earlier RTX 4070 Laptop solver-writeback follow-up versus `0bbf5f63`: median completed-step time falls 7.7% at 2k cubes and 1.5% at 5k, below the 20% target. The 100k and 200k cases improve 10.7% and 7.9%, with every paired large-scene trial faster. In the matched sweep, the first sampled GPU/CPU physics crossover moves from 20k to 15k cubes. Small-scene timings remain variable; this is not a universal speedup or crossover threshold. See the experiment README for dated charts, renderer measurements, raw evidence and earlier benchmark history. |

The solver-writeback change stores only contact impulses during solving; it
preserves geometry, prepared coefficients, arithmetic and scheduling. A controlled
dispatch-versus-workgroup probe supports retaining the existing occupancy-based
schedule. Narrow body writes and shader specialization did not yield reliable
additional completed-step gains and remain rejected experiments. The native-cache
suite passes 276 tests, plus nine cache-disabled checks; the timing probe passes
separately. Box3D and the WASM package are unchanged.

GPU contact-query snapshots allocate new handles in stable public shape-pair
and compound-child order. Existing handles retain their lifetimes and query
enumeration positions. This removes randomized hash-map iteration from public
contact ordering; it does not change GPU constraint ordering. Handles remain
opaque identities, not numbers promised to match across worlds or runs. Contact
end events also use stable logical pair/child ordering. Host-deferred and
GPU-derived ends for the same transition are deduplicated within each step;
refiltering can still produce a new begin after collisions are restored.
GPU slots keep their root-generation counters when temporarily used for mesh
patch staging or child manifolds. This prevents a retired handle from becoming
valid again when the original shape pair returns after unread intermediate
steps. The scratch-reuse regression exercises this through real mesh collision
processing; generation counters are not reset or inherited from a parent patch.
The current qualification candidate canonicalizes mesh children on the GPU after
graph cleanup and retirement, before constraint preparation. Roots and their
counters remain fixed; children follow root/chain order in the lowest non-root
slots. This removes child-placement variation from the next free-slot allocation.
It adds temporary snapshot and scan storage for mesh scenes; convex-only scenes
skip it. Lifecycle, full repeat and overhead qualification are tracked in
[gpu-solver-qualification.md](gpu-solver-qualification.md).
The current GPU-native qualification candidate also solves normal contact points
in pairs, using opposite points for four-point manifolds. Coupled unilateral
impulses retain the existing softness and material parameters; ill-conditioned
pairs fall back to scalar updates. The original contact-island spin/drift checks
pass on both backends, including five fresh captured runs of the initial
candidate. The new solve is selected only by explicit
`GPU_PHYSICS_LIVE_CONTACT_ORDER=0`; default and CPU-compatible ordering keep the
scalar point solve. Its mode bit is included in captured parameters. Broader
physical and performance qualification remains open.
Contact-retirement commands use storage separate from sticky contact-failure
reasons. Removing a shape or disabling a body pair must neither manufacture a
failure reason nor clear an earlier one; retirement still selects the same roots.
Explicit completion also refreshes status after contact mutations within the same
physics step, so cached clean metrics cannot hide a newly recorded GPU failure.
Clearing capacity diagnostics first observes unread failures and preserves terminal
invalidity; clearing counters cannot make a failed simulation usable again.
Scene-preparation errors (rejected capacity, simulator construction or scene
packing) likewise invalidate the world and any retained simulator. Clearing
the diagnostic and retrying preparation does not resume a failed simulation.
Simulator growth preserves failure reasons with their counters, including failures
that have not yet reached the host status readback.
Overflow of the large-static broadphase list reports insertion loss instead of
silently dropping collision candidates.

The earlier connected-scene scheduling follow-up compares against `c46b81e2` on the same
RTX 4070 Laptop. The repeated physics sweep measures about 2%, 15% and 20% lower
median completed-step time at 1k, 2k and 5k falling cubes; the 25% target was not
reached, and the 1k result is noisy. Dense small worlds now choose batched graph
memoization using an asynchronous contact-density hint; sparse worlds keep the
shared path. The initial 100k slowdown was investigated with balanced three-way
repeats and did not consistently reproduce; the report retains both rounds.
The [bounded desktop scheduling qualification](../experiments/gpu-physics/benchmarks/scheduling-2026-09-30/README.md)
adds an opt-in automatic choice using the existing dynamic-contact density hint.
At 50k cubes it matches explicit global scheduling; for 2,048 independent two-box
groups it reduces mean completed-step time 57.18% against global. Explicit modes
and existing defaults remain available. This result covers those two fixtures on
the i9-9900K / RTX 4070 SUPER; laptop and broader solver qualification remain open.

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

The [production-readiness roadmap](goals/gpu-production-readiness.md) is now the
authoritative ordered queue for `feat/gpu` and GPU-focused "What's next?" requests.
Its acceptance-backed checkboxes and current recovery block support long goals
across compaction. The first release scope is Linux/NVIDIA native; required Rain
correctness and complete final-build qualification remain open. The summary
below describes API/platform sequencing; it does not override the roadmap or
activate implementation on its own.

1. **Completed: ground-drag agreement.** Both tested GPUs pass the independent
   ten-drag sequence at unchanged normal and strict thresholds. Solver/contact
   regressions pass; the separate AMD sweep and secondary-queue failures below
   remain. These results do not establish general GPU parity.
2. **Next: non-recording native APIs.** Shape replacement, joint separation and reaction
   queries and substep force/torque integration are implemented with the precision
   limits below. Warm-start and speculative-contact controls are implemented; finish their readiness acceptance and then
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

Warm-start controls default to enabled, matching Box3D. Disabling them clears
active contact and joint impulses once during step preparation, after island
wake propagation. Substeps still reuse impulses accumulated within the current
step; sleeping islands retain their caches until they wake. Re-enabling uses the
latest solved cache. The combined viewer forwards the checkbox to both worlds
and reads the GPU setting. Destroying another comparison world preserves the
surviving world's mappings and controls.

Run `scripts/check-warm-start.sh` from `experiments/gpu-physics` for public API,
world independence/recreation, and tiny contact/spherical-joint CPU comparisons
at one and four substeps. Set `GPU_PHYSICS_SAMPLES_NATIVE_CACHE=1` for the cached
backend. The focused Rust test `warm_start_control_prepares_contact_chains_and_joint_caches`
checks all contact impulse channels, manifold children, all eight joint types,
repeated transitions and sleeping caches. The ordinary and cached paths pass these checks. Five paired ordinary-backend
runs measured mean default-enabled changes from -1.9% to +1.6% across the four
fixtures, without demonstrating a speedup. The
[verification receipt](../experiments/gpu-physics/benchmarks/2026-09-29-warm-start-verification.json)
and [timing receipt](../experiments/gpu-physics/benchmarks/2026-09-29-warm-start-timings.json)
contain results; [the warm-start goal](goals/gpu-warm-start.md) records recovery context.
A distance-joint prototype differs from CPU on its first step before toggling;
that minimal discrepancy is recorded separately and was not investigated here.

## Speculative-contact world control

`b3World_EnableSpeculative(world, enable)` now controls the GPU experimental
hull–mesh feature; worlds default to enabled. Rust callers use
`b3_world_enable_speculative` and can read `b3_world_is_speculative_enabled`.
Either shape endpoint can disable the feature. Convex–convex, sphere–mesh and
capsule–mesh contacts retain their existing behavior. Combined comparison
adapters forward the setter to both correctly mapped worlds. The upstream viewer
currently has no speculative checkbox; C/Rust calls exercise this control.

Off/on changes force a hull–mesh manifold refresh on the next positive step,
including after zero-duration steps. Independent worlds and recreated worlds
keep separate policy state. An actual preceding solid CCD hit retains the existing
6.25 mm TOI landing support shell even with speculation disabled; fast tangent
motion without a hit does not. Removing that support shell tunnels in both the
GPU reproducer and the independent CPU shape-off control. The pinned CPU world
setter only stores its flag, so CPU shape controls provide the independent
collision reference rather than a claim of CPU world-toggle parity.

Captured semantic state advances to v23 for world policy and pending refresh.
Older datasets remain readable. The
[PR01 report](../experiments/gpu-physics/benchmarks/production-readiness/pr01-speculative-2026-10-01/README.md)
retains rejected variants, exact build/source receipts, five-process control
repeats on both backends and relevant regressions. This focused evidence does
not complete the production roadmap's final physics or repeatability matrix.

## Native API availability

The initial Linux/NVIDIA GPU release excludes native recording/player APIs and
CPU worker/static-tree controls. The 39 operations listed in
[`native_unavailable.inc`](../experiments/gpu-physics/c_abi/native_unavailable.inc)
report `ENOTSUP` and a sticky thread-local operation name through
[`native_api_status.h`](../experiments/gpu-physics/c_abi/native_api_status.h).
Recording creation returns NULL and saving returns false. Combined adapters use
the same unavailable GPU implementations. This prevents successful CPU-only
recordings or worker settings from masquerading as GPU support.

Read `gpu_b3_native_api_last_error()` after an API error; its static name
persists until another error or `gpu_b3_native_api_clear_error()`.
Check errno immediately, before unrelated library calls. Clear affects only this
thread's API diagnostic and errno, never sticky world/capacity failure state.
The exclusion lookup is an exact named list; an unknown name returning false
is not a support claim. Other failure return values require consulting the
availability/error contract rather than interpreting zero as valid data.

The [PR02 partial report](../experiments/gpu-physics/benchmarks/production-readiness/pr02-api-2026-10-01/README.md)
publishes all 415 stateful API entries, linked-source audits and host-only checks.
Seven complete trials pass; one harness failure and all build failures remain.
A separate closed-stderr check preserves immediate ENOTSUP after an output error.
These checks do not initialize a GPU or qualify supported physics. The later diagnostic milestone adds focused GPU/API verification below. Broader
viewer controls and full API behavior remain open under the production roadmap.

The [current supported-contract checks](../experiments/gpu-physics/benchmarks/production-readiness/pr02-supported-2026-10-01/README.md)
pass 16 linked C processes and 20 exact Rust checks on ordinary/native builds
whose compiled inputs match the current sources. These cover named warm-start,
body/world controls, external forces, shape replacement, mass and joint-query
contracts at unchanged tolerances; combined fixtures use independent CPU state.
They do not qualify every inventoried operation or replace the remaining
population/viewer and final-build physical/runtime/performance gates. The
preserved warm-start clocks are incidental correctness output, not benchmarks.

The [population campaign](../experiments/gpu-physics/benchmarks/production-readiness/pr02-corner-populations-2026-10-01/README.md)
passes the CPU reference and standalone ordinary/native sensor, compound and
three-plane mesh diagnostic checks. It then exposes a real **combined-adapter
compound defect**: private GPU child creation uses dual-world constructors,
adding CPU child colliders before the full CPU compound. That historical combined
comparison has incorrect topology; native combined was unlaunched at its stop
rule. The [new ownership candidate](../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-ownership-after-bounds-2026-10-01/README.md) on the independently qualified AABB prerequisite passes14 first processes
with original assertions. Its12 precommit recordings pass300-step health and all six comparisons are reviewed;
Village retains an existing shared-renderer capacity failure underPR09. The
qualified ownership fix preserves public mirroring and removes orphan CPU
colliders; these focused checks do not complete PR02 or visual qualification. Both earlier CPU
fixture-assumption failures remain retained. The separate host receiver verifies
held mouse/key delivery; actual viewer controls remain unqualified until a fixed
adapter/build is frozen and exercised.

### Native GPU diagnostic contract (focused checks pass; broader PR02 open)

The current implementation replaces zero diagnostic placeholders. The
[focused report](../experiments/gpu-physics/benchmarks/production-readiness/pr02-diagnostics-2026-10-01/README.md)
records passing C/Rust/regression/storage checks, four viewer builds and 40
CPU-first scene clips. Actual counter/tab interactions remain open after a
retained mouse-stimulus harness failure. The
[frozen protocol](../experiments/gpu-physics/benchmarks/production-readiness/pr02-diagnostics-2026-10-01/protocol.json)
records the process budget, exact selectors, settings and stop rule.

[`native_diagnostics.h`](../experiments/gpu-physics/c_abi/native_diagnostics.h)
publishes extended status, timestamp and allocation information. Status is
`0` for invalid/stale arguments, `1` for valid data, `2` for busy/reentrant world
access, and `3` for sticky physics failure. The compatible `b3World_*` diagnostic
wrappers publish `EINVAL`, `EBUSY` or `EIO` and a sticky operation name for those
errors. A successful call does not clear an earlier API error. Error clearing
never heals a failed world. Extension callers must inspect status themselves.

`b3World_GetCounters` explicitly waits for completion and synchronizes native
contact records. Body/shape/joint counts are live public topology; private
compound collider slots are not extra public shapes. `contactCount` counts
public contact IDs, excluding sensors; touching contacts contribute to the
manifold buckets, with the last bucket including eight or more manifolds.
Island counts use completed GPU body labels, exclude static/kinematic/disabled
bodies, and are unavailable before a step or after topology/body mutation.
An empty world has zero islands. Unsupported CPU worker/tree/allocator/SAT/TOI,
color and awake/recycled-contact diagnostics return `-1`, rather than a false
zero. `byteCount` is primary owned GPU physics buffer bytes when representable
as a signed 32-bit value, otherwise `-1`. The 64-bit extension remains available.
Routine viewer/sidebar/benchmark metadata uses cheap public topology and
separate scheduling metrics; it does not implicitly download contact records.

GPU profile fields are aggregate aliases, not CPU profiler parity. The extension
provides an availability mask and the timestamp's physics step. Reads poll
without waiting; an older timestamp is labelled as such and an absent timestamp
has step zero and mask zero. Unmeasured fields are NaN.

| Compatible profile field | Measured GPU interval (milliseconds) |
| --- | --- |
| `step` | Device start through sleep |
| `pairs` | Broadphase |
| `collide` | Narrowphase plus graph |
| `solve` | Preparation plus solve plus integration |
| `solverSetup` | Island preparation |
| `solveImpulses` | Entire constraint solve aggregate |
| `integratePositions` | Integration aggregate |

These aggregates overlap; adding rows double-counts time. Other CPU/substage
fields are unavailable. The GPU metrics tabs show measured phase names and
availability instead of feeding NaNs into CPU flame bars or reporting zero.
The explicit combined-viewer CPU stats helper continues to return mapped CPU
measurements, while shared public diagnostic functions return GPU data.

`b3World_GetMaxCapacity` reports occupancy peaks, separate from allocation:
public bodies and enabled public shape proxies are sampled at positive step
boundaries; dynamic counts include kinematic bodies. Disabled bodies still count
as bodies. Contact peaks count supported non-sensor roots during occupied-list
collection before CCD correction. They persist across unread steps, retirement
and buffer growth. This phase/population must not be confused with synchronized
public contact records or reservation hints. Zero-duration steps do not sample
new peaks. `GpuNativeAllocation` reports current reserved internal slot counts
and actual primary buffer sizes in 64 bits, excluding staging, command-cache
copies, pipelines, renderer and driver allocations. Memory output states that
scope. Bounds output snapshots public shape/body ownership and speculative
AABBs, merging compound children, and invokes visitors after unlocking.

The [compound bounds diagnosis](../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-bounds-2026-10-01/README.md)
retains zero public AABBs and rotated native/CPU differences on baseline inputs.
The [bounds repair](../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-aabb-validation-2026-10-01/README.md)
now passes twelve first API validation processes on exact ordinary/native
compiled inputs. Public shape/body and native owned bounds match independent
CPU compound geometry at the unchanged1e-5 limit through transforms,
disable/enable and parent regeneration. Imported native source local tree boxes
are retained before hull baking; GPU proxy bounds still come from colliders.
The initial compiler failure and its sequencing deviation remain preserved.
The [precommit scene review](../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-aabb-captures-2026-10-01/README.md)
passes all twelve CPU-first captures at300 completed steps each; all six scene
comparisons are reviewed. The repair is retained for the verified milestone.
These focused results do not qualify every compound API or final physical release.
The [combined ownership candidate](../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-fix-2026-10-01/README.md)
was rejected separately and is preserved. The [new ownership fix](../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-ownership-after-bounds-2026-10-01/README.md)
on accepted bounds passes14 original topology/lifetime/regression processes.
Compound scalar/material getters and actual viewer controls remain open.
The [combined precommit review](../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-ownership-captures-2026-10-01/README.md)
retains Village's missing GPU ground/buildings: the unchanged shared65536-slot
renderer pool cannot register both52500-child compounds. This visual failure
belongs toPR09; no visual-parity or performance pass is inferred.

Full-state schema v24 includes host peak occupancy, the harvested GPU contact
peak and its persistent device query word. Readers retain earlier schemas and
reject missing or invalid new fields; historical captures do not prove coverage
of these new fields. Sixteen storage/comparator controls and the focused GPU/API checks pass. Broader
public sensor/compound/mesh populations, actual viewer controls and final-build
physical qualification remain pending. Host occupancy scans run once per positive
step; device contact peaks add diagnostic atomics. Their cost is unmeasured and
must pass PR08 performance qualification; no no-regression claim is made.

## Missing features and known failures

The historical PR01 linked-build audit on 2026-10-01 identified **35 stub definitions and 8
additional placeholders**, with 10 CPU-only comparison wrappers requiring semantic
review and no additional missing linked symbols in either selected backend.
The current diagnostics inventory has 415 symbols, 39 explicit exclusions and
zero remaining stub/placeholder/missing/duplicate/CPU-only passthrough entries.
This proves source/link coverage, not broad behavior qualification. The
speculative setter is implemented and routed to both mapped worlds. Full native
compatibility must not be inferred from scene checks. See the
[portable control report](../experiments/gpu-physics/benchmarks/production-readiness/pr01-speculative-2026-10-01/README.md); the production roadmap keeps PR02 API contract work open.

| Gap | User-visible consequence / remaining work |
|---|---|
| Joint query limits | Wheel angular separation is unimplemented upstream; its release fallback is zero. Reaction precision limits are described below. |
| Falling-cube GPU capacity | The archived sweep hit the old 65,536 spatial-insertion limit at 20,000 cubes. Insertion buffers now scale with reserved shape capacity; the static-contact sort also no longer packs body/index into 16-bit halves. Pair/contact buffers and their prefix scans now scale with reserved body capacity. Pair/history storage and callbacks/events now use full-width endpoints; canonical cell ownership removes online packed-key deduplication. High-index collision, event, remap, and reuse regressions pass. Native sample C metadata now uses per-world growable chunks and passes high-index ownership/callback regressions; Sokol debug/opaque renderer reservations now follow benchmark size, and both engines must upload every cube and the floor. The sample fixes a saved draw-distance culling issue. Updated full-scene app measurements replace the earlier unvalidated Sokol curve. Collision-heavy physics and direct rendering are qualified through 200,000 cubes across three 330-step trials each; Sokol stops below 10 FPS at 150,000. The former tiled-dispatch boundary is resolved. The 200,000-cube physics buffers occupy 3.42 GiB, excluding renderer/driver resources; this is not a measured memory ceiling. |
| World controls | Speculative switching controls experimental hull–mesh positive-gap contacts per world, subject to endpoint flags and the documented CCD handoff shell. Warm-start switching controls contact and joint caches per world. CPU worker/static-tree APIs explicitly report ENOTSUP. |
| World diagnostics | New public counters, partial GPU profiles, occupancy/allocation and bounds/memory diagnostics are implemented with explicit availability; Focused C/Rust checks and required recordings pass; actual counter/tab interactions and broader behavior remain open. See the native diagnostic contract above. |
| Recording/replay | Native recording creation, storage, file I/O, playback, seeking and query-history APIs are explicitly unavailable, with ENOTSUP and a thread-local operation name. Diagnostic state replay does not implement these APIs. |
| Combined viewer coverage | Current shared diagnostics report GPU data and explicit CPU helpers return CPU data; excluded operations preserve errors. No remaining CPU-only passthrough is found in the linked inventory. Other implemented API behavior still requires contract qualification. |
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

`c_abi/spherical_compliance_reference.cpp` independently checks steady cone-limit
compliance under constant torque. With unit isotropic inertia, coincident COM
anchors and unchanged constraint tuning (60 Hz, damping ratio 2), the predicted
deflection is torque divided by `I * (2*pi*effectiveHertz)^2`. Each of two loads
runs 600 steps at four substeps; the last 120 must agree within `1e-4` radians
and remain below `1e-3` rad/s. Link it as an independent CPU fixture or through
the GPU sample API bridge, using the same library configuration as the other
C-ABI reference fixtures. The qualification report records frozen build commands
and results for ordinary and native cached paths. This isolated check excludes
contacts and does not establish acceptance of friction-loaded Rain joints.

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
Transform setters retain the supplied float32 body origin while updating the
center of mass. This avoids losing an ULP by reconstructing an offset shape's
origin before any simulation step; integrated motion replaces the cached origin.

The resident GPU path and convex GPU CCD are eligibility-based optimizations.
Unsupported geometry, callbacks or other excluded world features retain the
ordinary path. Sleep dispatch elision requires a valid zero-active proof across
an unchanged eligible submission chain. Command replay must retain resources
through completion and invalidate on relevant parameter/resource changes.

CCD activation now caps the half-minimum-extent motion threshold at the existing
20 mm speculative-contact range. This prevents larger bodies from crossing that
range without either discrete contact response or a continuous sweep. Host and
device CCD use the same bound, with existing exclusions and smaller-body limits
preserved. The unchanged restitution fixture passes on both paths; broader
qualification and the cost of the additional sweeps remain under evaluation in
[the solver qualification report](gpu-solver-qualification.md).

Empty contact manifolds are recomputed rather than recycled: they contain no
separation bound proving that motion stayed outside the speculative range.
This matters after CCD stops a body near a surface, where the following step
must create discrete support contacts. Nonempty contact recycling remains
available. Both GPU paths pass the focused landing regression and existing
CCD, recycling and restitution checks; full solver qualification remains open.

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
center changes subsequent branch choices again. Checkpoint independent runs in
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

A follow-up inversion experiment on 2026-09-27 compares the saved full native
matrices with the same matrices after forcing the GPU's symmetry convention.
Box3D's own `b3InvertT`, compiled with contraction disabled, confirms that
64 of 112 bodies change: 272 of 1,008 inverse-matrix components differ, with
maximum absolute difference `3.05175781e-5`. An independent float32 Python
calculation gives the same counts and maximum. The diagnostic source is
`artifacts/ragdoll-impact-goal/check_mass_inverse.cpp`; per-body results are in
`mass-inverse-symmetry.json` alongside it. This isolates a consequence of the
storage difference, but does not yet prove a trajectory improvement. Next,
retain all nine native components through capsule mass aggregation and inverse
inertia upload in an experimental replay, and compare the earliest joint solve
before evaluating independent 600-step runs. Do not replace the full visual
gate with this initial-state comparison.

The full-matrix diagnostic was then run independently for all 600 steps on the
ordinary GPU path (`artifacts/ragdoll-full-inertia-final`). It recomputes capsule
mass from the GPU scene's original endpoints, radius and stored density, asserts
identical mass, applies the existing COM shift, and uploads all nine inverse
components. It neither reads CPU traces nor resynchronizes poses. Stability and
the collision-free check pass (maximum position error `2.73e-6` m), but visual
agreement still first fails at frame 90: maximum orientation error 5.02408°,
RMS 0.72294°. Frame 143 position error is 0.05656 m RMS / 0.25304 m maximum;
frame 600 is 0.29965 m RMS / 0.97645 m maximum. These improve some later averages
without meeting the goal. Frame 87 RMS position error is slightly worse than the
checkpoint (8.07 versus 7.36 micrometres), so this does not remove the early
solver drift. The earlier `ragdoll-full-inertia-experiment` used reconstructed
density and is superseded by the stored-density run.

The temporary upload/shader changes are archived in
`artifacts/ragdoll-impact-goal/full-matrix-experiment.patch` and removed from the
working implementation. They only covered initial single-capsule uploads;
a durable change would also need mass updates, compound aggregation, queries,
layout tests and native cached validation. There is no justification yet to
promote that incomplete experiment based on improved averages. The next useful
diagnostic is the first biased joint solve with identical input matrices and
states, particularly the gyro inertia reconstruction and prepared effective
mass arithmetic. Full-matrix retention alone is insufficient.

A separate joint-softness experiment isolates ordinary WGSL division as another
float32 mismatch. An independent `b3MakeSoft` fixture checks the actual ragdoll
default (60 Hz, damping ratio 2, 1/240 s), damping ratios 1/0/0.7 and a longer
timestep. The original shader fails exact coefficient comparison; using
`gyro_divide` and `gyro_recip` passes all five cases. All five existing/added
native-precision tests pass on ordinary and native cached backends (the first
suite run covered four softness cases; the subsequent ordinary run adds the
actual damping-2 default). Sources, CPU goldens, logs and the test/change patch
are in `artifacts/ragdoll-impact-goal/softness-*`.

However, independent runs in `artifacts/ragdoll-softness-precision[-native]`
regress despite byte-identical traces between backends for all six fixtures.
Stability and collision-free checks still pass, but the visual gate first fails
at frame 88 (6.0700° maximum orientation). Frame 143 position error is 0.23998 m
RMS / 1.01415 m maximum; frame 600 is 0.55979 m RMS / 1.74641 m maximum.
The maximum velocity-component error improves at frame 1 (8.94e-7 to 6.56e-7),
then grows at frame 2 (6.56e-7 to 1.12e-5) and jumps at frame 3 (2.23e-5 to
0.036965). `softness-early-errors.json` records each pre-impact frame. Correct
softness alone does not close the remaining contact/constraint branch
sensitivity. The experiment and regression test are archived together in
`softness-precision-experiment.patch`; production code is restored to the
checkpoint. Next replay the frame-2-to-3 transition with identical body states,
contact caches and joint impulses, starting with sample row 14 (the second
human's pelvis), which has the largest frame-3 velocity error. Compare the
earliest changed contact manifold/constraint operation rather than judging
only later averages.

The follow-up frame-3 trace rules out two coarse explanations for the softness
experiment's early jump. Re-evaluating all 91 capsule pairs within the second
human with the native collision routine and each run's frame-2 poses yields
17 candidate manifolds in both cases, with no point-count change or normal
component difference above 1e-3. This enumeration includes filtered pairs and
is only a geometry diagnostic. Live CPU/GPU contact traces then confirm the
same six retained contacts for that human through frames 1–3, with matching
point counts and feature IDs. At frame 3, public shape pairs (18,23) and (18,25)
have nearly identical normals but accumulated normal-impulse differences of
0.104900 and 0.399205 respectively.

Graph traces cover all 24 live contacts at each of frames 1, 2 and 3. After
converting GPU zero-based shape indices to public IDs, every pair and graph
color matches CPU. The traced CPU and GPU runs each reproduce all 448 states
of their corresponding softness-run prefix exactly. Evidence and reproducible
sources are `artifacts/ragdoll-impact-goal/early-*` and `compare_early_*.py`.
Contact membership, feature changes and contact-color assignment therefore do
not explain this jump; the next trace must compare per-substep contact inputs,
warm-start impulses and solver updates for pairs (18,23)/(18,25), including the
interleaved joints. The checkpoint implementation remains unchanged.

Per-operation probes now locate that amplification. CPU wrappers and temporary
GPU append-buffer probes reproduce all 448 uninstrumented states exactly, with
no trace overflow. Body 16 (sample row 15, spine-01) has 120 matching phase keys;
the pelvis, body 15, has 72. The first large jump is the color-5 **biased contact
solve in substep 3** of frame 3, for public shape pair (18,25): the spine's
maximum velocity-component error rises from about 4.9e-5 to 0.1045. Subsequent
unbiased joint solves transmit it to the pelvis and spine-02. Initial traces
of body 17 observed that propagation; body 16 is the direct contact endpoint.

A copied native contact solver, compiled with the oracle's optimization and
contraction flags, and GPU scalar probes identify the actual branch change.
At the preceding unbiased solve, CPU separation remains `2.98023224e-8` m while
GPU separation rounds to zero. In the next biased solve, CPU still uses that
positive separation and the speculative branch (`massScale=1`,
`impulseScale=0`); GPU uses zero and the soft branch (`massScale=0.9422793`,
`impulseScale=0.057720676`). The negative normal-impulse increments become
`-0.48603642` versus `-0.25074485`, despite nearly identical incoming normal
velocities (`-0.08993538` versus `-0.08993432`). Both use the same normal mass
and base separation. This is a discontinuous solver response to a one-ULP
separation difference, not a contact membership or warm-start ordering change.

Evidence is in `artifacts/ragdoll-impact-goal/early-phase-*` and
`early-normal-*`; the copied CPU probe source is `early_contact_solver.c`.
The final detailed probes again preserve all 448 states and report 200 records
without overflow. Temporary shader edits are archived in
`early-normal-instrumentation.patch` and restored out of production. Next
compare the moving-anchor/separation arithmetic on identical inputs, then its
input rotation/position deltas. Do not replace `s > 0` with a tolerance or
`>= 0`: that would change upstream solver behavior rather than fix the numerical
source. The independent 600-step acceptance gate remains unmet.

The moving-anchor calculation itself passes identical-input replay. The native
SIMD probe exports both delta quaternions, position deltas, contact anchors,
normal and base separation for all eight solves of the critical contact in
frame 3. Its 448-state trajectory is unchanged. Replaying those exact float32
inputs through the production GPU `current_sep` and quaternion helpers matches
all eight native separations **bit-for-bit**, on both ordinary and native cached
paths. In particular, each native `0x33000000` separation remains positive on
GPU with the same inputs. Evidence is `separation-input-*`,
`separation-replay-{gpu,native}.log`, and `separation-replay-test.patch` in the
same diagnostic directory. The divergence therefore enters through prior state
updates, not an isolated defect in the separation helper.

A combined full-inertia plus corrected-softness experiment now also has complete
independent runs on both paths in
`artifacts/ragdoll-combined-inertia-softness[-native]`. All six GPU fixture
traces are byte-identical across paths. Stability and collision-free checks
pass (collision-free maximum position error `2.85e-6` m). The softness-only
frame-3 amplification disappears: maximum velocity-component errors for frames
1/2/3 are `6.71e-7`, `7.86e-7`, and `1.22e-6`, respectively. However, visual
agreement still first fails at frame 90 (5.02276° maximum orientation). Frame
143 position error is 0.05642 m RMS / 0.25283 m maximum; frame 600 is 0.30779 m
RMS / 1.01822 m maximum. This demonstrates interaction between the numerical
fixes; neither a single-fix regression nor improved later averages establish
the final outcome.

The combined diagnostic is archived as
`artifacts/ragdoll-impact-goal/combined-inertia-softness-experiment.patch`;
checkpoint production code is restored because its full-inertia upload remains
an initial-single-capsule experiment. Use this combination as the next precision
baseline. The spherical joint still has concrete preparation differences:
GPU normalizes the swing axis with `inverseSqrt`, and computes
`IA*axis + IB*axis`; native normalizes through square root/division and computes
`(IA+IB)*axis`. Compare these on identical joint inputs before the next full
run. A durable full-matrix implementation and all final scene/performance
qualification remain required.

Spherical preparation now has a retained production correction. A native wrapper
captures world inverse inertias and prepared frame quaternions for bodies 2 and
16 at frames 1–3, with CPU swing axis/mass and twist Jacobian/mass as goldens.
On these identical inputs, the former GPU preparation differed in 24 of 48
output components; some swing-axis components differed by dozens of float32
steps. The shader now uses the existing scalar cross/normalization, corrected
square-root/division/reciprocal helpers, and native `(IA+IB)*axis` operation
order. All 48 components match bit-for-bit on ordinary and native cached paths.
The portable fixture is `src/fixtures/spherical_preparation.rs`, and
`native_spherical_preparation_matches_cpu` extracts the production helper code.
The GPU test-dispatch helper is shared with the existing scalar precision test.
All five native-precision tests pass on both paths after that refactor.

Both the combined experiment and the retained standalone correction have
complete independent runs. `artifacts/ragdoll-spherical-preparation[-native]`
includes full inertia and corrected softness: frame 143 position error is
0.05641 m RMS / 0.25283 m maximum, and frame 600 is 0.30503 m RMS / 1.01581 m
maximum. `artifacts/ragdoll-spherical-preparation-only[-native]` reflects the
standalone spherical preparation correction, without the incomplete inertia experiment or
softness change: frame 143 is 0.07262 m RMS / 0.25318 m maximum, and frame 600 is
0.36660 m RMS / 1.06212 m maximum. Both configurations still first fail at frame
90. In each configuration all six GPU fixture traces are byte-identical across
ordinary/native paths, and stability/collision-free checks pass. The retained
change fixes a proven same-input arithmetic mismatch; it does not establish
visual-goal completion or a substantial visual improvement by itself.

The combined shader/upload experiment is saved in
`artifacts/ragdoll-impact-goal/spherical-combined-experiment.patch`; at that checkpoint only the
spherical preparation correction, portable fixture and test helper remained in
production source. CPU capture and before/after logs are `spherical-prepare-*`
in the same directory. Durable full-matrix mass handling and the remaining
600-step divergence are still open, as are full Rain and performance
qualification. The checkpoint recordings below cover the retained solver change.

The capsule parallelism threshold was also checked with an independent native
boundary fixture. The old GPU literal `0.0025` is one float32 step below native
`0.05f * 0.05f`: GPU produced one point where native produced two. The shader
now preserves the typed float32 multiplication, and
`capsule_parallel_threshold_matches_native_float_product` changes from failing
(one point) to passing (two points) on ordinary and native cached GPU paths.
This restores native semantics without changing its tolerance. Both complete runs in `artifacts/ragdoll-capsule-threshold[-native]` have
byte-identical GPU traces across all six fixtures. The 600-step ragdoll trace
is also identical to the previous production trace, so this boundary bug does not explain
the observed frame-90 divergence. Stability and collision-free checks pass.
Projection of clipped capsule points also differed: native divides by segment
length squared and returns out-of-range endpoints exactly, while GPU projected
using a normalized axis. Six independent native point-projection goldens now
cover ragdoll capsule endpoints, oblique segments and both exterior regions.
The former formula fails the exterior-B case by one float32 step; the native
operation order passes all six cases bit-for-bit on ordinary and native cached
paths in `native_capsule_projection_matches_cpu`. The capsule manifold now uses
that helper for both clipped points. Full independent runs in
`artifacts/ragdoll-capsule-projection[-native]` produce identical GPU traces
across all six fixtures. Stability and collision-free checks pass, but visual
agreement still first fails at frame 90. The first state changed by projection
is body 98 at frame 89. Frame-143 position error increases to 0.08456 m RMS
(0.25318 m maximum); frame 600 is 0.40698 m RMS (1.06212 m maximum). This is a
verified local arithmetic correction, not a visual improvement. Earlier
inertia/preparation differences and the full acceptance target remain open.

The full-inertia/softness experiment was revalidated with these capsule fixes.
Its enlarged body-extra layout had left the old capacity divisor in the graph
memo shaders and an old slot stride in the body-extra test; the corrected
experiment updates those sites as well. The body-extra GPU readback test passes.
Graph memoization is disabled for these jointed ragdolls, and the complete
`artifacts/ragdoll-full-layout-projection[-native]` runs confirm all six GPU
traces are byte-identical both across paths and to the earlier combined
spherical-preparation experiment. Frame 143 returns to 0.05641 m RMS / 0.25283 m
maximum; frame 600 is 0.30503 m RMS / 1.01581 m maximum. First failure remains
frame 90 (5.02277 degrees maximum orientation error). Stability passes and
collision-free maximum position error is 2.85335e-6 m.

The corrected diagnostic is archived as
`artifacts/ragdoll-impact-goal/full-layout-corrected-experiment.patch` and removed
from production source. A durable implementation must retain all nine inertia
entries through shape/body mass accumulation, density/geometry changes, explicit
mass overrides, queries, impulse handling and both scene upload paths. The
current six-entry host representation loses independently rounded native matrix
entries before inversion; the diagnostic still bypasses it only for initial
single-capsule bodies. Its new stride also needs the corrected graph-cache
capacity calculations. These are implementation requirements, not evidence that
the 600-step visual goal is met.

The full tensor is now retained in normal capsule/shape/body mass storage, with
native cofactor inversion and both off-diagonal triangles uploaded after either
scene allocation or scene updates. Geometry and density setters preserve the
full capsule tensor; mass overrides, queries and host angular impulses also use
all nine entries. The body-extra allocation grows from 16 to 20 words, with graph
cache capacity calculations and step-force offsets updated together. The capsule
mass result keeps its existing six-component view and additionally exposes the
full column-major tensor. Joint and spring softness use the previously verified
native division/reciprocal corrections.

Five independent C goldens in `src/fixtures/full_mass_lifecycle.rs` cover initial
mass, a density edit, geometry replacement plus mass recomputation, adding a
second capsule, and removing it. All 13 components (mass, center and nine inertia
entries) match bit-for-bit in ordinary and native cached API tests, including an explicit mass
round trip. The GPU body-extra readback test checks both triangles and the
diagonal as live body counts change. Complete independent runs in
`artifacts/ragdoll-full-inertia-durable[-native]` require no diagnostic override:
all six fixture traces are identical across paths and to the corrected
experiment. Stability and collision-free checks pass, but the visual target
still fails at frame 90; frame-143 RMS remains 0.05641 m and frame-600 RMS
0.30503 m. Offset Kinematic (1e-5/300 steps) and all five Prismatic fixtures
(1e-4/120 steps) pass after this implementation. Logs are
`artifacts/ragdoll-impact-goal/full-durable-*`. Further impact-phase diagnosis,
Rain, ground dragging, performance and recording qualification are required.
Scene updates currently upload the full inverse tensor for every live body;
that upload cost is part of the pending performance check.

A fresh first-step phase trace on the full-inertia implementation is saved in
`artifacts/ragdoll-impact-goal/full-first-phase/`. Both instrumented runs preserve
their complete 224-state baseline prefixes exactly, with 240 GPU trace records
and no overflow. For each sampled spine body (engine indices 2 and 16), all 120
phase keys agree. Comparing decoded float32 values rather than decimal formatting,
warm start and the first biased joint solve match exactly. The first mismatch
for these bodies is after biased joint color 1 in substep 1: angular velocity Z
is -7.235892553580925e-6 on CPU versus -7.235892098833574e-6 on GPU, one float32
step (4.54747e-13). This identifies a remaining first-step joint difference; it
is not a claim that every body/operation has been traced.

A copied native spherical solver captures the selected joint's final torque
update inputs and preserves the same CPU state prefix. Replaying those identical
inputs through GPU matrix multiplication with either `cross` or `gyro_cross`
returns all three expected output bits exactly. The final torque update alone
therefore does not explain that mismatch; next compare the incoming angular
velocity, point-constraint impulse, lever arm and prepared inertia within joint
color 1. Temporary shader and replay-test probes have been removed from
production source; captures, probe sources and comparison scripts remain in
that artifact directory. The independent 600-step acceptance result is unchanged.

The follow-up probe in `artifacts/ragdoll-impact-goal/full-joint-inputs/`
preserves both baseline prefixes and emits 12 records without overflow. In the
first selected solve, the lever arm, point impulse and all nine prepared inertia
entries agree exactly; incoming angular velocity Z already differs by one step.
The complete point-constraint calculation also matches all four native substep
results on identical inputs. The spherical motor used `solve3` on the summed
inertia, whereas native prepares its inverse and then multiplies by the velocity
error. An independent four-case capture reproduces a one-bit motor-impulse
mismatch with the old helper. Spherical motor and spring paths now use the
existing native-order inverse-matrix helper; all four cases match bit-for-bit in
`native_spherical_motor_mass_matches_cpu`, using the portable fixture
`src/fixtures/spherical_motor_mass.rs`. All seven native precision tests pass.
Temporary probes are removed. Both complete runs in
`artifacts/ragdoll-spherical-motor-mass[-native]` have byte-identical GPU traces
across all six fixtures. Stability and collision-free checks pass, but visual
agreement still first fails at frame 90;
frame-143 position RMS is 0.05637 m and frame-600 RMS is 0.35936 m. The latter is
worse than the preceding full-inertia result, so the verified local correction
is not evidence of improved overall visual agreement. Further first-step joint
precision and impact-phase diagnosis remain necessary.

A subsequent identical-input motor-clamp replay captures 1,094 unique cases
from the first three independent CPU frames. Its 448-state CPU prefix remains
byte-identical to the uninstrumented reference. The old spherical GPU clamp
mismatches 154 cases; native compares the rounded vector length with the limit
and scales by a rounded division, whereas the GPU compared squared lengths and
used uncorrected square-root/division operations. The spherical path now uses
`gyro_dot3`, `gyro_sqrt`, and `gyro_divide` in native order. All 1,094 captured
cases match exactly with the correction. A compact 16-case regression retains
unchanged inputs and twelve previously failing cases in
`src/fixtures/spherical_motor_clamp.rs`; the full capture and replay logs remain
in `artifacts/ragdoll-impact-goal/motor-clamp/`. Full independent-run impact of
this clamp correction is still under validation; local exactness is not visual
acceptance. Both full-path builds initially crash inside NVIDIA 610.57.04's
shader compiler while compiling `solve_joints`, also with a fresh pipeline
cache. Removing the corrected square root allows compilation but leaves 53
identical-input mismatches, so it is not the selected implementation. Replacing
only the square-root midpoint rounding branches with equivalent nested selects
preserves all eight native precision tests, including all 1,094 clamp cases,
but still crashes the full compiler. Making the entire corrected square-root
helper branch-free (using safe placeholder input for exceptional values and
selecting the original fallback result) preserves all eight precision tests
and compiles the full ordinary joint shader. The ordinary complete run in
`artifacts/ragdoll-spherical-motor-clamp-branchless` passes stability and
collision-free checks (maximum position error 4.07304e-6 m), but visual agreement
still first fails at frame 90 (maximum angle 5.02612°, RMS angle 1.25228°).
Frame-143 position RMS is 0.084549 m and frame-600 RMS is 0.380374 m, both worse
than before the clamp correction. The native cached full run in the corresponding `-native` directory completes
with byte-identical GPU traces for all six fixtures and the same failing visual
result. The exact local correction does not establish improved trajectory
agreement.

The refreshed trace in `artifacts/ragdoll-impact-goal/post-clamp-phase/`
preserves the independent CPU/GPU state prefixes exactly. For engine bodies 2
and 16, all 48 before/after spherical-joint records align by phase and color.
The earlier first-substep biased-joint mismatch is gone for these sampled
bodies. Their first remaining mismatch is after color-0 warm start in substep
two: incoming velocities match, but outgoing angular velocity X differs by one
float32 step (2.98023224e-8). This is a sampled-body finding, not a claim of the
globally earliest operation across all 112 bodies. An independent native
warm-start capture preserves the reference CPU prefix. An initial replay placed
builtin and scalar cross products together and matched all eight updates, but
that combined test was insufficient to rule out context-dependent arithmetic:
separate replay shaders expose the mismatch, as described below.
Temporary GPU trace and replay-test probes have been removed from production
source; sources and results remain in that artifact directory.

The follow-up capture in `artifacts/ragdoll-impact-goal/warm-inputs/`
preserves all 224 CPU/GPU prefix states and emits 20 records without overflow.
At the first discrepant warm start (substep two, color 0, body B=2), every
recorded input is bit-identical: all nine inertia entries, rotated and base
lever arms, linear/angular/spring/motor impulses, swing axis and impulse,
twist Jacobian and limits, incoming angular velocity, and delta quaternion.
This contradicts the earlier cached-input hypothesis: the full-shader arithmetic
produces the difference despite identical inputs, while isolated replay matches.
Substeps three and four then contain differing cached inputs as a consequence.
Separate replay shaders expose the cause: builtin `cross` differs in three of
the eight captured torque updates, including the exact second-substep X error;
`gyro_cross` matches all eight. The earlier combined replay could share builtin
and scalar calculations and concealed this mismatch. Spherical warm-start
updates now use the explicit scalar cross in `spherical_warm_torque`. A portable
regression evaluates only the production expression against the eight native
goldens (`src/fixtures/spherical_warm_torque.rs`); it passes on the ordinary path,
and all nine native precision tests pass. Complete runs in
`artifacts/ragdoll-spherical-warm-torque[-native]` have byte-identical GPU traces
across all six fixtures. Stability and collision-free checks pass (3.19610e-6 m
maximum collision-free position error), but visual agreement first fails at
frame 90 (5.02220° maximum orientation error). Frame-143 position RMS is
0.0564995 m and frame-600 RMS is 0.359729 m. This improves on the preceding clamp
run, but does not meet acceptance. The auxiliary full-shader
intermediate-value probe preserves the original state prefix, yet its separately
computed builtin/scalar torque expressions both match CPU at substep two while
the original update remains different. Those side expressions therefore cannot
stand in for the production update; the separate-shader replay reproduces its
actual one-step error. Probe sources and all outputs are retained in the same
artifact directory, with no diagnostic probes left in production source.

The joint setup audit now reads actual engine getters for all 112 joints:
72 spherical, 32 revolute, and eight collision filters, with 224 body incidences.
All endpoint pairings and 3,672 stored float32 fields match exactly on ordinary
and native cached paths (`artifacts/ragdoll-joint-setup[-native]`). The 104
force-applying joints already matched; filter creation now retains identity
local rotations and native constraint defaults, and the C wrappers preserve
custom filter frames and tuning. Filters do not apply forces, so this metadata
fix is not a trajectory correction. The default/custom filter regression passes
in `artifacts/ragdoll-joint-setup/metadata`. The ragdoll validator checks these
settings before trajectory validation, including the collision-free fixture;
`--joint-setup-only` permits an explicit setup-only check without claiming a
600-step pass. Older trace artifacts predate this metadata and cannot satisfy
the extended validator without new runs.

The complete first-substep trace in
`artifacts/ragdoll-impact-goal/all-first-substep-complete` covers all 112 bodies,
with matching 1,024 CPU/GPU operation keys and unchanged 224-state prefixes.
It includes both spherical and revolute solves and contact chains. The first
mismatching output is the color-zero biased revolute solve on body 7:
angular Y differs by 4.54747e-12. Its incoming state matches. This supersedes
the earlier partial trace, which omitted revolute joints and covered only
88 bodies; later spherical warm-start differences are not the earliest
remaining divergence.

A native capture of the 32 first-substep hinge alignment solves preserves the
224-state CPU prefix (`artifacts/ragdoll-impact-goal/revolute-mass`). Separate
GPU replay shaders show discrepancies for all 32 original calculations and
exact results for all 32 after matching native inertia addition and `b3Solve2`
operation order. The solver now adds the two inertia matrices before multiplying
by the axes, and applies the reciprocal to each matrix coefficient before the
right-hand-side products. The portable regression exercises the production
helper against the four distinct captured inputs. It passes on the ordinary
path and all ten native precision tests pass. Full independent runs in
`artifacts/ragdoll-revolute-perp-mass[-native]` produce byte-identical GPU traces
across all six fixtures. Setup, stability and collision-free checks pass
(maximum collision-free position error 2.99709e-6 m). The first visual failure
moves from frame 90 to 106 (orientation RMS 1.10003°); frame-143 position RMS
falls from 0.0564995 m to 0.0327156 m and maximum error is 0.187503 m.
Frame-600 RMS falls from 0.359729 m to 0.165717 m, with maximum 0.523952 m.
These are improvements, but 495 frames still fail the unchanged visual targets.

The follow-up complete trace in
`artifacts/ragdoll-impact-goal/post-revolute-first-substep` preserves both
224-state prefixes and all 1,024 operation keys across 112 bodies. Mismatching
records fall from 174 to 46. The earliest remaining mismatch moves to operation
894, a color-two biased spherical shoulder solve between CPU body IDs 47 and
54 (logical bones 45 and 52); incoming states match. The velocity-Z difference
is 1.16415e-10 and angular-Y difference is 1.86265e-9. The previous color-zero
hinge divergence is absent. Temporary trace probes have been removed from
production source; their sources and results are retained with the capture.

The shoulder follow-up captures native inputs to the point constraint without
changing its 224-state prefix (`artifacts/ragdoll-impact-goal/shoulder-point`).
Separate replay shaders reproduce one-bit X/Z relative-velocity differences
from builtin `cross`; explicit `gyro_cross` matches CPU. Expanding the capture
to all 72 spherical solves gives 12 distinct cases, of which two fail with the
builtin and all match with the scalar cross. The spherical point-constraint
velocity now uses that scalar expression. Its portable production-helper test
passes on the ordinary path (`src/fixtures/spherical_point_velocity.rs`). The
full-shader intermediate capture preserves both 224-state prefixes, emits all
16 expected records, and confirms identical incoming velocities, levers, inertia
matrices and bias. Relative velocity is the first differing intermediate;
its X/Z errors propagate into the right-hand side and solved Z impulse.
This reproduces the isolated replay's error in the full solver. All eleven
native precision tests pass. Independent 600-step runs in
`artifacts/ragdoll-spherical-point-velocity[-native]` have identical GPU traces
across all six fixtures; setup and stability pass, and collision-free maximum
position error improves to 2.73652e-6 m. However, contact trajectory agreement
regresses from the preceding hinge-only correction: the first visual failure
returns to frame 90, frame-143 position RMS is 0.0565945 m (maximum 0.253419 m),
and frame-600 RMS is 0.296925 m (maximum 0.931233 m). The correction matches
native arithmetic, but is not evidence of improved overall visual agreement.

With the spherical velocity correction, the complete first-substep comparison
in `artifacts/ragdoll-impact-goal/post-velocity-first-substep` has zero mismatches:
all 1,024 CPU/GPU operation keys and values match across 112 bodies, with both
224-state prefixes unchanged. This covers warm start and the first biased joint
and contact passes before position integration. It does not cover subsequent
integration, relaxation, or later substeps; those remain the next place to
locate the earliest difference. Exact agreement in this early window does not
supersede the failing independent 600-step result above.

The expanded frame-one trace in `artifacts/ragdoll-impact-goal/full-first-frame`
covers 6,240 matching operation keys across all 112 bodies, including all four
substeps and final convex restitution. CPU/GPU state prefixes remain unchanged;
the temporary 8,192-record buffer did not overflow and has been restored to the
production size. There are 4,668 differing records. The first occurs at CPU
operation 1,038: first relaxation pass, color zero, revolute joint body 7/8.
Its recorded incoming velocities and position deltas match, while the outgoing
angular velocity differs by 1.49012e-8. This is later than the now-exact initial
warm/biased pass. Delta quaternions and internal joint inputs are not included
in this broad trace and still need comparison before attributing the error.

The first relaxation hinge investigation captures all 128 native hinge solves
across frame one's four substeps, preserving the CPU state prefix
(`artifacts/ragdoll-impact-goal/hinge-relax-point`). Separate replay shaders
find builtin-cross relative-velocity differences in 64/128 cases; scalar cross
matches every captured CPU result. Hinges and spherical joints now share
`joint_point_velocity`, and portable regressions pass for the 12 spherical
cases plus 16 hinge cases covering four hinges in all four substeps. The full
hinge probe preserves both 224-state prefixes and emits all 72 records without
overflow. At the first relaxation solve, both delta quaternions, incoming
velocities, lever arms, bias and inertia matrices match exactly. Relative
velocity is the first differing intermediate, followed by the solved impulse;
this confirms the isolated replay's cause in the complete solver. All twelve
native precision tests pass. Complete independent runs in
`artifacts/ragdoll-revolute-point-velocity[-native]` produce identical GPU traces
across all six fixtures. Setup, stability and collision-free checks pass
(maximum collision-free position error 2.93946e-6 m). Visual acceptance still
first fails at frame 90; frame-143 position RMS is 0.0564507 m (maximum
0.253096 m), and frame-600 RMS is 0.311826 m (maximum 1.09292 m). The latter
regresses from the preceding 0.296925 m, so exact local arithmetic still does
not establish improved contact trajectories.

The subsequent full-frame trace (`artifacts/ragdoll-impact-goal/post-hinge-velocity-frame`)
preserves both production state prefixes and aligns all 6,240 records without
overflow. Differing records decrease from 4,668 to 4,444; the former first
body-7/8 relaxation mismatch is eliminated. The first difference is now CPU
operation 1,046, color-zero relaxation of hinge bodies 11/12, with maximum
angular-velocity differences of 1.49012e-8 and 5.96046e-8 respectively. This
local trace improvement does not change the failing full-run acceptance above.

The body-11/12 point probe (`artifacts/ragdoll-impact-goal/hinge-left-arm-point`)
preserves both 224-state prefixes and all 72 trace records without overflow.
Its lever arms, delta quaternions, inverse inertias and linear velocities match;
angular velocities already differ before the point solve. An independent
native capture of all 128 relaxation alignment dot products finds 75 mismatches
with builtin `dot`, and none with the CPU-order scalar `gyro_dot3` expression.
The hinge alignment RHS now uses `revolute_alignment_velocity`, with 16 native
goldens covering four hinges across all four substeps. All thirteen native
precision tests pass on both ordinary and cached paths. Complete independent
runs (`artifacts/ragdoll-revolute-alignment-velocity[-native]`) have identical
GPU traces across all six fixtures. Setup, stability and collision-free gates
pass (maximum collision-free position error 3.23340e-6 m), but visual acceptance
still fails first at frame 90. Frame-143 position RMS is 0.0709477 m (maximum
0.237992 m); frame-600 RMS is 0.306381 m (maximum 0.706620 m). This is mixed
relative to the preceding result, including worse frame-143 RMS, and does not
establish a visual fix. The fresh full-frame trace
(`artifacts/ragdoll-impact-goal/post-alignment-velocity-frame`) preserves both
state prefixes and all 6,240 aligned records without overflow. Differing
records decrease from 4,444 to 4,264, but the first mismatch remains operation
1,046, body 11/12. Thus the isolated alignment-dot discrepancy is real, but
correcting it does not explain the earliest complete-solver mismatch; internal
alignment inputs and updates still require a full-solver capture.

The subsequent full hinge-stage probe
(`artifacts/ragdoll-impact-goal/hinge-arm-alignment`) preserves both 224-state
prefixes and all 76 records without overflow. On the first relaxation solve,
both incoming angular velocities, both frame quaternions, relative rotation
and alignment axes match exactly. The first difference is already present
after the spring: body 12 angular velocity differs by up to 1.39698e-9.
Motor and alignment differences follow. Thus the next diagnosis is the spring
inputs/effective axial mass/update, before the alignment RHS. Native prepares
axial mass by adding inverse-inertia matrices before multiplying by the axis;
GPU still multiplies each matrix separately, adds the products, and uses builtin
dot/division. Capture/replay is needed to distinguish this from spring velocity
dot rounding or impulse-application differences.

Native spring/preparation capture (`artifacts/ragdoll-impact-goal/hinge-spring-inputs`)
preserves the CPU state prefix and contains all 32 axial-mass preparations and
128 relaxation spring inputs. Separate GPU replay finds 24/32 axial-mass
mismatches using the old expression, versus zero using matrix-sum-first,
scalar dot and corrected reciprocal. With CPU mass held fixed, builtin spring
velocity dot changes 43/128 captured results; scalar dot matches all of them,
including the resulting spring impulse. Production hinges now use
`revolute_axial_mass` and `revolute_axial_velocity` (the latter also applies to
motor and lower/upper-limit axial velocities). Four distinct native mass
cases and sixteen velocity cases spanning four hinges/four substeps are
preserved in `src/fixtures/revolute_axial.rs`. All fourteen production native
precision tests pass on ordinary and cached paths. The cached independent
600-frame gate preserves setup and stability, and improves maximum
collision-free position error to 9e-7 m. Visual acceptance still fails from
frame 90: frame-143 position RMS is 0.0564328 m (maximum 0.252974 m), and
frame-600 RMS is 0.314150 m (maximum 1.10116 m). The mixed visual result remains
far outside the goal. The ordinary run is complete and byte-identical to the
cached path across all six fixtures (`artifacts/ragdoll-revolute-axial[-native]`).
The full-frame trace (`artifacts/ragdoll-impact-goal/post-axial-frame`) preserves
both production state prefixes and all 6,240 matched records without overflow.
Differing records decrease from 4,264 to 2,444. The former first hinge/spring
mismatch is eliminated: the earliest difference is now operation 1,506,
color-four contact relaxation, CPU body 11, angular-velocity Y differing by
9.31323e-10. Both incoming states match. The next investigation must isolate
normal, rolling, twist and central-friction updates in this contact solve;
this trace progress is not a passing visual trajectory.

The contact-phase probe (`artifacts/ragdoll-impact-goal/contact-relax-color4`)
preserves both state prefixes and all 40 aligned records. Incoming states and
the normal impulse update match exactly; the first difference appears after
rolling resistance between bodies 11 and 27. Box3D inverts the summed inertia,
packs its lower symmetric coefficients and uses a right-associated SIMD
matrix multiply. The GPU previously solved the full matrix directly. Native
inverse coefficients are slightly asymmetric after rounding, so packing is
observable. Four captured substep replays all differ with the old calculation
and all match with the native calculation. A separate clamp replay finds one
of four differences from builtin square root/division; corrected scalar helpers
match all four. Convex contacts now use those calculations, with the existing
native epsilon; nonconvex contacts retain their scalar path. The eight cases
are preserved in `src/fixtures/convex_rolling.rs` and exercise the production
helpers. All fifteen native precision tests pass on ordinary and cached paths.
The ordinary independent 600-frame gate (`artifacts/ragdoll-convex-rolling`)
preserves exact joint setup, stability and 9e-7 m collision-free precision.
First visual failure moves from frame 90 to 106 (orientation RMS 1.09878°).
Frame-143 position RMS improves from 0.0564328 to 0.0324712 m, with maximum
0.186759 m; frame-600 RMS improves from 0.314150 to 0.171877 m, with maximum
0.523480 m. These remain failing trajectories. The cached gate completes with
byte-identical GPU traces across all six fixtures and the same report
(`artifacts/ragdoll-convex-rolling-native`). The fresh full-frame trace
(`artifacts/ragdoll-impact-goal/post-convex-rolling-frame`) preserves both
production state prefixes and all 6,240 aligned records without overflow.
Differing records decrease from 2,444 to 2,037; the previous earliest mismatch
is eliminated. The first difference is now operation 2,530, the second biased
contact solve at color four/body 11, with angular-velocity Z error 3.72529e-9.
Its incoming state matches. Offset Kinematic and Prismatic regressions pass
their unchanged tolerances (`artifacts/offset-convex-rolling` and
`artifacts/prismatic-convex-rolling`). Ordinary and cached strict ground dragging also
retains 0.00602796 m maximum position error, 0.00597248 quaternion chord and
0.230405 m/s peak velocity error (`artifacts/drag-convex-rolling[-native]-strict.log`).

The second biased contact probe (`artifacts/ragdoll-impact-goal/contact-bias-color4`)
preserves both production prefixes and all sixteen expected records. Before
the second solve, separation, rotated anchors, delta rotations, point velocity,
normal mass and accumulated impulse match. Contact softness does not: GPU mass
scale is 0.942279279 versus CPU 0.942279339, and impulse scale is 0.0577206761
versus 0.0577206798. The first biased pass already has this discrepancy but
clamps the impulse to zero; the second pass applies a differing impulse.
Contact preparation now uses corrected reciprocal/division helpers and native
zero-hertz behavior. Twenty-four native `b3MakeSoft` cases span three timesteps,
dynamic/static pairs, clamped/unclamped frequencies and zero damping/frequency
(`src/fixtures/contact_softness.rs`). Both ordinary and cached sixteen-test
precision suites pass. Independent ordinary and cached trajectory gates
(`artifacts/ragdoll-contact-softness[-native]`) finish with byte-identical GPU
traces across all six fixtures. Setup/stability and 9e-7 m collision-free
precision pass, but visual agreement worsens relative to the rolling correction:
the first failure returns to frame 90 (5.02308° maximum angle), frame-143
position RMS is 0.0563350 m (maximum 0.252703 m), and frame-600 RMS is
0.280377 m (maximum 0.896352 m). Correcting the isolated arithmetic does not
establish a visual improvement. Offset Kinematic, Prismatic, and ordinary/cached
strict ground dragging pass unchanged tolerances; dragging retains its prior
0.00602796 m / 0.00597248 quaternion chord / 0.230405 m/s maxima
(`artifacts/{offset,prismatic,drag}-contact-softness*`). The fresh complete-solver
trace (`artifacts/ragdoll-impact-goal/post-contact-softness-frame`) preserves
both production state prefixes and all 6,240 aligned records without overflow:
every recorded first-frame solver state now matches exactly, versus 2,037
differences before this correction. This closes the first-frame solver trace,
not the 600-frame visual goal. Final first-frame poses and velocities also match
exactly. Frame two is the first differing output (38 quaternion rows and 95
velocity rows, maximum velocity/angular-velocity component difference 2.39e-7).
The frame-two trace (`artifacts/ragdoll-impact-goal/contact-softness-frame2`)
preserves both 336-row production prefixes and all 6,240 solver records without
overflow. Of these, 3,400 differ. The first mismatch is operation 482, contact
warm start at color four/body 11, with maximum component difference 5.58794e-9;
incoming states match. The initial diagnostic harness readback mistakenly
remained at frame one and captured no frame-two records; correcting readback
and rerunning the same instrumented library produced this complete result.
The targeted warm-start probe (`artifacts/ragdoll-impact-goal/contact-warm-frame2`)
preserves both 336-row prefixes and all sixty records without overflow.
Incoming and post-normal states match; the first difference appears after
central friction. Normals, tangents, anchors, inertia, masses, normal/twist/
rolling impulses all match. The cached second friction component does not:
CPU -0.000831828394 versus GPU -0.000831829559. Native stores world-space
friction and projects it during every preparation. GPU fresh contacts did this,
but `recycle_contact` copied the two components directly. Even an unchanged
normal has a measurable store/project roundoff because its tangent basis is
not exactly unit length. Recycled contacts now use the same projection helper
as refreshed contacts. Four native cases preserve this captured reprojection,
changed tangent bases and reversed endpoint sign
(`src/fixtures/contact_friction_reproject.rs`). Sleeping contacts retain their
cached components. Both seventeen-test precision suites pass. Ordinary/cached
independent runs (`artifacts/ragdoll-recycled-friction[-native]`) produce
byte-identical GPU traces across all six fixtures, preserving setup/stability
and 9e-7 m collision-free precision. Visual agreement still fails at frame 90;
frame-143 position RMS is 0.0565790 m (maximum 0.253201 m), and frame-600 RMS is
0.293754 m (maximum 0.932553 m). This is not a visual improvement over the
preceding softness result. Offset Kinematic, Prismatic and both strict drag
paths pass unchanged tolerances; drag maxima remain 0.00602796 m, 0.00597248
quaternion chord and 0.230405 m/s (`artifacts/*-recycled-friction*`). The fresh
frame-two trace (`artifacts/ragdoll-impact-goal/post-recycled-friction-frame2`)
preserves both 336-row production prefixes and all 6,240 records without
overflow: all recorded solver states now match exactly, eliminating the prior
3,400 differing records. Complete output poses and velocities match exactly
through frame 13. Frame 14 is the first differing output, affecting 37 bodies
with maximum velocity/angular-velocity component error 0.00120892. The targeted
frame-14 trace (`artifacts/ragdoll-impact-goal/recycled-friction-frame14`)
preserves both 1,680-row production prefixes without overflow, but record
identities do not fully align: CPU records 6,552 states, GPU 6,864. All CPU keys
exist in GPU; the extra 312 records come from six additional calf/thigh
contacts in the farther groups. The first differing shared state is the
color-one contact relaxation at CPU body 8 (operation 1,412). A manifold export
(`artifacts/ragdoll-impact-goal/frame14-manifolds`) shows differing capsule
geometry already at frame 13, while those contacts still apply no impulse.
Native capsule-input capture finds the near pair recomputed at frames 1, 2, 4
and 10, then recycled through 14. The complete capture
(`artifacts/ragdoll-impact-goal/frame14-capsule-inputs`) preserves both state
prefixes. GPU recomputes the near pair at frames 1, 2, 3 and 9, and the far
pair at 1, 2, 3 and 6 versus CPU 1, 2 and 4. All shared recomputations have
bit-identical captured inputs and segment-distance results. Empty GPU capsule
pairs cached canonical body order, while populated manifolds used the native
reference-body order. This changes the frame used for the conservative
recycling bound. Empty pairs now use the same reference-order helper as fresh
capsule manifolds. A direct native oracle confirms the expected order for four
dynamic/static/kinematic pair combinations (`ghost-oracle.log`); a GPU lifecycle
test exercises empty-pair creation, recycling and transition to contact for
those cases and passes on ordinary/cached paths. A fresh frame-14 trace preserves
both independent production prefixes and matches all 6,552 phase records
exactly, with identical contact membership and no overflow
(`artifacts/ragdoll-impact-goal/post-ghost-order-frame14`). The complete
`ragdoll-capsule-ghost-order` ordinary/cached gates produce byte-identical GPU
traces across all six fixtures. Setup, stability and collision-free checks pass
(maximum collision-free position error 9e-7 m). Independent poses and velocities
now match exactly through frame 86; the first output difference is frame 87
(`first-divergence.json`). The every-frame visual gate still fails from frame
106 (orientation RMS 1.09608 degrees). Frame 143 position RMS/max are
0.0326912/0.188822 m; frame 600 RMS/max are 0.153941/0.444002 m, with orientation
RMS/max 36.1929/161.874 degrees. These remain outside the goal. Offset Kinematic,
all five Prismatic fixtures, and ordinary/cached strict ground dragging pass
with this correction (`*-capsule-ghost-order*`); drag maxima remain
0.00602796 m position, 0.00597248 quaternion chord and 0.230405 m/s velocity. The frame-87
phase trace preserves both 9,856-row production prefixes and matches all
6,578 operation keys without overflow (`ragdoll-impact-goal/ghost-order-frame87`).
The CPU wrapper now includes mesh-contact stages, which first occur here.
The earliest differing output is the first biased mesh solve, color 22, native
body 8 (the first ragdoll's left calf); all preceding inputs/operations match.
Its maximum velocity/angular-velocity component difference is 0.000261307.
This narrows the next investigation to mesh-contact geometry/preparation or
normal-impulse arithmetic, before later joint propagation. The focused
`ragdoll-impact-goal/mesh-bias-frame87` capture preserves both production prefixes
and captures all 16 point solves without overflow. At the first solve, normals,
anchors, effective mass, softness and incoming normal velocity match exactly;
packed base separation differs by one float32 step (CPU 4.87677812576 versus
GPU 4.87677764893), changing speculative bias from 3.31958746910 to 3.31947302818.
Mesh base packing now preserves native cluster separation without an extra
normal projection and uses scalar dot order for the anchor projection. Changing
only dot order failed the second captured point because native clustering copies
its original separation unchanged. Two captured native preparation fixtures
cover both points. All 18 ordinary precision tests pass; the older capsule
projection test's extracted helper boundary was corrected to exclude the new
reference-order helper and its unrelated flag constants. All 18 cached precision tests also pass. Full independent `ragdoll-mesh-separation[-native]` gates produce identical
GPU traces across all six fixtures. Setup, stability and collision-free checks
pass. The visual gate still fails from frame 106, and outputs still first differ
at frame 87. Frame 143 position RMS/max are 0.0326913/0.188822 m; frame 600
RMS/max are 0.153951/0.444027 m, with angular RMS/max 36.1970/161.873 degrees.
These do not establish a meaningful trajectory improvement. The corrected full
frame-87 trace preserves both production prefixes and all 6,578 operation keys,
but the earliest difference remains unchanged at the first biased mesh solve
(`post-mesh-separation-frame87`). Finalization was still subtracting the mesh
rest offset after packing the base. The mesh path now retains original point
separation in transient `rb.w` through reduction/clustering, applies the rest
offset before packing, and passes raw separation through final COM-anchor
conversion and friction-center preparation. It resets `rb.w` before warm-start
matching. Mesh cluster merging also preserves original separation rather than
reconstructing it from a packed base. All 18 precision tests pass on ordinary/cached paths after this further change.
Full `ragdoll-mesh-raw-separation[-native]` validation produces identical GPU
traces for all six fixtures. Setup, stability and collision-free checks pass;
visual agreement still fails from frame 106, and outputs still first differ at
87. Frame 143 position RMS/max are 0.0326946/0.188842 m; frame 600 RMS/max are
0.153890/0.444340 m, with angular RMS/max 36.2072/161.820 degrees. The new full
frame-87 trace preserves both production prefixes and all 6,578 operation keys.
The first biased mesh solve now has exactly matching linear velocity; one
angular-velocity component differs by 2.38419e-6, down from 0.000261307
(`post-mesh-raw-frame87`). Mesh contacts were still using the six-coefficient,
right-associated SIMD inertia multiplication, whereas CPU mesh/overflow use
all nine matrix entries and scalar order. A copied native solver capture
preserves its full prefix and supplies 16 impulse applications; the old SIMD
multiplication differs in eight, while scalar multiplication matches all 16
(`mesh-inertia-frame87`). GPU contact inertia/application now selects the
native solver family in warm start, normal/friction/rolling impulses and
restitution. All 19 precision tests pass on both ordinary and cached paths.
`ragdoll-mesh-inertia[-native]` full gates agree byte-for-byte for all six GPU
fixtures; setup, stability and collision-free checks pass. Visual agreement
still fails from frame 106. Frame 143 position RMS/max are 0.0326851/0.188760 m;
frame 600 RMS/max are 0.153748/0.443927 m, with angular RMS/max
36.2108/161.885 degrees. Poses/velocities still first differ at frame 87.
The new `post-mesh-inertia-frame87` trace preserves both 9,856-row prefixes and
all 6,578 operation keys. The first two substeps now match completely; the first
difference moves from the first biased solve to the third mesh warm start
(record 3,765, stage 42/color 22/body 8/occurrence 2): angular y differs by
9.53674e-7, with all other captured components equal. The focused `mesh-warm-frame87` capture preserves both production prefixes and
records all 20 warm-start stages plus 20 rolling-cache records without overflow.
Normal/central-friction/twist stages match through the third warm start;
reapplying rolling resistance first changes angular velocity. Cached rolling
impulses already differ on the second substep (CPU y/z
0.0822351575/-0.642325759 versus GPU 0.0822351500/-0.642325699), while all other
cached impulses match. Native scalar mesh rolling prepares a full inverse,
whereas GPU still used a direct solve; its clamp also used approximate built-in
division/square root. The mesh/overflow path now multiplies by the native-order
full inverse and uses corrected scalar division/square root without the SIMD
convex denominator epsilon. Four native mass and four clamp fixtures captured
from the unchanged CPU run pass, with all 20 precision tests green on ordinary/cached paths
(`mesh-rolling-frame87`). Full `ragdoll-mesh-rolling[-native]` gates produce identical GPU output across all
six fixtures; setup, stability and collision-free checks pass. Visual agreement
still fails from frame 106. Position RMS/max are 0.0326911/0.188813 m at frame
143 and 0.153882/0.442982 m at frame 600; final angular RMS/max are
36.1257/161.843 degrees. The `post-mesh-rolling-frame87` trace preserves both
production prefixes and all 6,578 keys. Only three records now differ (down
from 261): the first is the last relaxed spherical neck/head solve,
record 5,928, stage 110/color 0/native body 5/occurrence 3, angular x differing
by 1.90735e-6. Body 6 differs immediately after, followed by the next input for
body 5. All earlier contact solves and warm starts match.

A separate CCD capture (`ccd-frame87`) preserves both production prefixes
against the inertia baseline. For native body 22 / GPU handle 23, the centers
entering CCD match, but impact fraction is CPU 0.737244666 versus GPU
0.73724437. The other seven ragdolls' initial frame-87 differences are pose-only,
with velocities matching. The host CCD correction uses world-space `glam` lerp
and normalizes both quaternion endpoints, while native uses a recentered sweep
and normalizes only the final blend. The TOI query also uses the normalized
endpoint interpolation through `query_toi.rs`. This is a separate discrepancy
to investigate. A same-input replay of 15 actual native CCD hits shows that
this final pose formula alone differs on 13 cases; matching native scalar
interpolation and normalizing only the final quaternion blend matches all 15
bit-for-bit. The host final correction now uses that formula, with captured
regression fixtures (`src/fixtures/ccd_pose.rs`), passing on ordinary and native
cached builds. The TOI fraction calculation
is unchanged. Full `ragdoll-ccd-pose[-native]` runs agree for all six GPU fixtures
and preserve setup, stability and collision-free checks, but still fail visual
agreement from frame 106. Frame 143 position RMS/max are 0.0326855/0.188746 m;
frame 600 RMS/max are 0.153972/0.442960 m, with angular RMS/max
36.2229/161.815 degrees. This is a local precision correction, not acceptance. The
19 existing CCD regression tests also pass (`ccd-frame87/regressions.log`).
The separate `toi-frame87` CPU wrapper captures 48 native TOI calls at frame 87
without changing its 9,856-row state prefix. Replaying its 19 positive hits with
COM-centered proxy geometry still leaves small fraction differences; changing
quaternion interpolation alone does not eliminate them. This diagnostic does
not yet reproduce the full native sweep representation or the host path's
world-space triangle setup, so it is not a production fix or exact-step proof.
The temporary query interpolation experiment has been restored.

The completed `neck-frame87` trace retains both rolling-baseline state prefixes
and all 96 operation keys. The last relaxed cone-limit solve is the first
mismatching neck stage; spring, motor and twist stages match. The finer `cone-frame87` capture preserves both CCD-pose-baseline prefixes and
all 112 records without overflow. All 16 recorded cone inputs match in the
first seven solves. In the last relaxed solve only the velocity dot differs:
CPU -41.8785972595 versus GPU -41.8786010742, with axes, incoming velocities,
swing mass, cached impulse, cone error and bias equal. The cone velocity now
uses explicit scalar dot addition order. Separately, an eight-case native
swing-angle replay fails with built-in vector length (first case differs by
one ULP); scalar corrected square roots match native `b3GetSwingAngle`.
These angle and velocity fixtures pass with all 21 precision tests on ordinary
and cached paths (`cone-frame87/precision[-native].log`). Full independent
`ragdoll-spherical-cone[-native]` runs produce byte-identical results for all six
GPU fixtures. Joint setup, stability and collision-free precision still pass.
The first velocity difference advances to frame 88; all 112 bodies' linear and
angular velocities match through frame 87. Post-initialization poses first
differ at frame 87, consistent with the independently observed CCD mismatch.
Visual agreement still fails from frame 106. Position RMS/max are
0.0326891/0.188769 m at frame 143 and 0.153998/0.442830 m at frame 600; final
angular RMS/max are 36.2208/161.791 degrees. The complete `post-spherical-cone-frame87` trace preserves both production
prefixes and matches all 6,578 CPU/GPU operation records across all 112 bodies,
with no missing/extra keys or overflow. This verifies intermediate solver
states as well as final velocities. Offset Kinematic, Prismatic and both
ordinary/cached strict dragging pass on this version (`*-spherical-cone*`);
drag maxima remain 0.00602796 m position, 0.00597248 quaternion chord and
0.230405 m/s velocity.

A follow-up TOI diagnostic restores native sweep semantics: retain original
proxy vertices, interpolate centers of mass in the shifted frame, then subtract
the rotated local center. With native quaternion interpolation this makes 7 of
19 native impact captures bit-exact, reducing maximum fraction error from
1.40071e-6 to 1.78814e-7 (`toi-frame87/replay-summary.json`). Quaternion
interpolation alone matches zero captures. The production mesh CCD path still
uses COM-shifted vertices and world-space triangles; the native-sweep experiment
is fully restored and has not been applied as a production fix. Remaining work
is to integrate faithful sweep representation and isolate residual TOI math,
then rerun independent trajectories. The visual goal remains unmet.

The TOI follow-up now identifies the remaining same-input rounding: native GJK
uses a quaternion-derived matrix to transform support points, while separating
axes use the scalar quaternion rotation formula. Matching both forms, native
COM sweeps, and quaternion interpolation reproduces all 19 captured positive
impact fractions bit-for-bit (`toi-frame87/replay-matrix-sweeps.log`). A native
standalone replay reproduces the original CPU captures exactly, including the
two previously mismatching intermediate hits. Production mesh CCD now retains
original proxy vertices, recenters before sweep construction, and follows those
native transform operations. All 19 capture fixtures pass on ordinary/cached
builds (`src/fixtures/toi_sweeps.rs`); all 19 existing CCD tests pass.

Full `ragdoll-ccd-sweeps[-native]` runs agree for all six GPU fixtures and pass
setup, stability and collision-free checks. All frame-87 rotations and velocities
now match, with remaining position differences. Visual agreement still fails
from frame 106. Position RMS/max are 0.0326771/0.188721 m at frame 143 and
0.153599/0.447999 m at frame 600; angular RMS/max are 36.2567/161.773 degrees.
The `ccd-origin-frame87` host capture preserves the production output prefix.
Eleven of 15 accepted impact fractions match; the other four are queries whose
native interval was clipped by a preceding triangle hit. The native C callback
uses BVH traversal order and shortens its maximum fraction after each closer
hit. Mesh CCD now preserves that order and interval for closest-hit queries;
sensor/pre-solve continuation keeps the full candidate list. A two-triangle
native capture checks exact clipping and deliberately reversed source IDs.
Both new tests pass on ordinary and native cached builds, and all 18 query
regression tests pass. `ragdoll-ccd-interval[-native]` full runs agree on all six
GPU fixtures and retain setup/stability/collision-free passes. Visual agreement
still fails from frame 106. Position RMS/max are 0.0326734/0.188721 m at frame
143 and 0.154135/0.447999 m at frame 600; final angular RMS/max are
35.3237/138.794 degrees.
A separate residual remains: native CCD stores the origin and COM independently
(`base + (relativeCOM - rotatedLocalCenter)` versus `base + relativeCOM`).
Deriving the origin again from the rounded world COM loses precision. The
capture records both formulas without changing state. At that checkpoint the
production origin representation was unchanged. The post-clipping host capture
`ccd-origin-clipped-frame87` preserves its complete production prefix and
matches all 15 native accepted impact fractions, COMs, and quaternions exactly.
Computing the origin in native order matches all 15 CPU origins; deriving it
from rounded world COM matches only three. This isolates the remaining
frame-87 discrepancy to origin storage, with no need to alter correct COM state.
The retained-origin implementation now stores the separately computed origin
with a validity flag alongside COM state. Collision geometry, relative contact
frames, host queries/events, pose snapshots and scene rendering consume it.
Applying a new dynamic/kinematic pose clears it; sleeping/static bodies retain
it, mass changes preserve it, and explicit transforms invalidate it. The hot
state grows from 96 to 112 bytes and the export record from 160 to 176 bytes;
CCD shaders, readback, viewer bridge ABI 2 and rendering use the new stride.
Historical 144/160-byte oracle dumps remain readable. The native-origin capture
regression passes all 15 cases while proving that COM reconstruction loses
precision for 12. GPU state readback and sleep/movement lifecycle pass across
65 slots on ordinary/native cached paths; API mass/teleport lifecycle also
passes on both, and CCD motion classification passes. The
CPU viewer bridge still exactly matches three native scenes through 121 frames.
`cargo check --lib --bins --tests` passes. The broader all-target check exposes
existing experimental-example API errors (u64 edge keys, obsolete
`pick_adapter`, and private `GpuDevice` fields); it does not pass.

The independent ordinary/native runs in `ragdoll-retained-origin-final[-native]`
produce identical outputs for all six GPU fixtures and match all
112 CPU bodies exactly from frames 1 through 87. The next position, orientation
and velocity differences begin at frame 88; its position RMS/max are
4.53557e-8/4.80000e-7 m. This fixes the identified first-impact origin error,
but visual acceptance still first fails at frame 106. Frame 143 position
RMS/max are 0.0326878/0.188820 m; frame 600 is 0.154499/0.443658 m, with
35.3235/139.400 degrees angular RMS/max. These later results do not satisfy
the goal. Frame-88 solver tracing is the next diagnosis. Offset Kinematic and Prismatic regressions pass. Ordinary/native cached strict dragging
still has maxima of 0.00602796 m position, 0.00597248 quaternion chord and
0.230405 m/s velocity. Rain, performance (including the larger hot state), and synchronized comparison
recordings remain unqualified.
The earlier `ragdoll-retained-origin` trial predates the final anchor/CCD layout
updates and is diagnostic only.

The frame-88 follow-up (`retained-origin-frame88`) compares 7,540 native/GPU
solver records across all 112 bodies, with identical operation keys and exact
9,968-row production prefixes. Native diagnostic emission now respects each
convex/mesh block's contact family and index range; emitting every contact of
a color for both block types had duplicated unrelated bodies in the trace.
The first state difference is the biased convex contact between native bodies
23/22 (the second ragdoll's right thigh/left calf). Its incoming velocities,
normal impulse, and inverse inertia matrices match. The contact normal and one
anchor component already differ before the impulse, accounting for the angular
velocity difference (`contact22-frame88`). Both instrumented trajectories still
reproduce production exactly.

An isolated replay of that contact from native capsule/relative-transform
inputs reproduces the normal mismatch: native local normal Y is
-0.0040961164 versus GPU -0.004096111. Changing dot-product order alone does
not fix it. `closest_segments` now uses native scalar dot order and corrected
rounded division for segment fractions, matching `b3SegmentDistance`. All 18
independent native one-point capsule captures at frame 88 then match their
fractions, local normal, point, and separation exactly. All 22 native precision
tests pass on ordinary/native cached builds. `ragdoll-capsule-segment[-native]`
produces identical outputs for all six GPU fixtures. Positions match all 112
CPU bodies exactly through frame 88, and the first position difference is now
frame 89. Rotation/velocity still first differ in the neck at frame 88, whose
maximum angular error falls to 1.78907e-6 degrees. Setup, joint stability, and
collision-free precision pass. The visual limits still first fail at frame 106;
frame 143 position RMS/max are 0.0326834/0.188772 m, and frame 600 is
0.154490/0.446255 m with angular RMS/max 35.3955/140.175 degrees. This is a
verified local arithmetic fix, not a pass of the 600-frame goal. Offset,
Prismatic and ordinary/native strict dragging pass on this version; drag maxima
remain 0.00602796 m, 0.00597248 quaternion chord and 0.230405 m/s.
The subsequent frame-88 trace (`post-capsule-segment-frame88`) preserves both
9,968-row production prefixes and all 7,540 operation keys. Its first difference
is the head's angular velocity entering the third substep's relaxed neck solve.
The incoming velocity before position integration matches; speed limiting
changes native X to 45.1419029236 versus GPU 45.1419067383. Integration now uses
the native multiplication by rounded inverse timestep, scalar dot order, and
rounded square root/division for speed clamping. All 15 captured native angular
clamp transitions match exactly, including the previously failing head case;
all 23 native precision tests pass on ordinary and native cached paths.
Independent `ragdoll-speed-cap[-native]` runs have identical GPU outputs for all
six fixtures and again pass the 112-joint/3,672-field setup audit, joint stability,
and collision-free checks. Position and rotation now match through frame 88;
the first remaining velocity difference is frame 88, scene body 71. The visual
gate still fails first at frame 106. Frame 143 position RMS/max are
0.0326820/0.188771 m; frame 600 is 0.154411/0.444576 m, with angular RMS/max
35.2959/139.554 degrees. This removes a demonstrated arithmetic discrepancy
without establishing the requested visual agreement. Evidence is retained in
`artifacts/ragdoll-impact-goal/speed-cap-frame88`.
Offset Kinematic, Prismatic, and the 12-case speed-limit reference gate pass on
this version (`offset-speed-cap`, `prismatic-speed-cap`, and
`speed-limit-native-arithmetic`). Ordinary/native strict dragging also passes
on the clamp version (`drag-speed-cap[-native]`), with unchanged maxima of
0.00602796 m, 0.00597248 quaternion chord, and 0.230405 m/s.

The next frame-88 trace (`post-speed-cap-frame88`) has identical production
prefixes and 7,540 matching operation keys, with only 15 differing records.
The first difference occurs in the sixth ragdoll's right shoulder, on the
fourth substep's relaxed solve (native bodies 76/85). A staged joint trace
(`shoulder-frame88`) places it in the twist limits: all incoming velocities
and motor outputs match; upper-arm angular Y first differs by 9.53674e-7.
Sixteen native lower/upper twist velocity captures reproduce the GPU dot
rounding mismatch in isolation. Both spherical twist-limit dot products now
use the scalar native accumulation order. All 24 precision tests pass on both
ordinary and native cached builds. Independent `ragdoll-spherical-twist[-native]`
runs have identical outputs for all six GPU fixtures. All body positions,
rotations, and velocities match through frame 89; positions remain exact
through frame 90. The first rotation/velocity difference is frame 90, scene
body 70. Setup, joint stability, and collision-free checks pass. The visual
gate still fails first at frame 106. Frame 143 position RMS/max are
0.0326785/0.188759 m; frame 600 is 0.154438/0.449159 m, with angular RMS/max
35.4478/139.481 degrees. The frame-92 jump remains about 1.472 mm maximum
position and 1.712 degrees maximum orientation, so this is another verified
local arithmetic correction, not acceptance of the visual goal.

The separate frame-92 trace (`speed-cap-frame92`) preserves both 10,416-row
production prefixes, but 156 of its 6,526 operation keys differ in color.
The first large state difference is the first biased contact solve on native
body 66, with angular Y differing by 1.43709 rad/s. The contact-lifetime audit
(`graph-frame92`) finds four pair-color differences at frame 92: CPU pairs
66/68 and 95/97 receive color 1, while GPU assigns color 2; CPU pairs 66/67 and
95/96 receive color 4, while GPU assigns color 1. CPU retires pairs 66/68 and
95/97 after frame 88 and recreates them by frame 91 with reused contact IDs;
GPU retains both throughout. GPU capsule broadphase bounds currently use a
surrounding sphere (`collider_aabb_extent`), unlike native endpoint-based
capsule AABBs. This is a concrete bounds/lifetime discrepancy to investigate;
the trace does not yet establish that correcting it alone satisfies the visual
goal. No contact colors or CPU simulation state are copied into production.

Capsule proxy bounds now transform the original endpoints, then inflate by
radius and speculative distance separately, matching `b3ComputeCapsuleAABB`.
Historical proxy-update commands pass their own origin/rotation to the same
helper; using the current body transform for those commands would lose their
history. All 224 native capsule captures from frames 88/92 match bit for bit.
The integration regression `capsule_proxy_retires_separated_parallel_pair`
checks actual pair retirement for capsules along all three axes, including a
teleport outside endpoint bounds but inside the old sphere bounds. The bounds
and retirement tests pass on ordinary and native cached backends; all 25
ordinary native-precision tests pass. Evidence is in
`artifacts/ragdoll-impact-goal/capsule-bounds`.

The follow-up `graph-capsule-bounds` trace confirms that pairs 66/68 and 95/97
now disappear at frames 89/90 and reappear at frame 91, matching native's
lifetime. Their graph colors still differ. The independent 600-frame trajectory
in `ragdoll-capsule-bounds-final` remains unchanged from `ragdoll-spherical-twist`,
so the bounds fix alone does not improve visual agreement. A diagnostic trial
sorting later graph insertions by GPU contact slot (`ragdoll-contact-slot-order`)
reduces frame-92 position RMS but introduces different ordering elsewhere;
frame-600 position RMS worsens from 0.154438 to 0.169085 m. That trial was removed.
GPU slots are not interchangeable with native's allocated/reused contact IDs.
There are still extra non-touching GPU pairs (96 at frame 85, 89 at frame 92),
so exact contact identity/lifetime remains unresolved before graph ordering can
be considered equivalent. These are independent runs without state copying.

The proxy-history trace (`fat-bounds-history`) identifies the remaining 2 cm
dynamic bound expansion during free fall. Native `b3SolveContinuous` stores an
unpadded end AABB when there is no impact; only its impact branch restores
speculative padding. GPU now records this distinction with `FLAG_CCD_NO_HIT`:
integration sets it for fast bodies, both CCD correction paths clear it on
impact, and explicit transforms clear it. Historical proxy commands always use
their normal padding. The hit/miss test exercises both convex and mesh targets;
the teleport regression verifies invalidation of the marker.

In `fat-bounds-no-hit`, dynamic bounds at GPU frame 85 versus native frame 84
(GPU refreshes before stepping, native after stepping) now differ by at most
9.53674e-7 m rather than 0.0200014 m. The independent trajectory prefix remains
unchanged. The contact audit `graph-no-hit` has exactly the same CPU/GPU pair
membership at every recorded frame 85–92, including all non-touching pairs:
140, 140, 156, 165, 242, 254, 230, and 197 pairs respectively. The four color
differences at frames 89 and 92 remain, isolating ordering from membership in
this window. Flat mesh proxies still have an extra 5 cm thickness from the host
minimum-half-extent clamp. The ordinary `ragdoll-ccd-proxy-padding` run preserves
setup/stability/collision-free checks and remains byte-identical to the prior
600-frame trajectory; it still fails the visual goal at frame 106. All 25
ordinary precision tests and the hit/miss and teleport tests pass. Native
`ragdoll-ccd-proxy-padding-native` produces identical GPU outputs for all six
fixtures, including the unchanged failing visual gate. Its isolated hit/miss
test also passes. The membership comparison checks all 1,524 shape-pair/frame
identities without duplicate pairs, not only body-level pair counts.

The `contact-id-history` audit extends exact pair membership to every frame
1–92. It logs 487 native creations and 290 destructions, with no contacts
created and destroyed within the same step; both diagnostic trajectories
remain exact production prefixes. Offline graph replay reproduces every CPU
color when transitions are interleaved in contact-ID order. Releasing stopped
contacts before adding new ones reproduces exactly the four frame-89 GPU
color discrepancies: native contact 33 starts before contact 35 releases color
1, so it receives color 2. At frame 92, reused IDs 151 and 89 put the returning
leg contacts before existing IDs 154 and 216. A LIFO logical-ID replay with
native initial creation order and pair-key order for subsequent allocations
also matches all colors through 92, despite 418 ID/frame differences. This is
diagnostic evidence for separating logical IDs from GPU storage slots and
preserving interleaved transitions, not proof of later allocation equivalence
or improved independent trajectories.

The following `ragdoll-logical-contact-order[-native]` implementation stores
logical contact IDs separately from physical manifold slots. It allocates
before retiring disjoint pairs, reuses freed IDs with a LIFO stack, and keeps
stopped edges occupied until their ordered transition. Initial creation uses
the existing native leaf-order metadata; subsequent allocation still uses pair
order and is not an exact native dynamic-tree implementation. The scratch
history survives scene-buffer growth and adds `8 * pair_capacity + 2` words
(about 2 MiB at the minimum capacity); performance remains unmeasured.

All six fixture outputs are byte-identical between ordinary and native cached
GPU paths. Setup, isolated joints, no-contact motion, and stability pass. At
frame 92, maximum position error falls from 1.471687 mm to 0.000470 mm and
maximum angle error from 1.711959 degrees to 0.000056684 degrees. The first
visual-limit failure moves from frame 106 to frame 125. At frame 143, position
RMS is 0.0294815 m and angle RMS 5.75144 degrees; at frame 600 they are
0.114680 m and 27.2258 degrees. The full visual gate still fails.

A native three-sphere reference confirms that a lower-ID ghost contact starting
before a higher-ID contact retires must receive color 1, even though color 0 is
free at step end. GPU transition, surviving-color, and buffer-growth regressions
pass on both paths; all 25 native-precision tests pass on both paths. The
ordinary sparse-joint-reuse invariant, Offset Kinematic, and Prismatic gates
also pass. Further explicit-mutation/history invalidation, Rain, performance,
and synchronized capture qualification remains outstanding.

The `logical-graph125` trace reproduces the new independent trajectory exactly.
CPU/GPU pair membership and colors match through frame 123. Position error
stays below 5 micrometres through frame 121, then jumps at frame 122 despite
matching graph colors. That impact needs manifold and solver-phase tracing;
the pair/color differences at frames 124–125 occur after the trajectory diverges.

The strict dragging gate caught an initialization regression in the first
history implementation: when entering the joint solver after jointless steps,
it failed to inherit the parallel graph's existing contact colors. Maximum
position error rose to 0.0191348 m and quaternion chord to 0.0115723, failing
the strict gate. The correction imports surviving colors on first entry or
resumption before assigning new joints. `jointed_contact_inherits_color_when_first_joint_is_added`
covers the case. Ordinary strict dragging returns to its previous maxima:
0.00602796 m position, 0.00597248 quaternion chord, and 0.230405 m/s velocity.
The corrected `ragdoll-logical-contact-order-inherit[-native]` reruns preserve
all six fixture trajectories exactly on both paths. All four contact-order
regressions and strict dragging pass on both paths with the same drag maxima.
Final evidence is in `artifacts/ragdoll-impact-goal/logical-contact-order`.
These corrected reruns are the authoritative baseline for further diagnosis.

The next `logical-frame122` probe preserves both independent trajectory prefixes
and matches all 6,526 solver-operation keys. Human 0 is exact through frame 121;
the first large frame-122 mismatch is the first biased ground solve for native
body 6 (head), with 0.104219 rad/s angular-component error. Native contact 277
has an additional edge patch on triangle 250. Captured local triangle/capsule
inputs show that flags 116 describe a non-flat edge, which the GPU rejected by
testing either directional concavity bit. Capsule terrain filtering now tests
the paired bits for flat seams. The captured-input regression fails before the
change and passes afterward.

The filter-only `ragdoll-capsule-terrain-edge[-native]` trial moves the first
visual failure to frame 146 but worsens final position RMS to 0.178562 m. The
`edge-manifold137` capture isolates why: both engines generate the same face and
edge patches for human 7's head, but GPU BVH order puts the edge first. Native
accepts triangle faces before tentative edge/vertex manifolds. Fresh capsule
patches now retain that priority and are stably partitioned before warm-start
transfer. A two-triangle fixture with forced edge-first visitation fails before
this ordering correction and passes afterward.

The combined `ragdoll-capsule-terrain-order[-native]` runs established the next
baseline. All six fixture outputs match byte-for-byte between backends; setup,
collision-free precision, and stability pass. All 26 precision tests and three
capsule-mesh regressions pass on both paths. At frame 143, position RMS/max is
0.00233723/0.0155341 m and angular RMS/max is 0.721920/3.75689 degrees, satisfying
all four visual targets at that frame. The first failure is frame 147. Frame
600 position RMS/max is 0.0702974/0.327442 m and angular RMS/max is
26.2981/168.050 degrees, so the complete goal remains unmet. Evidence is in
`artifacts/ragdoll-impact-goal/logical-frame122`.

The next large jump is frame 138, human 5's head. `terrain-manifold138` verifies
exact independent prefixes and shows a four-point native mesh manifold versus
three GPU points. Native retains the distinct witnesses from both triangles
along a shared seam; GPU proximity deduplication discards one. The native
standalone fixture in `mesh-seam-points` likewise keeps four points, ordering
the two extreme endpoints before the two coincident seam witnesses. GPU also
skips its reduction/order logic for four or fewer points.

Capsule mesh clusters now retain all raw triangle points in temporary contact
slots, then run the native projected-point selection with separation hysteresis
and swap-removal ordering. Reduction uses the original capsule-local witnesses
and triangle normal, before world-space anchor rounding. Temporary chains are
released after reduction or allocation failure. There is no fixed four/five-point
candidate limit, and coincident witnesses are retained both across triangles and
within a triangle. Other mesh shape reducers are unchanged. The two-triangle
seam and sixteen-triangle strip regressions match independent native point order;
the latter catches within-triangle deduplication as well as streaming reduction.

`ragdoll-capsule-cluster-complete[-native]` established the next independent baseline.
All six GPU fixture trajectories are byte-identical between paths. Joint setup,
collision-free precision, and stability pass. All 26 native precision tests and
five capsule-mesh regressions pass on both paths. The first visual failure moves
from frame 147 to 175. At frame 138 the previous 3.59 mm position jump is gone:
maximum position error is 0.0106 mm. Frame 143 position RMS/max is
0.000357764/0.00263717 m and orientation RMS/max is 0.117842/0.669121 degrees.
Frame 600 position RMS/max is 0.0176641/0.0661148 m and orientation RMS/max is
4.95815/21.6426 degrees, so the full goal is still unmet. Evidence and regression
logs are in `artifacts/ragdoll-impact-goal/mesh-seam-points`.

The next `terrain-manifold139` public contact capture preserves both independent
15,680-row trajectory prefixes. Body 83 has four CPU points but only two GPU
points. The missing face has positive raw separations above the generic 0.02 m
point cutoff. Native triangle/capsule collision accepts a shallow clipped face
after its closest-distance test without applying that second face-separation
cutoff. The `cpu-triangle` linker-wrapper capture preserves the CPU prefix and
records two accepted faces whose nearest raw separations are 0.0234374 and
0.0235441 m. The captured-input regression fails with no GPU manifold. Capsule
admission now keeps the closest-distance decision instead of applying a second
cutoff to the clipped face. Both captured cases then pass, including native
normals and anchors. This retains the existing capsule speculative-distance
test and does not change scene parameters or tolerances.

`ragdoll-capsule-face-admission[-native]` is the current independent baseline.
The six fixture trajectories match byte-for-byte between GPU paths; setup,
collision-free precision and stability pass. All six capsule-mesh regressions
and 26 native precision tests pass on both paths. Offset Kinematic and Prismatic
pass their original tolerances, and ordinary/native strict ground dragging
retain maxima of 0.00602796 m position, 0.00597248 quaternion chord and 0.230405
m/s velocity. Frame 143 position RMS/max is 0.00000278366/0.0000113157 m and
orientation RMS/max is 0.000736202/0.00353445 degrees. The first large jump is
now frame 167; maximum position error through frame 166 is below 0.028 mm.
The first visual failure remains frame 175. Frame 600 position RMS/max is
0.0157052/0.0634084 m and orientation RMS/max is 4.82960/22.2163 degrees, so
the goal remains unmet. Evidence is in `capsule-face-admission` under the impact
goal artifacts.

The `terrain-manifold167` and `logical-graph167` probes each preserve both
independent 18,816-row trajectory prefixes. After canonicalizing endpoint
orientation, all 79 public contact-patch records have matching counts, with
maximum normal-component error 0.000122159. Six awake arm contact colors differ
at frame 167: shape pairs (41,46), (42,46), (71,76), (72,76), (101,106), and
(102,106). Earlier graph differences include sleeping contacts and extra GPU
broadphase pairs from frame 163; their contribution to logical-ID allocation
and start/stop ordering still needs diagnosis. No graph correction is inferred
from the public contact comparison alone. Rain, performance, synchronized
captures, broader final regressions, and the logical-contact history mutation
audit remain pending; no completion or commit is claimed.

`logical-history167` reads the actual GPU logical-ID map without changing the
independent trajectory (18,816 rows verified). IDs agree through frame 17. At
frame 18, native creates shape pair (19,25) before (19,23), assigning IDs 235
and 65; GPU pair-key order assigns those IDs in reverse. Later allocation and
retirement propagates this difference. At frame 167 the six arm pairs above
have CPU IDs 25,228,164,160,80,221 versus GPU IDs 246,202,140,221,239,231.
An offline allocator replay with native creation order reproduces every native
ID through frame 167 when supplied native pair membership. With GPU membership
it first differs at frame 164, yet still recovers all six expected arm IDs at
167. This is diagnostic evidence, not a production ordering override or a new
accepted trajectory.

The same artifact directory contains `cpu-tree.log`, a native capture of the
112 initial dynamic proxies, 1,264 proxy enlargements, 16 partial rebuilds,
moved-proxy lists and query leaf order through frame 18. Its 2,128-row trajectory
prefix is unchanged. Query order for shapes 23/25 reverses between the initial
tree and frame 18; native prepends query results before creating contacts.
The existing Rust ordering tree supplies only initial insertion metadata,
and subsequent GPU allocations use pair-key order. Matching dynamic creation
order therefore requires tracking proxy updates, partial rebuilds, moved-proxy
order and reversed query traversal, rather than reusing the initial leaf ranks.
Diagnostic source edits are restored and the production library is rebuilt;
the authoritative numerical baseline remains `ragdoll-capsule-face-admission`.

The Rust ordering model now includes proxy enlargement, partial rebuilds that
retain untouched subtrees, native longest-axis/Hoare partitioning and internal
node reuse. `enlarged_tree_order_matches_native_partial_rebuilds` matches every
query traversal through frame 167: 112 initial proxies, 17,282 enlargements and
165 rebuilds. The versioned binary fixture is test-only; regenerate it with
`bash scripts/capture-broadphase-order.sh` and compare the artifact's
`broadphase_tree_updates.bin` with `src/fixtures/broadphase_tree_updates.bin`.
The checked regeneration is byte-identical and preserves all 18,816 independent
CPU trajectory rows. The existing insertion/rotation regression also passes.
The WGSL implementation in `shaders/physics/contact_order.wgsl` now passes the
same 167-query enlargement/rebuild fixture on ordinary and native cached GPU
paths. Its persistent node format can be seeded from the engine's own initial
geometry; tree traversal, enlargement and partial rebuilds execute on the GPU.

The creation comparator is separately checked against all 1,113 actual native
contact creations through frame 167 (`src/fixtures/broadphase_pair_order.bin`,
regenerated by the same capture script). Priority uses the owning proxy's
moved-list position, reversed tree-query type order, and reversed leaf rank.
For two moved dynamic proxies the smaller proxy key owns the pair; for a
cross-type pair a moved dynamic proxy takes priority. This matters when only
the higher-key endpoint moved, and cannot be replaced by sorting shape IDs.
The fixture covers 827 pairs with both dynamic endpoints moved, 154 with only
one moved, and 132 static/dynamic pairs. The GPU test also reverses every pair's
endpoints and requires identical priority. All five selected contact tests,
including tree evolution and creation priority, pass on ordinary and native
cached paths. The captured native trajectory is
byte-identical to all 18,816 prior reference rows. See
`artifacts/ragdoll-impact-goal/pair-order167` for results.

The live ordering path seeds three trees from this
engine's own scene geometry, updates them from GPU fat bounds, saves query
ranks before partial rebuilds, and uses the priority comparator for logical
contact-ID allocation. Tree storage survives scratch growth and same-topology
capacity growth; topology changes explicitly disable this incomplete path.
Proxy insertion/removal and explicit movement/reinsertion use the previous
contact path; the ordering cache is explicitly disabled on those changes.
Native awake-set ordering is not yet exact. The current
body-ID move order first differs from native at frame 226, verified by a
600-step native metadata trace with unchanged simulation output. The fixtures
and native metadata remain test-only, never runtime ordering overrides.

Independent `ragdoll-live-contact-order[-native]` runs produce byte-identical
GPU traces across all six fixtures. Joint setup, stability and collision-free
precision pass (9e-7 m maximum no-contact position error). At frame 600,
position RMS/max are 0.0104881814/0.0459093661 m and orientation RMS/max are
3.30864022/10.1754789 degrees. This improves the preceding baseline but fails
the original acceptance targets: 424 frames fail, starting at 177. The first
impact jump still occurs at 167 (0.00231470 m maximum position error), and the
frame-143/166 results remain unchanged. See
`artifacts/ragdoll-impact-goal/live-order` for logs, source hashes and remaining
work. Those first integration runs predate the flat-mesh correction below.

The live probe reproduces every native proxy key, moved-list rank and reversed
tree rank through frame 167, with all 18,816 GPU trajectory rows unchanged.
Logical IDs now match until frame 163, where the GPU admits three extra pairs:
(1,16), (44,61), and (91,104). All involve planar terrain. Mesh/height-field
construction clamped each half-extent to at least 0.05 m, giving flat terrain
a fat upper Y bound of 0.09 m instead of native 0.04 m. The three dynamic lower
bounds are 0.0603083, 0.0734217 and 0.0603356 m. These false broadphase contacts
consume logical IDs and change the subsequent arm-contact ordering.

Mesh and height-field bounds now preserve zero extents; the existing
speculative and static margins provide the native padding. The regression
`flat_mesh_does_not_create_pairs_above_native_fat_bounds` rejects the false
pair and still admits a real fat-bound overlap. With live ordering enabled,
`ragdoll-live-order-flat-mesh[-native]` passes every original visual threshold
over all 600 steps/112 bodies, with byte-identical GPU traces across both paths.
The control with only the mesh correction (`ragdoll-flat-mesh-only`) fails
starting at frame 178, confirming that both changes are needed.

Two ordering configurations are available through the existing process option:
`GPU_PHYSICS_LIVE_CONTACT_ORDER=1` requests CPU-compatible live ordering;
`GPU_PHYSICS_LIVE_CONTACT_ORDER=0` uses the GPU ordering path. The current default
remains CPU-compatible while the original CPU-agreement qualification is retained.
This is an ordering choice, not a determinism-versus-speed guarantee. Both modes
retain the collision, joint, mass/inertia and flat-terrain fixes.

The GPU-order checks in `artifacts/ragdoll-gpu-order` run the current ordinary
and native cached binaries three times each for 600 steps. All six complete
traces are byte-identical on the tested NVIDIA device, including across the two
paths. The unchanged collision-free, setup and joint-stability gates pass:
maximum no-contact position error is 9e-7 m, peak joint separation 0.213310137 m,
tail separation 0.001389399 m, and tail linear/angular speeds are zero. This is
fixture-level repeatability evidence, not a proof for every scene, driver or GPU.

GPU ordering intentionally does not reproduce the CPU trajectory: the unchanged
CPU-agreement validator fails first at frame 178, with worst all-frame position
RMS/max 0.0599098/0.458968 m and orientation RMS/max 15.0302/92.7230 degrees.
The validator's nonzero exit is retained; no tolerance is relaxed. These measured
trajectory differences coexist with passing stability checks and repeatability.
They must not be used to excuse other collision or constraint defects.

Live ordering is now enabled by default for supported initial jointed scenes;
`GPU_PHYSICS_LIVE_CONTACT_ORDER=0` is the diagnostic opt-out. The default runs
`ragdoll-contact-order-default[-native]` also pass with identical traces.
Worst position RMS/max over the entire run are 0.00238558/0.0149234 m, and worst
orientation RMS/max are 0.764070/4.34194 degrees. The frame-167 jump is removed:
maximum position error there is 0.0000290707 m. Joint setup, stability and
9e-7 m collision-free precision remain intact. Offset and Prismatic pass their
unchanged reference tolerances. Ordinary and cached strict dragging retain
their previous maxima: 0.00602796 m position, 0.00597248 quaternion chord and
0.230405 m/s velocity. Adding an initial joint after a jointless GPU upload
now reallocates ordering storage, with a passing regression for that sequence.
The final independent runs (`ragdoll-contact-order-final[-native]`) preserve
these results. Ordering-storage lifetime tests pass on both paths: capacity
growth retains persistent tree state, while explicit teleport/removal invalidates
it. The 28 native-precision and six capsule/mesh tests also pass on both paths.
All production-source hashes in the final ragdoll manifests still match.

Five interleaved warm 600-step fixture runs per binary measure median process
wall times of 5.803 s for the frozen `ragdoll-capsule-face-admission/gpu` baseline
and 7.671 s for `ragdoll-contact-order-final/gpu`: a 32.2% increase. This includes
startup, stepping, pose readback and output formatting, with no renderer; it is
not GPU-kernel timing or a measurement isolating only contact ordering. One
warmup per binary precedes the measured runs, and no viewer/build jobs overlap
this measurement. Binary hashes and individual timings are recorded in
`artifacts/ragdoll-impact-goal/qualification/performance.json`.
A subsequent same-binary comparison measures 5.752 s with live ordering disabled
and 6.666 s enabled (15.9% overhead), using five interleaved warm measurements
per setting. See `ordering-performance.json` in the same directory. This includes
trajectory/contact-work changes caused by the ordering choice, not only the
ordering kernel. It must not be subtracted from the earlier 32.2% figure as an
exact attribution across different runs and baselines.
Clean synchronized recordings are complete for Offset (300 paired frames),
Falling Ragdolls (600), and Rain (120); reports, binary identity and visual
inspection images are in `artifacts/ragdoll-parity-capture-final`. Each CPU/GPU
clip pair is cropped from one recording and has identical video frame counts
and duration. The ragdoll drop, impacts and settling views closely align. Rain
is a short visual record, not a claim of trajectory parity; its complete
600-step health qualification passes, with the CPU-relative limitations below.

The final Rain run (`artifacts/rain-contact-order`) exits zero after all 600
frames, with zero Sokol errors. Complete records, finite/runaway checks, the
4,300-body peak population and 20 spawn/replacement events pass. GPU peak speed
is 17.6709 m/s and peak constrained-anchor error is 0.325545698 m; the independent
CPU run also passes health checks. Rendering uses software OpenGL at 640×360;
physics defaults and the 600-step schedule are unchanged, and these timings
are not performance evidence.

Rain does **not** pass CPU-relative screening: 376 frames fail, comprising 13
anchor-error failures, 83 angular-error failures and 280 joint-identity
mismatches after recycling begins. The first failure is frame 170 (step 171),
joint 681: anchor error 0.0855421 m versus CPU 0.00176341 m. Identity mismatches
start at frame 320; the reports lack joint endpoints, so those IDs cannot be
assumed to represent corresponding joints. The earlier `rain-mesh-raw-separation`
run failed 377 frames, first at 107, with the same 280 identity mismatches.
This establishes completed health qualification, not Rain trajectory parity
or a guarantee that every individual Rain residual improved. The 112-body
Falling Ragdolls acceptance gate remains a separate, every-frame pass.

Offset Kinematic, Prismatic and ordinary/cached strict dragging pass for the
interval version (`*-ccd-interval*`). Drag maxima remain 0.00602796 m position,
0.00597248 quaternion chord and 0.230405 m/s velocity. Rain/performance/synchronized captures remain outstanding
for the final implementation; no acceptance or commit is claimed.
 Offset/Prismatic and ordinary/cached
strict dragging pass on the rolling version (`*-mesh-rolling*`), with maximum
drag position difference still 0.00602796 m. Offset Kinematic, all Prismatic
fixtures, and ordinary/cached strict dragging pass on this inertia-family
version (`*-mesh-inertia*`), with unchanged drag maxima. Offset Kinematic, Prismatic and ordinary/cached strict dragging pass on the
`mesh-raw-separation` version (drag maxima unchanged at 0.00602796 m,
0.00597248 quaternion chord and 0.230405 m/s). Those runs predate the subsequent
inertia-family correction. The 600-step Rain run in
`artifacts/rain-mesh-raw-separation` completed with application exit 0 and passes
finite/runaway and population/recycling checks (4,300 peak bodies, peak speed
17.6708 m/s and peak anchor error 0.325545192 m). Its CPU-relative screening fails on 377 frames, first at frame 107
(step 108), joint 63 angular error 0.286632 versus CPU 0.157644 radians.
As in the previous Rain run, post-recycling joint-ID mismatches need identity
reconciliation and are not evidence of corresponding-joint error. It predates the inertia change and uses software rendering,
so its timings are not performance evidence. The earlier
`ragdoll-mesh-base-order[-native]` runs retain the incomplete dot-order-only
experiment and are not final-code qualification.

The refreshed combined-view drag gate currently regresses
(`artifacts/drag-spherical-point-velocity.log`): isolated ten-drag comparison
passes exactly, but ground-contact drags fail with 1.33467 m maximum position
error, 1.41324 quaternion chord, and 3.10393 m/s velocity error. This contradicts
the older 6 mm qualification and must be resolved; that older pass is not current
acceptance evidence. The archived binary rerun passes under the same runtime
settings, reproducing its 0.00602796 m maximum position error and strict velocity
pass (`artifacts/drag-spherical-point-velocity/previous-binary-strict.log`).
This confirms a code regression. Initial cube mass and all nine world inverse-inertia components match CPU exactly (`box-mass-*.txt` in that artifact directory), ruling out a simple mass-setup difference.
The Rain check uses 600 default
steps with software OpenGL rendering, so its timings are not performance data.
CPU Rain completes all 600 frames, reaches 4,300 bodies, and exercises 20 column
creation/replacement events. Every-frame population counts, complete body/joint
records and existing finite/runaway checks pass (`cpu-health-result.json` in
`artifacts/rain-spherical-point-velocity`). The frozen GPU run also produces
all 600 frames, 4,300 peak bodies and 20 spawn/replacement events, with complete
records and finite/runaway checks passing. GPU peak speed is 17.6709 m/s and
peak constrained-anchor error is 0.325960368 m. Its launcher returned 143 even
though the file reports `status: ok` and the log ends with 600 frames and zero
Sokol errors; treat this as completed diagnostic data, not clean process
qualification. CPU-relative screening checks all 600 frames and fails 374 of them; its first
failure is frame 107 (submitted step 108), joint 63 angular error 0.286496 rad
versus CPU 0.157644 rad, beyond the existing extra 0.05-rad allowance. Anchor
error failures follow (e.g. frame 154, joint 51: 0.0944229 m versus 0.00181863 m).
The diagnostic comparator initially treated `classify`'s `pass` return as a
failure; the result is corrected by retaining only its actual non-pass results.
This frozen run predates
the subsequent hinge and persistent-color fixes, so latest-code Rain remains
unqualified. The subsequent persistent-color run completes all 600 frames
with application exit zero, captured separately from the Xvfb launcher
(`artifacts/rain-persistent-joint-colors`). Population/recycling and finite/runaway
checks pass: 4,300 peak bodies, 17.6704 m/s peak speed and 0.325534612 m peak
constrained-anchor error. CPU-relative screening still fails 381 frames:
80 angular-error checks, 21 anchor-error checks and 280 joint-ID-set checks
starting at frame 320 when recycling begins. At that first replacement, both
engines retain 4,200 joints, but CPU reuses IDs 1–420 while GPU allocates
4201–4620 (`joint-id-recycling-audit.json`); ID equality alone cannot establish
physical correspondence after recycling. The first physical
failure remains frame 107, joint 63 angular error 0.286522 versus CPU 0.157644 rad.
This binary predates the axial-hinge and convex-rolling corrections; it is
neither latest-code Rain qualification nor performance evidence.
The first cube's drag trace diverges at frame 60 in current code, whereas its
archived counterpart remains float32-exact until frame 1,560. At frame 60 the
contact geometry and feature IDs match but impulses differ. The ordinary GPU
path reproduces the same ten-drag failure and error maxima
(`artifacts/drag-revolute-point-ordinary.log`), ruling out a native-cache-only
regression. The motor/contact phase capture around frames 60–61 preserves all
6,120 first-cube body-state rows byte-for-byte against the frozen production
run. Contact diagnostic output differs during sleep, but frame-60 contact rows
match the baseline. Preparation, velocity integration, and warm start have
exact CPU/GPU velocities; the first differing completed phase is the biased
solve (maximum velocity/angular-velocity component difference 3.0649e-7).
The expanded `phase-capture,mesh-candidates` run also preserves all 6,120
production body-state rows and has no trace overflow. Its first GPU biased
contact input equals the CPU's pre-motor velocity, whereas the CPU executes
the motor before that contact. This exposes a constraint-order discrepancy,
not merely a motor arithmetic discrepancy. `compact_joint_components` assigns
joint colors before reserving surviving contact colors; adding a drag joint
can therefore take the existing ground contact's color and reverse priority.
The graph builder now preserves assigned joint colors, reserves surviving
contact colors, and only then admits new joints and newly touching contacts.
An internal joint flag records whether a color has been assigned; normal joint
readback preserves it. The new GPU regression adds two anchored joints to an
existing contact and removes the first, verifying surviving colors throughout;
it passes on ordinary and native cached paths. Ordinary pointer/isolated-drag
checks and strict ten-ground-drag checks pass again
(`artifacts/drag-persistent-joint-colors*`), reproducing the archived baseline:
0.00602796 m maximum position error, 0.00597248 quaternion chord and 0.230405 m/s
peak velocity difference. Cached-path pointer/isolated/ground drag reproduces
the same passing maxima. The ordinary independent ragdoll run is byte-identical
across all six fixtures to the preceding alignment-velocity baseline; its
visual failure is unchanged. The cached-path runner ended with status 143 after writing a complete,
byte-identical 600-frame ragdoll trace. Its missing collision-free fixture was
run separately with the same frozen binary and exited zero. All six resulting
GPU fixture traces match the ordinary path byte-for-byte; setup, stability and
collision-free checks pass, while visual agreement still fails from frame 90.

Offset Kinematic passes its 1e-5/300-step gate with the motor-mass, clamp, and
branch-free square-root corrections (`artifacts/offset-motor-clamp`). All five
Prismatic fixtures also pass their 1e-4/120-step gate with these corrections
(`artifacts/prismatic-motor-clamp`). Both gates also pass after the hinge
alignment correction (`artifacts/offset-revolute-perp-mass` and
`artifacts/prismatic-revolute-perp-mass`). They also pass after persistent joint
coloring (`artifacts/offset-persistent-joint-colors` and
`artifacts/prismatic-persistent-joint-colors`). Latest-code Rain and performance
measurements remain pending. No visual-agreement completion is claimed.

The Offset Kinematic regression checks explicit zero-mass center overrides on
static and kinematic bodies. `b3Body_SetMassData` now accepts those overrides,
matching upstream; previously the GPU ignored them and rotated the offset box
about its body origin instead of its center. From `experiments/gpu-physics`, run
`./scripts/check-offset-kinematic-reference.sh` for the independent CPU/GPU
comparison. It checks 903 states over 300 steps per case, including a moving
kinematic body's center change after 150 GPU steps, with absolute tolerance
`1e-5` on mass, center, pose and velocity. This qualifies the sample's zero-mass
center override, not all mass-data semantics.

For unobstructed native captures, pass `--hide-ui` to the sample viewer. This
hides both the controls dialog and settings panel at startup; benchmark mode
ignores keyboard input, so Tab cannot hide them during a benchmark capture.

The latest native captures are in `2026-09-27-ragdoll-parity`, with the matching
real Box3D CPU clips in `000-box3d-cpu`. The comparison grid includes these
300/600/120-step Offset/Ragdolls/Rain recordings. They use software OpenGL
and are not performance measurements. The older checkpoint below is retained
as historical evidence.

Earlier pre-commit native viewer captures for Offset Kinematic, Falling Ragdolls and
Rain are registered in the local comparison grid's corresponding rows. CPU
panes are in `000-box3d-cpu`; the latest GPU panes are in
`2026-09-27-spherical-checkpoint` (the previous captures remain in
`2026-09-26-ragdoll-checkpoint` and `2026-09-25-physics-fixes`). The checkpoint captures cover 300 steps for Offset
Kinematic and Falling Ragdolls and 60 steps for Rain. These are wall-time captures
with software OpenGL rendering, not performance measurements. Rain is a shortened
capture of the initial drops, not a completed benchmark run. The Rust demo
recorder does not host these native scenes. Capture reports and binary identity
are in `experiments/gpu-physics/artifacts/spherical-checkpoint-capture/`. The same
checkpoint also records all 19 Rust demo scenes and their real Box3D CPU oracle
clips at 120 frames, with the CPU column pinned first. These short recordings
are pre-commit comparisons, not the outstanding full qualification. The optional
Rust demo metrics pass was stopped after recording all clips; this checkpoint
does not include a refreshed performance result.

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
  controls must not be presented as functioning GPU features. GPU recording controls remain unavailable; the warm-start checkbox now reaches both engines; the combined worker slider is
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
