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
| Repairs | 29 verified / 0 open |
| Final PR10 acceptance | Open |

The 165 dispositions comprise 71 actual specific-control fixtures and 94 explicit shared-code reuses, not 165 new runtime/feature passes. Five inactive Ragdoll/Pose widgets are excluded; 342 active entries remain the denominator. See inventory.json and sample-applicability.md/json.

Verified: UI01, UI02, UI03, UI04, UI05, UI06, UI07, UI08, UI09, UI10, UI11, UI12, UI13, UI14, UI15, UI16, UI17, UI18, UI19, UI20, UI21, UI22, UI23, UI24, UI25, UI26, UI27, UI28, UI29. Open: . Individual reports preserve original findings, probes, limitations and evidence. issues.json is authoritative.

## Current evidence and resources

UI03 [x]: [repair](UI03-repair.md), [acceptance](ui03-acceptance.json). Real GPU bounds/COM/mass/names/jointframes/reactions/contactmanifolds now reach actual renderer sinks, including Shapesoff; unavailable Sleep/Graph/Islands honestly disabled or CPU-pane-only. Five CPUfirst routes,249 recorded checks/seven bounded apps, full-ID dedup and unchanged geometry/physical defaults/IDs/picking; CPU picking and source-identical readonly-observer checks explicitly reused. Original harness errors retained/adjudicated.

All65 cells in three CPUfirst before/after/regression grids inspected: GPU names/mass/bounds/joints/contactforce lines present; Shapesoff overlays remain; three availability labels are honest; picking visible. Actual renderer input/sink checks complement small/occluded glyph pixels; no exactpixel or solver parity claim.

Current pool: **ui03-build.json/link_plan PASS**; 5 viewers, 40 direct task objects and four C module objects and three small API carrier archives. Exact required inputs are in ui03-acceptance.json.resources.active_task_objects; next consumer is the next PR10 relink. Older manifests are historical and may have retired files/stale pins. Verify only the current pool.

Cleanup: raw 908654112 B; free 1707550420992 B; selected evidence 28195077 B. Removed 335455602 B of superseded builds, 99414495 B of consumed streams and 6324 B of logs. 66 recorded PIDs absent, no owned cores, measured shared-cache growth 0 B. Shared caches preserved. No live job; this batch is closed.

Freeze a new finite budget before execution: 1 GiB whole raw tree, 64 MiB retained evidence, 16 MiB cache growth/app and 32 MiB/repair, at least 20 GiB free. Replace viewers sequentially; audit/remove consumed task outputs before approaching the cap. Preserve media, current inputs and user work.

Box3D/WASM/Rust/WGSL/shared archives unchanged. CMake generates native UI copies with Bun; full fresh configure/build/native+WASM compatibility, final affected-scene recordings and criterion audit remain OPEN. PR10 unchecked; goal ACTIVE. No PR10 commit/push authorization.

## Recovery cautions

UI12 original CPU leading-0 crash/GDB evidence is sufficient; do not repeat crash/core extraction. Its three wrong analytic-zero probes were adjudicated from the same data against native minimum half-width 0.02. UI14 two wrong cache-equality probes included the hit flag; native fresh hit=0/reused hit=1 is correct. Original receipts/flags and adjudications remain preserved. UI11 two registry-vs-world-counter probes and UI10 observer symbol-chain/pin corrections remain in their reports. Never call original false flags all true.

STARTUP01 is PR05/08, Rain physics is PR04, query qualification is PR11, category truthfulness is PR02. No exhausted core/cold/solver/GTK campaigns. OBS01 combined Body Cast pale appearance is unconfirmed; OBS02 GTK dialog timeouts are preserved, not a reason to rerun unchanged GTK.

## Next actions

Resolve final observation dispositions from issues.json and evidence (OBS03 is CPU TOI combo Escape watch; separately audit Replay modal findings), then fresh native/WASM compatibility and final affected-scene/criterion audit. Stop after PR10; no commit/push authorization.

Finalcontinuation2026-10-16: UI03recorded29verified/0open; inventoryshared-host reuse all165 explicit/342widgets/23custom/85shared done. Currentui03-build PASS,5viewers/4Cmodule objects/3carriers. UI03 all65gridcells inspected,28,195,077B selected/raw908,654,112B;66PIDsabsent/cache0. final-observations.json dispositions all3:OBS01unconfirmedBodyCastappearance routedPR11;OBS02prioractualGTKtwo-Return acceptance/load reused,laterone-Returnhelperfailures kept;OBS03TOIcomboEscape inheritsdisablednativeNavKeyboard policy, originaleventuntraced (NOTReplaymodalwatch). No newOBSapps.
Freshfinal-compatibility.json native5/5configure/build/smokes PASS fromrootdeliverysources usingimmutableordinary/nativeRustarchives;2stepsBodyType each;111–~115s/build,peak15ownedprocesses;all5buildscratch removed. Fetchoriginfeat/gpu0ahead0behind. Initialfulltypecheck TS2688missingBuntypes: rootcatalogexistedbutnodependencydeclared. final-types-dependency.json PASS afterrootdevDependency @types/bun catalog +5locklines;3,084,038Brequireddependencygrowth; bunrun typecheck andfreshVitebuildPASS. Originalfailure retained;newdemo-dist scratch stillneedsfinalcleanup. No WASMartifacts orSDKexists, asynchronousexistingpathquestionpendingoptional. final-wasm-compatibility-budget.json FROZEN isolatedpinned6.0.2SDK/unchangedbridge+patches/defaultfixedheap/runtimeofficialSingleBox0,1,2;explicit3GiBtemporary/600souter/180sinstall/build/4compiler/32group/log2MiB;allSDK/download/patchedsource/newtemporarypublicsymlinkremoved terminal. Currentjob launched: final-wasm-compatibility.ts run, handlein final-wasm-compatibility.json.current, toolsessioncurrentturn; scratch /tmp/box3d-pr10-wasm-2026-10-16, ownedgenerated packages/box3d-wasm/.box3d-patched onlyafterprepare, demo/public/wasm temporarysymlinkonlyifcreatedbyjob. Do notrestartlivejob.
AfterWASMterminal: inspectreceipt,cleanremainingVitedist/consumedlogs,updatefinalnativecompatstatus/source/cachechecks,buildPR10evidenceindex andfinitecriterionaudit withexplicitprecommitrecordingapplicability(noPR10commitpushauthorized),update roadmap/log,completegoalonlyafterallapplicablegatespass;stopafterPR10. KeepBox3D/WASMbridge/Rust/WGSL unchanged. No solver/query/cold/exhaustedcampaign.

WASMstorageguardcorrection: first SDKinstallterminatedat.5s because /tmp is7.8GiBRAMfilesystem with6.8GiBfree, below frozen20GiBminimum. NoWASMconfigure/build/runtimeattempt;820798BtaskSDKsource removed. Archived final-wasm-tmpfs-guard.json/budget andrawlogs preservefailure. Correctedfrozenoneattempt scratch onphysicalhomefilesystem /home/firtoz/.cache/box3d-pr10-wasm-2026-10-16;same3GiB/20GiBfree/600s limits, sourcearchiveSHA requiredidentical. Currentfinal-wasm-compatibility.json exactrunninghandle; do notrestart.29verified/0open; finalgoalstillactive.
