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
| Repairs | **19 open; 10 fixes verified locally** | issues.json; UI01–UI29 reports |
| Final PR10 acceptance | Open | Repairs, resource/compatibility checks and final criterion audit |

The 165 sample dispositions comprise 71 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 165 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping, UI02 wheel routing, UI04 CPU Profile width, UI05 hidden HUD layout, UI06 current body-type transparency/default material and combined CPU ownership, UI07 combined Frame Camera, UI08 Unix Replay basename, UI09 honest Replay availability, UI19 Wheel steering damping and UI20 Driving torque are locally verified fixes. Open: UI03 missing GPU overlays; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size; UI29 Mesh Drop auto regeneration hides controls behind repeated GPU preparation and fails to switch off. See individual reports and `issues.json`. The full review is complete; verified repair evidence follows.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain open for repair.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Fullreview COMPLETE:342widgets/165sample dispositions/23custom groups/85shared calls,0left. **19open,10fixesverified locally.** Verified:UI01/UI02/UI04/UI05/UI06/UI07/UI08/UI09/UI19/UI20. Goal ACTIVE,PR10unchecked.

UI07[x]:[repair](UI07-repair.md)/[acceptance](ui07-acceptance.json). ActualnativeFrame identicalbounds/padding, combinedmenuUIaspect1.7778→radius47.1602 vsFpaneaspect.8889→53.0552; distH35.3702vs70.7403 firstwronginput traced. GeneratedcombinedSample menuusesactualviewportwidthsapp_width()/2; fullwindowUI/projection/picking/F/nativeCameraunchanged. StandaloneSampleobjects unchanged.27before+153repair=180true/0findings;CPU25/ordinaryGPU25/nativeGPU25/ordinaryBOTH39/nativeBOTH39 in4.97/6.13/6.24/8.68/8.91s. Splitoffon/resize1280x800/IDs/pausedposes/defaults/restart/lifetime/actualSingleBoxpickbothpanes checked.0scriptadvancingphysics;5–7startupsteps.23individualimages+2CPUfirstgrids inspected.165sourceapplicabilityrowsnot165newruntimes. Originalobserverstatichelpercompilefailure retained; exactexpectedgeneratorhash delta adjudicatesstaleabsolute manifestpin; noqualifiedapp/media repeated. Seehistory-ui07-execution-2026-10-16.md.

UI06[x]currenttype/opacity/defaultPBR+combinedCPUownership:297truechecks/30images+2grids;UI05[x]hiddenHUD:128positive+2originalwrongpaneprobesadjudicated/20images+2grids. Other8verifiedfixes retain applicableevidence; seeindividualreports. No reexecution of closedexperiments.

**Currentpool ui07-build.json/link_plan PASS:**5viewers,17objects (UI06previous16 withcombinedSample replacement+1cameraobserver),1smalltaskAPIarchive. Preserve namednextPR10relink consumers. Oldui06/ui05/etcviewer manifests historical/unrunnable aftersupersededviewers/oldcombinedSample retired. Currentpins/alllinkinputsexist/hashverified. ui07-pin-adjudication.json documents relative/absolutegeneratorpincorrection usingexactdelta/hash,notchanged productafterchecks.

Cleanup:UI07supersededbuilds254569837B/streams11756853B/logs9844Bremoved;raw919567671B/free1708044214272B/selected9095350B.28recordedPIDsabsent/0ownedcores. Measuredsharedcachegrowth6219197Bnativecombinedbefore,within16MiB/app, preserved/attributionunproved; all5repairapps0growth. Raw+measuredcache925786868B<1GiB. Allbudgetsclosed/no livejob. FreezeNEWfinitebudget before nextbuild/app/capture; default64MiBretained/16MiBcacheapp32MiBrepair/20GiBfree.

Box3Dclean/diff--checkPASS;WASM/Rust/WGSL/sharedarchives unchanged. Finalnative+WASMcompatibility/criterion audit/precommit affected-scene recordings OPEN. NoPR10commitpush authorization.

## Next actions

NextUI10 capturedkey-up leavesflycamera moving. ReadUI10.md/picking-camera-acceptance.json; nativeEvent camera.OnEvent condition onlyuiuncaptured orMOUSE_UP/UNFOCUSED, thenuiCapturedreturn skipsSetKeyDown(false) too. TraceactualKEY_UP/Wlatch underpickerkeyboardcapture andCameraUpdate displacement afterhardware release. Repair necessaryrelease routingwithoutallowingcapturednewkeydownintosample/camera; preservetextediting/mouseup/unfocus/normalkeys/IDs/picking. Reusecurrentui07pool;freezeNEWfinitebounds beforeexecution. UI11combinedEscape ownerclear isseparate cause next,thenUI12–UI29/UI03/OBS01–02/finalaudit. NoGTK/cold/generalmesh/core/solver retries;PR11/PR02category/RainPR04 separate. ContinueACTIVEgoaluntilPR10complete,thenstop.

UI10traceFROZEN:1CPP30s/5links25s/5CPUfirstapps45s/10beforeimages/430souter/192MiBnewbuild/1GiBraw64MiBretained16MiBcacheapp32MiBtotal/0productionphysicsretries. NativeHandleEvent capture→CameraOnEvent skippedkeyup→UpdateheldW/hostIsKeyDown/displacement. Perrouteviewerretired immediately.19open10verified/full34216523reviewcomplete.

UI10traceoriginalcompilePASS/linkFAILEDbeforeanyapp:existingUIobserveralreadywrapsCameraUpdate. Failurepreserved; qualifiedUI07poolunchanged. DistinctchaincompletionFROZEN2taskobjcopy5s/2symbolinspections5s/0CC/5unstartedlinks25s+apps45s/10images/410souter/samecaps;newwrapper→previousUIwrapper→actualnativeUpdateonce. No apprepeated/sharedobjectmutation.19open10verified.

UI10smallestboundary all5CLOSED/PASS40checks:actualKEYUPWcapturedtrue, noCameraOnEventup, camera+hostW1 andpivotmoves4.77–4.95m betweenselectedafterreleaseupdates.10beforeimagesinspected;5traceviewersretired,UI/keyobserverchainobjects retainednextrelinks. Originalwrappercollisionlinkfailurepreserved, noapprepeated. RepairFROZEN3CPP30s/1generation10s/5links25s/5CPUfirstapps75s/15screens/650souter/192MiBnewbuild/1GiBraw64MiBretained16MiBcacheapp32MiBtotal/0physicsretries.19open10verified.
