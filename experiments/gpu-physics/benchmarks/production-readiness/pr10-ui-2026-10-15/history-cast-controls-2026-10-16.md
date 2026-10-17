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
| Sample widgets | **283 reviewed / 59 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **155 reviewed / 10 left** | sample-applicability.md/json |
| Custom input sample groups | **19 reviewed / 4 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **25 open; 2 earlier fixes verified locally** | issues.json; UI01–UI27 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 155 sample dispositions comprise 61 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 155 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest plane-controls-acceptance.json: one Solve widget/three custom mouse groups across CapsulePlane, MoverOverlap and Bodies/Cast, all five routes. Actual plane callbacks/solver inputs and returned delta map to the changed query/capsule pose. MoverOverlap positive planes encountered all5; yellow queried and cyan solved poses checked. Mouse translation/release/Alt-ignore pass. Bodies/Cast accepts Shift only; actual ray+shape body transforms and rendered preview change together, Control/unmodified/AltShift are ignored, worldbody poses/IDs stay fixed and restart restores defaults. Combined BodyCast queries dispatch only GPU/public (both_dual.c3038/3044), no mappedCPUquerycall claim. No full query numeric qualification. All45images inspected CPU first.

OBS01, unconfirmed appearance flag: small shape appears pale/white in combined BodyCast Shift captures versus blue in standalone. Shape/color/selection provenance was not captured. Record in plane-controls-acceptance.json/inventory BodyCast; do not silently certify color or add a confirmed repair without tracing. Triage before final PR10 acceptance, preferably using existing actual_body_metadata and a bounded DrawHull color/transform observation if saved/source evidence is insufficient. Counts remain25confirmed open/2verified; OBS01 is one additional unconfirmed flag.

Build4485 CLOSED0/15.172s,17apps main24474 CLOSED1 and remaining99400 CLOSED0; closeout16505 CLOSED0. Originaltwo combined BodyCast preflight assertions expected one query perworld, contradicted by source/actual trace; only previouslyunreached2combinedcases run under newbudget. One ordinarycombined initial emptytool_shapes atrequest27 is capturephase; firstSolve.before request31 has capsule at unchanged[0,1,.4], later actualdrawchecks pass. Original failures retained, no productrepair. All34ownedPIDs absent/0cores/cachegrowth0. Cleanup deleted177299145bytes, retained original12394754/remaining2054672, raw741295347, free1708470771712. Superseded scene pool removed166714928bytes.

Current qualified pool **plane-controls-build.json**, raw/plane-controls-build;207originalinput hashes unchanged. Pass buildFile explicitly. Old scene/host/geometry binary/object pools deleted. Current read-only observers include CollideMover callback forwarding, SolvePlanes input/output and BodyCast transform inputs plus existing tuning/torque/wind/geometry/query/body/camera/mover state. Each original/callback invoked once, return/context preserved. No Rust/WGSL/Box3D/WASM edit. Retain current pool for remaining controls until qualified replacement.

Earlier character-controls-acceptance.json: four widgets/twoKeyboardgroups,30images inspected, all10apps/20PIDs absent. Actual camera flag/pivot, ClipVelocity Step argument, Debug/V lines/points, T/V down/up, IDs/poses/defaultrestart/sampleexit pass all5. Virtual/nativecharacter logic still runs perUIframe with pausedworldphysics; no WASD/jump/sprint physical claim. history-character-controls-2026-10-16.md.

Earlier query-input-controls-acceptance.json: seven widgets/two samples,30images inspected. MeshScale signedscale/bounds/queryorigin/branch/identity/restart checked; UI23zero all3axes/all4GPUroutes, combinedGetMeshCPUprovenance explicit. UI27 misspreview[4,0,0] omits origin[-2,1,1], endpointshould[2,1,1]; all5. Hitbranch source-applicable, verify duringrepair. Contact torque actual0/30000/60000 API vector/body/wake/Wrelease, no rolling/contactevents claim. Earlier scene-controls-acceptance.json11widgets/two samples/37images: OverlapRecoverygeometry/tuningAPI/recreation andStaticInvoke deferredflag/lifecycle all5. UI25alsoExtent.25→.2all4GPU/CPUcorrect, commoncausewithHeightField UNTRACED. Earlier host13widgets/3samples+VillageT/60images valid; UI26negativeTreeLoad recovery/10images valid. Alljobsclosed; linked histories preserve rejected harness/build inputs.

Keep earlier UI24 flat HeightField scaleY0 floor loss, UI25 HeightField.75→.7 and combined Benchmark post-ray100s performance exclusions open. Read geometry-controls-acceptance.json and linked histories. Do NOT resume Capsule cold startup, Benchmark post-ray, core extraction, solver or PR11 campaigns. UI12 protected~3MB old core could not be deleted without sudo;204MBpartial already deleted. Owned viewers use core0. Original207pins and CPUReplayfixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec retained for remaining Replay. Preserve shared caches/user work; raw ignored.

## Next actions

Continue **59widgetentries/4custominputgroups across10samples**, then repair demonstrated causes one at a time. Remaining: FallingTrees3; CastWorld10+MouseDown; InitialOverlap1; LongRay2; OverlapWorld mouse; ShapeCast1+mouse; MeshDrop5; GhostCollisions3; ReplayViewer29conditionalwidgets+handlers/focus/layout; Wind5.

Next: source-first cast/overlap controls. Existing geometry query snapshot records last CastShape/RayClosest input but not callback initialOverlap/mode/context or WorldOverlapShape. Add read-only bounded callback/context/overlap observations only as needed; pin native CastContext layout to sample_collision.cpp and never inspect another user context. Freeze NEW build/app/check/capture/storage budget before execution. Plane/current pool contains named remaining consumers; no live job or next execution budget. Do not replay completed plane/character controls. Keep OBS01 for finaltriage. Finish review before repair. GoalACTIVE/PR10unchecked; no PR10commit/pushauthorization.

Final continuation audit: git diff --check PASS; box3d submodule clean; no owned PR10 viewer/driver processes present. Derived inventory283reviewed/59left,155sampledispositions/10left=61actualspecificfixtures+94sharedreuses;19custom/4left. UI03–UI27 remain25open (UI25 status keeps cause-pending distinction),UI01/UI02verified,OBS01unconfirmed. Goal tool ACTIVE. Latest cleanup177299145bytesdeleted/raw741295347bytes,34PIDsabsent/cachegrowth0/free1708470771712bytes. All execution budgets closed; next cast/overlap budget not yet frozen. No PR10 commit or push.

Cast budgets FROZEN beforeexecution:1compile30s/5links25s/5symbolreads10s;25apps60s/3screens/0worldphysics/core0/1GiBraw/64MiBretained. CastContext exactsource-derivedlayout, callbackwhitelistfromcurrentbinary beforeanycontextread. Newquerycallback/WorldOverlapShape/cameraprojectionobservations only. Buildnotyetlaunched. Counts283/59widgets,155/10samples,19/4custom,25open/2verified+OBS01unconfirmed.

Cast matrix terminal; inspectcast-controls-matrix.json beforedisposition.

Cast build13285CLOSED0/15.521s and5callbacksymbolreadsPASS. Main35349CLOSED1/25apps:23finish, combinedCastWorld2endat16MiBAPIcap aftertypes/radius/4positiveModes, beforetail. Two newtailapps60s/24MiB/3screens freezeONLYunreachedflags/negativeMouse/Destroy/ring64/restart+missingsortedcapture, nofullcasereplay. OriginalCPU/standalonepassesreused. Reclickoriginfalsepredicates areharnesserrors(sametarget/camera); LongRayMath.cosfalsepredicates mustusenativeCPU reference because b3ComputeCosSin intentionallyapproximate deterministic math. BoxCastactiveSizehiddennewUIfindingall5; InitialOverlapflagreachescontextbutzero-translationcastaccepted0GPU vspositiveCPU, routedPR11queryfinding, noqueryrepair. Counts283/59widgets,155/10samples,19/4custom,25open/2verified+OBS01pendingcloseout.

Cast matrix terminal; inspectcast-controls-tail-matrix.json beforedisposition.

Castcloseout68140CLOSED1 atretainedsizecheck after5montagescomposedonce; no retiredpooldeletion ordispositionyet. Originalruns71438098bytes exceeded64MiBretainedcapby4.3MiB(raw989344809stillunder1GiB). Losslesswhitespace-only5CastWorldreceiptcompaction preservesallJSONvalues/failures/inputs; original/compacthashes+structuralequality in cast-receipt-compaction.json. No budgetincrease/evidencedeletion. ResumeONLYremainingcloseoutchecks fromexistinggridhashes, zeronewcomposition/apps/builds. Counts283/59widgets,155/10samples,19/4custom,25open/2verified pendingUI28registry.

Castcloseoutterminal.14widget/3mousegroupdispositionspending79imageinspection/inventorywrite. UI28hiddenBoxsizeconfirmedall5; PR11zero-translationinitial-overlapquerygap4GPU/CPUpositive, causeuntraced. Main25+tail2apps/54PIDsplus5nmPIDsabsent/0cores/cachegrowth0. Cleanup295569212deleted/raw795402497; retiredplane5binaries/object166936888bytes. Currentpoolcast-controls-build.json, original207pinsunchanged. Counts283/59widgets,155/10samples,19/4custom; now26UIissuesopen/2verified+PR11querygap+OBS01unconfirmed.
