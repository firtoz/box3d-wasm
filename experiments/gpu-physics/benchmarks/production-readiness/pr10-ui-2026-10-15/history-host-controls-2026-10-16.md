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
| Sample widgets | **247 reviewed / 95 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **143 reviewed / 22 left** | sample-applicability.md/json |
| Custom input sample groups | **13 reviewed / 10 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **23 open; 2 earlier fixes verified locally** | issues.json; UI01–UI25 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 143 sample dispositions comprise 49 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 143 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest geometry-controls-acceptance.json covers17widgetreviewdispositions/3samples: MeshHeightField9,MeshViewer7,BenchmarkHeightField1 onall5routes, withdefects/limits. Actualheightfield dimensions/scale/holes/groundreplacement/bounds/queryinputs/ray-vs-spherebranch/preview; meshfile/buildpolicy/nodeoverlay/identity/restart. MeshViewerall5routespass. Queryinputs/callbackcounts inspected, notPR11numericalqueryqualification. All49capturedimagesinspected, including46actualgridcells/4missing-captureplaceholders and3individualimages.

UI24all4GPUroutes: amplitude0 usesnativeb3CreateGridscale[2,0,2]; GPUvalidationworld.rs rejectsallscalecomponents<=0, returnsnullshape andworldshapecount0. CPUflatfloorvalid; combinedCPUleftfloorstays whileGPUrightfloorvanishes. Positive.8mainreview restoresfloor. flat-heightfield-before-grid.png ispreservedCPU-firstbefore-repair evidence. Otherflatheightfieldcases source-applicable/unreviewed. Noenginechange.

UI25all4GPUroutes: afterzeroamplitude, xdotooltypes.75 at1msdelay thenReturn; actualUIvalue.700000andheightfieldscaleY1.39999998. CPUapplies.75. Exactqueuedcharacter/ImGuifocus/loadingcauseUNTRACED; immediategeometryrebuild/loadingconsuming5isHYPOTHESISonly. PossibleUI12relationshipunproven. OriginalcaptureassertionstoppedbeforeGPUpositiveimage; donotclaimimageor.75state. Needtraceboundarybeforerepair; noalltypingrateclaim.

FiveoriginalHeightFieldrestartpredicatefailuresareharnessonly: serializedradius.200000003 differsfromexactMath.fround(.2) under===; actualdefaults/dimensions/scale correctatexisting1e-5. Originalreceiptsretained, no replay. Mainmatrix15apps closedwith13findingevents(8UI24+5harness), plus4UI25events inextra5-appbeforecapture.

BenchmarkRadius0/.4/.1actualeffects passCPU/standaloneGPU. Combinedoriginal3sUIreplytimeoutaftertyping0, butAPItailincludesreturnedRayClosestobservations/requestsnapshot; donotclaimdeadlock. Newfinite2-app100s/30sUIreplyprotocol confirmsactualray0transition/terrainpreservation, thenexhausts100swhileeditingbacktopositive. Returnaction8.6–8.8s, activation25.6s measured; post-raypositiveedit/reset/quitUNQUALIFIED. Source/HUD14278casts per sampleRender; cumulativecallcountsincludebothpane renders/otherobservedcalls, notautomaticallyoneframe. RecordPR08performanceoutlier; nofurtherretry, noUI26orquerynumericalcampaign.

Sessions6349(build14.167s),51600(main15appsmatrixterminal1),70874(flatcaptureterminal1),12687(slowinputterminal1),4736(closeout0) CLOSED. All44ownedPIDsabsent/no matchingcores/cachegrowth0. Cleanupdeleted201755883bytes, raw639674482bytes (~1.7TBfree). Mainruns22774105bytes,flat1137844,slow1396610, eachwithinitsfrozenbudget. Oldmeshfiveviewers andoldmesh/body-propertiesobserverobjectsdeleted(no207pinconsumer); originalsources/receiptsremain.

Currentqualifiedpool **geometry-controls-build.json**, fiveviewers raw/geometry-controls-build/. PASSbuildpins/includeshashes verified;207originalinputhashesunchanged. TheoldPERF06querytimingobserverandits5wrapflagsareomittedfromnewlinks; request-summarizedquerywrapperscalloriginalphysics onceandpreservecallbackcontext/returnvalues. NoRust/WGSL/Box3D/WASM edits. Includesheightfield/meshactualgetters,geometryconstruction,CastShape/RayClosestinputs andDrawBounds/preview. PassbuildFileexplicitly; defaultoldgeometrypool isdeleted.

Retaincurrentpool forremainingcontrols untilqualifiedreplacement, existing207link/preflightinputs andCPUReplayfixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec. Rawlocallyignored;preserveuserwork/sharedcaches. Earliermesh/launch/bodyproperties/joint/Driving/sharedreviewsremainvalidwithknownUI03–UI23defects andCapsuleSTARTUP01exclusion. Histories: history-geometry-controls-2026-10-15.md →history-mesh-controls-2026-10-15.md andearlierlinkedhistories. Do notresumeexhaustedruns/coreextraction/coldcampaigns.

## Next actions

Continue **95remainingwidgetentries /10custominputgroups across22samples**, thenrepairdemonstratedcausesoneatatime. Source-first,NEWfinitebudgets,TypeScript/Bun. Currentpool cancheckIssuesCrash(AddJoint) withGPU_PR10_JOINT_TRACE andCompoundVillageKeyboardT/camerastate withpreview/worldlabels; readsourceandactualcountsfirst. VillagecustomhandleronlyT(togglethirdperson), moverpolling/sharedinputhasseparatescope. Broaderremaininggroups needadditionalread-onlyboundaryobservations: Tree12; ReplayViewer29unreviewedentries(36totalincl7earliertransport); Character/movers/planeSolve/custominput; CollisionCastWorld10,InitialOverlap1,LongRay2,MeshScale6,OverlapWorldmouse,ShapeCast1+mouse,BodiesCastmouse; MeshDrop5; EventsContacttorque1; GhostCollisions3; BenchmarkFallingTrees3; StaticInvoke4/Wind5; OverlapRecovery7. Donotclaimunusedconditionalbranchespassed.

DoNOTretryBenchmarkHeightFieldcombinedpost-ray100sprotocol orCapsuleshadercoldcampaign. PR10finalacceptance mustretaintheseunqualifiedreadiness/performancebranches; noimplicitwhole-gatepass. NoPR11qualification,unrelatedsolverrepair,commitorpushauthorization. KeepgoalACTIVEandPR10unchecked; stopafterPR10.

Joint/camera controls terminal; inspect joint-camera-controls-matrix.json beforedisposition.

Activejoint/camera batch: joint-camera-controls-budget.json frozen0builds/10appsCPUfirst/100sapp/2screens/0physicssteps/retries/cores/1GiBraw/32MiBretained. Session41603RUNNING; joint-camera-controls-matrix.jsonauthority. Reusegeometry-controls-build.json. Crashactualweldcreation/repeat/restart; VillageTpress/release/HUD/camerafollow/identity atupstreamRelease52500children(defaultunchanged), virtualmoverstilladvancesperUIframebyupstreamdesign. ExistingPR09Villagebehavior/queriesnotrerunqualified; no trajectorymatchclaim. Counts247/95widgets,143/22samples,13/10custom,23open/2verified pendingcloseout.

Hostcontrolsmatrixterminal; inspect host-controls-matrix.json beforedisposition.

Joint/camera41603CLOSED0/all10appsconsumed. CrashAddJointactualall5passes. VillageoriginalHUDcheckswronglyreadworld_labels (DrawTextLineisscreen-space); Treleaseempty.every()originaltruechecksarevacuous/notevidence. CPU/combinedfollowpredicatecomparedroundedInfoatdifferentframes (.375lag), combinedtoolcapturecanbeempty; donotcallproductdefectorfeaturepass. Needpropercamera/moverflags/screenHUDsameframe.
Hostobserver69086CLOSED0/14.369scompiledbutREJECTEDbefore0apps: unusedprefixexpressionpassed2pointAABBto3pointtriangleserializer,potentialOOBstaticfound. host-controls-build-rejected.json storesfragment;166677408unusedbinary/objectbytesdeleted(host-controls-rejected-cleanup.json). Bunstdinpreflightattemptfailedbeforefilesystemmutation/build. Correctivehost-controls-corrected-build-budget.json frozen1CPP+5links,28515CLOSED0/14.352s; currenthost-controls-build.jsonnewraw/host-controls-corrected-build pool,207originalhashesunchanged. CameraUpdate/CharacterMoverStepread-onlystate,screenHUDandDynamicTreeoperations; noengineedit.
Host-controls-budget.json frozen10appsCPUfirst/100sapp/3screens/0steps/retries/cores/1GiBraw/64MiBretained; session49733RUNNING,host-controls-matrix.jsonauthority. TreeSave/Loadonlyprivatetree.dt<=8MiB,hash/extract/deleteperterminalapp. Currentcounts247/95widgets,143/22samples,13/10custom,23open/2verified untilcloseout. Originalgeometryqualifiedpoolretaineduntilnewall5routesqualify.

Hostcontrolsmatrixterminal; inspect host-controls-qualified-matrix.json beforedisposition.

Hostcorrected28515buildwasusableforCPUVillagePASSbutTreeobserverwatchedemptyphysics-worldpicking/cullingquerytree; CPUpreflight49733terminal1after2apps,8GPUappsnotlaunched. Treeactual288boundsdrawn, observed_tree overwrittenempty; noproductdefect. Originalhost-controls-build-unscoped.json andhost-controls-matrix.json preserved;166680736supersededbinary/objectbytesdeleted. Newhost-controls-scoped-build-budget.json1CPP+5links66707CLOSED0/14.502s,filtersnonemptytrees,original207hashesunchanged. Currenthost-controls-build.json usesraw/host-controls-scoped-build. Newhost-controls-qualified-budget.json9appsCPUtreefirst,existingCPUVillagepasscarried; session42100RUNNING/partialpasses,originalnull/aliasfailureretained. AttemptedcullingfixturepreflightBunevalsyntaxfailedbeforefilemutation; currentrunningdriverunchanged. MainKilometers.5/20bothdraw288becausedatasetwithin500m,nondecreasingpredicateinsufficient. Newtree-culling-budget.jsonfrozen5apps60s/2screens/0buildsteps tocoverONLYmissingeffectwith12realwheelzoomout/restore; notlauncheduntil42100terminal. Counts247/95widgets,143/22samples,13/10custom,23open/2verified untilallcloseout.
