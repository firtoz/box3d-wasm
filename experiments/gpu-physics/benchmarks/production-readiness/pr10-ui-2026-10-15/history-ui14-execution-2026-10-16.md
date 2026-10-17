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
| Sample widgets | **342 reviewed / 0 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **165 reviewed / 0 left** | sample-applicability.md/json |
| Custom input sample groups | **23 reviewed / 0 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **15 open; 14 fixes verified locally** | issues.json; UI01–UI29 reports |
| Final PR10 acceptance | Open | Repairs, resource/compatibility checks and final criterion audit |

The 165 sample dispositions comprise 71 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 165 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping, UI02 wheel routing, UI04 CPU Profile width, UI05 hidden HUD layout, UI06 current body-type transparency/default material and combined CPU ownership, UI07 combined Frame Camera, UI08 Unix Replay basename, UI09 honest Replay availability, UI10 captured key release, UI11 combined Escape selection, UI12 null hull drawing, UI13 honest Manifold controls, UI19 Wheel steering damping and UI20 Driving torque are locally verified fixes. Open: UI03 missing GPU overlays; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size; UI29 Mesh Drop auto regeneration hides controls behind repeated GPU preparation and fails to switch off. See individual reports and `issues.json`. The full review is complete; verified repair evidence follows.

Historical UI12 diagnosis (now repaired): real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

Historical UI13 diagnosis (now repaired): four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. UI13 is now verified; UI14 remains open.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Fullreview COMPLETE:342widgets/165sample dispositions/23customgroups/85sharedcalls,0left. **15open,14verified** (UI01/UI02/UI04/UI05/UI06/UI07/UI08/UI09/UI10/UI11/UI12/UI13/UI19/UI20). GoalACTIVE,PR10unchecked.

UI13[x]:[repair](UI13-repair.md)/[acceptance](ui13-acceptance.json). NewTSnativecopygenerator exposescacheonly5supportedpairs/manualSATonly2supportedpairs;4no-cache/7no-SAT availabilitytext. All9actualpairs×5routes/540checksalltrue;30.9–33.3s each;actualsimplexpopulated/off0/onreuse/SAT6,7,8/off0/UIavailability/translation/colliderarguments/mouseup/restartgeometry/defaults. Originalconstructors/Step/Render/input/math/defaults unchanged;UI14manual→auto remainsopen.6CPUfirstgrids90originalbefore/afterimages inspected plus2selectedindividuals. 1C++compile/5links/viewers, alloldManifoldmodulesbyteidentical/noGPU/BOTHconditional;Box3D/WASM/Rust/WGSL/sharedarchivesunchanged. CMakeBunwire/fullfreshconfigurebuildcompatibilityfinalgate stillopen.

Currentpool **ui13-build.json/link_plan PASS**:5viewers/22tasklinkobjects/smalltaskAPIarchive nextPR10relink. Exactinputs ui13-acceptance.json.resources.active_task_objects. Olderui12/etc manifests historical/unrunnable afterviewerretirement/CMakepinchange;do notverifythem. Allcurrentpins/linkinputs checked. UI12previous3wronganalyticzero probes adjudicatedsameactualdata/noCPUrepeat;ENOE​​NTsizewalkwarning independentcloseout/helpercorrected;seeUI12report.

Cleanup raw946485247B/free1707953516544B/selected11817968B;deletedbuild166512016B/streams20206001B/logs32550B.17recordedPIDsabsent/0ownedcores/cache0/sharedcachespreserved. No livejob/UI13budgetsclosed. FreezeNEWbounds beforefuturebuild/apps;1GiBwholeRaw/64MiBretained/16MiBcacheapp32MiBrepair/20GiBfree. Sequentialviewerreplacement;removeconsumedoldtaskoutputs asneeded/preservemedia/currentinputs/userwork.

Priorverifiedfix evidence inindividualreports;finalnative/WASMcompatibility/criterion audit/precommit affected-scene recordings OPEN.NoPR10commitpushauthorization.

## Next actions

NextUI14HullvsHullauto staleSATcache. NativeStep forces6/7/8 for manualfeature butauto branch leaveslastmanualcachetype. ResetSATcacheonce whenactualautoRadioButton transition returns true;preserveautomaticreusethereafter. CloselyrelatedTrianglevsHullnotdemonstratedfault butsharedgeneratedUI transition canclear staleforcedcache;verifybothactualSATpairs/all5routes includingmanual→auto andsubsequentautomaticreuse/off/on/geometry/defaults/restart. Freezefinitebudget first;reuseTSnativeManifoldgenerator/currentUI13pool/actualtoolobserver/priorbefore. ThenUI15–29/UI03/OBS/finalaudit;noGTK/cold/core/solver/exhaustedexperiments. ContinueACTIVEgoaluntilPR10complete,thenstop.

UI14FROZEN1CPP35s/1TSgen10s/5links25s/5apps40s/20screens/120actions/400souter/0physics/retries/1GiBwholeRaw64MiBretained/cache16MiBapp32MiBtotal. AutoRadioButtonactualclick clearsSATcacheonce; verify6/7/8→auto/automaticreuse/off/on/restart/defaults/geometry bothSATpairs/all5routes. PriororiginalCPUfirststale8 evidence reused;Triangleunaffected before.15open14verified.

UI14CPUcompleted11s actualmanual6/7/8→auto type3 /subsequentreuse/manifold/geometry/restartpasses.2falsecacheonfull-equalityprobes includedhitflag:originalofffreshhit0,actualreusedhit1/axis+separationequal. ui14-cache-hit-probe-adjudication.json same-dataPASS/originalflags/source retained;noCPUrerun/productchange. Fourunrunroutes completionfrozen0compile/gen/4links/apps underoriginaltotalbudget.15open14verified.
