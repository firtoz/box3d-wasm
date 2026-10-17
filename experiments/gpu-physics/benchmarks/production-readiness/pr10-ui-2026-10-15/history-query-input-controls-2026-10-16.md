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
| Sample widgets | **271 reviewed / 71 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **148 reviewed / 17 left** | sample-applicability.md/json |
| Custom input sample groups | **14 reviewed / 9 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **24 open; 2 earlier fixes verified locally** | issues.json; UI01–UI26 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 148 sample dispositions comprise 54 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 148 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latestscene-controls-acceptance.json adds11widgets/2samples: OverlapRecovery7,StaticInvoke4. Actualhullbounds/count/positions/overlap/contacttuningAPIinputs/recreation/restart anddeferredinvokecreationflag/spherecreate/destroy/identity all5routes. Paused0physics, notsolver/contactqualification. All37capturedimagesinspectedCPUfirst(30completedcells+7originalcells;8UNREACHEDplaceholders). UI25alsoOverlapRecoveryExtent.25→.2all4GPUroutes/CPUcorrect. Actualhullhalfextentconfirmsappliedvalue; commoncausewithHeightFieldUNTRACED. Remainingcontrols checkedusingonecharactersettings; norepair/typing-rateclaim.

Sceneinitialbuild75938CLOSED1 before0links/apps(missingmappedCPUdeclaration/redundanthullfield), rejectionmanifest/source/logretained. Corrected62001CLOSED0/14.625s; main4110CLOSED1/10apps(4originalExtentfailures), remaining35758CLOSED0/5apps(4reproducedUI25findings, allpreviouslyunreachedcontrolschecked). Closeout98833CLOSED0. All30ownedPIDsabsent/0cores/cachegrowth0. scene-controls-cleanup.json deleted198049949bytes/raw689903207bytes/free1708477874176bytes; originalruns8372422/remaining9564467retained. Supersededhostscoped5viewers/object deleted166668216bytes. Currentpool **scene-controls-build.json**/raw/scene-controls-corrected-build, 207originalinputsunchanged. PassbuildFileexplicitly; oldhost/geometrybinarypoolsdeleted. Currentpoolhasread-onlycontacttuning/torque/windinputs, geometry/query/body/hostcamera/moverobservations; namedconsumerremainingcontrols. NoRust/WGSL/Box3D/WASMedit.

Earlierhost-controls-acceptance.json13widgets/3samples plusVillageTinputall5;60imagesinspected. Tree12controlspositiveeffects inclactualcull0→288, Save/Loadscale; CrashweldIDs/restart; Villageactualcamera/mover/HUD,52500compoundchildren/defaults preserved. UI26negativeTreeLoademptieslivetree/hidesFilecontrols/noUIerrorall5; globalrestartrecovers288proxies, all10imagesinspectedCPUfirst/tree-missing-file-acceptance.json. Allpriorjobsclosed. Original18VillageHUD/followpredicates/emptytreeobserveraliasareharnesserrors, archived; neverproductfix. Seehistory-host-controls-2026-10-16.md.

UI24flatHeightFieldscaleY0lossandUI25HeightField.75→.7inputsremainopen; combinedBenchmarkHeightFieldpost-ray100sperformanceexclusion remainsunqualified. Readgeometry-controls-acceptance.json/history-geometry-controls-2026-10-15.md. DoNOTretryCapsulecoldstartup/Benchmarkpost-ray/coreextraction/solver/PR11campaigns. UI12protected~3MBoldcorecannotremove withoutsudo;204MBpartialalreadydeleted. Currentapps core0. Original207pins/CPUReplayfixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec retainedforremainingReplay. Preserve sharedcaches/userwork; rawignored.

## Next actions

Continue **71widgetentries/9custominputgroups across17samples** source-firstwithNEWfinitebudgets/TypeScriptBun. RemainingBenchmarkFallingTrees3;BodiesCastmouse;CharacterCapsulePlane1+mouse,BasicMover2+Keyboard,MoverOverlapmouse,RigidBody2+Keyboard;CollisionCastWorld10+mouse,InitialOverlap1,LongRay2,MeshScale6,OverlapWorldmouse,ShapeCast1+mouse;ContinuousMeshDrop5;EventsContacttorque1;GhostCollisions3;ReplayViewer29conditionalwidgets+handlers/focus/layout;ShapesWind5. CurrentpoolcancheckContacttorqueAPIwhilepaused, MeshScalequeryinputs/getters/spherepreview; readsourcefirst. Finishreviewbeforerepair. KeepgoalACTIVE/PR10unchecked; noPR10commit/pushauthorization.

Queryinputcontrols matrix terminal; inspectquery-input-controls-matrix.json beforedisposition.

Queryinput96794CLOSED0/10apps, closeoutterminal. All20ownedPIDsabsent/0cores/cachegrowth0. UI27misssphereprevieworiginomittedconfirmedall5, nofix; UI23zeroMeshScaleall3axesall4GPUroutes. Contacttorqueall5passat0/30000/60000actualAPI/Wrelease/IDs whilepaused. Sevenwidgetdispositions pending30imageinspection/inventorywrite; counts271/71widgets,148/17samples,14/9custom,25open/2verified. Currentpoolscene-controls-build.jsonunchanged;cleanupquery-input-controls-cleanup.json, raw703085197.
