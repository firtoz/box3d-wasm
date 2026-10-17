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
| Sample widgets | **260 reviewed / 82 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **146 reviewed / 19 left** | sample-applicability.md/json |
| Custom input sample groups | **14 reviewed / 9 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **24 open; 2 earlier fixes verified locally** | issues.json; UI01–UI26 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 146 sample dispositions comprise 52 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 146 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest host-controls-acceptance.json covers13widgets/3samples plusVillageKeyboardT: Crash1,Tree12,Village0. Actualweldcreation/IDs/restart, nativecamera.third_person/moverpivot/screenHUD, treefile/rebuild/query/profile/level/cull/save/load-scale all5routes. All60positiveimagesinspectedCPUfirst. TreeisnativehostCdynamicTreeonallroutes, notGPUbroadphasequalification. Village52500compoundchildrenandupstreamdefaults preserved; virtualmoveradvancesperUIframeevenpaused, no trajectorymatchclaim.

Original18VillageHUD/followpredicates areharnesserrors: screenHUDwasreadasworldlabels; empty.everypassesvacuous; roundedInfoandmoverweredifferentframes. Directsame-frameobserver supersedeswithoutproductfix. Firsthostbuildrejectedbefore0apps(potentialunusedAABBserializerOOB); secondhostTreepreflightwatchedemptyworldquerytree. Bothsupersededpoolsdeleted; archivedmanifests/failures retained. Thirdscopedhostpoolvalidall5routes. CPUVillagepassreusedwithreconstructedincludehashproof. Mainculling.5/20both288insufficient; focusedactualwheelzoomradius~784shows0vs288withouttreechangesall5.

UI26confirmedall5routes: privatecwdmissingtree.dtLoadreturnsemptytree, deletescurrenttreeandhidesFile/recoverycontrols, noUIerror; globalRrestores288proxies. tree-missing-file-acceptance.jsonall10before/recoveryimagesinspectedCPUfirst. This issharedsamplehostfailure, noBox3D/enginechange. PositiveLoad/Saveall5alreadyqualified.

Sessions41603(original10apps),49733(unscoped2apps),42100(qualified9apps),31315(culling5apps),29566(closeout)CLOSED. Missing-filematrixREVIEW_COMPLETE5apps, exactsessionlostacrosscompactionbutall10PIDsabsent/streamcleanupterminal. Positivecleanup527428016bytesdeleted/raw667951274bytes atthatreceipt. Latestnegativecleanup/tree-missing-file-cleanup.json recordscurrentbytes/cores/cache/free; all31appsclosed. Currentqualifiedpool **host-controls-build.json**, raw/host-controls-scoped-build;207originalhashesunchanged. PassbuildFileexplicitly; oldgeometrypoolbinaries/objectdeleted. Currentpoolretainedforremainingcontrols. NoRust/WGSL/Box3D/WASMchanges.

EarlierUI24flatflooractualscaleY0rejection/UI25.75→.7inputfailuresandcombinedBenchmarkHeightFieldpost-ray100sperformanceexclusion remainopen; readgeometry-controls-acceptance.json/history-geometry-controls-2026-10-15.md. DoNOTretryCapsulecoldstartup, Benchmarkpost-ray, coreextraction, solverorPR11campaigns. UI12protected~3MBoldcorecannotremovewithoutsudo; extracted204MBpartialalreadydeleted. Allcurrentapps core0. Original207pins andCPUReplayfixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec stillnamedconsumers. Preserve sharedcaches/userwork; rawignored.

## Next actions

Continue **82widgetentries/9custominputgroups across19samples**, source-firstfinitebudgets/TypeScriptBun. Remaining: BenchmarkFallingTrees3; BodiesCastmouse; CharacterCapsulePlane1+mouse,BasicMover2+Keyboard,MoverOverlapmouse,RigidBody2+Keyboard; CollisionCastWorld10+mouse,InitialOverlap1,LongRay2,MeshScale6,OverlapWorldmouse,ShapeCast1+mouse; ContinuousMeshDrop5; EventsContacttorque1; GhostCollisions3; ReplayViewer29conditionalwidgets+handlers/focus/layout; RobustnessOverlapRecovery7; ShapesStaticInvoke4/Wind5. Finishreviewbeforerepair, nofullsweepreplay. KeepgoalACTIVE/PR10unchecked; noPR10commit/pushauthorization.

Scenecontrols matrix terminal; inspectscene-controls-matrix.json beforedisposition.

Scenecompile75938CLOSED1 before0links/apps; missingmappedCPUdeclaration/redundanthullfield errors retained. Corrected62001CLOSED0/14.625s5links,207pinsunchanged. Main4110CLOSED1: CPURecovery7allpass, StaticInvoke4all5pass; firstExtent.25textapplies.2all4GPUroutes(actualhullhalfextent.2). UI25relatedmanifestation, sharedcauseUNTRACED. RemainingGPURecoverycontrolsnotreached. Newscene-controls-remaining-budget.json5apps60s/3screens/0buildsteps freezesmissingbeforecapture plusunreachedcontrolsusing1charactersettings. Counts260/82widgets,146/19samples,14/9custom,24open/2verified pendingcloseout.

Scenecontrols matrix terminal; inspectscene-controls-remaining-matrix.json beforedisposition.

Scenecloseoutcomplete;15apps/30ownedPIDsabsent,0cores/cachegrowth0. Currentpoolscene-controls-build.json/corrected; hostscoped5binaries/objectretired. Elevennewwidgetdispositions pending30gridimageinspection/inventorywrite. UI25newOverlapRecoveryextent.25→.2manifestationall4GPUroutes/CPUcorrect,commoncauseUNTRACED. StaticInvokeall5positive. scene-controls-cleanup.jsonauthorityforbytes.
