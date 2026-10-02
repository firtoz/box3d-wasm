# GPU native production readiness

Read `/home/firtoz/work/2026/box3d-wasm/docs/goals/gpu-production-readiness.md`
at the start of every continuation and after compaction or restart. Pursue the
whole objective, maintain this file, and use the goal-scratchpad skill. This is
the authoritative production-readiness queue for `feat/gpu`.

Updated: 2026-10-02. Status: active; full roadmap and verified milestone commit/push authorized by the submitted goal.
Next item: **PR03 — freeze the release qualification contract and baseline, starting with current fixture/criterion applicability and known failures**.

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
Preserve upstream scene geometry, defaults and existing analytical/physical
tolerances. CPU/GPU chaotic trajectories may differ, but unexplained residuals,
penetration, energy gain, missed collisions or stale identities are defects.

Browser/WASM/WebGPU, Windows/macOS, AMD/Intel/Apple and laptop qualification,
native recording/player API parity and complete Box3D API replacement are later
release scopes. They must be explicitly reported as unsupported/unqualified
where appropriate. Diagnostic captures and cached command replay are not native
recording/player API implementations. Keep Box3D and the existing WASM package
unchanged. Their existing workflows must still pass compatibility checks.

## Ordered checklist and acceptance contract

Work on one unfinished item at a time. Its evidence cell is initially `OPEN`.
Add links to committed receipts/reports and the applicable source/build identity
when closing it. Keep detailed physical results in
[`../gpu-solver-qualification.md`](../gpu-solver-qualification.md), API/support
status in [`../gpu-physics.md`](../gpu-physics.md), and this file as the concise
queue and recovery record. No separate competing production backlog.

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

- [ ] **PR03 — Freeze the release qualification contract and baseline.** Reconcile
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
  **Evidence:** [metadata preparation](../../experiments/gpu-physics/benchmarks/production-readiness/pr03-contract-baseline-2026-10-02/README.md);332 source inputs,18 exact Rust fixture selectors/28 numerical selectors/native replay selector. No device/build/timing runs. Full commands/criteria/CPU-oracle/executable baseline and finite evaluation budget remain open; this does not close PR03.
  [Current fixture baseline](../../experiments/gpu-physics/benchmarks/production-readiness/pr03-baseline-fixtures-2026-10-02/README.md) preserves121rawfiles:10fixturelinks,26first-process observations, original failedCPUlink and separate dependency repair. BothGPU distance probes retain the original first-step1e-5failure; ragdoll physical/isolated/57posemesh screens pass, original strict0.006m trajectory screen fails0.063585767m. No source/default/tolerance/timing change; fullmatrix/currentRain/loadeddrag/Rust baseline and future-state contract remain open.

- [ ] **PR04 — Resolve required physical correctness failures.** Work from the
  earliest demonstrated defect and smallest faithful reproducer. Close Rain's
  persistent/severe joint residuals and applicable dragging, joint, contact,
  CCD and energy/support failures from PR03. Read the old solver recovery and
  rejected experiments before choosing a new hypothesis. Numerical reference
  roots, partial captures and CPU reproduction are diagnosis, not GPU physical
  acceptance. A retained fix needs original-limit physical checks on both
  backends and relevant regressions; restore rejected production edits and
  preserve unfavorable evidence. Rain recycling must retain correct lifetime
  correspondence and healthy constraints throughout the full 600 steps.
  **Evidence:** OPEN.

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
  Run the PR03 matrix against PR06 binaries. Require at least five fresh processes
  per selected configuration on each backend, with complete required durations:
  Rain/ragdolls 600 steps, dragging 3,060, and existing fixture-specific durations
  for other capabilities. Compare future-relevant semantic state every step,
  including bodies/sleep, contact geometry/impulses/history, joints/caches,
  lifetimes, policy and replay state. Retain meaningful ordering; exclude only
  audited incidental fields. Validate compact/lossless capture against full
  records and corruption controls. Report first discrepancies. No pose-only,
  interrupted, skipped or historical run counts as a final pass. Preserve all
  analytical, precision, island, schedule-equivalence and capacity checks.
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

- [ ] **PR09 — Fix and qualify GPU floor visibility.** After PR01–PR08 pass,
  reproduce the user's report of invisible floors, identify every affected
  supported GPU sample and fix the demonstrated rendering cause. Check static,
  compound and mesh ground representations where used, camera framing and
  occlusion against the real Box3D CPU view. Preserve scene geometry and physics
  defaults; a rendering repair must not change collision/support behavior.
  Record and review all affected scenes before committing, CPU first in the
  comparison grid, and retain before/after evidence. The new [combined Village capture](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-ownership-captures-2026-10-01/README.md) reproduces missing nearby GPU compound ground/buildings while CPU geometry is visible. Unchanged shared65536-slot renderer pool cannot flatten both52500-child compounds; source audit and exact clip are retained. Fix/qualify this and inventory the other supported samples; document unresolved cases.
  **Evidence:** OPEN.

- [ ] **PR10 — Qualify sample UI and widgets end to end.** After PR09, inventory
  the controls and widgets exposed by supported GPU viewers and samples, then
  exercise actual mouse/keyboard interactions on ordinary/native GPU and
  combined viewers with a real CPU comparison. Cover sample selection, sliders,
  buttons, toggles, dragging/picking, overlays, pause/single-step/restart and
  resize/layout where available. Verify displayed state and the intended
  physics effect, including scene changes and destruction/recreation; a widget
  that accepts input but leaves stale state does not pass. Retain interaction
  evidence and record affected scenes before committing fixes. PR02's required
  truthful-control checks still belong in PR02; this is the later sample-wide
  usability check requested by the user.
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

- [ ] **PR12 — Publish the supported release and fresh desktop comparison.**
  After PR09–PR11, review their changes for evidence applicability and reopen/
  refreeze affected PR06–PR08 gates before final delivery. Create a new dated
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

Read this section before continuation; the checklist above remains authoritative.
Historical reports preserve their original failures, budgets and source snapshots.
Their old next actions and job handles do not authorize a new campaign.

| Field | Verified value |
| --- | --- |
| Workspace / branch | `/home/firtoz/work/2026/box3d-wasm`, `feat/gpu` |
| Authorization | Active full roadmap; implementation and verified milestone commit/push authorized by the submitted goal |
| Latest evidence milestone | `e801b3135f1c2fe7f10e2a692cd623934a7b8a83` pushed; remote0/0 and clean tree verified after push. New121rawfiles preserve current PR03 fixture builds and26baseline processes; no production change |
| Retained production source | `abc0a54a3ce3b285c9984f2c7c6b5baefff00d1b`, pushed. PR01 refresh `c2fd69f`, PR02 acceptance `1304839`, PR03 metadata `5a40f05` are pushed. Invocation revisions are context, not compiled source identity |
| Current item | **PR03 open**. Current ragdoll/mesh/distance fixture baseline is portable; remaining fixed matrix, Rain, loaded dragging, exact Rust selectors and future-state contract/current baseline remain next |
| Current jobs | All owned fixture build/baseline/publication processes terminal; no live owned GPU/build/capture job. Old sessions46787/84255/80345/31931 are terminal; never restart their closed drivers |
| Current compiled libraries | Ordinary SHA`aba7a834443ba7174d92bcf39217929ce9af83adc28ea9c244943b5e72eb6ec6`; native SHA`c8a16154cb576c49e7bd7fd8e030319601192b466d9ddb19305bd043aa6a7267`. Exact107Rust/723viewer inputs still match their producer receipts/source archive |
| Independent CPU | Full archive SHA`b35f71d07515e333fa19f3e6297bbfc2040984a49f366c94afc5f974acef2c0c`,119actual source/object pairs match CPU applicability/origin receipts. Combined fixtures use separate prefixed CPU archive. Older parent Rust receipts do not identify current GPU code |
| Hardware / policy | i9-9900K / NVIDIA GeForce RTX4070SUPER, driver610.57.04, actual NVIDIA Vulkan. Ordinary/native qualified separately. Current PR03 cells use efficient ordering0; scheduling/default policies unchanged |
| Scope protection | Box3D/WASM unchanged. Mixed scheduling target remains **unmet**, all55trials preserved; both historical scheduling budgets exhausted. Do not resume older Rain/scaling/benchmark next actions. New Rain work is required under this roadmap's separately frozen PR03–PR04 protocols |

### Latest closed budgets and observations

[Current PR03 portable report](../../experiments/gpu-physics/benchmarks/production-readiness/pr03-baseline-fixtures-2026-10-02/README.md)
contains121indexed rawfiles, original sources/protocols, actual link receipts,
producer receipts, all outputs and unchanged evaluators. Offline validator
passes and every indexed byte was verified in Git staging. Derived evaluator
imports suppress bytecode so repeated offline checks preserve the raw index.

- Original build protocol `b84f3265…`: three sharedC commands pass, firstCPU
  ragdoll link fails because the harness chose the GPU geometry-only archive;
  nine links remain unlaunched. Runner46787 terminalexit1. Preserve original
  receipt/log; no engine ran.
- Separate dependency repair protocol `0b745a9f…`: reuse three successfulC
  objects, full independent CPU archive, ten links/twelve translation units;
  allten binaries link, assertions active. Runner84255 terminalexit0. No
  Rust/viewer build or engine execution in either build budget.
- Distance baseline protocol `37a6d790…`: two first processes, ordinary/native
  combined. Both abort signal6 at distance step0/lane1, GPUY=-1.00004232 versus
  CPUY=-1.00028491, outside original1e-5. Driver80345 terminalexit0 after both
  planned cells; child failures remain failures. No240step distance completion
  or later four-substep distance case is inferred.
- Ragdoll/mesh protocol `d3a4df17…`:24first processes,8CPU+8ordinary+8native,
  allcomplete; driver31931 terminalexit0. Both backends pass unchanged600step
  ragdoll physical limits, four120step isolated cases,60step collision-free
  ragdolls and45grid/12torus mesh poses. Original full ragdoll reference screen
  fails first60step position error0.063585767m versus0.006m on both. Its later
  strict visual checks are unexecuted. Retain this independently of physical
  passes; no tolerance changed, cause unproven.

Allfour new protocols freeze zero candidates/retries/headline timing. Total26
engine processes are first baseline observations, not final five-run/full-state
release qualification. Incidental clocks never enter performance charts. Do
not repeat completed cells or extend these closed budgets.

### Next discriminating work

1. Finish PR03's fixed current contract and executable baseline. Freeze exact
   current Rust analytical/CCD/island/history/query/replay/capacity/schedule
   selectors, original durations/assertions and finite evaluation budget before
   launch. The existing metadata draft identifies18fixture/28numerical selectors
   and native replay, but is not a device pass. Current CCD filter count must be
   derived from current host lists rather than copied from the old27-test total.
2. Include original600step `Benchmark/Rain` and3060step loaded dragging criteria,
   actual current CPU/ordinary/native viewer/binary commands, creation/lifetime
   correspondence and required capture fields. Current source-linked fixtures
   are built; no fresh Rain or drag process has yet consumed a PR03 budget.
3. Preserve the original distance first-step and strict ragdoll trajectory
   failures in PR04's required failure register. Read historical rejected
   experiments/criteria before selecting a new hypothesis. A numerical root,
   CPU reproduction or physical-only pass cannot silently close another screen.

Local immutable campaign directories:
`experiments/gpu-physics/artifacts/production-readiness/pr03-baseline-builds/`,
`.../pr03-baseline-cpu-link-repair/`, `.../pr03-distance-baseline/`,
`.../pr03-ragdoll-mesh-baseline/`. Exact frozen commands/settings and artifacts
are also portable in the current report; ignored local paths alone are not
acceptance evidence. The accepted Rust test executables remain under
`.../pr02-world-lifetime-artifact-repair/`: ordinary SHA
`597b893bcbbb401866721e1b1b45cb03c58263ae8bee76cbe8af00cdfaef23b9`,
native SHA`34ede43142323516f585d4d236755b8ef46dc6d9e61b6ba51354f379c85aa4a3`.
Verify actual source/artifact hashes before reuse.

### Remaining applicability and release boundaries

PR01/PR02 named functional gates are closed, with exact current acceptance
reports linked in their checklist cells. PR01's additional all-window floor
screen and archived startup timeout remain failed/incomplete. PR02's32C+20Rust
processes pass under original assertions: actual16GPU+4host classification;
the original false GPU-banner harness stop is retained, and onlyone previously
unlaunched native host case uses the separate remaining budget. Current inventory
415stateful symbols/376implemented candidates/39explicit exclusions does not
prove376 independent physical behaviors. Source/archive/link proof is distinct
from semantic qualification.

Retained lifetime repair uses persistent root/child epochs, generation-safe
routing/destruction, matching CPU-map cleanup and stale/foreign joint endpoint
checks. Its72selected Rust checks,120first root/geometry C observations and42
mapped-cleanup observations remain distinct from broad lifecycle qualification.
PR05 must reconcile raw Rust world-index range with C metadata/viewer caps and
exercise concurrency/reentrancy/loss boundaries. PR07 must audit new registry
root/child epochs and ABI allocation state: schema24 does not yet prove capture
of every future-relevant field. No promise of complete-state determinism follows
from pose records or selected110step repeats.

The current four viewer controls pass116actions/44screenshots;52CPU-first clips
and26reviewed pairs preceded sourceabc0a54. Twelve native300step health cases
pass. These are functional/scene checks, not complete physics or performance.
Falling Cubes previews256bodies, not the50000body timing fixture; Mixed Stacks
has600dynamic/602total bodies. Native graphics used Mesa/Xvfb diagnostically,
while GPU physics used NVIDIA Vulkan. Such captures cannot establish desktopFPS.
Use `SKIP_METRICS=1` for new GPU snapshot work. Cached dependencies/NFD and
post-build dependency audits do not close PR06 clean-build acceptance.

PR09 retains reproduced Village missing GPU compound ground/buildings: unchanged
shared65536-slot renderer pool cannot flatten both52500-child compounds. Other
floor samples still need inventory. PR10/PR11 retain sample-wide widgets/raycast
numeric and visual checks. PR12 must show fresh desktop-only realCPU versus
ordinary/native GPU charts, with distinct consistent colors, matched exact
binaries/settings and separately measured renderingFPS/completed-stepms/
throughput/p95. Laptop validation/data remain deferred. Record every affected
scene CPU first before solver/visual commits; preserve old clips/datasets.

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
