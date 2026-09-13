# GPU physics experiment

Research reports, captures, benchmark JSON, and recordings are local ignored outputs under `experiments/gpu-physics/artifacts/` and `recordings/`. Historical evidence links require that local archive; fresh clones retain source, build manifests, test fixtures, and reproduction scripts. Regenerate the comparison index with `bun scripts/refresh-compare.ts .` from the experiment directory after recording clips. Copy the local archive to external storage before removing a checkout if it is needed for publication.

Greenfield engine in [`experiments/gpu-physics/`](../experiments/gpu-physics/): Rust + wgpu + WGSL. **Box3D C is not patched.** It is a visual/timing reference (compare-grid CPU column), not a per-frame lock-step oracle.

Current milestone: [final qualification](../experiments/gpu-physics/artifacts/gear-clipped-support-final/README.md) passes the named-scene goal with 55/0/0 correctness cases and five matched AC trials per engine and scene. GPU median frame cadence is 26% lower for mixed-stacks and 49% lower for Dominoes; p95 is lower on both. This claim is scoped to NVIDIA physics with AMD rendering and equal one-step display delay. Earlier investigation entries below retain their historical status.

Earlier reliability finding: the captured Gear Lift hull–capsule pair exposed incorrect fresh contact geometry: CPU +1.75 mm separation versus GPU −2.70 mm with a 24° normal error. The shader now computes segment/hull closest witnesses and clips face-aligned capsule contacts in hull-local coordinates. Seven geometric controls pass; the captured separation now differs by 0.24 micrometres. In 1,202-step runs, gear-chain-only hinge excess falls from 54.7° to 0.044°, and original Gear Lift from roughly 35° to 0.187°, without NaNs/capacity loss. Core intersections retain the prior penetration path. Full Gear Lift still fails the existing debris trajectory envelope (step 126, body 152), so full native compatibility is not established. Evidence and limits: [hull–capsule correction](../experiments/gpu-physics/artifacts/hull-capsule-witness-fix/README.md). The contact-list lifetime repair remains; its targeted tests and the subsequent ordinary milestone gate pass. The native viewer is also rebuilt: its seven geometry controls and exact mixed-stacks checkpoint pass with zero Vulkan validation errors. Five GPU-before/after frame pairs keep mixed p50 essentially unchanged and improve Dominoes p50, but worsen Dominoes p95 (5.37 → 10.99 ms). The initial short headless window was insufficient; a matched 600-step follow-up still shows nearly unchanged device p95 (2.787 → 2.807 ms), with worse host encode/completed-step tails. Nsight finds fence/present waits and outside-Vulkan time, but does not establish one root cause (`artifacts/hull-capsule-tail-trace/`). A subsequent canonical graph-endpoint prefetch reduces Dominoes graph p50 0.705 → 0.381 ms and device p50 2.123 → 1.803 ms, with byte-identical 600-step trajectories. Five GPU frame pairs improve Dominoes p50 2.518 → 2.128 ms but worsen p95 7.634 → 11.352 ms; mixed stacks is nearly unchanged. This remains provisional within the opt-in shared graph path; see [prefetch evidence](../experiments/gpu-physics/artifacts/graph-endpoint-prefetch/README.md). Caching uploaded scene capabilities then passes five focused tests and exact native Dominoes checkpoints. Five GPU frame pairs improve Dominoes p95 10.577 → 3.873 ms while worsening p50 2.114 → 2.221 ms; mixed stacks stays flat at p50 and improves p95. It is provisionally retained for reduced tails ([evidence](../experiments/gpu-physics/artifacts/scene-capability-cache/README.md)). Parallel publication of canonically chosen graph colors then reduces graph p50 0.384 → 0.334 ms and device p50 1.814 → 1.757 ms. Five GPU frame pairs improve Dominoes p50 2.212 → 2.142 ms but worsen p95 3.435 → 3.805 ms; this remains provisional ([evidence](../experiments/gpu-physics/artifacts/graph-parallel-publish/README.md)). The ordinary milestone gate below passes; no CPU performance qualification is claimed; `cpu_win_validated` stays false. A [fresh ordinary 1,202-step Gear Lift run](../experiments/gpu-physics/artifacts/gear-current-long/README.md) confirms finite, bounded mechanism motion and comparable CPU/GPU speed bounds. The step-126 debris envelope still fails, but body 152 leaves the stairwell depth at step85 and later settles on the main floor; this is not demonstrated tunnelling or an explosion. The original envelope remains unchanged.

How to run, record, and test: [`experiments/gpu-physics/README.md`](../experiments/gpu-physics/README.md).

## Split

| Where | What |
|---|---|
| **C (`hull.c`, compiled into the crate)** | Cook hulls (`b3CreateCylinder`, `b3CreateHull`, `b3CloneAndTransformHull`) |
| **Rust** | `b3_*` world API; copy cooked geometry into GPU buffers; CCD, events, explosions; callback/overlap queries |
| **WGSL** | Broadphase, collide, solve, integrate, closest-hit rays |

Hull cooking is host-side and rare. The GPU owns the every-frame step.

GPU buffers are a **slot pool**: `WorldDef.capacity` (same fields as Box3D `b3Capacity`) plus a 256-body floor. Spawn/despawn reuses slots and uploads scene/body state without recreating contact buffers. Overflow **doubles** capacity and GPU-copies contacts. Host dynamics (transforms, forces, explosions) stream `body_states` only.

Scene heaps are preflighted with checked sizes (`GpuSceneCaps::validate_allocation`). A Cartesian mix of every mesh material against every shape used to request `52500² × 48 = 132.3` GB on Compound / Village; materials are stored linearly and mixed at collide time. Host friction/restitution callbacks fill a **4096-slot** pair table when `n(n+1)/2 ≤ 4096` and all entries fit the shader's 32-probe limit. Exceeding either bound invalidates the world; default coefficients are never silently substituted. Mesh/height-field callbacks are explicitly unsupported because the current shape-pair table cannot represent triangle materials. Callback results are cached until scene mutation; callbacks depending on external mutable state must be reinstalled to invalidate that cache. This is bounded containment, not complete callback support. Compound import now permits up to 65,535 children, reserving one of the 65,536 packed shape slots for the public parent. Repeated mesh children share immutable geometry; negative counts and overflowing or excessive sums are rejected before native allocation. Additional shapes still have to fit the world slot limit. Historical 4,096-child refusals below describe earlier checkpoints. The ordinary-path v19 Village import/drop and scene-switch probes pass, with a 12,257,696-byte packed scene heap for the 52,502-shape drop fixture. This is scene storage, not total process memory, and does not certify settled support or performance. Rejected cooked data carries an invalid version so GPU and dual importers cannot accept it as an empty success; failure attaches to the importing world. Body extent maintenance during append-only shape creation is incremental when the COM is unchanged; mass/COM changes, deletion and geometry replacement retain full recomputation. Mesh contributions are updated after their geometry is populated. This removes quadratic work during large compound import. The focused extent test matches a full scan exactly, and the observed Village startup/short-transition process fell from 104.9 s to 10.8 s (one battery-mode comparison; not a frame-rate or CPU-win result). Requests over 512 MiB are rejected with `gpu_fail` instead of aborting the process.

Pair keys and body slots are 16-bit (65,536). Exceeding that refuses the shape or body instead of silently dropping collisions.

## Bar

Primary: performance, scaling, memory, bounded stability, same-adapter `--self-test`.

Not gates: cross-adapter bit identity, Box3D poses to `1e-5`.

The [Village native gate](../experiments/gpu-physics/artifacts/village-native-gate/README.md) now checks a 600-step sphere drop on the full ordinary compound, replacing the obsolete static-only rejection. The subsequent [ordinary milestone gate](../experiments/gpu-physics/artifacts/milestone-village-repaired/README.md) passes 53/0/0, with 220 library tests, all 12 native matrix cases and three 3,600-step runs. Village CPU/GPU traces pass at `1e-5`. These bounded checks do not resolve the longer Gear Lift debris divergence or qualify performance.

## Profiling workflow

The [component impulse-store optimization](../experiments/gpu-physics/artifacts/component-impulse-stores/README.md) is provisionally retained: three focused native component/reference tests pass, and a Vulkan-validated 600-step Dominoes dump is byte-identical to the frozen baseline. Five alternating GPU pairs improve mixed-stacks frame p50 0.8160 → 0.7999 ms and Dominoes 2.1306 → 1.9961 ms. Mixed p95 improves; Dominoes median trial p95 is essentially flat, with one worse tail run. The 53/0/0 ordinary milestone predates this change. No new CPU qualification is claimed. A subsequent [velocity-only body-store screen](../experiments/gpu-physics/artifacts/component-velocity-stores/README.md) passed focused physics tests but worsened frame tails and Dominoes median time; it was reverted. The [root-only color-offset screen](../experiments/gpu-physics/artifacts/component-root-offsets/README.md) also passed focused checks but showed negligible mixed-stacks median benefit and slower Dominoes median; it was reverted. A [current 8/16/32/64-thread sweep](../experiments/gpu-physics/artifacts/impulse-store-workgroups/README.md) still supports keeping16: 8 has similar median with worse tails, while32/64 slow the median. A [component-specific body-loader test](../experiments/gpu-physics/artifacts/component-body-load/README.md) passed after correcting an inertia-offset bug but regressed frame performance, so it was reverted.

[Fresh five-pair CPU comparisons](../experiments/gpu-physics/artifacts/impulse-store-matched-cpu/README.md) put Dominoes GPU/CPU p50 at1.9166/2.8389ms and p95 at8.0621/11.1437ms: aggregate thresholds pass, with one slightly worse GPU tail pair. Mixed stacks remains GPU-slower (0.8077/0.7803ms p50;2.7237/2.4576ms p95), requiring about22.7% lower GPU median to reach the0.62424ms target. CPU worker counts8/16 reuse the recorded calibration. Overall cpu_win_validated remainsfalse.

A [current CPU sampling capture](../experiments/gpu-physics/artifacts/encoding-cpu-profile/README.md) found 21 Vulkan command-buffer begin/end pairs per frame; incomplete stacks and tracing overhead limit attribution. An [isolated native reset replay](../experiments/gpu-physics/artifacts/native-reset-replay/README.md) preserves the 600-step state with zero Vulkan errors but trades a small mixed median gain for worse tails and slower Dominoes median. It was reverted. The submission split is 14 physics / 7 rendering command-buffer pairs per frame. A [five-pair matrix replay retest](../experiments/gpu-physics/artifacts/current-pair-replay/README.md) shows negligible median gain and worse tails, so that option stays disabled.

[Current single-queue counters](../experiments/gpu-physics/artifacts/single-queue-current-counters/README.md) again show low compute activity and low DRAM throughput in the selected device-wide samples. Teardown overflow is recorded explicitly; these are diagnostic data, not shader attribution or qualification timings. The [component header-load experiment](../experiments/gpu-physics/artifacts/component-header-load/README.md) passed physics comparisons but was reverted: tiny/noisy median gains accompanied worse tails on both target scenes.

The native-only [secondary-queue foundation](../experiments/gpu-physics/artifacts/secondary-queue-handoff/README.md) exposes an explicit two-queue device constructor and passes a 64-generation, two-slot semaphore handoff with zero Vulkan synchronization errors. The [tracked secondary wrapper](../experiments/gpu-physics/artifacts/secondary-tracked-device/README.md) now gives queue 1 independent wgpu completion and lifetime tracking; its focused tests pass with zero Vulkan validation messages. [Tracked cross-queue dependencies](../experiments/gpu-physics/artifacts/tracked-queue-dependencies/README.md) also pass 64 semaphore handoffs without intermediate CPU waits; the [shared pose ring test](../experiments/gpu-physics/artifacts/shared-pose-ring/README.md) now verifies every generation through independently owned alias buffers, including exporter teardown. The [first live integration](../experiments/gpu-physics/artifacts/live-render-queue/README.md) passes a Vulkan-validated mixed-stack run but loses whole-frame p50 in all three alternating pairs; it remains opt-in and is not a retained performance win. The [cold-prefix copy comparison](../experiments/gpu-physics/artifacts/render-prefix-copy/README.md) reduces copied data but shows only weak/noisy timing improvement; the [merged-copy comparison](../experiments/gpu-physics/artifacts/render-merged-copy/README.md) removes that submission on resident frames and improves this prototype by ~5%, but has not beaten the single-queue baseline. The [late-acquisition comparison](../experiments/gpu-physics/artifacts/render-late-acquire/README.md) still loses to a fresh single-queue control (~0.924 versus 0.817 ms p50) and has worse p95. Separate-queue optimization is paused; the original queue remains the performance baseline. Default stepping/rendering still use the original queue. Live integration is tested, but measured frame performance does not justify enabling it; no CPU win is claimed.

[Current worker calibration](../experiments/gpu-physics/artifacts/current-worker-calibration/README.md) uses three trials per CPU worker setting and current GPU controls. Mixed stacks selects8workers: CPU frame p50/p95 0.7509/2.4507 ms versus GPU0.8174/2.4822. Its20% target is0.60072ms, requiring about26.5% further GPU reduction. Dominoes selects16workers by p50: CPU3.0482ms versus GPU2.1554;8workers has a better CPU tail. This is calibration, not final qualification, and confirms mixed stacks as the next priority.

Native qualification now has an explicit `scripts/run-native-bench.sh` X11/PRIME launcher and restored physical window-size logs. Three unchanged-build runs per display path exposed substantial frame variation; older matched runs used X11 while recent experiments inherited Wayland. Cross-path dimensions also differ. See [repeatability evidence](../experiments/gpu-physics/artifacts/display-path-baseline/README.md); recent short frame screens do not establish causal performance changes.

Fresh ordinary Village builds now pass the 600-step CPU/GPU trajectory check and the same-process Village → Bounce House structure/health check, without GPU_PHYSICS overrides. Packed geometry remains about 12.3 MB. See [current support evidence](../experiments/gpu-physics/artifacts/village-current-support/README.md); performance qualification and broader compatibility remain separate.

A subsequent [early bounds rejection screen](../experiments/gpu-physics/artifacts/matrix-early-overlap/README.md) passed focused mutation/capacity/mesh checks and reduced mixed-stacks device time about .024 ms, but five viewer pairs had worse median and tail quantiles, so it was reverted. A persistent temporal pair cache is not justified by the current measured construction cost alone.

A [CPU/runtime trace](../experiments/gpu-physics/artifacts/nsight-cpu-waits/README.md) attributes substantial warmed main-thread waits to swapchain acquisition and presentation driver calls. This motivates evaluating bounded presentation overlap; no asynchronous-presentation implementation or speedup is claimed.

The subsequent [bounded presentation-worker prototype](../experiments/gpu-physics/artifacts/present-overlap/README.md) passed a focused Vulkan smoke, but raised presentation-return latency on both named scenes. Two reversed-order pairs did not justify keeping it; it was reverted.

A [shader-level GPU Trace attempt](../experiments/gpu-physics/artifacts/ngfx-shader-trace-live/README.md) did not produce usable output: replay readiness stalled, and a live trace timed out with a sample-rate warning. These failures must not be used as shader performance evidence; the earlier Systems measurements remain the usable profiler evidence.

Component TGS validates its immutable chains once per dispatch before integration, then uses a validated-chain solver helper. Large components validate their full compact range cooperatively, including overflow roots; the overflow solve order remains unchanged. Global color paths retain per-phase checks. [Focused tests and five paired frame trials](../experiments/gpu-physics/artifacts/component-chain-validation/README.md) support a modest mixed-stacks improvement; Dominoes p95 is slightly higher and no CPU-win claim is made.

The [large-component chain-validation screen](../experiments/gpu-physics/artifacts/large-component-chain-validation/README.md) passes 14 component regressions and an exact native600-step Dominoes checkpoint with zero Vulkan errors. Three device pairs reduce solve p50 0.7690→0.7516 ms. Five frame pairs improve Dominoes p50 2.2331→2.1440 ms, but median trial p95 worsens3.6659→4.6534 ms despite improving in3/5pairs. This remains provisional; no CPU or tail qualification is claimed.

A [large-component workgroup sweep](../experiments/gpu-physics/artifacts/large-component-workgroups/README.md) preserved tested physics at64 through1024 threads. It did not establish a frame/tail benefit:256 worsened viewer p95 and512/1024 increased device cost. Runtime remains64 threads; the prototype is reverted. A [render bind-group reuse screen](../experiments/gpu-physics/artifacts/render-binding-reuse/README.md) also remains reverted: mixed-stacks timings varied heavily, and Dominoes had essentially unchanged median frame time with worse tails.

Use uninstrumented matched whole-frame measurements to decide whether an optimization wins, GPU timestamps to identify expensive stages, and Vulkan validation separately to check synchronization. Full profiler traces now complement those checks: user-local Nsight Systems and Graphics are installed on the development laptop. Individual-workload Systems captures complete on the native command-reuse path; the first batch-mode GPU ranges were implausible and must not be used. Hardware counters are now accessible after the user’s session capability grant. A diagnostic backend omitting unused external-memory export enables a pixel-verified unnamed Graphics replay; named capture replays still render blank and are excluded from evidence. Bounded device-wide counter sampling now shows low occupancy during compute-heavy intervals, but no individual shader stall cause is established. Routing small components through the current cooperative large-component kernel passed focused position/validation checks but worsened solve time from 0.2222 to 0.3983 ms; it was reverted. See [the rejected screen](../experiments/gpu-physics/artifacts/cooperative-small-current/README.md). A subsequent [native global-wave replay screen](../experiments/gpu-physics/artifacts/native-global-waves/README.md) preserved reference positions and passed Vulkan validation, but both 325- and 117-dispatch variants remained device-slower than the component solver. That candidate was also reverted. A separate startup trace identified seconds of pipeline creation. A persistent pipeline cache (now enabled by the native launcher, with `GPU_PHYSICS_PIPELINE_CACHE=0` opt-out) reduced a measured headless warm launch to 1.73 seconds, with matching 120-step positions and zero direct Vulkan validation errors. This is startup evidence, not whole-frame qualification; see [the cache experiment](../experiments/gpu-physics/artifacts/pipeline-startup-cache/README.md). The subsequent [three-pair frame screen](../experiments/gpu-physics/artifacts/cached-frame-screen/README.md) still fails the performance target on both scenes; current CPU controls are faster than several historical measurements. Setup, reproducible capture commands, failed captures and the next focused steps are recorded in [the profiler notes](../experiments/gpu-physics/artifacts/nsight-systems/README.md). Profiler timings are diagnostic, never replacements for unprofiled CPU/GPU qualification. The first trace-driven candidate removes only the extra Linux CPU acquisition-fence wait while retaining GPU semaphore synchronization. Its bounded Vulkan smoke passes, but two further pairs per scene give mixed results: mixed-stacks median p50 improves about 12.5%, Dominoes worsens 4.2%, while p95 improves on both. It remains archived rather than enabled globally. See [the acquisition experiment](../experiments/gpu-physics/artifacts/async-acquire/README.md).

The small-component solver has an archived unreachable-fallback cleanup: entry guards already limit it to 8 bodies / 32 contacts, so its helper full-world fallbacks cannot run. Checkpoint positions, three component-transition/overflow tests and Vulkan validation pass; two reversed-order device screens save 4–5 microseconds. Two native pairs per scene do not demonstrate a frame benefit (mixed p50 and Dominoes p95 are worse in aggregate amid substantial variation), so the cleanup remains archived without integration. See [the small-component experiment](../experiments/gpu-physics/artifacts/small-component-dead-paths/README.md).

## Pipeline (one step)

A bounded pair-bitset candidate for flat worlds up to 704 shapes is isolated in `experiments/gpu-physics/artifacts/pair-bitset/`. It preserves ordered candidate pairs and contact retirement, with focused mutation, capacity and mesh-transition checks. Three profiles halve broadphase device cost; five native mixed-stacks pairs improve frame p50 from 1.1685 to 1.0702 ms, with p95 essentially unchanged. A command-reuse follow-up (`experiments/gpu-physics/artifacts/pair-bitset-cache/`) improves paired frame p50 5.0% but worsens p95 1.8%; short profiles do not show an encoding gain. It stays optional. Native tail replay (`experiments/gpu-physics/artifacts/native-tail-cache/`) lowers encoding significantly after explicit query-buffer initialization; seven focused checks and four exact scene comparisons pass. Fresh five-trial mixed GPU/CPU frame p50 is 0.920/0.867 ms, while Dominoes is 2.586/5.574 ms. Mixed still fails the overall target. The direct viewer now has an opt-in `GPU_PHYSICS_DEMAND_POSES=1` policy that omits unused automatic pose copies while preserving explicit current-state getters; C/Sokol and the default policy retain automatic copies. The isolated change and measurements are in `experiments/gpu-physics/artifacts/demand-pose-staging/`. Its five-pair medians improve mixed p50 about 3% with a slightly worse p95, and leave Dominoes p50 essentially unchanged. Eight focused prototype checks, native Vulkan smoke, and the four new ordinary-build tests pass. It remains opt-in. The following combined-submission candidate (`experiments/gpu-physics/artifacts/combined-frame-submit/`) preserved tested current-step state and passed native Vulkan smoke, but its three-pair mixed screen showed only 2.9% better p50 with essentially unchanged p95. It was archived without a runtime change. A bounded stage profile then identifies surface acquisition (~0.25 ms) as the largest GPU draw-stage cost. Moving acquisition before physics worsens mixed median frame time 4.3% despite improving p95 14.3%; that candidate also remains archived (`experiments/gpu-physics/artifacts/early-surface-acquire/`). A subsequent GPU-only current-frame render snapshot passes focused pose/pixel/shadow and native Vulkan checks but regresses mixed p50 3.1% (p95 improves 8.1%); it is also archived without a runtime change (`experiments/gpu-physics/artifacts/gpu-render-snapshot/`). Reusing declarations between consecutive native solver blocks likewise passes exact-state/Vulkan checks but yields no meaningful profile gain, and remains archived (`experiments/gpu-physics/artifacts/native-access-reuse/`). The subgroup component-prefix scan also passed 43 exact integer/capacity cases with no Vulkan errors, but saved only 0.352 microseconds at 602 bodies; it was archived without a runtime change (`artifacts/subgroup-component-scan/`). A separate eight-body invocation-private cache preserves the three tested scene dumps but worsens mixed solve/device cost in its initial screen; it was also archived (`artifacts/small-component-private-cache/`). Its two-scalar-state follow-up also matches those scenes but worsens mixed solve p50 about 18%; it remains archived (`artifacts/two-body-scalar-cache/`). An exact-isotropic inertia shortcut saves only about 4 microseconds of solve time in its initial screen and is also archived (`artifacts/isotropic-inertia/`); it does not establish a latency win. Driver diagnostics now show 168 registers per thread for both component kernels (`artifacts/solver-executable-stats/`). Removing three redundant finite-loop counters does not reduce that count; this compiler prototype is archived without a timing claim (`artifacts/bounded-manifold-loops/`). SPIR-V aggregate scalarization also leaves register counts unchanged and is archived without a performance claim (`artifacts/solver-scalarization/`). Splitting small-component phases increases solver dispatches from 2 to 22 and regresses mixed solve p50 0.223 to 0.327 ms; that candidate is archived too (`artifacts/small-component-phases/`). A single-slot component parameter upload reduces host encoding in two paired profiles and passes transition checks, but does not improve the three-pair mixed frame screen; it remains archived (`artifacts/component-param-upload/`). Ordinary builds still use the grid; neither this result nor the prior cache experiments certify the overall CPU-win target.

An incremental pair-discovery prototype was measured and reverted. Exact pair-set comparisons covered the named scenes, both moving-ID orders, filters, teleport history, sleep/wake, slot reuse and callback rejection/restoration; stable mixed stacks demonstrably skipped fresh discovery. Three alternating active stage trials reduced median Dominoes broadphase p50 from 0.334 to 0.301 ms, but completed-step p50 stayed near 2.81 ms; mixed stacks completed-step regressed from 1.269 to 1.391 ms. Five native frame pairs did not establish a consistent improvement with no-worse tails. The complete patch, focused tests and measurements remain in `experiments/gpu-physics/artifacts/incremental-discovery/`; runtime fat-bound records remain eight words and the diagnostic switch is not installed.

Spatial-hash pairs → filter → recycle or SAT → island graph → TGS (warm start, biased solve, integrate, relaxed solve) → restitution → sleep. Contact allocation preserves its ordered missing-pair list while building free slots, avoiding a repeated histogram/prefix pass and an indirect-command copy. The unique-pair count reset remains required by subsequent graph compaction, including steps with no new contacts. All 201 library tests pass; paired frame timings do not establish a whole-frame gain (`experiments/gpu-physics/artifacts/missing-contact-list/`). Step setup defers its parameter-table upload until the submission owner assigns the step ID; ordinary and callback submissions upload the final table once. Explicit world setup and queries still upload immediately. This removes a redundant host upload without changing the compute schedule; it is not a measured whole-frame win. A separate opt-in `GPU_PHYSICS_COMPONENT_TGS=1` prototype runs complete independent components in two solver dispatches, after five GPU list-building dispatches. It builds compact per-component body/contact/color lists with parallel counts/scatter and a cooperative prefix scan, and dispatches only nonempty large roots. Small components restore canonical contact order; large components preserve color order and serial overflow. A workgroup-uniform mask skips barriers only for colors empty in that component, preserving all active color boundaries. The strict comparison after 120 steps passes both full named scenes at unchanged 1e-5, with four resident/CCD tests also passing. Three paired development trials show global/prototype frame p50 2.59/2.04 ms (mixed stacks) and 3.48/3.06 ms (Dominoes); these compare GPU schedules, not CPU engines. It stays disabled by default: large connected components still occupy one workgroup, overflow scans remain, and timing varies. See `experiments/gpu-physics/artifacts/parallel-component-prefix/result.json`. This is separate from the old disabled per-body fused path. A tested hybrid that returned large components to global color waves was slower and removed; see `artifacts/hybrid-components/result.json`. Its merge/split ownership regression remains on the compact path. Sleeping small and large components now return after island wake propagation but before local-list traversal/substeps. Reconnection/wake and return-to-sleep comparisons retain the existing 1e-5 tolerance. The exact early Dominoes window shows solve p50 0.780→0.704 ms; five GPU-path frame pairs show p50 3.040→2.965 ms, without a CPU-win claim. See `experiments/gpu-physics/artifacts/sleeping-components/`. A later 192-byte workgroup color-metadata cache passed the component comparisons but showed no repeatable timing benefit when run order was reversed. It was removed; evidence is in `experiments/gpu-physics/artifacts/component-color-cache/`. A bounded body-state workgroup cache also passed its focused comparisons (including 256/257 members and sparse high body IDs), but its initial exact-window profile showed no solver/device improvement. It was removed; the prototype and measurements remain in `experiments/gpu-physics/artifacts/component-body-cache/`. Neither experiment establishes a frame or CPU win. Removing the unused GL export allocation/copy in the direct Vulkan viewer was also tested in five paired trials per scene. It did not produce a consistent frame improvement and p95 medians worsened; export behavior remains unchanged. See `experiments/gpu-physics/artifacts/unused-pose-export/`.

Hull–hull contacts use face SAT and clip the incident face (pick the face whose outward normal matches the SAT axis). Hull–box / hull–capsule still use Gauss-map edge tests. Mesh and height-field contacts run against convex partners on static or kinematic (and dynamic) bodies; triangle winding stays one-sided.

Queries: GPU **closest-hit** (`b3World_CastRayClosest`) scans resident shapes, including triangle meshes and height fields, after the queried physics submit completes. Ties pick the lowest CPU shape index, then the lowest triangle index. Inside origins (`fraction <= 0`) miss. Results live in a dedicated query buffer, not physics scratch. Callback casts, overlaps, and explosions use the host mirror. Hull mover queries match Box3D’s zero-distance convention: a capsule axis intersecting a hull yields no plane; positive separation below linear slop still yields a plane. Fabricating deep-overlap hull planes altered character velocity clipping and prolonged Village’s sphere-drop interaction. The corrected 600-step comparison matches CPU states within `1e-5`, with both settling at step 305 (`artifacts/village-hull-mover/result.json`). This is focused query validation, not full native compatibility. Native `b3World_Step` **submits** and returns. Do not reuse an earlier collision grid for picking unless it matches post-step poses.

Synchronous getters (`GetPosition`, AABB, velocities) wait for that completed step, run pending CCD once, and must agree. The Sokol pose snapshot stays asynchronous for drawing and is not the public oracle. Picking uses the same finalized state rather than a second CCD pass. Events are harvested once from the finalized step; unread events are dropped at the next `Step`. Host teleports upload only dirty body slots. The in-development GPU CCD library now has endpoint classification and convex conservative advancement, validated against 167 CPU sweeps. Default stepping still uses CPU CCD. The opt-in `GPU_PHYSICS_GPU_CCD=1` convex path now integrates GPU traversal/filtering and correction, with mesh/sensor/bullet/callback worlds retaining CPU CCD. Eligible clean, joint-free worlds without contact-event readback now submit consecutive steps and render without a CPU body mirror. Dirty mutations use the upload boundary; explicit synchronous getters still finalize and read current state. `GPU_PHYSICS_RESIDENT=0` restores the mirror for A/B measurement. A 240-step regression verifies zero body-mirror bytes and pose maps, including render metadata calls, then checks getters and mutations. Four integration tests pass after the final follow-up; the preceding full library run passed 191/191. Five same-binary GPU-path comparisons reduced frame p50 from 4.88 to 2.88 ms (mixed stacks) and 6.45 to 3.03 ms (Dominoes), but p95 did not consistently improve. Immediate presentation is confirmed at wgpu, not at the compositor. These are not CPU-engine comparisons; `cpu_win_validated` stays false. A subsequent same-renderer comparison now runs real CPU Box3D through a private Linux bridge, with four CPU workers and CPU pose upload included. Five paired trials show CPU/GPU frame p50 1.64/3.01 ms for mixed stacks and 2.13/3.36 ms for Dominoes (`artifacts/shared-renderer-cpu-gpu/result.json`). GPU host physics submission remains roughly 2.3–2.5 ms; reducing that cost is the next target. A body-count-bounded direct solver-dispatch experiment reduced host cost but was removed after five repeated pairs failed to establish reliable frame/p95 improvement (`artifacts/direct-color-repeat/result.json`). Indirect scheduling and wgpu validation remain enabled. The bridge is opt-in via `GPU_PHYSICS_CPU_REFERENCE`; this is a measurement path for the two named scenes, not general CPU renderer parity. See `experiments/gpu-physics/artifacts/resident-immediate-frames/result.json`. The comparison found a CPU GJK degeneracy: a tetrahedron with negative barycentric weights must not certify overlap. Both implementations now retain the previous valid simplex in that case.

Eligible resident worlds now skip physics dispatches by default (`GPU_PHYSICS_IDLE=0`
disables the shortcut)
after a GPU-produced zero-active count proves the current state is asleep.
The result may arrive late only if all intervening submissions form a contiguous
eligible chain with matching input/output state contexts, topology and mutation
epoch. In that closed world, sleeping bodies remain unchanged through integration,
contact solving and CCD. A mutation, context gap or ineligible step breaks the
chain. This inference does not advance completion IDs or relabel measured status.
The count shares the asynchronous status transfer; it adds no blocking readback. Submission/completion, status and pose exports remain real. Relevant
mutations invalidate the proof. Sleeping contacts retain cached impulses instead
of solving two immovable endpoints and changing warm-start history. Four idle
regressions cover wake equivalence at 1e-5, pending mutations, queries, forces and
growth; the initial release library validation passed 199/199. This does not
establish an active-scene CPU win. Delayed-proof tests additionally cover
1/2/5-step delivery, impulse, gravity, state-context and eligibility breaks, and
zero-duration steps. Five native mixed-stacks smoke runs now enter idle for
118–119 of 120 timed frames, compared with the prior 0–119 range. These are
unmatched diagnostic comparisons, not CPU-win certification; see
`experiments/gpu-physics/artifacts/idle-chain/`.

The default-enabled path passes 201 release library tests. Five longer matched
CPU/GPU trials give mixed-stacks aggregate frame p50 GPU1.150/CPU2.189 ms and
Dominoes GPU2.953/CPU1.231 ms; only one paired trial per scene meets both target
thresholds, so `cpu_win_validated` remains false. Evidence: `experiments/gpu-physics/artifacts/idle-matched-current/`.

The settled-step diagnostic improves completed-step p50 1.446→0.068 ms,
but five paired viewer trials give mixed-stacks frame p50 1.905→1.843 ms with
inconsistent tails; active Dominoes is 3.032→3.169 ms. These were initial
opt-in measurements, preceding the longer matched runs.
See `experiments/gpu-physics/artifacts/resident-idle/`. Exact proof delivery in the
asynchronous viewer still needs assessment; no new blocking wait was added.
A retained follow-up polls ready status after presentation when idle mode is
requested, including the cost in total frame time. Five paired GPU trials reduce
mixed p50 2.253→1.310 ms and increase zero-solver frames from median58 to97/120,
but p95 worsens 11.186→12.183 ms. No default promotion or CPU-win claim.
Evidence: `experiments/gpu-physics/artifacts/resident-idle-late-poll/`.
A three-slot status ring improved proof delivery but worsened mixed frame p50
1.193→1.884 ms in five paired trials; it was removed. Single-slot late polling
remains, and no status-ring runtime option exists. Archived evidence:
`experiments/gpu-physics/artifacts/resident-status-ring/`.
Skipping proven-idle status transfers likewise reduced host submission but not
frame time (five mixed pairs: p50 1.183→1.243 ms, p95 5.093→7.367 ms). It was
removed; the previous implementation is retained. Evidence:
`experiments/gpu-physics/artifacts/resident-idle-no-status/`.

Full mirror reads publish available timestamp results because a consumed pose snapshot bypasses the drawing path that used to refresh those clocks. The regular colored solver keeps bodies in lane-local storage: coloring already guarantees exclusive writable endpoints, so its old per-lane shared-memory staging and workgroup barrier were unnecessary. Overflow order and the one-workgroup wave are unchanged. Three paired trials show 2–4% lower solve p50; 181 release library tests pass (`artifacts/v22-solver-local/`).

Native application benchmarking now explicitly sets and queries the GLX drawable swap interval. Sokol replaces descriptor zero with default interval one; older JSON hard-coded zero and therefore did not verify unpaced presentation. Unknown platforms report -1, and the timeline script rejects unpaced certification unless the query returns zero. Five corrected Dominoes trials give median frame p50 6.17 ms CPU versus 9.98 ms GPU, so the goal remains unmet (`artifacts/v22-unpaced/`). These are callback-cadence measurements, not photon latency, and compositor pacing may still influence presentation.

Host CCD now builds per-fast-body shape lists and eligible target lists once per harvest, retaining shape-slot order and all existing filtering/TOI checks. Previously each fast body rescanned the entire shape array for ownership and target classification. Three alternating AC-powered native Dominoes trials (30 warmup, 120 timed, unpaced, sleep off, four CPU workers) reduced the median of frame-p50 results from 16.31 to 14.63 ms; each pair improved. This is an old/new GPU comparison, not CPU certification: the separate AC CPU baseline was 9.03 ms. Physics completion waits remain a major cost inside synchronous picking. Evidence: `experiments/gpu-physics/artifacts/v20-ccd-index/result.json`. The temporary timing probe was removed; no solver math or GPU closest-hit fallback changed.

### Broadphase

Dynamics go in the spatial hash. Fat statics (grounds that would fill `MAX_INSERTS`) are a compact list; each dynamic tests that list instead of every shape.

Radix/unique compact dispatch from the live pair count. Unique compact **must not** scan stale radix bucket offsets in unused 256-wide groups (`compact_unique_bases` zeros inactive group counts). GPU tests prefills those words with garbage so the original overflow cannot regress.

New contacts are assigned from a deterministic free-slot prefix (sorted empty slots × missing pair indices), then published into the contact hash. Pair and contact hashes mix the full 32-bit packed key before masking so ground contacts (`minShape=0`) do not collapse into one bucket.

Find-existing / prepare / island-union dispatch from live pair/contact counts. Unique pair compact is a 256-wide workgroup prefix-sum. wgpu 26’s Naga still rejects `enable subgroups`, so the device requests the feature when present but the compact kernel uses shared memory.

### Graph and coloring

Graph construction rebuilds each step:

1. Unique pairs are radix-sorted by pair key.
2. A parallel classify + stable compact splits static–dynamic and dynamic–dynamic edges while keeping that order.
3. Static coloring is parallel: degree-one edges share one reserved color. Opt-in `GPU_PHYSICS_AB=bounded-static-sort` worlds with a proven degree-two bound find each writable body's minimum pair index with an atomic minimum, assign ranks 0/1 in pair order, and scatter into two exclusive colors. Larger/unproven degrees use stable radix sorting by (writable body, pair key), ranking, then color scatter.

Histogram/prefix/scatter use the live group count for that stage (`SCR_RADIX_GROUP_N`). Inactive radix rows are not summed. Degree-one and empty sorts set group count 0 so prefix/base kernels are no-ops and do not clobber compact metadata.

Dynamic greedy assignment preserves pair-key order but now selects the lowest
free color with two occupancy loads and a bit scan, avoiding repeated atomic
loads in the candidate-color loop. A GPU fixture checks exact colors, gaps and
overflow. The temporary Dominoes probe measured dynamic assignment 0.469 ms of
0.567 ms graph time; after removing the probe, graph p50 improves 0.559→0.511 ms.
Five frame pairs are essentially flat (3.142→3.178 ms), so no frame/CPU win is
claimed. See `experiments/gpu-physics/artifacts/dynamic-color-bits/`.

Opt-in `GPU_PHYSICS_GRAPH_SHARED=1` caches 8,168 occupancy masks and 24 color
counters in exactly 32 KiB of workgroup memory. Cooperative loading/writeback
surround the same one-lane greedy walk. Unsupported devices and larger slot spans
keep the global-memory entry. The new pipeline is lazy; native device limits are
bounded by adapter support. Exact colors and global masks match across the cache
boundary and overflow; named-scene comparisons and 201 release library tests pass.
Dominoes graph p50 improves 0.509→0.382 ms in a device diagnostic, but five frame
pairs remain flat for Dominoes (2.999/3.116 ms) while mixed stacks improves in the
short window (2.868/1.889 ms). This stays opt-in and is not a CPU-win claim.
Evidence: `experiments/gpu-physics/artifacts/shared-graph-cache/`.

The current combined configuration still misses the native frame target. Five
alternating CPU/GPU trials per scene (200 warmup / 600 timed, sleep disabled,
CPU four workers, same Vulkan renderer) measured median per-trial whole-frame
p50 CPU/GPU of **1.5166/1.6982 ms** for mixed stacks and **2.2346/3.0871 ms** for
Dominoes. Dominoes p95 was **3.0503/13.6833 ms**. Mixed stacks had substantial
run variation. Evidence: `artifacts/combined-current-matched/` in the experiment.
`cpu_win_validated` remains false.

Appending rendering to the resident physics command encoder was tested and
removed. Five paired trials showed essentially unchanged Dominoes p50 and only
about 2% lower mixed-stacks p50; every paired p95 worsened in both scenes.
Separate submissions remain in use. The rejected prototype and results are in
`artifacts/combined-submit/`. A temporary backend probe subsequently split acquisition into semaphore reuse,
`vkAcquireNextImageKHR`, and the explicit image-ready fence wait. On Dominoes
GPU their median costs were 0.0006, 1.8073, and 0.0071 ms respectively. Removing
the last wait on Linux did not improve whole frames; that dependency variant was
removed. These are single-run diagnostic scopes, not new CPU-win evidence.

A bounded acquisition worker also failed five paired native trials: Dominoes
p50 stayed 3.0793/3.0820 ms and p95 worsened 13.5097/14.2635 ms; mixed stacks
p50 was 1.7008/1.7194 ms. It was removed. The pinned wgpu-core holds a device-fence
read lock throughout acquisition, while submission takes its write lock, so the
worker can move the wait into submission. A Linux timeline-fence snapshot prototype removed this lock conflict while
preserving semaphore waits and device lifetime. Five paired trials still had
essentially unchanged frame medians: Dominoes 3.0981/3.0915 ms and mixed stacks
1.7032/1.7021 ms. It was removed along with the worker; no dependency fork is
retained. Thus the lock explains wait attribution, but its removal did not solve
whole-frame throughput. Evidence: `artifacts/acquire-backend/`,
`artifacts/threaded-acquire/`, and `artifacts/acquire-fence-snapshot/`.

A direct grouped-contact-list prototype also showed no solver gain and was
removed: four paired device trials measured mixed-stacks solve at 0.2068 ms in
both modes, and Dominoes at about 0.5852 ms in both. Its useful regression remains:
28 overlapping supports force multiple overflow contacts, and the complete-component
solver matches the global solver across all 120 checked states at `1e-5`.
Evidence: `artifacts/direct-component-lists/`.

Further impulse-path probes were also removed. A local inverse-inertia matrix
changed the 120-step Dominoes result beyond the existing `1e-5` comparison limit
(about `0.000445` in one checked value). A subsequent three-trial diagnostic
comparison also found no useful speed benefit: mixed-stacks solver time rose
from 0.2068 to 0.2335 ms with the larger private structure, even with matrix
evaluation disabled; enabling it did not recover the loss. Dominoes was flat.
This numerical difference alone does not establish physical instability. An exact-zero impulse
skip passed that comparison but produced no useful measured solver gain.
Evidence: `artifacts/local-inertia-cache/` and `artifacts/zero-impulse-work/`.

A separate-queue transport probe now exercises two wgpu contexts sharing one
Vulkan device, distinct queues, and two GPU-only snapshot allocations with distinct
buffer handles. Three 512-publication rounds verify every consumed word, skipped
presentations, repeated slot reuse, size changes, and teardown. Timeline semaphores
order publication and reuse; CPU waits occur only at initialization and batch-end
readback. A locally extracted Khronos validation layer (1.4.357) with synchronization
validation reports no errors. This is a transport proof, not a viewer integration,
physics gate, or demonstration of concurrent hardware execution.
Evidence: `artifacts/two-queue-probe/`; source: `examples/two_queue_probe.rs`.

A subsequent capability probe found real overlap only on the dedicated compute
family. Four rotated-order trials per mode/scene use controlled 64-lane compute
plus the existing scene draw code with frozen poses, with calibrated timestamps
explicitly enabled for cross-queue comparison. Exact compute results and image
hashes match, and all four modes separately pass synchronization validation.
Device envelopes for single/universal-pair/dedicated queues were 0.5146/0.5699/0.4007
ms on mixed geometry and 0.5260/0.6044/0.4110 ms on Dominoes geometry. Serialized
semaphore controls show no overlap. This excludes real physics, shadows,
presentation and live pose transport; it is not a native speedup or CPU-win claim.
Evidence: `artifacts/queue-overlap/`.

The dedicated queue cannot yet run the pinned wgpu physics backend correctly:
its generic storage-buffer barriers include unsupported vertex/fragment stages.
The retained capability probe therefore uses a native Vulkan compute command;
the invalid wgpu variant and its validation failure are preserved. Loading the
existing physics pipelines also exposed a separate relaxed-atomic SPIR-V
validation error (VUID 10871) in the pinned shader compiler. Neither failure is
silenced or counted as a pass. The next bounded work is valid compute-family
backend support and a minimal atomic-emission repair, followed by actual physics
snapshot integration. Ordinary renderer/physics behavior is unchanged by these
standalone examples; Naga is an additional direct dev dependency only.

An isolated, reproducible candidate backend now addresses those prerequisites:
relaxed atomic instructions omit storage-class semantics bits while retaining their
scopes and explicit barriers; uniform/storage barrier stages follow queue
capabilities; ordinary buffer/image barriers use ignored ownership indices for
concurrent shared allocations. The minimal atomic SPIR-V regression fails before
and passes after the patch. Across all 120 checked steps, High Resistance, mixed
stacks and Dominoes states match exactly between graphics-capable and dedicated
compute queues on the candidate. Both same-family and cross-family two-slot
transfer probes pass, including cancellation, reuse, size changes and teardown;
the existing graphics probe preserves its image hash. All final checks ran with
native synchronization validation and no errors. Evidence, source hashes and
isolated preparation script: `artifacts/compute-backend/`. This is not an old/new
compiler comparison, full physics gate, or native-frame win. Ordinary dependency
selection is restored.

The live two-queue viewer prototype was rejected and removed after two rounds of
native comparisons. Initial evidence is in `artifacts/async-viewer/`; the follow-up
in `artifacts/merged-queue-waits/` folds external timeline waits into the actual
submissions, preserving synchronization and two-slot depth. Its focused snapshot
regression and native synchronization validation passed, but five rotated trials
per scene did not justify keeping the runtime. Mixed single/merged p50 was
1.7359/1.8691 ms (p95 12.5547/12.2910); Dominoes was 3.1040/2.9976 ms
(p95 13.3457/14.4254). These are medians of trial statistics, not pooled frames.
No CPU comparison was rerun and no CPU win is claimed. The ordinary single-queue
renderer and dependency selection are restored. Rejected runtime and pinned
backend patches remain as reproducible artifacts; the standalone queue probes
remain available. Next work should reduce measured device work, not grow this
transport or hide waits with deeper/stale frame queues.

Three subsequent device-work candidates were tested and reverted. Deferred
collider loading (`artifacts/lazy-collider/`) and radix prefix fusion
(`artifacts/radix-prefix-fusion/`) produced no material total-device benefit.
Bitset radix ranks (`artifacts/radix-bitset-rank/`) saved about 0.04 ms device time
on both named scenes and improved mixed native p50 1.7113→1.6630 ms, but slightly
regressed Dominoes median in two five-pair batches. Their focused sort/checkpoint
comparisons passed; none establishes a CPU win. The original shaders remain.
Two-body/four-contact solver specialization was subsequently rejected too:
solve p50 rose 0.2068→0.2263 ms mixed and 0.5837→0.6083 ms Dominoes. Named dumps
and a partition/overflow regression passed, but the extra dispatch and smaller
private arrays did not improve measured work (`artifacts/micro-component-solver/`).
Captured-graph analysis now supports a bounded parallel-coloring probe:
`examples/coloring_frontiers.rs` verifies canonical colors before comparing
frontiers. Mixed's300 edges take one round; Dominoes320 takes75 canonical rounds
versus6 hash-priority rounds, changing464 colors. These are CPU scheduling results,
not GPU timings or physics-quality evidence (`artifacts/coloring-frontiers/`).
The subsequent isolated GPU endpoint-election probe passed16 native validation
cases, including full pair capacity and bounded fallback. Coloring plus ordered
overflow/list output improved from0.091→0.051ms mixed and0.258→0.155ms late
Dominoes (`artifacts/parallel-color-probe/`). These are probe times with contiguous
edge input, not production graph or frame times. Live integration was subsequently
tested and rejected (artifacts/live-parallel-color/). Ten captured live schedules
matched the independent CPU coloring model, and the uncached variant passed
3,600-step support/bounded checks on High Resistance, mixed stacks and Dominoes.
Caching immutable edge records reduced the live overhead, but three paired late
Dominoes trials gave graph 0.3727→0.3359 ms and solve 0.5868→0.6349 ms; total device
1.6896→1.6988 ms. Mixed device was essentially unchanged, 0.7076→0.7055 ms.
No native-frame or CPU-oracle win was established. The candidate was removed;
the existing canonical coloring path remains. Further coloring work must account
for solver cost as well as assignment cost.

A direct validation path for single-manifold roots is retained
(artifacts/single-manifold-validation/). It preserves all ownership, endpoint,
point-count and failure checks while avoiding general chain traversal.
Twenty-one chain tests passed, and the three named 120-step state comparisons
were exact. Three paired device trials reduced solve p50 0.2068→0.1946 ms mixed
and 0.5806→0.5652 ms Dominoes. Five matched native reference/candidate pairs gave
frame p50 1.7031→1.6831 ms mixed and 3.1073→3.0643 ms Dominoes; median trial p95
was lower in both, but tails vary widely between runs. This is a small
same-GPU improvement, not fresh CPU-win or full native compatibility evidence.

Component count/scatter now scan active candidate roots once instead of scanning
all24 colors per lane. Current color-list membership is checked before adding
a root, and original color-local ordering keys are preserved. Three component
regressions and the three named step120 state comparisons pass. Device p50
improves 0.6932→0.6851 ms mixed and 1.6568→1.6333 ms Dominoes.
Five mixed native pairs improve frame p50 1.6850→1.6724 ms. Across ten Dominoes
pairs, p50 improves 3.06825→3.0100 ms but median trial p95 rises
13.53235→13.6701 ms, with opposite directions in the two batches.
The development change is retained for median/device cost; the p95 milestone
remains unresolved.

The subsequent surface-buffering experiment was rejected: latency hint1 regressed
both scenes and hint3 did not improve both median/tail requirements. Renderer
defaults were restored and the rebuilt binary matched the prior binary.
A 1920×1080 Wayland/XWayland probe then found a potentially lower XWayland
median, but one trial ended at physics step508 despite recording600 timed frames.
That run is invalid. The interrupted follow-up status probe produced no output,
so the cause remains unknown; no backend change is retained. Temporary physical
size/window/status instrumentation is archived and removed from runtime.
Resume by reproducing this failure and reading sticky world status before any
new backend performance claim (artifacts/native-window-backend/).
The overnight checkpoint includes comparison clips for all18 selectable scene
slugs, including Dominoes and the four scenes omitted by the old recording
scripts; the real Box3D CPU column stays first. These clips are pre-commit
review material, not a fresh full compatibility or CPU-win gate.
The final 2026-09-11 checkpoint release library run passed208/208 tests.
Both18-clip sets contain120 frames per clip, verified with ffprobe.
Work is paused at the user's request; resume with the unresolved native stall
described above. Checkpoint status: artifacts/overnight-checkpoint/status.json. Evidence: artifacts/component-active-scan/.
The preceding temporary timing split (artifacts/component-cost-current/) found
late Dominoes list/small/large component costs near 0.14/0.15/0.26 ms.
Its probe-specific timestamp meanings have been restored to ordinary timings.

Small-component solver workgroups now default to 16 lanes, with
`GPU_PHYSICS_SMALL_COMPONENT_WG=8|16|32|64` as an override. Shader specialization
and host dispatch use the same validated value; large-component workgroups stay
64. The component solver remains opt-in and all math/substeps are unchanged.
Four stage trials per size favor 16: mixed solve p50 0.333→0.207 ms and
completed-step 1.218→1.077 ms. Five native pairs (warmup200/timed600, no sleep)
improve every trial's p50: mixed median1.951→1.778 ms, Dominoes3.137→3.070 ms.
Mixed p95 remains problematic (median12.906→12.914, four paired regressions).
Named states, empty/partial dispatch boundaries, merge/split, sleep/wake and
growth checks pass, and the final default release library suite passes 206/206;
this does not establish a CPU or tail-latency win.
Evidence: `experiments/gpu-physics/artifacts/small-component-workgroups/`.

A dense small-component dispatch experiment passed named-scene, merge/split,
sleep/wake and mutation/growth comparisons, and reduced mixed-stacks launches
from 640 lanes to 320 (300 jobs). It was reverted because near-matched solve
stage timings stayed essentially unchanged (0.3195/0.3185 ms). The component
count/scatter passes still dispatch against allocated contact capacity and loop
over all colors; reducing that excess work is the next candidate. The complete
rejected patch and evidence are in `experiments/gpu-physics/artifacts/dense-small-components/`.

The follow-up live-list bound prototype reduced count/scatter indices from
16,384 to 602 in mixed stacks and passed boundary, scene and mutation/growth
checks. It was also reverted after no useful solve-stage saving was isolated.
A temporary timer split measured component kernels at 0.31–0.41 ms, while
ordinary preparation plus all list construction together took 0.10–0.12 ms.
Those diagnostic timer meanings are explicitly different from the HUD; normal
boundaries have been restored. Evidence: `experiments/gpu-physics/artifacts/live-component-lists/`
and `experiments/gpu-physics/artifacts/component-stage-split/`. The next candidate
was workgroup granularity for the independent small-component solver jobs; the retained size16 result is recorded above.

Static and dynamic graph lists now use three paired stable-compaction passes
instead of six separate passes. Each preserves unique-pair index order. Prefix
counts reuse hash-cell scratch after broadphase has finished; they do not alias
either scatter output. Exact list tests cover zero/partial/full workgroups,
maximum counts, homogeneous and mixed classifications, and stale scratch.
The final default release library suite passes 204/204.
The A/B stage samples save about 0.008 ms with similar surrounding GPU timings;
no frame-rate or CPU-win gain is claimed. The reference kernels and temporary
switch were removed after comparison; artifacts and the A/B patch are in
`experiments/gpu-physics/artifacts/paired-graph-compaction/`.

`GPU_PHYSICS_GRAPH_MEMO=1` adds exact memoization of the dynamic greedy walk
when shared coloring is enabled, the allocated body capacity is at most 8,160,
and the world has no joints or mesh triangles. It compares the ordered live
contact slots, full body IDs, pair identities, initial static occupancy masks
and color counts. Identical inputs reuse the saved color/local assignments and
final masks/counts; changed inputs run the ordinary canonical walk. Contact
forces, manifold preparation, wake/island processing and indirect arguments are
still computed for the current step. Storage is private, capacity-sized, checked
against device limits and reset on simulator growth; default runs allocate none.

Focused comparisons cover exact dynamic schedule output, active named-scene
physics at 1e-5, demonstrated cache hits, filters, teleports, shape reuse, body
types, wake changes and asserted growth beyond 256 bodies. The full release
library suite passes 203/203 with shared coloring and memoization requested
(unsupported worlds still exercise the fallback). Three active stage
pairs halve mixed-stacks graph p50 (0.203→0.103 ms), but short frame trials are
inconclusive and changing Dominoes graphs incur overhead. Five longer mixed
pairs (sleep off, warmup 200, timed 600, step 800) improve every trial's frame
p50: median-of-trial p50 1.937→1.836 ms, p95 13.027→12.753 ms; four of five
individual p95 comparisons improve. This remains opt-in, is a GPU/GPU
comparison, and does not establish the required matched CPU win. Evidence:
`experiments/gpu-physics/artifacts/graph-memo/`.
A later cooperative contact-batch variant reduced graph time further (0.380→0.214
ms) but was reverted after mixed whole-frame results: Dominoes p50 improved while
its p95 and mixed-stacks p50 worsened in five pairs. The existing cache remains.
Evidence: `experiments/gpu-physics/artifacts/batched-graph-cache/`.
A temporary HUD timestamp opt-out preserved completion/capture/failure behavior
but did not improve five paired frame trials (Dominoes p50 2.987/3.085 ms;
mixed stacks 1.955/2.062 ms, on/off). It was removed; timestamps remain unchanged.
See `experiments/gpu-physics/artifacts/hud-timestamp-overhead/`.

Longer temporary frame probes show tails in swapchain acquisition and queue
submission for Dominoes, and often physics encoding for mixed stacks. A mixed
encoding-tail diagnostic measured 7.323 ms wall versus 6.839 ms thread CPU,
usually without context switches; this includes possible driver spinning.
Costs span broadphase, narrowphase and graph encoding, rather than one explicit
completion wait. These diagnostic windows are not CPU-win validation. Probes
were removed; see `experiments/gpu-physics/artifacts/frame-tail-isolation/` and
`experiments/gpu-physics/artifacts/frame-tail-encode-stages/`.
A subsequent five-pair diagnostic removed wgpu's release indirect-call validation
on the fixed scenes. Frame medians improved only about 1% for Dominoes and 4%
for mixed stacks; Dominoes p95 did not improve. The diagnostic was removed and
validation restored. See `experiments/gpu-physics/artifacts/indirect-validation-cost/`.
A resolved-contact-slot shortcut was also evaluated and removed: although the
slot was already available, both direct-load and identity-filtered variants
increased Dominoes narrowphase time from about 0.26 to 0.37–0.38 ms. See
`experiments/gpu-physics/artifacts/resolved-contact-slot/`. Runtime behavior remains
unchanged by that experiment.

The host skips encoding the general static radix rounds only when a conservative proof holds: no meshes, and every writable body has `D_b * N ≤ 1` (or ≤2 with `bounded-static-sort`), where `N` is the count of non-dynamic shapes (static, kinematic, or disabled, matching WGSL `is_non_dynamic`) and `D_b` is that body's attached shape count. With the opt-in, two compound children against one ground satisfy the degree-two bound; a third child restores the general sort. Meshes remain excluded. Within a bounded color, scatter order may vary because writable endpoints are exclusive; per-body rank and color order remain unchanged.

- `GPU_PHYSICS_AB=general-static-sort` and `DIAG_FORCE_GENERAL_STATIC_SORT` force the general path independently of `DIAG_GENERAL_SOLVER`. API diagnostic flags persist when set before lazy GPU initialization and across capacity growth.
- If the GPU graph exceeds the selected degree-one or degree-two host bound, the submission is marked invalid.

Dyn–dyn greedy coloring is a compact serial pair-key walk. Packing those edges by writable island and coloring islands in parallel was measured on mixed stacks and **rejected** (extra compact cost; Dominoes is one component so it cannot skip a 24-color **multi-workgroup** encode).

### Joints

Stored joint anchors are **body-origin**. Solver lever arms are `R * (origin_anchor - local_center)` via `joint_lever`. Do not subtract COM when storing anchors; `stored_origin_anchor` is identity. Mass recomputation must not rewrite stored anchors.

Joint collision filtering uses a host-built canonical body-pair hash (`JOINT_FILTER_CAP`, 32 probes) with the old joint scan as overflow/oracle (`GPU_PHYSICS_AB=joint-filter-scan`).

Endpoints that are static, kinematic, zero-mass, disabled, or sleeping are not writable. `solve_joints` stores only writable endpoints.

Default schedule is compact **parallel** by writable component after island union. Each component owns an ordered list of live joints; the solver iterates only that list. List storage is sized by `max(bodies, joints)` so paired joints (Bridge) are not dropped. If list construction cannot fit, lane 0 solves every joint (fail closed). `GPU_PHYSICS_AB=serial-joints` / `DIAG_SERIAL_JOINTS` forces one-lane serial. A single long chain stays serial under component scheduling. Joint waves still run as separate kernels between contact phases.

Prismatic joints lock the two angular degrees of freedom with the same softness/impulse path as Box3D, not pose slerp.

### Contact waves

Worlds whose host-known shape pair bound `S*(S-1)/2 ≤ 64` (no meshes; joints allowed) encode **only** the existing one-workgroup contact wave per mode: 13 contact-wave dispatches for four substeps, not 325. Joint dispatches stay separate.

Larger worlds retain the existing GPU-gated whole-wave shortcut, then use an adaptive color schedule: overflow first, wide dynamic colors in parallel, the remaining dynamic colors in one workgroup, and static colors 20–22 in parallel. Each tail color processes every contact in batches of 64 with barriers between batches/colors; a stale hint cannot omit constraints. The previous graph reports its last wide dynamic color through the existing status readback (56 bytes, previously 48), without an additional dispatch or wait. Scene uploads reset the adaptive split. `GPU_PHYSICS_COLOR_PREFIX=23` restores the full reference schedule; unset or `auto` selects the adaptive default. Fused TGS remains off.

The v24 regression suite passes 182/182 tests, including overflow, joints, growth and shortcut exclusion. Native Dominoes has exactly matching positions, rotations and velocities for all 5,430 dynamic bodies at step 121 against the full schedule. Five matched trials under variable desktop load reduced median physics-submit p50 from 3.07 to 1.76 ms and frame p50 from 13.53 to 12.80 ms; CPU frame p50 was 8.85 ms. This is not a CPU win. See `experiments/gpu-physics/artifacts/v24-final/result.json`. The next bottleneck measurement must account for GPU graphics/presentation execution outside the physics timestamps; CPU render submission time cannot establish that cost.

Measured and **rejected**:

- Applying the 64-lane fused color loop *instead of* the 24-color path (mixed stacks fall through).
- Skipping the fused prefix on large worlds (box-stack quality).
- Per-island contact-wave replacement of the 24 color encodes as a Dominoes default.

Experimental fused TGS for static-only islands stays **off**.

### Capacity loss

Capacity drops (`ATOM_*_DROPPED`) are **sticky**: per-step counters still clear, but sticky bits, first failing submitted-step ID, and accumulated counts survive later clean steps and fail the run. After a capacity/schedule loss the world refuses further physics submissions until it is destroyed. Clearing diagnostic counters does not rehabilitate it.

## Timing

`b3_world_gpu_wait` waits on the device queue and harvests timestamps; it does not copy the CPU body mirror. Use `b3_world_gpu_wait_with_mirror` or `b3_world_sync_from_gpu` when poses are required.

GPU clock **device** is timestamp 0→6 (broadphase through sleep). Collide is 0→3 (broadphase + narrowphase + graph). **Prepare** is 3→4 (islands/TGS setup). Collide+solve+integrate is **not** the whole device step.

HUD timestamps use a two-slot staging ring: Poll + try_recv only; wait APIs still block. Each HUD sample is tagged with the **submitted physics step** (`gpu_b3_world_physics_step`). A full timestamp ring may drop a sample without changing that ID. `completed_step` / `completed_known` are independent of the ring; unknown completion reports in-flight `-1`. Sokol pick frames also record GPU query wait/dispatch/map and copied bytes.

`--bench` records those stages plus:

- completed-step (submit + wait, no pose mirror)
- encode including `encoder.finish`
- `step_plus_host_mirror` (not Sokol present time)

Five CPU oracle trials use unique temp dirs. CPU latency authority is per-step wall around `b3World_Step`.

Sleep-enabled completed-step benches now honor exactly the requested warmup on
both engines. They record awake-after-warmup and `sleep_window`; `settle_wait_steps`
is zero. The old automatic GPU-only settle wait (up to 200 steps) mismatched the
CPU window and was removed. Historical headless GPU/GPU experiments still compare
their respective recorded windows, but reports with a nonzero settle wait are not
matched CPU comparisons or the requested early window. Native timeline runs were
unaffected. Warmup 200 is not proof of sleep. Run
`python3 experiments/gpu-physics/scripts/check-bench-window.py` after building to
check both sleep modes; evidence is in `artifacts/fixed-bench-window/`.

Native Sokol application timing is `./scripts/sokol-timeline.sh` (`samples:cpu` / `gpu` / `both`, unpaced vs paced, optional `--completed-step` and `--pause-script`). Cadence is callback-entry to next entry, including the Sokol event loop. `sg_commit` is not photon time. Both-mode is two worlds and is not a one-world CPU baseline. The Rust `--native-timeline` remains a secondary viewer diagnostic.

## Results (not a CPU win)

Balanced 5×CPU / 5×GPU `--no-sleep` on RTX 4070 Laptop/Vulkan (`artifacts/v9-spot`, 2026-09-06):

| Workload | bodies | window | GPU completed-step p50 | GPU encode p50 | CPU wall p50 (5 trials) | Win? |
|---|---:|---|---:|---:|---:|---|
| Revolute | 3 | warmup 0, 20 timed | 1.32 ms | 0.51 ms | ~0.004 ms | no (tiny) |
| High Resistance | 11 | warmup 0, 20 timed | **1.28 ms** | 0.47 ms | ~0.009 ms | no (tiny) |
| Dominoes | 5431 | warmup 20, 20 timed | 5.46 ms | 2.31 ms | 4.78–6.04 ms | no (not ≤80% CPU) |
| Mixed stacks | 602 | warmup 0, 20 timed | 4.62 ms | 1.98 ms | 0.68–0.72 ms | no |

`cpu_win_validated=false`. Tiny scenes remain CPU-faster.

Compact parallel joints beat serial on 256 independent pendulums (completed-step p50 8.7 ms vs 12.6 ms). Long chains stay serial inside one component.

### Sokol timeline

v9 numbers in `artifacts/sokol-timeline` were **spot checks** (warmup 6 / timed 12). Reported `render_p50 < 0.002 ms` measured the empty `Sample::Render()` callback, not `b3World_Draw` / `RenderFrame` / `sg_commit`.

v12 five-trial unpaced scopes (warmup 12 / timed 24), observed — not a claimed win:

| Scope | Dominoes GPU (median of trial p50s) | Dominoes CPU |
|---|---:|---:|
| physics (`b3World_Step`) | ~3.5 ms | ~3.5 ms |
| pick (`CastRayClosest`) | ~4.5 ms | ~0.004 ms |
| cadence | ~13 ms | ~8.5 ms |

Dominoes GPU pick waits for physics (`query_wait_ms` ~2.8 ms). Exclusive dispatch ~0.13 ms, map ~0.003 ms, encode ~0.12 ms. Picking now shares the same per-step CCD finalization as getters rather than applying a second correction. `cpu_win_validated=false`. The historical 6.86 ms pick figure is stale.

## Correctness

`experiments/gpu-physics/scripts/correctness-gate.sh` rebuilds the crate **and** the Box3D oracle, fingerprints sources plus both binaries and compiler/adapter (`fingerprint_parts`, `correctness-fingerprint-inputs.txt`), then runs:

- `cargo test --release --lib -- --test-threads=1`
- GPU-vs-GPU self-tests (High Resistance sleep on/off, mixed-stacks)
- C ABI High Resistance, mixed-stacks, overlapping compounds, queries, mesh-queries and height-queries
- native scene matrix (`native-scene-gate.sh`): fresh artifact dir, CPU+GPU JSON, required sample identity / timed / measured, generic finite/runaway plus CPU envelopes. Missing/stale/truncated records fail. Village capacity refuse is **unsupported**. Same-process Village→Bounce House switch is a resource test, not two launches. `--health-scan` is correctness-only and is timed as `health_p50_ms`, not charged into `physics_ms`. Library test counts are not native compatibility proof. `cpu_win_validated=false`. Historical `native-scene-v13` artifacts are stale.
- 3600-step long-runs
- CPU oracle compares for revolute, High Resistance (awake and sleep), and mixed-stacks

Mesh contacts use all hull vertices plus face clipping against each visited triangle. The box path tests triangle, box-face and edge-cross separating axes and clips either the triangle or the incident box face according to the selected reference face. Shallow sideways hull-face candidates are tentative, following Box3D’s mesh-contact classification: confirmed surface/deep contacts take priority regardless of BVH order. Replacing a tentative group clears its points and accumulated material values; a normal is never changed beneath existing points. This remains one normal group per mesh/shape pair, not the full upstream multi-manifold and adjacency pipeline. Multiple meshes store BVH offsets in nodes, not packed-vector units. Speculative keep is `SPECULATIVE` (0.02) unless `SHAPE_DISABLE_SPECULATIVE`. BVH stack overflow and leftover visits report `ATOM_CONTACT_DROPPED`.

Revolute collinearity Jacobians store each 3D axis as a column of a WGSL `mat2x3`. The previous mixed row/column packing injected free-axis rotation while correcting tilted hinges. The regression checks both constrained tilt and unintended free-axis twist in rotated frames.

Prismatic constraints now use a specialized two-axis point-to-line solve with angular effective mass and the moving-line `d + rA` Jacobian, followed in the same phase order as Box3D. Warm starting replays accumulated impulses; solving no longer nudges positions directly or skips transverse velocity constraints when anchors coincide. `scripts/check-prismatic-reference.sh` compares five real CPU/GPU C ABI fixtures over 120 steps (offset anchors, dynamic A, motor, spring and limits), requiring finite complete records and component differences within `1e-4`. The targeted Gear Lift rerun improves peak constrained angular error from 0.1021863 to 0.0669222 rad, but still fails; this is not a native compatibility claim.

Body definitions preserve `enableContactRecycling` through both native adapters and the Rust API. Disabling it on either endpoint forces fresh narrowphase contacts, independently of the global `no-recycle` diagnostic. The default remains enabled. The runtime `b3Body_EnableContactRecycling` / `b3Body_IsContactRecyclingEnabled` entry points also forward to Rust; the dual adapter updates its mapped CPU body as well. Getters and unchanged setters read host metadata without harvesting the world. A changed flag first finalizes pending state, then uploads the modified body, preserving its completed pose and other flags. Repeated toggle tests cover either endpoint and reuse of an existing contact.

Fresh contacts are converted from shape centres to body COM exactly once, in `finish_manifold`; the former second conversion in `collide_pairs` corrupted offset-shape lever arms. Packed `ra.w` changes by the corresponding projection, preserving `s = ra.w + dot(rb-ra, n)`. Tests verify actual world anchor positions as well as that scalar equality, since the equality alone can survive a double conversion.

CCD mirror corrections consume the staged pre-CCD pose snapshot so subsequent pose preparation cannot overwrite the corrected CPU state. Mesh CCD follows Box3D's initial-contact safeguard: if the full-shape sweep starts touching, retry with a small centroid sphere to prevent the centre crossing while a rotating tip remains in contact. A positive TOI is retained even when GJK returns a zero witness normal: mesh sidedness comes from the starting centroid and the response normal comes from the triangle. Conservative-advancement iteration exhaustion returns the last safe fraction rather than an unproven miss. All BVH-selected CCD triangles are processed; the previous silent 4096-candidate truncation is removed. CCD still runs on the host harvest path; these changes do not establish a GPU-resident CCD implementation or a performance win.

The native gate now checks every recorded frame, matching body/joint identities, settings, step sequence and finite transforms. Joint anchor error is screened against matched CPU error plus 0.05m; constrained angular error allows CPU error plus 0.05rad. Free wheel/prismatic travel is excluded from the linear residual. These are explicit screening budgets, not exact trajectory or full constraint certification. Mesh Drop's clock-seeded random setup is overridden only in generated benchmark sources with `GPU_SOKOL_SEED` (gate default 52977); the actual seed is recorded and must match. Interactive samples retain their clock seed when the variable is absent. No upstream sample source is edited.

Latest native matrix (`artifacts/native-scene-20260909-v15-prismatic`, source `3c4c3b58…`): **8 pass, 2 fail, 1 unsupported, 1 incomplete**. Release library tests pass **137/137**, and five 120-step prismatic CPU oracle cases pass. GPU and dual viewers were rebuilt. `cpu_win_validated=false`; these are correctness runs, not performance measurements. The expanded complete correctness gate has not been rerun end-to-end in this continuation; its last checkpoint was **25 pass / 1 fail / 0 skip** (native matrix failure, source `68f8808b…`). Scoped current evidence is in `artifacts/v15-prismatic-validation.json`.

Gear Lift's peak constrained angular error improves from 0.1021863 to 0.0669222 rad, but it still exceeds the matched CPU screening budget. Mesh Drop still pauses at step 80 after losing support. Historical v14 Hit's 49m comparison mixed GPU COM with CPU origin and must not be cited as a launch.

| Scene | Status | Evidence |
|---|---|---|
| Joints/Motion Locks | pass | all-frame matched CPU screen |
| Joints/Gear Lift | fail | step 29 joint 79: angular error 0.057027 vs CPU 0 radians |
| Joints/Driving | pass | all-frame matched CPU screen |
| Joints/Bridge | pass | all-frame matched CPU screen |
| Joints/Ball and Chain | pass | all-frame matched CPU screen |
| Issues/s&box mover | pass | all-frame matched CPU screen |
| Issues/s&box Ghost Collisions | pass | all-frame matched CPU screen |
| Events/Hit | pass | all-frame matched CPU screen |
| Determinism/Wave Pile | pass | all-frame matched CPU screen |
| Continuous/Mesh Drop | fail | frame 78: unexpected physics step 80 |
| Compound/Village | unsupported | compound/scene capacity rejected before a successful Village simulate |
| scene-switch | incomplete | same-process switch completed (switch_count=1); Village remains unsupported |

Missing required oracle compare is fail/incomplete, not pass. A Git hash plus `dirty:true` is not enough.

Compare details:

- High Resistance samples every step in 300..600 (bounce cycles hide in 300-step snapshots).
- `--epsilon` is **not** generic |Δpos|. Compare uses COM-y 0.002 m, support **0.02 m**, speed, and quaternion **component L1**.
- Sleeping bodies keep support/finite checks and skip the no-sleep flat/tilted classification.
- Mixed stacks check layer y, spawn xz, orientation, and settling speed against CPU COM-y 0.002 m.
- Capsule mass uses Box3D `b3ComputeCapsuleMass`.
- Contact recycle/rebase uses manifold endpoint order.

The v31 render bridge exposes immutable collider geometry through `b3_world_render_scene`, keyed by world generation and topology revision. Mesh storage is shared; compounds retain collider/public-owner identities and child ordinals. Body transforms are deliberately absent. `b3_world_render_buffers` returns live GPU buffers and their slot span without a wait, fixing the old renderer's live-count truncation after body deletion. The window and video renderer now use this slot span and an explicit compatibility finalization boundary. The Rust window now consumes the geometry stream through a topology-cached instanced draw pipeline for spheres, capsules, boxes, hulls and mesh triangles. Vertex shaders fetch GPU COM, rotation and local center by body slot; identical geometry shares draws. An offscreen GPU pixel test verifies a nonzero COM offset and a live body beyond a deletion hole. Full Sokol material/lighting/UI parity is not claimed; the video path still uses its legacy primitive geometry. The window fits the initial dynamic-body bounds (excluding giant static supports), provides arrow-key orbit and wheel zoom, and uses body colors to distinguish touching layers. The ground checker now handles negative coordinates consistently and fades at subpixel frequency. Camera fit is tested in portrait and landscape viewports. The v33 preview tightens framing using projected bounds, darkens the ground/lighting, and adds a 2048² directional depth map with filtered ground reception. The GPU pixel test verifies sparse-slot/COM-correct shadow casters. Object-to-object shadow reception, full edge antialiasing and Sokol UI parity remain outstanding.

## Sokol poses

`samples:both` reads GPU poses through `b3Body_GetPosition` / `GetRotation`. Those getters copy a completed pose snapshot **once** per kick into CPU body state (bounded maps).

- Bodies with a newer host setter keep that value; a stale snapshot must not overwrite `SetTransform` / velocity.
- Public getters after a setter return the new value.
- Mapping the whole buffer per shape made Dominoes a slideshow (~300 ms/frame) even when physics timestamps looked fine.
- Linux NVIDIA may export poses over `VK_KHR_external_memory_fd`; **DrawShape still uses the CPU snapshot**, not the imported GL buffer.
- Do not advertise GPU-resident Sokol rendering merely because the FD import succeeded.
- The Rust window draws from GPU `body_states`, but currently finalizes CPU CCD/body state before drawing. This was previously hidden in `b3_world_body_count`; it is now an explicit `b3_world_finalize_render_state` boundary. It is not yet a readback-free frame path.

Sokol timeline `render_p50_ms` is `RenderFrame` CPU time. `draw_p50_ms` is `b3World_Draw` / pose-list work. v9’s sub-0.002 ms “render” column was the empty `Sample::Render()` callback.

For targeted graphics diagnosis, `GPU_SOKOL_GL_TIMESTAMPS=1` enables a Linux GL timestamp ring around RenderFrame, UI and sg_commit. It logs `gl-gpu-frame <frame> <milliseconds>` on stderr; only available results are read, full slots drop samples, and final pending frames are omitted. This excludes the platform swap and reports elapsed GPU time including scheduling stalls, not isolated shader execution. Three alternating v25 Dominoes trials report GPU-mode graphics p50 5.36–5.68 ms and CPU-mode 3.94–6.34 ms, with zero Sokol errors and verified swap interval zero. CPU render submission timings therefore cannot dismiss rendering as a source of contention. Causal attribution still needs a controlled render-pass comparison. Evidence: `experiments/gpu-physics/artifacts/v25-gl-probe/result.json`. The v26 controlled diagnostic (three trials per CPU/GPU mode and shadow/AO combination) confirms that removing AO substantially lowers graphics elapsed and GPU pick time; GPU frames still lose even with AO and shadows disabled. The temporary injection was removed and viewers restored. This rules out rendering quality reductions as the route to the CPU-win goal; remaining current-state mirror/query synchronization needs work. Diagnostic artifacts explicitly exclude certification: `experiments/gpu-physics/artifacts/v26-render-passes/result.json`. The v27 candidate staged cold body data and island IDs alongside poses so current-state picking could reuse one transfer. It passed 15 existing snapshot tests but lost two of three paired whole-frame comparisons; it was reverted. Median frame p50 10.76→10.53 ms was not a repeatable gain, and submission cost increased. See `experiments/gpu-physics/artifacts/v27-snapshot-reuse/result.json`. Further readback micro-optimizations are deferred in favor of investigating GPU CCD finalization without weakening current-state queries. The v28 CCD workload measurement narrowed that proposal: CPU CCD was only 0.62–0.64 ms versus 5.2–5.8 ms completion wait, with zero corrections in two sampled 120-step Dominoes windows (warmup 30 and 300). This does not justify skipping CCD; a full port is deferred until its benefit outweighs its correctness risk. Evidence: `experiments/gpu-physics/artifacts/v28-ccd-work/result.json`. A v29 trial raising the dynamic tail threshold from 64 to 256 contacts was also reverted: five paired runs gave no convincing repeatable whole-frame gain (`artifacts/v29-tail-threshold/result.json`). The retained threshold remains 64.

Native Vulkan requests the adapter’s storage/buffer size (not wgpu’s 128 MiB default). A sample that still exceeds that is a bug: `b3World_Step` keeps running (Sokol/C cannot unwind a Rust panic) and the HUD shows **GPU FAILED** with the heap vs limit.

### v15 contact and terrain validation (2026-09-10)

The v15 checkpoint passed **141/141** release library tests. Prismatic constraints use CPU-reference moving-line Jacobians and impulse warm starts; the five-fixture CPU comparison remains required. Fast mesh contacts bypass recycling using a GPU-computed motion flag, and cached bounds include every child shape (16 extra bytes per body, no new dispatch/binding/readback). Slow/non-mesh recycling and runtime continuous-off are covered. Event endpoint/hit reconstruction now uses stable body slots, with a regression for preceding deletions. Hull–mesh backface rejection includes the CPU's 5 mm tolerance; spheres/capsules keep their strict rule.

The latest full native matrix is **8 pass / 2 fail / 1 unsupported / 1 incomplete** (`artifacts/native-scene-20260910-v15-fast-mesh`). After the event/backside fixes, targeted matched runs in `artifacts/v15-slot-backside-native/result.json` keep Wave Pile passing. Mesh Drop now keeps body 233 supported and settled, but body **842** fails at step **101**. Gear Lift still fails at step 30, joint 78 (angular error 0.0560034 rad). These targeted checks do not replace a fresh full matrix. Village remains unsupported, its switch incomplete, and `cpu_win_validated=false`. `samples:both` has been rebuilt.

`./scripts/check-mesh-impact-reference.sh` is a required correctness-gate case. Both engines start native body 233 from identical frame-55 state on the actual wave mesh. The latest result (`artifacts/v15-mesh-impact-geometric/result.json`) passes terrain support and settling but **fails distinct manifold normals**: at step 58 Box3D retains two non-coplanar patches while GPU retains one. Support is measured against the cooked triangles beneath the body; CPU trajectory deltas are separate diagnostics. The overall oracle remains fail. The validator's synthetic self-check is not physics evidence, and the aggregate correctness gate has not been rerun for this follow-up.

Mesh face retention and bounded point reduction fixed earlier support failures but cannot represent several independent normals/friction states. Next work reproduces body 842's first divergence at steps 57–58, checks missing triangle face/edge/fallback semantics, and then adds proper manifold storage and scheduling. The earlier body-233 fall was resolved by backside tolerance, so missing multiple manifolds must not be assumed to cause every remaining tunnel. Public contact-data stubs and compound/event identity mapping also remain outstanding. No native compatibility or new performance-win claim is made.


### v16 checkpoint: manifold ownership infrastructure (2026-09-10)

The checkpoint build passed **141/141 release library tests** (`artifacts/v16-checkpoint-lib.log`). Body-842 shared-state diagnostics reproduce the native Mesh Drop failure; splitting the same terrain into separate triangle shapes makes it settle. This is diagnostic evidence for preserving multiple contact normals, not a production mesh replacement. The mesh-impact validator explicitly rejects split-mesh diagnostic runs.

Contacts carry explicit manifold links, and every solver entry point visits the chain under one body-pair owner; graph coloring excludes child slots. The bounded GPU allocator reserves primary pairs first, retains old children until history transfer completes, and reports exhaustion rather than dropping constraints. Per-point triangle IDs have separate persistent storage (16 bytes per slot; hot solver storage unchanged). Warm starts require both triangle and feature identity, and clipping/reduction preserve those identities.

Production mesh narrowphase now generates per-triangle candidates, clusters compatible collision and triangle normals, matches old patches once each, and publishes complete chains. Recycling checks and updates every member; sensor overlap checks every patch before retaining one event witness. Contact events aggregate multiple patches under one pair identity. Empty candidates are explicitly rejected before allocation; a regression exercises this between two live patches. The current release library suite passes **153/153** (`artifacts/v16-mesh-multinormal-lib.log`). The full correctness gate is **28 pass / 1 fail / 0 skipped** (`artifacts/v16-multi-manifold-correctness.json`); its only failing case is the native matrix. Both mesh oracles, all CPU comparisons, query cohorts and all three 3600-step runs pass. `correctness-latest.json` reflects this failing aggregate, and `artifacts/v16-multi-manifold-validation.json` records the remaining scope. Two earlier tests assumed one mesh manifold; executed Box3D fixtures in `c_abi/mesh_patch_normals_reference.cpp` demonstrate two, and the GPU tests now verify the corresponding normals without depending on slot order.

The required shared-state body-233 mesh-impact oracle now **passes** distinct normals, terrain support and settling (`artifacts/v16-mesh-guard-oracle/result.json`). At frame 58 the GPU retains both CPU terrain normals; final COM-y differs by about 2.9 mm and tail speed is below 0.00007 m/s. The separate body-842 reproduction also settles through frame 100 (`artifacts/v16-mesh-guard-842.txt`). Its evolved frame-58 normals differ from CPU after the first impact; a replay from identical CPU frame-57 state produces all three CPU normals (`artifacts/v16-842-frozen57/`). The gate now includes that frozen-state fixture so contact geometry is compared before trajectory divergence. Neither reproduction replaces the native Mesh Drop scene. The fresh native matrix (`artifacts/v16-native-result.json`) is **9 pass / 1 fail / 1 unsupported / 1 incomplete**. Mesh Drop completes all 180 measured steps and passes the CPU screening envelope. Gear Lift fails at step 90 (body 175 more than 2 m below CPU); Village remains unsupported. These are screening checks, not full contact/constraint certification. The CPU/GPU/both viewers have been rebuilt. CPU tentative-edge topology filtering, general triangle narrowphase parity, mixed-material cluster policy, compound/event identity, Gear Lift and Village capacity remain work to validate or complete. No native compatibility or performance win is claimed; `cpu_win_validated=false`.

The pre-commit comparison grid has a new `2026-09-10-v16-checkpoint` GPU column (120 frames per scene, one diagnostic metrics run), alongside the existing real Box3D CPU column. These recordings are visual regression evidence, not matched performance trials.


### Contact identity and callback follow-up

GPU collider indices now map to public shapes consistently across events and callbacks: compound parents consume no GPU slot, hidden children report their parent, and body slot holes are preserved. Callback decisions are shared per public pair. Custom-filter-only steps now update completion/pose/event bookkeeping. Pre-solve vetoes validate and clear every owned manifold, invalidate cached geometric misses, and can be reversed while stationary. Host destruction/refilter end events are published without requiring a GPU contact download after the last event-enabled shape disappears. Begin events retain actual GPU start flags, so a first late read does not manufacture a begin.

Current validation is **157/157 release library tests** and **10/10 affected C-ABI cohorts**, recorded in `artifacts/v17-contact-identity-validation.json`; `samples:both` is rebuilt. The full/native gate has not been rerun for this host/API slice; the v16 28/1 aggregate and 9/1 native matrix remain historical evidence. Public contact-data/validity APIs are still stubs. Completing alive-pair IDs, non-touching validity, skipped-harvest event history and manifold exports is the next API work. Gear Lift, Village and measured GPU performance remain unresolved. No new commit or CPU-win claim.


Point-history validation now passes **158/158 release library tests**, both shared-state mesh-impact oracles (233 and 842), and the `samples:both` rebuild. Four existing lifecycle bits preserve actual point matching through publication and recycling, including zero-impulse persisted points; buffer sizes are unchanged. A real CPU fixture confirms the zero-impulse semantics. Evidence: `artifacts/v17-point-provenance-validation.json`.

The skipped-read C-ABI reproduction confirms a remaining event bug: after reading the first begin, separating on step 2 without harvesting, and reading step 3, GPU reports one stale end while CPU reports none (`artifacts/v17-event-history/result.json`). This is a known failure, not a passing compatibility claim. Contact getters remain unfinished and the full native/performance gates have not been rerun.


Checkpoint `0e014bc` includes the mesh/identity/persistence work and 300-frame CPU/GPU comparison recordings. The event-history follow-up preserves previous-step touching keys in the existing GPU broadphase-clear pass, before retirement/reuse. It uses four bytes per contact slot in scratch storage (256 KiB reserved at maximum pair capacity), no additional dispatch or storage binding, and no per-step CPU history download. Explicit contact-event harvest copies the live capacity's history alongside the existing contact snapshot. Immutable public-shape maps preserve the previous dense-index interpretation across compound-child deletion; ordinary steps share these maps without rebuilding them.

`scripts/check-contact-event-history.sh` is now a required correctness-gate case. The executed CPU/GPU cases cover skipped old ends, latest-step ends, separation/re-touch, and an end whose begin was never read. All four pass in `artifacts/contact-event-history-gate/result.json`. The original stale-end failure is retained in `artifacts/v17-event-history/result.json`. Contact-ID lifetime, the public contact-data API, and skipped-harvest sensor/continuous-event history remain separate work; these changes do not establish full native compatibility. The follow-up is uncommitted.


Final event-history follow-up validation: **158/158 release library tests**, **4/4 matched CPU/GPU event-history cases**, **10/10 affected C-ABI cohorts**, and a successful `samples:both` rebuild. Both mesh oracles passed with the new scratch layout before the final host shape-map correction. `artifacts/v17-event-history-validation.json` records the exact scope. No fresh full native gate or performance-win claim; all follow-up changes remain uncommitted.


### Native manifold data and remaining contact API

The shared decoder in `src/api/contact_data.rs` now supplies native-layout manifold data to trace diagnostics: cached/current separation, original COM anchors, final/total impulses, pre-solve velocity, full feature and triangle IDs, persisted bits, and world-space friction/rolling impulses. This avoids confusing the packed solver separation with the native cached separation. Native C++ and Rust layout assertions agree. `scripts/check-contact-manifold-reference.sh` executes the layout/decoding and CPU/GPU persistence checks. The library suite is **160/160**, both mesh oracle comparisons pass, and `samples:both` builds. B3TR format is unchanged.

The six contact getters now use a cached public-pair registry and native manifold data. Explicit reads refresh once per changed world state; repeated getters reuse the snapshot. Public shape ordering follows Box3D, and contact handles do not alias across world-slot reuse. The world contact-recycling distance and standalone body-destruction bridge are implemented as well.

Checkpoint `9259d97` validation: **161/161 release library tests**. The expanded native lifetime gate passes CPU box/sphere/compound and GPU box/sphere, but **GPU compound crashes during factory-built hull import before stepping**. The shim box factory supplies only an AABB, leaving an invalid cooked hull for compound cloning. This is a required failing case, not native compatibility. Evidence: `artifacts/contact-api-reference-gate/result.json`. The earlier stub failure in `artifacts/v17-manifold-data/result.json` is historical.

Compound public-pair lifetime across child gaps and unread steps remains unresolved: physical child contacts are not sufficient proof of public compound-pair lifetime. Skipped-read sensor history, Gear Lift, Village capacity, and measured GPU performance also remain open. No fresh full native gate or performance-win claim applies to this checkpoint.


The follow-up removes the incomplete box/cube/offset-box shim factories and uses the actual upstream geometry factories already compiled into the experiment. This fixes the factory-built compound import crash without changing upstream sources or adding CPU physics stepping. The lifetime fixture now checks cooked hull size, topology, volume, points, and translated cloning before simulation.

Current `artifacts/v17-hull-factory/contact-api/result.json`: **7 pass / 1 fail** (four cases per engine). Both factory cases pass; GPU compound construction and initial contact succeed, then validity fails at the child-gap check (exit 20). This is the public-pair lifetime limitation described above, not the former import crash. `samples:both` rebuild succeeds. The follow-up is uncommitted; full native compatibility and a GPU performance win remain unclaimed.

Factory-repair validation: **161/161 library tests**, latest-step event and manifold gates pass, and both mesh-impact comparisons (233/842) pass. Rebuilt `samples:both` completes High Resistance (2 warmup + 8 measured frames, zero Sokol errors); this short run is a smoke check, not a performance result. `artifacts/v17-hull-factory/result.json` keeps overall status **fail** because the compound lifetime assertion still fails.


### Native compound contact contract correction

Further CPU fixtures and `box3d/src/contact.c::b3CreateContact` / `broad_phase.c::b3PairQueryCallback` disprove the earlier one-contact-per-public-parent-pair assumption. Native compound contacts include **child index** in their key. Overlapping children create separate contact IDs (each may itself have multiple mesh manifolds). A child contact survives a gap while parent fat bounds overlap; touching another child creates a different ID while the old zero-manifold contact can remain valid. Initial outer-AABB overlap without a child candidate creates no contact. The host registry/event union and corresponding GPU-only regression expectations are therefore incomplete compatibility work.

A trial that added one parent ghost pair was removed after these CPU failures (`artifacts/v17-public-proxy/rejected.json`); its green library run is not native validation. The required native fixture now covers separate overlapping-child contacts, initial gaps, unread child switching, and ray identity. The ray path now returns the public parent ID with a stored, stable native child ordinal. Both native import paths pass the original ordinal explicitly; sibling deletion does not renumber it, and newly attached children do not reuse retired ordinals. These findings supersede the prior plan's recommendation for a single public parent-pair registry. No overall pass is claimed.

Validation for the compound-contract correction: **162/162 library tests**; required native API gate **12 pass / 4 fail** (CPU 8/8, GPU 4/8). GPU factory, box, sphere, and compound-ray cases pass. Gap retention and per-child identity cases remain mandatory failures. `samples:both` builds and completes Compound/Simple (2 warmup + 8 measured smoke frames, zero Sokol errors). Source/binary fingerprints and exact scope: `artifacts/v17-compound-contract/result.json`. No changes from this follow-up are committed or pushed.


The child-contact follow-up implements structured `(public shape pair, child ordinal)` keys for contact data and begin/end/hit history. Submitted ordinal maps are shared between steps and rebuilt only with topology, preserving previous-step interpretation through child deletion. Custom-filter and pre-solve decisions distinguish child contacts too; mesh manifold chains remain grouped within their owning child contact.

GPU children now reference bounds-only parent metadata. Initial candidate insertion still visits physical children; retained child roots use parent bounds for disjoint tests and their original geometry for narrowphase. Thus an old child can remain valid with zero manifolds in a gap, while touching a different child creates another ID. Parent records never enter candidate hashing or ray geometry. This adds one existing-format shape record per compound, without another solver contact, dispatch, storage binding, or per-step scene readback.

Required native contact gate: **20/20** CPU/GPU cases, including overlapping children, initial gap, unread switching and retirement/recreation, public ray identity, callback counts, and selective pre-solve veto. Library suite **162/162**; event-history/manifold checks and mesh-impact comparisons 233/842 pass. Evidence: `artifacts/v17-child-contacts/`. This is not full native compatibility: CPU fat-proxy containment/hysteresis is not yet reproduced, topology edits still use the adjacent-observation identity fallback, and Gear Lift, Village, sensor history and performance certification remain outstanding. The earlier rejected single-parent-contact design was not reinstated.


The fat-bound follow-up stores persistent world AABBs in an eight-word-per-shape scratch tail, updated by existing broadphase-clear lanes. Candidate hash ranges and overlap tests use that cache. Standalone/public proxies preserve their fat box until new tight bounds escape; compound child discovery uses raw child bounds. The parameter block remains 256 bytes and no dispatch, binding, or readback was added. Allocation reporting includes the tail.

The executed CPU/GPU per-step movements near x=4.10/4.14 now retain the same contact. A new batched-teleport fixture remains a **required failure**: two SetTransform calls before one step update CPU proxy history twice, but GPU currently sees only the final pose. Native gate **23 pass / 1 fail**; library **162/162**, event/manifold checks, both mesh-impact comparisons, and rebuilt samples:both Compound/Simple smoke pass. Evidence: `artifacts/v17-fat-proxy/result.json`. Scene rebuilds also reset the cache, so unrelated edits and topology-generation continuity remain unfinished. No full native compatibility or performance win is claimed.


### Ordered transform history checkpoint

The GPU now replays each body's ordered host SetTransform history in the existing broadphase pass, with one packed upload per submission. Zero-duration steps retain pending history; callback broadphase passes consume each batch once. Material-only scene uploads preserve unchanged fat bounds using geometry keys instead of invalidating the whole cache. No ordinary-frame dispatch or readback is added.

Current validation: 162/162 release library tests and 31/32 required CPU/GPU contact cases. Batched transforms, callback stepping, dt=0, and friction-only edits pass. The new compound-fat-grow case fails on GPU at retirement (exit 25): after adding 300 unrelated bodies, a far teleport leaves the old contact valid. This checkpoint deliberately retains that required failure. Evidence: `artifacts/v17-transform-history/contact-api/result.json`. Growth must preserve/rebuild contact scheduling and fat-bound history correctly; packed topology identity, full native compatibility, and performance certification remain unfinished. `cpu_win_validated` remains false.


### Capacity-growth history repair

After checkpoint `cc7537a`, GpuSim growth now preserves the occupied-contact count/list needed to revisit retained roots, plus matching fat-bound records and transform-history epochs. Fixed-layout contact metadata and coalesced matching bounds are copied GPU-to-GPU in the existing reallocation submission; body-dependent graph scratch is rebuilt. Normal steps gain no dispatch or readback.

Fresh validation: **34/34 native CPU/GPU contact cases**, **162/162 release library tests**, event-history and manifold gates pass. The growth cases verify retention, far retirement, recreation, and batched teleports after reallocation. Rebuilt `samples:both` completes Compound/Simple with two warmup/eight measured smoke frames and zero Sokol errors. This concurrent-validation smoke is not timing evidence. Fingerprints and scope: `artifacts/v17-growth-history-final/result.json`.

This follow-up is uncommitted. Dense physical contact keys across shape compaction/reuse, sensor history, Gear Lift, Village, full native validation, and performance certification remain open. Preserving bounds by source slot and geometry does not establish replacement-shape generation identity. `cpu_win_validated` stays false.


### Required topology identity regressions

The expanded native contact gate is **40 pass / 2 fail** (`artifacts/v17-topology-persistence/result.json`). Both failures are GPU-only; all 21 CPU cases pass. Deleting an unrelated earlier shape compacts GPU collider indices. After three unread steps, the surviving contact changes ID (exit 73). Reading immediately masks that change through the host adjacency fallback, but a separate fixture proves all four native manifold points persist while the GPU reports a new point (exit 80). Immediate latest-step event counts and same-slot replacement identity pass these fixtures. These results do not prove warm-start impulse parity: the fixture deliberately uses zero gravity to isolate identity and point history.

Do not fix this by relaxing ID/persistence assertions or retaining public IDs indefinitely. Preserve physical contact roots, manifold chains, and point history across generation-aware shape-index remapping. Rebuild the GPU contact hash consistently with remapped packed keys, preserve occupied-root traversal, and maintain old submitted keys for previous-step event decoding. The current growth repair does not solve in-place shape compaction. No runtime correction for these two failures is claimed yet; the required gate now exposes them.


### GPU topology remapping implemented

The required shape-hole ID and point-persistence failures are repaired. Geometry uploads track actual shape slot/generation identities. On compaction or replacement, three GPU passes remap every surviving physical contact key, retire deleted endpoints, and rebuild the root hash. Physical contact slots, manifold links, and point history survive. Occupied-root traversal remains intact; matching fat bounds move through coalesced GPU copies rather than a CPU download. Append-only additions skip the contact-remap passes. Normal stepping gains no dispatch or readback; the existing 256-byte parameter block carries the remap controls.

The remap captures old touching keys before changing them, preserving previous-step event decoding through the next submitted physics step. Zero-duration steps do not advance submitted shape maps. Contact getters after a query-triggered rebuild use the actual GPU slot identities while retaining the old submitted event maps; this fixed an additional executed native failure where getters returned zero contacts before the next step.

Fresh evidence in `artifacts/v17-topology-remap-verified/`: **54/54 native CPU/GPU contact cases**, **162/162 release library tests**, event-history/manifold gates and mesh-impact references 233/842 pass. The native cases cover unread compaction, point persistence, compound contact ownership, growth plus compaction, dt=0, query-triggered uploads/getters, and fresh history for a replacement shape. The identity fixture uses zero gravity, so it does not certify nonzero warm-start impulse parity or whole-scene stability.

The host adjacency fallback remains until broader shape-edit/manifold ownership cases prove it unnecessary. Multiple query-triggered topology rebuilds before one step, changed geometry without a generation change, mutable compound-child lifetime, sensor/continuous event history, Gear Lift, Village, and matched performance certification still need work. No full native compatibility or CPU-win claim; these follow-up changes are uncommitted.


### Native settings and mesh-seam follow-up

Repeated query-triggered topology edits (including slot reuse, growth and dt=0 before the next step) now have an executed passing CPU/GPU regression. The full native matrix on the remapping build remains **9 pass / 1 fail / 1 unsupported / 1 incomplete**, at `artifacts/native-scene-20260910-v17-remap-summary.json`. Gear Lift still exceeds the per-body downward-drift screening threshold; Village is refused and its switch case incomplete. That matrix predates the following setting/seam repair.

Native GPU and both-mode shape import ignored `ShapeDef.enableSpeculativeContact`. Both now forward it into the Rust shape state. The WGSL hull/mesh gate checks both endpoints, matching native's hull-only experimental setting; sphere/capsule speculation is not disabled by it. An executed positive-gap fixture previously retained GPU hull contacts when CPU disabled them, and now passes for either disabled endpoint.

The sphere control exposed a separate missing-contact bug exactly above a shared flat triangle edge. The rounded-shape sample path discarded both triangles after its projected closest point was classified as an edge. It now preserves face support for a projection lying on that boundary, while rejecting outside projections. The native regression requires exactly one sphere contact point, preventing duplicate seam constraints.

Fresh scope: **64/64 CPU/GPU contact cases**, **162/162 release library tests**, mesh-impact references 233/842 pass, GPU/both viewers rebuild. Ghost Collisions explicitly disables speculative contact in upstream; its corrected GPU run completes all **300 measured steps** with zero Sokol errors and passes the matched CPU screening envelope. Evidence and source/binary fingerprints: `artifacts/v17-speculative-fixed/result.json`. No full-native or performance-win claim, and no new commit.

Next work remains native terrain/constraint validation (including Gear Lift's divergent debris path), world-level speculative control (currently a native stub), compound mesh material forwarding, sensor/continuous events, and Village's shared geometry/capacity architecture. The release Village setup contains 40,000 hulls, 10,000 alternating spheres/capsules and 2,500 mesh instances when the building asset loads. Do not merely raise the 4096-child guard while importing a transformed copy of every building mesh; preserve bounded allocation and introduce shared immutable geometry with per-instance transforms and suitable spatial queries.


### Compound mesh material import repaired

The native GPU and both-mode compound import paths now upload each mesh child's complete material table through `child.materialIndices`, which maps triangle-local indices to the compound's shared material entries. Previously only the first/base material survived: an executed high-friction compound-mesh patch kept a sliding box at 1 m/s, while CPU and standalone GPU meshes stopped it. After repair, the box stops on the high-friction patch; a frictionless control retains 1 m/s. An earlier hull child deliberately permutes the compound material indices, covering both sides of the mapping rather than only an identity table. Material upload occurs at import and adds no simulation dispatch or readback.

Validation: **72/72 native CPU/GPU contact cases**, **162/162 release library tests**. Rebuilt `samples:both` completes Compound/Mesh Tile with two warmup and twenty measured smoke frames, zero Sokol errors. Evidence: `artifacts/v17-compound-material-verified/result.json`. This is material/import verification, not full scene stability or performance certification. Changes remain uncommitted; native compatibility and CPU-win flags remain false.

World-level speculative-control audit correction: in the pinned upstream revision, `b3World_EnableSpeculative` stores `world->enableSpeculative` (`physics_world.c`), and `world_snapshot.c` serializes it, but no collision/solver path reads that world field. Shape-level flags do affect hull/mesh collision. Do not implement an invented GPU-wide collision cutoff to make the native world setter appear functional. World snapshot/recording state remains separate unfinished API work. Gear Lift, Village, sensor/continuous event history, general geometry edits and measured performance still remain open.


### Gear Lift support diagnostics

The recorded Gear Lift failure is now accompanied by a reproducible geometry diagnostic, `scripts/diagnose-gear-support.py`, using vertices emitted by the real native `b3CreateRock` (`c_abi/gear_rock_geometry.cpp`). It validates matching trace settings/steps and derives the 32-point stair profile from upstream source. In the remap-build traces, CPU/GPU show 8/9 body-frame vertex intrusions over 2 cm, peaking around 5.54/5.56 cm; neither has debris centers deeper than 2 cm. Body 175 leaves the positive-z open side before the height screen fails. Source/input hashes and limitations are in `artifacts/v18-gear-support/report.json`.

These samples do not prove whole-hull or swept separation. Native acceptance was not relaxed, and the 9/1/1/1 matrix remains a failure. The next investigation requires identical-state impact replays and complete geometry/support checks before changing any trajectory-based criterion. No runtime physics change or performance claim applies to this diagnostic follow-up.


### Identical-state Gear Lift impact replay

`c_abi/gear_impact_replay.cpp`, `scripts/prepare-gear-impact-replay.py` and `scripts/run-gear-impact-replay.py` reproduce the native terrain/rock geometry with recorded pre-impact states. Velocities are restored after shape creation establishes COM; the runner rejects any initial-state mismatch, missing body/frame, nonfinite value or failed process. Four inputs (CPU frame 26 / GPU frame 45, single rock / all 120 debris) each completed 12 steps on both engines. Initial pose/velocity errors are zero at the recorded precision. Evidence: `artifacts/v18-gear-impact-contacts/runs.json` and `impact-comparison.json`, including contact witnesses, separation, impulses and source/binary hashes. The earlier `v18-gear-impact` batch is rejected as identical-state evidence because shape creation changed its velocities.

The CPU-frame-26 single rock develops 0.449 rad/s angular speed after impact on CPU versus 2.865 rad/s on GPU. CPU retains two penetrating points and GPU one; both retain four points overall. The same starting rock therefore diverges without the mechanism or other debris. This localizes a contact-response discrepancy, but does not yet distinguish candidate generation, clustering/reduction, rest offset, and solver contributions. GPU's streaming deepest-first five-to-four reduction differs from native's full-cluster farthest-pair/area selection with separation hysteresis. Do not blindly transplant a serial whole-world reduction or claim this algorithm difference alone is the cause.

These are cold-start diagnostics: mechanism bodies/joints and old contact caches are omitted. No runtime physics changed in this replay follow-up, no native tolerance was relaxed, and no performance claim was made. Gear Lift, Village and full compatibility remain open.


### Gear Lift mesh witness and reduction repairs (v18)

The opt-in `GPU_PHYSICS_AB=mesh-candidates` trace isolates the missing witness: the GPU generates the native second penetrating point within 2.23 micrometres, then loses it during reduction. It allocates a bounded 2048-record tail in the existing atomic buffer only at world creation; no extra binding or ordinary-step readback is added. Overflow records sticky capacity loss and is rejected by the replay runner. `gpu_b3_world_dump_mesh_candidates` is a diagnostic export, not a viewer path. Evidence: `artifacts/v18-mesh-candidates/comparison.json`.

Hull vertex samples now use hull-surface witnesses consistently with face clipping, avoiding midpoint duplicates and incorrect friction lever arms. Bounded five-to-four reduction uses farthest-pair/area selection with native separation hysteresis, retaining the existing deepest-witness safety invariant for nonplanar inputs. This remains streaming per pair, not native's full-cluster algorithm or a serial scene pass. Native mesh rest offset (one linear slop) is applied after clustering. The slow-recycling fixture now starts its tiny sphere at the same positive solver gap above that offset; all original assertions remain.

Targeted validation: 162 existing release library tests plus the new executed GPU witness regression pass; 72 native contact API cases and Mesh Drop references 233/842 pass. Four recorded states, each with CPU/GPU 12-step replay, retain identical initial pose/velocity and mass/local inertia at recorded precision. Source/binary evidence is in `artifacts/v18-gear-impact-final/` and `artifacts/v18-mesh-final-evidence/result.json`.

The isolated impact's GPU angular speed falls from 2.865 to 2.043 rad/s; CPU is 0.449. Both penetrating witnesses and their separations now match closely, but solver response remains wrong enough to investigate. Disabling continuous collision handling changes neither engine's result in this case. Returning selected points in native order alone also does not resolve it. These are partial corrections, not a native compatibility pass. The complete native matrix, viewer and performance measurements have not been refreshed for this build.

The native convenience getters `b3Body_GetLocalRotationalInertia` and `b3Body_GetWorldInverseRotationalInertia` still resolve to zero stubs in this experiment's replay link; do not infer zero solver inertia from them. `b3Body_GetMassData` is implemented and its mass/local tensor matches CPU in all replay inputs. Implement the convenience getters with current-state/lock semantics and native regressions as part of remaining API parity. Next solver diagnosis should compare prepared inertia, softness, normal mass, friction centres and velocities after each substep against native, keeping the same corrected manifold. Gear Lift, Village, full functionality and CPU-win certification remain open; these changes are uncommitted.


### Native inertia getters implemented

`b3Body_GetLocalRotationalInertia` and `b3Body_GetWorldInverseRotationalInertia` now have real Rust/C ABI implementations in both the GPU and combined viewer bridges. The world tensor is reconstructed from the complete symmetric local inverse tensor and synchronized current rotation. Partial angular locks leave the tensor intact, as native does; static, kinematic and fully rotation-locked bodies return zero. The mass-data getter also stops exposing a nonzero inertia for these bodies. Local mass/inertia reads use host-owned metadata and no longer trigger a GPU mirror download; current world-inertia reads still use the existing synchronized mirror path.

The required `scripts/check-inertia-reference.sh` compares 24 native/GPU cases covering body types, partial/full locks and unlocking, teleport, post-step rotation, and zero mass with zero inertia. All pass; maximum absolute tensor-entry difference is 1.5e-5 on entries around 200 (relative float tolerance). `correctness-gate.sh` now requires this check. The release library suite passes 163/163 and the combined viewer rebuilds. Artifacts: `artifacts/v18-inertia-final/`. These checks establish the specified getter behavior, not equality of actual solver preparation or resolution of Gear Lift's excess spin. Positive inertia with zero mass, broader native scenes and performance certification remain separate work.

The rebuilt combined viewer also completes Gear Lift with 2 warmup and 12 measured frames under the normal NVIDIA launcher settings: pose import succeeds, zero GL/Sokol errors. A direct launch without those settings selected AMD OpenGL alongside NVIDIA Vulkan and failed external-memory import; that run is retained separately as cross-adapter diagnostic evidence, not a renderer pass. This short smoke does not exercise Gear Lift's later impact failure.


### Solver input correction and fresh native screening (September 10)

The isolated Gear Lift impact mismatch is resolved in the executed fixture. WGSL indexed body-extra data using the live body count even though the host packs it after the allocated BodyCold capacity. Reading the actual allocation offset restores off-diagonal inverse inertia and motion bounds. Mesh rolling resistance now uses the native full hull inner radius. The same cold-start impact produces 0.448573 rad/s on GPU versus 0.448577 on CPU; maximum state error is 5.13e-6. The required `check-gear-impact-reference.sh` prevents this regression. This supersedes the earlier unresolved 2.043 versus 0.449 comparison above.

Release library tests pass 164/164, contact API controls 72/72, and mesh references 233/842 pass. The fresh frozen-binary native matrix (`artifacts/native-scene-20260910-v18-solver-frozen-summary.json`) reports 10 pass, 0 fail, 1 unsupported and 1 incomplete. Those passes are unchanged CPU screening envelopes, not complete physical certification. Village remains unsupported and its scene-switch gate incomplete. One earlier Gear Lift step-10 capacity failure has not reproduced in three repeats or the frozen matrix; its cause remains open. Sticky reporting now retains individual failure causes. The earlier solver-fixed matrix is explicitly invalidated because binaries changed during its run.

`cpu_win_validated` remains false. Remaining work includes shared Village geometry, complete support/joint/API checks, zero-mass positive-inertia ownership, the generic convex hull rolling radius, and matched repeated performance measurements. GPU solver tracing is opt-in; normal stepping adds no diagnostic readback.


The post-checkpoint continuation (`d2d6540`) corrects generic convex-hull rolling resistance to use one quarter of the live child's stored inner radius, matching native `contact.c`. Bounds remain appropriate for the experiment's analytic boxes; sphere/capsule radius and full mesh hull radius retain their separate rules. The GPU regression uses two compound children with equal bounds and unequal inner radii. This change adds no buffers, dispatches or host readbacks. Checkpoint recordings/native results precede this continuation; they are not validation of the new shader.

Post-checkpoint rolling-radius validation: 165/165 release library tests pass, including the executed child-radius regression. Evidence: `artifacts/v18-convex-rolling/`. Native/performance results for this continuation remain pending.


### Dynamic zero-mass bodies

Zero mass no longer changes a dynamic body into a static solver endpoint. Host topology proofs, joint ownership, GPU islands, integration, force/impulse APIs and continuous-motion selection now use body type/disabled state. Gravity remains suppressed when inverse mass is zero, matching native. `SetMassData` preserves independently specified rotational inertia instead of clearing its GPU diagonal tensor. This permits assigned motion and rotational response with zero translational mass, including motorized joints; zero inertia still permits explicitly assigned velocity.

The required native `check-zero-mass-reference.sh` executes 124 state rows across angular impulse/torque, off-center linear impulse, zero-inertia assigned motion and a revolute motor. Maximum CPU/GPU state error is 1.6089999999996385e-06. Evidence: `artifacts/v18-zero-mass-final/`; the preceding motor failure is retained under `v18-zero-mass-verified/`. No tolerance was widened. This is a bounded native regression, not full compatibility certification.

The earlier full checkpoint gate reports 32 pass / 2 fail: incomplete Village/native compatibility and a source fingerprint change during the run. It is not a clean aggregate pass. Subsequent source changes require a new frozen run. Village shared geometry/capacity, unexplained intermittent Gear capacity failure, wider API/physics parity and GPU performance remain outstanding; cpu_win_validated stays false.


### Shared mesh snapshots and density controls

Host mesh snapshots now share immutable vertex, triangle, triangle-ID and BVH arrays with their owning shape. Repeated query snapshots no longer copy those four arrays. Replacing geometry publishes new arrays; retained snapshots survive replacement and world destruction. The isolated lifetime regression passes. This is storage ownership groundwork: GPU packing still repeats instance geometry, and Village remains unsupported.

`b3Shape_SetDensity` now updates unit-density mass/inertia caches and optionally recomputes body mass/COM; zero-to-positive density and deferred updates are supported. GPU and combined-viewer C bridges are wired. Related corrections keep zero-mass and static/kinematic centers at the native origin, clear mass/inertia when all shape densities become zero, and make velocity setters respect static/disabled bodies, angular locks and sleep wake semantics. The required native `check-density-reference.sh` passes 30 rows covering sphere, capsule, offset hull, static/kinematic types, deferred/zero/restored density and a subsequent step. Mass/tensor comparisons use relative tolerances; pose/velocity use absolute tolerances. Evidence: `artifacts/v18-density-final/`. The combined viewer rebuilds; a combined-mode behavior run is not yet claimed.

The preceding frozen zero-mass full gate is 34 pass / 1 fail / 0 skip with a stable fingerprint (`artifacts/v18-zero-final-correctness.json`). Its failed native matrix includes Gear Lift's divergent escape trajectory and unsupported Village; later density/storage edits are not covered by that aggregate. The linked-stub audit and executed API controls also expose substantial remaining functionality gaps (`artifacts/v18-linked-stub-audit/`, `artifacts/v18-api-controls-before/`). No full native compatibility or CPU-win claim.

Combined storage/density/velocity continuation validation: 166/166 release library tests pass; the isolated shared-storage suite also passed 166/166. The API control probe now observes GPU mass increasing from ~1000 to ~2000 after SetDensity, matching CPU. Remaining API controls still fail and remain in scope. No new commit.


### Identity accessors and body-slot lifetime

The GPU and combined C bridges implement body/shape/joint world lookup, joint type (translated to native enumeration), and joint endpoints. These and validity/shape-owner reads use host metadata without harvesting pending GPU state. Body slots retain a generation counter across deletion; reuse increments it. Internal shape-to-body paths reconstruct the live generation. Destroying a body now removes its attached joints and rejects stale body IDs, preventing old constraints or handles from acting on a replacement body.

The native identity fixture covers all nine joint types, endpoint generations, world IDs and attached-joint destruction. It is required by `check-joint-metadata-reference.sh`. A GPU regression repeats accessors/validity reads during a pending submit and requires the mirror to stay stale, then checks deletion/reuse and stale-handle containment. These changes do not complete joint setters, per-body sleep APIs, other remaining stubs, Village instancing or performance certification.

Identity/lifetime continuation: 167/167 release library tests and 20 exact native control rows pass; combined viewer rebuild succeeds. Evidence: `artifacts/v18-joint-metadata-final-controls/`. No additional commit or broad compatibility/performance claim.

World maximum linear speed now honors `b3WorldDef.maximumLinearSpeed` and runtime `b3World_SetMaximumLinearSpeed`/`GetMaximumLinearSpeed` in GPU and combined C bridges. Integration reads a per-world uniform instead of hardcoding 400. Configuration access does not harvest the pending body mirror. The 256-byte parameter block retains its size, with an explicit offset assertion for the new field. Required `check-speed-limit-reference.sh` passes 12 native cases (dynamic, kinematic, zero-mass, motion-locked; creation/decrease/increase), comparing actual integrated positions and velocities with maximum absolute difference 1.9e-6. The combined viewer rebuild passes; combined-mode runtime behavior is not claimed. This is a bounded API correction; Gear Lift, Village, remaining stubs, joint collision lifecycle and matched GPU performance still require work. `cpu_win_validated` stays false.

Speed-limit validation completed: release library tests 167/167, pending-state metadata regression pass, and native speed controls 12/12. Evidence: `artifacts/speed-limit-reference-gate/`. No fresh full native-scene matrix or matched performance benchmark was run for this change.

Joint collision controls now implement native `SetCollideConnected`/`GetCollideConnected` in GPU and combined bridges. Filter joints follow their collision flag rather than unconditionally blocking. Disabling collision dispatches a GPU mutation over unique contact roots, retires each matching manifold chain and pair-hash entry, invalidates public contact IDs, and queues end events for the next step. Re-enabling rebuilds filtering inputs; disable/re-enable before a step cannot resurrect an old ID. Metadata reads and no-op setters do not harvest GPU state; actual mutations currently harvest pending state/events first. The mutation is queue-ordered and does not advance a physics step or wake endpoints explicitly.

The required native `check-joint-collision-reference.sh` passes eight observations covering two joints sharing a pair, filter enable, no-op retention, immediate contact capacity/data removal, old-ID invalidation and deferred end/fresh begin events. A GPU manifold regression verifies child retirement, preservation of unrelated roots and fail-closed malformed ownership. Combined viewer rebuild passes. This does not complete joint creation/destruction lifecycle parity, other API stubs, Village, Gear Lift or CPU-win validation.

Joint collision validation completed: 168/168 release library tests, eight exact native lifecycle observations, and combined viewer rebuild pass. Evidence: `artifacts/joint-collision-reference-gate/`. No new full native scene matrix or performance certification is claimed. The previous checkpoint remains `f6aa597`; speed-limit and joint-collision continuation changes are uncommitted.

The GPU now honors the world restitution threshold at creation and through native runtime setters/getters. All restitution solver paths use the per-world uniform instead of a hardcoded 1.0; negative setter values clamp to zero like native. Configuration access does not harvest pending GPU body state. The existing 256-byte parameter block retains its size and asserts the new field's byte offset. Required `check-restitution-threshold-reference.sh` compares actual impact velocity/position below, exactly at, and above the threshold, including runtime changes after GPU initialization. This implements a missing control; it does not establish full scene compatibility or a CPU performance win.

Restitution validation: 24/24 native impact cases pass (maximum absolute state difference 2.36e-10), release library tests 168/168, combined viewer rebuild pass. Evidence: `artifacts/restitution-threshold-reference-gate/`. A fresh full correctness run for accumulated world/joint controls is running with frozen implementation sources; its intended output is `artifacts/v18-world-controls-correctness.json`. Do not treat that aggregate as passed until the process completes and its result is inspected. Preserve all remaining native functionality, Village/shared geometry, Gear support, scheduling and performance requirements.

The translated/mirrored compound mesh candidate has been promoted to main. GPU mesh rays now use body-origin-relative vertices/BVH bounds consistently; baked negative-scale mesh instances retain winding and inverse edge flags. The required release `check-compound-mesh-reference.sh` passes 32 ray observations and 16 support cases (horizontal/oblique, all eight sign combinations, parent/child rotations, public compound IDs, freed source mesh lifetime). Maximum ray difference is 1.49e-7; maximum support difference is 3.58e-7 m. Evidence: `artifacts/compound-mesh-reference-gate/`; promotion is recorded separately from the earlier isolated stage reports.

The preceding frozen full gate completed 39 pass / 1 fail / 0 skip with a stable fingerprint (`artifacts/v18-world-controls-correctness.json`). Native compatibility remained false: Gear Lift stopped at step 10 with one contact loss, and Village remains unsupported. New contact-drop reason bits distinguish 14 allocation/ownership/graph/traversal sites; their map is `artifacts/v18-contact-drop-reasons/reason-map.json`. The existing asynchronous status transfer grows from 24 to 28 bytes, with no extra map, submission, buffer, or scene mirror. Explicit live-stat reads also include this word. Fail-closed behavior is unchanged.

Four subsequent instrumented Gear Lift runs all reached 122 submitted steps and passed the unchanged CPU screening envelope (`artifacts/v18-contact-drop-reasons/gear-runs.json`). This does not resolve the intermittent step-10 loss or certify constraints/support generally. Retain its failed artifact and use the new cause bits on recurrence. Village geometry sharing, remaining API semantics and measured GPU performance remain required; cpu_win_validated remains false. Current continuation is uncommitted.

Post-promotion release validation: 168/168 library tests and combined viewer rebuild pass. Both async sticky-cause reporting and malformed-chain cause reporting are exercised. Evidence: `artifacts/v18-contact-drop-reasons/validation.json`. No process from this validation remains running.


Checkpoint query follow-up: the expanded translated compound-mesh regression now exercises overlap, callback raycast, and mover queries. Removing a duplicate mesh-center offset from host BVH traversal restores all three hits, but the regression still fails public compound identity (GPU returns an internal child ID). `artifacts/v18-host-mesh-before/` preserves the missed-hit baseline and `artifacts/v18-host-mesh-fixed/` preserves this partial fix. These are failing diagnostic artifacts, not a passed gate. Public identity/child metadata is the next correction. The earlier 168-test result predates this host-query change. Full native compatibility and CPU-win validation remain false.


After checkpoint `8b9d0ee`, host query snapshots retain internal collider IDs for geometry/cache updates and carry public compound IDs plus child ordinals separately. Overlap callbacks now run once per public shape; compound AABB queries use union bounds; callback ray/shape casts select the closest child before invoking the user; mover results batch child planes under the public shape (native limit 64). Query groups are cached with topology. CastMover traverses mesh triangles instead of casting against a convex envelope of the mesh. The GPU-only closest-hit path is unchanged.

The required compound query native control matches callback counts, two-plane batching and public identity, plus exact opposite-direction ray fraction 0.25, child ordinals 0/1 and material IDs 100/101. The expanded compound mesh control passes 32 closest-ray observations, 32 host query/refit cases and 16 physical support cases (all mirrored scale signs and both orientations). Evidence: `artifacts/compound-query-reference-gate/` and `artifacts/compound-mesh-reference-gate/`. A separate translated overlap diagnostic exposes a native compound-tree coordinate discrepancy; it is retained in `artifacts/v18-host-mesh-refit/`. The parity refit control uses equivalent origin-relative query coordinates and keeps full hit/ID checks. These bounded controls do not establish full native compatibility, Village support or a CPU performance win.


Post-checkpoint query validation is complete: 168/168 release library tests, combined viewer build, required compound callback/identity control, and expanded compound mesh control pass. The mesh control also executes 16 mirrored/oblique mover casts against native (maximum fraction difference 2.980000000096794e-07). Evidence: `artifacts/v18-query-continuation/validation.json`. No full native matrix or matched performance rerun is claimed. This continuation remains uncommitted after checkpoint `8b9d0ee`; Village, Gear contact-loss diagnosis and wider API/physics parity remain open.


Static-only shape type rules now match native: bodies owning baked compounds or height fields reject dynamic/kinematic conversion before solver ownership changes. Native GPU/combined bridges now honor BodyDef.isEnabled and expose generation-aware Body/Shape_IsValid rather than validity stubs. Required `check-static-shape-type-reference.sh` compares 12 compound/height/sphere cases across enabled/disabled bodies plus stale/null-ID checks following four legal sphere transitions. Evidence: `artifacts/static-shape-type-reference-gate/`; baseline mismatches are retained under `artifacts/v18-static-shape-types/`. Native rejected type changes leak the world's locked flag and refuse teardown; this native defect is documented, not reproduced in the GPU implementation. Full native compatibility and CPU-win validation remain unproven.


Static-shape continuation validation: 168/168 Rust library tests pass; the final native control passes 12 type cases, four legal-transition ID-lifetime checks and four initial awake/enabled combinations. The final C initialization guard only calls SetEnabled when disabling, preserving initially sleeping bodies; the native control and combined-viewer rebuild pass after that guard. Evidence: `artifacts/v18-static-shape-types/validation.json`. These changes remain uncommitted. No new native-matrix or CPU-win claim.


The shape-less timeout was localized with GDB to NVIDIA create_compute_pipeline inside GpuSim::new. An opt-in GPU_PHYSICS_TRACE_PIPELINES trace recorded collide_pairs compilation at 40374.855 ms. It eventually returned GPU kinematic velocities 2/3 versus native 1.91879284/2.80806375; this was pipeline startup plus a real damping mismatch, not a demonstrated GPU queue hang.

Creation damping is now wired through both C bridges. Kinematic velocities receive damping on the GPU, with inverse mass still suppressing gravity; static creation velocities are zeroed like native. WGSL quat_rotate now uses native's cross-product form, avoiding the homogeneous formula's small axis-speed scaling when quaternion length rounds away from unity. Required body-damping native controls pass 36 pose/velocity/rotation observations across creation/runtime settings, all three body types and with/without shapes (maximum state difference 1.91e-6 at unchanged 1e-5 tolerance).

Collision pipelines are lazy and cached per simulation with a mesh-free specialization and a general mesh variant selected from live triangle count. Later mesh introduction selects/compiles the general pipeline even if geometry capacity is unchanged. A geometry switch test passes primitive→mesh→primitive support and compiles two variants; the extended test also removes all supports and verifies free fall/contact retirement. With fewer than two collider slots, no collision pair exists, so only collision dispatch is skipped; allocation, retirement and GPU integration continue. Unexpected nonempty mesh use of the mesh-free variant sets contact-loss reason bit 14. This is not a CPU fallback or a CPU-win claim.

The frozen damping/query checkpoint gate has completed: **43 pass / 1 fail / 0 skip**, stable source fingerprint, with **169/169** release library tests. The native matrix is **10 pass / 0 fail / 1 unsupported / 1 incomplete**: all ten supported scenes pass the existing all-frame CPU screening envelopes, but Village still exceeds the child cap and the successful same-process switch cannot certify an unsupported Village. These envelopes are not full constraint/support proofs; the earlier intermittent Gear Lift contact loss remains unresolved despite this passing run. Evidence: `artifacts/v18-damping-pipelines-correctness.json` and `artifacts/native-scene-20260910-v18-damping-summary.json`. Full native compatibility and `cpu_win_validated` remain false.

A separate executed direct-shape ray probe finds native compound hits (including disabled bodies) while the GPU bridge returns misses: `artifacts/v18-direct-compound-ray-before/`. Next, resolve public compound handles to their children for direct ray queries, preserve child/triangle/material indices, and keep direct disabled-shape queries distinct from world-query filtering. Validate against native before expanding this API path. Village still requires shared immutable geometry with per-instance transforms, bounded packing and lifetime checks, followed by the unchanged full scene and switch.

Pre-commit comparison recordings completed for all 13 scenes, 300 frames each, with the real Box3D CPU column first and GPU label `2026-09-10-v18-query-damping`. All 26 clips have 300 reported video frames. Recording metrics use one run and are not performance certification. Evidence: `artifacts/v18-query-damping-recordings.json`.

After checkpoint `0075c2e`, direct `b3Shape_RayCast` resolves public compound handles in one synchronized lookup, returns the nearest child, and remaps mesh-local material indices into the compound material table. The C and combined bridges retain each child's four-entry material map. Native direct sphere rays return initial-overlap hits at fraction zero, including zero-length interior rays; the direct API now preserves that behavior separately from GPU world closest-hit picking. Direct shape/body snapshots include disabled bodies, while world-query snapshots continue to exclude them. Public compound density reads now use generation-checked metadata without a pose/contact download.

Validation: **170/170** release library tests, combined viewer build, **99** direct native ray cases plus **9** world-filter/density checks, and the existing compound world-query and translated/mirrored mesh controls pass. The direct control covers initial overlap, zero length, surface entry/exit, finite segment misses, parent teleport/rotation, disabled bodies, and child/triangle/material indices (maximum hit-state error 9e-7). The new lifetime regression rejects wrong-generation and destroyed/replaced parent handles. Evidence: `artifacts/v18-direct-ray-validation.json`. Follow-up changes are uncommitted; no new full native matrix or matched performance run is claimed.

Next: audit the combined importer's negative-scale mesh bake against the GPU-only shim before expanding shared geometry. Source inspection shows `both_dual.c` still passes original triangle winding/flags after baking negative scale, while `shim.c` reverses winding and uses inverse flags. This is a source finding, not yet an executed combined-mode failure. Prefer one shared checked importer so these paths cannot diverge, and add an actual combined-mode mirrored-mesh control. Direct compound lookup currently scans host shape slots once; cache generation-safe child membership or use the forthcoming instance index as part of Village scaling. Other public compound AABB/metadata operations, per-triangle user materials in host world callbacks, mesh/mover coverage and full API/physics/performance requirements remain open.

The combined viewer's mirrored-mesh importer now uses the same checked bake as the GPU-only bridge (`c_abi/compound_mesh_bake.h`). Before the repair, real combined-world controls lost support in all four negative-determinant orientations (about -19.76 m); after it, both paths pass all 32 mirrored/oblique ray observations, 16 support cases, host-query/refit cases and 16 mover casts. The test links the real combined and sample APIs and checks CPU/GPU object mappings. It does not substitute render-hook stubs or test only the GPU shim.

Allocation/attachment failure is explicit: either a missing compound parent, a missing child, a rejected child attachment or a failed mesh bake returns a null shape and invalidates the GPU world. Five injected failures plus a successful import/teardown control pass for each backend. The shared bake passes eight orientation and three allocation-failure checks under AddressSanitizer/leak detection. Required gate scripts cover these controls. Geometry is still baked per instance; this change does not claim Village scalability.

Fresh validation after the interrupted build: **170/170** release library tests, **99** direct native ray comparisons and **9** world-filter/density checks, both GPU and combined mesh references, failure controls, and the combined viewer build pass. Evidence: `artifacts/v18-shared-import-validation.json`. These continuation edits are uncommitted. The latest full native matrix remains the checkpoint's 10 supported-scene screening passes plus unsupported Village/incomplete switch; no full-matrix or matched CPU-win rerun is claimed here. Next is owned shared raw geometry with per-instance transforms and deduplicated GPU packing, followed by actual capacity/lifetime/Village/switch tests before raising limits.

Raw mesh-instance core is now implemented in Rust/WGSL. `b3_create_mesh_instance` shares immutable vertex/triangle/triangle-ID/BVH Arcs and supplies independent affine transforms and materials. GPU scene packing deduplicates those geometry ranges by owned storage identity. ShapeGpu is 176 bytes (44 words), with asserted affine-field offsets; legacy baked meshes retain a separate flag-zero interpretation. Host mesh triangles/proxies/BVH queries and GPU mesh vertices/rays/BVH traversal apply the affine transform consistently. Reflections select reversed winding and inverse edge flags without copying shared triangles. Geometry replacement leaves clones and retained snapshots on their original data.

The executed raw-instance regression covers eight nonuniform/reflected instances, public compound child IDs/materials, host/GPU rays and 120-step physical support. Shared GPU **live counts** stay at 3 vertices/1 triangle/1 BVH node; replacing the source introduces a second range (6/2/2), and destroying it returns to 3/1/1 while clones survive. This is a deduplicated live-range proof, not a measured allocated-memory or frame-rate win. Release library tests **171/171** pass, and both legacy GPU-only and real combined mirrored-mesh native controls pass with the new layout. Evidence: `artifacts/v18-instance-core-validation.json`.

Next, connect the common C importer to the raw-instance constructor with an import-local bounded geometry cache, retaining real CPU/combined controls. Check transformed bounds for finite results before exposing the constructor through C; finite inputs can still overflow during affine arithmetic. Avoid material or identity aliasing, temporary source-shape leakage and quadratic pointer lookup. The current native importer still bakes geometry, and Village's cap remains in place. Full native matrix and matched performance have not been rerun for this core layer; compatibility and CPU-win claims remain false. All continuation changes remain uncommitted after `0075c2e`.

Pre-commit comparison recordings for the shared-mesh core: real Box3D CPU first, GPU `2026-09-10-v18-shared-mesh`, 13 scenes each and 300 frames per clip verified with ffprobe. Recording metrics use one trial and do not certify performance. See `artifacts/v18-shared-mesh-recordings.json`. Native compound importer integration remains the next step.

The native GPU-only and combined compound importers now create raw mesh instances through an import-local, bounded hash cache. Each distinct native mesh pointer is copied into one temporary GPU source shape; children retain its immutable geometry, and source shapes are destroyed on success and every subsequent import-failure exit. Native pointers are never retained after the import. Per-child transforms, materials and public compound ordinals remain independent. The old bake helper remains a tested reference utility, not a production fallback.

The instance constructor rejects quaternion-norm overflow and non-finite transformed corners before attaching a shape, and computes bounds without unnecessarily overflowing sums. The cache has a required sanitizer control for repeated sources, allocation/source/clone failures and idempotent cleanup. This does not raise the Village cap or establish a native-frame performance win. Follow-up: measure actual shared geometry/capacity totals on the unchanged Village, resolve broadphase limits and validate Village→Bounce House before lifting support restrictions.

Native raw-instance integration validation: **171/171** release library tests; GPU-only and real combined native references with two shared mesh children; direct-ray parity; explicit import-failure controls; cache AddressSanitizer/leak checks all pass. Evidence: `artifacts/v18-native-instance-validation.json`. Full native matrix, actual Village allocation and matched performance were not rerun. Integration changes remain uncommitted after checkpoint `856c07a`.

Village diagnostics after native raw-instance integration: the unchanged 52,500-child scene imports under the explicit `GPU_PHYSICS_AB=large-compound-import` diagnostic override. Default support remains capped at 4096. Executed allocation telemetry shows 52,501 packed shapes, 2,678 mesh vertices, 4,370 mesh triangles and 1,571 mesh nodes, with a 12,257,472-byte packed scene heap. This is actual shared geometry packing, not the previous derived estimate. The initial idle probe completed three frames without gpu_fail but had no dynamic bodies and therefore does not validate physical support. Its process peak RSS was 1,581,428 KiB including the native viewer and shader compilation, not just scene storage.

Host query work now uses existing BVH candidates for shape overlap, shape cast and mover-plane queries. Snapshots retain collider order and public compound grouping and leave the world lock before callbacks. A 512-child regression checks full-scan hit agreement, candidate count and post-teleport refit. Unchanged body poses no longer force a query BVH refit. Release library tests 172/172 and native compound query / GPU+combined mesh controls pass. An opt-in `GPU_SOKOL_VILLAGE_DROP=1` fixture adds an identical radius-0.5 dynamic sphere at (0,30,0) to the unchanged Village in either native backend; ordinary viewer runs are unchanged.

Next correctness/performance work must retain the full objective:
- Finish the 240-step native CPU/GPU Village drop comparison and same-process Village→Bounce House; record mismatches rather than treating the idle scene as supported.
- Cache public compound bounds and prune mover casts while preserving the native filter callback even when the public bounds overlap but no child does. Do not simply use child candidates and silently drop that callback.
- Profile startup: C hull/mesh/height mirror lookup currently scans fixed 65,536-slot arrays. Replace with a generation/world-safe bounded lookup only after lifetime and multiworld controls; do not assume this explains all shader startup time.
- Callback ray reentrancy is repaired using owned BVH candidate snapshots. Keep native controls for callback state reads, nested closest/callback rays and clipping. Arbitrary mutation/destruction during native callbacks is not claimed as a supported contract.
- Validate actual dynamic broadphase insertion/pair/contact usage before raising the compound cap. A static-only Village cannot prove those capacities fit, and the serial large-static scan is not an effective final GPU broadphase.
- Full native scene matrix and repeated matched performance remain outstanding; compatibility and cpu_win_validated remain false.

Bounce House CCD correction: fresh and switched GPU traces were identical, ruling out scene-switch lifetime as the cause of the lost tangential step. Conservative advancement and root refinement now target the actual TOI separation instead of the outer tolerance shell; initial-contact tolerance and conservative iteration-exhaustion handling remain. The failing wall regression returned fraction 0.7018745 before the fix versus native target 0.7025. The rotating Mesh Drop test formerly demanded fraction <0.01; an executed native `b3TimeOfImpact` probe returned 0.0439814366, so that assertion now checks the native-backed interval and positive target separation instead.

Validation: 173/173 release library tests; native Bounce House 120-step maximum position difference 0.00063229 m and no lost tangential step; 180-step native Mesh Drop passes the existing CPU screening envelope (CPU minimum y -0.4174, GPU -0.4111). Evidence: `artifacts/v18-ccd-target-validation.json` and `artifacts/v18-ccd-target-oracle/`. Bounce House raw harness status is still incomplete because the generic 120 m/s health limit rejects the native 169.7 m/s initial velocity in both backends. Do not mistake the separate trajectory comparison for a repaired general harness.

Next: calibrate scene-aware health validation using the executed native reference; replace the misleading unpopulated GPU contact counter; address large-static GPU scheduling and remaining Village query/startup costs. Also inspect non-closing fixed-orientation sweeps near the tolerance shell so conservative iteration exhaustion cannot unnecessarily truncate tangential motion. Full functionality, native compatibility and CPU performance certification remain incomplete. Changes remain uncommitted after 856c07a.

Callback ray continuation after the laptop restart: the corrected C probe timed out (exit 124) at its first body read before the fix. The earlier interrupted probe used a missing GPU C helper and is not deadlock evidence. Callback rays now capture only owned BVH candidates, preserve collider/public-child order, and release the world mutex before exact traversal and external callbacks. Immutable mesh geometry remains shared. Query profiles start fresh, exclude callback duration from exact-test time, and publish through generation-checked world lookup; the last completed query owns the profile after nested queries. Blocking GPU closest-hit picking is unchanged.

`scripts/check-query-reentrancy.sh` runs bounded native CPU and GPU controls for callback body reads, nested closest-hit and callback rays, and continue/terminate/ignore/clip responses. It is a required correctness-gate entry. The owned-snapshot regression checks candidate pruning, public compound grouping, refit and survival after index destruction. Arbitrary mutation/destruction inside native callbacks and a GPU performance win are not claimed.

Callback-ray validation: **174/174** release library tests pass; bounded native CPU/GPU reentrancy and native compound-query reference controls pass. Evidence: `artifacts/v18-query-reentrancy-validation.json`. Full correctness gate/native scene matrix and performance were not rerun for this slice. Remaining priorities are scene-aware Bounce House/switch health validation, truthful contact telemetry, near-shell non-closing CCD sweeps, and large-static GPU/query scheduling. Changes remain uncommitted after `856c07a`.

Native scene-health validation now records the actual scene on every frame, including after switches. Bounce House alone permits up to 180 m/s because its native initial velocity is (120,0,120); validation additionally requires one dynamic sphere in the native enclosure at y=4 and speed within 0.05 m/s of sqrt(2)*120. Other scenes retain the 120 m/s screen. The GPU hook derives scene health from finite/pose/speed measurements instead of inheriting the Rust scanner's generic 120 m/s explosion label. The engine scanner itself is unchanged. The same-process gate now requires complete phase identities, 8 Village steps and a reset to Bounce House steps 1–20; merely counting a switch is insufficient.

Fresh CPU/GPU correctness probes pass: Bounce House 120 steps, maximum position difference 0.00063229 m; same-process Village→Bounce House, maximum post-switch position difference 0.00004196 m and velocity difference 0.000412 m/s. Eleven negative controls reject missing frames, wrong identities/step reset, incomplete status, capacity loss, an empty refused Village, non-finite state, energy loss, enclosure escape, tangent trajectory error, and applying Bounce House's speed exception to Events/Hit. The first recording attempt had malformed scene-field JSON and is retained as invalid evidence under `artifacts/v18-scene-health`; only `artifacts/v18-scene-health-verified` supports these results. Aggregate/source fingerprints: `artifacts/v18-scene-health-validation.json`.

Reproduce after building both viewers with `python3 scripts/scene-health-probe.py <fresh-output-dir>`. The script uses the explicit `large-compound-import` diagnostic override, hashes binaries before/after each run, checks matched CPU trajectories and runs negative controls. Default Village remains unsupported at the 4096-child cap; the 8 static Village frames do not certify settled dynamic support or large-scene performance. Full native matrix and CPU-win certification remain outstanding. Next: replace unpopulated contact telemetry, prove non-closing near-shell CCD sweeps cannot freeze tangential movement, and improve large-static GPU scheduling/query costs. No commit or push.

Near-shell CCD continuation: a fixed-orientation sphere sweep parallel to a wall at core separation 0.496999741 incorrectly exhausted 512 conservative-advancement iterations and returned a false hit at fraction 0.51194763. Executed native Box3D reports separated at fraction 1; a nearby closing control reports a real hit at fraction 0.00334964064. The engine now verifies support extrema on a single separating axis at both endpoints. For exactly fixed orientations (including opposite quaternion signs), support points translate linearly, so separation beyond the TOI target plus a scale-aware roundoff margin proves the entire sweep is separated. Approximate GJK distance alone is not trusted as that proof. Any real rotation, including one small enough for acos(dot) to round to zero, retains ordinary conservative TOI. Unresolved iteration exhaustion remains conservative.

Validation: **175/175** release library tests; regression controls for reversed body order, common translation, quaternion signs, genuine closing motion and tiny rotations; fresh native Bounce House 120-step comparison passes (max position difference 0.00063229 m); native Mesh Drop 180-step CPU screening envelope passes (CPU min y -0.4174, GPU -0.4111). Evidence: `artifacts/v18-ccd-parallel-validation.json`, `artifacts/v18-ccd-parallel-oracle`, and `artifacts/v18-ccd-parallel-native`. This is a host CCD correctness/work-reduction repair, not a measured native-frame speedup or a migration of CCD to the GPU. Full native compatibility and cpu_win_validated remain false.

Next performance/telemetry action: `b3_world_counts` currently uses `with_world`, which calls `ensure_cpu_mirror_world` despite body/shape/joint counts already being host topology. Remove that unnecessary synchronization with a copy-count/no-harvest regression, count only live joints, and provide contact telemetry with explicit submitted/completed step identity. Do not replace the current fake-zero contact count with a stale value presented as current. Then use truthful capacity/workload evidence to continue large-static GPU scheduling and Village support. No commit or push.

World topology counters now use a no-sync world lookup: body/shape/joint reads do not download the body mirror, harvest CCD/events, or mark a physics submission complete. Public compound parents count once and hidden collider children do not inflate shapeCount; JOINT_NONE holes do not inflate jointCount. Both regressions failed before the fix. Validation: **177/177** release library tests, including 1000 repeated reads preserving the exact pending state, and native CPU public-count comparisons through the same GPU FFI used by samples_api.c. The fixture explicitly links the GPU joint-destroy FFI because that C wrapper resides in samples_api.c rather than the core shim archive. The bounded `scripts/check-world-counters.sh` is a required gate entry. Evidence: `artifacts/v18-world-counters-validation.json`. No native-frame speedup is claimed without new measurements.

Contact telemetry audit: `LiveStepStats.live_contacts` currently reads SCR_NCONTACTS, which compact_unique_bases assigns the bounded unique candidate-pair count. It is neither active patch count nor native contactCount. Native Box3D contactCount is the live contact-ID pool size, including non-touching allocations; compounds introduce additional public-versus-child identity requirements. Do not expose candidate pairs as contact counts. Next implement distinctly named GPU metrics (candidate pairs, occupied roots, active patches, unique public contacts), return submitted/completed sample identity and topology revision, and add fresh no-contact/compound/multi-patch/retirement controls. Reuse the existing compact async status readback where possible, with explicit unknown/stale semantics. A synchronous public native counter must not pretend an older async sample is current. The current samples_api.c contactCount zero remains unimplemented, not validated evidence. Native functionality, Village support, large-static GPU scheduling and CPU-win certification remain unfinished. No commit or push.

GPU contact metrics core now reuses the occupied-contact collection pass to report distinct unique candidate pairs, allocated roots, allocated root/child manifold slots and touching roots (including sensors). Per-workgroup sums limit the added global metric atomics to two per workgroup. The existing asynchronous sticky-status readback grows from 28 to 44 copied bytes; no extra compute dispatch, body download or contact-buffer download is added. Regular and callback-finish submissions share status capture/mapping. A snapshot includes its full u64 submitted physics step and capacity-loss flag; a busy status slot may return an older completed snapshot, never relabeled as the newest step. The measurement phase is contact scheduling before solve/integration and host CCD, not a claim about post-CCD geometry.

Misleading names corrected: LiveStepStats.live_contacts is now narrowphase_pairs, and the benchmark workload JSON active_contacts key is now narrowphase_pairs. Historical artifacts retain their original schema and must not be reinterpreted as actual touching-contact measurements. Native Box3D public contactCount is not implemented by these diagnostics. Native compound child identity and mutation handling must be preserved before using these diagnostics for the native counter.

Validation: **178/178** release library tests and release build pass. An expanded targeted GPU test also passes with the only occupied root in the tail of the third workgroup. Controls distinguish 3 (then 130) unique candidates from 1 root, 2 manifold slots and 1 touching root; compare allocation totals with actual contact storage; verify initial unknown, retirement to zero, no pose maps, a busy-slot stale snapshot, and step IDs above 32-bit range. Evidence: `artifacts/v18-contact-metrics-validation.json`. No native matrix or performance run was repeated for this slice; no CPU-win claim.

Next: expose metrics through a world-generation/topology-aware API and viewer fields with explicit unknown/stale status, replace the viewer's unpopulated zero contact display, and implement native allocated-public-contact semantics separately. Validate ordinary and callback paths with actual compound/multi-patch scenes, mutation/retirement and same-process world reuse before making support/capacity claims. Large-static GPU scheduling, Village support and full native parity remain unfinished. No commit or push.

World-aware contact metrics are now exposed through `b3_world_contact_metrics` and the nonblocking `gpu_b3_world_contact_metrics` FFI. Each captured status slot preserves the topology/state revisions from submission alongside its full step ID. The result includes snapshot/current revisions and steps, known/current/capacity-loss flags, and distinct GPU lifecycle counts. World-generation lookup rejects destroyed IDs. Teleports, geometry changes, retirement and new submissions cannot relabel an older snapshot as current. Host CCD corrections now advance query_state so both query-index freshness and scheduling-snapshot invalidation reflect corrected poses. Diagnostics may explicitly wait for the compact status; the viewer only polls and does not harvest CCD/events or download bodies.

The native GPU/combined sidebar shows pending or older-state contact metrics and separate roots/manifold slots/touching roots/candidate pairs. Per-frame benchmark JSON includes `gpu_contact_metrics` with phase `contact_scheduling`; `contact_count_known` is false in GPU/combined mode because native allocated-public-contact semantics are still unimplemented. CPU mode retains its real native contact count. C/Rust layout assertions cover the shared 80-byte result struct. Do not interpret current scheduling metrics as post-CCD collision geometry.

Validation: **180/180** release library tests; ordinary and custom-filter callback stepping, no-harvest state, teleport/topology changes, shape retirement, world destruction/reuse, and an actual fast-impact CCD correction invalidating its prior scheduling snapshot. Release CPU/GPU/both viewers rebuild successfully. Fresh 32-frame High Resistance probes pass in all three modes: GPU/both each report 31 known snapshots and peak 10 roots, with initial unknown kept explicit. Reproduce after building viewers with `python3 scripts/probe-contact-metrics.py <fresh-output-dir>`. Evidence: `artifacts/v18-world-metrics-validation.json` and `artifacts/v18-world-metrics-native`. These are plumbing/correctness probes, not latency certification or a full native matrix rerun. No commit or push; future visual/solver commits still require the pre-commit comparison recordings.

Next: implement native public allocated-contact counts with correct compound-pair identity and independent CPU controls, rather than aliasing GPU roots or touching patches. Then use current, valid capacity metrics for large-static GPU broadphase/query work and removal of Village's default support restriction. Full Box3D functionality, native scene coverage and CPU-win validation remain unfinished.

Native contact allocation semantics are now backed by an executed C reference, correcting the earlier suggestion to deduplicate all public shape pairs. Native `b3CreateContact` assigns separate contact IDs to compound children (`childIndex` participates in the contact key); mesh manifold patches belong to their root contact. Sensors do not enter the native contact pool. Non-touching allocations still count. Teleporting preserves allocations until the next step, whereas destroying a shape removes them immediately. Preserve compound child identity; do not collapse distinct child contacts into one public-pair count.

The GPU scheduling metrics now include `non_sensor_roots`, excluding sensors, bounds-only public proxy shapes and unsupported mesh/mesh pairs while retaining non-touching allocations. This uses the existing occupied-root pass and per-workgroup reduction; the shared status copy is now 48 bytes, with no additional compute dispatch. The C result remains 80 bytes by using the prior trailing padding; a new offset assertion covers non_sensor_roots at byte 76. The viewer and benchmark JSON label this as a scheduling-phase diagnostic, not the native public world counter.

Validation: **180/180** release library tests; rebuilt CPU/GPU/both viewers and fresh 32-frame High Resistance probes pass, with GPU/both peak 10 non-sensor roots. `scripts/check-contact-count-semantics.sh` is a required gate: 48 CPU/GPU registry lifecycle observations and 24 current post-step GPU count observations match, including actual allocated non-touching contacts verified through GetContactData. Evidence: `artifacts/v18-native-contact-semantics-validation.json`. Runtime CPU fallback was not added. Full native compatibility and CPU-win certification remain false.

Next exact counter implementation must handle immediate mutation without downloading the entire world. Add an on-demand parallel GPU reduction over allocated root slots, excluding child manifold slots, sensors, unsupported pairs and retired logical shape identities. Supply generation-checked shape liveness/retirement metadata from host topology (small uploads only on mutation); keep each compound child distinct. Use a dedicated scalar result and cache it by submitted physics step plus contact-allocation revision. Teleports must not incorrectly retire allocations; shape/body destruction, filter changes and joint collision vetoes must invalidate/update the result immediately. Extend the native reference with those mutations and slot reuse before wiring b3World_GetCounters.contactCount or setting contact_count_known=true. Do not route ordinary HUD reads through the full CPU pose/contact mirror. Then proceed to large-static GPU broadphase/query scheduling and Village support. No commit or push.

The measured Vulkan command-cache path is now available as an opt-in source build: run `./scripts/build-native-cache.sh`, then `./scripts/run-native-cache.sh --scene mixed-stacks` from `experiments/gpu-physics`. Backend preparation is fingerprint-checked and uses a separate generated workspace/lockfile; the ordinary Cargo build remains available. See [the native backend build notes](../experiments/gpu-physics/compiler/native-backend/README.md). This integrates existing measured work; it does not establish a new CPU win.

A packed small-root follow-up is archived in `artifacts/packed-small-roots`: GPU activation was confirmed (300 mixed-stacks components), but solve p50 remained 0.2232 ms in its paired screen. The candidate was reverted; no frame or CPU-win claim follows.

A two-body/four-contact small-partition screen also left the combined solve-stage p50 unchanged at 0.2232 ms and was reverted (`artifacts/small-component-partition`). That timer includes component-list construction, not only constraint math; a subsequent count/scatter grid-stride screen regressed the stage slightly and was reverted (`artifacts/component-grid-stride`). A focused timestamp split (`artifacts/component-stage-split`) attributes roughly 0.19 ms to solver kernels and 0.03 ms to list construction, with only 0.001 ms total-device change in that diagnostic screen. Solver execution remains the larger target.

A follow-up sweep of small-component workgroups 1/2/4 was slower than 16 (`artifacts/subwarp-component-groups`); the experimental sizes were reverted. The native launcher now honors explicit supported workgroup-size overrides, while retaining 16 as its default.


### Display-local overlap viewer (Linux experiment)

The opt-in `experiments/gpu-physics/scripts/run-split-adapter.sh` launcher keeps physics on NVIDIA and renders Dominoes/mixed-stacks on the display-local AMD adapter. A two-slot staging ring carries exact state bytes and step IDs; the consumer waits only for the required older copy submission. Both GPU and matched CPU viewers display N-1 while simulating N, and timeline runs drain N without an extra physics step. This is host-staged transfer, not cross-vendor opaque-FD sharing or a zero-copy claim. Immutable fixture metadata is cached per ring generation; other scenes use the existing NVIDIA rendering path. Scene switching recreates the surface and rendering resources.

The final integrated implementation passes 55/0/0 correctness cases, all 12 native matrix cases, and 222 library tests. Five fresh matched AC trials per engine and scene meet the aggregate cadence criteria: mixed-stacks GPU/CPU p50 0.489/0.661 ms, Dominoes 1.885/3.687 ms, with lower GPU p95 on both. All trials remain included, including one slower GPU mixed-stacks pair. Final-binary lifecycle and source identity checks pass. The scoped `cpu_win_validated=true` report is [documented here](../experiments/gpu-physics/artifacts/gear-clipped-support-final/README.md).

The captured Gear Lift step-800 rock exposes hull–triangle polygon truncation: a penetrating vertex beyond the first four was discarded before manifold reduction. Contact generation now processes the complete clipped polygon through the bounded reducer, retaining the deepest witness and its feature identity. The captured first-step position matches CPU after this change. `check-hull-mesh-rest-reference.sh` includes the saved pose and requires its deepest support separation to match CPU within `1e-5`. The expanded CPU contact regression and all 26 manifold regressions pass. The 1,200-step native Gear Lift replay also passes: 3.53 cm peak solid penetration, one-frame maximum deep streak, no final deep overlap or floor crossing; matched CPU is 5.89 cm and two frames. Evidence is under `artifacts/gear-final-regression/`. The fresh full gate passes 55/0/0 with unchanged source fingerprints (`artifacts/gear-clipped-support-final/`); final-build performance qualification also passes in the same artifact directory.
