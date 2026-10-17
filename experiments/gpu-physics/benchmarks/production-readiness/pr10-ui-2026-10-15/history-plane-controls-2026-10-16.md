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
| Sample widgets | **282 reviewed / 60 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **152 reviewed / 13 left** | sample-applicability.md/json |
| Custom input sample groups | **16 reviewed / 7 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **25 open; 2 earlier fixes verified locally** | issues.json; UI01–UI27 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 152 sample dispositions comprise 58 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 152 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest: character-controls-acceptance.json covers four widgets and two Keyboard groups, both character samples on all five routes. Camera checkbox/T down/up uses actual flag and same-frame mover/body pivot; Clip Velocity reaches actual mover Step argument; Debug checkbox/V removes and restores actual lines/points. IDs/poses preserved; restart restores defaults; sample exit resets orbit mode. All30images inspected CPU first. Paused/no world physics; native character logic and virtual mover still run perUIframe. No movement/jump/sprint physical or query-numerical qualification.

Sessions62914(main10apps) and98506(closeout) CLOSED0. All20ownedPIDs absent;0cores/cachegrowth0. Cleanup deleted33479634bytes, retained23538764, raw726623961, free1708501671936. Current qualified pool **scene-controls-build.json**, raw/scene-controls-corrected-build;207originalinput hashes unchanged. Pass buildFile explicitly; old host/geometry pools deleted. Current pool has read-only tuning/torque/wind, geometry/query/body and camera/mover observations for remaining controls. No Rust/WGSL/Box3D/WASM changes.

Previous query-input-controls-acceptance.json: seven widgets/two samples,30images inspected, all10apps closed/20PIDs absent. Mesh Scale actual signed scales/bounds/origins/ray-sphere branch/identity/restart checked. UI23 zero scale fails all3axes/all4GPUroutes; combined CPU GetMesh does not proveGPUstate. UI27 actual miss preview omits query origin: [4,0,0] instead of[2,1,1] at origin[-2,1,1]/translation[4,0,0], all5routes. Hit branch source-applicable, needs repair verification. Contact torque0/30000/60000 actual API magnitude/body/wake/Wrelease checked while paused; no rolling/contactevent pass. history-query-input-controls-2026-10-16.md.

Previous scene-controls-acceptance.json:11widgets/2samples,37images inspected. Actual Overlap Recovery geometry/count/overlap positions/tuning API inputs/recreation/restart and Static Invoke deferred flags/create/destroy/IDs all5. UI25 also Extent.25→.2all4GPUroutes/CPUcorrect, actual hull bounds confirm; common cause with HeightField UNTRACED. Remaining controls pass with one-character settings, no repair/typing-rateclaim. Observer compile declaration/field rejection retained before0apps; corrected pool qualified all5. All15apps/30PIDs absent, old host pool removed. history-scene-controls-2026-10-16.md.

Earlier host-controls-acceptance.json:13widgets/3samples plusVillageT,60images inspected. CrashweldIDs/restart; directcamera/mover/screenHUD; Tree12controls inclcull0→288andSave/Loadscale. UI26missingTreeLoademptieslivetree/hidesrecovery/noUIerrorall5; globalrestartrecovers288proxies, all10imagesinspected. NativehostTreeisnotGPUbroadphasequalification. Village52500children/defaults retained. OriginalHUD/follow/observer failures are archived harness errors, notproductfixes.

Keep earlier UI24 flat HeightField scaleY0 floor loss, UI25 HeightField.75→.7 and combined Benchmark post-ray100s performance exclusions open. Read geometry-controls-acceptance.json and linked histories. Do NOT resume Capsule cold startup, Benchmark post-ray, core extraction, solver or PR11 campaigns. UI12 protected~3MB old core could not be deleted without sudo;204MBpartial already deleted. Owned viewers use core0. Original207pins and CPUReplayfixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec retained for remaining Replay. Preserve shared caches/user work; raw ignored.

## Next actions

Continue **60widgetentries/7custominputgroups across13samples**, then repair demonstrated causes one at a time. Remaining: FallingTrees3; BodiesCast mouse; CapsulePlane1+mouse; MoverOverlap mouse; CastWorld10+mouse; InitialOverlap1; LongRay2; OverlapWorld mouse; ShapeCast1+mouse; MeshDrop5; GhostCollisions3; ReplayViewer29conditionalwidgets+handlers/focus/layout; Wind5.

Next source-first batch: read-only capsule plane/solve and body-cast input observers for CapsulePlane, MoverOverlap and BodiesCast mouse handlers. No budget frozen or job launched yet. Extend current observers only where an existing trace cannot show intended inputs/effects; preserve original207pins. Then cast/overlap controls, remaining mutation/wind and conditional CPUReplay fixture. Do not replay completed character controls. Finish review before repair. Keep goal ACTIVE/PR10 unchecked; no PR10 commit/push authorization.

Plane controls budgets frozen: oneCPP30s/five25slinks, then15apps60s/3screens/0worldsteps/core0/noretries/1GiBraw/64MiBretained. Read-only actual movercallbacks/solveinput-output/bodycasttransforms for CapsulePlane/MoverOverlap/BodiesCast mouse. No new productfix orPR11numericclaim. Counts282/60widgets,152/13samples,16/7custom,25open/2verified. Buildnotyetlaunched atpreparewrite.

Plane observer4485CLOSED0/PASS15.172s(onecompile/fivelinks), original207inputhashes unchanged. Currentcandidateplane-controls-build.json/raw/plane-controls-build; previousqualifiedscene pool retained until newall5routesqualify. plane-controls.ts launched once; plane-controls-matrix.json authoritativecurrentdriver/app receipts. Counts282/60widgets,152/13samples,16/7custom,25open/2verified pendingcloseout.

Plane mousecontrols matrix terminal; inspectplane-controls-matrix.json beforedisposition.

Plane24474CLOSED1/15apps. CPU/standaloneall9positivepass; combinedPlane/Movercontrolscomplete. OrdinarycombinedCapsuleinitialdrawpredicate sawemptytool_shapes atrequest27; firstSolve.before suppliesfreshcapsuleatunchanged.4origin, no productfinding. CombinedBodyCast2preflightassertions wronglyexpectedCPU+GPUquerycalls; both_dual.c3038/3044 dispatchesonlyGPUpublic, actualtraces1 vs2worlds. Twoappsfailedbeforeanymouseinput. Newplane-controls-remaining-budget.json2apps60s/3screens/0buildphysics freezesONLYunreachedcombinedBodyCast withcount1, originalCPU/standalonepassesreused. Counts282/60widgets,152/13samples,16/7custom,25open/2verified pendingcloseout.

Plane mousecontrols matrix terminal; inspectplane-controls-remaining-matrix.json beforedisposition.

Planecloseoutterminal:17apps/34ownedPIDsabsent/0cores/cachegrowth0. Onewidget/3mousegroupsall5 actualeffects, positiveMoverOverlapplanes encounteredall5. Threeharnessconditionsadjudicated, no productfix. All45imagespendinginspection/inventorywrite. Currentpoolplane-controls-build.json, supersededscene5binaries/objectdeleted166714928bytes. Cleanup177299145bytes/raw741295347. Counts282/60widgets,152/13samples,16/7custom,25open/2verified pendingdisposition.
