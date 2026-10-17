# PR05 / PR06 current recovery — COMPLETE LOCALLY

Read this FIRST after continuation/compaction/restart, then AGENTS.md, docs/goals/gpu-production-readiness.md and the goal-scratchpad skill. Current receipts override historical notes.

## Objective / authorization / stop

User requested PR05 runtime safety and PR06 reproducible release builds/consumer integration on2026-10-17. Both are complete: **PR05 8done/0left; PR06 5done/0left. Stop here. User authorized this verified milestone commit on 2026-10-17; push was not requested.** Workspace /home/firtoz/work/2026/box3d-wasm, branchfeat/gpu, HEAD7792af8f7. One boundedfetch succeeded;2ahead/0behind, no incoming changes/integration needed. User roadmap reorder preserved;next only onnewrequest isPR04→PR07→PR08→PR12→PR13. All existingPR04Rain/dragging/CCD physicalfailures remainopen atoriginaltolerances. Later relevantchanges requirePR06applicabilityreview/refreeze. Do not resume old exhaustedexperiments or startPR04automatically.

## Acceptance and evidence

[README.md](README.md) explains defects,scope,commands,limits and original failures. [acceptance.json](acceptance.json) closes all13gates; [final-artifact-verification.json](final-artifact-verification.json) checks all20compiledartifacts against their originalbuildreceipts. [process-cleanup.json](process-cleanup.json) verifies277recordedPIDsabsent and noGPUskipmarkers. [evidence-applicability.json](evidence-applicability.json) distinguishes historical runtime binaries from final diagnostic-only source change.

| Gate | State | Evidence |
| --- | --- | --- |
| P1 Bounds/layout/reservation | PASS |98mainRusttests+actualdispatch controls |
| P2 IDs/ownership/replacement | PASS |epochs/stale IDs/replacement/multiworld Rust/C |
| P3 Callbacks/events | PASS |CPUcontrol+deadlockbaseline,4routeguards/query/event assertions |
| P4 Async/replay/failure | PASS |actualdevice.destroy terminalflag,clearcannotheal,48stepreplay/invalidation/owner checks |
| P5 Lifecycle/loading | PASS |96public lifecyclecycles,9viewers×10singlesteps+scene switch,4actualworker-loading closes |
| P6 Launch/cache matrix | PASS |CPUfirst/ordinary/native standalone+combined,on/off,actualNVIDIAVulkan/cacheimportpolicy |
| P7 Resources/concurrency | PASS |ownershipaudit/recursion/threadisolation,FD24/24+threads6/6plateau;explicitCownerthread |
| P8 Docs/evidence/cleanup | PASS |finalreport/applicability/process/storage/sourceproof |
| B1 CPU clean build | PASS |emptyCMake build+18frameviewer/0idle/10singles/1switch |
| B2 Ordinary clean targets | PASS |emptyRusttarget/GPU/combined,4on/off viewers |
| B3 Native clean targets | PASS |emptyRusttarget/GPU/combined,4on/off viewers+actualreplay controls |
| B4 Consumer/compatibility | PASS |5CPUfirstexternalconsumers at1e-5,maxerror0;4finalcallbackguards;TS/demo/WASM |
| B5 Freeze/applicability | PASS |actualcompiledsource/header/generatedsource+dependencies/toolchain/recipes+20artifacts |

## Repairs and preserved failures

1. Callback b3Shape_GetBody re-lock deadlock: actualCPUcontrolcompleted;ordinary/nativeGPUtimedout8s atthatgetter. Ownedshape/body/world identity snapshot and commonRust/CrecursiveEBUSYguard nowpreventlockwait andpre-Cmetadata/CPU-mirrormutation. Main82Rustselectorprocesses/98tests+34Cfixtures and4finalreleasecallbackguards pass.
2. Controlled actualdevice.destroy previouslyleft invalid=false/fail=false. Sharedfaultflag nowmakesallsharingworldsterminal/observable andstopssteps/mutation;diagnosticclearcannotheal;teardownsafe;independentdevicehealthy. Cshareddevice requiresprocessrestart;rawRustcancreatenewdevice. Uncontrolleddriverresetnotqualified.
3. Fullstate recorderexpected14byte replaytrailer butproducerhas2u32+8flags=16. Typed sharedflagcountcorrectsdiagnosticdecoder;productionkeybytes/order/shader/solverunchanged. Originalfailure retained,48stepfullstate/replayhit/invalidationchecks pass. Onlysim.rs diagnostic/countchangedaftermainruntimecandidate;finalcleanbuildcompilesit.

Firstloadingharnessfailedtoidentifywindow: Sokol setsno_NET_WM_PID,soxdotoolPIDpropertysearchcannotwork. It sentnoclose;viewerready0.917s;15sprobe capreapedit. Exactoriginalharness hashmatched/preserved. Test-only forwardingXCreateWindowobserver suppliedactualchildPID/XIDwithoutviewer/engineedit. All4actualWM_DELETEworker-loadingclosesPASS. Correctedphase5jobsallzero/accepted;its finalhashcheckthenfailedonintentionallyconsumedhelperENOENT. OriginalFAILEDreceiptretained;loading-final-reconciliation.json verifiesallruntime/log/source/processproof withoutGPUreruns. Do not relaunch eitherclosedphase. Otheroriginalpreparation/diagnosticfailures also retained.

## Build / storage / recovery state

All jobs terminal; no owned live process or raw directory. Sevenreleasebuilds:2emptyoffline/lockedRusttargets with external-c-shim,replay-diagnostics(+native-command-cache),then5independentemptyCMake dirs CPUfirst. No historicalcompiledobjects reused. Sourcecaches pinned. Actualdep-info/generatedsource/headers/commands/toolchains/libs/executables retained inreceipts. NoBox3D/WASM/shader/scene/default change. ExistingPythonbuildgenerators unchanged;allneworchestrationTypeScript/Bun.

Officialcompatibility:rootworkspaceTS,twodisposable demo builds(includingfreshWASM),SDK6.0.2unchanged-sourceWASMbuild andSingleBoxframes0/1/2 pass. SDK/build/copiedpatchedsource/publicWASMlink allremoved, restoringoriginalabsentpaths. Box3Dsubmoduleclean. No recording/sweep.

Frozenexception6GiBtemporary/512MiBignoredcandidate/64MiBtrackedevidence/free20GiB,2compilerjobs,perjob+phasetimeouts/core0/ownedgroupreap;allcapsmet. Sampledrawpeak2,127,978,845bytes (1.98GiB);evidenceabout16MiB. Candidate20files/367,035,504bytes (350MiB) at experiments/gpu-physics/artifacts/pr05-pr06-release-candidate-2026-10-17, ignored byGit;namedconsumerPR07orPR04applicability/refreeze. It is requiredfreezeoutput, not anobject/dependencycache. Retainuntilreplacement/qualification;removeobsoletecandidateafterrefreeze. Sharedpipelinecache269,836,621B/NVIDIAcache1,053,651,339B andpriorRusttargets preserved. Alltaskdependency/object/SDK/helper/traceoutputsconsumed/deleted. Originalruntime/recoveryhistory is initial-runtime-recovery.md.

Publication: user authorized committing this verified milestone on 2026-10-17. Source, artifact and retained-log hashes were rechecked before staging. Commit includes source/docs/evidence and checks/*.log; ignored release binaries remain local. The commit containing this recovery record publishes the milestone; its base is7792af8f7. Push and PR04 implementation were not requested. If askedfornextroadmapitem, recommendPR04andreaditsoldsolver/rejectedexperiments beforechoosinganewminimalhypothesis;do not silently reuseobsoletecandidateevidence.
