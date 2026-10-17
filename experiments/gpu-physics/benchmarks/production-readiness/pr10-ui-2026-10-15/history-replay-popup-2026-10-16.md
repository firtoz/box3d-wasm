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
| Sample widgets | **313 reviewed / 29 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **164 reviewed / 1 left** | sample-applicability.md/json |
| Custom input sample groups | **22 reviewed / 1 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **27 open; 2 earlier fixes verified locally** | issues.json; UI01–UI29 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 164 sample dispositions comprise 70 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 164 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size; UI29 Mesh Drop auto regeneration hides controls behind repeated GPU preparation and fails to switch off. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

ReplayfixtureprerequisiteREADY: replay-fixture-acceptance.json/readreplay-fixture.md. ActualnativeCPU64frame/76546Brecording **raw/replay-rich-corrected/fixture.b3rec** (hashinreplay-counts.json) validated/nativeplayercoverageextracted.5stablebodies+spawnframe9/deleteframe17 hole, namedshapes/revolute/contact, all7querykinds/stablekeys/duplicateinstances/untagged. First5querykindspositive/miss; CastMoverandCollideMoverhitlists0infixture, notqualifiedno-collision. CastMovernativeStashQueryBeginNULL0bydesign; preservefraction distinction.

Fixturehelperfailuresretained: originalShapeProxyconstpointercompileerror; correctedrecord/validate/scanpassedallbeforefinalpositive-per-kindprobeassert; firstread-onlyextractretainedsametoo-strongassert andprintedtoo late. Finalread-onlycountsextractionPASS; no recording regeneration, noenginefix. 4smallcompileattempts/3CPUprocesses,320nativeCPUsteps total(64record+64validate+3x64scan);0GPU/apps/captures. All7PIDsabsent/0cores, deletedexecutables3451352bytes/retained79773/raw864636534/free1708287430656; currentCPUlibraryhashunchanged. Old28KBfixture stillneededUI08/basicReplayregression. Allnewbudgetsclosed/no livejob. Counts313/29widgets164/1samples22/1custom27open2verified unchanged;fixtureprerequisitedoesnotreviewwidgets.

Latest ghost-controls-acceptance.json:3widgets/1sample/all5routes. All16imagesinspected (15cellCPU-firstgrid+originalCPUpreconditionlimit). Bothpausedspeedslidersreachactualfloat32inchconversion/horizontalvelocityandpreserveverticalvelocity/IDs/poses. ResetCounters clearsactualnonzeroLaunches/worstspeeds/redmarkers, preservesbodyIDs/poses. RealCtrlpick/bodyidentitycheckedbeforephysics. No new issue or productfix.

Original93121 CLOSED1/CPUonly: bothsliderspass;2/4/8pixelrealgrabs allvY>.5butcenterY>native1.026groundedthreshold, sozerocountercorrect. Threefailedtestpreconditionsretained, notcounterbug/zero-to-zeropass.4GPUapps unlaunched/unusedbudgetclosed. Source/data-backedchanged41729 CLOSED0/all5PASS:1wheelout+1pixelactualupwardgrab/oneactualstep yieldsnonzerocounter, thenResetclears. CPUsliders/initialcapture reused; GPUslidersfirstattempt. Total8worldsteps(3failedCPUpreconditions+5changedsteps), notghost-collisionphysicalqualification. Closeout87358 CLOSED0/113finalcheckkeys. All12ownedPIDsabsent/0cores/cachegrowth264543. Streamsdeleted11494823/retained6877799/raw864556761/free1708292009984. Allbudgetsclosed/no livejob. Currentpool **mutation-force-build.json**,207originalpinsunchanged, retainedforReplay/repairs; noRust/WGSL/Box3D/WASMchanges.

Latest mutation-force-acceptance.json:13widgetreviewdispositions/3samples/all5routesCPUfirst. All49imagesinspected: three15cellgrids+4held-offimages. MeshDropactual1024dynamicbodies/fourtypes/flooramplitude0/1/all1029filterinputs/getters/Generate; Windshape/countlifecycle+4realsteps/ApplyWindarguments/poststepordering/restart; FallingTrees50trees/1100hulls/all3meshresolutions60000/240000/960000triangles/worldrecreation. No productfix. No independentforce/solvernumericqualification.

UI29 confirmed: CPUAutoGenerate starts/stops; all4GPUroutes failoffbothoriginal50msclickanddistinct2UIframeheldinput. OriginalButtonactivation1GPUvs2CPU, actualcreated/destroyedcontinueswhilepaused. All8GPUoffcaptures showPreparingSimulation, CPUkeepscontrolsvisible. LoadinghostbypassesnormalDrawUI whilepreparing; event-to-button causeUNTRACED, UI25relationonlyhypothesis. DoNOTrepeatunchangedautotests/coldcampaign; afterreview traceactualevents/loading/UIframes/button. TwoUIframes maybeloadingframes, notinteractiveInfoframes.

Originalprobeassertionswerewrong: nativeenumhas6values(capsule0/compound1/height2/hull3/mesh4/sphere5), observerretainsbins0..4plusfullshape/filtercount; deriveSphere5bycountminusotherbins, actualgetterexamplesconfirm. CPU/combineddefaultcategoryUINT64_MAX, standaloneGPU1(existingPR02gap); Collidefalse2/1all5 andactualall1029filterequalityinsideC++checked. JSONUINT64_MAX losesintegerprecisionbutC++comparisondoesnot. FlatmeshAABBheight.04 isnativebroadphasemargin, notactualfloor. Alloriginalfailuresretained/source-dataadjudicated/no replay.

Build21924 CLOSED0/15.785s/1CPP5links; main64015 CLOSED0/15apps withfindings; heldvariant21437 CLOSED1/4apps allterminalsameconfirmedfailure/no retry; closeout7684 CLOSED0/474finalcheckkeys/49images. All38ownedviewer/driverPIDsabsent/0cores/cachegrowth0. Streamsdeleted67756711, supersededcast5binaries/objectdeleted167031008; mainretained60602936/tail1645569; raw857678962/free1708318498816. Eachfrozenbudgetclosed. Currentqualifiedpool **mutation-force-build.json**, raw/mutation-force-build;207originalpinsunchanged. RetainforremainingGhost/Replay/repairqualification. Alloldcast/plane/scene/host/geometrypoolsdeleted. NoRust/WGSL/Box3D/WASMchanges.

Earliercast-controls-acceptance.json14widgets/5samples/3mousegroups/79images remainsvalid withUI28. TwoexplicitPR11querygaps(cast-pr11-query-findings.json) remainoutsideUIrepair: InitialOverlapzero-lengthCPUaccepted1/allGPU0; OverlapWorldidenticalshiftedboxCPUcallback1/allGPU0. Noqueryrepair/qualification. Priorplane/character/query/scene/hostreportsandhistoryremainvalid. Seehistory-mutation-force-2026-10-16.md forpreviousclosedbatches.

OBS01 combinedBodyCastpale/whiteappearance remainsUNCONFIRMED; inputs showmissesall5, projection/occlusiononlyhypothesis. ActualDrawHullcolor/selectionnotretained; triagebeforefinalacceptance.

Keep earlier UI24 flat HeightField scaleY0 floor loss, UI25 HeightField.75→.7 and combined Benchmark post-ray100s performance exclusions open. Read geometry-controls-acceptance.json and linked histories. Do NOT resume Capsule cold startup, Benchmark post-ray, core extraction, solver or PR11 campaigns. UI12 protected~3MB old core could not be deleted without sudo;204MBpartial already deleted. Owned viewers use core0. Original207pins and CPUReplayfixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec retained for remaining Replay. Preserve shared caches/user work; raw ignored.

## Next actions

Finish **29conditionalwidgetentries/1custominputgroup in ReplayViewer**, thelastremainingnative sample. ThenrepairdemonstratedPR10causesoneatatime andtriageOBS01. Current27UIissuesopen/2earlierverified; twoPR11queryfindingsandPR02defaultfiltergapremainexplicitoutsideUIrepair.

Usevalidated **raw/replay-rich-corrected/fixture.b3rec**, readreplay-fixture-acceptance.json first. NextneededobservercanbeCPU-only C++wrapper/linkagainstcurrent207pinnedinputs; keepGPU4currentbinariesforunsupportedpopup/Cancelchecks. RecordactualPlayer creation/policy/worker/seek/step/pre-spawnstate andDrawFrameQueriesindices/callbacks/nativecameraZup; addUIselected/hover/clickflagsforTreeNodeExifneeded. No engine/WASMbuild. LinkCPU-onlyavoidsduplicating5viewerpool/rawcap. Freezefiniteobserver/UIbudgetbeforeexecution. NativeReplay29conditionalwidgets/customhandlers requireactualCPUchecks; GPUplayerexplicitunsupported/UI09pending, nofakeGPUfeaturepass. Earlier7basictransportentriesreviewed,donotrepeatunchanged. Remainingpopupmemory/mininterval/Cancel, Timeline/DrawAllQueries/FollowSelection/Zup, QuerySearchtext/kind/minhits/rebuild/results, body/shape/joint/querytrees/details, speed/loop/substepspawns/workers/frameSlider andKeyboard/mouse/focus/layout. CurrentnativeTSdriver item helper assumesInfo; extenddiagnostichelpers toactualpopup/timelinewindows asneeded. No nextbudgetfrozen/livejob. Nativeinput/loading/buttoneventtraceforUI29/25canpiggybacknextneededobserverbuild, nofixbeforereviewfinishes. GoalACTIVE/PR10unchecked/noPR10commitpushauthorization.

Replayrichfixture FROZEN:1CPPcompilelink25s/1CPUrecordvalidate8s/64record+64validate+64scansteps/0GPU/viewer/capture/retry,20MiBtemp/4MiBretained/core0. NativeCPUall7querykinds, namedbodies/shapes/revolute/contact/spawn/delete/keyinstances. TypeScriptorchestration/Box3Dunchanged. Counts313/29widgets,164/1samples,22/1custom,27open/2verified.

Replayrichfixture terminal FAILURE_RETAINED; inspect replay-rich-fixture.json. Executabledeleted/ownedPIDsabsent/currentCPUlibraryunchanged.

Replayrichfixture compile CLOSED1/.18s, runtimeunlaunched: diagnosticwronglytreatedShapeProxy.points asarray; nativeAPIconstpointer. Originalsource/logpreserved/0steps/0GPU. Correctedpointerfixture FROZENsame1compile25s+1CPUrun8s/192CPUstepscap/20MiBtemp4MiBretained/0GPU/apps/capture/retry; distinctdiagnosticcompilecorrectiononly. Counts313/29widgets164/1samples22/1custom27open2verified.

Replayrichfixture terminal FAILURE_RETAINED; inspect replay-rich-fixture.json. Executabledeleted/ownedPIDsabsent/currentCPUlibraryunchanged.

Replaycorrectedfixture compilePASS/.454s/CPUrecordvalidate .068s reachedfinalcoverageassertfailed becauseCastMoverhitCount0 nativeAPIbydesign(StashQueryBeginNULL,0). Actualrecording76KB retained, executable1153320Bdeleted/core0/PIDsabsent. Sourcevalidationandbody/lifecycle/key/contactassertionsprecededfailureandpassed; originaldata/sourcepreserved. Read-onlyexistingfixtureextract FROZEN1compile25s/1CPUscan8s/64steps/0record/validate/GPU/apps/captures/retries/20MiBtemp4MiBretained. Allother6querykindspositive/missstillrequired, moverquerycount/fractioncontractexplicit. Counts313/29widgets164/1samples22/1custom27open2verified.

Replayrichfixture terminal FAILURE_RETAINED; inspect replay-rich-fixture.json. Executabledeleted/ownedPIDsabsent/currentCPUlibraryunchanged.

Read-onlyReplayextract compilePASS/.39s,scan.065scoveragepositive-per-kindassertfailed beforeprint; actualrecordingunchanged/executable1148992Bdeleted/core0/PIDsabsent. Thisisprobeobservabilityfailure, notproductissue. FinalcountsextractFROZEN1compile25/1scan8/64CPUsteps/0record/validate/GPU/apps/captures/retries/20MiBtemp4MiBretained: printflushallactualcounts BEFOREcoverageassertions, UIemptykindresults explicit. No repeatedrecording.313/29widgets164/1samples22/1custom27open2verified.

Replayrichfixture terminal PASS_EXISTING_REAL_CPU_FIXTURE_COVERAGE; inspect replay-rich-fixture.json. Executabledeleted/ownedPIDsabsent/currentCPUlibraryunchanged.

CPUReplayobserverbuild FROZEN1CPP30s/1link25s/48MiBtemp/0GPU/apps/retries. Read-onlyPlayerboundary/callbackdraw/nativecameraZup/Treeflags; mixedpoolnewCPU+4unchangedGPUviewers/current207pins.313/29widgets164/1samples22/1custom27open2verified.

Replaypopup FROZEN5apps60s/3screens/120actions/0worldphysics/CPU64recordedpregensteps/0build/retry,16MiBAPI/1GiBraw/64MiBretained/128MiBcachegrowth. CPUnewobservermixedpool,4GPUunchanged.313/29widgets164/1samples22/1custom27open2verified.

CPUReplayobserver31947 CLOSED0/1compile1link, original207pinsunchanged/mixedpoolreplay-observer-build.json. Popup64519 CLOSED1/CPUonly9.546s atGTK8sdiscovery beforeanywidget, nativeGTKcriticalmessages/windowinventorymissing;4GPUunlaunchedbudgetclosed/streamsconsumed. SourceNFDtitleOpenFileconfirmed. DistinctGTKhandlingFROZEN5apps60s/0build/retry/samecaps, persistownedallwindowtitles+phase,50msactivation/100mslocationentryready. No productdefectinferred/313widgets29left/164samples1left/22custom1left/27open2verified.

ReplayGTKdiscovery9624 CLOSED1/CPUonly: ownedOpenFilefound/focused/pathsent, timeoutat8sawaitchooserclose, beforeanyReplaywidget. SourceNFDOpenFileconfirmed; actualacceptanceunproven, OBS02probe/environmentGTKpathsubmissionUNCONFIRMED, no productdefectclaim. StopGTKattemptsunchanged. DistinctnativeSUPPORTED --replay input budgetFROZEN6apps60s/0builds/retries/samecaps: CPUcancel+Ractualpolicy, CPUapply,4GPUpopupunsupported/Cancel. OriginalsharedDriverdefaultargsunchanged; optionaldiagnosticstart_argsusesnativeCLIonly.313widgets29left164samples1left22custom1left27open2verified.
