PR05 and PR06 completed locally, 2026-10-17: **PR05 8/8 gates closed; PR06 5/5, 0 left.** [Verified runtime repairs, clean builds and consumer report](../../experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17/README.md), [acceptance](../../experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17/acceptance.json), [current recovery log](../../experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17/log.md). Read the log FIRST after continuation/compaction. Stop after PR06. User authorized the verified milestone commit on 2026-10-17; push was not requested. PR04 physical failures remain open and are next only on a new user request. Later relevant changes require release applicability review/refreeze. This supersedes older activation/stop prose below.

Queue update, 2026-10-17: the user moved PR04 after PR06. PR05/PR06 are now complete; remaining order: **PR04 → PR07 → PR08 → PR12 → PR13**. All open physics criteria remain unchanged. Do not start PR04 automatically. If its repairs change release inputs, refreeze affected PR06 evidence before PR07.

PR02 completed and committed as `7792af8f7`, 2026-10-17: **5 checks done, 0 left.** PR11 is committed as `17e3575a7`; the user authorized the PR02 follow-up commit. Push was not requested. [Current PR02 report](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-contract-refresh-2026-10-17/README.md), [acceptance](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-contract-refresh-2026-10-17/acceptance.json) and [recovery log](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-contract-refresh-2026-10-17/log.md). Read that log FIRST on continuation. Current415stateful/376candidate/39excluded+17default-alias inventories pass on all4routes;59headless processes cover original API contracts, exclusions, independent CPU/defaults and supported native launcher policy. Original harness errors and configuration limits remain explicit. No production/Box3D/WASM edits, no live jobs or raw run directory. Stop after PR02; PR04 physics and later release gates remain open. This supersedes historical next-action prose below.

# GPU native production readiness

PR11 verified locally, 2026-10-17: **7 acceptance gates done,0left**. Initial Overlap/Overlap World misses, mesh witness normals/local-frame precision, standalone child metadata and filter defaults are repaired; Body Cast's white miss preview is resolved without a palette change. [Report and numeric/visual evidence](../../experiments/gpu-physics/benchmarks/production-readiness/pr11-queries-2026-10-17/README.md), [final audit](../../experiments/gpu-physics/benchmarks/production-readiness/pr11-queries-2026-10-17/final-acceptance.json), [recovery log](../../experiments/gpu-physics/benchmarks/production-readiness/pr11-queries-2026-10-17/log.md). Read that log FIRST on recovery.95residual Height Field cases are explicitly qualified conventions:93adjacent-facet choices and2cyclic-vertex-anchor rounding cases, each checked independently against CPU at1e-5. No whole-grid witness/pixel/trajectory identity claimed. Task-owned336,398,948bytes removed;~5.1MiB retained; no live jobs. User authorized the PR11 commit and PR02 continuation on 2026-10-17; push was not requested. PR02 is next under a distinct finite audit budget; other roadmap items remain open. This supersedes historical PR10 stop/next-action wording below.

PR09 publication, 2026-10-15: **165 native samples reviewed, 0 left; 21 fixes, 1 resolved without repair, 0 open original repair groups.** [Portable inventory, reports and before/after evidence](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/README.md) close the PR09 comparison gate. Rain physical failures remain open under PR04; UI/widgets and full query qualification remain PR10/PR11. Earlier dated counts, local audit archives and source-specific limitations below are historical and do not certify the full release.

Read `/home/firtoz/work/2026/box3d-wasm/docs/goals/gpu-production-readiness.md`
at the start of every continuation and after compaction or restart. Pursue the
whole objective, maintain this file, and use the goal-scratchpad skill. This is
the authoritative production-readiness queue for `feat/gpu`.

Updated: 2026-10-17. The 22-group repair goal is COMPLETE: 21 fixed, 1 resolved without repair, 0 open; 165 native samples reviewed, 0 left. The user authorized PR09 acceptance closeout and commit/push to `feat/gpu`. This publication closes PR09 with explicit unavailable/unreviewed and nonvisual dispositions; later release gates remain open. Read the portable native review first on a fresh checkout; this machine also retains the original full-review log and local audit archives. Use the goal-scratchpad skill on recovery.

PR10 completed locally, 2026-10-16: **165 native sample dispositions, 342 active widgets, 85 shared calls and 23 custom input groups reviewed; 29 repairs verified, 0 confirmed PR10 repair groups open.** The 165 comprise 71 actual specific-control fixtures and 94 explicit shared-code reuses, not 165 new runtime passes. Five inactive Ragdoll/Pose widgets are excluded explicitly.

[Final acceptance and before/after report index](../../experiments/gpu-physics/benchmarks/production-readiness/pr10-ui-2026-10-15/README.md), [criterion audit](../../experiments/gpu-physics/benchmarks/production-readiness/pr10-ui-2026-10-15/final-acceptance.json) and [simple recovery log](../../experiments/gpu-physics/benchmarks/production-readiness/pr10-ui-2026-10-15/log.md). Read that log first when recovering this completed task. Individual reports retain their original failures, exclusions, unsupported capabilities and finite test windows.

All five delivery-source native CMake builds/startup checks, full typechecking, demo build and unchanged-source WASM runtime checks passed. The root now declares its existing Bun type catalog required by scripts. Box3D, WASM bridge/API, Rust and WGSL sources stayed unchanged. Temporary SDK/builds and consumed diagnostic viewers/objects/carriers were removed; shared caches/archives and user work were preserved. There is no live job or runnable PR10 diagnostic pool.

OBS01 retains unconfirmed Body Cast query appearance under PR11; OBS02 native GTK acceptance reuses retained actual evidence while later helper timeouts remain archived; OBS03 inherits the native keyboard-navigation policy, with the original event untraced. These dispositions do not certify query appearance or GPU diagnostic/player parity.

The activated goal stops after PR10. **PR10 commit and push authorized.** User direction, 2026-10-17: recordings and comparison grids are optional diagnostic tools, not commit/push prerequisites. Do not launch full-catalog capture campaigns just to publish verified work. Reuse applicable checks and evidence; targeted captures remain available when the task needs them. This supersedes older mandatory capture wording throughout the historical recovery notes. The unnecessary publication recordings and scripts were removed; original repair reports and before/after screenshots are preserved. PR09 is already pushed as 51ff8b06b4964d359386f08605d835ac6e2ea23d.

On a new user request, PR11 is next. Rain physical failures remain open under PR04; exact pixels and chaotic CPU/GPU trajectories remain deferred. Do not resume historical exhausted experiments. PR10 completion does not certify the full release.
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
   Retain applicable verification evidence. Additional recordings/comparison grids are optional and do not block commits.

### Storage budget and cleanup

User requirement, 2026-10-14: produce only files needed for the current work and
clean temporary outputs when their use is finished. This supersedes older
instructions to retain every raw dump, binary or cache indefinitely. Keep the
acceptance evidence and unresolved failure inputs needed to explain or reproduce
a result; retain failed hypotheses as concise reports rather than whole builds.

- Before execution, list each output, its consumer, expected maximum size and
  deletion point in the finite run protocol. Default limits are **1 GiB of new
  temporary data** and **64 MiB of retained diagnostic evidence per experiment**.
  Include task-owned cache copies, backup libraries, shared-build growth and
  shared GPU driver-cache growth in the temporary estimate. Record relevant
  shared driver-cache sizes before/after GPU batches; preserve caches used by
  other applications and do not attribute unmeasured growth to this task. Explicitly budget any necessary larger capture or
  repair media before it starts; do not silently accumulate another directory.
- Check free space before launch and after each bounded command. Maintain at
  least 20 GiB free and enough for twice the remaining planned output. Bound each
  producer's output and runtime; arrange termination at the declared size limit.
  If growth exceeds the budget, preserve the small failure receipt, restore
  sources/libraries, clean temporary outputs and reduce the capture scope before
  another run. A timeout must also run cleanup.
- Prefer selected bodies, joints, operations and frame windows. Stream
  validation/digests when full records need only be checked once; extract the
  minimal reproduction input and summary before deleting a full health dump.
  Reuse one current shared build/cache per backend. Avoid per-scene copies of
  executables, static libraries, restored-library backups and pipeline caches.
- Put generated scratch data outside the worktree or in a locally ignored
  directory before writing it. The two existing PR09 run directories are locally
  excluded in `.git/info/exclude`. Explicitly stage only selected reports,
  regression inputs and visual evidence for any future authorized commit;
  never stage a raw run tree. Keep existing file links valid for retained media.
- After verification and source/library restoration, clean task-owned temporary
  binaries, objects, backup archives, cache copies and consumed full dumps on
  both success and failure. Never remove live-job inputs, user work, the current
  shared candidate or required before/after evidence. Retain each remaining large
  input only with a named next consumer; remove it when that consumer is done.
  Record retained/deleted bytes and available space in the recovery notes. A
  batch is not closed until cleanup is complete or a specific remaining consumer
  is recorded. Historical audits describe evidence at their execution time;
  cleanup receipts must identify raw files that are no longer available.

## Ordered checklist and acceptance contract

Work on one unfinished item at a time. Its evidence cell is initially `OPEN`.
Add links to committed receipts/reports and the applicable source/build identity
when closing it. Keep detailed physical results in
[`../gpu-solver-qualification.md`](../gpu-solver-qualification.md), API/support
status in [`../gpu-physics.md`](../gpu-physics.md), and this file as the concise
queue and recovery record. No separate competing production backlog.

Priority updated by the user on 2026-10-02: address visible functionality gaps
before continuing the deeper physical-correctness investigation. Execution order
is **PR09 → PR10 → PR11 → PR05 → PR06 → PR04 → PR07 → PR08 → PR12 → PR13**;
PR01/PR02/PR03 are complete at their named functional/preparation scope. The user prioritized PR02 closeout after PR11 on2026-10-17; its filter-default reopening is now resolved by the current receipt below. Stable item IDs are preserved. The user
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
  Update API audit/docs and retain applicable verification evidence.
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

  **Reopened2026-10-03:** VIS06 actual filter-default trace (local audit archive) exposes standaloneC `b3DefaultFilter.categoryBits=1` versus actualCPU/header/combinedUINT64_MAX. Existingfocused/APIinventory results remainpreserved but didnotqualify thisdefault alias. Correct CPU-compatible defaults and mask-specific behavior needtheir ownfinite acceptance; notfixed byvisibilityrepair. This doesnotactivatefullreleasework orchange the original22group counts.

  **PR11 update2026-10-17:** that specific default alias is now repaired and independently checked against CPU with masks1,2 andbit63 on both backends. [Query qualification](../../experiments/gpu-physics/benchmarks/production-readiness/pr11-queries-2026-10-17/README.md). At the PR11 checkpoint PR02 remained unchecked pending its complete criterion audit. The subsequent PR02 resolution below closes that audit separately.

  **Resolved2026-10-17:** PR11commit17e3575a7 corrects standalonecategoryBits toUINT64_MAX. [Fresh contract acceptance](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-contract-refresh-2026-10-17/acceptance.json) and [current inventory/default evidence](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-contract-refresh-2026-10-17/README.md) close the reopening and named API gate:32unchanged API fixtures,4explicit-error fixtures,5CPU/GPU default fixtures and18distinct native-launcher-policy cases pass. Both library identities exactly match qualifiedPR11. Seventeen no-ID default aliases are now inventoried separately; independent default behavior is checked for filter/shape-filter/query-filter only. Source/link coverage is not376independentphysical passes. Original compile/parser failures and default-runtime applicability correction are retained; no prior failed evidence or tolerance replaced.

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

- [x] **PR09 — Compare all existing CPU samples and fix obvious GPU differences.**
  Next after PR01–PR03, complete the exhaustive inventory and bounded comparison
  sweep described above, including samples outside the short fixture list.
  Reproduce invisible floors, identify every affected sample and fix the
  demonstrated rendering cause. Check static, compound and mesh ground
  representations where used, camera framing and
  occlusion against the real Box3D CPU view. Preserve scene geometry and physics
  defaults; a rendering repair must not change collision/support behavior.
  Review affected scenes and retain applicable before/after evidence. Additional
  recordings and comparison grids are optional rather than a commit prerequisite. The new [combined Village capture](../../experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-ownership-captures-2026-10-01/README.md) reproduces missing nearby GPU compound ground/buildings while CPU geometry is visible. Unchanged shared65536-slot renderer pool cannot flatten both52500-child compounds; source audit and exact clip are retained. Village is one known case, not the inventory boundary. Fix/qualify every demonstrated floor omission and obvious visual/behavioral defect; triage severe performance differences. Route controls to PR10, queries to PR11 and concrete solver defects to calculation-first PR04 work without losing their sample inventory entries. PR09 closes only after every inventory entry is reviewed or has an explicit unavailable-gap disposition, required floor/render fixes pass, and other findings are linked to named open gates. Routing a solver/control/query/performance finding preserves it as an overall release blocker; it must not force completion of later gates before the sweep can advance. No finding is silently marked passed.
  **Evidence:** [portable acceptance matrix](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/acceptance.json), [full inventory and original per-sample log](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/README.md), and 22 repair reports with original before/after media. All 165 native entries have individual bounded review dispositions; 174 browser entries include 13 explicit native-counterpart gaps and unreviewed browser-runtime status; 20 CPU oracle fixtures are nonvisual and remain numerical PR03/PR04 inputs. Seven shared heightfield scenes (008/039/041/065/091/096/121), compound ground, visibility, drawing and material defects are verified. Final source-specific applicability and unavailable/runtime limits remain explicit. The 21 fixes and one resolution are closed; remaining physical/control/query/lifecycle/performance findings are routed to the named open gates. Existing CPU-first affected recordings were completed before this commit; publication adds no build or capture. Full local audit archives remain excluded; selected media and required regression inputs are deliberately published within the frozen storage budget.

- [x] **PR10 — Qualify sample UI and widgets end to end.** After PR09, inventory
  the controls and widgets for the full PR09 inventory, including samples outside
  the short list, then
  exercise actual mouse/keyboard interactions on ordinary/native GPU and
  combined viewers with a real CPU comparison. Cover sample selection, sliders,
  buttons, toggles, dragging/picking, overlays, pause/single-step/restart and
  resize/layout where available. Verify displayed state and the intended
  physics effect, including scene changes and destruction/recreation; a widget
  that accepts input but leaves stale state does not pass. Retain interaction
  evidence. Extra recordings/comparison grids are not a commit prerequisite. PR02's required
  truthful-control checks still belong in PR02; this is the sample-wide
  usability check requested by the user, now prioritized before PR04.
  **Evidence:** [Verified local acceptance and 29 before/after repair reports](../../experiments/gpu-physics/benchmarks/production-readiness/pr10-ui-2026-10-15/README.md), including complete control/applicability inventory, five fresh native builds, frontend/WASM compatibility and cleanup. Commit/push authorized; the user removed the additional recording/comparison prerequisite on2026-10-17. Existing verified checks and repair evidence support publication.

PR10 watch disposition: [OBS01's original unconfirmed observation](../../experiments/gpu-physics/benchmarks/production-readiness/pr10-ui-2026-10-15/final-observations.json) is resolved by PR11 actual query/draw provenance: the pale shape is the native white miss sphere; frozen CPU/GPU queries agree and actualRGBA is[1,1,1,1]. No palette repair. OBS02 native chooser acceptance reuses retained actual GTK/load evidence; later helper timeouts remain archived. OBS03 inherits native disabled keyboard-navigation policy; no GPU defect or Escape-input pass inferred.

- [x] **PR11 — Qualify raycast behavior and appearance.** After PR10, reproduce
  the user's reported CPU/GPU difference in the supported raycast samples and
  picking paths. Compare hit/miss, closest-hit selection, public shape/body
  identity, fraction, point, normal, filtering and transformed geometry with
  independent fixtures and the real CPU reference at existing tolerances.
  Check the displayed ray, hit marker and normal against returned query values
  and camera coordinates, including misses and scene changes. Fix demonstrated
  discrepancies and document intentional conventions. Retain applicable numeric and visual evidence; additional recordings/comparison
  grids are optional and do not block committing verified fixes. The user
  observation does not yet establish a query or rendering defect.
  **Evidence:** [Verified repairs, original failures and targeted CPU-first visual evidence](../../experiments/gpu-physics/benchmarks/production-readiness/pr11-queries-2026-10-17/README.md).601independent records/backend,216additional encroachment/transform cases/backend,14,278frozen Height Field casts/13,340hits,95CPU-selected-facet primitive checks,4nested callback response modes and25Rust query regressions/backend pass at original1e-5. Actual query/draw/camera checks14/combinedroute pass. Cyclic vertex order and near-tied facet selection are explicit conventions, not whole-grid point/normal equivalence. Box3D/WASM unchanged; local verified milestone, not yet committed.

- [x] **PR05 — Qualify runtime safety and lifecycle behavior.** Review and test
  buffer bounds/dispatch coverage, reservation growth/overflow, C/Rust layouts,
  ID generation/reuse, shape/body/joint replacement, callbacks/reentrancy,
  query/event ordering, asynchronous status, replay invalidation and ownership.
  Sticky loss/invalid state must remain observable; clearing diagnostics must
  not make a failed world usable. Exercise repeated create/destroy/scene changes,
  sleep/wake, pause/single-step, shutdown during loading and repeated startup.
  Verify supported launch paths with caching enabled and disabled. Reuse existing
  focused checks; add tests only for a concrete failure or uncovered requirement.
  **Evidence:** [Runtime repair report and retained original failures](../../experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17/README.md), [13-gate acceptance](../../experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17/acceptance.json).98main Rust tests/34Cfixtures, supplemental dispatch/replay/cache controls,96public lifecyclecycles across cache modes,9viewer pause/single-step/scene-switch cells and4actual worker-loading shutdowns pass. Callback owner deadlock, unobservable terminal device failure and diagnostic16-byte replay-key schema are repaired. Owner-thread C/callback whitelist and controlled device-loss limits remain explicit; original harness/receipt failures are reconciled without concealing them.

- [x] **PR06 — Freeze reproducible release builds and consumer integration.**
  Clean-build CPU, GPU and combined native targets for both backends, verify
  correct adapter selection and actual cache/replay behavior, and freeze the
  release candidate's compiled inputs and executables. A minimal external native
  consumer must create, step, read and destroy a world using the supported API.
  Check the original TypeScript/demo workflow remains usable without modifying
  Box3D/WASM. Record exact commands, dependency/toolchain revisions and build logs;
  invocation checkout metadata alone cannot identify compiled source. Any later
  relevant code change invalidates affected release evidence and requires an
  applicability review/refreeze before final qualification.
  **Evidence:** [Clean release/consumer receipts](../../experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17/README.md), [20artifact final verification](../../experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17/final-artifact-verification.json), [input/toolchain snapshot](../../experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17/release-inputs.json).Twoempty Rust targets/fiveindependentempty native builds,actualNVIDIA Vulkan/cache controls,5CPUfirst external consumers atoriginal1e-5,maxerror0,workspaceTS/demo andunchanged-sourceWASM runtimepass. Exactcompiledinputs/generatedsource/commands/dependencies/binaries arefrozen;consumedbuilds/SDK/rawremoved,350MiBnamedcandidate keptforPR07/refreeze. Relevant laterPR04changes invalidate affected evidence;fullphysics/repeats/performance remainopen.

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

2026-10-15 scope decision: the user closed PERF04timeout under option1 while retaining all existing Rain physical failures here, at original tolerances. [Timeout closure](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/PERF04.md). Exact before/after state preservation establishes no new physical pass. PR04 remains unchecked; no historical solver campaign is resumed by this milestone.

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
  Use targeted recordings/comparisons when useful for the release evidence;
  they are not a prerequisite for committing verified changes. Publish readable charts, portable raw timing/check data,
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

## Sample repair goal

Read /home/firtoz/work/2026/box3d-wasm/docs/goals/gpu-production-readiness.md
after first reading the full-review log and AGENTS.md on every continuation,
compaction or restart. Use goal-scratchpad and pursue this whole22-item repair
objective, one item at a time. The app goal was created2026-10-02. User selected option1 on2026-10-15: close the verified timeout milestone, retain pre-existing physics failures underPR04; the app goal is COMPLETE (update_goal confirmed2026-10-15); recovery is in PERF04 CURRENT.

**Objective:** repair the22 logged issue groups from the completed native C sample
review while preserving real Box3D CPU comparisons, ordinary/native GPU,
combined routing, geometry, defaults, body/shape identity and picking. For each
item trace identical boundary inputs/intermediates to its smallest demonstrated
cause, verify the repair, share a separate report with displayed before/after
screenshots and mark the checklist [x] only with applicable evidence.

**Counts:**21 fixed,1 resolved without code change,0 open. Review coverage remains
165 done/0 left. These are issue groups, not samples or established independent
causes. Several findings need diagnosis. Retain original IDs; if one cause fixes
several groups, verify/report each explicitly. Never silently delete unfinished
criteria. Update fixed/resolved/open counts in every progress update and report.

**Acceptance and reports:** before each expensive build/check/capture, freeze
that item's finite candidate/build/process/scene/duration/watchdog budget and
invariant limits. Retain failed hypotheses and raw data; no implicit retries or
extension of closed campaigns. No blanket solver rollback or changes to physics
defaults/tolerances. Each report must name the trigger, smallest input, first
failing intermediate, exact repair/source/binary identity, affected-scene matrix,
checks, limitations and open findings. Display actual before/after screenshots
(CPU/ordinary/native comparisons and combined routes where affected), label
scene/mode/settings/stage and link CPU-first clips. Approximate screenshot
timing must be labeled; screenshots alone cannot close physics/query/runtime
criteria. Additional recordings/comparison grids are optional and do not block commits.
Reuse verified baseline media only with established source/settings applicability.

[x] means the named acceptance criteria passed, not merely that a patch was
written or a timeout disappeared. Diagnosis-only, unsupported or failed cases
stay [ ]. If independent evidence proves an observation intentional/non-defective,
record it separately as resolved-without-repair with evidence/screenshots; do not
inflate the fixed count or call availability exclusions repairs. Exact pixels
and chaotic CPU/GPU scene trajectories remain deferred. Restore a checkbox to
[ ] if later changes invalidate the evidence.

### Repair checklist (authoritative22 groups)

Each OPEN evidence cell must become a report link on completion. Scene numbers
refer to the full-review log. Broad findings derive the affected matrix from
every tagged per-scene review, not just representative clips.

- [x] **VIS03 — Missing heightfield surfaces.** Restore unchanged visible
  terrain in008/039/041/065/091/096/121 on ordinary/native and combined routes.
  Verify geometry/transform/shape mapping and body/picking identity; use box/mesh
  floor guards. Do not claim collision repair. Evidence: [verified heightfield repair/screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS03.md).12 focused metadata cases, four C-only viewers,60 default captures/12 reviews and41 completed camera/control captures verify allseven confirmed andthree related cases on ordinary/native/combined paths. One earlyCPU observer failure, limitedF view and initial review correction remain retained. No physics/query/picking/performance claim.
- [x] **VIS01 — Combined CPU ground grid/axes missing.** Restore expected
  decoration without duplicate floor geometry or world routing changes.
  Reconcile all tagged scenes and audit shared-path applicability (old85 count corrected to80 positive/10 negative mentions;97 verification scenes including seven terrain guards). Evidence: [verified name/material repair and screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS01.md).12 before+12 after actual CPU draw/name cases;97 individual CPU-first reviews/194 after captures, source/build/media audit PASS.014/039 pixel limits and eight health-warning cells retained; no physics/runtime/picking pass.
- [x] **VIS05 — GPU ignores CPU visibility bounds.** Original findings were CPU distant-body omissions in Many Pyramids014, Mixed Stacks075 metadata discrepancy and Isotropic Friction139. Actual CPU trace proves configured culling, with no body loss. GPU debug drawing now queries each existing public shape AABB and applies inclusive overlap against incoming drawingBounds before allocation/submission; IDs, metadata and wide-bound reentry remain intact. Evidence: [verified bounds repair/screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS05.md).21before/21after actual C cases cover44736 frozen records and12 boundary cases; four C-only viewers and all165 individual CPU-first reviews/660after cells pass the rendering acceptance with explicit limits. Full604source/604object/162linked-input audit and20383-file portable verification PASS.652cells completed/8timeout/28flags retained;013 unavailable,060 physics4/capacity failure, query/physics/runtime/picking/performance remain unqualified. CPU fat-tree/tight-GPU boundary candidate differences are explicit; no exact pixel/count/trajectory claim.
- [x] **VIS06 — GPU shows hidden sensors.** Actual Sensor016 trace proves default camera/view/shadow bounds exclude every sensor byZ; GPU previously ignored those bounds. ExistingVIS05 repair closes this group with no additional production delta. Evidence: [verified sensor visibility/screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS06.md). RealCPU bounded0/wide1681 atsteps1/30; actualfourGPUroutes trace all1681 live static sensors/IDs/poses/AABBs atbothdraw stages, allfalse overlap.14before/14after actualCcases on3362CPU frozenrecords+12boundaries verify cache/identity/wide reentry. Fiveobserverunits/links/five30stepcaptures,723source/723object/718unchangedunit/199linkedinput audit and279-file portablePASS. RUNTIME02step32 remainsopen; queries/events/collisions/picking/performance/overlays unqualified. Separatepreexistingstandalonecategory1 vsCPUcombinedALL default mismatch recordedOPEN underPR02, not a23rdoriginalgroup or visibility cause underALLdrawmask.
- [x] **VIS07 — Missing direct primitives/Step overlays in GPU halves.** Accepted directStep submissions now survive pane reset through an owned frame-local journal; base world geometry remains independent, and late pane-local UI labels receive a right-pane copy. No second Step/query/spawn or fake bodies. Evidence: [verified repair/before-after screenshots and all55 CPU-first rows](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS07.md). Full165-source inventory yields28positive/8negative tagged mentions plus49 source-related candidates,55union cases individually inspected before/after ordinary/native combined.110captures complete; actual1/30 replay2692packed primitives and text input checks PASS; Tree4lateUI label pairs verified separately. Twelveviewerunits/twoarchives/twolinks and516-input ASAN/UBSAN lifecycle check PASS;725sources/725objects/713unchangedunits/198linkedinputs and4518-file portable check PASS. Conditional/panel/offcamera/sharedtool limitations retained;071flags, query/runtime/physics/picking/performance and VIS08 remainopen. Allbudgets CLOSED,zero engine retries.
- [x] **VIS08 — Shifted combined GPU text.** Shared Render cursor now starts at the same after-Step entry in both panes and restores the first Render end for late UI. All18 original tags,28 source cursor paths+5 negative guards individually inspected (33 done,0 left),66 completed ordinary/native combined captures and99 CPU/before cells. [Verified repair/report with actual CPU-first before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS08.md). Exact cursor/RenderXY and Step replay atdisplay1/30,Tree4lateUIpairs PASS; SAT Step line and intentional blank spacing preserved.2main compiles/2links/0archives,725source/object/723unchangedunit/198linked-input audit and2,815-file portable verification PASS. Shared tool/numeric provenance, panel/loading/blacktail limits and all separate colour/query/physics/runtime/picking/performance findings retained. All budgets CLOSED,zero engine retries; Box3D/WASM unchanged.
- [x] **VIS02 — Material/state colors differ.** Shared selector preserves packed custom material and actual body type/sleep/enabled/sensor/bullet/FAST; actual cap/acceptedCCD diagnostics use CPU precedence and retain idle lifetime, with unchanged clamp arithmetic and intentional combined ordinary-state blue. [Verified report with CPU-first before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS02.md), all165 comparison rows (local audit archive), acceptance (local audit archive). All96 original mentions included;165 done,0 left;660 after cells dispositioned,825 references,550884 selector rows/2360 custom passthroughs;725 source/object audit and29245-file portable verification PASS.74 actualCPU/2304 precedence cases and focused ordinary/native cap/CCD/lifetime/identity checks PASS.646 completed/12 timeout/2 capacity failures,34 runtime/health-limited cells,unexercised/occluded pixels and approximate CPU/GPU progress retained. Unavailable scenes disposed,not passed. PR04 FAST thresholds and all separate physics/query/runtime/picking/performance issues stay open; Box3D/WASM/defaults/geometry unchanged. All budgets CLOSED,no engine retries.
- [x] **QRY01 — Long Ray Cast discrepancy.** Shape-specific native-order query arithmetic fixes generic advancement stagnation and long-ray surface-point errors, including the demonstrated box reciprocal/slab grab-anchor defect. [Verified report with actual CPU-first before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/QRY01.md), 17 final affected scene rows (local audit archive), acceptance (local audit archive).1800callback+1800closest pairs/backend exact default outputs;768boundary pairs/backend hit/point/fraction exact with91 independently valid adjacent facets at unchanged1e-5 normal limit. Actual production pointer selection/anchors/release/worlddestroy and one-sided misses PASS bothbackends,0physicssteps.17 individual final confirmations/68 completed clear-health captures;132source/receiptpins,4viewers,1939files and153absentPIDs audited PASS2.193s. Original165survey done; earlier full165 gallery retained,148shared scenes explicitly unreviewed on final binary. Full draggingphysics/conditionalcontrols/runtime/performance/pixel/trajectory qualification remains open. All budgets CLOSED; original failed hypotheses and earlier timeouts preserved;Box3D/WASM/defaults/geometry/identities unchanged,no newcommit/push.
- [x] **DIAG01 — False health explosion classification.** Native scene contracts correct the absoluteY200/general120m/s heuristic for003/016/060/061/071/125, preserving raw flags and independent nonfinite/speed/height/capacity/progress failure detection. [Verified report with actual CPU-first before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/DIAG01.md), nine comparison rows (local audit archive), acceptance (local audit archive).2880 archived inputs+45 actualC++/Python boundary cases and24 actualnativeCPU/GPU body/productionrecorder cases PASS; malformed limits rejected and legacy records unchanged. All165 names checked,159 profiles unchanged/six source-driven contracts. Five hook-only compiles/links, nine individual reviews/45 completed display captures:41 clearhealth, four realStall step4 capacity/progress failures retained. Final audit8.880s PASS1207producerpins/202uniqueunchangedlinkedinputs/21compilerdependencies/5viewers/1240records/1328files/105absentoriginalPIDs. SensorGPU30 does not exercise32spawn/freeze; actualY211 zero-step fixture separately verifies classification.156 other finalDIAG01viewer scenes explicitly unreviewed. Broad outlier screens do not qualify fullenergy/support/CCD/contact/events/runtime/performance; RUNTIME02/03 remainOPEN. Original preparation failure retained; allbudgets CLOSED,zeroengine retries;Box3D/WASM/defaults/geometry/identities unchanged,no newcommit/push.
- [x] **BEH01 — Conveyor Belt transport stops.** Scene135; trace tangent-velocity
  contact calculation at identical inputs; verify belt transport and Conveyor
  Mesh136 guard on both backends. Evidence: [verified body-frame material repair/screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/BEH01.md). Native identical-input diagnosis isolates missing fresh convex contact tangent write; corrected body quaternions pass28 primitive/order/zero controls at1e-5. Bothbackends120step belt transport/identity, original240step friction/600step support-energy and12 cached/material/recreation contacts PASS. Two CPU-first rows/ten completed120step display cells reviewed; mesh retained. Audit PASS11.746s/1252pins/21deps/163linkedinputs/196media/73absentPIDs. Firstcapsule candidate8failures, setup/offline failures, threecold90scompile timeouts andresourceffprobe remainpreserved; separate0step native-constructor prep130/128s precedes fourwarmMeshconfirmations. Other163finalsolver scenes UNREVIEWED; fullstate/repeats/longcirculation/controls/coldstartup/FPS/release remainunqualified.
- [x] **BEH02 — Passive Static Invoke creates separating contacts.** Scene143;
  creationpolicy/publicproxy movement now preservesPassive while explicitinvoke,
  changedfilters andcompoundexceptions follow native behavior. [Verified repair and before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/BEH02.md), CPU-first clips (local audit archive).307APIsteps/15worlds perbackend,16raycontrols/12stale-IDinvalidations and4original240friction/600support regressions PASS. Threefinalscene reviews143/039/049 done0left;13fresh120stepcells+2unchangedCPUreferences clearhealth onfourGPUroutes.10testedfiles retained with startupguard; auditPASS4.890s/1011pins/163linkedinputs/144media/59absentPIDs; afterinterruption722pin checkPASS. CapsuleXresponse1e-5screen failure unchangedbeforecompletion retained; generalhullresponse/sensorpositive discovery/manualcontrols/picking/162otherfinalscenes/longwindow/repeatability/performance/release remainunqualified. No Box3D/WASM/default/geometry/ID changes;no newcommitpush.
- [x] **BEH03 — Far Mesh Drop falls through.** A fixed local physics frame repairs lost sub-ULP motion and rounded mesh contact anchors; public point/callback/GPU-picking boundaries preserve representable global geometry and identity. [Verified repair with before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/BEH03.md), all ten CPU-first comparison rows (local audit archive), acceptance audit (local audit archive). Both ordinary/native standalone/combined300step scenes have0interiorfalls/all400asleep/zero speeds. Six defaults+four actual native offset presets reviewed;90primary cells/74actualAPI/publication/lifecycle/CCD/friction/support-energy/numerical checks PASS. All original failures and stockCPU8falls/9awake retained; FarPyramid physical interpretation and FarRagdoll joint/longwindow qualification remainOPEN. Other159 final-candidate native scenes explicitlyUNREVIEWED; no arbitraryspanningworld/fullmanualdragging/determinism/FPS/release claim. Eight exactchecked files retained; geometry/defaults/Box3D/WASM/other localwork preserved. No additionalcommitpush.
- [x] **CAP01 — Large World preparation exceeds device capacity.** Checked typed
  reservations retain supported layouts and the complete million-tile floor;
  static traversal, redundant inertia uploads and exact distant-box CCD rejection
  remove traced preparation/step costs. [Verified repair with CPU-first before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/CAP01.md), three final comparison rows (local audit archive), audit (local audit archive). ActualCPU/ordinary/native original540step full100sphere support/geometry/identity/3picks each PASS; checked allocation8cases/staticAPI21cases/packed6vectors/BOX8vectors+10support cells PASS. All15final60step captures and3individual reviews013/019/052 pass clearhealth; source/media audit320pins/119CPU source-object pairs/36absentPIDs PASS2.888s,536relocated rawfiles PASS0.763s. Original failures retained. Defaultcamera floor-only, other162final scenes UNREVIEWED, manual controls/maximumdensity/growthrelease/determinism/FPS/nativeperformance and VillagePERF06 remainunqualified. Box3D/WASM/defaults/geometry/IDs/picking preserved; localuncommitted/no newcommitpush.
- [x] **RUNTIME01 — Candy Cups does not complete: resolved without code change.** Scene001. Original90s software-rendered collector cutoff, CPU alsoaffected; full300step actualCPU/ordinary/native/combined clearhealth, all25after/beforestages reviewed. Originalfactory300step support/geometry/IDs/3picks/destroy PASS eachbackend; 350visual+359physicalpins/1500framerecords/12absentPIDs auditPASS77.854s. [Report and before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/RUNTIME01.md), CPU-first clips (local audit archive). No engine repair/nativeFPS/manualcontrol/longwindow qualification; original failures retained.
- [x] **RUNTIME02 — Sensor freezes at step32.** Scene016; original callback shape metadata getter deadlock repaired with owned all-world metadata scopes. [Verified repair with actual CPU-first before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/RUNTIME02.md), closure (local audit archive). Both original GPU step33 failures retained; two-file candidate/metadata guards/ordinary-native ABI builds PASS. Original120-step factory and nine filter/sensor/deletion/pre-solve/CCD controls per backend PASS. Five120-step viewers complete/clear health;1 final review0left, other164 final scenes UNREVIEWED. Postcapture435+371pins/600records/12absentPIDs and closure362+364pins/119 CPU source-object pairs/38linked inputs PASS. Three CPU fixture assumptions, offline folder/spawn/provenance mistakes, native CCD transient and first/late preparation frames explicitly retained. Default geometry is culled; full120 progress established numerically, not inferred from late screenshots. Arbitrary callback API reentry, manualpicking, longwindow, repeatability/performance/release remain unqualified. Box3D/WASM/defaults/geometry/IDs unchanged; all budgets CLOSED, no newcommit/push.
- [x] **RUNTIME03 — Stall freezes at step4/capacity loss.** Scene060 (CAP02 duplicate). Actualshape BVH nodecount replaces arbitrary2048 traversalcutoff; unchanged64stack/dropfallback and stickyinvalid-state remaintruthful. [Verified CPU-first before/after report](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/RUNTIME03.md), closure (local audit archive). ActualGPU/CPU exactfrozen32767node traversal2048/6pending→3036/0;6boundaryguards andforcedcapacity/proof checksPASS. OriginalCPU/ordinary/native120factory/geometry/identity/Launch/publicrayselection andgroundsupport/initialenergy boundsPASS. FiveactualCPU-first120captures clearhealth/all600scalarrecords EXACTfactory;all25before/25after stagesreviewed. Source/mediaaudit PASS3.366s/482pins/119CPUsource-objectpairs/38linkedinputs/82absentPIDs. Sevenlargeallocationwatchcases reconciled per-shape whereextractable;Characterheadlessobserverfailures, stockCPU transientanalyticvertexpenetration andalloriginalfailures retained. Other164 finalcandidate scenesunreviewed; coldstartup/manualdrag/postLaunchphysics/fullSAT/sweptCCD/longwindow/repeats/performance/release remainunqualified. Allbudgets CLOSED,Box3D/WASM unchanged,no newcommitpush.
- [x] **PERF01 — Falling Trees timeout.** Scene007; conservative per-part mesh CCD sweep bounds remove millions of irrelevant TOI calculations, with exact per-call static-bound memo and UUID-compatible pipeline caches. [Verified repair / CPU-first before-after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/PERF01.md), closure (local audit archive). Actual4400 frozen TOI inputs/3.43million point enclosures/90 boundary cases/positive BVH and existing GPU CCD controls PASS. Original120 steps4.22/4.02s vs180s cutoffs; before102/103 rows EXACT5253/5304. OriginalCPU-first600 steps/defaults/51bodies/1101shapeowners/45floorpicks/destroy PASS, bothGPU facetedSAT checkpointsmax.003611721≤.005/every-step energybound PASS. Five120-step captures/all50before-after stages reviewed;120000 state-channel records EXACTfactory. Audit5.591s/228pins/119CPU source-object pairs/195media/293absentPIDs/54terminalphases PASS. StockCPU transientSAT.093727081 and original failures retained. Onlydefault007 finalreviewed;other164 finalcandidate scenes/presets/manualcontrols/substep/bodybodySAT/longwindow/repeats/nativeFPS/fullroadmap unqualified. Box3D/WASM unchanged,no additionalcommitpush.
- [x] **PERF02 — Height Field timeout.** Scene008; conservative moving-proxy/BVH-node slab intervals remove irrelevant candidates while preserving leaf order/narrowphase outputs. [Verified query repair and all10 CPU-first before/after scene comparisons](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/PERF02.md), closure (local audit archive). Actual14,278frozen inputs/81transformedcontrols EXACT,6,549,202→196,362candidates; normal builds/threeguardselectors PASS. CPU-first120Step+120actualRender2.969/17.850/13.890s;1,713,360casts/backend,13,340beforecallbackrows EXACT/everyframedigest/9publicfloorpicks/geometry/defaults/owners/destroy PASS.50clearhealth finalcaptures/500stages actuallyinspected/10scenes done0left; some30display guards do not step physics. AuditPASS10.070s/3921pins/119actualCPU source-objectpairs/2038media/1950healthrecords/182absentPIDs. Original3diagnostic/proof and2CPUaudit assumption failures retained. Independent117CPU/GPUfacet differences/.014739m point watch remainsPR11OPEN; can_encroach=true numeric matrix, manual/presets/longwindow/repeats/nativeFPS/other155finaldefaults and VillagePERF06 unqualified. All budgets CLOSED,Box3D/WASM unchanged,no newcommitpush.
- [x] **PERF03 — Joint Grid timeout/truncated CPU health.** Per-scan CPU joint bitmap, parallel exclusive non-overflow joint colors and pure CCD mask-first rejection remove three traced costs without changing fields, equations or defaults. [Verified repair / six CPU-first before-after scene comparisons](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/PERF03.md), closure (local audit archive). CPU collector35defaultscenes:88803Grid+23181related fullnumericrecords EXACT; complete120lossless1.083GB export11.134s, originalpartialhash retained. Ten focused ordinary/native selectors PASS, actual120Grid52.116/64.790s and2376000anchorchecks/backend with per-frameCPUmaxima EXACT/.0246697664. Normal observer-free libs/4viewerlinks/12publicstatic-dynamicpicks PASS. All30finalcaptures clearhealth;6actualindividualreviews150before/150lateafterstages, allfiveGrid120per-frameanchors/populationsCPUexact. FinalauditPASS28.467s/2078uniquehashes/283absentPIDobservations;4candidatefiles retainedbyte-exact. GPUdefaultaffected002010015123129164;other159GPUcandidate defaults/presets/manualcontrols unreviewed. CCDpredicate order shared more broadly with same pure decisions; no all165CCDqualification. Rain107residual/recycling, far-coordinatephysicalbounds, occludedindividualparts, startup/nativeFPS/fullroadmap remainOPEN. Allfailures/partialfiles/startup-shutdownframes preserved. PR09unchecked,Box3D/WASM unchanged,no additionalcommitpush.
- [x] **PERF04 — Rain timeout.** Verified and retained under user-selected option1 (2026-10-15). Per-call joint lookup and qualified loading/compiler prerequisites remove the traced timeout; geometry/defaults/IDs/picking and120-step states remain unchanged, first recycling/static picks pass. [Final report with CPU-first before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/PERF04.md), completion audit (local audit archive). Existing18anchor/27angular/249cone physical failures remain open underPR04, original limits unchanged; they are not marked passed by closing this performance item. Closed solver/scheduling experiments stay closed.
- [x] **PERF05 — Washer timeout.** Conservative static/kinematic convex target bounds remove traced hostCCD misses with originalgeometry/defaults/TOI/IDs preserved. [Verified repair and CPU-first before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/PERF05.md), closure (local audit archive). Actual41misses/90controls/269336pointenclosures/5positive/2invalid/regression andnormalordinary/native buildsPASS; matchedsoftwareordinaryfetch25.672→8.366s/all80dynamicrecords EXACT. ActualCPU-first120factory/all8002body8041shape defaults/hulls/IDs/120drumposes CPUexact;16publicfloorpicks+16CCDcontrolsteps/twoedit-reuseworlds eachbackend PASS. FivefullUI andfiveadditionalgeometry120captures clearhealth/all4.8m records perphase exact;25before+25fullUIafter+25unobscuredafter stages reviewed. AuditPASS3.481s/602pins/75absentPIDs,4exactfiles retained/32localchanges+49WASMpublicpatchfiles preserved. All165source/findingrows reconciled (2templatealiases manual), only017taggedPERF05; other164candidate defaults/presets UNREVIEWED. Hardware vssoftware also differsDPI/UI context, noGL-alone timing/nativeFPS/longmixing/drumSAT/fullstate/release claim. Originalfailures/metadata guards/occlusion/Escapeineffectiveness retained;allbudgetsclosed. PR09unchecked,Box3D/WASMunchanged,no newcommitpush.
- [x] **PERF06 — Village timeout.** Cached originalpublicgroupbounds and owned sweptBVH childcandidates remove demonstratedfullcompound movercasts whilepreservingfiltercallbackgroups/IDs/order/defaults/castmath. [Verified repair with actualCPU-first before/after screenshots andbothaffecteddefault rows](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/PERF06.md), closure (local audit archive). Actual14frozeninputs/fractions CPUexact/unit1536children3groups/normalbuilds PASS. Eachbackend16positive-veto-gapcasts/6publicpicks/twoworlds/edit-reuse-staleIDs/destroy PASS; all20matched120captures/100stages individuallyreviewed for032/052. Every131/176moverinputfraction and120bodyjointrecord unchangedperroute; existingjoint/supportscreens pass. Village~29→~7.8s allfourGPUroutes, CPU~2.4s unchanged; matchedNVIDIA1600x900/hideUI, notreleaseFPS. AuditPASS14.931s/421pins/86absentPIDs/32existinglocalchanges+49WASMpublicpatchfiles preserved; exact3testedfiles retained. Full165sourceinventory reconciled; other163candidate defaults/presets/manual/radius/reflectedmesh/fullchildSAT/substep/energy/longwindow unreviewed/unqualified. Priorfailure/software90stimeouts preserved; allbudgetsclosed, Box3D/WASM unchanged/no newcommitpush.

Runtime/performance items require successful bounded progression and applicable
physical/function checks after attributed repair, not unbounded retries or
suppressing capture. Final release timing/FPS remains PR08/PR12.

### Separate watch list and limits

<!-- PERF06 CURRENT BEGIN -->
PERF06 CURRENT — VERIFIED / RETAINED2026-10-07. **20fixed,1resolved,1open(RainPERF04); original165reviewed0left; final2affecteddefaults reviewed0left,other163currentdefaultsUNREVIEWED.** ReadPERF06/{README,log}.md andfinal-evidence/{retention,closure}.json. Allphasebudgets CLOSED, no live/queuedproducer. Finalauditexec39312terminal0collected PASS14.931s:421uniqueinputpins/86absentproducerPIDs,32priortrackeddirtyentries/49WASMpublicpackagepatchfiles+twoWasherfixtures preserved; Box3Dtrackedclean. ReleaseWASMartifactabsentbeforeafter/no build. Freshfetch86799terminal0/headc076092/0ahead0behind; gitdiffcheck0. No newcommit/push.
Exact3testedfiles retained: query7fabd4e8/worlda4c62a29/perf06_mover_snapshot.rsfb47e5ab. Moverqueryindex cachesoriginalpublicbounds/refits, ownsconservativeBVHchildcandidates; preservespreviouspublicfiltergroups/order/IDs (evenemptychildren)/castmath/defaults/geometry. Unit1536children3groups/positive-miss-boundary-order-refit-ownedlifetime PASS41.054s; normalordinary17.705/native19.409s build; heldarchivesf4a61bab/1c949ef5 matchtestedsource. SharedCargooutputs nowcandidate; retainedsourcesfreshmtime makefuturerebuildhonest. OlderWasherworldb979/query659 andMC3f9df6cc/c90ac650 remainimmutablebaselineevidence.
ActualCPU-first publiccontrols each16casts/8positive/4veto/2parentgapcallbacks/6picks/twoworlds/transformshapeedit/staleIDs/destroy PASS withunchanged1e-5fraction/0.005endpoint limits,0physicssteps. Matched120beforeafter20apps+recorders PASS/10links0:all131(032)/176(052) moverinputsfractions andall120dynamicbody/jointrecords EXACTperroute (1535moverpairs/1200bodypairs/600jointpairs). ExistingBasicMoverjoint/supportscreens PASS; Villagecapsuleendsy3.0795455CPU/3.07954574GPU, camera-clipped notunboundedfall. Villageordinary28.995→7.839/native29.180→7.739/ordinaryboth29.067→7.924/nativeboth28.444→7.772s, realCPU~2.4/BasicMover~3.3unchanged. SameNVIDIA1600x900/defaultgeometry/hideUI/context; boundedtimingsnotreleaseFPS. All100stages inspected, CPUfirstbeforeafterscreens/report and2row10columnclipgallery saved. All165sourcerows reconciled163classes+2templatealiases; directmoverdefaults032/052 only, other163explicitUNREVIEWED.
Original16-callofflineassumptionfailure preserved/correctedtoactual14 withoutengine rerun; alloriginalsoftware90stimeouts preservedseparately. No whole52500childvisual/SAT/substeps/energy/longwindow/presets/fullUI/manualdrag/T/jump/radius/reflectedmeshqualification. PR09unchecked; remainingRainPERF04 combinedphysicalgate unchanged. NEXT followlatestPERF04CURRENT: contact22/position split and native chain operation are closed; actual WGSL valid-zero mesh ID guard passes13 artifact-only cases. Full finalization/history controls pass on ordinary/native; the current-source GPU scene cuts off after73steps including124.433s pipeline creation. Next is a distinct isolated compiler-boundary comparison under a new finite budget (no repeated passing primitives or older600/900/scheduling/cold campaigns). Scopequestion pending; noautomaticwaiver. GoalACTIVE, notcomplete.

Finaldocumentation receipt CLOSED/PASS0.010s/directterminal0:22groups/21checked/onlyPERF04open;20fixed1resolved1open/PR09unchecked, exact3retainedsources/all86PIDsabsent, CPUfirstreport/gallerylinksvalid, originalinventoryonly032/052repairannotations added. No newproducer/commitpush.
<!-- PERF06 CURRENT END -->

<!-- PERF05 CURRENT BEGIN -->
HistoricalWashermilestone below; currentcounts/source/nextaction areinPERF06CURRENT above. OriginalWasherproof/archives/budgets unchanged.
PERF05 CURRENT — VERIFIED / RETAINED2026-10-07. **19 fixed,1 resolved,2 open; original165 reviewed,0 left; finalWasher1 reviewed,other164 candidate defaults UNREVIEWED.** Read PERF05/README.md, final-evidence/{retention,closure}.json and washer-geometry-capture/review.json. Alljobs/budgets CLOSED, no live or queuedproducer. Lastaudit timeout60s final-audit.py PASS3.481s/exec12732terminal0collected:602unique pins/75absentproducerPIDs,32existinglocaltrackedchanges and49WASMpackage/public/patchfiles preserved; Box3Dtrackedclean. ReleaseWASMartifactabsentbefore/after, no WASMbuild.
Exact4testedfiles retained: world.rs b979bac9 (per-call conservative static/kinematic nonmesh target bound afterold filters/COM/coarsecheck), query.rs659ffbee (internalexistinghelper wrapper/testinclude), perf05_ccd_bounds.rs226e4103/jsonffd85076. OriginalTOI/fastbounds/mesh/dynamicbullet arithmetic andgeometry/defaults/IDs unchanged; invalidbounds fallback. Normalheldarchives MCordinary3f9df6cc/nativec90ac650; sharedCargo outputs matchcandidate; sourcefreshmtime makeslater rebuildhonest. No readonlyobserverenginearchive retained. Focusedunit/ordinary/native builds+41misses/90controls/269336actualpoints/5positive/2invalid PASS; first80 dynamicexports exactperbackend. Same-software ordinaryfetch25.672→8.366s (~67% less), nativeunchanged/no speedupclaim.
ActualregisteredCPU-firstWasher120 factory/defaults/all8002body8041shape IDs/hulls/initialstates/120static+kinematicstates CPUexact; eachroute16floorpicks/16CCDcontrolsteps in2worlds/transform+shape+worldrecreate/staleIDs/destroy PASS.0analyticalcube-groundvertex violations, cubesaboveground; no settledground/drumSAT/substep/energy/manualdrag/fullmixing proof. FullUI5captures and additionalexisting--hide-ui5geometrycaptures all120 clearhealth;4.8m dynamicrecords eachphase exactrespectivehardwarecontrols. All25originalbefore+25fullUIafter+25unobscuredafter stages inspected, CPUfirstgallery/actualscreens displayed inreport. Originaltimeouts/failures/controlsocclusion/Escapeineffectiveness retained. Captureactualsapp_x11_get_window+PID, neverdesktoproot. Actualbefore1280x720Mesa versusafter1600x900NVIDIA/desktopXft120=1.25; envlogical1280honored, notignored/crop. Earlyannotations/hypotheses superseded; context+resolution/UI changes notGL-alone causal timing/FPS. EarlierVIS01/VIS02 explainpalette/griddifferences, notthisCCDrepair. Clipsapproximate andlateCPU/native terminalposesrepeat; no exactticks/trajectories/longmixing claim.
Full165 source/finding reconciliation (163lexical+2manualMixedStackstemplate aliases) confirms017 onlyPERF05tag; potentialsharedgenericpath inallother164/presets remains explicitlyunreviewed. Original165survey complete, browsergaps separate. PR09unchecked. Branchfeat/gpu c076092; freshfetch82015terminal0/0ahead0behind. No newcommit/push authorization; source/docs remainlocal.
NEXT: bounded PERF06 Village052 diagnosis fromsavedoriginalevidence, undera newfiniteprotocol; notyetfrozen/launched. RainPERF04 scopequestion pending/currentcombinedphysicalgate unchanged; donotwaivepriorfailures/resumeolder exhaustedsolver/scheduling/600/900/cold/825cellcampaigns. ReadfullreviewlogFIRST,AGENTS,roadmap/currentreport afterrestart/compaction. Finishoneverifieditem/report atatime; goalACTIVE withtwoopen, notcomplete.

Finaldocumentation closure PASS3.386s/exec40544terminal0collected: all602 artifactpins checkedagainstimmutablefiles or preserved3metadata snapshots,4retainedsourcesexact/all75PIDsabsent;22checklistgroups20checked (19fixed+1resolved),2openPERF04/06;PR09unchecked; report/media links CPUfirst, onlyoriginalinventoryrow017 amended. No newproducer/commitpush.
<!-- PERF05 CURRENT END -->

New read-only VIS06 finding: standaloneCdefault filter category1 vsCPU/combinedUINT64_MAX. Exactsource/actualvalues (local audit archive) reopenPR02; mask-specific query/collision effects unqualified. Preserve as an independent API/default watch, outside the original22group count; no unrelatedsolver/default repair authorized by closingVIS06.

Far Pyramid163 setup triage is complete: [native setup/input audit](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/FarPyramid.md) proves shared float32 placement creates200coincident cube pairs before CPU/GPU routing. Evolved collapse remains unconfirmed as a GPU defect; retain it as a physical watch, not another repair. The13 browser/native gaps, browser qualification, recording/player
implementation and full PR01–PR13 certification are outside this22-item goal.
VIS04 camera/panel occlusion is an inspection limitation, not proved missing
geometry; audit it where repair verification requires it. Before solver work
also read gpu-solver-goal.md and qualification history. Box3D/WASM stay unchanged.
No exhausted experiment is resumed.

### Historical repair recovery (current publication recovery is at the top)

Freeze clarification, 2026-10-09: the user confirms freezes mainly affect this chat and commands. Retained incident evidence shows 13,492 Git children of the Codex runner (6,765 zombies), an app scope peak at its 18,903-task limit and command-spawn EAGAIN failures. App subprocess buildup is the leading diagnosis; the initiating Git operation and a persistent RAM leak remain unproved. The existing host report (local audit archive) already records a quiet subsequent snapshot. Do not repeat quiet snapshots to answer the same reply: the next useful host evidence requires an actual recurrence. Isolated 256-task qualification scopes are a workaround, not an app repair. No new host diagnostic budget was opened for this clarification. Current loading checks after Wheel are 9 done/3 left; the remaining three require the held general-mesh path. Repairs remain 20 fixed/1 resolved/1 open.

<!-- PERF04 CURRENT BEGIN -->
PERF04 CURRENT — **VERIFIED / RETAINED;21 fixed,1 resolved,0 open;165 reviewed,0 left.** Updated2026-10-15. User selected option1: finish verified timeout milestone and retain existing Rain physical failures underPR04. PERF04[x];22-item repair goal COMPLETE, confirmed by update_goal on2026-10-15 after final preservation/cleanup audit. PR09 comparison acceptance is now closed by the portable publication above; full production readiness remains incomplete. The user subsequently authorized this PR09 commit/push.

**Result:** [Final report and actual CPU-first before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/PERF04.md). Repeated hostCCD joint scans replaced by a per-call blocked-body-pair lookup; loading/compiler prerequisites verified. Six exact current source files and ordinary/native normal.a archives2740d6/8e1f69 match tested artifacts. Current retained implementation is accepted for the timeout milestone only; no rejected solver fallback/coupled/order/CCDpolicy is promoted. Existing.so/.rlib were not rebuilt and are not claimed source-fresh.

**Evidence:** matched headless120 ordinary98.206→9.933s/native102.214→10.184s;134400body+134400joint observations perbackend exactlyunchanged. Five current CPU-first120 captures3.246/9.724/9.408/9.628/9.440s;45unique stages+4fullPNG previously inspected. Current326 CPU/ordinary/native lifetime/staticpicks pass798000body+joint each,420retirements at321,6floorpicks and destroy invalidation. Completion audit (local audit archive) also checks22reports/521image references,165reviewed native entries/13explicitbrowser gaps and current source/static-archive identity.23fresh source-specific scene reviews plus prior applicability/21held covered0left;142notfreshlyrecaptured is not a new review backlog. Capture timings are not nativeFPS/coldstarts or exact scene-trajectory matching.

**Open release work:** Rain120 original18anchor/27angular/249cone/0/0twist physical failures persist unchanged and unwaived underPR04. Older326physical failures, loaded dragging/CCD, energy/penetration, fullstate and long windows remain source-scoped/open. Native calculations at selected106/107 boundaries pass but do not qualify whole-scene physics. Far Pyramid setup triage complete: shared native float32 arithmetic creates200coincident cube pairs; evolved collapse unconfirmed, not fixed/countedin22groups. Manual/presets/dynamicpick/browser/recording/otherdevice/FPS qualification remains separate. All historical failed/exhausted budgets stay closed.

**Recovery:** feat/gpu c076092; fetch2026-10-15 origin0ahead0behind. All producers terminal/absent and execution budgets closed. Current six-source accepted timeout patch remains in the shared worktree with normal static archives2740d6/8e1f69; Box3D/WASM unchanged and other local work preserved. Do not run historical source-restoration scripts. Required before/after media and minimal physical failure inputs remain; Final preservation/cleanup receipt (local audit archive) verifies53other local files unchanged and removes the consumed88,600,826-byte native restoration backup after finalization; current builds/media/minimalPR04inputs remain. Closing diagnostics~28KiB, no private cache/fullhealth/binary outputs. No new builds/captures during closure. Stop after22-item milestone; do not startPR04 or fullroadmap automatically.
<!-- PERF04 CURRENT END -->

<!-- PERF03 CURRENT BEGIN -->
PERF03 CURRENT — VERIFIED/CLOSED,2026-10-06.18 fixed,1 resolved,3 open;165 reviewed,0 left;PERF03 final6done0left. Read PERF03/README.md/log.md/closure.json/retention.json. Finalauditexec12680collected0/PASS28.467s,2078uniquehashes/283absentPIDobservations;allcapture/process phases terminal, no livejobs. Exact4compiledcandidatefiles retained: solve.wgsl perjointvalidnonoverflowcolor lanes, world.rs pureCCDmaskfirst+regression, manifold_chain_tests.rs actualoldshaderwordequivalence, sokol_bench_hooks.cpp per-worldCPUjointbitmap. CPUcollector35defaults exact;10GPUselectors/picking12/120Gridactualanchors per-frameCPUexact;30CPU-first finalclips/sixindividualreviews with originalpartialfailures/loading/shutdownframes preserved. Normal namedGPUarchives observer-free; sharedCargo outputs correspondcandidate, sourcefreshmtime forces honest subsequentbuilds. PR09unchecked; remaining originalgroupsPERF04Rain,PERF05Washer,PERF06Village. NEXTfirstfreezea newbounded Rainboundarydiagnosis fromexisting107residual evidence; no old600/900s/exhaustedsolvercampaign resumed. Manual/presets159otherGPUcandidate defaults/farphysicalbounds/recycling/startup/releaseFPS/fullstatequalification remainunreviewed. Box3D/WASM/localworkpreserved;fetchHEAD/origin0ahead0behind. No additionalcommitpush.
<!-- PERF03 CURRENT END -->

<!-- PERF02 CURRENT BEGIN -->
PERF02 CURRENT — **VERIFIED / RETAINED**, 2026-10-06. **17 fixed, 1 resolved without repair, 4 open; original165 reviewed/0 left; final affected/default guards10 reviewed/0 left.** PR09 and the22-group goal remain incomplete. Branch feat/gpu c076092a;remote0ahead0behind;Box3D/WASM/other local work unchanged. No additionalcommit/push requested; checked repair retained uncommitted.

Conservative translating-proxy/BVH-node slab filtering removes irrelevant candidates from actual00880km diagonal casts while preserving surviving leaf order and narrowphase.14,278 frozen casts/13,340 hits and81 transformed controls old/new EXACT; candidates6,549,202→196,362. Normal unit/ordinary/native builds and three CPU guard selectors PASS. Actual registered CPU-first120Step+120Render2.969/17.850/13.890s;1,713,360casts/1,600,800hits perbackend, all13,340beforecallbackrows EXACT/everyframequerydigest invariant/9publicfloorpicks/geometry/defaults/owners/destroy PASS. Static008 has no dynamic landing solver claim. Independent117CPU/GPUtriangle/normal differences/maxpoint.014739m remain OPEN PR11 watch; numerical can_encroach=true matrix remains unqualified.

All50 finalcaptures complete/clearhealth; all500before-after sampled stages individuallyinspected across008/020/032/034/036/038/040/043/052/121.008120physicssteps; related30displayrecords with explicit query-only/empty nonstepping guards020/040/036. Original before clips unchanged; prior terrain/material/overlay repairs and preparation/blacktail/offcamera limits separately labeled. Final CPU-only auditPASS10.070s:3921pins/2038media/1950healthrecords/119actualCPU source-object pairs/182absentPIDs. Original3diagnostic/proof failures and2CPUaudit assumption failures retained. All budgets CLOSED and jobs TERMINAL; lastcommand timeout60s final-completion-audit.py exit0/session62028 collected, queue66079 terminal0 collected. No GPU/compiler/capture job running. [Report with before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/PERF02.md), CPU-first all10-row grid (local audit archive), closure audit (local audit archive).

NEXT: PERF03 — Joint Grid010 timeout/truncated CPU health. Read full-reviewlog FIRST, AGENTS.md, roadmap and the PERF02 report/source identities after every restart/compaction. Freeze a distinct finite budget before builds/checks/captures, inspect retained raw010CPU truncation/GPU progression, trace the smallest relevant operation, preserve constraints and do not assume this mesh-query repair fixes joints. PERF04Rain/PERF05Washer/PERF06Village remainOPEN. All manual/presets/longwindow/other155finalcandidate defaults/nativeFPS/repeatability/fullroadmap unqualified. Original165sweep and older exhausted Rain/scheduling/cold missing-display budgets stay CLOSED. No unchanged retries or newcommitpush.
<!-- PERF02 CURRENT END -->

<!-- PERF01 CURRENT BEGIN -->
PERF01 HISTORICAL HANDOFF (superseded by PERF02 CURRENT above) — **VERIFIED / RETAINED**, 2026-10-05. **16 fixed, 1 resolved without repair, 5 open; full native survey165 reviewed/0 left; Falling Trees007 final review1 done/0 left.** PR09 remains unchecked. Active22-group goal remains incomplete. Branch feat/gpu c076092a; remote0ahead/0behind; prior local work/docs retained, Box3D/WASM unchanged, no additional commit/push.

All budgets/jobs are CLOSED/TERMINAL: closure audit5.591s verifies228 source/linked pins,119 actual Box3D CPU source-object pairs,195 media files,293 absent PIDs and54 terminal phases. Last command was final-audit.py under60s timeout, exit0/session2821 collected. No GPU/display/capture job is running. [Verified repair and CPU-first before/after screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/PERF01.md), closure (local audit archive). Read those before reusing binaries or results; observer archives are artifact-only, checked normal production archives are mesh-bound-candidate/ordinary.a and native.a with final sources saved in final-sources/.

Root cause: per-part COM spheres admitted millions of irrelevant mesh triangles into host CCD. A conservative COM-linear/nlerp sweep enclosure retains TOI arithmetic, BVH relative order, target shell and centroid retry; exact per-call static-target bound memo removes repeated floor scans. Actual4400 frozen inputs preserve TOI outputs;3.43million point enclosures/90 boundary cases/positive BVH and existing actual GPU CCD controls pass. Original120 factory now4.22/4.02s vs180s timeouts at102/103; all5253/5304 prior rows EXACT. Original600 steps14.54/13.59s,geometry/defaults/IDs/45publicfloorpicks/destroy pass; GPU faceted SAT checkpointsmax.003611721≤.005 and every-step mechanicalenergy≤initial.49asleep/1awake at600. StockCPU transientSAT.093727081 and both analyzer/fixture failures are preserved, not relaxed or repaired.

Diagnostic coldstarts separately traced to missing DISPLAY=:87 selecting incompatible NVIDIA eglcore cacheUUID. Pipeline cache validates/namespaces actual UUID and preserves compatible legacy; actual per-backend cacheAPI controls and live recording preflights pass. All5CPU-first120 captures clearhealth; all50before/after stages reviewed and120000 capture p/q/v/w channelrecords EXACTwithinbackend. Softwaregraphics/capture walltimes are not nativeFPS. Other164 scenes on this final revision,50/25cm presets/manualwidgets/bodybodySAT/sweptsubsteps/beyond600/repeats/fullrelease remain UNREVIEWED or unqualified. Original exhaustive165 review stays complete;13browser/native gaps are outside22groups.

HISTORICAL NEXT (completed; current next PERF03 above): PERF02 — Benchmark/Height Field008 timeout, one new bounded item/report at a time. Read this log FIRST, AGENTS.md and the roadmap after continuation/compaction; then PERF01 closure/source identities and original008 evidence. Do not assume the shared bound change already fixes008, and do not replay old/exhausted budgets, cold missing-display compiler waits or Rain/scheduling campaigns. Freeze a distinct finite budget and trace the smallest actual operation before repair. The other open groups are PERF03 JointGrid, PERF04 Rain, PERF05 Washer and PERF06 Village.
<!-- PERF01 CURRENT END -->


observer-free-cache-export CLOSED:actualwgpu export100bytes withoutLD_AUDIT/LD_PRELOAD; stopSIGTERM0.358s aftercompletefile,beforeanypipelineend/heavyshader. Diagnosticobserver notsufficientcause. instance-provider-metadata FROZEN:0Rustbuild/candidate/1Ccompile25s/1actualhelper10s/0shader/physics,captures,retries. Completeinstance/AppVersion/pNext/allocator +physicalUUID/API andVulkanproviderlibrary metadata at earliestdevice/cacheboundary.

device-ci-padding CLOSED:actual/copy-original-padding/zero-padding all36bytes. observer-free-cache-export FROZEN:0build/candidate/1actualhelper10s/0physics,captures,retries. NO LD_AUDIT/LD_PRELOAD; capturesactualretainedwgpu cacheget_data; supervisor1ms pollsvalidcompletefile andterminatesbeforefirstheavycompiler; inspectpipelinefrontendlogs. Thisvalidatesobservereffect atAPIboundary.

device-return-cache-import CLOSED:bothactualfactory/helper driverdiscardsblobIMMEDIATELYafterunchangeddevicecreation (36bytes),beforeanywgpu resources/shaders. Queues0flags/family0/1count/priority1,deviceflags0/layers0/allocatorNULL. device-ci-padding FROZEN:0Rustbuild/candidate/1Ccompile25s/1actualhelper10s/3Vkdevices+cacheimports/0shadercalls,physics,captures,retries. OriginalvsbytecopiedvsABIpaddingzeroedDevice/Queue/featureCI;semanticfields unchanged, explicitcleanup.

live-internal-pipeline-cache CLOSED:both2kernel-liveanddestroyedretain26MBexact. device-return-cache-import FROZEN:0Rustbuild/candidate/1Ccompile25s/2actualfactory-helper10s/0shadercalls,physics,captures,retries. ImportexactcacheIMMEDIATELYafteractualunchangedVkCreateDevice returns, beforewgpu resourceinitialization; logqueueCI; explicitlydestroycache/device thenstop86.

c-cache-import-context CLOSED:standaloneC andsameC linkedfullsample bothretain26MBexact; binarylibraries notcause. live-internal-pipeline-cache FROZEN:0build/candidate/2rawprocesses5s/4tinyinternalcalls/0physicsshader/physics,captures,retries. Keepbothinternalpipelines+modules+layouts+setsLIVEatcacheimport vspriorreplaydestroyedthem; explicitcleanupafterexport.

cache-import-ci CLOSED:actual/normalized/aligned64 all36driverbytes onbothfactory/helper; inputtype17,nextNULL,flags0,size26532353,pointermod64=16. c-cache-import-context FROZEN:0Rustbuild/candidate/2Ccompiles+1link25s/2Cimportprocesses5s/0shadercalls,physics,captures,retries. ExactsameCAPIinputs standalonevsoriginalfullsamplelinkedcontext isolatebinarylibrariesfromwgpuinit.

internal-pipeline-cache-order CLOSED:raw exactdevice retains26MBafter2actualinternalmain kernels(6.97/9.16ms); initializationkernelorder notcause. cache-import-ci FROZEN:0Rustbuild/candidate/1Ccompile25s/2actualfactory-helper10s/6cacheimports/2internal tinycallsperprocess/0physicsshadercalls,physics,captures,retries. AtfirstfailingimportcompareunchangedCI/normalizedpadding/64alignedbytecopy; stop86.

exact-device-cache-import CLOSED:exactcapturedfeaturechain/order/instance retains26MBbyteexact; reverse/cacheControlalsoexact. internal-pipeline-cache-order FROZEN:0build/candidate/2rawprocesses5s/2tinyinternalmainfullcalls/0physicsshadercalls,physics,captures,retries. Replayactualwgpu internal2kernelsbeforecacheimport vsnone withsamecaptureddevice andbindings/pushranges4/8.

original-observer-control CLOSED:retainedexactoldobserver nowalso100WGPUbytes/36driverbytes, no altered-observer attribution; positionpipeline94.512ms only, jointedstoppedbeforecall. exact-device-cache-import FROZEN:0build/candidate/3rawprocesses5s/0shadercalls/physics,captures,retries. Recreatefullcaptureddevicecore/features/order/extensions plusinstance; exact/reverse/cache-control compareatcacheimportitself.

helper-render-environment CLOSED:all6 unmodifiedhelpercache roundtrips36bytes, envvariablesshow nocause. original-observer-control FROZEN:0build/candidate/1actualhelper10s/1positionkernelallowed/0jointeddrivercalls,physics,captures,retries. Retainedoriginalobserver aspairedcontroltochangedobservers; newcontrolforprobeperturbation, notfullscene retry.

minimal-cache-roundtrip CLOSED:bothactualfactory/helperunmodifiedfeaturesstill36bytes. Compareearlieractualhelperenv:softwareGL/vendor/prime/DISPLAY+factoryflags wereabsentinsuccessfulhelper. helper-render-environment FROZEN:0build/candidate/6actualhelperprocesses5s/3modules each/0physicspipelinecalls/physics,captures,retries. baseline/softwareGL/vendor/prime/display/allfactoryenv; exactminimalobserverunchanged.

unmodified-cache-roundtrip CLOSED:bothoriginalfeature factory/helper imports26MB butexports36driverbytes; featureenablealone notcause. minimal-cache-roundtrip FROZEN:0Rustbuild/candidate/1Ccompile25s/2actualfactory-helper10s/3modules each/0physicspipelinecalls/physics,captures,retries. Originalminimalobserver noInstance/Device wrappers orpreimportmalloc; isolateobservereffect. Initialsource-generationdelimiterfailure preserved, no test hadlaunched.

helper-cache-miss-scope CLOSED:changedcache-control device exports100WGPUbytes(36driverbytes) vsoriginalhelper26MB. Priorapplicationmiss may beobserver-induced, notproductioncachemiss. unmodified-cache-roundtrip FROZEN:0Rustbuild/candidate/1Ccompile25s/2actualoriginalfactory-helper10s/3moduleseach/0physicspipeline calls orfeaturechanges/0physics,captures,retries. Exportcacheatfirstpipelineboundaryandstop86 beforedrivercompiler.

helper-cache-miss-scope FROZEN beforeexecution:0build/candidate/1actualhelper10s/5modules+3cache-onlyqueries/2internaltinycalls/0physics,captures,retries. Reuseexactsameobserver asTreesboundary; compare actualwgpu helper tofactory, notonlyrawVulkan. Previousgoalturn PROGRESS (applicationglobal3-entry missscope changednextaction), no blocker/job live.

HISTORICAL PERF01 RECOVERY — recovered 2026-10-05; diagnosis only. **15 fixed, 1 resolved without repair, 6 open; 165 initially reviewed, 0 left; Falling Trees final review 0 done, 1 left.** PERF01/PR09 remain unchecked. Branch feat/gpu at c076092a; local work retained; Box3D/WASM unchanged; no new commit/push.

All 21 diagnostic protocols are CLOSED, all 80 launched operations terminal and their recorded PIDs absent. Last command: `python3 .../PERF01/application-cache-miss-scope/run.py`, driver exit0 / intentional child86 in0.564s. No job running. Recovery audit (local audit archive). world/c_abi/sim/native_precision restored byte-exact; observer build archives are artifact-only, previous verified RUNTIME03 archives/viewers retained.

Original CPU reference passes. Previous actual ordinary/native first steps expire180s in pipeline preparation; 115 common pipelines take77.2/76.5s, then meshcollide driver-facing creation exceeds remaining>102/>103s. Current actualVulkan cache-only checks avoid compilation: inside actualTrees device, radix_histogram_0/integrate_pos/solve_jointed_wave all return COMPILE_REQUIRED; isolated exactshader/cache inputs hit~4–140ms with empty-cache controls requiringcompile. Actual cache/shader bodies match byte-exact. Recreating shader/layout/cache or normalizing CI insideactualdevice stillmisses. Corefeatures/extension booleans/stageflags/baseindex/app-engine/environment/GDPA/LD_AUDIT/readback/instanceextensions checked; cause remains unresolved. Earlier raw int8 type1000180000 was actuallyatomic64: erroneous interpretation retracted, correctlycaptured1000082000 probe alsohits. No production candidate or afterscreenshots; no completedGPU physics step in these probes.

NEXT: trace remaining application initialization context/order at this exact firstpipeline boundary, using a newly frozen short cache-only probe. Do not repeat timed-out fullscene/compiler processes or increase watchdog. No old Rain/scheduling budgets resumed. Old per-step fetch/hostCCD hypothesis remains unproved. Details: assessment (local audit archive), startup diagnosis (local audit archive), full log (local audit archive).

Prior PERF01 recovery history is retained in history (local audit archive).

RUNTIME03 VERIFIED / RETAINED / CLOSED (2026-10-05). **15 fixed,1 resolved without repair,6 open; original165 reviewed,0 left; finalStall1 reviewed,0 left.** RUNTIME03[x], PR09 unchecked, appgoal ACTIVE. Read fullreview log FIRST, AGENTS.md, roadmap and RUNTIME03/README.md/log.md after recovery. All jobs terminal; finalaudit verifies82 originalPIDsabsent. No newcommit/push; branchfeat/gpu baselinec076092a, lastfetch0ahead0behind, priorlocalworkretained, Box3Dclean/WASMunchanged.

Exactone-line retained shaderrepair: mesh-convex DFS visitlimit2048→max(actualshapeBVHnodecount,1), sourceSHAf43aa4c67628c8b786da4dd70a7a4ff5f26cbd6443a2e4a2767b0bacfdc0cadf. ActualGPU/independentCPU frozenoriginaltree32767 oldwalk2048/6pending vscomplete3036/0pending/allbitmapEXACT. Originalcompleted3 p/q/v/w remainsEXACT inrepairedfactory. Empty/small/balanced/valid64stackoverflow guards PASS; existingforcedstickycapacity/proof testsPASS, no diagnostics cleared/default/tolerance change.

OriginalCPU/ordinary/native120factory PASS.164/133.937/132.732s;360finite/stablebodyshapeownerrows each, sameactualnative10points/mass/inertia, originalLaunch/newbodygeneration/staleinvalid/publicrayowner checksPASS. GroundminvertexY.000483392/no fall-through; Emax<=unchanged400clampedinitialenergy, speed/quaternion1e-5 PASS. GPUanalytic torusvertexpenetration.000376919<existing.005slop;vertex-only screen, stockCPU.030665957transient retained, no fullSAT/sweptCCD claim.

FiveactualCPU-first final120captures clearhealth46.399/49.418/49.714/87.780/88.089s, all600statechannels EXACT correspondingfactory outputs. All25before+25after stages visuallyinspected;floor/torus retained, allfourGPUinvalidstep4 freezes cleared. Report withactualbefore/after screenshots/CPU-first clips: RUNTIME03/README.md andcompare/index.html. Source/media/CPUaudit PASS3.366s/482unique candidatepins/119CPUsource-objectpairs/38unchangedCPUlinkedinputs/600capturerecords/82absentPIDs, closure.json. Allfinitebudgets CLOSED; no unchangedretry.

Relatedper-shape audit: Chains4095/Trees16383 potentialapplicability;Rainmax127/Reflectionmax1571 despiteglobaltotals;HeightField131071(defaultstaticonly);Stallonlyconfirmedcutofffailure. Characterheadlessmetadata segfault twice retained (firstmissingassets, thenwithassets), no thirdattempt/fullviewerdefectclaim. Other164 finalcandidate scenes UNREVIEWED underthisitem, original165survey complete. Initialobserverquaternionfailure/120s preparationtimeout/hostparserfailure retained. Coldstartup (~133s firstfactoryuse), postLaunchphysics/manualdragging/fullcontactSAT/longwindow/repeatability/performance/release remainunqualified.

NEXT firstunfinished originalgroup: PERF01 FallingTrees007. Read its originalreview/source andfreeze aNEW finitebudget before attribution/checks. Do notresume exhausted Rain/scheduling/observer/capture budgets or whole release roadmap. Detailed oldinvestigation notes retained in RUNTIME03/investigation-history.md.

RUNTIME02 VERIFIED / RETAINED / CLOSED (2026-10-05). **14 fixed, 1 resolved without code change, 7 open; original 165 reviewed, 0 left; final Sensor 1 reviewed, 0 left.** RUNTIME02[x], PR09 unchecked, app goal ACTIVE. Read the full survey log FIRST, AGENTS.md, roadmap and RUNTIME02/README.md/log.md after recovery. No live build/viewer/capture job; all previous budgets CLOSED. Branch feat/gpu at c076092a; prior bounded fetch0ahead/0behind; local work retained, Box3D clean, WASM unchanged/not rebuilt. No new commit/push authorization.

Confirmed Sensor step33 callback(882:1:1,1682:1:1)→Shape_IsSensor deadlock while GPU stepping holds the registry mutex; both original backends expire60s, CPU120 passes. Two-file retained production candidate uses owned immutable all-world shape metadata and thread-local RAII scopes; no raw alias/mutation or physics/default/geometry/ID changes. Sensor/event-flag/user-data/validity getters cover custom-filter, pre-solve and pending host-CCD completion. Arbitrary callback API reentry/concurrency/mutation remain unqualified.

One unit build/two ABI builds/two metadata guards PASS; original CPU/ordinary/native120-step factory and nine sensor/filter/pre-solve/CCD/deletion controls per backend PASS. All original1681 static shapes/120 source spawns, owners/IDs/finite state, row20 rejection,75 begin/40 end events and calculated geometry support checked. Three CPU-only fixture assumptions and native CCD transient penetration retained; no unchanged GPU retries or physics/default/tolerance edits.

Five actual CPU-first120-step captures complete/clear health in11.408/15.982/15.883/19.062/18.939s. All25 before and25 after sampled stages reviewed, five running stage3 originals show UI53/57/58/57/58 beyond32. Default bounds correctly hide all Sensor geometry, so screenshots do not establish visible dynamics. First/late black/preparation panels retained explicitly; separate600 health records establish full120 progress. Other164 final scenes UNREVIEWED for this candidate; original165 survey complete.

Postcapture audit PASS5.321s/435 visual+371 physical pins/all600records/120 stable dynamicIDs per mode/12 absent original PIDs/two exact production sources. Closure PASS3.561s/362 candidate+364 link pins/119 original CPU source-object pairs/38 CPU linked inputs/exact corrected DIAG01 hook object+binary. Original hook witness0ba3b2 reconstructed by reversing only previously verified BEH03 health AABB1e8 expansion to1e5; original and current observer bounds include Sensor. Folder-name, post-step spawn-count and CPU-hook provenance audit failures retained; no reruns of engine/captures.

Report: RUNTIME02/README.md; actual before/after: before-late.jpg/after-running.jpg; CPU-first clips: final-visual/compare/index.html; evidence: physical-assessment.json, postcapture-result.json, closure.json. Manual picking, arbitrary callbacks, long-window arrival, repeatability, FPS and release qualification remain open. NEXT: RUNTIME03 Stall060, then PERF01–06. Read its existing evidence and establish a distinct finite protocol before any execution. Do not resume closed Sensor/Candy/CAP01/Rain/solver/scheduling campaigns or the full release roadmap. FarPyramid/FarRagdoll/default-query-mask watches remain open.

RUNTIME01 closure below is retained completed evidence; the RUNTIME02 record above is current.

RUNTIME01 VERIFIED / RESOLVED WITHOUT CODE CHANGE (2026-10-05). **13 fixed, 1 resolved, 8 open; original 165 reviewed, 0 left; final Candy Cups 1 reviewed, 0 left.** RUNTIME01[x], PR09 unchecked, app goal ACTIVE. Read full survey log FIRST, AGENTS/roadmap, RUNTIME01/README.md and full-visual/log.md after continuation/compaction. No live job: session62320 terminal0, five300step viewer cells PASS; session45799 terminal0, offlineaudit PASS77.854s under180s. All12 originaldriver/display/viewer/recorder PIDs absent. Actualboot06ee28f2-b324-4a7b-922c-10b294553735 remained unchanged at recovery; fetch0ahead/0behind, Box3D clean, localwork retained, no newcommit/push.

Original300step/90s captures (CPU included) expired during software-rendered viewer work; no proved Candy runtimefreeze. GPUfirstpreparation/drawing2–3s was not90s coldstart. FullactualCPPfactory300step physical progression PASS CPU0.665s/ordinary18.454s/native17.752s; original4096hulls/4097bodies/defaults/geometry/IDs/owners/everystep finite/support/worldbounds, finalall4096sleep/speed0,3closestpicks and postdestroy invalidation/backend. Nativehullvertex floorbound-0.015 unchanged, worstCPU-0.00119533041/GPU-0.00080751098. Initialfixturecompilefailure/publicaccessorcorrection preserved; no production candidate.

Allfive actual300step viewers complete/clearhealth: realCPU138.195s,ordinary144.640s,native139.794s,ordinarysplit264.977s,nativesplit245.902s. Mesa softwaregraphics/NVIDIAcompute. Render medians436.3123/431.9822/428.1350/821.8135/782.9870ms versusphysics0.9289/20.9777/6.7009/21.6647/6.7791ms; diagnostic clocks, notdesktop/releaseperformance and notadditive. All25afterframes and25originalbefore stages visuallyreviewed: solidfloor/intactarray/supportedsettling, expectedstatecolors/intentionalblue; defaultpanelcrop remains. Offlineaudit350visual+359physicalpins/all1500framerecords/stable4096bodyidentities/allfinalasleep/noinvalidflags/12absentPIDs PASS. SevencheckedCAP01productionsources unchanged. Originalclips/rawfailures retained; allbudgets CLOSED, no reruns.

Report `pr09-repairs-2026-10-02/RUNTIME01/README.md`, CPU-firstclips `RUNTIME01/full-visual/compare/index.html`, audit `RUNTIME01/postcapture-result.json`, actualphysicalassessment `RUNTIME01/physics/header-correction/assessment.json`. Manualcontrols, longwindow, repeatability and nativeperformance unqualified; other164finalscenes explicitlyUNREVIEWED underthisitem. Original165survey complete. PR02defaultmask/FarPyramid/FarRagdoll watches and remaining8items remainopen.

NEXT: RUNTIME02 Sensor016 freezes at32. Read its fullsurvey/beforemedia and relevant lifecycle/spawning/event sources, establish first failing boundary, then freeze a distinct finite budget before any build/check/capture. Do not resume closed Candy/CAP01/Rain/solver/scheduling campaigns or fullrelease roadmap. Remaining8 RUNTIME02/03 and PERF01–06.

CAP01 closure below is historical completed evidence; currentRUNTIME01 record above takes precedence.


CAP01 VERIFIED / RETAINED / CLOSED (2026-10-05). **13 fixed, 0 resolved, 9 open; original 165 reviewed, 0 left. Final CAP01 matrix 3 reviewed, 0 left; direct Large World 1/1.** CAP01[x], PR09 unchecked, app goal ACTIVE. Workspace feat/gpu at c076092a; bounded fetch this boot found origin0ahead/0behind. Boot06ee28f2-b324-4a7b-922c-10b294553735. Local work is uncommitted; no new commit/push authorization. Box3D remains clean;593tracked WASM/demo/patch files unchanged. Ignored WASM binary absent before work; it was not rebuilt.

Report: `pr09-repairs-2026-10-02/CAP01/README.md`; CPU-first3row comparison: `CAP01/final-visual/compare/index.html`. All4original GPU preparation-failed panels and allocation-only180s/step38 timeout retained. The old90s failures were explicit preparation rejections, not measured shader cold starts. Checked typed reservations preserve previously supported layouts and select a static-heavy fallback only after original rejection; fullmillion static list/insertion workspace, native/old contact floors and nonstatic demand remain. Parallel static traversal preserves predicates/canonical keys/workspace ownership. Bit-exact redundant inertia uploads and exact BOX CCD rejection remove measured costs while retaining signedzero/asymmetric/oneULP and callback/nonfinite fallbacks. Exact7file source snapshots and separate before/candidate sources preserved; artifact observer exports read-only, production files restored after builds.

Focused allocation8cases, static/API21CPU/ordinary/native cases, actualpacked6vectors, BOX6radius+2fallback vectors and10CCD support/callback cells PASS; both GPU callback outputs1087visits identical. Original fixture/observer/link/preparation failures remain. Full originalC540step schedule PASS on actualCPU0.815s/ordinary388.991s/native388.680s:1m tiles+100spheres, valid body/shape/owner IDs and exact sphere geometry, all100supported/finite and3correct closest picks/backend.124pins verified; no scene/default/tolerance/ID change or trajectory/FPS claim. `full-default-progression/assessment.json`.

Final15captures (013LargeWorld/019BodyType/052Village ×CPU/ordinary/native/combined),60steps/180s watchdog,allcomplete/clearhealth. All75after sampledframes and20original failure frames actually viewed;3individual reviews saved. LargeWorld defaultview establishes floor only; spheres are offcamera and numerically checked separately. BodyType8bodies/3joints retains support/visible geometry; controls unexercised. Village nearby geometry/earlymover visible,1publicstaticbody and correct compound aggregate; latercapsule leaves defaultview, fullcharacter controls/support and PERF06 remainopen. Other162final CAP01candidate scenes explicitlyUNREVIEWED; full165original survey dispositions preserved.

Finalsource/media audit session61823 terminal0/PASS2.888s:320source/link/referencepins,119CPU source/object pairs (118current+exact original health-hook witness),38CPU linkedinputs,15captures/90mediafiles/36absentoriginalPIDs. Publication PASS0.763s/536relocated rawfiles/66.6MB; separate prose-only polish PASS0.216s/537current rawhashes, oldreport/indexhashes retained. Evidence `final-audit-result.json`, `publication-result.json`, `prose-polish-result.json`. AllCAP01 budgets CLOSED; sessions43316/53499/6723/48395/61823 terminal0, no live viewer/capture/display job. Nativeviewer stillslow; maximumdensity/growthrelease/repeatability/manualpicking/longwindow/performance remainunqualified. No exhausted campaign restarted.

NEXT unfinished item: RUNTIME01 Candy Cups001. Read full survey log FIRST, AGENTS/roadmap, CAP01 report/log, then its retained001 CPU/GPU/combined failures and relevant source. Create/read a simple item log, trace first failing collector/runtime boundary, and freeze a distinct finite budget before builds/checks/captures. Do not launch another item inside the completed CAP01 milestone. Remaining9: RUNTIME01/02/03 and PERF01–06. Candy/Sensor/Stall runtime, Villageperformance, FarPyramid interpretation, FarRagdoll joints, PR02defaultmask and capsuleX1e-5 watch remainopen. Do not resume old Rain/solver/scheduling experiments or the full release roadmap implicitly.

Current CAP01 diagnosis/log (local audit archive), exact source-sizing trace (local audit archive), allocation design and full archive census (local audit archive).

[Current report](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/BEH03.md), simplelog (local audit archive).

[BEH02 report](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/BEH02.md). BEH02 VERIFIED/CLOSED;11fixed0resolved11open,165originalreviewed0left. Passivecreationflag+publicproxy moved/forced eligibility fixes143 without disabling passivegeometry; explicitchangedfilter invocation/compoundexception/lifecycle/querycontrols PASS. Exactclean startupmerge retained10testedfiles,0newRustbuild;6links/307APIsteps/backend+4original240friction/600supporttests PASS. Final3CPU-first scene reviews143/039/049 done0left;13fresh120stepcells+2unchangedCPUreferences clearhealth onfourGPUroutes. Audit73455terminal0/PASS4.890s/1011pins/163linkedinputs/144media/59absentPIDs; afterinterruption49021terminal0/PASS3.464s/722pins/auditPID3968918absent. ReportREADME.md;gridfinal-visual/compare/index.html;final-evidence/result.json;verified-retention.json. SeparatecapsuleXcase1step8 1e-5responseFAIL unchangedbeforecompletion/generalhullresponse/sensorpositive discovery/162otherfinalscenes/manualcontrols/picking/longwindow/repeats/performance/release remainunqualified. Startupguard preserved; necessarymeshcompile/cacheinvalidation OPEN. AllbudgetsCLOSED/noactivejob;Box3D/WASM/defaults/geometry/IDs/otherlocalworkpreserved;HEADc076092a/origin0ahead0behind afterboundedfetch77951;no newcommitpush/PR09unchecked/goalACTIVE. NEXT BEH03 FarMeshDrop162: readlogFIRST/AGENTS/roadmap andphysicalcriteria; inspectactualCPU/GPU earliestfar meshboundary underfreshfinitebudget, neverresumeoldsolver/scheduling experiments.

BEH01 simple log (local audit archive). BEH01 VERIFIED/CLOSED;10fixed,0resolved,12open. Originalfull165sample survey done0left;twoaffected finalCPU-first comparisons done0left,ten120step cells completed/clearhealth. RetainedWGSL4786c0ca; body-frame freshconvex tangentwrite verified28material/10transportbody/4physicalregressions/12lifecyclecontacts. Audit83547terminal0/PASS11.746s:43successfulproducerops/1252pins/21deps/163linkedinputs/196media/73absentPIDs; auditPID also absent afterterminal. ReportREADME.md,gridcompare/index.html,acceptancefinal-evidence/result.json,verified-retention.json. ColdMeshshader compile130/128s0step exactnativeconstructor;threeoriginal90stimeouts/resourceffprobe/setupfailures retained; fourwarmMesh120step confirms19–25s. Other163finalBEH01solver scenes UNREVIEWED; fullstate/repeats/longcirculation/controls/pickingphysics/coldstartup/FPS/release unqualified. Alljobs terminal/no activecapture;Box3D/WASM/defaults/geometry/identities/otherlocalwork preserved;PR09unchecked/goalACTIVE/no newcommitpush. NEXT BEH02 StaticInvoke143 under distinctfinitebudget;readfullsurveylogFIRST/AGENTS/roadmap/report/relevantsolverhistory;donotresume exhaustedcampaigns.

DIAG01 simple log (local audit archive). DIAG01 VERIFIED/CLOSED. 9fixed,0resolved,13open; original165 survey done0left. All9 CPU-first comparisons reviewed0left;45freshdisplay cells completed,41clearhealth+4retainedStallcapacity failures atphysics4. Finalaudit session65433terminal0/PASS8.880s,1207producerpins/202uniqueunchangedlinkedinputs/21compilerdependencies/5viewers/1240records/1328files/105absentoriginalPIDs; auditPID3258750alsoabsent. ReportREADME.md,acceptanceacceptance.json,gridvisual/compare/index.html. Falseclassification repaired for003/016/060/061/071/125; threeguards053/039/158 checked. SensorGPU30excludes32spawn/freeze; actualzero-stepnativeAPIY211 fixture coversclassification; RUNTIME02/03remainOPEN. Other156finalDIAG01viewer scenes explicitlyUNREVIEWED; all165-name policy audit159unchanged/sixcontractprofiles is source scope,notdynamicqualification. AllcampaignsCLOSED,no capture/build/audit jobrunning;0engine retries. Box3D/WASM/defaults/geometry/IDs/otherlocalworkpreserved;HEADc076092/originfeatgpu0ahead0behind afterboundedfetchsession53019terminal0;no newcommit/push. PR09unchecked,goalACTIVE. NEXT BEH01 ConveyorBelt135/ConveyorMesh136 underdistinctfreshfinitebudget: readfullsurveylogFIRST,AGENTS/roadmap/currentreport then relevantsolverhistory before solverwork. Do notresumeexhaustedcampaigns.

**Historical previous repair recovery (DIAG01 above takes precedence):**

QRY01 diagnosis/report (local audit archive),simple log (local audit archive).

QRY01 VERIFIED/CLOSED. Independent nativeCPU queries and actual production picking setup/cleanup PASS ordinary/native at unchanged tolerances. Final shader0427f784 and host88b72af7;17affected confirmations done/0left,68/68GPUcells completed/clearhealth. One final offline audit session14532 terminal0,CLOSED/PASS2.193s verifies132source/receiptpins,4viewers,1939files and153originalPIDsabsent. Current reportbox-final-report.md, CPU-first gridbox-final-visual/compare/index.html, finalacceptanceacceptance.json. No running capture/build/audit; last158session38838completed allfourroutes and reviewed. Originalsurvey165done0left;8fixed0resolved14open. Earlier full165 candidate gallery remains historical,148shared scenes explicitly UNREVIEWED_ON_FINAL_BOX_CANDIDATE;17direct/helper+guards confirmed. Conditionalcontrols/full draggingphysics/runtime/performance/pixels/trajectories remain unqualified;Box3D/WASM/defaults/geometry/identity/otherlocalwork preserved. PR09unchecked,goal ACTIVE;no newcommit/push. All budgetsCLOSED,0unchangedretries. NEXT DIAG01 false health classification under a distinct fresh bounded unit; read fullreviewlog FIRST,AGENTS/roadmap/currentreport,then relevant physical criteria/history before any solver work. Never resume exhausted campaigns.

The following QRY01/VIS02 histories retain their original next actions and counts; the VERIFIED/CLOSED recovery above supersedes them.

Latest continuation: **PROGRESS120→165 after-reviewed/0 left**,45 individual reviews/180 original GPU apps completed. Process/evidence checkpoint (local audit archive).127 prelaunch and141 media extraction resource failures recovered within distinct closed budgets,0 completed-engine repeats;125 diagnostic flags and camera/control/physics limits retained. Full evidence audit (local audit archive) CLOSED/PASS12.2s,all165 reviews/126 sources/4viewers/660 GPU receipts,652COMPLETED8TIMEOUT. No capture running; fetched remote remains0/0. NEXT resolve1km closest-point residual/applicability and picking applicability before checkbox;QRY01/PR09/goal remain unfinished.


 Overall7 fixed,0 resolved,15 open; original/full candidate165 checked,0 left. [Final material repair report](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS02.md) and acceptance (local audit archive) retain all individual reviews, source/build identities, before/after screenshots, original helper/audit failures and explicitly unverified pixels/unavailable cells.

VIS02 VERIFIED/CLOSED with retained limits. Original survey and final candidate165/165 individual CPU-first before/after comparisons saved,0 left;660/660 after cells dispositioned. Overall7 fixed,0 resolved,15 open. Focused actualCPU/ordinary/native material/clamp/cap/acceptedCCD/sleep/idle/identity checks PASS; original helper failure retained. Final source/object reconstruction audit PASS725 source/object pairs,721 unchanged units,192 linked inputs;550884 actual colour rows/2360 custom passthroughs;825 CPU/before references hashed. Portable29245 saved files/11095964792 bytes/725source records PASS. Audit28.27s,index36.59s,portable35.48s,all180s envelopes CLOSED; no engine/build/check/capture repeats. Original offline assertion failures preserved and contextual/schema corrections separately frozen. Capture646 COMPLETED/12 TIMEOUT/2 CAPACITY_FAILURE;34 runtime/health-limited cells and every pixel/state/occlusion/approximate-progress limit remain explicit. Unavailable cells disposed,not passed; shared rendering cause verified by applicable completed scenes and focused calculations. No physics/query/runtime/picking/performance qualification. All capture/offline PIDs absent; sessions38578/57030/71305 terminal0. Box3D/WASM unchanged; geometry/defaults/identity/picking operations preserved. Fetch2026-10-04 session82530 terminal0,HEAD/originfeatgpu0/0; local work preserved/uncommitted,no newpush. Final acceptance and CPU-first before/after report are metadata/acceptance.json and metadata/README.md. PR09 unchecked,goal ACTIVE. NEXT QRY01 LongRayCast039: read fullsurvey log FIRST,AGENTS/roadmap,metadata current report after continuation/compaction; create/read its simple log,trace actual closest-hit/fraction/point/normal/filter/identity and provenance at unchanged tolerances,and freeze a distinct finite budget before execution. Do not resume any exhausted campaign.

The shared material repair is common to every sample; availability failures do not become rendering/physics passes. CPU-first references and immutable source snapshots are repository-relative, and the portable verifier never reads original absolute build paths. Prior prototype14 comparisons and exhausted frontend/driver/capture campaigns remain historical/CLOSED. No new commit/push. Do one next issue under its own finite envelope; the whole release roadmap is not activated.

The following completed-repair histories retain their original counts and next-action chronology; they do not override QRY01.

**VIS08 VERIFIED/CLOSED.** Overall 6 fixed, 0 resolved, 16 open. Original full sweep:165 done,0 left. Current repair:33/33 individual CPU-first before/after comparisons accepted,0 left;66 after captures and99 reference cells audited. Full165-class source inventory follows qualified inherited Render calls:28 cursor cases+5 guards,all18 original tags retained.

The shared text cursor advanced across two Render calls. Exact before traces show Driving110px, manifold44px; CapsuleMass before images/source show330px including three blank increments. The repair saves the after-Step entry and CPU Render end, starts GPU Render at that entry, then restores CPU end for late UI. HullHull SAT Step label stays first; Step/late Tree UI replay guards pass. Geometry/defaults/identity/picking implementation/Box3D/WASM/Rust/CABI and the prior VIS07 journal remain unchanged by this repair.

One frozen candidate:2main compiles/2links/0archives,66 after30-display captures/90s viewer/310s outer,zero new before/engine retries/Rust/performance. Build33457 and lastcapture90900 terminal0; no livejob. Offline audit24021/index93067/portable95633 terminal0 under one180s process each; all budgets CLOSED.725sources/objects,723unchangedunits,198linkedinputs;282raw Step primitive replays; cursor/RenderXY at1/30 andTree4lateUIpairs PASS. Portable2,815files/566,764,722bytes/725source records PASS, no ignored binaries/engine needed.

[Final report and displayed screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS08.md), all33 CPU-first comparison rows (local audit archive), individual log (local audit archive), final acceptance overlay (local audit archive). Original source matrix/review records preserved; final acceptance supersedes their pending-at-capture labels. CPU120 versusGPU30 progress approximate; shared readouts/tools, conditional/panel/loading/blacktail limits explicit. VIS02/QRY01 and all physics/runtime/picking/performance findings remain open. PR09/fullrelease unchecked. Latest bounded fetch HEAD/origin0/0; all local work preserved/uncommitted, no newcommit/push.

**Historical VIS08 next action (now in progress): VIS02 material/state colors**, all96 original mention scenes (94potential defects,023negative/163intentional-palette guards) plus concrete source-related guards. Read fullreviewlog FIRST, AGENTS/roadmap, then create/read its simple log. Derive full matrix and distinguish intentional backend colors from custom-color/sleep/awake rendering semantics using actual inputs. Freeze a separate finite budget before builds/checks/captures. Do not resume any closed campaign or unrelated solver repair. GoalACTIVE.

Previous completed repair (historical recovery; VIS08 above is current):

**VIS07 VERIFIED/CLOSED.** Overall5 fixed/0 resolved/17 open; original165 done/0 left. Repair verification55/55 individually reviewed,0left;110/110 aftercells and165 reused CPU/before cells audited. [Final report/screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS07.md), all55 CPU-first rows (local audit archive), per-scene log (local audit archive).

One frozen frame-local journal candidate,12viewer compiles/2archives/2links and516transient-input ASAN/UBSAN check passed. ActualStep/replay packedbytes/text space/XY/strings match atdisplayframes1/30;2692primitives checked. World draw excluded and independent; lateTree4screen-string pairs x5/x645 verified with0→8UItext, no extra Step/queries/spawning/events. Fullsource-generation/observer reconstruction PASS;725sources/objects,713unchangedunits,198linkedinputs. CPU/standalone paths and prior VIS03/VIS01/VIS05/VIS06 sources remain applicable. Box3D/WASM/Rust/CABI/scenes/defaults/IDs/picking implementation unchanged. Shared readouts/tools retain provenance; CPUreference120 vsGPU30 approximate progress, panel/conditional/loading/shutdown limitations explicit. 071 two diagnostic flags remain DIAG01OPEN; FarMeshDrop162 BEH03, FarPyramidwatch, VIS02/VIS08 and other physics/runtime/query/picking/performance findings remainopen.

AllVIS07 engine budgets CLOSED;last165session66975/driver975522 terminal0. No further captures/retries. 046/052 completed-engine media interruptions recovered host-only with unchanged clips/receipts. Originaloffline audit53465 schema rejection retained; distinct corrected-media audit91063terminal0 checks frozen recovery hashes. Index35701terminal0; portable verifier50404terminal0/PASS4518files/797808056bytes,725portable source records, no engine/ignored binaries needed. Allsource/media/report checks passed before checkbox. Local work uncommitted; no newcommit/push. Read immutableprotocols/recovery records before acting, never restart closedjobs.

**Historical VIS07 next action (now completed): VIS08 shifted combined GPU text**, all18tagged scenes. Read fullreviewlog FIRST, AGENTS/roadmap, then derive the matrix and trace text projection/routing using the current verified VIS07 baseline. Freeze a separate finite budget before any build/check/capture. Do not claim shared annotations are independent CPU/GPU numeric outputs or resume unrelated solver work. GoalACTIVE; fullrelease unactivated.

The following earlier completed-item recovery is historical; its old “Next” directions do not override current VIS02 next action above.

**Previous completed item: VIS06 VERIFIED/CLOSED.** Overall4fixed/0resolved/18open; original165done/0left,VIS06affectedscene1done/0left. Fiveobserverunits/links/fourarchiveplacements all14operations0, five30stepcapturescomplete1681live sensors/0draws,16810actualshapeobservations; focused14before/14after casesPASS, publicAABB/filter/identity and source/caster uniontraced. All4CPU-first before/after composites actuallyinspected/displayed, progression differencesexplicit. Currentproduction samples_api SHAa24402a0a5ce422c4025702834238ee7b070d14be31f6381f6052d02fb571a05 unchanged fromVIS05; source audit reconstructs observer-only deltas exactly. All723sources/723objects/718unchangedunits/199linkedinputs match. Full165VIS05productionmatrix/priorVIS03terrain/VIS01decoration remainapplicable without rerun.

AllVIS06build/focused/capture/diagnostic/offlinebudgets CLOSED: build86213/capture24378/audit73444terminal0; index279files/44594608bytes andportableverifierterminal0/PASS, no ignoredexecutables/engine needed. Lastcapture474400/build472521 andfocused/auditdriversabsent, no livejob. Originalbefore120timeout/UI32/1721bodies vsafter30pre-spawn/1681bodies retained, no matchedtrajectory/population claim; immutablefocusedCPUinputs establishcause independently. RUNTIME02step32/events/query/physics/picking/performance remainopen. NewpreexistingPR02defaultcategorywatch: standalone shimdefault1 vsCPU/combinedUINT64_MAX, all accepted byALLdrawmask and geometricallyexcluded byZ; default-filter parity unqualified, PR02 reopened independently. No new23rdoriginalrepairgroup counted or solver/default edit. ReadVIS06/log.md/report.

Next: **VIS07**, all28positive/8negative tagged direct primitives/Step overlay findings plus source-related cases. Readfullsweep log FIRST, AGENTS/thisroadmap, then create/readVIS07log; derivefullaffectedmatrix andtraceper-world routing/context atsmallest drawboundary. Freeze its own finitecandidate/build/check/capturebudget beforeexecution; no oldcampaignretry. GoalACTIVE;4fixed/0resolved/18open,165reviewed/0left. No newcommit/push; localworkpreserved atfeat/gpu baselinec076092, fetch2026-10-03 HEAD/origin0/0.


**VIS05 VERIFIED/CLOSED.** Overall3fixed/0resolved/19open. Full initial review165done/0left; VIS05 all165/165 individual repair reviews done,0left,660after/825reused CPU-before cells. Full audit PASS604sources/604objects/162unchanged linked inputs;652completed/8timeouts,28flagged cells retained. Source API SHAa24402a0a5ce422c4025702834238ee7b070d14be31f6381f6052d02fb571a05; exact four ordinary/native standalone/combined binaries are in the report. VIS03 terrain/VIS01 CPU decoration source applicability retained. Box3D/WASM/Rust/scene geometry/defaults/picking implementation unchanged.

All VIS05 diagnostic/focused/build/capture budgets CLOSED; no launches/retries remain. Full audit session44252 terminal0; corrected index18093 terminal0; portable verifier27562 terminal0/PASS20383files/13989353794bytes without engine/ignored executables. Last capture427267/audit432142/index455698/verifier457962 absent; no live job. Initial offline receipt-root failure retained, then a distinct frozen host-only path correction packaged unchanged receipts; no engine campaign resumed. Allfour actually-inspected CPU-first standalone/combined before/after composites are displayed in the linked report. Combined before30/after120 progression explicitly differs; identical-input checks establish cause independently.

Next: **VIS06 Sensor016 hidden geometry**, exact incoming camera/shadow bounds, shape AABB and filter provenance. Existing VIS05 shortguard0draws/1681bodies suggests the same cause but does not close this group; freeze its own finite diagnosis/acceptance budget first. Independent step32 runtime failure remainsRUNTIME02. Read full-sweep log FIRST, AGENTS/this roadmap, then VIS06's log before each job. No old campaign retries or implicit solver work. App goal ACTIVE; PR09/fullrelease remainunchecked. Workspacefeat/gpu atc076092, freshfetch previouslyHEAD/origin0/0; local work preserved, no newcommit/push authorization.

Historical second-repair recovery (VIS05/current counts above supersede): VIS01 VERIFIED/CLOSED;97/97 reviewed,0left;194 after cells and291 reused CPU/before cells audited;6971-file portable verification PASS without engine/ignored binaries. Exact lastPID3588641 absent. No further VIS01 builds/captures/retries. Overall2fixed/0resolved/20open. Next VIS05 diagnosis:014/075/139 enumeration/culling/body metadata; read logs and freeze a distinct finite budget before execution.

Workspace feat/gpu, baseline c076092. Fetch2026-10-03 HEAD/origin0/0; preexisting
roadmap changes and full165-sample evidence preserved. App goal remains ACTIVE:
4 fixed,0 resolved,18 open. VIS03/VIS01/VIS05/VIS06 [x]; PR09/fullrelease remainunchecked.
No new commit/push request. Current changes are local and uncommitted.

[VIS03 report/screenshots](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS03.md)
records the actual cause: factories dropped the original heightfield pointer and
ensure_user_shape rejected its type before the existing adapter callback. Repair
retains immutable geometry/publicID and refreshes replacement metadata. Only
samples_api.c/shim.c/both_dual.c plus focused test changed. Rust/WGSL/Box3D/WASM,
scene geometry/defaults and picking path unchanged; full picking remainsunqualified.
12actual before/after metadata cases pass, four C-only GPU viewers built using
unchanged CPU/Rust/scene/render objects;157 linked input hashes pass. Allseven
confirmed heightfields andthree related constructor/control cases are visually
verified ordinary/native/combined with CPU first; two box/mesh guards retained.

Allbudgets CLOSED:60default captures/12manualreviews;42 distinct UI attempts,
41complete andone original earlyCPU observer failure retained. Total102scene
attempts/101complete. NoRust/performance/unchanged engine retry. Original path/
compile/collector preparation failures remain; corrections freeze only corrected
or never-launched work, with no lost receipts. CPU032 original fixed0.4s assertion
read stale frame0; separate one-process actual-observer confirmation passed.
034 F frame was too distant; initial optimistic review/count corrected and retained.
Distinct T/30wheelout nine-cell overview visibly verifies its unchanged terrain,
without fixing/qualifying F framing. Separate finite protocols/actual inputs/raw
media/compiled sources/full hashes are linked in report. Local offline audit passes; portable verification checks2321 indexed files
without ignored executables or an engine.
No livejob remains; active-job.json CLOSED and exact lastPID3201232 absent.
Read fullsweep log FIRST, then AGENTS/thisroadmap; VIS03 log and interaction-log
preserve individual checks/limits. Do not relaunch closed cells or historical
campaigns. Before any new item freeze its own finite budget and audit applicability.

**Completed second repair: VIS01.** [Report/screenshots and CPU-first grid](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS01.md) records the raw CPU debug-shape name routed through GPU namespace, returning empty instead of ground; adapter selected solid/cell0. Scoped CPU name routing during split/offset CPU draw restores ground-grid/cell1 without geometry/default/ID/world/picking changes. One candidate;12before+12after focused cases;two C-only viewer units/links;194 corrected-display captures/97 individual reviews. Original compile rejection, hull ABI crash andtwo DISPLAY startup failures retained with pre-execution correction protocols. Source audit covers165classes/80positive+10negative mentions+seven additional terrain guards.356source/356object/82unchanged linked hashes,194after/291baseline cells pass; sources preserve priorVIS03 terrain evidence.014 minified grid and039 current panel-hidden terrain are explicit pixel limits, not independent passes. Eight health-warning cells003/060/061/071 retained; Stall still completes physics only throughstep4. No physics/runtime/query/picking/performance qualification. All VIS01 budgets CLOSED, no livejob, no new commit/push.

**Next: VIS07**, direct primitives/Step/control overlays:28positive/8negative mentions and49source-related candidates,55union acceptance scenes. Old29 tally corrected above. Other18groups remain [ ]; PR09/fullrelease unchecked. This is the sequential22-item repair goal only. The separate PR02defaultcategory mismatch remainsOPEN; no whole-roadmap activation. Detailedlogs/reports liveunder experiments/gpu-physics/benchmarks/production-readiness/pr09-repairs-2026-10-02/.
The following recovery is historical, not current job/scope authorization.

## Current recovery record

The block below is a historical archive from before PR09 publication. Current PR09/PR10 completion and recovery are at the top of this file and in the linked PR10 log; these old next actions are not active.

**Historical completed sweep (active repair recovery above takes precedence):** full visual-only C sample sweep, with durable per-scene log; no fixes before the list is complete. Authoritative current sweep state, frozen 825-cell/zero-build/zero-retry budget and findings: [review log](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/native-review.md). Read it first. Historical stop/job prose below applies to closed milestones, not this newly authorized sweep. No additional commit/push instruction for this pass. PR09 remains unchecked. Current sweep COMPLETE:165/165 reviewed/dispositioned,0 pending,825/825 unique capture cells; budget CLOSED with zero builds/tests/retries/performance runs.151 rows reviewed and14 include runtime/diagnostic limitations, not physical passes. No live capture remains; all issues stayed unfixed during the sweep. Confirmed heightfield floor omissions in008/039/041/065/091/096/121; Wave Pile065 best matches the user’s uneven-floor description, identity unconfirmed. Conveyor Belt135 transport, Static Invoke143 passive contact behavior and Far Mesh Drop162 fall-through remain untraced physical/API findings. Far Pyramid163 collapse difference needs setup/physical diagnosis. Detailed findings/evidence/audit and unavailable/browser gaps are in the linked log and README. PR09 remains unchecked. Fix work needs a separate finite calculation-first protocol; do not rerun this exhausted sweep or old campaigns. No new commit/push for this pass. The table below is historical PR04 recovery; current sweep process/cell state lives in the linked log and active-job.json.

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
Current state: repair1660c66 and portable evidence checkpointedcdab2 are pushed
to feat/gpu; both origin0/0 and clean worktree verified. Milestone complete. All25 before
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

Offline evidence/source/status checks pass; CPU-first grid has all45 clips.
Repair1660c66 is pushed and HEAD/origin feat/gpu0/0 is verified. The final tracked
portability audit identified91 indexed logs excluded by Git's log ignore rule;
their exact existing bytes are included in the documentation-only checkpoint.
No code/build/engine/test changes or new physics process. All537 indexed files
and45 clips passed validation from the staged Git archive, without ignored host
artifacts. No live capture/viewer/display job remains. This milestone is complete;
stop. No further item is authorized. Full roadmap remains paused; do
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

HISTORICAL VIS08 checkpoint:4/33 individual comparisons saved,29left;8/66aftercells reviewed,5fixed0resolved17open,original165done0left. Current047 session92356,driver1007253,lastverifiedcaptured-awaiting-human-review; pollsamehandle, next048afterreview. Frozen66cells/90sviewer/310souter,onecandidatebuild completed, no retries/newbefore/solver changes. VIS08unchecked pending full matrix/audit/report/portable.

HISTORICAL VIS08 checkpoint:9/33reviews saved24left,18/66aftercells reviewed,5fixed17open,original165done0left. Current052exec63398/driver1014607,lastverifiedcapturing;pollsamehandle,next067afterreview. VIS08unchecked/fullmatrix-audit-report-portablepending; no extraengine/build/retry/commit/push.

HISTORICAL VIS08 checkpoint:17/33reviews saved,16left,34/66aftercells reviewed,5fixed17open,original165done0left. Current107exec84262/driver1027663,lastverifiedcaptured-awaiting-human-review,next108afterreview. Frozenprotocol unchanged,no extra engine retries/builds/commit/push. VIS08unchecked/fullaudit/report/portable pending.

Current metadata continuation: both actual clamp/accepted host+device CCD inputs now have focused ordinary/native PASS checks, preserving15 actualCPU clamp vectors bit-exactly. First idle test helper failure retained; corrected read-only helper verifies actual idle plus callback lifetime, with no production correction or passingordinary repeats. All14 viewer build commands PASS46.2s, source/binary receipts pinned. Earlier prototype14 comparisons are retained/closed; the final complete-state candidate requires all165 individual comparisons under its distinct660-cell protocol. [Focused report and raw evidence](../../experiments/gpu-physics/benchmarks/production-readiness/pr09-verified-2026-10-15/reports/VIS02.md). Preparation path failure occurred before any engine launch and is retained.

CURRENT repair recovery: VIS02 VERIFIED/CLOSED with retained limits. Original survey and final candidate165/165 individual CPU-first before/after comparisons saved,0 left;660/660 after cells dispositioned. Overall7 fixed,0 resolved,15 open. Focused actualCPU/ordinary/native material/clamp/cap/acceptedCCD/sleep/idle/identity checks PASS; original helper failure retained. Final source/object reconstruction audit PASS725 source/object pairs,721 unchanged units,192 linked inputs;550884 actual colour rows/2360 custom passthroughs;825 CPU/before references hashed. Portable29245 saved files/11095964792 bytes/725source records PASS. Audit28.27s,index36.59s,portable35.48s,all180s envelopes CLOSED; no engine/build/check/capture repeats. Original offline assertion failures preserved and contextual/schema corrections separately frozen. Capture646 COMPLETED/12 TIMEOUT/2 CAPACITY_FAILURE;34 runtime/health-limited cells and every pixel/state/occlusion/approximate-progress limit remain explicit. Unavailable cells disposed,not passed; shared rendering cause verified by applicable completed scenes and focused calculations. No physics/query/runtime/picking/performance qualification. All capture/offline PIDs absent; sessions38578/57030/71305 terminal0. Box3D/WASM unchanged; geometry/defaults/identity/picking operations preserved. Fetch2026-10-04 session82530 terminal0,HEAD/originfeatgpu0/0; local work preserved/uncommitted,no newpush. Final acceptance and CPU-first before/after report are metadata/acceptance.json and metadata/README.md. PR09 unchecked,goal ACTIVE. NEXT QRY01 LongRayCast039: read fullsurvey log FIRST,AGENTS/roadmap,metadata current report after continuation/compaction; create/read its simple log,trace actual closest-hit/fraction/point/normal/filter/identity and provenance at unchanged tolerances,and freeze a distinct finite budget before execution. Do not resume any exhausted campaign.

CAP01 runtime-boundary phase FROZEN2026-10-05:1native observer Rust build180s/jobs2,1Ccompile25s,1link25s,1exact million-tile GPU process16steps/120s,0candidates/captures/retries. Artifact-only observer restores exact production sim/world bytes immediately after build; records existing seven-stage GPU timestamps,160read-only header bytes and existing mirror/CCD/apply profile. Previous failed180s capture retained/closed.12fixed0resolved10open;165reviewed0left;CAP01unchecked0/1. Protocol: `CAP01/runtime-boundary/protocol.json`.

restart-uuid-boundary FROZEN before execution: one audit C compile25s, one actual helper5s, one retained raw cache import5s, zero Rust builds/physics shaders/steps/captures/retries. Check whether UUID changes during actual capability queries; compare current raw import after reported machine restart. Source pins frozen; all prior budgets remain closed.

restart-uuid-boundary CLOSED: actualUUID2df201c9 unchanged at initialproperties/properties2/features2/device; rawimport26MBexact. loader-enumeration-uuid FROZEN:1Ccompile25s/4rawimports5s each/0Rustbuilds,shadercalls,physics,captures,retries. Replayentryversion/layer/extensions prelude beforeunchangeddevice, logprovider+UUID.

loader-enumeration-uuid CLOSED:all4rawcontrols retain26MB/UUID4ce2c594/providerlibnvidia-glcore; entryqueryprelude notcause. loader-provider-route FROZEN:0builds/1actualhelper+1rawcontrol5s each/0shader,physics,captures,retries; VK_LOADER_DEBUG driver routing distinguishes nowconfirmedglcore vs actualeglcoreprovider.

loader-provider-route CLOSED:bothmanifest/physicaldriverlibGLXidentical butfunctionprovideractualeglcore/rawglcore. loader-binding-uuid FROZEN:1Ccompile25s/3rawimports5s/0Rustbuild,shader,physics,captures,retries; isolate libloadingLAZY vsrawNOW/global dlopen flags.

loader-binding-uuid CLOSED:NOW/LAZY/GLOBAL alloldUUID/fullcache. validation-discovery-uuid FROZEN:1Ccompile25s/2rawimports5s/0Rustbuild,shaders,physics,captures,retries; actualwgpu unconditionally queries KHRONOS_validation layerextensions evenwithvalidationOFF, missingfromrawprelude.

validation-discovery-uuid CLOSED/FAILED queryVK_LAYER_KHRONOS_validation unavailable rc-6, retained; noretry. driver-library-load-origin FROZEN:0builds/1actualhelper+1raw5s each/0shader,physics,captures,retries; LD_DEBUG files requester explains differingdriverlibraryload.

uuid-cache-candidate FROZEN before production edit/build:1cache-only candidate;unit+ordinary+native builds180s each,1unit20s,1actualcacheAPIcontrol/backend10s,5C/CPPcompile-link25s max;0physicssteps/captures/retries. Reject mismatch before driverimport, UUIDnamespace, preservecompatiblelegacy. FullTreesprogression/physics/captures require a distinct frozen protocol after cachechecks, PERF01 remainsunchecked. Sourcebackup/pin retained.

display-provider-uuid FROZEN:0builds/3rawimports+1actualhelper5s each/0shaders,physics,captures,retries. Actual factory/helper had DISPLAY=:87; Xvfb absent after restart, current tool DISPLAY=:0 liveXorg. Compare dead/live/unset display at UUID/import boundary before candidatebuilds.

uuid-cache-candidate CLOSED/PASS cachegate only: unitbuild41.545s/unit0.003s, ordinary/nativeartifactbuild24.866/26.768s;3actualcacheAPIcases/backendPASS0.414/0.314s; candidateproduction source restoredexact; zerophysics/captures, PERF01unchecked. live-display-progression FROZEN:0builds/2s live:0 preflight/CPU→ordinary→native original12steps60s each/0captures,retries; retainedclock binaries/unchangedscene andprivate caches. Attributeoriginalfetch costs after nowprovedmissing-display cachecause.

live-display-progression CLOSED:originalCPU12PASS0.064s/ordinary4.272s/native4.172s; meshpipeline141/128ms. ClockinterpretationCORRECTION:cursorpick overwritesQueryProfile.ccd_ms=0 and itswaitincludesCPUmirror, so CCDNOTexcluded. ccd-bound-calculation FROZEN:0productioncandidates/2readonlyclock ABIbuilds180s+2links25s/displaypreflight2s/2GPUoriginal2steps60s/0captures,retries. Measuretarget sweep_bound scan/actualmirrortimings before overwrite; preservefrozenfirstboundinputs, restoreworldexact.

ccd-bound-calculation CLOSED:CCD~177–180ms/devicewait.35–.84ms; targetstaticbound1100calls/33,386,100vertex scans/~32ms perstep exactsame[-125,125]floorbound. Static-bound-candidate FROZEN beforeedit:1exactmemo candidate/1unit+2NORMALABIbuilds180s/2links25s/2existingGPUcontrols120s/2original12stepfactories60s/live:0preflight2s/0captures,retries. Dynamic targets unchanged/per-callcache only/callback ordering preserved; compareoriginal12statechannels andbeforeCPUreference. RemainingCCDcost/fullphysics/finalcaptures remainOPEN.

static-bound-candidate CLOSED/preliminaryPASS:2actualCCDcontrolsPASS, original12-step663initial+state rowsEXACT/backend; fetchmedian189.49/193.28→158.32/155.12ms. Notfinal/performancequalification. Sparse-bound-candidate FROZEN beforeedit:refine onlytemporarymemo storage to visitedtarget HashMap (avoid28MB/1mshape vector). Samefinitebudget1unit+2normalABI180s/2links25s/2controls120s/2original12factories60s/preflight2s/0captures,retries. Changedsourcejustifiesnewchecks; oldbudgetclosed.

sparse-bound-candidate CLOSED/shortgatePASS:3normalbuilds40.994/24.563/26.466s; actualkinematic/filter+joints andhostCCDmetadata controlsPASS6.928/.214s; original12stepsCPU-reference/ordinary3.721/native3.771s with663initial+state rowsEXACT/backend. Default120-confirmation FROZEN:0Rustbuilds/candidates/1CPP+3links25s/preflight2s/CPU12015s+ordinary/native120180s each/0captures/retries. Stablebody/1101shape owners/quaternions/defaultspeed/9floor publicpicks/destroy checks; globalfloorAABB onlygrossfallthroughscreen. SetupTypeError/protocolmissing errors retained before anycompile/GPU; corrected, no execution budgetconsumed. Finalsupport/visualreview stillopen.

Default120-confirmation CLOSED/FAILED: CPU120 PASS0.214s; ordinary/native180s watchdogs after102/103 completed steps, each6/9floorpicks, no final120/destroy gate. Short cache/memo checks remain PASS; full progression not repaired. Do not replay/increase watchdog. No job live. Counts15fixed/1resolved/6open;165reviewed/0left;Trees0finaldone/1left.

mesh-ccd-cost FROZEN before execution:1readonly ordinaryABI180s/1CPP+2links25s/display2s/originalearly2steps20s+NEWseededlate2steps20s/0candidates,captures,retries. Save actual mesh/CCD inputs, candidate/filter/TOI/distance counts and clocks; seeded publicstep102state is NOT replay of originalstep103. Restorequery sourceexact; normalproduction archives retained. PERF01open;15fixed1resolved6open/165reviewed0left/Trees0finaldone1left.

mesh-ccd-cost CLOSED/FAILED:readonly ABI PASS24.963s, seededcompileFAIL wrongVec3/Quat names; NO links/physicslaunched; queryrestoredexact. mesh-ccd-cost-corrected-fixture FROZEN:0Rustbuild/1correctedCPP25s+2unlaunchedlinks25s/preflight2s/early+NEWseeded2steps20s each/0candidate,capture,retries. Correctb3Vec3/b3Quat,reuseexactartifactarchive.15fixed1resolved6open/165reviewed0left.

mesh-ccd-cost-corrected-fixture CLOSED/attributionPASS:compile1.366s/links1.466each,originalearly2steps8.181s/newseeded2steps15.747s. Actual2200CCDcalls/fixture;early5,975,050candidates allcentroid-plane rejected;seeded7,067,078candidates/6,033,658TOIcalls/4.003sTOI+1.255sdistance+7.708soperation,0hits. Observerwriting5.8s/fixture separately excluded. Frozen30351vertices/60000triangles/16383nodes plus4400actualCCDinput/output rows retained. queryrestoredexact; no job live. NextCPU-only conservativebound proof/replay;PERF01unchecked,15fixed1resolved6open.

mesh-bound-proof FROZEN:1CPUunitbuild180s/actual4400frozeninput TOI+enclosure proof40s/existingmeshcontrols10s/0GPUdevices,productioncandidates,captures,retries. Artifactrotation-envelopeincludesoriginalCOMframe/TOIshell/centroidretry, nearpi/antipodal/normrounding/largeCOM controls; fallbacklegacyforinvalidinputs. Restorequery+query_toi exact. No PERF01closure;15fixed1resolved6open/165reviewed0left.

mesh-bound-proof CLOSED/artifactPASS:unitbuild42.046s/frozenactual4400TOI inputs0.515s/existing4meshCCDtests0.003s. Early5,975,050→0candidates;seeded7,067,078→7,862;actualTOI outputsEXACT;3,432,000sampledactualnlerp/rotatepoints enclosed/90norm-antipodal-nearpi-largeCOM cases/positiveBVH hit retained. Prototypeonly,query+query_toi restoredexact, noGPU/capture/closure.15fixed1resolved6open.

mesh-bound-candidate budget FROZEN beforeproductionedit:1conservativeCCDquery-bound candidate/1unit+2NORMALABIbuilds180s/2links25s/boundCPUunit10s+4existingmeshCPUcontrols10s+2actualGPUCCDcontrols120s/live:0preflight2s/ordinary+nativeoriginal120steps60s each/0captures,retries. OriginalCPU120 reusedunchanged. PreserveTOImath/leafrelativeorder/centroidretry/shell/geometry/defaults/IDs. Finalphysical/visualreport remainsopen;15fixed1resolved6open.

mesh-bound-candidate CLOSED/shortproductionPASS:unit41.445/normalordinary25.113/native26.466s; bound+existingmeshCPU controls and2actualGPUCCD controlsPASS; originalordinary/native120steps4.222/4.022s,9floorpicks/defaultspeed/quaternion/1101shapeowners/destroy checksPASS. Raw retained before102/103state-row comparison inresult.json; local-support/longerphysical/final5routevisualreport stillopen.15fixed1resolved6open.

physical600 FROZEN:0Rustbuild/candidates/1CPP+3links25s/display2s/actualCPU60015s→ordinary+native60090s each/1CPUofflineanalysis60s/0captures,retries. Fulloriginaldefault50trees/full60000trianglewave retained. Publicgeometry/mass exports; all600finite/IDs/speed/norms/45floorpicks/destroy; totalmechanicalenergyallsteps+facetedhull-triangleSAT checkpoints0/120/300/450/600 at existing.005slop (no toleranceincrease). Before102/103 rows EXACT5253/5304. Finalvisualreport pending,15fixed1resolved6open.

physical600 enginegate CLOSED/PASS CPU0.564/ordinary14.544/native13.592s,all600default/IDs/45picks/destroy. OfflineanalysisFAILED assertion wrong same-order hull assumption:CPU shapeenumreversed; geometry multisetEXACT vertices/planes/edges/floor. RawGPU SATmax.003611721/energyPASS,stockCPU SATmax.093727081 retained. CorrectedCPU-only30sanalysis FROZEN; no engine rerun/toleranceincrease, GPUacceptance separateCPUbaselinefailure.15fixed1resolved6open.

final-viewer-links FROZEN:0build/candidates/Ccompiles/GPUprocesses/captures;4links25s each,0retries. Checkednormalordinary/native archives+unchangedverifiedC/sampleobjects; actualCPUviewer retained. No finalvisualpassinferred;15fixed1resolved6open.

physical600 correctedanalysis CLOSED/GPUphysicalPASS13.541s: all600energy≤initial,facetedSAT checkpoints0/120/300/450/600 max.003611721≤.005 forbothGPU;49asleep/1awake at600. StockCPU transientSAT.093727081 at120/body23 retainedasbaselinefailure, no Box3Drepair/toleranceincrease. Floor30351vertices/60000triangles/all22hulls EXACThullmultiset; CPUenumerationreversed. Final4viewerlinksPASS1.466–1.617s. final-visual FROZEN:0build/candidate/retry; liveXvfb10s+2zero-physics/cache-importpreflights5s; abortbeforecellsifcacheincompatible;5CPU-first120stepcells120s each/5stages percell/ffprobe-extraction15s each. ActualcachedNVIDIA compute/softwaredisplay, noFPSclaim.25beforestages retained; no reviewinferred.15fixed1resolved6open/Trees0finaldone1left.

007 PERF01 BEFORE25stages visuallyreviewed:CPU~20→115; ordinary/native~22/23→68, combined~18→62 withCPUleft/GPUright. Fullwavefloor and50compoundtrees visible; slowprogress/fetch~1.7s notliteralfreeze/missingfloor. Intentionaltint/panelandupper-tipclipping retained. AFTER pending,no[x];15fixed1resolved6open/165reviewed0left/Trees0finaldone1left.

007 PERF01 AFTER25stages visuallyreviewed; all50before/after stages complete. Fullunevenfloor/50compoundtrees onall5routes; actualCPUfirst/CPUleftincombined; clearhealth120allcells,120000p/q/v/wchannelrecords EXACTperbackendfactory. No freeze/missinggeometry/grossfallthrough; intentionaltint/panel/tipclipping retained. CPU44.731/ordinary52.471/native53.106/combined77.012/77.310s softwarecaptures, noFPSclaim. Finalreview1done0left; closureaudit/report pendingbefore[x]. Counts15fixed1resolved6open.

Finalclosureaudit FROZEN:1CPU-onlyaudit60s/0GPU,build,candidate,capture,retries. Verifycheckedsource/archive/viewer/media/CPU119source-objectproofs/allhistoricalterminalPIDs before[x]. Report+50stages reviewed; counts15fixed1resolved6open untilauditPASS.

PERF01 CLOSED/VERIFIED afterfinalauditPASS5.591s:228pins/119actualCPU source-object pairs/195media/293absentPIDs/54terminalphases. Reportbefore/after+all50stages published;[x]PERF01,PR09unchecked. Counts16fixed1resolved5open/165reviewed0left/Trees1finaldone0left. Nojobs or additionalcommitpush. NEXTPERF02 withfreshfinitebudget; other164candidate scenes explicitlyunreviewed.

PERF04 reboot continuation2026-10-06: 2026-10-06 restart recovery: no old capture/build/GPU processes present; RTX4070SUPER NVIDIA610.57.04 available. All completed evidence and localwork intact. affected-visual/protocol.json FROZEN before execution:38 runnable rows×9CPU-first before/after cells=342max,30steps,90s viewer/30shealth/15smedia/10sdisplay/cachecopy,0builds/links/retries. Exact9 binaries verified against successful producer receipts and Rain capture receipts; stale prior protocol metadata preserved and corrected only in new protocol. Source correction:013 active classline994 CreateLargeWorld, disabled duplicate Rain classline159 excluded;014125136 helper comments/#if0 are negativeguards;084 AddJoint is conditional/unexercised. Original40union retained. 013 old five-stage sheet actually inspected again: known capacity-unavailable exclusion, no candidate launch/pass/retry. Counts18fixed/1resolved/3open; original165done0left; candidate1actualreview+1unavailable=2dispositioned/38left. NEXT002Chains, readlogFIRST and inspect actual sheets before counting.

affected-visual002Chains LAUNCHED once exec23588/driverPID3259040 under1800s outer limit; state.json owns exact child operations; no prior job restarted. Individual90s watchdog/0retries. Fresh fetch20s completed HEAD/origin0ahead0behind. Poll same handle before continuing.

Restart checked2026-10-06: GPU RTX4070SUPER/610.57.04 available; no previous Rain/cargo/Xvfb processes observed. joint63-full-ccd-boundary/protocol.json FROZEN before execution:2read-only ABI builds60s/2links25s/2actual unchanged Rain120 runs30s/2cachecopies10s/helper5s/display10s/outer360s/0physics candidates,captures,retries. Capture actual105-107 pre/post/start endpointstates, selected triangles and TOI input/root intermediates; retain LIFETIME_TRACE=1. All three production source files guarded/restored aftereachbuild; artifactlookup candidate only. Counts18fixed1resolved3open;165reviewed0left;affected39reviewed1unavailable0left. No old budget resumed.

joint63-full-ccd-boundary LAUNCHED once exec85433/driverPID3792154; state.json records every child and watchdog. Poll SAME session; no unchanged retries.18fixed1resolved3open;165done0left;candidate39reviewed1unavailable0left.

joint63-full-ccd-boundary exec85433 TERMINAL0:2observer builds26.018/27.417s,2links1.617/1.667s,ordinary/native120 runs10.283/10.134s; allPIDs absent; all3production sourcefiles restoredexact. Actual105-107 fullCCD inputs saved; no solver/default/identity modification or fix retained. Next frozen30s completepublic-record/trace analysis, then actualCPU identicalinput TOI comparison.18fixed1resolved3open;165done0left;39candidate reviewed1unavailable0left.

Continuation PROGRESS: previous actualCPU identicalCCD inputs/posewrites ruledout CCD arithmetic; new joint63-point-row/protocol.json FROZEN beforeexecution. 1ordinaryread-onlyABI60s/1CPP25s/1link25s/1privatecachecopy10s/helper5s/display10s/newshader1defaultRainstep60s/actualunchanged120steps30s/outer240s/CPUoffline30s;0physics candidates/native runs/captures/retries. Trace only actualknee pointfragment at107 (before/aftervelocities,levers,inertia,massmatrix,softness,solution,impulse), originalscene/defaults preserved. Suppress unrelated diagnosticlogging beforeboundedappend. Publicbody/joint records includingcreation mustmatch prior120 beforecausal inference. Restoreworld/solve/collide sourcesexact. AlloldbudgetsCLOSED; Rainphysics stillFAIL;18fixed1resolved3open/165done0left/candidate39reviewed1unavailable0left.

joint63-point-row LAUNCHEDonce exec97834/driverPID3825048; state.json owns children/watchdogs. PollSAMEhandle; no unchangedretry.18fixed1resolved3open/165done0left/candidate39reviewed1unavailable0left.

joint63-point-row exec97834 TERMINAL1/FAIL_RETAINED:observerABI28.873s/CPP1.266s/link1.566s PASS; shaderprewarm watchdog60.114s/-15 before firstStep/staticpick completion, no107row output andactual120 neverlaunched. Compatiblecachehelper86/.364s; processobserved42s/99.6%CPU duringcompile/preparation, no per-pipeline trace so exactsubstage unproved. AllproducerPIDs absent/world+solve+collide restoredexact. BudgetCLOSED/no unchangedretry or timeoutextension. NEXTdistinct shader-unchanged Rustcommand-buffer copies aroundjoint-color dispatches at107, preserveexactdispatchorder andrequirefullpublic-state equality.18fixed1resolved3open/165done0left/candidate39reviewed1unavailable0left.

Restart recovery: no old Rain/build/Xvfb processes; RTX4070SUPER NVIDIA610.57.04 available, feat/gpu c076092 and local work preserved. joint63-point-row is CLOSED/FAILED: shader warm-up reached60s before firstStep, no joint output; no retry. joint63-dispatch-copies/protocol.json FROZEN BEFORE EXECUTION: one Rust-only ordinary observer build60s/link25s/cachecopy10s/helper5s/display10s/unchanged defaultRain120 process30s/outer150s/offline30s; zero shader changes, CPP compiles, native runs, physics candidates, captures or retries. Copies576 selected joint/body records around existing step107 dispatches. Require exact complete public body/joint state equality before causal interpretation. All previous budgets closed; previous exec51296 TERMINAL1; corrected diagnostic exec84313/analysis31762 TERMINAL0; exact publicstate PASS. 18fixed1resolved3open;165reviewed0left;candidate39reviewed1unavailable0left.

joint63-dispatch-copies LAUNCHED ONCE exec51296/driverPID3851810; state.json owns exact children/watchdogs. Poll SAME session, no relaunch.18fixed1resolved3open;165reviewed0left;candidate39reviewed1unavailable0left.

joint63-dispatch-copies exec51296 TERMINAL1: helper86/.314s PASS; ordinary observer compile6.677s FAIL E0599 feature-gated read_diagnostic_body_extras; zero links/physics/captures. AllPIDs absent andworld/sim exactrestored. Failure/budget CLOSED. Distinct joint63-dispatch-extras-copy FROZEN beforeexecution, same onebuild60s/link25s/cache10s/helper5s/display10s/actualordinary12030s/outer150s/offline30s, zero shaders/features/candidates/captures/retries; onlyextras helper replaced with actual two80byte buffer copies.18fixed1resolved3open;165done0left.

joint63-dispatch-extras-copy LAUNCHED ONCE exec84313/driverPID3854315; poll SAME session; state.json owns exact children.18fixed1resolved3open;165done0left.

joint63-dispatch-extras-copy exec84313 TERMINAL0:build25.865s/link1.516s/cache.032s/actualordinary12010.084s PASS;576actualjoint/bodycopy records368640bytes plusactual160byte extras/layout. AllPIDs absent/world/sim exact restored; no physics candidate retained. Reservedone offline30s comparison FROZEN beforeexecution; exact publicstate and typedidentity acceptance pending.18fixed1resolved3open;165done0left.

joint63-dispatch-extras-copy offline exec31762 TERMINAL0/PASS2.946s:134400completebody INCLUDINGcreation and134400joint records EXACT;576actual typed records/12selectedknee operations/identity162163kind1color0 verified. Capture/build/offline budgets CLOSED. Distinctcpu-joint-row FROZEN:1actualCPU Ccompile25s/link25s/run5s/outer65s/offline30s;12actualnativewarm/solve calls on frozenexactinputs, noGPU/Rust/candidate/capture/retry; unchanged1e-5 screen.18fixed1resolved3open;165done0left.

joint63-dispatch-extras-copy allphasesCLOSED/PASS:actualCPU all1596inputwords/324outputscalars float32-bitEXACT;134400completebody+jointpublicrecords EXACT, allPIDs absent/worldsim exactrestored. Selectedknee warm/solve/relax arithmetic ruledout on12operations. Failures preserved/physical107326stillFAIL/candidateARTIFACT_ONLY;reportactualCPU-firstold30stepbeforeafterimages.18fixed1resolved3open;165done0left. NEXTinspectexistingcopiesaroundcontact/otherjoint changes beforeanynewbudget; no nextjob launched.

Interrupted continuation: the recovery-file read tool call returned no output for10611.7s until user interruption; classify previous turn NO PROGRESS, not a verified wait. It requested only sed/cat reads, no build/physics/capture. Exact transport/shell stall cause unproved. Current short tool calls respond again; recorded diagnostic producer PIDs absent, filtered process check found no cargo/rustc/Xvfb/viewer/sed/cat. Add5s shell timeouts to routine reads and return/poll short sessions; shell timeout does not guarantee tool transport deadline. No experiments restarted or budgets extended.18fixed1resolved3open;165reviewed0left. NEXTinspect saved surrounding dispatch records under new finite offline budget.

Prior continuation NO_PROGRESS: recoveryread tool stalled13824.2s andwasinterrupted; no experiment launched. Current transientcodehost EAGAIN prevented firstfreeze command fromstarting; absenceofdirectory verified, no budget consumed. Shortsinglecall1000msyield nowresponds. joint63-surrounding-dispatches FROZEN BEFOREexecution:oneoffline10s analysis576savedcopies/0buildGPUCPUcandidatecapture/retry.18fixed1resolved3open;165done0left.

Surrounding dispatch offline budget CLOSED/PASS.037s:288joint/276isolatedcontact intervals;24jointchanges(12selected+12otherjoint),only2contactchanges bothsmallangularvelocity atsub0warm/relaxcolor1.12lastcolor22contactintervals notcaptured, nocontactidentity/linearresponse inference beyondobservedintervals. Distinctjoint63-contact-boundary FROZEN BEFOREexecution:oneordinaryRust60s/link25s/cache10s/helper5s/display10s/unchangeddefaultRain12030s/outer150s/offline30s/max128MiBraw/0shaderfeaturescandidatesnativecapturesretries. Copyactual107prepared/endTGScontacts+hotbodies+sceneheap; fullpublicstate mustEXACTbeforeinterpretation. Allpriorbudgets CLOSED.18fixed1resolved3open;165done0left.

joint63-contact-boundary LAUNCHED ONCE exec13291/driverPID117497; poll SAME handle, state.json owns childwatchdogs.18fixed1resolved3open;165done0left.

joint63-contact-boundary exec13291 TERMINAL0:helper.264/build23.011/link1.216/cache.032/actualordinary12011.991s PASS. AllPIDs absent/worldsim exact restored/106pins verified. Reservedoneoffline30s fullpublicstate/actualcontactinput decode FROZEN BEFOREexecution; no physics repair or rerun.18fixed1resolved3open;165done0left.

Continuation PROGRESS:actualcontactboundcapture/publicrecordexact identifiedterraincontact677/color22 inprioruncapturedlastcolor;normal190.356 response present, notmissingfloorcontact. Newflooroperationtarget selected; raw/evidence/reports preserved;allbudgetsCLOSED/nojob.18fixed1resolved3open;165done0left.

Continuation PROGRESS:prioractualflooridentity/boundary changednextaction. floor677-operation FROZEN BEFOREexecution:1ordinaryABI60s/link25s/cache10s/helper5s/display10s/actualdefaultRain12030s/outer150s/offline30s;0WGSL/features/candidates/native/captures/retries. Copybody1/163+contact677/color22 beforeafter12actualwarm/biased/relax operations; exactfullpublicstate required beforeactualCPUreference.107pins.18fixed1resolved3open;165done0left.

floor677-operation LAUNCHED ONCE exec87928/driverPID139015;state.json ownschildren/watchdogs;pollSAMEsession/no relaunch.18fixed1resolved3open;165done0left.

floor677-operation exec87928 TERMINAL0: ordinary Rust build22.560s/link1.266s/cachecopy.064s/originalRain12011.489s PASS; driver and all recorded child/Xvfb PIDs absent; world/sim restored exact ee0c8bf5/58382438. Captured576x944 bytes with24selectedfloor records and actual params/extras/layout. Reservedoffline analysis notyetexecuted. No stuck build/simulation; no physics repair proven.18fixed1resolved3open;165reviewed0left.

floor677-operation reservedoneoffline30s analysis FROZEN BEFOREexecution: exact134400fullbody+134400joint records, actualRustlayout24selectedrecords/12flooroperations, identity/params/extras;0buildGPUCPUcandidatecapture/retry.18fixed1resolved3open;165done0left.

floor677-operation ALLPHASES CLOSED/PASS: exec87928 capture andexec28826 offline terminal0;134400fullbody+134400joint records EXACT,24actualfloor beforeafterrecords/12operations decoded,114pinsverified/allPIDsabsent/worldsim exactrestored. ActualCPUcontactreference remainsNEXT/unlaunched; no repair yet.18fixed1resolved3open;165reviewed0left.

floor677-operation/cpu-contact-row FROZEN BEFOREexecution:1actualCPU Ccompile25s/link25s/run5s/outer65s/reservedoffline30s,12actualMeshwarm/solve calls on frozenexactinputs;0GPUbuild/process/candidate/capture/retry. Preparedmasses/softnesssupplied, pairedGPU vssequentialCPU difference retained;unchanged1e-5screen.18fixed1resolved3open;165done0left.

floor677-operation actualCPU reference TERMINAL0:compile.315s/link.016s/run.002s,12actualnativeMesh operations; producerPIDsabsent. Reservedoneoffline30s comparison FROZENbeforeexecution:1776inputwordexactness/264outputscalars/unchanged1e-5screen,0GPUCPUbuildcandidatecaptureretry.18fixed1resolved3open;165done0left.

ActualCPUcontactcomparison CLOSED:1776inputwordsexact,222/264outputsbitexact;6impulsescalarabsolute1e-5screenfailures retained, each1ULP; max1.2207e-4 cumulativeimpulse, allbodyvel/omegadiffs<=4.2916e-6. Newpaired-model offline10s FROZENBEFOREexecution/0buildGPUCPUcallcandidatecaptureretry; classifyalgorithmicexpression branch fromsavedinputs withoutclaimingactualintermediateWGSLcapture.18fixed1resolved3open;165done0left.

paired-model offline CLOSED.0036s: nativeprestate/f32model givesall8onlypoint0branches,14/16GPUcachednormal valuesbitexact/8scalarCPUpoint0exact;remaining2model values1ULP mismatchretained, no actualWGSLintermediateproof. Distinctcpu-contact-prepare FROZENBEFOREexecution:1Ccompile25/link25/actualnativeMeshprepare5/outer65/offline30s/0GPUcandidatescaptureretry.18fixed1resolved3open;165done0left.

cpu-contact-prepare actualnativecall TERMINAL0:compile.365/link.016/run.002s. Reservedoneoffline30s comparison FROZENBEFOREexecution:129inputwords/46preparedoutputscalars/unchanged1e-5screen;0buildGPUCPUcallcandidatecaptureretry.18fixed1resolved3open;165done0left.

ActualCPUprep comparison TERMINAL0/PASS.0099s:129inputs/46preparedscalars float32-bitEXACT. NewCPUevidenceclosureaudit oneoffline10s FROZENBEFOREexecution; no builds/GPUCPUphysics/candidates/captures/retries.18fixed1resolved3open;165done0left.

Continuation PROGRESS: actualnative contactoperations compared1776exactinputs/222of264bitexact/6oneULPscreenfailures retained; actualnativeMeshpreparation129inputs/46outputs bitexact. Offlinepairedmodel14of16match,firstalgorithmic expression difference explained withlimits; no physicalrepair. AllnewbudgetsCLOSED/closure246pins19absentPIDs PASS; nextmanifoldgeneration/cacheboundary.18fixed1resolved3open;165reviewed0left.

Continuation priorPROGRESS revalidated: actualCPUprep46bitexact/solver6screenfailures retained; no livejob. floor677-manifold extraction FROZENbeforeexecution oneoffline10s/max128MiB/0buildGPUCPUcandidatecaptureretry. Decodeactual471vertices/originalcapsuleendpoints/poses/features; life11norecycled andFAST32768/continuous provesfreshmesh.18fixed1resolved3open;165done0left.

Resumedafterlock: priorabortedcommand onlyrg sourceconstants read, noCPUcollisionrun launched. Geometryextraction CLOSED/PASS.254s, PIDabsent,471vertices/endpoints/origins/rotations andfreshFAST/lifeflagsverified. ActualCPUtrianglecapsule FROZENBEFOREexecution:1Ccompile25/link25/run5/outer65/offline30s,0GPUcandidatecaptureretry;36inputs,featuremappedgeometry+pointorder retained.18fixed1resolved3open;165done0left.

floor677-manifold actualCPUcollision TERMINAL0:compile.215/link.032/run.002s; allproducerPIDsabsent. Reservedoneoffline30s comparison FROZENbeforeexecution:36inputs/twofeatureIDs/order/17geometryscalars/unchanged1e-5screen,0buildGPUCPUcandidatecaptureretry.18fixed1resolved3open;165done0left.

Angular observer COMPLETED17.550build/1.416link/9.633Rain120; all5producerPIDs absent, production sourceexactrestored. Extraction FROZEN1offline30s/0retry/build/engine/capture beforeexecution; 36traces and full134400body+joint transparency required.18fixed1resolved3open;165reviewed0left.

Angular extraction PASS2.494s/terminal0:full134400body+134400joint arrays EXACT;36events,24sphericalcases frozen, finalpose words tied to actualhealth identities. Native FROZEN1compile25/link25/run5/outer65/offlinecomparison30;0GPUbuild/run/newcandidate/capture/retry; actual native geometry and all984inputwords/24flags required.18fixed1resolved3open;165reviewed0left.

Spherical43-dispatch FROZEN beforeexecution:readonlyselected43/45 GPUjoint input/output copies at106–107, unchangedshader/order onrejectedartifact.1ordinary static90/link25/Rain12060/display10/outer200/offline30;0newcandidate/native/capture/retries. Full134400body+joint publicarraysmustremainexact. Setupgeneration errors retained, no build/run consumed.18fixed1resolved3open;165reviewed0left.

Spherical observer launchedonce exec42627/driverPID907404;state.json authoritativechild/display, pollSAMEhandle;0retries. Source temporary world+sim guardedexactrestoration.18fixed1resolved3open;165reviewed0left.

Spherical observer exec42627terminal0/17.350build+1.417link+9.032actual120; two737280byte buffers with48selectedoperations, source world/simexactrestored. Extraction FROZENone30s/0retry beforeexecution; completepublic arrays/endpointgenerations required.18fixed1resolved3open;165reviewed0left.

Spherical extraction PASS2.267s/full134400body+joint exports EXACT,48selected operations at106–107/public43,45/generation2. Native FROZEN inputtranslation5/compile25/link25/run5/outer65/comparison30 oneeach;0GPUbuild/run/newcandidate/capture/retries;actual unchanged Box3D prepare/warm/solve,6528inputword echoes and1152outputscalars at unchanged1e-5 required.18fixed1resolved3open;165reviewed0left.

Spherical43 phase CLOSED/PASS493pins/12absentPIDs2.158s;actualnativeall1152outputsbitexact/6528inputs;physicsfailuresretained. Distinct post-joint-angular-rate FROZENextract30/compile25/link25/native2304records5/outer90/analysis15/audit30 oneeach;0GPUbuild/run/candidate/capture/retry. Useexistingrawcopies/nativepreparedaxis toattributevelocityprojection changes; lastcontactcolorunobserved immediateafter; noinstantaneousangle/shaderintermediate claim.18fixed1resolved3open;165reviewed0left.

Neighbor-warm-dispatch FROZEN beforeexecution:actual8neighborjoint GPUcolors/caches/endpoints at106–107; oneordinary static90/link25/registeredRain12060/display10/outer200/extract30/input5/reusednative48warmcases5/compare30/audit30;0nativecompiles/links/GPU/newcandidate/capture/retries. Native targets44,46,48,50,52,54;43/45 passingcontrols reused. Full134400body+joint transparency andgenerations required.18fixed1resolved3open;165reviewed0left.

Neighbor observer launchedonce exec21134/driverPID910811;state.json authoritativechildren/display;pollSAMEhandle;0retries,newpolicyqualifications. Source world+simtemporarilyinstalled/guardedrestoration.18fixed1resolved3open;165reviewed0left.

Neighbor observer COMPLETE exec21134terminal0:18.152staticbuild/1.516link/9.633Rain120, sources worldee0c8bf5/sim58382438exactrestored. Extraction FROZEN1offline30/0retry;192jointoperations/fullpublicstate/colors/generations.18fixed1resolved3open;165reviewed0left.

Neighbor extraction PASS2.988s/full134400body+joint records EXACT;192ops actualcolors43/45=0,44/46/48=1,50/52=2,54=3 bothsteps/gen2. Native FROZENoneinput5+oneexistingreference48warmoperations5/outer15,0compiles/links/43-45rechecks/retry.18fixed1resolved3open;165reviewed0left.

Neighbor native48warmoperations PASS6528inputwords/1152outputsbitexact,terminal0/.114input+.008native+.009comparison;0compiler/GPUrepeats. Cache provenance FROZEN distinct1offline15/0native/GPU/build/capture/retry beforeexecution:actual8joint cachechain and color1 writerassociation; earlierselectedcontrolraw segment transparency required.18fixed1resolved3open;165reviewed0left.

Neighbor point-intermediates FROZENbeforeexecution:4nativecases44/48 at107/sub0relax+sub1warm; readonlycopiedupstreamC intermediateprints, originalsourceunchanged.1input5/2Ccompiles25/1link25/1run5/outer90/comparison15/audit30;0GPUbuild/run/newcandidate/capture/retry. Totaloutputs/inputwords must matchactualGPU beforeinterpreting prints.18fixed1resolved3open;165reviewed0left.

Four nativepoint/warm observers COMPLETED terminal0:input.032/helpercompile.265/copiedupstreamcompile.565/link.032/run.001s,0GPU/sourcechanges. Comparison FROZENone15s/0retry;544inputwords/96actualoutputscalars plus30tracebindings and48priornativewarm outputs required.18fixed1resolved3open;165reviewed0left.

PERF05 render-operation-observer FROZEN before execution:24 operations/251pins;5 hookcompiles+5 renderercompiles+5links25s each,4cachecopies10s,CPU-first5 originalWasher120 processes90s each,displayready5s/outer600s,oneofflineanalysis20s;0Rust/newcandidate/capture/retries. Named normalPERF03 archives; readonly host timing with no addedGPUwaits and per-frame flushed prefixes, no physical/render-call changes. Allold90s captures remainclosed.18fixed1resolved3open;165reviewed0left.

PERF05 observer launched once:exec3404/driverPID915984/XvfbPID916003;render-operation-observer/state.json owns operationPIDs. Poll same handle;no observation-timeout retries.18fixed1resolved3open;165reviewed0left.

PERF05 observer CLOSED/PARTIAL:exec3404terminal1/driver915984 allproducerPIDsabsent. CPU12050.075s/native12057.202s complete;ordinary90.115s TIMEOUT117validprefixes. Hostrender sums45.928/45.063/42.412s andphysics.518/5.559/43.509s (ordinary117). No physical/render-call changes. Splitlink failed because preservedVIS07counter vis07_renderer_counts missing from production-source copy;nm old/new single missing symbol, duplicate archived renderer extracted. Bothsplit runs unlaunched. Original failure retained. Offlineanalysis20 CLOSED/PASS1.124s diagnosis, no rawfailure waived.
Distinct split-symbol-correction FROZEN9ops/260pins:appendexact existing readonlycounter/externdecls;2renderercompiles+1nativehook+2links25s,2copies10s,2previouslyunlaunchedsplitWasher12090s,displayready5/outer300/offline20;0CPU/standaloneGPU reruns,Rust,candidates,captures,retries.18fixed1resolved3open;165reviewed0left.

PERF05 correctedsplit observer launchedonce exec23176/driverPID925253/XvfbPID925279;corrected state.json authoritative. Pollsamehandle,zero retries;standalone campaignclosed.18fixed1resolved3open;165reviewed0left.

PERF05 step-profile-boundary FROZEN beforeexecution:8ops/260pins;2hookcompiles+2links25s,2copies10s,2actualregisteredWasher80 cells90s/outer240/displayready5/offline20;0CPU reruns/renderer orRustbuilds/candidates/captures/retries. Reusefirst80 prefixes of pinnedrealCPU120 evidence first. Read actualcheap encode/fetch/device/timestamp getters afteroriginalcompletedwait, beforequeries; no newwait/physics/render-call changes. Shortattribution,notrepair/120closure.18fixed1resolved3open;165reviewed0left.

PERF05 step boundary launchedonce exec46501/driverPID934666/XvfbPID934686;step-profile-boundary/state.json ownsPIDs. Poll same handle; no observation-timeout retries.18fixed1resolved3open;165reviewed0left.

PERF05 washer-factory-check FROZEN before execution2026-10-07:9ops/306pins;1CPPcompile25s/3links25s/2cachecopies10s/CPU-first3actualregisteredWasher120+2smallcontrolworlds16steps perbackend60s/outer330/offline30;0Rust/newcandidate/captures/retries. Preserve all8002body/8041shape defaults/geometry/identities,12factory+4control floorpicks anddestroy/stale invalidation. Analyticalvertex violations recorded unchanged. Candidateheldarchives/sourcebaseline staysrestored, no[x].18fixed1resolved3open;original165reviewed0left;candidatevisual0done165pending.

PERF05 factory fixture compile FAILED1.216s/exit1 fieldmaterial vsactualbaseMaterial;9opprotocol closed after1compile, all3enginecells unlaunched, exec18590terminal1collected/957467+957516absent. Originalsource/protocol/error retained. New washer-factory-material-field-correction FROZEN9ops/311pins with only2fieldname changes; same120+16steps/60scells/330souter/0Rust/newcandidate/captures/retries.18fixed1resolved3open;original165reviewed0left.

PERF05 factory confirmation CLOSED/PASS2026-10-07: correctedfixture9ops all0, exec4159terminal0collected;actualCPU120+16controlsteps.615s/ordinary48.061/native38.789s.311pins/8producerPIDsabsent, finaloffline1.697s/exec44227terminal0collected. All8002body/8041shape defaults/initialposes/hulls matchCPU, all120static+kinematic publicstates CPUexact;0analyticalcube groundvertex violations, minbottomCPU4.961/ordinary5.008/native5.021 (no longmixing/settledground/drumSAT claim). Eachroute16publicfloorowner picks, transform/shape/worldrecreate+staleIDs/destroy PASS. ControlscenterYminus.2 proxy notallrotatedvertex/substep qualification. Originalfixturecompilerfailure and twoofflinearrayorder/worldgeneration assumption failures retained; no engine reruns. Backendactualowners/IDs enforced, geometrycomparedbycreationindex ratherthanfakingcrossengineworlddomains. No sourceinstalled/retained, no newcaptures/[x].18fixed1resolved3open;original165reviewed0left;candidatevisual0done165pending.

PERF05 washer-default-visual FROZEN2026-10-07:5CPU-first originalregisteredWasher120cells90s/4cachecopies10s/5PID-owned1280x720window-only recorders10fps90s/5probes5s/5decoders20s/25stages/outer660/offline30;352pins,0builds/newcandidate/retries. Beforemedia original017 retained; oldsoftwarecontext and earliermaterial/grid repairs separatelylabelled. Sourceaudit all165rows/22files reconciled only017tagPERF05;163classnameslocated/twoMixedStackstemplatealiases manuallyreadline7/registrations37,39, allgenericexposuresremainexplicitUNREVIEWED. Prefreeze decodercommasescape corrected beforeanycell; initialsource/protocol retained. No checkbox/retention.18fixed1resolved3open;original165done0left;candidatevisual0done165pending.

PERF05 original washer-default-visual captureguard FAILED beforeanyrecorders: requested1280x720 ignoredbyallfiveheld mainobjects; actualrenderer1600x900 uniform. CPU120completed/pinnedhealthy, otherfour cellsunlaunched; exec51956terminal1collected/962612+962666absent. Originalclips/croppedsoftwarecontext preserved. New washer-default-visual-viewport-correction FROZEN5CPU-first120cells90s/4cachecopies/5PID-owned actual1600x900 recorders/25stages/outer660/0builds/newcandidate; changedcaptureinput, no unchangedretry.18fixed1resolved3open;original165done0left;candidatevisual0done165pending.

PERF05 viewportcorrection captureguard FAILED0recorders: pinned SokolX11 neverpublishes_NET_WM_PID, so PIDpropertysearch cannotfindwindow. CPU120completed/rawhealthy; otherfour unlaunched; exec79924terminal1collected/963104+963238absent. No furtherdimension-only trial. ActualSokolwindowAPI nm/sourceverified. New washer-window-handle-capture FROZEN366pins/1readonlyCPP10s+5links25s/5CPU-first120captures90s+4cachecopies/actualproducerhandle-and-PID1600x900/25stages/outer800/offline30/0Rust/newcandidate/retries; compile/owner metadata only changes, no math/renderwait alterations. Prefreeze Matchgroup grammar fixed beforejobs, originaldriver/protocol kept.18fixed1resolved3open;original165done0left;candidatevisual0done165pending.

PERF05 actualSokolwindowhandle capture launchedonce: exec18910/driverPID963872; washer-window-handle-capture/state.json authoritative. Sixcompile/links0, first3CPU/ordinary/native actual1600x900 clipscomplete5.155/22.131/8.268s withffmpeg0/5stages each; two combinedcellscontinueunderoriginal90s watch. No uncheckedmetadata/propertyfallback ordesktoprootcapture.18fixed1resolved3open;original165done0left;capture3done2left,visualreviewstillpending.

PERF05 actualwindow capture CLOSED/PASS:exec18910terminal0collected/all5apps+recorders0,5.155/22.131/8.268/22.216/8.634s;actual1600x900 producerSokolhandles,366pins/17PIDsabsent/numeric10.446s exec8871terminal0collected/4.8m dynamicrecords exact HWcontrols. All25before+25afterstages inspected; intact visiblefloor/drum/content, priorcolours/grid/context/crop separate, newUserControls panel blockscenter. review.json retainslimitations. Candidate1reviewed164otherdefaultsUNREVIEWED; original165done0left. New washer-unobscured-capture FROZEN5UI-onlyEscapeclose commands2s/5CPU-first120cells90s/25stages/outer660/0builds/newcandidate/retries onexactsameviewers; mustverifyall120states exact. SourceEscape closesControls only beforeselection.18fixed1resolved3open; no retention/[x].

PERF05 UI-onlyEscape capture CLOSED5apps/recorders0:exec89540terminal0collected,5.120/22.092/8.116/22.315/8.571s. Esccommand0 doesNOTproveUI effect; inspectedCPUfinal stillControls occluded, raw clips retained. No UIcodefix/physicsdefault change. New washer-geometry-capture FROZEN5CPU-first120cells90s withexistingverified--hide-ui parser,25stages/outer660/0builds/newcandidate/retries; retainallfullUI captures, label HUDhidden only/requireexact120physics. IMPORTANT dimensiondiagnosis correction: generatedmain DOEShonorlogical1280x720 env; actual SokolX11 scales withdesktopXft.dpi120/96=1.25→1600x900. OriginalS017 andRO rendererbanners1280x720 (notassumedcrop). MatchedsoftwareCCD improvement valid; hardware-vssoftware comparescontext+resolution/newUserUI, notGLalone isolatedexperiment orreleaseFPS.18fixed1resolved3open;original165done0left;candidateWasher reviewedwithocclusion/other164UNREVIEWED.

PERF06 hardware-progression-boundary FROZEN before execution: 5CPU-first registeredVillage120 cells90s/4cachecopies10s/5actualowned1600x900window recorders10fps/5probes5s/5decoders20s/25stages/outer660/offline30;0builds/candidates/retries. Use current retained normal Washer archives, exact source/library/viewer pins; old052software captures preserved. Diagnose attribution, no repair or[x] yet. 19fixed1resolved2open;original165reviewed0left.

PERF06 launched once: terminal exec39512/driverPID978402; hardware-progression-boundary/state.json owns app/ffmpeg IDs. Poll same handle, no observation-timeout relaunch. 19fixed1resolved2open; original165reviewed0left.

PERF06 query-operation-boundary FROZEN beforeexecution:1readonlyCPP25s/3links25s/2cachecopies10s/3CPU-first actualVillage8 cells30s/outer240/offline20;0Rust/candidates/captures/retries. Time5unchanged query/mover calls, recordexactCastMover inputs/fraction toattributemissingStep span; prior5route120budget CLOSED/PASS. 19fixed1resolved2open;original165reviewed0left.

PERF06 hardware boundary CLOSED/PASS:exec39512terminal0collected/5apps+recorders0;CPU2.580/ordinary29.294/native28.976/ordinaryboth29.107/nativeboth28.740s/all120clearhealth/freshposes/1static0joints/1553instances. Offline5.119s/exec41547terminal0:388pins/11producerPIDsabsent. All25before+25afterstages inspected;nearbyfloor/buildings/capsule retained/combinedCPUleft, full52500children/moverlanding/identity/picking unqualified. Physics.44-.62s/draw-render~3s explainonlypartof~27sGPUcadence; explicitquery/mover boundary next. 19fixed1resolved2open;original165reviewed0left;currentVillage1reviewedother164unreviewed.

PERF06 query boundary CLOSED/PASS:exec47212terminal0/9ops0;CPU.565s,ordinary3.471/native3.421s. Actual14(notassumed16)movercasts each; all14inputs/fractions CPUexact; ordinary1.619/native1.539sCastMover vsCPU.000017s. Offlineinitial16callassumptionfailure retained; distinct14countcorrection PASS 2.797s/392pins/8absentPIDs/0engine reruns. CastMover fullworld snapshot+wholecompoundbounds+allchild narrowphase isnextcandidateboundary; preservepublicfiltercallbackgroups. 19fixed1resolved2open;165reviewed0left.

PERF06 bounded-mover-candidate FROZEN beforeexecution:1candidate/1unit90s/2normalstaticbuilds90s/2links25s/2cachecopies10s/2actualVillage8cells30s/outer450/offline20;0CPUreruns/CPP/captures/retries. Cacheoriginalpublicgroupbounds; ownonlyBVHchildcandidates whilepreservingallpreviouspubliccallbackgroups/order (includingempty); unchangedcastmath/defaults/IDs. Temporarystaged3files restoredfreshmtime aftereachbuild; no retention/[x] beforefullgates. 19fixed1resolved2open;165reviewed0left.

PERF06 candidate launchedonce exec75430/driverPID986427; bounded-mover-candidate/state.json authoritative. Poll exacthandle;3temporaryfiles guarded/restoredperbuild, no observationtimeoutrerun. 19fixed1resolved2open;165reviewed0left.

PERF06 mover-function-controls FROZEN beforeexecution:1CPP25s/3links25s/2cachecopies10s/3CPU-first cells30s/outer240/offline20;each16movercasts6publicpicks/twofreshworlds/positivebox+compound/veto/gapcallback/transform/shape/worldreuse/IDs;0Rust/newcandidate/physicssteps/captures/retries. Candidatearchivesheld/sourcebaseline restored; no[x] orretention.19fixed1resolved2open;165reviewed0left.

PERF06 function controls CLOSED/PASS:exec45174terminal0/9ops0;CPU.016/ordinary1.217/native.866s. Each16casts/8positive/4veto/2empty-compoundparentcallbacks/6publicfloorpicks/twofreshworlds/transform+shapeedit/staleIDs/destroy. FractionsCPUwithinunchanged1e-5;owner/world/generation checkswithinbackend, noforcedcrossdomainIDs.409pins/8absentPIDs/offline2.804s;0physicssteps/newcandidate/retention/[x].19fixed1resolved2open;165reviewed0left;final2scenes0done2left.

PERF06 full-scene-confirmation FROZEN beforeexecution:10links25s/20CPU-first120cells(032/052,before+after,all5routes)45s/16cachecopies10s/20owned1600x900windowrecorders10fps/20probes5s/20decoders20s/100stages/outer1850/offline30;0Rust/CPP/newcandidate/retries. Exactnormalheldarchives baselineMC versusnewboundedmover; samehardware/DPI/hideUI/defaultgeometry; logactual120moverinputs/fractions andbody/jointexports forperbackendbeforeafterequivalence. Publicquery/picking/lifetime controls alreadyPASS; no[x]/retention untilfinalaffectedreviews/audit/report.19fixed1resolved2open;original165reviewed0left;final2scenes0done2left/other163defaultsUNREVIEWED.

PERF06 finalconfirmation launchedonce exec9677/driverPID988899;full-scene-confirmation/state.json ownslinkedunits/apps/recorders. Poll exacthandle;20freshboundedcells,no observation-timeout retries.19fixed1resolved2open;165reviewed0left;final0done2left.

PERF06 final032 REVIEWED_WITH_LIMITS:all25matchedhardwarebefore+25afterstages inspected/all5routes/default120;nearbyfloor/cubes/posts/blue+visiblepinkgreen capsules andcombinedrouting preserved. Actualquery/full120bodyjointrecords exactperbackend; farlevel/stairs/torus/heightfield/door/presets/manual/meshSAT unqualified. Capture20done0left;affectedreviews1done1left;19fixed1resolved2open;original165reviewed0left.

PERF06 fullconfirmation CLOSED/PASS:exec9677terminal0/all10links+20apps+recorders0;offline6.800s/exec62866terminal0/416pins/51absentPIDs. Villageordinary28.995→7.839/native29.180→7.739/ordinaryboth29.067→7.924/nativeboth28.444→7.772;BasicMover~3.3sunchanged. All120publicbody+jointrecords and131(032)/176(052)moverinput/fractionrecords EXACTbeforeafter perroute, allhealthy/fresh/capacityclear. All100stages individually inspected/2affecteddefaults reviewed0left/other163unreviewed;nearbyfloors/models/capsules/routing preserved. Villagecapsuleendsy3.0795455CPU/3.07954574GPU (min3.05781221), notunboundedfall; closedefaultcamera clipsrestposition. Positivequery/lifetime/pickingcontrols verifiedseparately; completegeometrySAT/longwindow/manual/presets unqualified. SourceNOTRETAINED/no[x] untilfinalreport/audit.19fixed1resolved2open;original165reviewed0left.

PERF06 guardedretention/finalaudit FROZEN:3exacttestedsourcewrites/1artifactaudit60s/1safefetch30s/1diffcheck15s/1doccheck15s;0builds/runs/captures/candidates/retries. Beforehashesexact;32existingtrackeddirtyfiles+49protectedWASMpublicpatchfiles+priorWasherfixtures pinned. Allfunction/full120/equivalence/2scene100stagegates PASS; counts/checkmark pendingaudit.19fixed1resolved2open;original165reviewed0left;final2done0left/other163unreviewed.

PERF04 earlier-point-load FROZEN beforeexecution2026-10-07:16 previously untested step106 nonwarm native44/48 updates from retained rejected-artifact inputs;1input5s/1helpercompile25s/1link25s/1native5s/outer90s/1offline20s/1audit20s;0GPUbuilds/sceneruns/captures/candidates/retries/sourcewrites. Trace biased/relax cache growth and isolate contact-only dispatch gaps; final-color phase gaps explicitly include integration. Currentworlda4/query7fab/sim5838 pinned, Box3D/WASM unchanged.20fixed1resolved1open;165reviewed0left. Read PERF04/earlier-point-load/native-protocol.json; not a currentproduction physical qualification or repair. Initialpreparation wrong mathsourcepath failed before protocol/execution; retained preparation-failure.json, math helpers areinlineheader.

PERF04 earlier-point-load CLOSED/PASS2026-10-07:16 new step10644/48 operations2176inputs/384outputs bitexact/144 primitivebindings; initial48largecache starts106/sub0relaxY−.00003769→−26.47929, later106end−126.02556. Body149firstlargevelocityjump is INSIDE precedingcolor0 JOINTdispatch, notcontactgap. Owner49revolute and56filter arethe otherconnectedjoints; inspect49next. Offline.086s/audit.037s/102pins6absentPIDs/execdirectterminal0; sourcesunchangedworlda4/query7fab/sim5838, no repair/newphysicalqualification. Initialpreexecution wrongmathpath failure retained.20fixed1resolved1open;165reviewed0left. Report PERF04/earlier-point-load/README.md.

PERF04 joint49-input-writer FROZEN beforeexecution:1ordinaryreadonlystatic90s/1link25s/1registeredRain120 inputcapture30s/display10s/outer180s/1extraction30s; conditional12new106 nativehingeops:1freeze5s/1compile25s/1link25s/1run5s/1compare20s;1audit30s,0nativeGPU/visualcaptures/newpolicies/retries. Original rejectedcoherentartifactphysics unchanged; newobserver selects49instead50 andpreserves48control. Currentworlda4/query7fab/sim5838 andsharednormalordinaryf4 restored; no retention or checkbox beforeclosure.20fixed1resolved1open;165reviewed0left.

PERF04 joint49-input-writer launchedonce execsession88391/driverPID1004178; state.json owns build/link/app/Xvfb. Poll exact88391; never retry after observation timeout.20fixed1resolved1open;165reviewed0left.

PERF04 joint49 originalbudget CLOSED compileE0425 failure3.871s,88391terminal1collected/all3PIDsabsent/currentworlda4/query7fab/sim5838+normalordinaryf4 restored. Distinctjoint49-mover-helper-correction FROZEN: appendonlylatestmoverhelper (unusedRain) tohistoricobserverworld;1build90s/1link25s/1Rain12030s/display10s/outer180/1extract30s, conditional12newnative106hingeops1freeze5s/compile25s/link25s/run5s/compare20s/1audit30s;0nativeGPU/visualcaptures/newpolicy/retries. Oldfailure/phaseunchanged.20fixed1resolved1open;165reviewed0left.

PERF04 correction launchedonce exec58471/driverPID1004657; state.json owns jobs; pollsamehandle, no retries.20fixed1resolved1open;165reviewed0left.

PERF04 finaldocumentation receipt CLOSED/PASS2.560s/exec74244terminal0collected/gitdiffcheck0:584immutableinputpins unchanged/23producerPIDsabsent/22groups21checked/onlyRainopen/PR09unchecked. Reports/recovery links valid; allnewbudgetsclosed/noqueuedjob/newnext-phasebudget.20fixed1resolved1open;165reviewed0left. GoalACTIVE; nextcontact22/position split remainsunexecuted.

PERF04 contact150-split FROZEN beforeexecution2026-10-07:1ordinaryreadonlybuild90s/1link25s/1Rain120inputcapture35s/display10/outer190;3rawstates beforecontact22/aftercontact22/afterposition106sub0 max128MiB;1extraction35s; conditional1newexclusive terrain nativebiasedcontact operation:1freeze5/compile25/link25/run5/compare20;1audit30.0nativeGPU/visualcaptures/newpolicy/retries. Currentworlda4/query7fab/sim5838/ordinarynormalf4 restoredafterbuild; originalphysicalFAIL/oldbudgets preserved.20fixed1resolved1open;165reviewed0left.

PERF04 contact150-split launchedonce exec19943/driverPID1006765; state.json owns children; poll exact19943, no observation-timeout retry.20fixed1resolved1open;165reviewed0left.

PERF04 contact150-split producer/extraction CLOSED/PASS:19943/13035terminal0collected;build18.451/link1.566/Rain1209.983; analysis5.413s/126444288bytes/3states/134400body+134400joint exact. Contact22produceswholecalf150velocityjump; followingpositionintegrationvel/omega exact. Root1520color22 linkschild995, counts2+1; original conditional singlemanifoldnative phase SKIPPED. Distinctcontact150-native-chain FROZEN:one actualnativeMeshcall coveringbothmanifolds;1freeze5/compile25/link25/run5/outer90/compare20/audit30;0GPUbuild/run/capture/newpolicy/retry.20fixed1resolved1open;165reviewed0left.

PERF04 contact150-native-chain CLOSED/PASS:2404/21823terminal0collected;182inputs exact/30outputs23bitexact/max7.6293945e-6/0failuresunchanged1e-5; actualNativeMeshcallreproduceslargecalfresponse. Contactarithmetic notdemonstratedcause. Distinctcontact150-manifold-geometry FROZEN:2previously untested106freshnative triangles470(root1520)/247(child995) capsule249/body149;1extract10s/compile25/link25/run5/outer100/compare20/audit30;0GPUbuild/run/capture/candidate/retry. Old677/107 passinggeometry remainsclosed.20fixed1resolved1open;165reviewed0left.

PERF04 mesh-zero-feature-operation FROZEN before execution:1 shader extraction10s/1 cached-dependency rustc compile25s/2 actual GPU fresh processes15s each (before/after artifact-only explicit-mesh0 guard)/outer80s/1 compare15s/1 audit20s;13 cases including captured child995 input;0 engine builds/scenes/captures/retries/production edits. Source trace identifies pack_point_features valid-zero sentinel replacement; Rain physical impact still unproven.20fixed1resolved1open;165reviewed0left.

PERF04 mesh-zero-feature-finalization FROZEN:1 full-shader/input extraction10s;2 isolated cached-dependency harness compiles30s each;4 fresh NVIDIA GPU processes (ordinary/native RadixCache ×before/after)35s each/outer220s;12 full finalizer/old-patch history cases;1compare25s/1audit30s;0engine builds/scenes/captures/production edits/retries. Preserve physical failures; candidate artifact-only.20fixed1resolved1open;165reviewed0left.

PERF04 full-finalizer CLOSED/PASS:41141terminal0collected;12 ordinary/native cases,2688 backend words exact/body+scene byteexact/only expected ID-history fields changed. Audit3.974s/570pins/36absentPIDs. Distinct mesh-zero-feature-current-rain FROZEN:current production +only mesh-zero guard;1ordinary staticbuild90s/1link25s/fresh actualCPU Rain12060s FIRST/ordinary candidate120150s/display10/cachecopy10/outer350/1analysis30/1audit30;0native GPU builds/scenes/visualcaptures/retries. No lookup/CCD policy changes. Physical failures remain open; guard artifact-only and source/archive restore mandatory.20fixed1resolved1open;165reviewed0left.

PERF04 current-source Rain probe launched ONCE exec85885/driverPID1012644; owned state.json records children/display; poll exact85885; no observation-timeout retry. CPU-first; guarded only math shader duringbuild with mandatory restoration.20fixed1resolved1open;165reviewed0left.

PERF04 current-rain original launch CLOSED/FAIL85885terminal1: missingCPU executable in failed normal120 setup directory; cachecopy only,0actualCPU/GPU runs/builds/sourcechanges. Failure/preflight omission preserved; allPIDsabsent. Distinct CPU-path correction FROZEN samefinitelimits/current source+onlyguard; verified successorCPU executable +all external link inputs preflight exist; fresh realCPU120 first then1ordinary build90/link25/Rain120150s/display10/cache10/outer350/analysis30/audit30.0native/captures/unchanged retries.20fixed1resolved1open;165reviewed0left.

CURRENT LIVE JOB: current-rain CPU-path correction exec34375/driverPID1012996; confirmed owned live PIDs[1012996, 1013152]; poll exact34375, no relaunch on observation timeout. Source may temporarily contain guarded math ONLY duringbuild; driver mandatory restores normal/archive.20fixed1resolved1open;165reviewed0left.

PERF04 current-source candidate CLOSED:34375terminal1/150.114s cutoff;73 completed profiles, no GPU finalphysical rows. Full-finalizer native controls pass independently. All currentLIVE append entries above are historical; no running/queuedproducer. Latest PERF04 CURRENT supersedes earlier next-actions.20fixed1resolved1open;165reviewed0left.

PERF04 kind-compile2026-10-07: distinct budget FROZEN before execution at PERF04/mesh-zero-feature-kind-compile/budget.json;3 isolated compiles30s each,4 corrected collider-input finalizer runs15s each,2 actual collide_pairs pipeline controls60s each,0full builds/scenes/captures/retries. Source edits artifact-only. Driver not yet launched.20fixed1resolved1open;165reviewed0left.

PERF04 kind-compile launched: exec_command session98959, owned driver1016117/display1016122; read state.json and poll same handle, no relaunch.20fixed1resolved1open;165reviewed0left.

PERF04 mesh-helper-inline-control distinct budget FROZEN:1 SPIRV hint transform/validation10s,1isolatedcompile30s,1GPUcreate60s,0fullbuilds/scenes/captures/retries; artifact-only append_mesh_point_with_identity DontInline control. No producer yet.20fixed1resolved1open;165reviewed0left.

PERF04 inline-control launched session67980 driver1017393/display1017478; inspect state.json and poll same handle.20fixed1resolved1open;165reviewed0left.

PERF04 relaxed-atomic-operand-correction distinct budget FROZEN:3isolatedcompiles30s;export10s/fourvalidators5s;4finalizer15s;2validpipeline60s;0productionbuilds/scenes/captures/retries. Only invalid relaxed operandID→existingzero; one separatevalidhintcontrol. No producer yet.20fixed1resolved1open;165reviewed0left.

PERF04 atomic-correction launched session99414 driver1018020/display1018098; inspect state.json/poll same handle, no relaunch.20fixed1resolved1open;165reviewed0left.

PERF04 atomic-correction CLOSED99414terminal1collected:3compiles/export complete;12atomicsemantics operandIDs only corrected, originalVUID10871 cleared; subsequentvalidatorVUID10684 onunchanged workgroupBodyArrayStride stopsbeforeallGPU runs. AuditPASS 0.887s/69pins;allPIDsabsent/source archives/priorBox3D/WASM preserved. NEXT independentworkgroup/interface type reachability and validlayout correction only undernewbudget.20fixed1resolved1open;165reviewed0left.

PERF04 workgroup-layout-correction distinct budget FROZEN:1type/layoutfilter10s/fourvalidators5s;0newcompiles;4correctedfinalizer15s;2validpipeline60s;0fullbuilds/scenes/captures/retries. Abortifremovedlayouttype sharedinterface. ReusepinnedunexecutedA binaries aftervalidation.20fixed1resolved1open;165reviewed0left.

PERF04 workgroup-layout-correction launched session57908 driver1018656/display1018742; read state.json and pollsamehandle.20fixed1resolved1open;165reviewed0left.

PERF04 capsule-mesh-specialization budget FROZEN:3isolatedcompiles30s/input10s/exportvalidate25s;1ordinary actualcreation60s;conditional4actualprimitive45s ordinary/native;0fullbuilds/scenes/captures/retries. Reservednative maincreationnotwired/UNQUALIFIED; no claim beyond actualnativeprimitiveRadixCache. Sourceartifact-onlyallcapsule/mesh/noSensor branch;1880actualshape inventory. No launchyet.20fixed1resolved1open;165reviewed0left.

PERF04 capsule-specialization launched session8060/driver1020354/display1020435; pollsamehandle andreadstate.json, no relaunch.20fixed1resolved1open;165reviewed0left.

PERF04 specialization8060 CLOSEDfixturepairassumption failure beforeallbuilds/GPU; retained. Distinct pair-correction budgetFROZEN480outer:4compiles30s/extract10s/exportvalidate25s/2ordinary-native actualcreates60s/4conditionalprimitive45s;0scenes,captures,retries. Actual249/250/reverse+4explicitnearby synthetic cases, realowners checked.20fixed1resolved1open;165reviewed0left.

PERF04 specialization-pair-correction launched session11001/driver1020625/display1020704; readstate.json andpollsamehandle.20fixed1resolved1open;165reviewed0left.

PERF04 positionintegration budget FROZEN2026-10-07: contact150-native-position-integration/budget.json; actual unchanged native static function copied byteexact with original CPU flags, two awake dynamic thigh148/calf149 saved106sub0 aftercontact22.1compile25s/link25s/native5s/compare10s/outer80s/audit30s;0GPU/Rust/scenes/captures/retries,1e-5. GPU→native flag mapping explicit; no next production change.20fixed1resolved1open;165reviewed0left.

PERF04 joint11 angular measurement budget FROZEN2026-10-07: joint11-native-angular-measurement/budget.json; four existing poses (CPU/GPU frames105/106), actual native CreateHuman frames plus original native/GPU health expressions.1extract10/Ccompile25/link25/native5/Rustcompile25/Rust5/compare10/outer110/audit30;0GPUscenes/captures/retries,1e-5. No solver/default/physics edits or waivers.20fixed1resolved1open;165reviewed0left.

PERF04 joint11 originalbudget CLOSED linkfailure: diagnostic Human archive missing creationobserver callback in fixture. Originalcompile0.365s/link0.114s,0native/Rust calculations, allproducerPIDsabsent/failurepreserved. Distinct joint11-angular-hook-correction FROZEN:1noop observerhook compile5/link25/native5/Rustcompile25/Rust5/compare10/outer80/audit30; reuseexact native.o,0helpercompiles/GPUscenes/captures/retries.20fixed1resolved1open;165reviewed0left.

PERF04 joint11 hookcorrection CLOSED: native fourcase calculationPASS/link0.114/native0.004s; Rustcompile0.265s fails unused anchor delta glue before all Rust runs. AllPIDsabsent, originalfailurepreserved. Distinct joint11-angular-expression-glue FROZEN:1Rustcompile25/Rust5/compare10/outer45/audit30; nativeoutputexactreuse,0Ccompiles/native/GPUscenes/captures/retries; angular expression unchanged, anchor-only delta stub0 notqualified.20fixed1resolved1open;165reviewed0left.

PERF04 joint11 measurement CLOSEDnumericdifference: Rustcompile0.465/run0.002s,4cases12scalar comparisons4failed max2.6559457e-5 at1e-5; actualfirstscreen native vsGPU3.278e-6, gross.053rad difference remains. Distinct joint11-angular-primitive-trace FROZEN1Rustcompile25/run5/compare10/outer45/audit30;0native/GPUscenes/captures/retries, four nativeoutputs reused; split rawproduct/normalization/rotation/dot before any repair.20fixed1resolved1open;165reviewed0left.

PERF04 candidate zero-ID review FROZEN2026-10-07: atomic-zero-selection-review/budget.json; prepared compiler HashMap first-match selection may vary when equivalent zero constants coexist.1source10/Rustcompile25/hostprobe10/outer45/audit30; max128synthetic selections/18savedSPIRVmodules,0GPU/scenes/captures/retries/productionwrites. No claim before execution; scopequestionpending/no criteria changed.20fixed1resolved1open;165reviewed0left.

PERF04 general-mesh-dead-code-control: budget frozen before execution:1isolated compile30s/export10s/2optimizer10s/3validator5s/2distinct creation45s/comparison15s/audit15s/outer210s;0fullbuilds/scenes/captures/retries/productionedits. Exact latest candidate general-mesh constants, actual production layout. Row005 Explosion also mesh+noncapsule; left UNREVIEWED, do not blindly repeat row004 timeout. Native joint-connected CCD exclusion hypothesis falsified by solver.c761: eligibility has dynamic/continuous/maxMotion only, no joint exclusion.20fixed1resolved1open; current4reviewed161left.

PERF04 general-mesh-dead-code-control CLOSED: bothdistinct creations45s timeout, debugexe instructions exact/prunedmath exact/functions211unchanged; no startuprepair. Audit29pins/all10PIDsabsent/session62726terminal0collected. Compilercontrol62726 CLOSED/audited29pins/all10PIDsabsent. Row005 Explosion UNREVIEWED/unlaunched/sharedgeneral-mesh failure hold. Next006 FallingBoxes once under frozen protocol; no mesh, different shader domain. Do not repeat known failed general-mesh compilation unchanged.20fixed1resolved1open;current4reviewed161left.

006 CLOSED/REVIEWED/auditPASS3.359s128pins/all19PIDsabsent: capture58646/compose69257/audit5362terminal0collected;9cells120steps/all45images inspected,3201bodies0joints identity/countsexact,visiblefloor/lattice/colours/overlaypreserved,upperlatticeclipped.5currentreviewed160left;original165reviewed0left;20fixed1resolved1open. Allcurrentproducersabsent, no retainedcandidate.005sharedmeshshaderhold stillUNREVIEWED;next007source-domain qualificationbeforelaunch.

PERF04 polytopes-mesh-startup-control budget FROZEN:125outer/oneisolatedexportcompile30s/export10s/validator5s/onecreation45s/comparison15s/audit15s,0fullbuilds/physics/captures/retries/productionedits. Existing algorithms unchanged; narrowed static kind/sensor dispatcher, fallback unmodified, routing/lifecycle/compound outputs unqualified.007FallingTrees wave mesh+convex cylinder hulls selects samefailedgeneralmeshvariant; held UNREVIEWED/unlaunched alongside005.20fixed1resolved1open/current5done160left.

PERF04 polytopes-mesh-startup-control CLOSED:validated exact narrowed module still45.123s creationtimeout,211functions emitted;comparison.013s/auditPASS.263s30pins/all5PIDsabsent/session77875terminal0collected. No productionretention/checkoff. Polytopescontrol77875 CLOSED/45s timeout/audit30pins/all5PIDsabsent. Next verify actual Destruction box-only collider input domain, then freeze distinct narrowed startup test; no current test budget or live job.005 and007 UNREVIEWED/unlaunched/shared shader hold; do not repeat known failed modules.20fixed1resolved1open/current5done160left.

PERF04 destruction-constructor-kind-boundary FROZEN:oneCPPcompile25s/3links25s/actualCPU-first3constructor15s/comparison15s/audit20s/outer185s;0Rustbuilds/GPUsteps/creations/captures/retries/productionedits. Actualsampleconstructor/alllivebodyshapes+owners+axisboxprecondition/publicGPUkind;no fullGPUupload/collisionclaim.20fixed1resolved1open/current5done160left.

PERF04 constructor-boundary44621 CLOSEDfixtureenumfailure:CPUconstructorPASS,ordinaryall4027actualshapes recordedthenobserver BOX0wrongassert,actualBOX1 fromtypes.rs;nativeunlaunched/alloriginalunusedbudgetsclosed. Distinctenum-correctionbudgetFROZEN150outer/CPP25s/2links25s/2constructor15s/comparison15s/audit20s;oneobserverintegeronly/0GPUsteps orproductionedits.20fixed1resolved1open/current5done160left.

PERF04 constructor enum correction80259 CLOSED/PASS:actualCPU/ordinary/native1mesh+4026axisboxes/no sensors,allowners/corners/destroy checks;comparison.028s/audit.563s137pins/alloriginal+correctedPIDsabsent. OriginalobserverBOX0failure retained. Distinct BOX-mesh controlbudgetFROZEN125outer/compile30s/export10s/val5s/creation45s/comparison15s/audit15s;0physics/productionedits/retries.20fixed1resolved1open/current5done160left.

PERF04 boxes-mesh control47519 CLOSED:actualcreation24.199s/24.565sprocess,validationPASS;comparison.010s/audit.295s38pins/all5PIDsabsent/sourceartifact-only. BOX-mesh control47519 CLOSED/PASScreation24.199s/audit38pins/all5PIDsabsent;artifact-only. ActualconstructorCPU-first1mesh+4026boxes qualified/enumcorrection80259audit137pinsPASS/originalfailure44621retained. Next pin same-input box/mesh+box/box primitive fixture provenance, then freeze bounded ordinary/native general-specialized output checks; no new budget or live job. Do not resume failed general variants or fullscene loops.20fixed1resolved1open/current5done160left.

PERF04 Destruction native-input provenance budget FROZEN before execution:1CPP25s/1CPUlink25s/1realCPUfactory+12triangle-hull+6hull-pair calls15s; comparison15/audit20/outer105;0GPU/Rust/steps/captures/retries/productionedits. Preparation filename failure preserved before any operations; corrected to actual convex_manifold.c.20fixed1resolved1open;original165done0left;latestcandidate5done160left.

PERF04 Destruction native-input phase49241 CLOSED: CPUcompile.766/link.315/actualfactory+18nativecalls.032s,4027bodies/0steps/destroyed. RawJSONserialization failure preserved; distinctmetadata-only single-bracket correction.001s, no numericchange orengine rerun. AuditPASS.538s/135pins/all4PIDsabsent.18frozeninputs available, GPUoutputequivalence unqualified; allbudgetsclosed/artifact-only.20fixed1resolved1open;original165done0left;candidate5done160left.

PERF04 Destruction primitive output control FROZEN:74pins/10operations;2isolatedRust30s/2exports10s/2validators5s/4ordinary-nativegeneral-box GPUcontrols45s/compare20/audit20/outer330;actualCPU-derived18cases.0fullscenes/captures/retries/prodedits. Cold primitive outputs only, not history/wholemesh/fullpair startup/routing/lifecycle proof. Original native triangleIDs782/783 mapped to local fixture0/1 explicitly.20fixed1resolved1open;original165done0left;candidate5done160left.

PERF04 Destruction primitive output66519 CLOSED/PASS:2builds1.368/1.317;2exports/validators;4GPUprocesses5.676/4.223/5.626/3.922s.18cases/all1152outputwords perbackend general-vs-box EXACT and ordinary/native allbytesEXACT;all18CPUgeometrywithin1e-5/max1.262e-7. AuditPASS.651/76pins/all11PIDsabsent. No startupperformance/fullscene/history/routing/lifecycle qualification, no sourceedit/checkoff; allbudgetsclosed.20fixed1resolved1open;original165done0left;candidate5done160left.

PERF04 BOX routing lifecycle budget FROZEN beforeexecution:4normalbuilds90s/10selectors8non-GPU10s+2actualGPUlifecycle90s/compare20/audit30/outer740.38sourcepins/five temporarysource replacements/three archives mandatoryrestored. Domain excludes sensors/proxy/compound/unsupported kinds; no genericfailedmodule dispatch, no captures/retries/retention. New routing variant unreviewed; existingcombinedcandidate5done160left;20fixed1resolved1open;original165done0left.

PERF04 BOX routing88671 CLOSED/TIMEOUT:ordinarynormalstatic21.056/unit47.114+4exactdomain/regressionsPASS; actual-world lifecycle90.164s terminated before completed-step marker. GPUsetup/poseexport visible, pipeline/dispatch/wait attributionUNRESOLVED; native7opsunlaunched/unusedbudgetclosed. Metadata1.252/auditPASS1.540s/120pins/all8PIDsabsent,5sources/3archives restored/49WASM+localworkpreserved. Earlier66519all18CPU/GPUprimitive/outputequivalencepasses remainindependent. No retention/captures/checkoff. Nextactualloading-onlyprepare_collision boundary0physics underNEWnotyetfrozenbudget; no unchangedfullstep/generalcompiler retries.20fixed1resolved1open;original165done0left;existingcombinedcandidate5done160left/newBOXroutingvisuallyunreviewed.

PERF04 previousgoalturn PROGRESS: actual18CPU/GPUprimitive results changednextboundary; runtime90s failure retained/restorationverified. New loading-onlyprepare budget FROZEN:1unitbuild90/1ordinaryprepare45/compare20/audit30/outer205/42pins/0steps/native/captures/retries/retention. Stage/selectedoverride observer only;5sources/3archives mandatoryrestore.20fixed1resolved1open;original165done0left;existingcombinedcandidate5done160left/newBOXvisuallyunreviewed.

PERF04 loading59334 CLOSED45.114sINITIALIZATION_TIMEOUT:unit47.812passes;ensure_sim doesnotreturn, collisionprepare nevercalled/0physics.103creation intervals41.063s; no-joint scene eagerlycompiles solve_joints6.603s/solve_joint_color6.225s/unfinishedsolve_jointed_wave. Original88671timeoutlocationstillunknown. Comparison1.199/auditPASS1.219/113pins/all3PIDsabsent/5sources+3archivesrestored/49WASM+localworkprotected. Distinctevidence changesnextactiontolazyjointpipelinecreation, no solvermathchanges.20fixed1resolved1open;original165done0left;existingcombinedcandidate5done160left/newBOXvisuallyunreviewed.

PERF04 lazyjoint loading budget FROZEN:48pins/1unit90/1changedordinaryloading75/compare20/audit30/outer235;3eagerpipelinefields→OnceLocks/firstusegetters, originalshader/layout/cache/nooverrides/dispatchbranches/mathunchanged. Formerunconditionalclones guardedbyexistinghas_joints.0physics/native/static/captures/retries/retention; mandatory5source/3archive restore. Newloadingcheck allthreefieldsuncreated before/afteractualBOXprepare. Newcapallowsbaseinitialization+previous24.199sBOXcreation, no completebaselinespeedclaim.20fixed1resolved1open;original165done0left;existingcombined5done160left/newvariantunreviewed.

PERF04 lazy3joint73879 CLOSEDcompile101/10.736s: twoexistingtestcallers3fieldrefs needgetter, runtimeunlaunched/unusedbudgetclosed. Compare.017/auditPASS1.368/116pins/2PIDsabsent/restored. Distinct caller-correction budget FROZEN54pins/6sources/1unit90/1changedloading75/compare20/audit30/outer235/0physics/native/captures/retention; originalassertions preserved, previous3jointlazy candidate unchanged.20fixed1resolved1open;original165done0left;existingcombined5done160left/newvariantunreviewed.

PERF04 lazy3joint caller-correction90506 CLOSED/PASS: unit48.312/loading55.830s/actualBOXcollisionprepare22.910s. Ensure/selectedoverride/prepare return verified; all3jointOnceLocksfalse andno jointcreationtrace,0physics/positionunchanged/IDsvalid/destroyed. Comparison1.237/auditPASS1.230/125pins/all3PIDsabsent/6sources+3archivesrestored/49WASM+localworkprotected. Originalpartial41.063spipelinetrace retained, no equallycached totalbeforeafter claim. Artifact-only; native/jointfirstuse/replay/callback/fullscene/captures remainunqualified. Nextdistinctnormalnative+actualexistingjointcontrols/changednativelifecycle NEWbudgetnotyetfrozen.20fixed1resolved1open;original165done0left;existingcombined5done160left/newBOX+lazyunreviewed.

PERF04 HeightField actual query timing control FROZEN before execution: heightfield-query-timing-control/protocol.json;1CPPcompile+5links25s each,10runs30s (two fixed reversed orders,realCPUfirst),6actualRender14278casts/run,0physics/0Rust/0captures/0retries,560souter,20scomparison/20saudit. Newcandidate6reviewed159left;20fixed1resolved1open;original165done0left. Driver not launched.

2026-10-13 distinctforearmcontact2 FROZEN205pins:1observerbuild90/link25/Rain12090,2cachecopies10/display10/midstepreadback<=64MiB/raw<=1MiB/max8selected/extract30/audit30/outer300,0native/visual/newpolicies/retries. Current6sourcecandidate; reuse liveelbowcontrol17.448s. Readonlyafterprepareselection ofallactive color2 contacts touchingGPU111; complete hot/persistent/prepared+twoendpointbody/cold/extras. Require publicrecordnoninterference+positivecreation/hostshapeownership before nativequalification.20fixed1resolved1open/165reviewed0left/held21covered0left.

Host diagnostic steering, 2026-10-14: user asks whether process exhaustion is a leak we can resolve. Current read-only check: 425 processes, zero zombies, affected app scope589 tasks versus18,902 limit; four-second follow-up stable. Historical13,492 Git children/6,765 zombies remains evidence of severe buildup, not an identified trigger. Distinct current finding: app-owned Git private-index snapshot operation had run791s/752 CPU seconds and read132GB; causality to the earlier burst is unproved. No processes killed, settings changed or physics jobs started. See bounded host findings (local audit archive). Goal replacement remains proposed only; do not mark the unfinished goal complete to replace it.

Host investigation closure, 2026-10-14: identified automatic temporary-index Git staging of125,901 unignored PR09 evidence files (211GB combined logical file sizes). Applied reversible local `.git/info/exclude` rules for the two generated run directories only: untracked125,922→21 source files; listing15ms; tracked status and real index unchanged; all captures preserved. Selected future reports require deliberate `git add -f`; never stage the raw trees. The historical Rust-backend13,492-child/6,765-zombie burst is still not tied conclusively to the desktop snapshot;30-second observer caught no burst and is terminal. No claim of repaired process cleanup. Culprit investigation and checks (local audit archive). No new physics budget, goal replacement, commit or push. Counts20 fixed/1 resolved/1 open;165 reviewed/0 left.

2026-10-14 user-authorized cleanup/resume: removed723 retired health/build files +3585 archived pipeline/display caches (145.704GB logical total); screenshots/clips/reports and all non-scene PERF04 inputs retained. Historical raw replay completeness intentionally reduced; see host-fork-recurrence-2026-10-14/cleanup-and-resume manifests. Parent10 FROZEN201pins/current6sourcecandidate:1build90/link25/Rain12090/extract30/conditionalnative25+25+5/compare20/audit30/engineouter300;24operations/30KiB,0visual/retries. User asks only a few steps then host check; do not start another campaign.20fixed/1resolved/1open;165reviewed/0left.

2026-10-14 cleanup and bounded continuation closed: see PERF04 CURRENT and linked reports. User explicitly authorized disposal of redundant generated data; archived raw replay completeness is reduced.20fixed/1resolved/1open;165reviewed/0left. Parent10 native576 words exact; original physical failures still open. No new app task-limit events or zombies during this batch.

User storage requirement2026-10-14 is now standing policy in AGENTS.md and roadmap Storage budget and cleanup: 1GiB default temporary/64MiB retained per experiment, explicit exceptions, named consumers/deletion points, ignored/outside-worktree scratch, bounded output producers, and cleanup on success/failure after restoration. Applied to latest parent10 batch: 15 disposable files/442976846 logical bytes deleted; retained task evidence 2782953 bytes. Full observer dump no longer available; compact inputs/extraction/comparison/audit receipts preserved. Current shared CPU/control/candidate/cache seed still have named next consumers and remain unchanged.20fixed/1resolved/1open;165reviewed/0left. No physics runs or new experiment budget.

2026-10-14 continuation PROGRESS: native angular attribution on16 retained elbow pose pairs shows107/sub1 .0148787→.0648977rad, sub2→.1342315, sub3→.1644467; final quaternion words match actual published poses.384float+64metadata echoes exact,0GPUbuilds/runs/captures. Contact60 cache carries bit exactly across16 within-substep+6 between-substep transitions; cross-step friction projection differs by<3e-7, not a demonstrated cause. Audit20pins/.0095s/allPIDsabsent; native object/executable deleted,~36KiB before report retained. Original18/27/249 failures remain open.20fixed/1resolved/1open;165reviewed/0left. Goal tool ACTIVE; all new budgets closed. Next source-only cross-step cache projection coverage audit before any further calculation; no new scene budget.

Remote fetch2026-10-14 completed safely:HEAD/origin feat/gpu0ahead/0behind; local changes preserved. New source coverage report identifies cross-step contact60 friction projection as not covered by the prior preparation fixture; no new execution budget yet.

2026-10-14 PROGRESS: contact60 cross-step native source blocks reproduce both friction values bitexact; that hypothesis closed/no repair. New mesh-unilateral scalar-fallback candidate fixes actualfloor677 numeric gate6→0 failures perbackend/all352values numericallyexact(342bits,signedzerosonly). 18branchcontrols/backend passed after retained orphan-entry-attribute harness failure/type-only correction; convex/both-active branches unchanged,declinesleave stateuntouched. Current7source actualordinary/native120 validation FROZEN231pins,2build90/2link25/2run90,500souter,1.5GiBexplicittemp/64MiBretained,0captures/retries. Ownedjob launched once; exacthandle in current-job.json. No productionpromotion;20fixed/1resolved/1open;165reviewed/0left.

2026-10-14 PROGRESS: contact60 friction projection native2/2bits exact, hypothesis closed. New artifact-only mesh-unilateral scalar fallback clears floor677 native numeric failures6→0/backend;352values numericallyexact(342bits,signedzeros),2,848inputechoes/4,096backendwords exact.18branchcontrols/backend pass after retained annotation-only harness correction. Actual current7source Rain120 ordinary/native complete:build21.729/22.904,link1.271/1.466,run44.980/45.260s;134400body+joint records/backend andCPUcreationmatching,backendpublicstates bitexact. Firstchangedstep106. Physical18anchor/27angular/249cone unchangedFAIL;no productionpromotion. Audit276pins/2.281s,sources7/archives3restored,PIDsabsent/scopescollected. Cleanup537MB,~386KiB factoryrecords retained.20fixed/1resolved/1open;165reviewed/0left. NEXT completeCCDmesh-candidate coverage preflight (native16x16 torus/b3ShapeTimeOfImpact, respectbackfaces), not newtimingpolicy; verifyoldfrozen-inputapplicability first. No nextbudget/livejob.

PR10 mutation/force diagnostic build budget FROZEN:1CPP30s/5links25s/230MiB/0apps/enginebuild/retries. Summary mutation observer/all-body filters/bounded examples; native original calls unchanged. Current297widgets/45left,160samples/5left,22custom/1left,26open/2verified.

PR10 mutation/force build21924 CLOSED0/15.785s;15app review FROZEN90s/app/20s transitions/45screens/4Windsteps perroute/0othersteps/0retries/1GiBraw/64MiBretained. Counts297/45widgets,160/5samples,22/1custom,26open/2verified.

PR10Ghostcontrols FROZEN:5CPUfirstapps90s/3screens/24stepscap/3realmouseoffsets/0builds/retries/1GiBraw/64MiBretained. Actualpausedslidersandnonzerocounterresetprecondition.310widgets32left/163samples2left/22custom1left/27open2verified.

PR10Ghost93121 CLOSED1/CPUonly actualsliderspass,Resetnonzeropreconditionmissesnativeheightthreshold; no defect/passinvented. Distinct1wheelout+1pixelcounterinput FROZEN5apps90s/4stepcap/0builds/retries,CPUslidersreused/4GPUslidersfirstattempt.310/32widgets,163/2samples,22/1custom,27open/2verified.

PR10ReplayfixtureprerequisiteREADY: replay-fixture-acceptance.json, actualCPU64frame76546Brecording withnativevalidation/bodylifecycle/joint/contact/all7querykinds. Originaldiagnosticcompile/overstrictcoveragefailuresretained; finalread-onlyscanPASS, no regeneration. First5typespositive/miss, moverhitlists0explicit/notnocollisionqualification.7PIDsabsent/0cores/executablesdeleted3451352/retained79773/raw864636534; allbudgetsclosed/0GPU/apps/engineedit. NextCPU-onlyplayer/UIobserver andremaining29Replaywidgets/custom1 underNEWnotyetfrozenbudget; counts313/29widgets164/1samples22/1custom27open2verified.

PR10CPUReplayobserver31947 compile/linkPASS; popup64519 CPUonlyGTKdiscovery8stimeoutbeforewidgets, terminal/streamsconsumed/4GPUunusedclosed. Distinctownedwindow/phase/inputhandling correction FROZEN5apps60s/0build/retry/samecaps, no productdefectclaim.313/29widgets164/1samples22/1custom27open2verified.

PR10GTK9624 CPUonlyfoundownedchooser/pathsentbutacceptancetimeoutbeforewidgets; OBS02probe/environmentunconfirmed/noproductdefect. Native --replay supportedsourceinput distinctpopupreview FROZEN6apps60s/0build/retry/samecaps, CPUcancel+Rpolicy/CPUapply/4GPUunsupportedCancel. OriginalGTKfailuresretained/no unchangedretry.313/29widgets164/1samples22/1custom27open2verified.

PR10ReplayTimeline FROZEN1CPUapp90s/3screens/200actions/512step40seekcalls/3200recordedstepupperbound/0buildretry,24MiBAPI/1GiBraw64MiBretained.7widgets+Keyboardpartial/nativeCPUplayeractualeffects/GPUloadedunsupportedreuse.316/26widgets164/1samples22/1custom27open2verified.

PR10ReplayQuery FROZEN1CPUapp120s/3screens/170actions/400step30seekcalls/2400recordedstepupperbound/0buildretry,32MiBAPI/1GiBraw64MiBretained;10widgets actualindex/filter/selection/draw/identity.323/19widgets164/1samples22/1custom27open2verified.

PR10Query71394 CPU25.741s closed/all10effectswithoneweakimmediatetextassertion(knownfullnativechangevalue/64rows,actualbadvaluenotretained). Selection9widgets/custommousefocuslayout+textobservability FROZEN1CPUapp140s/5screens/220actions/400step30seekcalls/0buildretry/32MiBAPI/1GiBraw64MiBretained.323/19widgets164/1samples22/1custom27open2verified.
