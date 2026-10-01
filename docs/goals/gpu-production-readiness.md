# GPU native production readiness

Read `/home/firtoz/work/2026/box3d-wasm/docs/goals/gpu-production-readiness.md`
at the start of every continuation and after compaction or restart. Pursue the
whole objective, maintain this file, and use the goal-scratchpad skill. This is
the authoritative production-readiness queue for `feat/gpu`.

Updated: 2026-10-01. Status: active; full roadmap and verified milestone commit/push authorized by the submitted goal.
Next item: **PR01 — working world-level speculative-contact control**.

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

- [ ] **PR01 — Implement speculative-contact world controls.** Replace the
  no-op `b3World_EnableSpeculative` with real per-world behavior and correct
  combined-viewer routing. Preserve the default and upstream semantics, including
  interaction with per-shape flags, creation and runtime off/on transitions,
  independent worlds and destruction/recreation. Verify convex/mesh cases and
  interaction with CCD on both backends using independent CPU/GPU fixtures.
  Relevant default-behavior, lifecycle and repeatability checks must pass.
  Update API audit/docs and record affected scenes before a solver commit.
  **Evidence:** OPEN.

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
  **Evidence:** OPEN.

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
| Workspace / branch | `/home/firtoz/work/2026/box3d-wasm`, `feat/gpu` |
| Starting revision | `d9d77230abb59414ae781281261b0e2055797008` |
| Goal status / authorization | Active full-roadmap goal; user explicitly authorizes implementation and commit/push of verified milestones |
| Current item / next action | PR01: upstream world flag has no collision consumer; clarify world-gate scope, freeze focused fixtures, implement and verify both backends |
| New candidate / build identity | None; no production-ready candidate claimed |
| Jobs / completed runs / remaining budget | No jobs launched for this roadmap; no campaign budget yet frozen |
| Known blockers | World speculative control is a no-op; Rain physical acceptance and final-build whole-matrix/repeat/performance gates remain open |
| Completed prerequisites | Warm-start controls and bounded scheduling work have existing evidence; audit applicability before relying on them |
| Rejected work to preserve | Mixed split scheduler failed; production changes restored. All 55 trials retained; original budget exhausted |
| Commit/push / worktree | Saving the roadmap activation milestone; branch and fetched origin start synchronized; Box3D unchanged |
| Last verification | 2026-10-01: source references for enableSpeculative audited; no GPU jobs running; implementation not yet edited |

PR01 discovery: the pinned CPU world toggle only assigns `world->enableSpeculative`
and serializes it; no collision code reads it. The per-shape switch explicitly
covers hull/triangle contacts only, while sphere/capsule and convex speculation
remain enabled. A preference question is pending about gating this existing
supported feature versus a broader all-pairs GPU contract. Proposed scope is the
existing hull/mesh gate, with the upstream shape switches as the independent CPU
reference. Do not claim CPU world-toggle parity for behavior it does not implement.

Evidence and recovery sources:

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
for portable reports/receipts. They have not been created by this documentation
task. Record each actual path, command, exit, source revision, build/input hashes,
adapter/driver, budget consumption, result and next discriminating action here
or in its linked item report. Mark PR10 complete only after a final evidence audit.
