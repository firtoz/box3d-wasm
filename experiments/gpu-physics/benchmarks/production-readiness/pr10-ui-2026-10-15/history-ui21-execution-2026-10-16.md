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
| Repairs | 19 verified / 10 open |
| Final PR10 acceptance | Open |

The 165 dispositions comprise 71 actual specific-control fixtures and 94 explicit shared-code reuses, not 165 new runtime/feature passes. Five inactive Ragdoll/Pose widgets are excluded; 342 active entries remain the denominator. See inventory.json and sample-applicability.md/json.

Verified: UI01, UI02, UI04, UI05, UI06, UI07, UI08, UI09, UI10, UI11, UI12, UI13, UI14, UI15, UI16, UI17, UI18, UI19, UI20. Open: UI03, UI21, UI22, UI23, UI24, UI25, UI26, UI27, UI28, UI29. Individual reports preserve original findings, probes, limitations and evidence. issues.json is authoritative.

## Current evidence and resources

UI18 [x]: [repair](UI18-repair.md), [acceptance](ui18-acceptance.json). Nativeacceptedpreviewbounds fallback fixesTOI initial/F/menu onallfive routes.19relatedscenes×5routes:18whole-boundfits/route,ShapeCastDebugkilometrepreview explicitly1000mVIEW_LIMIT_ROUTING_ONLY.858recordedchecks inclfocusedrestart;3originalfalseharnesschecks/resolutions preserved. Actualinputs/geometry/defaultangles/nativecamera/bodyfulbounds/.75padding/IDs/picking unchanged.

All8CPUfirstgrids inspected (10before+112afterimages);CPUinitialTOI/Tree/picking individually inspected. ShapeCastDebug range limitation preserved;Z-up unavailable inactualpreviewUI..

Current pool: **ui18-build.json/link_plan PASS**; 5 viewers, 30 direct task objects and the small combined API archive. Exact required inputs are in ui18-acceptance.json.resources.active_task_objects; next consumer is the next PR10 relink. Older manifests are historical and may have retired files/stale pins. Verify only the current pool.

Cleanup: raw 946368220 B; free 1707734687744 B; selected evidence 36872020 B. Removed 171374429 B of superseded builds, 65925876 B of consumed streams and 58956 B of logs. 59 recorded PIDs absent, no owned cores, measured shared-cache growth 0 B. Shared caches preserved. No live job; this batch is closed.

Freeze a new finite budget before execution: 1 GiB whole raw tree, 64 MiB retained evidence, 16 MiB cache growth/app and 32 MiB/repair, at least 20 GiB free. Replace viewers sequentially; audit/remove consumed task outputs before approaching the cap. Preserve media, current inputs and user work.

Box3D/WASM/Rust/WGSL/shared archives unchanged. CMake generates native UI copies with Bun; full fresh configure/build/native+WASM compatibility, final affected-scene recordings and criterion audit remain OPEN. PR10 unchecked; goal ACTIVE. No PR10 commit/push authorization.

## Recovery cautions

UI12 original CPU leading-0 crash/GDB evidence is sufficient; do not repeat crash/core extraction. Its three wrong analytic-zero probes were adjudicated from the same data against native minimum half-width 0.02. UI14 two wrong cache-equality probes included the hit flag; native fresh hit=0/reused hit=1 is correct. Original receipts/flags and adjudications remain preserved. UI11 two registry-vs-world-counter probes and UI10 observer symbol-chain/pin corrections remain in their reports. Never call original false flags all true.

STARTUP01 is PR05/08, Rain physics is PR04, query qualification is PR11, category truthfulness is PR02. No exhausted core/cold/solver/GTK campaigns. OBS01 combined Body Cast pale appearance is unconfirmed; OBS02 GTK dialog timeouts are preserved, not a reason to rerun unchanged GTK.

## Next actions

Next UI21 mesh ragdoll Respawn anchor leak. Read UI21.md/body-properties-controls-acceptance.json/native shared/human.c DestroyHuman/Human_CreateParallelAnchors and sample_ragdoll.cpp. Actual29→43→57 bodies causedby14anchorIDs omittedbyDestroyHuman beforeCreateHuman overwrites them. Repair generated native Human lifetime helper withoutdirtyBox3D/default/geometry/physics changes; cover mesh repeatrespawns plus Box/noanchors andrelatedactualanchorowners, actualoldIDinvalidity/joints/defaults/picking/lifecycle. Freeze newfiniteCcompile/link/app/capture/storagebudget;reusecurrentUI18pool andcurrentobserverchains. ThenUI22–29/UI03/OBS01–03/freshcompatibility/finalrecordings/criterionaudit;continueuntilPR10complete thenstop.

UI21objects READY:5actualarchiveHumanmodules byteexact;1C/1C++compiles complete,onlyDestroyHumananchorcleanup delta proof. ui21-build CANDIDATE_OBJECTS_READY/noapps. UI21controls mustexistbefore run;19verified10open.

UI21stopped: AssertionError [ERR_ASSERTION]: ;ui21-build/current operations inspect;noissueclosure.19verified10open.

UI21CPU/ordinaryGPU/nativeGPUqualified35checks each,8.84/10.72/10.93s;originalanchor/bone/jointIDs invalid,3Meshrespawns stable29+Box15+Reflection20destructors+picking. Combinedlink stoppedby96MiB newbuildcap (wholeRaw987.6MB<1GiB);partialunlaunchedbinaryconsumed/hashreceiptui21-original-build-size-stop.json. Distinctremaining2combinedlinks/apps90s+240souter/0builds FROZEN;correctnewbuild192MiB/wholeRaw1GiB unchanged;noqualifiedroute repeats.19verified10open.

UI21stopped: AssertionError [ERR_ASSERTION]: exitCode===null&&this.app.signalCode===null,`Viewer exited ${this.app?.exitCode}/${this.app?.signalCode}`);assert(performance.now()-this.start<this.budget.seconds_per_app*1000,'App time budget');for(const [f,cap]of[['observer.jsonl',this.budget.observer_bytes_per_app],['api.jsonl',this.budget.api_bytes_per_app],['stdout.txt',1024**2],['stderr.txt',4*1024**2]]as const){const p=join(this.dest,f);assert(!existsSync(p)||statSync(p).size<cap,`${f} byte cap`)}if(performance.now()-this.lastResource>1000){this.lastResource=performance.now();assert(tree(join(D,'raw')).lo;ui21-build/current operations inspect;noissueclosure.19verified10open.

UI21ordinarycombinedactualanchorcleanup28checks pass (Mesh3respawns/restart+Box),thenSelectMesh/ReflectionSIGABRT;noissueclosed/nativecombinedunlaunched. Originalstderrconsumerdiscardedlateassertion (startup8only),explicitmissingdiagnostic. Consumerfixedboundedlast32nonPERFlines. Distinctactual__assert_fail readonlytraceFROZEN1CPP20s/1link25s/1ordinarycombined30s/outer85/0captures/steps,minimumsameunresolvedtransitionsetup;nofullqualifiedroute repeat. Seeui21-original-reflection-abort.json.19verified10open.

UI21assertiontraceTERMINAL:ui21-assert-execution.json/native_assertion andboundedlate stderr retained;diagnosticduplicateviewer/object consumed,ownedPIDsabsent. Noqualificationclaim;readactualboundary beforeanyfurtherbuild/run.19verified10open.

UI21abortattribution: diagnosticJointRef asserts512;Reflection20humans*21joints*2owners840, originallastacceptedpublic/CPUjoint249, compiled512assertionstring. Firstreadonlyasserttrace omittedJOINT_TRACE, actualReflectionReached/cleanquit7.20s;flagcontrolpositive. Exactoriginalassertmessageunretained;attributioninference explicit. Noenginebug/solverfixclaimed. DISTINCTcompletionFROZEN0compile/1nativecombinedlink/3apps (nativecore45s +2Reflection25s)/outer135. Mesh/Boxjointchecks retained;ReflectionseparateJOINT_TRACEoff+actual20HumanIDs/slots/picking. Prior3fullroutes/ordinarycore reused.19verified10open.

UI21five-route qualification COMPLETE:ui21-build PASS;ui21-composite-evidence.json records3fullroutes +ordinaryMesh/Boxsubset+Reflectioncompletion +nativefirstcore/Reflectioncompletion. AllactualanchoroldIDs/joints/slots stable29,Box15,Reflection20destructors/picking checks pass. Diagnostic512limit separatefixture/JOINTTRACEoff onlyReflection;originalabort/sourceinference preserved. Allcurrent5viewers/32objects nextrelink;3gridscloseout+inspection/registryupdate pending.19verified10open.
