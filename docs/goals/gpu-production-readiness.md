# GPU native production readiness

Read `/home/firtoz/work/2026/box3d-wasm/docs/goals/gpu-production-readiness.md`
at the start of every continuation and after compaction or restart. Pursue the
whole objective, maintain this file, and use the goal-scratchpad skill. This is
the authoritative production-readiness queue for `feat/gpu`.

Updated: 2026-10-02. Status: incremental PR09 repair verified; commit/push in progress. Full roadmap remains paused. Stop after this milestone.
Next item on resumption: **PR09 — compare every existing CPU sample with GPU and fix obvious differences, including missing floors**, followed by PR10 UI/widgets and PR11 raycasts. Exact pixel and CPU/GPU scene position/angular matching are deferred to a separate goal; physical correctness remains required.

## Objective and release boundary

Make `experiments/gpu-physics` ready for an initial **Linux/NVIDIA native release**
on `feat/gpu`: physically correct within a published capability contract,
repeatable on the same build/device/configuration, reliable under supported
lifecycle operations, honest through its public API, and reproducibly built and
qualified. Complete every required gate below with directly applicable evidence.
Preparing this document or producing a diagnosis does not complete the engine.

Initial hardware qualification is the i9-9900K / RTX 4070 SUPER desktop using
actual NVIDIA Vulkan. Publish the tested distribution, driver and adapter; do
not extrapolate to all Linux/NVIDIA systems. Qualify ordinary wgpu and native
cached backends separately. Keep CPU-compatible ordering available. Efficient
GPU ordering may become normal only after its existing physical, complete-state
repeatability and performance requirements pass. Automatic island scheduling
is a separate policy; this roadmap does not require changing its default.

Required physics includes contacts and meshes, support, friction/restitution,
mass/inertia, joints and mechanisms, motors/limits, CCD, sleep/wake, dragging,
queries/events and runtime creation/deletion/replacement. **Rain articulated-body
correctness is required**; it cannot be removed to obtain a production label.
Preserve upstream scene geometry, defaults and analytical/physical tolerances.
The comparison-scope amendment below supersedes cross-engine scene-trajectory
matching requirements only. CPU/GPU chaotic trajectories may differ, but unexplained residuals,
penetration, energy gain, missed collisions or stale identities are defects.

Browser/WASM/WebGPU, Windows/macOS, AMD/Intel/Apple and laptop qualification,
native recording/player API parity and complete Box3D API replacement are later
release scopes. They must be explicitly reported as unsupported/unqualified
where appropriate. Diagnostic captures and cached command replay are not native
recording/player API implementations. Keep Box3D and the existing WASM package
unchanged. Their existing workflows must still pass compatibility checks.

## Sample comparison scope and debugging method

User amendment, 2026-10-02: compare **every existing CPU sample**, including
samples outside the short solver/benchmark fixture list. Derive an exhaustive
inventory from the actual CPU sample registrations available to the native
viewer; reconcile other existing CPU demo/catalog entries so unavailable ports
are explicit gaps rather than silent omissions. Record each sample's CPU identity,
GPU availability, ordinary/native/combined review status, scene settings,
interactions, visual/behavioral findings, obvious performance differences and
evidence. A missing GPU counterpart or unsupported feature must have a named
status/reason and disposition; do not call an unreviewed sample equivalent.

The present goal requires practical comparisons: visible floor/static/compound/
mesh geometry, body and shape counts, dimensions, materials/camera where they
hide geometry, collisions/support, mechanisms, motors/limits, spawning/recycling,
queries and available interactions. Look for grossly different falling, bouncing,
exploding, drifting, failing to move, falling through geometry, broken mechanisms,
freezes or severe slowdowns. Match setup, defaults and scripted inputs, and record
what was reviewed. Establish a finite review duration and run budget before the
sweep; do not claim long-window equivalence from a short inspection. Investigate
performance outliers with small bounded measurements under matched conditions;
keep these triage timings separate from PR08/PR12 final performance claims.

**Exact pixel matching and CPU/GPU scene positional/angular trajectory matching
are a separate future goal.** Do not chase a ragdoll's final resting pose or fail
this goal merely because corresponding bodies follow different trajectories.
Historical trajectory failures remain archived and are marked deferred, not
passed. This scope change does not waive physical bounds (penetration, constraint
residuals, support, CCD, energy), truthful API/query semantics, focused calculation
accuracy, schedule-equivalence or same-build/device/configuration repeatability.
A joint anchor error is a physical constraint error; a ray hit point is a query
result. Neither is merely a scene-trajectory mismatch. Audit frozen PR03 selectors
before PR07 and record which assertions are trajectory-only and deferred; retain
all independent physical/function checks without increasing their tolerances.

For each demonstrated discrepancy, use **calculation-first debugging**:

1. Use the scene to identify the first relevant operation or state transition,
   then capture the inputs at that boundary. Verify geometry, defaults, units,
   coordinate frames, body/shape identity, ordering and precision first.
2. Feed identical frozen inputs to the relevant CPU/reference and GPU calculation.
   Compare `f(x)` with the expected `y`, using existing numerical tolerances and
   explicit invariants. For intentionally different algorithms compare the
   specified mathematical/physical result, not incidental implementation details.
   Do not compare two already-diverged scene states as if
   their input `x` were equal. Execute the actual implementation where practical;
   a rewritten formula alone cannot verify its shader/ABI/data-layout behavior.
3. If outputs differ, split `f` into its constituent operations and compare their
   intermediate values. Recurse to the first differing primitive, branch, lookup,
   transform, Jacobian, effective mass, bias, impulse, clamp or state write. For
   rendering, similarly trace shape enumeration, allocation, index mapping,
   transform and draw submission. Record input/output values and the first cause.
4. Fix that cause and add/reuse the smallest meaningful regression. Include
   applicable boundary/transition cases and verify both GPU backends when affected.
   Use a finite build/process budget; retain failed hypotheses and original data.
5. Once focused checks pass, rerun the affected scene/interaction and relevant
   regressions, then final qualification at the appropriate gate. Full scenes are
   confirmation and release checks, not the inner loop for every hypothesis.
   Record CPU-first before/after clips before visual or solver commits.

## Ordered checklist and acceptance contract

Work on one unfinished item at a time. Its evidence cell is initially `OPEN`.
Add links to committed receipts/reports and the applicable source/build identity
when closing it. Keep detailed physical results in
[`../gpu-solver-qualification.md`](../gpu-solver-qualification.md), API/support
status in [`../gpu-physics.md`](../gpu-physics.md), and this file as the concise
queue and recovery record. No separate competing production backlog.

Priority updated by the user on 2026-10-02: address visible functionality gaps
before continuing the deeper physical-correctness investigation. Execution order
is **PR09 → PR10 → PR11 → PR04 → PR05 → PR06 → PR07 → PR08 → PR12 → PR13**;
PR01–PR03 are already complete. Stable item IDs are preserved. The user
explicitly defers exact visual/scene-trajectory parity as described above; other
acceptance criteria remain required. Floors, widgets and query behavior can be
checked independently of remaining solver failures. Relevant safety/correctness checks still accompany
each fix; a concrete dependency may require a focused prerequisite repair. Do
not mark an entire gate passed from that repair alone. Final-build qualification
and fresh charts follow the functionality changes to avoid qualifying a binary
that will immediately change. Physics failures remain release blockers.

- [x] **PR01 — Implement speculative-contact world controls.** Replace the
  no-op `b3World_EnableSpeculative` with real per-world behavior and correct
  combined-viewer routing. Preserve the default and upstream semantics, including
  interaction with per-shape flags, creation and runtime off/on transitions,
  independent worlds and destruction/recreation. Verify convex/mesh cases and
  interaction with CCD on both backends using independent CPU/GPU fixtures.
  Relevant default-behavior, lifecycle and repeatability checks must pass.
  Update API audit/docs and record affected scenes before a solver commit.
  **Evidence:** [requirement audit](../../experiments/gpu-physics/benchmarks/production-readiness/pr01-speculative-2026-10-01/acceptance.json),
  [portable report/receipts/raw state/recordings](../../experiments/gpu-physics/benchmarks/production-readiness/pr01-speculative-2026-10-01/README.md).
  Compiled ordinary/native inputs and library/test hashes are exact in those
  receipts. The added all-window floor screen remains failed; source analysis
  explains its discrete landing scope without raising a tolerance. The archived
  baseline startup diagnostic remains incomplete and never counts as a pass.
  Final physical/runtime qualification remains under PR03–PR07. Functional
  milestone `695c93ada50220089018009230209e5050ed9252` is committed and pushed to `feat/gpu`.
  **Reopened/resolved:** root-world ABA diagnosed2026-10-01; retained repairabc0a54 and the [current acceptance refresh](../../experiments/gpu-physics/benchmarks/production-readiness/pr01-current-acceptance-2026-10-02/README.md) now close this original functional-control gate. All20 public trials,2 CCD guards,10 raw-state repeats and126 unchanged regression checks pass on exact current ordinary/native sources. The [criterion audit](../../experiments/gpu-physics/benchmarks/production-readiness/pr01-current-acceptance-2026-10-02/acceptance.json) links current lifetime/control/API/source/CPU-first52-clip evidence. Focused110-step schema24 repeats match all captured fields; broader future-state lifecycle, final physics and release repeatability remainPR03–PR07. The extra all-window screen/baseline timeout remain failed/incomplete; no tolerance raised or result replaced.

- [x] **PR02 — Close the native API contract and remaining truthful-control gaps.**
  Audit current linked symbols, Rust/C implementations and combined wrappers;
  old stub counts and documentation are not current-code proof. Verify world
  counters/contact counts, profile/capacity diagnostics, warm-start/speculative
  controls, forces, replacement and joint queries against their actual behavior.
  Implement remaining required gaps; supported operations must not silently
  succeed as no-ops or report placeholder values. Publish an exact supported API
  inventory and explicit errors/unavailability for excluded recording/player or
  CPU-specific worker/static-tree operations. Preserve available core API
  behavior; exclusions cannot excuse a required physics failure. Exercise
  per-world ownership, mutation and lifetime through C/Rust and viewer controls.
  **Evidence:** [current acceptance matrix](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-current-acceptance-2026-10-02/acceptance.json)
  and [portable current report/inventories](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-current-acceptance-2026-10-02/README.md).
  Exact sourceabc0a54:32 C world tests/20 Rust selectors pass unchanged;
  corrected classification16GPU+4host. Original runner adapter-banner rejection
  of a passing host selector is retained; onlyone previouslyunlaunched host test
  consumes the remaining budget, no repeats. Current415-symbol/backend/linkage
  inventory covers376 implemented candidate operations/39 explicit exclusions;
  all156 excluded definitions match applicable successful compiled error methods.
  Per-world/lifetime, actualfour-viewer controls and52CPU-first scene clips are
  linked in the criterion audit. Historical failures and rejected compound fixes
  stay preserved in their reports. This closes the named functional API gate,
  not376 independent physical behavior passes or full release qualification.
  Rain/physical/runtime/full-state/clean-build/performance remainsPR03–PR08;
  Village omission and sample-wide widgets/query appearance remainPR09–PR11.

- [x] **PR03 — Freeze the release qualification contract and baseline.** Reconcile
  the existing fixed fixture matrix and evidence with current sources. Record
  named commands/selectors, geometry/defaults, durations, physical limits,
  capture fields and backend/policy combinations for every required capability.
  Preserve existing limits; justify any genuinely missing criterion before
  evaluating a candidate. Record known failures separately from historical
  passes and final-build passes. Include Rain, Falling Ragdolls, loaded dragging,
  mesh/CCD, all supported joint types, numerical controls and the recorded
  distance-joint first-step discrepancy. Freeze source inputs, CPU oracle,
  binaries, build receipts, toolchain and adapter/driver. Every later experiment
  gets a finite recorded budget and stop/retain rule before execution.
  **Evidence:** [requirement audit](../../experiments/gpu-physics/benchmarks/production-readiness/pr03-contract-reconciliation-2026-10-02/acceptance.json),
  [frozen contract and baseline](../../experiments/gpu-physics/benchmarks/production-readiness/pr03-contract-reconciliation-2026-10-02/README.md).
  Offline verification checks 127 original process observations, 700 upstream
  raw files, 107 original compiled inputs, 101 required Rust selectors,
  13 trace/14 C recipes and four final backend/ordering configurations.
  Original distance, loaded-drag, strict trajectory and Rain failures/timeouts
  remain explicit PR04 inputs. At reconciliation the only source difference from the
  original baseline was the independently qualified cfg(test) sleeper correction.
  Subsequent retained repairs/tests have separate linked applicability records.
  Order-storage applies to mode 1; existing slot-32768 coverage remains mandatory
  in all four final configurations. Complete registry/ABI capture obligations
  and known omissions are frozen; implementation/qualification stays PR05–PR07.
  This closes contract/baseline preparation, not physical or release readiness.
  No new build, engine, candidate, diagnostic, retry or timing run.

- [ ] **PR09 — Compare all existing CPU samples and fix obvious GPU differences.**
  Next after PR01–PR03, complete the exhaustive inventory and bounded comparison
  sweep described above, including samples outside the short fixture list.
  Reproduce invisible floors, identify every affected sample and fix the
  demonstrated rendering cause. Check static, compound and mesh ground
  representations where used, camera framing and
  occlusion against the real Box3D CPU view. Preserve scene geometry and physics
  defaults; a rendering repair must not change collision/support behavior.
  Record and review all affected scenes before committing, CPU first in the
  comparison grid, and retain before/after evidence. The new [combined Village capture](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-ownership-captures-2026-10-01/README.md) reproduces missing nearby GPU compound ground/buildings while CPU geometry is visible. Unchanged shared65536-slot renderer pool cannot flatten both52500-child compounds; source audit and exact clip are retained. Village is one known case, not the inventory boundary. Fix/qualify every demonstrated floor omission and obvious visual/behavioral defect; triage severe performance differences. Route controls to PR10, queries to PR11 and concrete solver defects to calculation-first PR04 work without losing their sample inventory entries. PR09 closes only after every inventory entry is reviewed or has an explicit unavailable-gap disposition, required floor/render fixes pass, and other findings are linked to named open gates. Routing a solver/control/query/performance finding preserves it as an overall release blocker; it must not force completion of later gates before the sweep can advance. No finding is silently marked passed.
  **Evidence:** [incremental inventory/floor repair](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-floor-2026-10-02/README.md):
  165 active native/174 browser/20 oracle entries; five native floor-only reviews,
  original/repaired calculation evidence, ordinary/native/combined CPU-first
  captures and explicit remaining gaps. Shared metadata exhaustion is repaired;
  full sweep is OPEN, including separate Single Box grid decoration. One capture
  retains transport-recovery exit limitations.

- [ ] **PR10 — Qualify sample UI and widgets end to end.** After PR09, inventory
  the controls and widgets for the full PR09 inventory, including samples outside
  the short list, then
  exercise actual mouse/keyboard interactions on ordinary/native GPU and
  combined viewers with a real CPU comparison. Cover sample selection, sliders,
  buttons, toggles, dragging/picking, overlays, pause/single-step/restart and
  resize/layout where available. Verify displayed state and the intended
  physics effect, including scene changes and destruction/recreation; a widget
  that accepts input but leaves stale state does not pass. Retain interaction
  evidence and record affected scenes before committing fixes. PR02's required
  truthful-control checks still belong in PR02; this is the sample-wide
  usability check requested by the user, now prioritized before PR04.
  **Evidence:** OPEN.

- [ ] **PR11 — Qualify raycast behavior and appearance.** After PR10, reproduce
  the user's reported CPU/GPU difference in the supported raycast samples and
  picking paths. Compare hit/miss, closest-hit selection, public shape/body
  identity, fraction, point, normal, filtering and transformed geometry with
  independent fixtures and the real CPU reference at existing tolerances.
  Check the displayed ray, hit marker and normal against returned query values
  and camera coordinates, including misses and scene changes. Fix demonstrated
  discrepancies and document intentional conventions. Record affected scenes
  CPU first before committing; retain numeric and visual evidence. The user
  observation does not yet establish a query or rendering defect.
  **Evidence:** OPEN.

- [ ] **PR04 — Resolve required physical correctness failures.** Work from the
  earliest demonstrated defect and smallest calculation-level reproducer, using
  the method above. Do not use final ragdoll location as the debugging target.
  Close Rain's persistent/severe joint residuals and applicable dragging, joint, contact,
  CCD and energy/support failures from PR03. Read the old solver recovery and
  rejected experiments before choosing a new hypothesis. Numerical reference
  roots, partial captures and CPU reproduction are diagnosis, not GPU physical
  acceptance. A retained fix needs original-limit physical checks on both
  backends and relevant regressions; restore rejected production edits and
  preserve unfavorable evidence. Rain recycling must retain correct lifetime
  correspondence and healthy constraints throughout the full 600 steps.
  **Evidence:** Partial [retained rigid-distance repair](../../experiments/gpu-physics/benchmarks/production-readiness/pr04-distance-captures-2026-10-02/README.md): four original comparisons/49,920 lanes at unchanged 1e-5, 242 applicable regressions and 15 CPU-first scene recordings pass. Exact 107 candidate inputs match retained source; two known mode-0 selection failures remain recorded. This closes that defect only. Strict CPU/GPU ragdoll trajectory matching is now deferred by user instruction, with its failed evidence preserved. Loaded dragging/CCD and Rain physical defects remain unresolved; PR04 stays open.

- [ ] **PR05 — Qualify runtime safety and lifecycle behavior.** Review and test
  buffer bounds/dispatch coverage, reservation growth/overflow, C/Rust layouts,
  ID generation/reuse, shape/body/joint replacement, callbacks/reentrancy,
  query/event ordering, asynchronous status, replay invalidation and ownership.
  Sticky loss/invalid state must remain observable; clearing diagnostics must
  not make a failed world usable. Exercise repeated create/destroy/scene changes,
  sleep/wake, pause/single-step, shutdown during loading and repeated startup.
  Verify supported launch paths with caching enabled and disabled. Reuse existing
  focused checks; add tests only for a concrete failure or uncovered requirement.
  **Evidence:** OPEN.

- [ ] **PR06 — Freeze reproducible release builds and consumer integration.**
  Clean-build CPU, GPU and combined native targets for both backends, verify
  correct adapter selection and actual cache/replay behavior, and freeze the
  release candidate's compiled inputs and executables. A minimal external native
  consumer must create, step, read and destroy a world using the supported API.
  Check the original TypeScript/demo workflow remains usable without modifying
  Box3D/WASM. Record exact commands, dependency/toolchain revisions and build logs;
  invocation checkout metadata alone cannot identify compiled source. Any later
  relevant code change invalidates affected release evidence and requires an
  applicability review/refreeze before final qualification.
  **Evidence:** OPEN.

- [ ] **PR07 — Complete final-build physical and full-state repeat qualification.**
  Run the PR03 matrix against PR06 binaries with the documented 2026-10-02
  scope amendment: enumerate trajectory-only assertions deferred to the separate
  parity goal, retaining physical/function assertions and archived outcomes.
  Require at least five fresh processes
  per selected configuration on each backend, with complete required durations:
  Rain/ragdolls 600 steps, dragging 3,060, and existing fixture-specific durations
  for other capabilities. Compare future-relevant semantic state every step,
  including bodies/sleep, contact geometry/impulses/history, joints/caches,
  lifetimes, policy and replay state. Retain meaningful ordering; exclude only
  audited incidental fields. Validate compact/lossless capture against full
  records and corruption controls. Report first discrepancies. No pose-only,
  interrupted, skipped or historical run counts as a final pass. Preserve all
  analytical, precision, island, schedule-equivalence and capacity checks. Include
  the added `gpu_invariants::distance_joint_one_substep_matches_cpu_reference`
  regression with its original absolute 1e-5 independent CPU value; this extends
  the original frozen matrix for the concrete repaired defect.
  **Evidence:** OPEN.

- [ ] **PR08 — Qualify performance and make the documented mode decision.**
  Measure final binaries without tracing, recording or competing GPU work. Freeze
  budgets, settings and decisions first; retain all trials and diagnostic runs
  separately. Use the existing small/large-scene GPU-order acceptance rules from
  the solver qualification report, plus global/component/automatic checks on
  the three fixed scheduling fixtures. The default-switch screen requires five
  paired runs in each small/large scene and ordinary/native backend cell: the
  one-sided 95% Student-t upper bound of paired log latency ratios must be below
  zero in all four cells. Report and investigate p95/max regressions as specified
  there; a mean-only pass cannot bypass tails. Match timestep/substeps, sleep, warmup,
  duration, cache settings and rendering conditions; alternate configurations
  across fresh processes. Publish median trial mean completed-step time,
  corresponding throughput and median trial p95 with raw data/provenance.
  Compare fastest applicable explicit controls. Do not promise universal speedup
  or selectively extend budgets. Efficient GPU ordering becomes normal only
  after all applicable gates pass; otherwise this item remains open and the
  existing default stays. Do not reuse either exhausted scheduling budget.
  **Evidence:** OPEN.

- [ ] **PR12 — Publish the supported release and fresh desktop comparison.**
  After PR04–PR11 pass in the documented execution order, review changes for
  evidence applicability and reopen/refreeze any affected PR06–PR08 gates before
  final delivery. Create a new dated
  CPU/GPU chart set from fresh desktop measurements of the exact delivery
  binaries, using PR08 trials only when their sources/settings remain applicable.
  Freeze any additional run budget/settings/order before measuring; preserve all
  outcomes. Compare real Box3D CPU with ordinary/native GPU under matching
  scene, physics and rendering conditions. Show median trial mean completed-step
  milliseconds, corresponding physics steps/second and median trial p95 latency.
  Measure and label rendering FPS separately; physics throughput is not rendering
  FPS. Use the actual desktop graphics adapter for headline rendering FPS, record
  resolution/render options and frame/vsync caps, and match CPU/GPU presentation
  conditions. Software/Xvfb diagnostic captures cannot establish desktop rendering
  FPS. Include the three frozen scheduling fixtures and fastest applicable
  explicit GPU controls, clearly label policies/settings, and use distinct,
  consistent CPU/GPU colors. These new charts must contain desktop data only;
  laptop qualification/charts are deferred, and older datasets stay preserved.
  Display the fresh charts in the final user-facing delivery with links to their
  portable raw data and reproduction/provenance receipts.
  Record every affected
  scene before committing visual/solver changes, with real Box3D CPU first in
  the comparison grid. Publish readable charts, portable raw timing/check data,
  build/source receipts, scene/recording hashes and a compact final coverage
  matrix. Document API semantics, capacity/failure behavior, performance limits,
  default/override selection and reproduction commands. Clearly state the
  verified hardware/platform scope and all exclusions. Keep earlier datasets
  and failed experiments. Delivery must be reviewable without ignored local
  binaries or captures; local artifact paths alone are insufficient evidence.
  **Evidence:** OPEN.

- [ ] **PR13 — Audit completion and deliver the branch.** Verify PR01–PR12 against
  the exact delivery sources and evidence, with no required failure or stale
  claim. Complete the release review, including concurrency, FFI and resource
  ownership. When commit/push is authorized, deliver verified changes to
  `feat/gpu`, verify remote synchronization and a clean tree, and record the
  actual commit(s). Only then report the scoped engine ready and complete the
  active goal. If a gate is blocked, leave its checkbox open and state what input
  or external change is required; a diagnosis-only handoff is not readiness.
  **Evidence:** OPEN.

## How to answer “What's next?” and continue

For `feat/gpu` or an explicitly GPU-focused request, inspect Git status first,
then read this file and recommend the first unchecked item whose prerequisites
are satisfied. Finish relevant unfinished work before adding another task. Give
one primary item, its reason and acceptance criteria. If the first item is
blocked, name the blocker and choose useful independent work from this same
checklist; do not skip a failed gate permanently. The WASM sample-port queue
continues to live in `docs/SAMPLES.md` and applies to sample-port requests.

“What's next?” is a recommendation request. “Let's do it” authorizes that one
item. An explicit `/goal` to complete this roadmap authorizes continued work
through its required items; a pause/stop from the user takes precedence. This
documentation request does not itself activate the goal or resume old Rain work.
Commit/push require explicit user authorization; the suggested activation prompt
can provide it. Do not infer authorization from old unrelated task records.

Use `[ ]` for unresolved, failing, blocked or stale items. Change it to `[x]`
only after the stated acceptance criteria have passed and evidence is linked.
Update the evidence cell, recovery and next item in the same change. Reopen an
item if later code invalidates its evidence; record why. Never tick a required
capability merely because it has been relabelled unsupported.

For an active full-roadmap goal, repeat this loop without relying on chat history:

1. Read this file and verify the recovery record against the worktree, build
   receipts and actual process state.
2. Work on the first unfinished, unblocked item, within its acceptance criteria
   and any frozen experiment budget. Preserve failures and unrelated local work.
3. After meaningful progress, update its evidence and the recovery record. Once
   all its criteria pass, change `[ ]` to `[x]` and advance the next-item pointer.
4. Continue to the next required item. Finish only after PR01–PR13 pass against
   the delivery sources and PR13 records the pushed commit and release audit.

Suggested activation prompt (only submitting it activates implementation and
provides the stated commit/push authorization):

```text
/goal Complete the Linux/NVIDIA GPU production-readiness roadmap in /home/firtoz/work/2026/box3d-wasm/docs/goals/gpu-production-readiness.md. Read it at the start of every continuation and after compaction/restart. Use the goal-scratchpad skill, maintain its recovery record, and mark [ ] as [x] only when linked acceptance evidence passes. Continue through all required gates, preserving its scope and constraints; complete the goal only when PR01–PR13 pass. Commit and push verified milestones to feat/gpu.
```

This follows the [official goal format](https://developers.openai.com/cookbook/examples/codex/using_goals_in_codex):
outcome, verification, constraints, iteration policy and an explicit blocker
record. Do not treat the example stored in a file as a submitted user command.

## Recovery procedure and experiment discipline

At each continuation/compaction, read this file, AGENTS.md and the active item's
relevant source/evidence. Before physics work also read
[`../gpu-solver-goal.md`](../gpu-solver-goal.md) for full criteria and rejected
approaches and [`../gpu-solver-qualification.md`](../gpu-solver-qualification.md).
Their historical “live” jobs and next actions do not override this queue or prove
a process is running. Closed scheduling goals remain closed.

Verify branch, local work, source/build hashes and actual process state before
trusting the recovery block. Preserve unrelated changes; fetch/integrate remote
updates when beginning authorized implementation without overwriting local work.
Do not run timing concurrently with builds, tests, captures or other GPU work.
Resume validated interrupted batches without duplicating completed trials; replace
only genuinely incomplete runs according to the frozen protocol. Never count a
resumed process as a fresh-process repeat.

Before an expensive campaign, record its hypothesis, inputs, binary identity,
settings, acceptance limits, fixed run/candidate budget, order and stop rule.
Record consumption after every result. At budget exhaustion, preserve failures,
restore rejected edits and choose a distinct evidence-supported next action;
do not restart the same experiment under a new label to chase a favorable result.
An exhausted experiment does not imply the whole readiness goal is complete.

Maintain the block below after meaningful work and before handoff. Keep it a
current-state summary; detailed experiments belong in linked reports. Never
rely on conversation history as the sole record of a pending experiment.

## Current recovery record

Read this section before continuation; the ordered checklist is authoritative.
Older reports preserve all original failures, budgets and source snapshots.
Their former next actions/job handles do not authorize another campaign.

| Field | Verified value |
| --- | --- |
| Workspace / branch | `/home/firtoz/work/2026/box3d-wasm`, `feat/gpu` |
| Authorization | Full-roadmap goal currently paused in the app; verified milestone commit/push authorization retained. This priority/documentation update does not resume builds, experiments or implementation |
| Latest published milestone | `2df60fa4037189288fdfb5a381c597da7a25b626` direct native GPU CCD boundary/fast-flag acceptance audit committed and pushed. All 43 staged files, including 32 indexed raw files, matched bytes; offline and relocated verification passes. Verified origin 0/0 and clean tree. Native coverage gap resolved; full dragging/PR04 remains open. No production/default/policy/Box3D/WASM change. Earlier ordinary boundary `58eb04c`, focused regression `437fd1c` and retained repair `c1fb15f`/CPU-first recordings remain pushed |
| Retained production source | `c1fb15faaaec3c827c4381c6694e9041bbebd076` rigid-distance repair, shader SHA `22fe506b97b97c24f724a09b2dad2efcd52db6b71b95d892bc3bc31c4726ea31`. Production sources unchanged. Only cfg(test) `src/gpu_invariants.rs` now adds the focused CPU-reference regression; explicit 107-input applicability against prior producer is in its report. HEAD is context only |
| Current item on resumption | **PR09** exhaustive comparison of every existing CPU sample against GPU, including those outside the short fixture list: floors/geometry, obvious behavior and severe performance differences. Then PR10 UI/widgets and PR11 raycasts. User priority/scope amendment2026-10-02 defers exact pixels and cross-engine scene position/angular matching to a separate goal; physical/function limits remain. Calculation-first debugging required. Only documentation changed; no build/process launched |
| Deferred physical investigation | **PR04**: distance repair and focused one-step regression pass; 4 candidate physical comparisons/242 applicable regressions/15 CPU-first clips retained. Strict ragdoll trajectory-only screen deferred (not passed); loaded dragging/CCD and Rain physical defects remain unresolved. Both ordinary and native CCD boundaries directly attribute the first held-position jump. Native GPU coverage gap resolved; fast-flag handoff and low-speed landing acceptance interaction remain to resolve before a candidate. Physical/function limits retained; scene-trajectory parity deferred by user instruction |
| Current jobs | All campaigns terminal. Native observer driver **92458 exited 0**, frozen SHA `267a0e661267832ef81807e63e3dd5431721f626fd2b094d7b58c30b105cf1ee`: one native library build (4 actual hull C units), one fixture compile/link, one fresh native 240-step process; no candidates/retries/timing. All 14,020 archived printed lines match; 39 identical-duplicate observations cover 13 selected GPU boundaries. Actual convex CCD fraction 0.623385488986969 and 16.992787 mm correction directly observed. [32-file portable report](../../experiments/gpu-physics/benchmarks/production-readiness/pr04-native-ccd-boundary-2026-10-02/README.md) validates offline/relocated; full dragging and PR04 remain open. Budget closed; do not restart |
| Current libraries | Candidate ordinary SHA `5bb8a5d7988eea83e5f6bdbfc0831872f59e06389d69c4a1b9919e7bf7f8e3f1`, native SHA `53819158e2321f1f270fa3d885664ce6519770f6448163d4894de1041b19bc3e`; exact source/build identities in clamp and viewer receipts. Shared `target/native-cache-build/release/libgpu_physics.a` now contains the artifact diagnostic observer SHA `652bc603cc8ded7bc854c4ad7cfc2a2f2a790dadcc54402b166e455ac5f3fdec`; do not use it as a production provider. Retained named libraries remain unchanged; use those receipts or a new budgeted production build |
| Independent CPU | Full archive SHA `b35f71d07515e333fa19f3e6297bbfc2040984a49f366c94afc5f974acef2c0c`, 119 actual source/object pairs; combined fixtures use separate prefixed CPU archive |
| Hardware / policy | i9-9900K / NVIDIA RTX 4070 SUPER, driver 610.57.04, NVIDIA Vulkan. Ordinary/native and ordering 0/1 remain separate; scheduling/default policies unchanged |
| Scope | Box3D/WASM unchanged. Mixed scheduling target **unmet**, all 55 trials retained; both old scheduling budgets exhausted. New Rain work is required under this roadmap, not a resumption of historical jobs |

### Frozen evidence and remaining failures

The [PR03 reconciliation](../../experiments/gpu-physics/benchmarks/production-readiness/pr03-contract-reconciliation-2026-10-02/README.md)
is the compact baseline/criterion index. It verifies all 700 original indexed raw
files without a GPU and retains 127 original processes, unchanged commands and
results. Physical/state/build/performance/presentation acceptance remains open.
The [original fixed criterion table](../../experiments/gpu-physics/benchmarks/production-readiness/pr03-rust-baseline-2026-10-02/release-contract.md)
and frozen solver criteria preserve original limits and historical rejected hypotheses.
The user's 2026-10-02 scope amendment above overrides trajectory-only acceptance
for this goal; it does not alter historical reports or convert failures to passes.

- Distance historical baseline: both backends abort at original step 0/lane 1,
  GPU Y=-1.00004232, CPU Y=-1.00028491, absolute 1e-5. The retained clamp now
  passes all four complete original comparisons without raising that limit.
  Historical failures remain archived; general spring/cache equivalence is open.
- Ragdolls: 600-step physical and isolated/mesh checks pass; original strict
  first-60 trajectory screen fails 0.063585767 m versus 0.006 m. That
  cross-engine trajectory screen is deferred to the separate parity goal, not
  passed. Investigate any demonstrated physical defect using calculation-level
  values; do not chase the final pose.
- Loaded dragging: original 3,060-step screens fail both backends; first held
  position exceedance frame 227/body 0, peak about 0.14745 m versus 0.005 m.
  Isolated/released-endpoint passes do not replace physical qualification.
  Retain the original failed trace; separate CPU/GPU trajectory-only assertions
  (now deferred) from actual held-anchor, support, collision and energy bounds.
  Diagnose the demonstrated CCD boundary from identical calculation inputs.
- Rain: CPU 600 complete (1,948,800 body/joint observations); ordinary/native
  900-second attempts time out without health files. Separately bounded
  ordinary 30/30/180 diagnostics complete; observer subset matches exactly.
  Original residual screens first fail at frame 106/completed step 107, with
  272,160 matched joint observations over 180 frames and no recycling.
  Health is 99.798% of broad physics diagnostic clocks; internal wait/CCD/collector
  attribution is open. These clocks are not headline performance.
- Rust: 33 processes/199 checks, 195 pass/four original failures retained.
  Corrected generation-safe sleeper passes four backend×ordering cells at
  original 400+1 duration. Order-storage requires mode 1 and unchanged controls
  pass there. Slot 32768 was not baselined and remains mandatory at final release.
- Capture/runtime: schema24 does not prove registry epoch/root/child arrays or
  full C metadata. Generation-1 diagnostic cache setters and Rust/C world-index
  range mismatch remain concrete PR05/PR07 audit inputs. PR06 clean-build and
  final five-fresh-process qualification are still required.

All original campaign budgets are closed. Detailed protocols, receipts, hashes,
failures, raw indexes and validators remain in their linked portable reports.
Local artifacts are reusable only after checking actual binary/input hashes;
ignored executables are not required for offline evidence verification.
No unchanged retries or extension of prior watchdogs are authorized by this record.

### Incremental PR09 milestone recovery (2026-10-02)

Read this roadmap and AGENTS.md after compaction/restart. The active bounded
objective is to inventory every existing CPU sample and its GPU counterpart,
explicitly mark unreviewed entries, reproduce/fix one confirmed missing-floor
rendering cause and related cases, verify actual Box3D CPU versus ordinary/native
and combined GPU while preserving geometry/defaults/identity/picking, retain
CPU-first before/after evidence, then commit/push to feat/gpu and stop. Exact
pixels and scene trajectories are deferred. Box3D/WASM/solver remain unchanged;
closed historical budgets remain closed. PR09 is unchecked until its full sweep
criteria pass; this milestone does not resume the whole roadmap.

Frozen [protocol](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-floor-2026-10-02/protocol.json):
five viewer builds, two focused-test builds/12 checks, 45 captures (five scenes,
25 before/20 after), 60 paced steps each, 180s build/300s capture watchdogs,
zero retries/Rust builds/performance runs. Stop/retain on failure; no implicit
extension. Before any build/engine launch, original local roadmap diff is archived
with its hash; fetch found HEAD/origin feat/gpu 0/0. No other starting local work.
Current state: repair and verification complete; commit/push pending. All25 before
captures passed. Both combined Village viewers reproduced missing GPU ground/
buildings; the common stable-address debug metadata allocator now grows beyond
65536 without dropping children. Original105004-child input registered65535,
first missing65535; repair registers105004/no missing with131072 stable slots.
Two focused builds/12 checks and five viewer builds are complete and closed.

All45 planned captures have60 completed/rendered healthy steps. Both repaired
combined Village views restore ground/buildings; visible GPU instances143→1551,
body/joint/draw-shape counts unchanged. Allfive scenes reviewed at two sampled
stages; other four floor representations stay visible. Actual NVIDIA Vulkan
compute; Mesa/Xvfb diagnostic graphics, no desktop FPS/performance claim.
One after driver terminated143 after ordinary combined Grid completed its engine
steps. Existing ffmpeg finalized via SIGINT, original exit observations absent;
full media/health/shutdown log/black tail retained. Only five never-launched native
combined cells continued, all normal exits. No engine retry or budget extension.
[Portable evidence/inventory/recovery](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-floor-2026-10-02/README.md)
retains original interrupted and merged receipts, actual source/build hashes,
45 clips,100 sampled frames, and the frozen budget. Offline validator passes:537 indexed hashes, complete catalog counts,12 actual
calculation cases,5 viewer builds,45 healthy captures and explicit exit limitation.

No live job remains (sessions93883/30444 terminal0;54585 interrupted143).
Old objects/libraries/viewers/clips and preexisting local roadmap documentation
are preserved. Only renderer generation, capture step support, regression/tools/
inventory/docs/grid changed; no Rust/WGSL/Box3D/WASM/scene/default/picking API change.
Inventory:165 active native registrations,174 browser entries,20 CPU oracle
fixtures; five native floor-only reviews,160 other native entries and browser/
oracle catalogs unreviewed,13 browser native-counterpart gaps, Replay recording
unavailable. Single Box CPU grid decoration in combined view remains a separate
PR09 finding; late body color differs, not an independently qualified sleep defect.
PR09 remains unchecked; broader physical/widget/query/performance gates open.

Offline evidence/source/status checks pass, the CPU-first grid has all45 clips,
and final fetch still found HEAD/origin feat/gpu0/0. Next: commit/push this verified
milestone to feat/gpu, then stop. Full roadmap remains paused; do
not begin another sweep, solver repair or old campaign without new authorization.


### Next work on resumption

Start PR09: inventory every existing CPU sample and its GPU counterpart, including
those outside the short list. Inspect the retained Village evidence and current
renderer allocation/compound mapping, review all floor representations and obvious
visual/behavioral/performance differences, and fix demonstrated defects while preserving physics
geometry, body identity and picking. Freeze a finite build/check/capture protocol
before execution. Record every affected scene with real Box3D CPU first, retain
before/after evidence, and check both ordinary/native viewers and combined routing.
Then complete PR10 actual UI/widgets and PR11 numeric/visual raycast comparisons.
No performance campaign or closed solver experiment is restarted by this change.
The app goal remains paused until the user resumes it or authorizes one item.

### Deferred PR04 recovery

Retain the following diagnosis for continuation after PR09–PR11. Its historical
next experiment is not the current queue head.

The [retained clamp report](../../experiments/gpu-physics/benchmarks/production-readiness/pr04-distance-captures-2026-10-02/README.md)
links the completed four-case physical checks, applicable regressions, four
artifact viewer relinks and 15 CPU-first clips. Its source-applicability record
matches all 107 retained inputs to the compiled candidate. All three affected
scenes are reviewed; Joint Events retains an unrelated free-joint position
difference. Review is sampled and timestamps approximate, not a full numerical
scene qualification. No default, tolerance, Box3D or WASM change.

The distance milestone is pushed; original clamp/regression/viewer/capture budgets
are closed. Do not repeat them or mutate raw receipts. Keep PR04 unchecked.

The [focused one-substep regression](../../experiments/gpu-physics/benchmarks/production-readiness/pr04-distance-focused-test-2026-10-02/README.md)
now passes all four ordinary/native × ordering 0/1 cells, reporting GPU and
independent CPU Y=-1.000284910, original absolute 1e-5. Its two-build/four-process
budget is closed. Only cfg(test) changes; previous production evidence retains
bounded applicability. Include this selector alongside the original PR03 matrix
at PR07. The host preparation path failure remains archived, zero engine/build
consumption. All portable raw hashes validate.

The focused regression/current-impact diagnosis (`437fd1c`) and CCD boundary
report/static native observation audit (`58eb04c`) and direct native capture/flag
acceptance audit (`2df60fa`) are pushed. PR04 stays
unchecked; production shader/API/viewer/scene/defaults are unchanged. The boundary
validators pass, including native report relocation; all 43 new staged files
matched their worktree bytes. Fetch found
no remote divergence; push verified origin 0/0 and a clean worktree. No live job.

1. The [ordinary/CPU boundary diagnosis](../../experiments/gpu-physics/benchmarks/production-readiness/pr04-drag-ccd-boundary-2026-10-02/README.md)
   and new [direct native GPU capture](../../experiments/gpu-physics/benchmarks/production-readiness/pr04-native-ccd-boundary-2026-10-02/README.md)
   attribute frame 227's first held-position jump: CPU motion 45.120 mm is below
   its 250 mm fast threshold. Native motion 45.119993 mm exceeds 20 mm cutoff;
   fraction 0.623385489 shifts the endpoint 16.992787 mm. Pre-CCD difference 1.056 mm,
   post difference 17.165 mm; velocity still agrees within 2e-6 m/s. Both diagnostic
   prefixes exactly match their original printed records; native post pose lanes
   match direct GPU words at float32 precision. Native observation gap is closed.
   Both budgets are exhausted, no new candidate/production/default change.
   The [fast-flag/acceptance audit](../../experiments/gpu-physics/benchmarks/production-readiness/pr04-native-ccd-boundary-2026-10-02/handoff-audit.json)
   finds shader `apply_deltas` already computes CPU-style motion, but sets
   `FAST`/`CCD_NO_HIT` with the shell cap. These flags affect proxy padding,
   mesh refresh and handoff; historical host-only rollback left them inconsistent.
   That concern does not prove the cause of its late toppling. Existing low-speed
   CCD landing still requires first Y0.505±0.001/nextY≥0.495. Next use the smallest
   independent CPU/GPU reproducer to resolve that landing/loaded-joint interaction
   before selecting a physically supported candidate. Freeze a distinct finite
   protocol before any build/process; keep all existing limits. No consistent
   cutoff-only change is assumed to pass. Do not repeat old host-only controls,
   disable CCD or blanket rollback. Preserve settling/restitution/empty recycling
   and original full-duration dragging physical checks; explicitly defer only
   cross-engine scene-trajectory assertions under the user scope amendment.
2. Resolve demonstrated ragdoll physical defects and Rain failures using
   identical-input calculation comparisons and distinct evidence-supported
   finite budgets. Strict cross-engine ragdoll trajectory matching is deferred;
   it is not the next solver target. Read historical
   criteria/rejected experiments first. For Rain separate actual GPU wait from
   mirror/deferred host CCD/collector costs; no closed 30/30/180 cell or old
   900-second attempt may be restarted unchanged. Final full-state/repeat/build/
   performance qualification remains PR05–PR08.

PR09–PR11 are now the user's first unfinished priorities: every existing CPU
sample compared with GPU for obvious differences, including floor visibility
(Village's shared
65,536 renderer slots cannot flatten both 52,500-child compounds), sample-wide
UI/widgets and raycast numeric/visual parity. PR12 remains after core release
qualification and supplies fresh desktop-only real CPU versus ordinary/native GPU
charts. Measure rendering FPS separately from completed-step
ms/throughput/p95 with matched binaries/settings and distinct consistent colors.
Diagnostic Mesa/Xvfb graphics with NVIDIA compute cannot establish desktop FPS.
Laptop validation/data remain deferred. Use `SKIP_METRICS=1` for diagnostic GPU
snapshots; preserve all existing scenes, clips and datasets.

### Historical evidence ledger

This ledger preserves detailed receipts, failures and decisions without exposing
obsolete job instructions as the current handoff. Earlier recovery prose remains
available in commit `e801b3135f1c2fe7f10e2a692cd623934a7b8a83`. Do not rerun closed
PR01/PR02 campaigns or either exhausted scheduling campaign.

- [Portable build report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-controls-current-builds-2026-10-01/README.md)
- [Portable failure report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-controls-current-apps-2026-10-01/README.md)
- [CPU record/correction evidence](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-controls-focus-apps-2026-10-01/README.md)
- [Portable event-window/GPU failure report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-controls-event-window-apps-2026-10-01/README.md)
- [Portable root-world report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-diagnostic-2026-10-01/README.md)
- [portable repair subset report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-repair-2026-10-01/README.md)
- [direct mapped-CPU cleanup check](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-mapped-cleanup-2026-10-01/README.md)
- [four fresh builds](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-viewer-builds-2026-10-02/README.md)
- [stale coordinate](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-viewer-apps-2026-10-02/README.md)
- [publication race](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-adaptive-apps-2026-10-02/README.md)
- [Atomic/current-screen campaign](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-atomic-apps-2026-10-02/README.md)
- [portable current-source report](../../experiments/gpu-physics/benchmarks/production-readiness/pr01-current-acceptance-2026-10-02/README.md)
- [portable scene report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-scene-captures-2026-10-02/README.md)
- [PR01 report](../../experiments/gpu-physics/benchmarks/production-readiness/pr01-speculative-2026-10-01/README.md)
- [PR02 exclusions report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-api-2026-10-01/README.md)
- [PR02 diagnostic report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-diagnostics-2026-10-01/README.md)
- [PR02 supported-contract report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-supported-2026-10-01/README.md)
- [PR02 population report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-corner-populations-2026-10-01/README.md)
- [PR02 held-input report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-viewer-controls-2026-10-01/README.md)
- [`gpu-warm-start.md`](gpu-warm-start.md)
- [`../gpu-solver-goal.md`](../gpu-solver-goal.md)
- [`../gpu-solver-qualification.md`](../gpu-solver-qualification.md)
- [mixed scheduling goal](../gpu-mixed-scheduling-goal.md)
- [rejected compound-fix report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-fix-2026-10-01/README.md)
- [bounds diagnostic protocol](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-bounds-2026-10-01/protocol.json)
- [AABB validation report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-aabb-validation-2026-10-01/README.md)
- [compound property diagnosis](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-properties-diagnostic-2026-10-01/README.md)
- [property subset repair](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-properties-lifetime-2026-10-01/README.md)
- [portable capture report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-aabb-captures-2026-10-01/README.md)
- [portable current report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-current-acceptance-2026-10-02/README.md)
- [report](../../experiments/gpu-physics/benchmarks/production-readiness/pr03-contract-baseline-2026-10-02/README.md)
- [portable fixture report](../../experiments/gpu-physics/benchmarks/production-readiness/pr03-baseline-fixtures-2026-10-02/README.md)

- [Current Rust baseline and fixed release contract](../../experiments/gpu-physics/benchmarks/production-readiness/pr03-rust-baseline-2026-10-02/README.md): protocol SHAa30762d9a3803bcfca730218b421607a7faf408ba967ef4417b1c9020746a2ca, driver8018terminal0. All33processes/199checks retained,195pass/4fail;100portable rawfiles exclude local driver pipeline-cache blobs. No source/tolerance/default change.
- Original Rain/drag protocol SHA9ab2100327945b5d29cb37a8709ac037310deb1d22a23afb4683855ca82984bb: driver33069terminal1 beforeanyengineprocess, missingxvfb-run. Displayrepair SHAa86ba2c824bbc09a10e0ddd613d24aea058077df1c59734ea24e7b45948a636e uses pinned previouslyproven bundledXvfb. Driver70354terminal1: CPU600complete, ordinaryGPU900second timeout/nohealth. Sevenremainingcells were neverlaunched; distinctremainingprotocolabove preserves these failures and does not repeat them. All clocks incidental, no performance claim.

PR04 preparation path error is retained in `pr04-distance-clamp/preparation-launch-failure.log`: host script was initially placed under an accidental nested engine path; zero builds/engine launches occurred. The same script was moved before protocol freezing; no physics result replaced. Float32 source reconstruction predicts exactly the recorded initial Y values for 60/15 Hz; this is attribution evidence, not a GPU acceptance pass.

Focused-test preparation path error (repository-relative path used inside engine cwd) failed before protocol/build/engine launch; retained `preparation-failure.txt`, zero budget consumption. Corrected host path before freezing. Only cfg(test) invariant source changed; production solver remains the exact retained clamp.
