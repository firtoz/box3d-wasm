# PR10 UI review — recovery log

Read this FIRST on every continuation and after compaction/restart, then `AGENTS.md` and `docs/goals/gpu-production-readiness.md`. Apply the goal-scratchpad skill. This is the current state; dated histories contain closed experiments, not next actions.

## Active goal and authorization

Finish **PR10 UI/widgets end to end** in `/home/firtoz/work/2026/box3d-wasm` on `feat/gpu`. The user activated the app goal on 2026-10-15. Continue autonomously across turns; do not pause unless explicitly requested. Stop after PR10. No token budget was requested. Each execution batch still requires finite resource limits.

Review applicable controls for all 165 native samples: 342 per-sample widget entries, 85 shared calls and 23 samples with custom input handlers. Compare real Box3D CPU first, ordinary/native GPU and both combined viewers. Exercise actual mouse/keyboard input, displayed state, intended geometry/physics/render effects, identity and lifecycle. Shared implementation may reuse exercised cases only with explicit per-sample applicability. Source registration, a screenshot or input acceptance alone does not prove an effect. Unsupported features require honest UI and explicit dispositions. Preserve previous passes and original failures; do not repeat PR09's completed default visual sweep.

Finish the remaining review **before fixing newly recorded issues**. Then trace the smallest failing operation and fix one cause and its affected cases at a time. Verify applicable routes, retain CPU-first before/after screenshots and required affected-scene comparison evidence, mark [x] only after verification, and share screenshots with each repair report. Report controls/samples done and left and issue counts in every progress update. Finish with a criterion audit; PR10 stays unchecked until its actual criteria pass.

Use **TypeScript/Bun** for new automation. Freeze build/check/capture/process/runtime/storage budgets before each batch. Enforce reasonable timeouts and output caps, disable owned viewer core dumps, and reap owned child processes. Default limits: 1 GiB temporary / 64 MiB retained diagnostic evidence per batch; budget exceptions beforehand. Measure and preserve shared caches. Delete consumed task-owned binaries/traces/copies after extracting minimal evidence, including failed runs. Keep a named consumer for retained large inputs. Never stage `raw/`; it is locally excluded.

Keep Box3D/WASM unchanged and preserve geometry, physics defaults, body identity, picking and other local work. Review probes also leave engine/Rust/WGSL inputs unchanged. No unrelated solver repairs, PR11 query qualification, cold-start campaigns or exhausted experiments. Exact pixels and chaotic CPU/GPU trajectory matching are outside scope. Integrate remote changes safely if needed. No new PR10 commit/push authorization. PR09 is already pushed as `51ff8b06b4964d359386f08605d835ac6e2ea23d`.

## Counts and acceptance gates

| Gate | Reviewed / remaining | Evidence |
| --- | --- | --- |
| Native source inventory | 165 mapped / 0 left | inventory.json; inventory-ts-acceptance.json; source-callsite-audit.json |
| Shared calls | 85 reviewed / 0 left, including defects/exclusions | Per-control reports and acceptance receipts |
| Sample widgets | **78 reviewed / 264 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **110 reviewed / 55 left** | sample-applicability.md/json |
| Custom input sample groups | **9 reviewed / 14 left** | manifold-controls-acceptance.json; 27 overridden mouse methods exercised |
| Repairs | **15 open; 2 earlier fixes verified locally** | issues.json; UI01–UI17 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 110 sample dispositions comprise 16 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 110 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Closed evidence and recovery state

- `disable-controls.md`: two controls pass all five routes / 155 checks. Actual enabled flags, original IDs, disabled pose stability, resumed motion and restart defaults verified.
- `geometry-controls.md`: 18 widgets / 130 component/action cases across five routes. Valid hull edits and restoration verified; UI12 remains open.
- `manifold-controls.md`, acceptance and CPU-first HTML grid: nine tools / 45 per-sample widgets / nine custom mouse groups reviewed. Actual LMB translation, Shift rotation, release stopping and Alt-ignore produce the expected render/collider poses; 135 gestures across five routes. These no-body tools use native C preview math in GPU viewers; this is not GPU solver qualification.
- Manifold's 1,344 recorded check evaluations include 165 policy findings grouped into UI13/UI14 and 20 invalid nonzero-cache stimulus assertions. Original failed receipts remain unchanged. Deep overlap and separation can legitimately empty simplex caches. Only the two unfinished cache effects were probed at source-derived shallow contact: Capsule vs Hull **3 → 0 → 3**, Hull vs Sphere **2 → 0 → 2**, on all five routes. Final targeted batch: 75 passing checks, geometry preserved across toggles.
- Planning initially assumed constructor FOV 60; `main.cpp` sets 50. The corrected ray model matches frozen native deltas within 1e-5. It plans inputs; actual native results prove effects. No tolerance changed.

**All jobs are terminal; no live viewer or next runtime budget.** Closed sessions: Disable 55214; Manifold build 98550 (1 read-only C++ compile + 5 links, 10.784 s); initial Manifold 79687 (CPU seven tools completed, then picker reflow assertion); tail 42263 (CPU remaining two tools plus four previously unlaunched GPU routes); shallow-only 5489 (all five routes). Passed widgets/mouse groups were preserved, not replayed. Details remain in `history-manifold-2026-10-15.md`, `history-controls-2026-10-15.md` and `history-2026-10-15.md`.

Current qualified pool: **`manifold-control-build.json`**, five viewers in `raw/manifold-control-build/`. Original engine/gfx/main/Sample input hashes are unchanged. Old geometry viewers and consumed streams were deleted: 185,837,721 bytes (`manifold-controls-cleanup.json`). Consumed renderer stderr timing streams were summarized and removed: another 13,431,560 bytes (`manifold-stdout-cleanup.json`). Sixteen owned app/driver pairs are confirmed absent. Shared cache growth was 0; core dumps disabled. Latest raw total: **365,695,335 bytes**; free space is recorded in cleanup receipts.

Retain the current pool for remaining controls until a verified replacement exists. Main/Sample objects in `raw/ui02-after/` and `raw/ui01-after/` remain diagnostic link inputs. Keep the 28,432-byte CPU Replay fixture `raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec` for remaining Replay controls; delete when consumed. Keep selected screenshots, receipts and minimal operation values. Preserve user work/shared caches.

## Next actions

Continue the **264 remaining widgets and 14 custom input sample groups across 55 samples**, under new finite budgets and using TypeScript/Bun. Read source first and derive intended effects before running. Next useful groups are body mutation/explosion/joint controls or remaining collision tools; a broader read-only boundary probe can reuse existing link inputs without rebuilding engines. Use the current shared viewer pool until a replacement is qualified, then delete the superseded pool. Closed budgets remain closed.

Finish the whole review, then repair issues one cause at a time with CPU-first before/after evidence. Keep the active goal running; PR10 remains unchecked. No PR11/solver/cold campaign or commit/push instruction is active.

Next readonly build budget FROZEN: distance-observer-build-budget.json (1CPP20s/5links25s/0apps/0retries/210MiBtemporary/8MiBretained). Consumer3no-body collision tools11widgets +ShapeDistance custom mousegroup; actualGJK/TOI inputs/outputs anddraw/texteffects. Sourcehypothesis:TOImenu4labels for3enumtypes; runtimeconfirmationrequired. No engine or productrepair.

Distance stage1 buildsession68715 CLOSED0/11.253s,0apps. Supplemental immediate invalid-cache boundary probe needed forShapeDistance proxy transitions. New finite budget distance-observer-boundary-build-budget.json frozen1CPP20s/5links25s/0apps/0retries; unlaunched stage1 fivebinaries removed164755016bytes before replacement, oldmanifest/source/logprovenance retained. CurrentqualifiedManifoldpool remains usable.

Distance boundary buildsession49450 CLOSED0,1readonlyCPP/5links originalinputsunchanged. RuntimebudgetFROZEN distance-controls-budget.json:15independentfixtureapps (3CPUfirst then3per4GPUroutes),150s/app,ready25,4screens160actions/API32MiB/temp1GiB/retained64MiB/sharedgrowth128MiB/core0/retries0. Only3no-bodytools11widgetentries +1custommousegroup. KnowninvalidTOIenum/cacheproxychanges exercisedlast; faultmayconfirmnegativefinding, nevercalledcleanquit/PASS. Independentappboundaries avoidreplayingpositivecases. No productfix.

Distance15fixture matrix launching CPU3fixturesfirst; distance-controls-matrix.json records current ownedchild. NegativeTOIenum/cachetransition arelast, core0; actualfaults countonlyasreviewfindings, nevercleanquitpasses.

Distance matrix terminal; inspect distance-controls-matrix.json before disposition/cleanup.

Distance stage1session18783 CLOSED1/CPU3fixturesonly. DistanceDebuglabelhit failedTempInput activation (bar96px;combinedrectincludeslabel), no productfailure. CPUShapeDistance8widgets/customhandlerscompletedwithUI16hiddenradius/UI17invalidcacheinput, cleanquit0. CPUTOI2widgetsvalidchoicescheckedthenfinalindex3producedzerovertexinput andSIGSEGV(UI15); core0andnoPID-matchingfileinsystemcoredump. FourGPUroutesunlaunched. NewtailbudgetFROZEN distance-controls-tail-budget.json:13apps=CPUremainingDD+4GPU×3fixtures, samecaps, Ctrlclickbar+12. PreservepassedCPU8+2widgets/handlers andoriginalfailure; no repeats or productfix. Issues15open/2verified.

Distance matrix terminal; inspect distance-controls-tail-matrix.json before disposition/cleanup.

Distance tail84026 CLOSED0/all13remainingfixtures; originalCPUShapeDistance+TOI reused, all3tools nowreviewedon5routes. Proposedclosure11widgets→89/253,1customgroup→10/13,3samples→113/52; acceptance/cleanup notyetwritten. UI15/16/17 confirmedall5; UI17invalidcache causesSIGILL4GPUroutes, CPUinvalidinputwithoutfault; TOIinvalidenum causesfaultall5. Newobservedcamera candidate:TOIdefaultCapsulepreviewoffscreen; F/MenuFrameCamera useemptyphysicsworldbounds. Newempty-world-framing-budget.json FROZEN0builds/5CPUfirstapps60s,2screens30actions/API16MiB/core0/retries0, standardstorage/cachecaps. Actualcamera/controlconfirmationbeforeclosingcameraapplicability; no repeats ofGJK/TOIwidgetchecks orbadoptions.

Framing matrix terminal; inspect empty-world-framing-matrix.json before finaldisposition.
