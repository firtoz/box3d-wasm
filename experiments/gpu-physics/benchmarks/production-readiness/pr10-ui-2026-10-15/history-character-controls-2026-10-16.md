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
| Sample widgets | **278 reviewed / 64 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **150 reviewed / 15 left** | sample-applicability.md/json |
| Custom input sample groups | **14 reviewed / 9 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **25 open; 2 earlier fixes verified locally** | issues.json; UI01–UI27 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 150 sample dispositions comprise 56 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 150 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest: query-input-controls-acceptance.json, seven widgets/two samples on all five routes. Mesh Scale changes actual signed scales, bounds, query origins/radius/branch and retains body/shape identity. Zero scale confirms UI23 on all three axes and all four GPU routes; combined CPU GetMesh provenance still cannot certify GPU geometry. UI27: with query origin[-2,1,1] and translation[4,0,0], actual missed sphere preview is[4,0,0] instead of[2,1,1]. Shared native sample drawing omits m_start. Miss branch demonstrated all five; hit branch source-applicable, needs repair-stage verification. Contact torque0/30000/60000 reaches actual ApplyTorque with expected vector magnitude, body, owner and wake; W release stops calls. Paused/no physics: no rolling/contact-event/query-numerical qualification. All30capturedimages inspected CPU first.

Session96794 CLOSED0/10apps; closeout80442 CLOSED0. All20ownedPIDs absent,0cores/cachegrowth0. Cleanup deleted17224928bytes; retained runs13181990bytes; raw703085197bytes; free1708521918464bytes. Current qualified pool **scene-controls-build.json**, raw/scene-controls-corrected-build;207originalinput hashes unchanged. Pass buildFile explicitly. Old host/geometry pools deleted. Current pool has read-only tuning/torque/wind, geometry/query/body and host camera/mover observers for remaining controls. No Rust/WGSL/Box3D/WASM change.

Earlier scene-controls-acceptance.json covers11widgets/2samples: Overlap Recovery7 and Static Invoke4, all five routes;37images inspected. UI25 also occurs for Extent.25→.2 on all4GPUroutes while CPU applies.25. Actual hull bounds prove it; common cause with HeightField remains UNTRACED. Remaining controls pass using one-character settings; no typing-rate claim or repair. Actual contact tuning API inputs/geometry/recreation/restart and deferred invoke flags/create/destroy/IDs checked while paused. All15apps/30ownedPIDs absent; superseded host pool removed. Observer compile declaration/field failure archived before any viewer used it. See history-scene-controls-2026-10-16.md.

Earlier host-controls-acceptance.json covers13widgets/3samples and Village T: actual weld IDs/restart, direct camera/mover/screenHUD and all Tree controls including culling0→288 and Save/Load scale,60images inspected. UI26 missing Tree Load empties live tree, hides recovery controls and shows no UI error; global restart recovers288proxies. All10negative images inspected. Native host Tree is not GPU broadphase qualification. Village52500children/defaults retained. Original HUD/follow/observer failures adjudicated as harness errors, never product fixes.

Keep earlier UI24 flat HeightField scaleY0 floor loss, UI25 HeightField.75→.7 and combined Benchmark post-ray100s performance exclusions open. Read geometry-controls-acceptance.json and linked histories. Do NOT resume Capsule cold startup, Benchmark post-ray, core extraction, solver or PR11 campaigns. UI12 protected~3MB old core could not be deleted without sudo;204MBpartial already deleted. Owned viewers use core0. Original207pins and CPUReplayfixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec retained for remaining Replay. Preserve shared caches/user work; raw ignored.

## Next actions

Continue **64widgetentries/9custominputgroups across15samples**, then fix demonstrated causes one at a time. Remaining: FallingTrees3; BodiesCast mouse; CapsulePlane1+mouse; BasicMover2+Keyboard; MoverOverlap mouse; RigidBody2+Keyboard; CastWorld10+mouse; InitialOverlap1; LongRay2; OverlapWorld mouse; ShapeCast1+mouse; MeshDrop5; GhostCollisions3; ReplayViewer29conditionalwidgets+handlers/focus/layout; Wind5. Current pool can check character camera/clip/debug inputs; source first and freeze new finite budgets. Finish review before repair. Keep goal ACTIVE and PR10 unchecked. No PR10 commit/push authorization.

Active character batch: character-controls-budget.json, zero builds/10 apps/60s each/3screens/0physics/core0/no retries. Session62914 exact handle; character-controls-matrix.json authoritative. Camera/clip/debug/T/V and restart/exit controls only; virtual mover still advances per UI frame. Counts278/64widgets,150/15samples,14/9custom,25open/2verified pending disposition.

Character matrix terminal; inspectcharacter-controls-matrix.json beforedisposition.

Character62914CLOSED0/10apps, closeoutcomplete. All20PIDsabsent/core0/cachegrowth0. Fourwidgets/twoKeyboardgroups actualall5pass;30imagespendinginspection/inventorywrite. Cleanup33479634bytesdeleted/raw726623961. Counts278/64widgets,150/15samples,14/9custom,25open/2verified pendingdisposition. Currentpoolscene-controls-build.json.
