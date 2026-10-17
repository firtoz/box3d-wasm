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
| Sample widgets | **310 reviewed / 32 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **163 reviewed / 2 left** | sample-applicability.md/json |
| Custom input sample groups | **22 reviewed / 1 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **27 open; 2 earlier fixes verified locally** | issues.json; UI01–UI29 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 163 sample dispositions comprise 69 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 163 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size; UI29 Mesh Drop auto regeneration hides controls behind repeated GPU preparation and fails to switch off. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest mutation-force-acceptance.json:13widgetreviewdispositions/3samples/all5routesCPUfirst. All49imagesinspected: three15cellgrids+4held-offimages. MeshDropactual1024dynamicbodies/fourtypes/flooramplitude0/1/all1029filterinputs/getters/Generate; Windshape/countlifecycle+4realsteps/ApplyWindarguments/poststepordering/restart; FallingTrees50trees/1100hulls/all3meshresolutions60000/240000/960000triangles/worldrecreation. No productfix. No independentforce/solvernumericqualification.

UI29 confirmed: CPUAutoGenerate starts/stops; all4GPUroutes failoffbothoriginal50msclickanddistinct2UIframeheldinput. OriginalButtonactivation1GPUvs2CPU, actualcreated/destroyedcontinueswhilepaused. All8GPUoffcaptures showPreparingSimulation, CPUkeepscontrolsvisible. LoadinghostbypassesnormalDrawUI whilepreparing; event-to-button causeUNTRACED, UI25relationonlyhypothesis. DoNOTrepeatunchangedautotests/coldcampaign; afterreview traceactualevents/loading/UIframes/button. TwoUIframes maybeloadingframes, notinteractiveInfoframes.

Originalprobeassertionswerewrong: nativeenumhas6values(capsule0/compound1/height2/hull3/mesh4/sphere5), observerretainsbins0..4plusfullshape/filtercount; deriveSphere5bycountminusotherbins, actualgetterexamplesconfirm. CPU/combineddefaultcategoryUINT64_MAX, standaloneGPU1(existingPR02gap); Collidefalse2/1all5 andactualall1029filterequalityinsideC++checked. JSONUINT64_MAX losesintegerprecisionbutC++comparisondoesnot. FlatmeshAABBheight.04 isnativebroadphasemargin, notactualfloor. Alloriginalfailuresretained/source-dataadjudicated/no replay.

Build21924 CLOSED0/15.785s/1CPP5links; main64015 CLOSED0/15apps withfindings; heldvariant21437 CLOSED1/4apps allterminalsameconfirmedfailure/no retry; closeout7684 CLOSED0/474finalcheckkeys/49images. All38ownedviewer/driverPIDsabsent/0cores/cachegrowth0. Streamsdeleted67756711, supersededcast5binaries/objectdeleted167031008; mainretained60602936/tail1645569; raw857678962/free1708318498816. Eachfrozenbudgetclosed. Currentqualifiedpool **mutation-force-build.json**, raw/mutation-force-build;207originalpinsunchanged. RetainforremainingGhost/Replay/repairqualification. Alloldcast/plane/scene/host/geometrypoolsdeleted. NoRust/WGSL/Box3D/WASMchanges.

Earliercast-controls-acceptance.json14widgets/5samples/3mousegroups/79images remainsvalid withUI28. TwoexplicitPR11querygaps(cast-pr11-query-findings.json) remainoutsideUIrepair: InitialOverlapzero-lengthCPUaccepted1/allGPU0; OverlapWorldidenticalshiftedboxCPUcallback1/allGPU0. Noqueryrepair/qualification. Priorplane/character/query/scene/hostreportsandhistoryremainvalid. Seehistory-mutation-force-2026-10-16.md forpreviousclosedbatches.

OBS01 combinedBodyCastpale/whiteappearance remainsUNCONFIRMED; inputs showmissesall5, projection/occlusiononlyhypothesis. ActualDrawHullcolor/selectionnotretained; triagebeforefinalacceptance.

Keep earlier UI24 flat HeightField scaleY0 floor loss, UI25 HeightField.75→.7 and combined Benchmark post-ray100s performance exclusions open. Read geometry-controls-acceptance.json and linked histories. Do NOT resume Capsule cold startup, Benchmark post-ray, core extraction, solver or PR11 campaigns. UI12 protected~3MB old core could not be deleted without sudo;204MBpartial already deleted. Owned viewers use core0. Original207pins and CPUReplayfixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec retained for remaining Replay. Preserve shared caches/user work; raw ignored.

## Next actions

Finish **32widgetentries/1custominputgroup across2samples**: Issues/s&boxGhostCollisions3widgets, ReplayViewer29remainingconditionalwidgets+handlers/focus/layout. ThenrepairdemonstratedPR10causesoneatatime andtriageOBS01. Current27UIissuesopen/2earlierverified, twoPR11queryfindingsandPR02defaultfiltergapremainexplicitoutsideUIrepair.

GhostWalkSpeedX/Z settersrunwhilepaused; verifyactualvelocity/nativeinchconversion/bodyidentity. ResetCounters needsactualnonzero counters beforeclear; default0→0isnotfullpass. Source-firstfinitephysical/mouseinteraction, noartificialcounterinjection/solvercampaign. Currentmutationpoolhasbodyvelocity/cameraobserver; projectedtargets currentlysphereonly. ReplayneedsactualCPUfixturewithjoints/queries/spawn/delete andread-onlyplayerobservations toreach29conditionalcontrols; GPUplayerunsupportedexplicitly/UI09pending. Earlierbasictransport7entriesalreadyreviewed,donotrepeat. No nextbudgetfrozen/livejob. Nativeinput/loading/buttoneventtracecanpiggybacknextneededobserverbuild forUI29/25; nofixbeforereviewfinishes. CurrentgoalACTIVE/PR10unchecked/noPR10commitpushauthorization.

Ghostcontrols FROZEN5appsCPUfirst/90s each/3screens/120actions/24stepscap/3realmouseoffsets/0builds/retries,12MiBAPI/4MiBobserver/1GiBraw/64MiBretained/128MiBcachegrowth. Sourcecamera estimateonly, actualnativepick/bodyidentityrequired. CPUprerequisitefailure stopsmatrix. Counts310/32widgets,163/2samples,22/1custom,27open/2verified.

Ghost93121 CLOSED1/CPUonly8.449s,3steps: bothslidersactualnativeinchconversion/IDspass;2/4/8pxrealgrabthresholdpreconditionsfaillegitimately(y1.0272733>1.026, v1.106>0.5), notcounterbug.4GPUapps unlaunched/unusedbudgetclosed; streamsconsumed/core0/cachegrowth0. DistinctthresholdbudgetFROZEN5apps90s/CPUcounteronly+firstGPUsliders/1wheelout+1pixel/1actualstep,4stepcap/0builds/retries,1GiBraw/64MiBretained; originalCPUsliderpasses reused. Counts310/32widgets,163/2samples,22/1custom,27open/2verified.

Ghostcloseoutterminal:3widgets/1samplepending16imageinspection/inventorywrite. Initial93121CPUonly3stepsthresholdmisses retained; changed41729CLOSED0/all5routesPASS/nativecounterpositive-to-zero/markers/HUD/IDs,1step/route. All12PIDsabsent/0cores/cachegrowth264543. Deletedstreams11494823,retained6877799,raw864556761. Currentpoolmutation-force-build.json/207pinsunchanged;27open2verified/counts310/32widgets163/2samples22/1custompendingdisposition.
