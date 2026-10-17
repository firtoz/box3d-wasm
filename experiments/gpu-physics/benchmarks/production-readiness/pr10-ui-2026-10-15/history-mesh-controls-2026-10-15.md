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
| Sample widgets | **206 reviewed / 136 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **136 reviewed / 29 left** | sample-applicability.md/json |
| Custom input sample groups | **13 reviewed / 10 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **20 open; 2 earlier fixes verified locally** | issues.json; UI01–UI22 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 136 sample dispositions comprise 42 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 136 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest body property review is CLOSED: body-properties-controls.md and body-properties-controls-acceptance.json. Seven samples/28widgetentries plus Door MouseDown onallfive routes: Bridge1, MotionLocks6, Motor4, Door6, RagdollBox4/Mesh4, GearLift3. Full150-body gravity aggregates/setter coverage; all6locks on4bodies with2bounded steps; Motor actualforce/torque/impulse/target speed with1step; hinge tuning/recreation/impulses/picking; ragdoll jointpolicy/respawn; Gear driverparameters. Otherfixtures0physicssteps. Sourcegeometry/defaults andoriginal207inputs preserved. All70CPU-first imagecells inspected. Default ragdoll framing clips upperbody/head onactualCPU/GPU alike; camera observation, not a newly demonstrated widgetfailure.

UI21 confirmed allfive routes: Mesh Respawn leaves14valid oldkinematicanchors each time, body29→43→57 whiledynamic14 andshapes19 remain. Original anchorIDs remainvalid; Box(noanchors) actualrespawn count15stable. DestroyHuman omitsanchorbodycleanup, thenCreateHuman overwritesIDs. UI22 confirmed allfive: Door m_enableLimit isuninitialized andCreateJoints hardcodeslimits on. Some initialcheckboxes disagree; other routes canfail toclear. Original unchecked-preservation predicates whereUI stayedtrue are notvalid stimuli; failedclear isrecorded instead. Rawboolbyte hypothesis hasnotbeen instrumented—trace that boundary before repair ifneeded. Door jointID members also lackinitializers beforevaliditychecks (sourceonly, no additional runtimefailure claimed).

The37app receipts contain686summed per-receipt finalcheckkeys; repeated Door togglesreusekeys.33findingevents comprise26UI21/UI22productevents plus7harness assertions. Gearactual205bodies/274shapes matchesfullsource includingthebackwall; original273expectedpredicate retained/adjudicated without rerun. Combinedboth_pointer_impulse callsGPUlowlevel directly, bypassingpublicwrappers; actualpublicGPUvelocities proveimpulses. Originalleftpane alsochangedboth engines; earlier CPU-only inference waswrong. Thetwo focusedrightpane assertions requiring a publicGPUwrappercall were rejected fromsource/actualvelocities; no productissue orrerun. Bothpanes nowexplicitlyreviewed.

All jobs terminal: bodypropertiesbuild54561closed0/14.727s, matrix9641closed0/35apps, rightpane64409closed0/2apps, closeout9588closed0. Earliergeneratorpathpreflight failedbeforecompile/apps; corrected withunusedcompilebudget. All74owned app/driverPIDs absent/no matchingcores; sharedcachegrowth0. Per-app streams consumed immediately; cleanupdeleted309367070bytes including retiredjointviewers. Raw554409599bytes. Main35-app diagnosticdirectory65811698bytes (<64MiB); separate2-app rightprobe1103003bytes. Closeoutgrids/metadata have separatefrozenbudget. Originalreceipts, failures, media andminimal actualstates preserved.

Currentqualifiedpool: **body-properties-build.json**, five viewers under raw/body-properties-build/. PassbuildFile explicitly. Driverlatest() honors optional budget.api_tail_bytes; thisgroup uses2MiB forlarge rows, priorconsumers retain64KiB default. Propertyobserver has512jointrefcap and64bodyexamples pluscomplete dynamicpropertyaggregates. Bridgejointtraceoff (302joints unnecessaryforgravity). Motor/Filterjointgetters andconstrainttuning, actualgravity/lock/bulletflags, impulses/targettransforms covered. Capsule shape-specific geometrymetadata remains uncollected here; originalsourcehashes, hostshapecounts, bodyposes/masses andrealimages preserve geometry evidence.

Retaincurrentpool forremaininglaunch/propertycontrols untilqualifiedreplacement, then delete. Existing raw/ui01-after, raw/ui02-after andotherobserverobjects remain actual link/preflightinputs. Keep raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec forremainingReplay controls. Raw locallyignored; userwork/sharedcaches preserved. Earlier joint, Driving/TopDown, bodymutation, collision, Manifold, geometry, Disable/shared evidence remainsvalid withknown defects/limits. Closedhistories: history-body-properties-2026-10-15.md, history-driving-controls-2026-10-15.md, history-joint-controls-2026-10-15.md andearlierlinked histories. Do notresume exhaustedruns/coreextraction.

## Latest closed launch review

launch-controls-acceptance.json: four widgets / three samples and Bullet vs Stack Keyboard L reviewed on all five routes. Sensor Hits polled B also checked. Actual bullet flags, velocities, shape types/radii, replacement IDs, preserved other bodies and restart defaults. All 30 CPU-first image cells inspected. No new product issue. The 545 final assertion keys include 28 original Stall velocity predicates adjudicated as comparing origin input with COM velocity after an off-centre shape is attached; all routes match actual CPU. Mass-centre coordinates were not separately captured. No physics steps or CCD/sensor-event/performance qualification.

Sessions 73041 and 70007 CLOSED0. All 30 owned PIDs absent, no matching core, cache growth 0; 12,129,101 stream bytes deleted. Retained runs 16,267,706 bytes; raw total 570,677,305 bytes; ~1.7TB free. Current five-viewer pool and existing named build/Replay inputs retained. See launch-controls-cleanup.json. History of previous current log: history-launch-controls-2026-10-15.md.

## Next actions

Continue **136 remaining widget entries / 10 custom input groups across 29 samples**, then repair recorded causes one at a time. Current pool remains body-properties-build.json; pass it explicitly. Likely next review: mesh shape/scale/reflection controls, requiring read-only shape getters and mutation wrappers. Freeze a new finite observer build and app budget first. Remaining collision queries/custom handlers, character/input, meshes, static creation/wind, benchmarks/tree/robustness and Replay Viewer controls require actual effect evidence. No repeated launch review, CCD/solver/cold campaigns or PR11 qualification.

Keep goal active and PR10 unchecked. No new PR10 commit/push authorization.

Active mesh batch: build69874 CLOSED0/14.017s, original207inputs unchanged. mesh-controls-build-budget.json frozen1CPP+5links; mesh-controls-budget.json frozen20apps150s/app,3screens,0steps/retries/cores,1GiBraw/64MiBretained. Session52441; mesh-controls-matrix.json authoritative. Currentcounts206/136widgets,136/29samples,13/10custom,20open/2verified pendingcloseout. Current mesh pool becomes qualified only after applicable five-route review; old body-properties pool retained until then.

Mesh matrix terminal; inspect mesh-controls-matrix.json before counts/disposition.

Mesh session52441 terminal1: all20plannedapps consumed, CPU4pass; GPUGrid/BigBox Sphere→Capsule UItransition timesout3s (8apps), Boxzero scale2standalone failures perroute, Reflectionall5pass. Do notrun mesh-controls-closeout.ts yet: its allpass assumptions areinvalid. No countupdate yet. Zero scale confirmedGPUreject, combinedGetMeshreadsCPUonly and hidesGPUunchangedbounds; sourceboth_dual.c1635/3013. CandidateUI23. Capsulecauseunknown, donotassumeGPUsolver/coldstartup: lastcompletedDestroyBody, nextCreateBody/Capsule unobserved. New frozen capsule-boundary-budget.json1CPP+2links/2appsCPUfirst/one8sGDBattach/0steps, session48193build running; preserveoriginalfailurestreamsummaries andoldqualifiedbody-propertiespool untilcurrentqualification.

Capsule diagnostic build48193closed0; apps49714closed0. BothCPU/nativebody andcapsule creation ENTER/RETURN captured. Native screenshot capsule-created.png is PREPARING SIMULATION/Scene collision shader/Compiling shader:collide_pairs. Thus original3sstate timeouts are STARTUP01 shader readiness, notdemonstratedcapsulecreationfailure; noUI24, noresume/retrycoldcampaign. DiagnosticnativeCtrlQcouldnotquitduringcompile; ownedSIGTERMreaped. GDBbudgetunused becausetransition clickreturned. UI23registeredconfirmedzero-scale GPUrejection withcombinedCPU-onlyGetMeshprovenance and actualGPUAABBunchanged. Counts206/136widgets,136/29samples,13/10custom,21open/2verified. Newfrozen mesh-remaining-controls-budget.json8apps/0build/0steps checks ONLYuntouchedBox/Cylinder/X/Zcontrols; session17407running. OriginalCPUfullpassescarriedforward, Capsuleexcludedfromrerun. Needreconcilecloseoutscript forpartial/knownloadingratherthanallpass.
