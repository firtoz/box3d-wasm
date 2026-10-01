# GPU native production readiness

Read `/home/firtoz/work/2026/box3d-wasm/docs/goals/gpu-production-readiness.md`
at the start of every continuation and after compaction or restart. Pursue the
whole objective, maintain this file, and use the goal-scratchpad skill. This is
the authoritative production-readiness queue for `feat/gpu`.

Updated: 2026-10-01. Status: active; full roadmap and verified milestone commit/push authorized by the submitted goal.
Next item: **PR02 — fix combined compound ownership, then qualify scalar getters and viewer controls**.

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

- [ ] **PR02 — Close the native API contract and remaining truthful-control gaps.**
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
  **Evidence:** [partial native audit/error-contract report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-api-2026-10-01/README.md).
  All 415 stateful symbols are inventoried; 39 exclusions report explicit errors.
  The [diagnostics campaign](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-diagnostics-2026-10-01/README.md)
  passes 8 C processes, 4 Rust processes, 16 preserved regressions, 16 host controls
  and four viewer builds. Shared routing is corrected. The 40 CPU-first standard clips are validated/reviewed. A two-app viewer campaign stops on a retained mouse-stimulus harness failure. The [supported-contract campaign](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-supported-2026-10-01/README.md) passes 16 linked C processes and 20 Rust checks on exactly matching compiled inputs. The [corner populations campaign](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-corner-populations-2026-10-01/README.md) passes the independent CPU and standalone ordinary/native checks, then exposes duplicate CPU child colliders in the combined compound constructor. Native combined is unlaunched at the stop rule. The held-input host proof passes; all actual viewer controls remain OPEN. The [ownership candidate](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-fix-2026-10-01/README.md) is rejected and restored after bounds failure; the [separate bounds diagnosis](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-bounds-2026-10-01/README.md) confirms zero public compound AABBs on both backends. The [AABB repair](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-aabb-validation-2026-10-01/README.md) passes all12 first API validation processes; the [precommit captures](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-aabb-captures-2026-10-01/README.md) pass all12 CPU/GPU scenes at300 completed steps each and all six comparisons/callers are reviewed. AABB source/evidence milestone `776577937bce5e9a265264f327c524f4f1e4cc4f` is committed and pushed to `feat/gpu`; remote0/0 and a clean tree verified after push. PR02 stays open.

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
  **Evidence:** OPEN.

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
  comparison grid, and retain before/after evidence. The reported symptom is
  unconfirmed until reproduced; document tested samples and any unresolved case.
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

| Field | Verified value |
| --- | --- |
| Workspace / branch | `/home/firtoz/work/2026/box3d-wasm`, `feat/gpu` |
| Goal / authorization | Active full roadmap; implementation and verified milestone commit/push authorized by the submitted goal |
| Delivered source | PR01 `695c93ada50220089018009230209e5050ed9252`; exclusions `806e634c9b0f5fe58fdd8a9a0dac171e862890d4`; truthful diagnostics `d4e91bca235ce3565b66d5ee031565c857806ede`; supported API/evidence milestone `2c9d832564e497242b661c692fb22d0ac15dc575`; population regression/diagnosis milestone `30b2972b520f4df76d8275e92b385656183f9007` is committed/pushed with remote 0/0 and a clean tree verified. Compound ownership rejection/bounds diagnosis milestone `6cb0cca3470c79dfb02888d9083dc786dc0de940` is committed/pushed. Retained AABB source/API/capture/evidence milestone `776577937bce5e9a265264f327c524f4f1e4cc4f` is committed/pushed; remote0/0 and clean tree verified after push. Later recovery-only commits do not change compiled inputs; verify actual HEAD/remote on continuation |
| Current item | PR02: standalone sensor/compound/three-plane mesh diagnostics now pass on ordinary/native. Combined compound creation is defective: public dual child constructors add CPU colliders before the full CPU compound. The attempted ownership candidate is restored after bounds failure. The baseline demonstrated zero public compound AABBs and native/CPU rotated-box differences. Bounds repair passes all12 first API validation processes and all12 precommit CPU/GPU captures; six scene/caller reviews complete. AABB milestone is pushed; combined ownership, scalar getters and viewer controls remain required next work. PR03–PR13 unchecked |
| Current source/build proof | Retained AABB milestone changes seven production files; prior diagnostic binaries remain frozen historical evidence. Compile-only builds provide exact new source snapshots and hashes: ordinary library096d9a9bfad7ba87677723ba37f3b6ee413f7ed266682b53927b7ebe3cc897a0 / test8f2b164cd88edd81efaffd0705e75db279d70aa678cf72a4139ad560205a4763; native libraryf5d3c3856536d53b8f8bfef51655cd062aed414adfeb68e6be80905be1d6d366 / test00642a2e11e4ea879b0291bbf053e37f5f31aa656be88eb165c646c5e913eb30. Baseline frozen diagnostic inputs remain preserved: Ordinary library SHA-256 `edbdb251e06e9c9bc5955045191e4ca2932bfacae6265581a0acfe3be16a6050`, native `a736a81fae6b45751fadd0f57dcf795a0e44f7c21d1e54996c668b754cc393e9`; exact tests, archives, generated adapters and linked CPU inputs are identified in the reports below |
| Linked inventory | 415 stateful symbols, 39 explicit exclusions; no remaining detected stub/placeholder/missing/duplicate symbol or CPU-only combined passthrough. Source/link coverage does not qualify all implemented behavior |
| Supported API campaign | Runner 81222 exits 0: 16/16 C processes and 20/20 Rust checks (18 GPU, 2 host), unchanged assertions/tolerances; all completed results retained. Portable validator passes 73 raw files. No production changes, candidates or timing runs |
| Diagnostic campaign | Exhausted: 2/2 baseline reproductions, 8/8 candidate C, 4/4 Rust, 16/16 preserved regressions and one 16-control host suite. One candidate, no timing. Baseline reproductions get no acceptance credit; do not repeat/extend |
| Actual UI campaign | Stopped after CPU + ordinary GPU, both exit 0. GPU Profile shows measured intervals/step IDs; attempted Counters/Frame Time screenshots still show Profile. Immediate synthetic mouse down/up did not select tabs. Three apps unlaunched, no interaction pass or rerun |
| Required-control stimulus | New protocol stops before all five apps after compound defect discovery. Pure host receiver proves mouse hold251ms/key100ms; four portable files preserve observed events/source/log/protocol. Never change old app hashes to represent a fix; after rebuilding freeze a new finite viewer protocol and reuse directly applicable successful host proof |
| Recordings | Prior40 standard diagnostic clips preserved. New12 CPU-first Compound captures all complete300 steps, finite health/capacity,0 Sokol errors,1280×720/30fps; six comparisons reviewed. GPU column `2026-10-01-compound-aabb` follows CPU through persisted manifest chronology. Portable103 raw files plus clips, manifests, review sheets and browser screenshot validate; actual Play/Pause loads both Village media without errors. This is scene/health evidence, not native control/physical/performance qualification |
| Population campaigns | Two CPU harness failures retained separately (wrong compound ID count; coplanar grid cannot produce multiple patches), each CPU1/GPU0. Corner campaign stops after CPU0/ordinary-GPU0/native-GPU0/ordinary-both-6; native-both unlaunched. All 41 portable files validate; neither previous failure is replaced. Original physical limits unchanged |
| Compound-fix campaign | Closed at first failure: baseline2/2, one candidate, ownership1/4, existing C regressions0/8, timing0. Candidate first sphere count GPU1/CPU1, then unchanged bounds1e-5 check fails GPU-1.57911253/CPU-1.6875484. All10 candidate fixtures built,11 processes unlaunched. Production adapter restored to exact baseline SHA; 32 portable raw files validate in the linked report |
| Compound-bounds diagnosis | New distinct public/extended AABB requirement: source proves GPU public parent uses zero-sized placeholder, while native diagnostics union child world bounds and CPU transforms the local enclosing box. Frozen diagnostic2, candidates0, timing0; all six lanes for sphere/capsule/hull/mesh, rotated/unrotated, independent CPU geometry. Runner exec81152 exits0, both processes complete; mapped CPU matches independent CPU in all8 cases per backend at original1e-5, public GPU bounds zero for all cases. All96 lane observations/10 raw files validate; diagnosis only, no API/physics acceptance |
| Compound AABB candidate | Original protocol `pr02-compound-aabb-fix-2026-10-01/protocol.json` is rejected at build: Rust E0282 map-index inference failure; ordinary/native libraries both exit101. Shell incorrectly proceeded to native after ordinary failure; both logs retained, no validation/GPU/timing process ran, all12 validation cases unlaunched. Seven production files restored to baseline before repair. Source refinement/initial protocol/rejected source are retained; no API acceptance |
| Compile-only repair | New distinct compiler requirement protocol `pr02-compound-aabb-compile-2026-10-01/protocol.json`: one explicit HashMap<ShapeId,usize> annotation, ordinary/native library+test builds4, GPU0/timing0; stop/restore first failure. Same bounds arithmetic/import behavior, no CPU ownership change or original-budget rerun. Driver82382 exits0: all four library/test compile commands pass with exact before/after source snapshots and library/test hashes. No GPU process or acceptance credit. These four compile commands alone supply no API acceptance; later first API validation is recorded in the next row |
| AABB API validation | Fresh first-validation protocol `pr02-compound-aabb-validation-2026-10-01/protocol.json` pins compiled repair receipts/hashes, one variant, Ccontract4/Cregression4/Rustsuite4, baseline0/timing0; original build-rejected budget remains closed. Runner12492 exits0: all12 processes pass; four Ccontract processes validate1280 six-lane bounds results against independent CPU at unchanged1e-5, four unchanged C regressions and four Rust suites (18query+1diagnostic per backend) pass. Fresh C adapters and actual108 built C units/backend have source/object/linked archive proof. Portable validator passes74 core raw files across failed build/compile repair/validation; five subsequent native-viewer build proof files are added for precommit captures. All12 later precommit captures pass/review in the separate campaign; milestone `776577937bce5e9a265264f327c524f4f1e4cc4f` is committed/pushed. No ownership or final physics qualification |
| Processes | AABB compile/build/validation/viewer/capture runners81218,82382,12492,94269,39314,33642,90304 and publication99474 are terminal. CPU6/GPU6 captures complete; no live physics/recording/timing jobs. Local comparison HTTP server10795 remains available at127.0.0.1:8766; browser gallery is a handoff view. Verify actual state before reuse |
| Remaining known physics/performance limits | Rain residuals and distance-joint first-step discrepancy remain unresolved. PR01 additional all-window 5mm floor screen fails CPU and GPU at step 6; discrete-landing analysis is retained, original isolated CCD limits unchanged. Native baseline startup diagnostic times out at 180s, no pass. Diagnostic host occupancy scans/device peak atomics add unmeasured work; PR08 must assess cost |
| Late-stage user requirements | Added 2026-10-01: finish core PR01–PR08 first, then floor visibility PR09, sample UI/widgets PR10 and raycast behavior/appearance PR11. These are unchecked user observations/requirements, not reproduced defects or completed fixes. PR12 delivers a fresh desktop-only CPU/GPU FPS, completed-step ms, throughput and p95 chart set; laptop work remains deferred. Final publication/audit moved from PR09/PR10 to PR12/PR13; earlier gate IDs and current PR02 priority are unchanged |
| Next action | Freeze a new finite ownership candidate protocol on the accepted AABB inputs. Reuse unchanged exact Rust library provenance, build fresh ordinary/native combined adapters, and fix private primitive child creation without CPU dual mirroring. Preserve public constructor mirroring and independent topology/bounds/regression tolerances. The older ownership rejection remains failed and closed; the now-qualified AABB contract is a changed prerequisite, not a replacement result. scalar getters and actual viewer controls remain required PR02 work. Never rerun the closed AABB or capture campaigns |
| Last verification | 2026-10-01: API evidence validator79 raw files/12 passes at unchanged1e-5; capture validator103 raw files/12 passes,3,600 completed scene steps and six CPU-first visual comparisons reviewed. Exact fresh CPU/native viewer compiled source/unit/object proof and supplemental third-party source bytes preserved. AABB source/evidence milestone `776577937bce5e9a265264f327c524f4f1e4cc4f` is committed/pushed; remote0/0 and clean tree verified after push. Later recovery-only edits do not change compiled inputs. No Box3D/WASM, solver/shader, policy/default or timing changes |

### Evidence to resume from

- [PR01 report](../../experiments/gpu-physics/benchmarks/production-readiness/pr01-speculative-2026-10-01/README.md): completed world controls, rejected variants, original-limit repeat/regression checks, recordings, additional failed floor screen and incomplete baseline attribution. Retain those failures. Third candidate preserves 6.25mm support only after a solid CCD hit (FAST set, CCD_NO_HIT clear), not a fast tangent gap. CPU's world toggle only stores the flag; the pinned hull/mesh shape scope is the independent reference. Final physical qualification is still PR03–PR07.
- [PR02 exclusions report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-api-2026-10-01/README.md): 39 ENOTSUP operations, NULL recording creation, sticky thread-local operation names and combined routing. Closed eight-process host budget has seven passes and a retained errno-after-libc harness failure. Separately frozen closed-stderr case passes; no failed trial replaced.
- [PR02 diagnostic report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-diagnostics-2026-10-01/README.md): 269 portable files, exact build/input receipts, all 40 clips, CPU-first galleries and stopped UI protocol. Counter reads synchronize public non-sensor contacts; seven profile aliases use NaN/mask/timestamp availability; occupancy peaks differ from reservation sizes. Simple box fixtures do not qualify all public populations.
- [PR02 supported-contract report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-supported-2026-10-01/README.md): immutable protocol, all 16 C / 20 Rust results, 73 raw files, compiled fixture archive and source applicability. Independent CPU comparisons cover named forces/replacement/joint contracts at original 1e-5 tolerances. Warm-start incidental clocks are not performance data. Wheel angular separation remains unavailable upstream.

- [PR02 population report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-corner-populations-2026-10-01/README.md): original two CPU harness failures, successful CPU/standalone ordinary/native corner controls, failed combined topology and linked-source cause. Ordinary combined raw assertion does not print actual CPU numbers; duplicate shapes are explained by source, not invented numeric observations. Three CPU and three GPU processes consumed across three closed campaigns, no candidates/timing.
- [PR02 held-input report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-viewer-controls-2026-10-01/README.md): one pure host pass, zero viewer apps. This supersedes the immediate-click hypothesis with received events, but does not prove actual control interactions.

Local frozen engine/test inputs are under
`experiments/gpu-physics/artifacts/production-readiness/pr02-diagnostics/candidate-complete-inputs/`.
Four C/viewer build directories are under `.../pr02-diagnostics/c/<ordinary|native>-<gpu|both>/cmake`.
Supported runner, executables and terminal receipt are under
`.../pr02-supported/`; exact commands/environments are also portable in its report.
Do not run those closed runners again. Use a new protocol only for an explicit
uncovered requirement or a distinct evidence-supported hypothesis.

Schema v24 adds host/device occupancy peaks. Device query word 74 is independent
of ray/status/phase/allocation workspaces, never feeds solving, and persists
across unread steps, retirement and growth. Earlier schemas remain supported.
Routine viewer accounting stays cheap; explicit public diagnostic reads may wait.
Profile polling does not guarantee a timestamp is available on the first read.
Busy/reentrant diagnostic calls return status; clearing errors never heals a failed world.

The ordinary recorder (`9311af7b0a6f47c0ba2c8b011c028904460baefdae99d3f99ef0eb8f52f06368`)
and real CPU oracle (`848477c467c448fcbbd938127f411ba32ad0b87ab7343579c65efff9a3238250`)
have separate receipts. Native viewers likewise have separate compiled-input proof.
Use `SKIP_METRICS=1` for GPU snapshot work. Record every affected scene before
solver/visual commits, real Box3D CPU first, without creating an unbudgeted timing campaign.

Historical sources remain useful for criteria and rejected approaches:
[`gpu-warm-start.md`](gpu-warm-start.md),
[`../gpu-solver-goal.md`](../gpu-solver-goal.md),
[`../gpu-solver-qualification.md`](../gpu-solver-qualification.md).
Their old jobs/next actions do not override this record. The
[mixed scheduling goal](../gpu-mixed-scheduling-goal.md) is closed with its target
unmet and all 55 runs retained; the earlier scheduling budget is also exhausted.
Do not reuse either budget. Rain is required by this roadmap later, but no Rain
or performance campaign has resumed during PR02.

Current population artifacts and terminal receipts are under
`experiments/gpu-physics/artifacts/production-readiness/pr02-populations/`,
`.../pr02-population-identities/` and `.../pr02-corner-populations/`.
The first two archived fixtures preserve their incorrect assumptions. The current
`c_abi/native_diagnostic_populations.cpp` is a correct source-audited regression
reproducer: standalone passes, combined fails. Do not weaken its CPU topology
assertion to match duplicate shapes. The compound mesh path already creates GPU
children directly; sphere/capsule/hull paths invoke public dual constructors.
Source inspection identifies five affected native registrations in
`box3d/samples/sample_compound.cpp`: Compound/Simple, Spheres, Hulls, Tile Floor
and Village. Compound/Mesh Tile uses direct GPU mesh children and is a required
unaffected control. Preserve their defaults and record every affected scene
before a relevant production commit.
The new viewer-control protocol/receiver lives under `.../pr02-viewer-controls/`;
all app budgets are unlaunched and the protocol is stopped because adapter inputs
must change. Reuse received-event proof only after auditing stimulus/source
applicability. All older population/ownership jobs are terminal and their rejected edits restored. The later bounds repair is retained as recorded above. Closed compound campaigns and their remaining required API/ownership work are summarized above.

The [rejected compound-fix report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-fix-2026-10-01/README.md) retains two numeric baseline reproductions, exact candidate build proof and the failed bounds check. Production adapter is restored, not accepted from its first matching count. The separately frozen [bounds diagnostic protocol](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-bounds-2026-10-01/protocol.json) addresses a distinct required API gap; no ownership or final physics acceptance is inferred from diagnostic completion.

The [AABB validation report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-aabb-validation-2026-10-01/README.md) links the retained compiler failure, annotation repair, exact new libraries/tests/C adapters and all12 successful first validation processes. The source/caller and separate CPU-first scene review now complete; AABB milestone `776577937bce5e9a265264f327c524f4f1e4cc4f` is committed/pushed; do not present an unchecked later ownership candidate as covered by these bounds results.

Source audit also identifies compound scalar getters for later PR02 qualification: `b3_shape_get_friction`, `b3_shape_get_restitution` and `b3_shape_get_user_data` depend on `query_shape`, which rejects public compound parents. This is an unexecuted source finding, not invented CPU/GPU numeric evidence. Add independent faithful coverage before closing PR02; do not alter the closed AABB budget for it.

Precommit captures are complete in the [portable capture report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-aabb-captures-2026-10-01/README.md). Frozen protocol SHA25e7885e67e3c9681ea3f205d9466d09fc2e76655e305e54c3141121cf367ecb consumes CPU6/GPU6, timing0/retries0, all12 pass within600s/scene watchdog. Fresh CPU viewer SHA9d879afb242a73049e62c5a1ac85c2b701e3c7b070148605cb62b6e8339a28a0 and native GPU viewer SHA164b03b5f55cfc8f02b7d49f1d7d1155ff7557057047cb39377e251588f98a61 have exact compiled-unit/object/archive proof. Compound/Simple, Spheres, Hulls, Tile Floor, Village and Mesh Tile control preserve upstream defaults/assets, sleep/warm-start/CCD and60Hz/four substeps. All six scene comparisons/callers are reviewed; clips include startup, while review sheets sample completed scene windows. Native viewer interactions and capsule trajectory equivalence are not inferred from passive captures. Third-party compiled sources match build hashes; dependency headers are a post-build audit, so PR06 must freeze complete final-build dependencies. The comparison generator's postcapture chronology correction puts this GPU column next to CPU using persisted recording manifests; no compiled viewer input or older clip/data is changed. All budgets are closed; do not rerun.
