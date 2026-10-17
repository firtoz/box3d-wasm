# PR10 UI review — recovery log

Read this FIRST on every continuation and after compaction/restart, then `AGENTS.md` and `docs/goals/gpu-production-readiness.md`. Apply the goal-scratchpad skill. This is the current state; dated histories contain closed experiments, not next actions.

## Active goal and authorization

Finish **PR10 UI/widgets end to end** in `/home/firtoz/work/2026/box3d-wasm` on `feat/gpu`. The user activated the app goal on 2026-10-15. Continue autonomously across turns; do not pause unless explicitly requested. Stop after PR10. No token budget was requested. Each execution batch still requires finite resource limits.

Review applicable controls for all 165 native samples: 342 per-sample widget entries, 85 shared calls and 23 samples with custom input handlers. Compare real Box3D CPU first, ordinary/native GPU and both combined viewers. Exercise actual mouse/keyboard input, displayed state, intended geometry/physics/render effects, identity and lifecycle. Shared implementation may reuse exercised cases only with explicit per-sample applicability. Source registration, a screenshot or input acceptance alone does not prove an effect. Unsupported features require honest UI and explicit dispositions. Preserve previous passes and original failures; do not repeat PR09's completed default visual sweep.

Finish the remaining review **before fixing newly recorded issues**. Then trace the smallest failing operation and fix one cause and its affected cases at a time. Verify applicable routes, retain CPU-first before/after screenshots and required affected-scene comparison evidence, mark [x] only after verification, and share screenshots with each repair report. Report controls/samples done and left and issue counts in every progress update. Finish with a criterion audit; PR10 stays unchecked until its actual criteria pass.

Use **TypeScript/Bun** for new automation. Freeze build/check/capture/process/runtime/storage budgets before each batch. Enforce reasonable timeouts and output caps, disable owned viewer core dumps, and reap owned child processes. Default limits: 1 GiB temporary / 64 MiB retained diagnostic evidence per batch; budget exceptions beforehand. Measure and preserve shared caches. Delete consumed task-owned binaries/traces/copies after extracting minimal evidence, including failed runs. Keep a named consumer for retained large inputs. Never stage `raw/`; it is locally excluded.

Keep Box3D/WASM unchanged and preserve geometry, physics defaults, body identity, picking and other local work. Review probes also leave engine/Rust/WGSL inputs unchanged. No unrelated solver repairs, PR11 query qualification, cold-start campaigns or exhausted experiments. Exact pixels and chaotic CPU/GPU trajectory matching are outside scope. Integrate remote changes safely if needed. No new PR10 commit/push authorization. PR09 is already pushed as `51ff8b06b4964d359386f08605d835ac6e2ea23d`.

## Counts and acceptance gates

| Gate | Done / left |
| --- | --- |
| Native sample inventory | 165 / 0 |
| Shared controls | 85 / 0 |
| Sample widgets | 342 / 0 |
| Sample dispositions | 165 / 0 |
| Custom input groups | 23 / 0 |
| Repairs | 21 verified / 8 open |
| Final PR10 acceptance | Open |

The 165 dispositions comprise 71 actual specific-control fixtures and 94 explicit shared-code reuses, not 165 new runtime/feature passes. Five inactive Ragdoll/Pose widgets are excluded; 342 active entries remain the denominator. See inventory.json and sample-applicability.md/json.

Verified: UI01, UI02, UI04, UI05, UI06, UI07, UI08, UI09, UI10, UI11, UI12, UI13, UI14, UI15, UI16, UI17, UI18, UI19, UI20, UI21, UI22. Open: UI03, UI23, UI24, UI25, UI26, UI27, UI28, UI29. Individual reports preserve original findings, probes, limitations and evidence. issues.json is authoritative.

## Current evidence and resources

UI22 [x]: [repair](UI22-repair.md), [acceptance](ui22-acceptance.json). GeneratedDoor initializesenabledUIpolicy/initialhingeIDs andbothhingedefinitions usecurrentpolicy. All5routes/252checks:50actualhingerecreations preservefalse/true policy/one-twohinges/currenttuning;oldjointIDs invalid,newIDs/bodyposes/mass/geometry/otherdefaults/restart/picking pass. AllotherJointsource/UI19/20 byteexact;0physics/engine/WASM edits.

BothCPUfirstgrids inspected (10original/20afterimages);originalconfiguredimpulse/magnitude difference explicit,actualnativepolicy/lifecycle data verifiesrepair..

Current pool: **ui22-build.json/link_plan PASS**; 5 viewers, 32 direct task objects and the small combined API archive. Exact required inputs are in ui22-acceptance.json.resources.active_task_objects; next consumer is the next PR10 relink. Older manifests are historical and may have retired files/stale pins. Verify only the current pool.

Cleanup: raw 978067129 B; free 1707688493056 B; selected evidence 14138940 B. Removed 167327522 B of superseded builds, 15700455 B of consumed streams and 17573 B of logs. 19 recorded PIDs absent, no owned cores, measured shared-cache growth 0 B. Shared caches preserved. No live job; this batch is closed.

Freeze a new finite budget before execution: 1 GiB whole raw tree, 64 MiB retained evidence, 16 MiB cache growth/app and 32 MiB/repair, at least 20 GiB free. Replace viewers sequentially; audit/remove consumed task outputs before approaching the cap. Preserve media, current inputs and user work.

Box3D/WASM/Rust/WGSL/shared archives unchanged. CMake generates native UI copies with Bun; full fresh configure/build/native+WASM compatibility, final affected-scene recordings and criterion audit remain OPEN. PR10 unchecked; goal ACTIVE. No PR10 commit/push authorization.

## Recovery cautions

UI12 original CPU leading-0 crash/GDB evidence is sufficient; do not repeat crash/core extraction. Its three wrong analytic-zero probes were adjudicated from the same data against native minimum half-width 0.02. UI14 two wrong cache-equality probes included the hit flag; native fresh hit=0/reused hit=1 is correct. Original receipts/flags and adjudications remain preserved. UI11 two registry-vs-world-counter probes and UI10 observer symbol-chain/pin corrections remain in their reports. Never call original false flags all true.

STARTUP01 is PR05/08, Rain physics is PR04, query qualification is PR11, category truthfulness is PR02. No exhausted core/cold/solver/GTK campaigns. OBS01 combined Body Cast pale appearance is unconfirmed; OBS02 GTK dialog timeouts are preserved, not a reason to rerun unchanged GTK.

## Next actions

Next UI23 zero mesh scale normalization. Read UI23.md/mesh-controls-acceptance.json/query-input-controls-acceptance.json and c_abi/shim.c/both_dual.c b3Shape_SetMesh→gpu replacement→actualGPUAABB. Native b3SafeScale clampszero to+.01;combinedCPU-backedgetter masksstaleGPU geometry. Trace identicalinput/currentactualGPUshape beforechoosingadapter normalization;preserve reflectionsigns/geometry/defaults/identity/picking. Freeze finitecompile/link/runtime/capture/storage budget;wholeRaw currently978MB,requiressequentialviewerreplacement/strictoutputcaps tostay1GiB. ReusecurrentUI22pool andneverrecheckall165unchangedscenes. ThenUI24–29/UI03/OBS01–03/freshcompatibility/finalrecordings/criterionaudit;continueuntilPR10complete thenstop.

UI23FROZEN:2C25s/1CPPobserver20s/<=30archivecommands5s/4links25s/5apps90s/<=30screens/outer850/0physics/retries. WholeRaw1GiB/newBuild192MiB/retained64MiB/cache16MiBapp32MiBtotal/free20GiB. OnlyfiniteSetMeshGPUscale usesnativeb3SafeScale inshim/dual;standalonemirror normalized,combinedCPUraw input unchanged.4affectedscenes9zeroaxes+±1/Reflectionsigns/nativeactualRustreturn+GPUAABBvsCPUfirstoracle/identity/picking. InitialCreateMeshzeroAPI unchanged/unqualified. CurrentUI22pinsverifiedbeforedelta;noUI23build/vieweryet.21verified8open.

UI23objects READY:2C/1CPP compiles,2taskAPIcarrierarchives onlyexpectedmemberdelta/allothermembershashidentical. ui23-build CANDIDATE_OBJECTS_READY/0apps. Driverneededbefore run;CPUexistingviewer reusedfirst/4GPUlinks.21verified8open.

UI23PHYSICSSTAGE CLOSED/909checks alltrue/45zeroaxes/132GPUaccepted+CPUoracleAABBs,0Rustbuilds. ui23-build PASS iscurrentphysics-onlypool;issueNOTclosed. Visualinspection3grids confirmsnewrelatedGPUrenderstale:Grid/BigBox floorstillfullwhileCPUsliver. shim updatesGpuMeshMirror, samples_api ShapeVis.mesh/userShape stayscreationgeometry;no SetMeshrendercallback. Preservedall35images/physicsacceptance+grids renamedui23-physics-stage-*. Needmeshreplacement callback usingexistingdestroy/recreate+name-preservation path andactualCreateShape.mesh.scale trace beforefurtherqualification. No nextbudget/livejob.21verified8open.

UI23RENDERFROZEN:4C25s/<=45ar5s/4links25s/4GPUapps90s/outer850/24newimages/0CPUapps/Rustbuilds/physics/retries. WholeRaw1GiB/newBuild192MiB/retained64MiB/cache16MiBapp32MiBtotal. Dedicatedmeshreplacehook destroyscacheduserShape andupdatesonlymesh/body/type,preservingname/othermetadata;calledonlyafterRustsuccess. ActualnativeCreateShape.meshscale/accepted andcacheinvalidated traces ingeneratedlocalcopies only. OldVIS02qualifiedAPI2sources/hashes/compilecommands identified;sharedmodules identitycheckbeforeuse. CPU33oracle/6images reused. No newjob yet;21verified8open.

UI23RENDERobjectsREADY:4Ccompiles/3taskAPIcarriers/qualifiedVIS02originalAPI2moduleidentity proofs andunchangedothermemberhashes complete. ui23-render-build CANDIDATE_OBJECTS_READY/noapps yet. Needrenderdriverbefore run;CPU33oracle/images reused;4GPUfirstchanged-render qualification.21verified8open.

UI23RENDERstopped: AssertionError [ERR_ASSERTION]: false == true;ui23-render-build/current/operations inspect;noissueclosure.21verified8open.

UI23RENDERordinary33cases complete;24actualworld-drawnewCreateShape.scale pass/Grid+BigBoximagesnowCPUsliver.9wrongMeshScaleCreateShape predicates false: nativeMeshScale overridesStep(noSample::Step WorldDraw),inherits emptyRender,drawsquerypoints/lines/sphere only;hadCachefalse/currentprior-sceneCreateID proveswrongexpectation. Samecaptureddata adjudicationui23-query-only-render-adjudication.json,0ordinaryreruns;all9physicsAABB/nativeRustboundschecks pass. Remaining3GPUfirstchanged-render routes FROZEN0compile/ar/3links/3apps90/outer370,wholeRaw1GiBunchanged.21verified8open.
