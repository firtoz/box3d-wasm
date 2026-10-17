# PR10 UI review — recovery log

Read this FIRST after continuation/compaction, then AGENTS.md and docs/goals/gpu-production-readiness.md. Use the goal-scratchpad skill. Historical receipts: [earlier history](history-2026-10-15.md), [closed control batches](history-controls-2026-10-15.md). Do not restart closed budgets.

## Objective and authorization
**ACTIVE APP GOAL, created at the user's request on 2026-10-15:** complete PR10 end to end, continuing autonomously across turns until its applicable review and repair criteria pass. Stop after PR10. Do not pause unless the user explicitly asks. No token budget was requested or set; each execution batch still needs finite resource limits.

Finish PR10 UI/widgets only: source inventory and actual mouse/keyboard checks on real Box3D CPU first, ordinary/native GPU and both combined routes. Verify displayed state, actual effect and lifecycle; group shared implementation only with explicit applicability. Review all controls before fixing newly found defects, then one cause at a time with CPU-first before/after evidence. Preserve geometry/defaults/body identity/picking, Box3D/WASM/Rust/WGSL and other local work. No PR11/solver/cold campaign. No new PR10 commit/push authorization. PR09 already published as 51ff8b06b4964d359386f08605d835ac6e2ea23d.

Review all165 samples' applicable342 sample widget calls,85 shared calls and23 custom input handlers. Use actual CPU/ordinary-GPU/native-GPU/ordinary-combined/native-combined inputs and outcomes; unsupported features need explicit honest dispositions. Reuse passed checks with justified source applicability and preserve original failed receipts. Do not repeat PR09's completed default visual sweep. After review, trace the smallest failing operation, fix one cause and its affected cases at a time, and share before/after screenshots with each repair report. Mark [x] only after verification. Report controls/samples done and left and issue counts in each progress update. Finish with an acceptance audit; PR10 remains unchecked until its actual criteria pass. Preserve local work and integrate remote changes safely if needed; commit/push requires separate authorization.

All new automation uses TypeScript/Bun. Freeze finite build/check/capture/process/runtime/storage budgets before each batch, enforce reasonable timeouts/output limits, disable owned viewer core dumps and reap child processes. Default1GiB temporary/64MiB retained evidence per batch; budget exceptions beforehand. Measure and preserve shared caches; delete consumed task-owned outputs only after minimal evidence is extracted. Cleanup includes failed runs. Do not resume exhausted experiments or unrelated solver work; exact pixel/chaotic scene trajectory matching is outside scope.

## Goal acceptance gates

| Gate | Current status | Required evidence |
| --- | --- | --- |
| Exhaustive native control inventory | Source inventory complete; runtime review partial | inventory.json; inventory-ts-acceptance.json |
| Actual input, state and intended effects on five routes | Shared85 reviewed; sample33/342 reviewed;23 custom handlers open | Per-control receipts and explicit applicability/exclusions |
| Per-sample interaction dispositions | 101/165 reviewed with explicit reuse;64remaining | Individual inventory rows with evidence; no inferred passes |
| Finish review before new repairs | In progress | Review closure plus complete issue checklist |
| Resolve demonstrated PR10 issues one cause at a time | UI01/UI02 verified; UI03–UI14 open | Actual operation traces, affected-case checks, CPU-first before/after screenshots in each repair report |
| Resources and compatibility | Existing geometry batch closed/cleaned; ongoing requirement | Finite budgets, cleanup/process/cache receipts; unchanged Box3D/WASM/defaults/identity |
| Final PR10 acceptance | Open | Criterion audit and retained affected-scene comparison evidence; PR10 checked only when criteria pass |

## Counts
- Source inventory: 165 mapped / 0 left. 342 sample widget call sites in 67 samples; 23 custom handlers.
- Shared calls: 85 reviewed / 0 left, including defects and exclusions; this is not 85 passing features.
- Sample widgets: 33 reviewed / 309 left (Jenga2, BodyType4, Replay7, geometry18, Disable2).
- Full sample interaction reviews: 101 reviewed / 64 remaining (7actual sample-widget fixtures,94explicit shared-implementation reuse). This is review coverage with known defects, not feature-pass or new-runtime counts. Prior PR09 default visual sweep is complete; do not repeat it.
- Issues: 12 open (UI03–UI14); 2 earlier fixes locally verified (UI01 Info wrapping, UI02 wheel routing). New issues remain unfixed until review completes.

## Findings and evidence
UI03 missing GPU overlays: all 14 overlay/effect checks absent on four GPU routes, actual CPU callbacks positive; overlay-controls.md. UI04 CPU Profile labels clipped; UI05 hidden HUD overlap. UI06 cached body-type transparency/stale combined getter ownership; UI07 combined Frame Camera aspect. UI08 Replay Unix basename; UI09 GPU unsupported Replay message. UI10 captured W key-up leaves camera moving (all five routes); UI11 combined Escape selection restored from authoritative drag state. See each UIxx.md and issues.json.
UI12 Box Hull crash confirmed on real CPU: typing valid 0.7 transiently applies 0, eight CreateHull points become coplanar Y=0, constructor returns null, original DrawHull dereferences null. GDB stopped at this boundary; geometry-null-acceptance.json and geometry-null-debug.txt. GPU runtime not yet confirmed; shared source applicability only. No Box3D change. Continue valid fractional review using .7/.5/.25 spelling to avoid the zero prefix; this is stimulus adjustment, not a fix.
Shared picking/camera reviewed with named defects; picking-camera.md. Recording/replay.md, body-view-controls.md, render-controls.md, solver-controls.md and overlay-controls.md retain actual API/callback and screenshot evidence. STARTUP01 belongs to PR05/08; Rain physical failures PR04 remain outside this task.

## Current recovery state
All jobs terminal; no live viewer. Current qualified diagnostic pool: geometry-control-build.json, five readonly observer viewers under raw/geometry-control-build; original engines/gfx/main/Sample inputs unchanged. Original CPU partial geometry run (session90287) closed with h:0 change/restore PASS then UI12 crash. Subsequent TypeScript runs have now completed all5routes. Minimal GDB session25245 terminal, boundary established, no retry needed. Extracted partial core (204181534 bytes) deleted; protected system compressed original ~3 MB could not be deleted without sudo. Do not repeat extraction. Future owned viewers MUST set RLIMIT_CORE=0.
Retain main/Sample objects raw/ui02-after and raw/ui01-after for later diagnostic linking, and 28432-byte CPU recording raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec for remaining Replay controls. Never stage raw/; it is locally excluded. Measure/preserve shared caches, delete consumed streams and superseded binaries only after verification. No cache copies, full engine builds or videos.

## Closed TypeScript work and next actions
User requests TypeScript instead of Python (2026-10-15). All new automation uses TypeScript/Bun; old Python drivers remain historical provenance. Bun1.3.14. New scripts/inventory-sample-controls.ts independently matches every existing structural inventory field (165samples/342samplecalls/85shared/23handlers); inventory-ts-acceptance.json. Reviewed dispositions preserved; Jenga's2already-passed route fields normalized into widget.status during closeout (no new interaction claim).
Geometry budgets geometry-controls-ts-budget.json and geometry-controls-ts-tail-budget.json are CLOSED:0builds/5CPUfirst routes,240s/app,ready25/scene90,8screens/app,320actions,API32MiB,temporary1GiB/retained64MiB/sharedgrowth128MiB,0retries,core0. The first TS CPU process was cancelled on a filtered picker row race; BoxHull passed cases preserved. Structural world comparison and stable filtered row/actual scene label gate corrected only automation. Remaining CPU3tools+4unlaunchedGPU routes then completed in session91637 (CLOSED0):CPU26.355s,ordinary53.011s,native53.321s,ordinarycombined53.203s,nativecombined53.613s. All130component/action cases reviewed (26perroute), actual hull geometry/restoration; no new product fixes. Fifteen rotation-count predicates were harness assumptions: typing20 applies2 then20, with two distinct actual intermediate hulls; resolved from existing traces, no rerun or original receipt edits.
See geometry-controls.md, geometry-controls-acceptance.json and CPU-first geometry-controls-grid.html. UI12 remains open. Old overlay5binaries and consumed original/TS streams deleted:202175195bytes in geometry-controls-cleanup.json plus6550332bytes in geometry-stage1-cleanup.json. Current qualified pool geometry-control-build; it remains a named input for remaining UI work. Sharedcachegrowth0/core0. Resource receipt geometry-final-resources.json:raw346815382bytes,free1709239603200bytes; all owned apps/drivers terminal.
Next: remaining311samplewidgets and23customhandlers under a NEW finite budget, using TypeScript/Bun. No next runtime budget frozen; do not restart closed runs. Fullsample0done165left remains conservative pending per-sample applicability reconciliation. Shared85done0left, samplewidgets31done311left, issues10open/2earlierfixed. PR10 stays unchecked; no PR11/solver/cold campaign, no new commit/push instruction. Representative interaction screenshots do not replace a future complete affected-scene precommit grid.

Next budget FROZEN disable-controls-budget.json:0builds/5CPUfirstapps180s,ready25/scene+step90,max6steps/app,3screens80actions,API32MiB/temp1GiB/retained64MiB/sharedgrowth128MiB,core0/retries0. Reuse geometry-control-build viewers; generic TypeScript driver extracted without changing closed geometry runner. Consumer Bodies/Disable2checkboxes; actual enabled flags/IDs/disabled pose freeze/enabled motion/restart. No product repair.

Disable matrix launching CPU first; disable-controls-matrix.json records current owned child. New budget only; previous geometry jobs remain terminal.

Disable matrix terminal; see disable-controls-matrix.json and receipts before disposition/cleanup.

Disable matrix session55214 CLOSED0, all5routes31checks each/155total, max6steps each,3screens perroute; no new issues/fixes. Two samplecalls closed→33done309left. Fullsample0done165left pending applicability reconciliation. Cleanup deleted2834193bytes, sharedcachegrowth0, allownedapps/drivers absent. See disable-controls.md/acceptance/grid. Next per-sample applicability reconciliation, then remaining controls; no runtime live.

Applicability reconciliation CLOSED/offline:101sample reviews dispositioned (7actual-widget fixtures+94base-only classes with no input/solver/focus/layout overrides),64remain. Every row has original source hash/inheritance, actual shared evidence, unchanged PR09 visual/runtime limits and reuse method; sample-applicability.md/json. This is not101new runtimes or featurepasses. Globalsourceaudit5unaccounted widgets are all disabled Ragdoll/Pose #if0, explicitlyexcluded;342active samplecall denominator unchanged. Currentcontrols33done309left; samples101reviewed64left;23customhandlersopen; issues10open/2verified. No runtime live.

Manifold readonly buildbudget FROZEN manifold-observer-build-budget.json:1CPP20s/5links25s/0apps/captures/0retries,210MiBtemp8MiBretained. Observe actual9Collide inputs/cache/results and directdrawposes; original once, no mutation. Consumer9Manifoldtools45widgets+9customhandlers; source-only findings remain hypotheses until actualUI/API review.

Manifold buildsession98550 CLOSED0:1readonlyCPP/5links PASS originalengine/gfx/main/Sample inputpinsunchanged. RuntimebudgetFROZEN manifold-controls-budget.json:0builds/5CPUfirstapps300s,ready25/scene90,10screens280actions/API32MiB/temp1GiB/retained64MiB/sharedgrowth128MiB/core0/retries0. Nine tools45widgetcalls+9customhandlergroups, actualcache/math anddirectdrawposes; no-body native math inallviewers, not GPUsolver. Controls cachepolicy sourcehypotheses need actualUI/APIconfirmation; nofix.

Manifold matrix launching CPU first; manifold-controls-matrix.json records currentchild/PID. No previous run restarted; actualmath traces gated totool mode.

Manifold stage1 session79687 CLOSED1/CPU44.342s;193checks/7tools35widgets/21mousegestures reviewed, CPUlast2tools+4GPUroutesunlaunched. Stablepickerassert stoppedbeforeTriangleHullclick; allownedappsterminal. UI13unusedcapabilitycontrols andUI14auto-staleSATtype8 confirmedCPU/unfixed; issues12open/2earlierfixed. Two populated-cache predicates were unsupportedoverlapassumptions (actualcache0allowed); new separatedstimulus required onlyforthoseeffects. ManifoldtailbudgetFROZEN beforeexecution, samecaps:CPUremaining2tools+2cacheeffecttails,4unlaunchedGPUfull9tools. Originalsources/receipts preserved; no repeats ofpassed CPUwidgets/mousehandlers.

Manifold matrix terminal; inspect manifold-controls-tail-matrix.json and receipts before acceptance/cleanup.

Manifoldtail session42263 CLOSED0/all5routes. All9tools widgets/mousecases executed, but2Hull/simplexUseCache effects remainunqualified: deepoverlapandseparation legitimatelyempty. No solverbug. Shallowmouseinputs nowderivedfromactualCamera+mainFOV50 (constructor60hypothesisfailed anddiscarded), frozenactualdelta comparison<1e-5; cache-shallow-stimuli.ts/json. New finite2-control-onlyCPUfirstbudgetneeded; do notrerun43otherwidgetcalls or9mousegroups. Allapps terminal.

Cache-shallow-onlybudgetFROZEN cache-shallow-controls-budget.json:0builds/5CPUfirstapps90s,ready25,2screens60actions/API32MiB/temp1GiB/retained64MiB/sharedgrowth128MiB/core0/retries0. Only2UseCacheeffects unfinished. Capsuletargetgap.075<radius.15; Spheretargetx2.2 (delta.7) keepspositiveboxdistance belowradius1 onbothaspectratios. No repeatsof43otherwidgetcalls/9mousegroups, nofix.

Shallow matrix terminal; inspect cache-shallow-controls-matrix.json before closeout.
