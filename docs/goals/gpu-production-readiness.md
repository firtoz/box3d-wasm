# GPU native production readiness

Read `/home/firtoz/work/2026/box3d-wasm/docs/goals/gpu-production-readiness.md`
at the start of every continuation and after compaction or restart. Pursue the
whole objective, maintain this file, and use the goal-scratchpad skill. This is
the authoritative production-readiness queue for `feat/gpu`.

Updated: 2026-10-01. Status: active; full roadmap and verified milestone commit/push authorized by the submitted goal.
Next item: **PR02 — finish supported API behavior and actual viewer-control qualification**.

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
  and four viewer builds. Shared routing is corrected. The 40 CPU-first standard clips are validated/reviewed. A two-app viewer campaign stops on a retained mouse-stimulus harness failure; actual counter/tab interactions and broader supported API behavior remain OPEN.

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

- [ ] **PR09 — Record and publish the supported release.** Record every affected
  scene before committing visual/solver changes, with real Box3D CPU first in
  the comparison grid. Publish readable charts, portable raw timing/check data,
  build/source receipts, scene/recording hashes and a compact final coverage
  matrix. Document API semantics, capacity/failure behavior, performance limits,
  default/override selection and reproduction commands. Clearly state the
  verified hardware/platform scope and all exclusions. Keep earlier datasets
  and failed experiments. Delivery must be reviewable without ignored local
  binaries or captures; local artifact paths alone are insufficient evidence.
  **Evidence:** OPEN.

- [ ] **PR10 — Audit completion and deliver the branch.** Verify PR01–PR09 against
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

Suggested activation prompt (only submitting it activates implementation and
provides the stated commit/push authorization):

```text
/goal Complete the Linux/NVIDIA GPU production-readiness roadmap in /home/firtoz/work/2026/box3d-wasm/docs/goals/gpu-production-readiness.md. Read it after every compaction/restart, maintain its recovery record, and mark [ ] as [x] only when acceptance evidence passes. Continue through all required gates, preserving its scope and constraints. Commit and push verified milestones to feat/gpu.
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
| Workspace / branch / delivered milestone | `/home/firtoz/work/2026/box3d-wasm`, `feat/gpu`; solver milestone `695c93ada50220089018009230209e5050ed9252`; partial API milestone `806e634c9b0f5fe58fdd8a9a0dac171e862890d4`; recovery delivery `90efcc44b910233436519f0bee84dff9a38dcacb`. Resolve current HEAD and remote synchronization on continuation |
| Goal / authorization | Active full roadmap; implementation and verified milestone commit/push explicitly authorized |
| Current item | PR02 in progress: unavailable operations and focused diagnostic APIs are implemented/verified. Broader supported API behavior and actual counter/tab interactions remain open. PR01 delivered; PR03–PR10 open |
| Current implementation | Diagnostic C/Rust/header APIs, shared combined routing, UI labels and schema v24 are verified by the focused campaign. Forty required CPU-first clips are validated/reviewed. Partial diagnostic milestone `d4e91bca235ce3565b66d5ee031565c857806ede` is committed/pushed. No broader API/viewer/performance acceptance claimed |
| PR01 delivered candidate | Third candidate keeps6.25mm support only after an actual solid CCD hit (FAST set,CCD_NO_HIT clear); fast tangent/no-hit gaps remain disabled. Default-on unchanged |
| PR01 closed focused checks |20/20 C confirmations,4/4 stricter guards,CPU tangent reference,10/10 fresh110step raw-v23 repeats with every-step health,126/126 frozen regressions,14storage/comparator controls |
| Linked API audit | Both current linked builds inventory 415 stateful symbols: 39 explicit exclusions, 0 remaining stubs/placeholders, 0 missing/duplicate symbols and 0 CPU-only comparison passthroughs. Implemented behavior remains unqualified outside the recorded focused cases |
| Build/source proof | Frozen ordinary/native library and test receipts;10engine input sets resolved byte-exactly from base Git plus source overlays. Renderer/oracle and serial native-viewer compiled inputs/binaries recorded separately |
| Native scene finding | Zero ghost launches/healthy300step CPU/GPU runs. Both FAIL additional5mm all-window screen on step6. Offline source calculation independently matches both engines’ first6heights/vertical velocities exactly asfloat32; unchanged mesh glancing filter leaves discrete landing. Original isolated0.495m CCD bound/tolerances remain passing and unchanged; failed extra screen retained |
| Attribution budget | One separately frozen pre-PR01 native baseline diagnostic consumed; startup times out180s,0completed results. Retained logs/invalid capture;no qualification credit or selective retry |
| Original rejected budget | Zero-shell:baseline1,CPUreference1,CPUdiagnostic1,GPUtrial1/20,GPUdiagnostic1;CCD fails,remaining19cancelled. Broad guard:20C/4guard/10state/126regressions pass but semantic no-hit gap defect rejects it. Neither budget reused |
| Recordings | PR01 clips preserved. New diagnostic milestone: 40 standard CPU/GPU clips,300frames,1280×720,30fps; real CPU first/latest GPU next, wide mixed camera. Portable full clips/manifests/hashes/review sheets included. Previous CPU column archived; older native CPU reference restored for its existing row |
| Processes | All builds/tests/recordings are terminal. Standard runner 85415 and sheet runner 80429 exit 0; CPU viewer 45878 and ordinary-GPU viewer 57613 exit 0. Viewer campaign stopped on missing tab selection, three other apps unlaunched. Fresh inspection finds no physics/build/recording jobs; verify before resuming |
| Portable evidence | PR01/partial PR02 datasets preserved. PR02 diagnostics report contains 269 raw files (~73 MiB): focused results, compiled input archives/receipts, full 40 clips/manifests/review sheets, two actual viewer clips/actions and retained harness failure. Both CPU-first HTML galleries are portable |
| Next discriminating work | Audit directly applicable existing warm-start/speculative/force/replacement/joint-query evidence against current sources; freeze a distinct finite budget only for uncovered required behavior. Actual viewer-control qualification must validate synthetic stimulus delivery before testing controls; do not rerun or extend the stopped tab-only campaign |
| New frozen diagnostics budget | [Protocol](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-diagnostics-2026-10-01/protocol.json) exhausted: 2/2 baseline GPU, 8/8 candidate C, 4/4 Rust, 16/16 preserved regression processes, storage suite once (16 controls). One candidate; 0 timing runs. All GPU processes pass their stated contract; baseline reproductions do not receive acceptance credit. Do not reuse or extend this campaign |
| Scope / known remaining failures | Rain physical acceptance and final whole-matrix/API/runtime/build/performance gates remain OPEN. Closed mixed scheduling budget55runs remains closed;no timing/Rain campaign resumed |
| Last verification | 2026-10-01: HEAD `d4e91bca235ce3565b66d5ee031565c857806ede`; milestone push succeeded, remote synchronization verified below. Compiled diagnostic engine inputs still match current sources exactly. Validator passes all 269 files, receipts/budget and 40 clip hashes/formats. Actual GPU Profile observed; Counters/Frame Time screenshots remain Profile and receive no pass. Protected Box3D/WASM paths unchanged; no timing/performance claim |

PR02 partial evidence is under
`experiments/gpu-physics/benchmarks/production-readiness/pr02-api-2026-10-01/`.
Both baseline host processes reproduce nonnull recording creation without an
error. The eight candidate-process budget is closed: seven complete passes and
one retained harness failure. That failure occurs after all immediate API error
assertions, on an incorrect errno-after-libc assumption; its old fixture is
preserved. A combined link failure exposes a real generator-selection defect,
fixed before spending any combined trials. A separately frozen one-process
closed-stderr case passes after publishing ENOTSUP again after failed output.
No failed trial is replaced and no campaign budget is extended.

Thirty-nine excluded recording/player/CPU-worker/static-tree operations now
report ENOTSUP and a sticky thread-local operation name; recording creation
returns NULL. Combined generation retains these errors rather than successful
CPU-only passthroughs. The 81 portable raw files and six exact compiled-source
sets include all failures and build/source receipts. Partial API milestone `806e634c9b0f5fe58fdd8a9a0dac171e862890d4` is committed and pushed; remote0/0 verified. PR02 remains open.

The diagnostic implementation and its focused campaign now pass; PR02 as a
whole remains open. Read the linked report and `progress.json` for the exhausted
budget. Public contact counters synchronize the native registry; routine sidebar
and benchmark accounting retains cheap public topology and labelled scheduling
metrics. Profiles publish seven measured aggregate aliases with NaN/mask/step for
availability; tests validate the returned mask, not complete CPU-profiler parity.
CPU-specific/color/recycling fields remain explicitly unavailable. Occupancy peaks
and 64-bit primary-buffer reservations have separate documented meanings.

The new diagnostic device word 74 is independent of ray words 0–31, retirement
commands 32–34, sticky status 66, phase/hint words 67–72, allocation high-water 73,
and component/memo workspaces at 256+. It is never read by scheduling/solving,
never cleared per step, and is copied across query/simulator growth. Host/device
peaks are captured in v24; old readers remain supported. Two fresh peak fixtures
prove unread-step/retirement/growth preservation. Busy reads fail without recursion,
and capacity-loss queries/clear never heal a failed world.

Frozen current diagnostic libraries are under
`experiments/gpu-physics/artifacts/production-readiness/pr02-diagnostics/candidate-complete-inputs/`:
ordinary `edbdb251e06e9c9bc5955045191e4ca2932bfacae6265581a0acfe3be16a6050`,
native `a736a81fae6b45751fadd0f57dcf795a0e44f7c21d1e54996c668b754cc393e9`.
Receipts include actual Rust/WGSL/Box3D C/header inputs before/after compilation,
not only invocation HEAD. A preliminary incomplete input receipt remains local;
complete receipts supersede it before GPU trials. Pre-launch cwd/syntax failures
are recorded; no GPU trial is discarded, replaced or extended. Native runtime
uses `scripts/native-samples-cache-env.sh` plus explicit NVIDIA/Vulkan and
CPU-compatible live ordering. Exact settings/selectors are in the protocol.

CMake's viewer include path and shared-generator dependency were completed after
the C campaign. Four viewer builds verify byte-identical generated adapter sources
from the campaign, plus separate before/after UI sources, compile commands and
binary hashes. Their CMake directories are under `.../pr02-diagnostics/c/<backend>-<gpu|both>/cmake`.
Four viewer builds do not claim actual tab interaction coverage. New required snapshots are now complete; diagnostic source is committed/pushed as `d4e91bca235ce3565b66d5ee031565c857806ede`. Use
`SKIP_METRICS=1` for GPU recording to avoid starting an unbudgeted timing campaign.
Preserve the old CPU clips before refreshing the real CPU-first comparison column.
Freeze recording-app/oracle build inputs and hashes separately; prior PR01 app
binaries do not represent the changed diagnostic shader.

Next, close the recording/viewer portion of this milestone, then the remaining
supported force/replacement/joint/control behavior. Source/link inventories alone
cannot close them. Reuse directly applicable existing evidence only after a
source/contract audit; freeze a distinct finite budget for uncovered requirements,
not a selective rerun of this exhausted diagnostic campaign. Sensor/compound/mesh
public diagnostic populations and special profile/capacity freshness cases are
not broadly qualified by the simple box fixture. No Rain or timing campaign has
been resumed. All PR03–PR10 release gates remain open.

PR01 discovery: pinned CPU `b3World_EnableSpeculative` only stores/serializes a
flag; collision code never reads it. Its shape switch covers hull/triangle only,
with sphere/capsule and convex speculation retained. After allowing time for an
optional preference question, use this existing hull/mesh scope. The CPU shape
flags are an independent reference for isolated controls; do not claim CPU world
toggle parity or promote the unsafe CPU shape-off CCD result to a pass.

The first zero-shell implementation reproduced CPU shape-off CCD tunnelling.
The second guard preserved floor safety and repeated exactly, but its recorded
5mm fast tangent gap has contacts=1 despite no CCD impact. This conflicts with
the documented shape-off positive-gap behavior and can revive ghost witnesses;
the native s&box Ghost Collisions scene specifically disables hull speculation.
Reject that broader guard instead of closing PR01 on narrower passing checks.

The third distinct hypothesis uses existing captured body flags to distinguish
an actual solid CCD handoff (FAST set, CCD_NO_HIT clear) from mere fast motion.
Only the actual handoff retains `LINEAR_SLOP + 0.25*LINEAR_SLOP`. All original
limits remain unchanged, and independent CPU fast/tangent gap controls pass
with no contacts. Budgets/stop rules are separately frozen; prior campaigns and
unfavorable trials remain closed/retained. No timing campaign is running.

Core-state capture advances to v23 for the future-relevant world policy and
pending refresh flag. Readers retain old schemas and require valid Boolean
policy fields in v23. Native full replay already keys every SimParams byte;
meshes are ineligible for full replay. Both five-process control captures and
all63 selected checks/backend have also been freshly requalified on the
handoff candidate; prior rejected results were not carried forward.

Required20standard scenes are now recorded through the scripts with real CPU
first, latest GPU next; all40clips have300frames at1280×720/30fps. Both regions
are visible in mixed-topology's shared wide camera. Old CPU clips remain in a
local archive and older datasets are untouched. Portable clips, manifests,
compiled-input receipts and four first/last review sheets are in the PR01 report.
The native s&box pair is also recorded/reviewed. Initial/end geometry and motion
are plausible; visual comparisons do not close analytical physics gates. The extra floor-screen failure is retained. Independent source/float32 analysis
explains this glancing/discrete landing; it does not raise a limit or claim full
scene physics qualification. The0.495m isolated normal-impact CCD bound and all
existing checks remain unchanged/passing. The180s baseline diagnostic cannot
distinguish slow first-time collision compilation from a hang; it is incomplete,
not a baseline pass. PR01 functional gates close after this applicability review;
final physical/runtime gates remain open. No live jobs.

Evidence and recovery sources:

- [PR01 report and retained raw data](../../experiments/gpu-physics/benchmarks/production-readiness/pr01-speculative-2026-10-01/README.md): control/repeat/regression passes, rejected variants, native floor failure and incomplete attribution. Source/float32 review explains the initial discrete landing; failed screen and incomplete startup diagnostic remain visible. Final physical/runtime qualification remains open.

- [`gpu-warm-start.md`](gpu-warm-start.md): completed control implementation;
  old narrower exclusions apply to that completed task.
- [`../gpu-mixed-scheduling-goal.md`](../gpu-mixed-scheduling-goal.md): closed
  experiment, target unmet, rejection/restoration and exhausted budget.
- [`../gpu-scheduling-goal.md`](../gpu-scheduling-goal.md): completed opt-in
  desktop policy; broader/laptop qualification is not implied.
- [`../../experiments/gpu-physics/benchmarks/mixed-scheduling-2026-09-30/README.md`](../../experiments/gpu-physics/benchmarks/mixed-scheduling-2026-09-30/README.md):
  portable mixed evidence and rejection details.

When activated, use `experiments/gpu-physics/artifacts/production-readiness/`
for local captures and `experiments/gpu-physics/benchmarks/production-readiness/`
for portable reports/receipts. PR01 local baseline/candidate build logs, frozen libraries and source receipts
are under artifacts/production-readiness/pr01-speculative; portable evidence and current recordings are in the linked PR01 report.
Its source-applicability review closes the original functional contract while
retaining the extra failed screen and incomplete startup diagnostic. Record each actual path, command, exit, source revision, build/input hashes,
adapter/driver, budget consumption, result and next discriminating action here
or in its linked item report. Mark PR10 complete only after a final evidence audit.

PR02 recording builds are complete: standalone ordinary recorder SHA-256
`9311af7b0a6f47c0ba2c8b011c028904460baefdae99d3f99ef0eb8f52f06368`;
real Box3D oracle `848477c467c448fcbbd938127f411ba32ad0b87ab7343579c65efff9a3238250`.
Before/after compiled input archives and receipts are under
`artifacts/production-readiness/pr02-diagnostics/recording-build/`.
Both build runners are terminal (8280/34938 exit 0); process inspection finds
no active physics/build jobs. A separate recording protocol is frozen at
`.../pr02-diagnostics/recording-protocol.json`: 20 CPU then 20 GPU clips,
300 frames, shared mixed camera, no GPU metric runs. Older CPU clips are
archived before refresh; recordings have not yet received acceptance credit.

PR02 diagnostics recording milestone is now complete: all40 new standard clips
validate and first/last sheets are reviewed. See the portable report's CPU-first
gallery. The separate five-app actual-UI protocol stops after two apps because
immediate synthetic mouse press/release does not switch tabs. Real CPU and
ordinary GPU launch/quit cleanly; GPU Profile rows/step IDs are observed. Captures
named Counters/Frame Time still show Profile: they receive no interaction pass.
All full clips/actions/logs are retained; three other configurations are unlaunched,
with no rerun or budget extension. Future uncovered-control work must validate
stimulus delivery first. PR02 stays open. Native diagnostic host occupancy scans
run once per positive step and GPU peak atomics add unmeasured work; PR08 must
assess performance rather than assuming no regression. No GPU/build jobs remain.

Partial diagnostic milestone `d4e91bca235ce3565b66d5ee031565c857806ede` is committed and pushed to `feat/gpu`.
The source matches the frozen diagnostic/recording inputs; the working tree was
clean after that source commit. PR02 remains unchecked. All stopped/exhausted
budgets remain closed. Next: supported-API evidence applicability audit and a
distinct bounded campaign only for uncovered required contracts. Exact existing
selectors include `accumulated_forces_survive_uploads_and_zero_steps`,
`replacement_reuses_storage_and_preserves_body_mass_and_metadata`, and
`reaction_impulses_use_solver_accumulators_for_every_joint_kind`; read their
assertions before deciding what is still uncovered. No new campaign launched.
