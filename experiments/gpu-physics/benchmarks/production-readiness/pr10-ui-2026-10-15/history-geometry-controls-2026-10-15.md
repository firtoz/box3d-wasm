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
| Sample widgets | **230 reviewed / 112 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **140 reviewed / 25 left** | sample-applicability.md/json |
| Custom input sample groups | **13 reviewed / 10 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **21 open; 2 earlier fixes verified locally** | issues.json; UI01–UI23 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 140 sample dispositions comprise 46 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 140 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest mesh-controls-acceptance.json:24 widget review dispositions/four samples onallfive routes. This includes UI23 and explicit Grid/BigBox Capsule GPU render-readiness exclusions, not24featurepasses. CPU Capsule dimensions pass. Sphere/Box/Cylinder getters, body poses/replacement and static IDs, signed X/Z scales and allsixReflection signs reviewed. ActualGPUAABB demonstrates zero-scale rejection inall4GPUroutes/3samples; combined GetMesh reads CPU and falsely suggests success. All50CPU-first gridcells plus6Capsule/diagnosticimages inspected. No product repair.

Original20apps retain8Capsule3sreadiness timeouts; untouchedBox/Cylinder/scales completed in8additional apps. OneCPU/native boundary probe observed body/capsule creation return. Native post-Capsule screenshot shows Scene collision shader/Compiling shader:collide_pairs; no ready GPU capsule rendering/getter claim. Native quit duringcompile timedout5s andownedSIGTERMreaped. GDBbudget unused becauseclickreturned; no backtrace captured. NoUI24, no coldcampaign retry. STARTUP01 staysoutsidePR10.

Mesh999per-receipt finalcheckkeys/28originalfindingevents; successfulcombinedzero getter predicates wereinsufficient, actualGPUbounds resolves them. Originalreceipts unchanged. Sessions69874(build14.017s),52441(originalmatrixterminal1),48193(diagnosticbuild),49714(boundary),17407(untouchedmatrix),10036(closeout) allterminal. Original207engine/UIinputpinsunchanged; Box3D/WASM/Rust/WGSL unmodified.

Cleanupdeleted283277396bytes, includingobsoletefivebody-properties viewers andbothboundaryviewers/object. Raw615208233bytes, ~1.7TBfree. Mainruns36876527bytes, untouched6081612, diagnostic927085, eachwithinseparatefrozenbudget. All60ownedPIDsabsent, nomatchingcores. Sharedcachegrowth3247716bytes measured/preserved. Currentqualifiedpool **mesh-controls-build.json** (fiveviewers raw/mesh-controls-build/); passbuildFileexplicitly. Previousbody-properties andcapsule-boundary binariesdeleted—do nottrytorerunthosepools.

Currentobserveraddsactualsphere/capsule/hull/mesh getters/bounds andmeshsetters. Reflections286bodyIDdigest/fullcount,64poseexamples;noallposeclaim. Includes/pins/sourcehashesinbuildreceipt. Retaincurrentpool forremaininggeometry/creation controls untilqualifiedreplacement. Existing raw/ui01-after,raw/ui02-after,otherobserver/link/preflightinputs andCPUReplayfixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec stillneeded. Rawlocallyignored.

Earlierlaunch-controls-acceptance.json covers4widgets/3samples+KeyboardL andSensorHitsB, all5routes,30CPU-firstimages, no newissue. Stall28origin-vs-COM velocityassertions adjudicatedfromsource/identicalCPUactualresults;COMcoordinatesnotseparatelycaptured. Alljobsclosed/streamsconsumed. Earlierbodyproperties/joint/Driving/shared/etc results andUI21/UI22 details remainvalid inlinkedreports. Histories: history-mesh-controls-2026-10-15.md, history-launch-controls-2026-10-15.md andtheirlinkedearlierhistories. Do notresumeexhaustedruns/coreextraction.

## Next actions

Continue **112remainingwidgetentries /10custominputgroups across25samples**, thenrepairrecordedcausesoneatatime. FreezeNEWfinitebuild/app/check/capture/storagebudgets; TypeScript/Bun only. Source-first. Currentpool maycoverremainingmeshgeometrycontrols withadditionalread-only boundaryobservations: MeshHeightField9,MeshViewer7; Character/movers/custominputs; collisionqueries/scale/shape casts/customhandlers; CompoundVillagekeyboard; ContinuousMeshDrop5; EventsContact1; IssuesCrash1/ghost3; BenchmarkFallingTrees3/HeightField1; ShapesStaticInvoke4/Wind5; RobustnessOverlapRecovery7; Tree12; ReplayViewer36/fullhandlers. Do notclaimunusedbranchespassed.

NoCapsulecold/shadercampaign retry, solverrepair, PR11qualification, commit/push orotherroadmapwork. Preserveconfirmedfailures, geometry/defaults/identity/picking anduserwork. KeepgoalactiveandPR10unchecked; stopafterPR10.

Active geometry review: geometry-controls-build-budget.json frozen1CPP+5links; session6349CLOSED0/14.167s,207originalpinshashesunchanged. OldPERF06querytimingobserver/5wrapflags omitted,replaced byrequest-summarizedCastShape/RayClosestobservers,originalphysicscalledonce. Actualheightfieldgetters/holes/bounds, meshbuilddefinition/results,nodeoverlay andqueryinputs/preview added; noengineedits. geometry-controls-budget.json frozen15appsCPUfirst/150sapp/3screens/0steps/retries/cores/1GiBraw/64MiBretained. Session51600RUNNING; geometry-controls-matrix.jsonauthority. Counts230/112widgets,140/25samples,13/10custom,21open/2verified pendingcloseout. Per-appstreamsconsumed; currentmeshpoolretaineduntilnewpoolqualified.

UI24confirmedordinary/native standaloneHeightFieldamplitude0: CPUflatgridscale[2,0,2]valid, GPUvalidationrejectsY0/nullshape/worldshapecount0;positiveamplitude restoresfloor. Sourceworld.rs2054validation; nofix. Combinedpendingmatrix. Harnessrestartpredicate exactserialized.200000003 vsMath.fround(.2) falselyfails; retain/adjudicatewithoutcontrolsreplay. Counts230/112widgets,140/25samples,13/10custom,22open/2verified. flat-heightfield-capture-budget.json frozen5apps50s/2screens/0builds/steps forbefore-repairimages; NOTLAUNCHEDuntilsession51600terminal.

Geometry controls matrix terminal; inspect geometry-controls-matrix.json before updating counts.

Session51600terminal1: all15apps consumed. HeightFieldall5 reviewed (UI24zeroamp and5serializedfloat harnessfailures),MeshViewerall5pass,BenchmarkHeightFieldCPU+2standaloneGPUactualRadiuspass; bothcombinedtimeout3saftertypingRadius0. OriginalAPItailprovesRayClosestreturned/full28557-cast frame, notdemonstratedquerydeadlock. Newbenchmark-heightfield-input-budget.json2apps100s/30sUIreply/0builds/steps frozen tocompletecombinedinputchecks andmeasurelatency; originalfailuresretained, notreleaseperformancepass. NOTLAUNCHEDuntilflatcapturesession70874terminal. NoUI25claimed.

Flatcapture70874terminal1/all5appsconsumed. Amplitudezeroimage+actualstates preservedall5; CPUpositive.75imagepassed. All4GPUpositive.75stimuli actuallyapply.7 (UItrace0.700000,actualscaleY1.39999998), assertionstopsbeforepositiveimage; donotclaimcaptureprotocolpass. UI25registeredwrongvalueconfirmed,causependingevent/ImGui/loadingtrace. 1mstyping tested; noallrateclaim, possibletransientrebuild/inputloss hypothesisnotproof. UI24alsoall4GPUconfirmed inclcombinedCPUvalidfloor. Counts230/112widgets,140/25samples,13/10custom,23open/2verified. Session12687RUNNINGnewcombinedBenchmarkRadius30sUIreplyprotocol; original3stimeoutsretained andactualRayClosestfullframereturnshowsnoguaranteeddeadlock. Needclosout+visualdispositionsandcleanupbeforecounts.

Geometry controls matrix terminal; inspect benchmark-heightfield-input-matrix.json before updating counts.
