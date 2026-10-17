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
| Repairs | 18 verified / 11 open |
| Final PR10 acceptance | Open |

The 165 dispositions comprise 71 actual specific-control fixtures and 94 explicit shared-code reuses, not 165 new runtime/feature passes. Five inactive Ragdoll/Pose widgets are excluded; 342 active entries remain the denominator. See inventory.json and sample-applicability.md/json.

Verified: UI01, UI02, UI04, UI05, UI06, UI07, UI08, UI09, UI10, UI11, UI12, UI13, UI14, UI15, UI16, UI17, UI19, UI20. Open: UI03, UI18, UI21, UI22, UI23, UI24, UI25, UI26, UI27, UI28, UI29. Individual reports preserve original findings, probes, limitations and evidence. issues.json is authoritative.

## Current evidence and resources

UI17 [x]: [repair](UI17-repair.md), [acceptance](ui17-acceptance.json). Both actual type-change branches invalidate the simplex cache before proxy rebuild. All four types/all12 ordered transitions per axis/all five routes pass515 recorded checks:120 actual first-native-GJK boundaries receivecache0, then validindices/reuse/finiteGJK/geometry/defaults/restart. Original four GPU faults reused,none repeated. Read-only distance observer chains through prior observer/nativeonce; allotherhooks preserved.

All20 before/after images inspected in two CPU-first grids; original CPU controls/nativecombined point transition individually inspected.

Current pool: **ui17-build.json/link_plan PASS**; 5 viewers, 24 direct task objects and the small combined API archive. Exact required inputs are in ui17-acceptance.json.resources.active_task_objects; next consumer is the next PR10 relink. Older manifests are historical and may have retired files/stale pins. Verify only the current pool.

Cleanup: raw 916155975 B; free 1707794714624 B; selected evidence 6690394 B. Removed 167591966 B of superseded builds, 31949184 B of consumed streams and 78133 B of logs. 21 recorded PIDs absent, no owned cores, measured shared-cache growth 0 B. Shared caches preserved. No live job; this batch is closed.

Freeze a new finite budget before execution: 1 GiB whole raw tree, 64 MiB retained evidence, 16 MiB cache growth/app and 32 MiB/repair, at least 20 GiB free. Replace viewers sequentially; audit/remove consumed task outputs before approaching the cap. Preserve media, current inputs and user work.

Box3D/WASM/Rust/WGSL/shared archives unchanged. CMake generates native UI copies with Bun; full fresh configure/build/native+WASM compatibility, final affected-scene recordings and criterion audit remain OPEN. PR10 unchecked; goal ACTIVE. No PR10 commit/push authorization.

## Recovery cautions

UI12 original CPU leading-0 crash/GDB evidence is sufficient; do not repeat crash/core extraction. Its three wrong analytic-zero probes were adjudicated from the same data against native minimum half-width 0.02. UI14 two wrong cache-equality probes included the hit flag; native fresh hit=0/reused hit=1 is correct. Original receipts/flags and adjudications remain preserved. UI11 two registry-vs-world-counter probes and UI10 observer symbol-chain/pin corrections remain in their reports. Never call original false flags all true.

STARTUP01 is PR05/08, Rain physics is PR04, query qualification is PR11, category truthfulness is PR02. No exhausted core/cold/solver/GTK campaigns. OBS01 combined Body Cast pale appearance is unconfirmed; OBS02 GTK dialog timeouts are preserved, not a reason to rerun unchanged GTK.

## Next actions

Next UI18 no-body preview framing. Read UI18.md/empty-world-framing-acceptance.json/native Sample::FocusHome, Frame Camera menu and TimeOfImpact direct draw inputs. Actual native World_GetBounds omits preview geometry because there are no physics bodies; original TOI default/F/menu miss all moving capsule poses on all five routes. Trace actual bounds→camera/perspective/projection, then frame real preview bounds while preserving geometry/physics defaults/identity/picking and UI07 split aspect. Cover TOI plus closely related no-body previews sharing the cause; do not assume registration proves framing. Possible shared fix is a native direct-draw preview-bounds fallback for zero-body worlds; investigate exact draw lifecycle/relative origin before choosing. Preserve original camera angles/intended user controls; avoid changing every sample default camera unnecessarily. Fix TOI initial view too if required by its confirmed default-offscreen criterion. Freeze a finite compile/check/capture/runtime/storage budget before execution. Current ui17 pool has chained read-only distance observer; do not duplicate wrappers. Then UI21–29/UI03/OBS01–03/fresh compatibility/final recordings/criterion audit. Continue until PR10 complete, then stop.

Continuation closeout verified: current ui17 pool pins/link inputs and 21 recorded PIDs absent; diff --check passes, Box3D clean. Goal remains ACTIVE/PR10 unchecked;18 verified/11 open. All UI12–17 reports have CPU-first before/after evidence. Historical timing cleanup v2 consumed47,212,789B and retained332,262B summaries; original receipts/media/failure inputs preserved. Next UI18; no budget or live job for it yet.

UI18 FROZEN:5C/8C++ compiles35s(20s projectionobserver),1objcopy5s/2nm5s/10ar5s,11generations10s,5links25s/5apps150s for19scenes each(transition3s),<=120screens/300actions/app/outer1300s. Same1GiB wholeRaw/64MiB retained/cache16MiBapp32MiBtotal;0scriptphysics/retries. Nativeacceptedpreviewbounds + F/menu fallback/full-fitpadding1.25 onlyemptyworlds; bodyfulpadding.75 unchanged. TOIoneinitialframe;otherdefaultcameraangles/radii preserved. CMakeTSpostprocessors/noPythonchanges. 19fixtures nativebounds/matrices/projection; Tree1000mviewlimit ifhitexplicitrouting-only,notwhole-fitpass. Currentui17poolpreflightpinsverified;noUI18jobyet.18verified11open.

UI18 execution failure retained: AssertionError [ERR_ASSERTION]: CanonicalBOTHgfx mustdifferonlydiagnosticprefix/headerposition
+ actual - expected
... Skipped lines

  '#include "sokol_split_step.h"\n' +
    '#include <string.h>\n' +
    '#define SPLIT_STREAM_REMAP\n' +
    '#include "sokol_split_stream.h"\n' +
    '#include "sokol_capacity.h"\n' +
...
    '\tinst->flags[3] = 0;\n' +
+   '\tvis07_payload("line",inst,sizeof(*inst; inspect ui18-build.json/current/operations and logs. No unverified issue closure.18verified11open.

UI18 candidateobjects ready: ui18-build.json CANDIDATE_OBJECTS_READY;all13compiles/10archiveidentities/sourcecanonical/symbolchain proofs complete. No viewers launched yet. Run ui18-repair.ts run within separately frozen900s run budget; completedprepareinputs reused; onlynewcandidate5routes thenretireoldpoolafterqualification.18verified11open.

UI18 execution failure retained: AssertionError [ERR_ASSERTION]: hmarks/production-readiness/pr09-repairs-2026-10-02/PERF05/render-operation-observer/cpu/renderer.o: in function `ResetFrameArena':
renderer.c:(.text+0xdb40): multiple definition of `ResetFrameArena'; /home/firtoz/work/2026/box3d-wasm/experiments/gpu-physics/benchmarks/production-readiness/pr10-ui-2026-10-15/raw/ui18-build/cpu-renderer.c.o:renderer.c:(.text+0xd830):; inspect ui18-build.json/current/operations and logs. No unverified issue closure.18verified11open.

UI18renderer/link correction CLOSED: firstCPUlinkfailed duplicate existingPERF05renderer (0apps);exact5actualdirectsources audited,2byteexactsourcegroups pluspreviewcalls compiled35s each;priorread-onlytiming/VIS07diagnostics preserved,unused2rendererobjects deleted,remaining5firstapps/links unchanged. ui18-build CANDIDATE_OBJECTS_READY;ui18-controls.ts ready.18verified11open.

UI18 execution failure retained: AssertionError [ERR_ASSERTION]: est,'receipt.json'),this.r)}
22 |  emit(s:string){console.log(this.label,s)}
23 |  state(){return tail(join(this.dest,'observer.jsonl'),16384).at(-1)}
24 |  latest(api:string){return tail(join(this.dest,'api.jsonl'),this.budget.api_tail_bytes??65536).findLast(x=>x.api===api)}
25 |  guard(){assert(this.app&&this.app.exitCode===null&&this.app.signalCode===null,`Viewer; inspect ui18-build.json/current/operations and logs. No unverified issue closure.18verified11open.

UI18firstCPUfixture stoppedat3s snapshot timeout: geometryobserverhasno requestfield;noactualpreviewfixturecompleted/no productfindings. Originalreceipt/streamtail preserved; noqualifiedruntime repeated. Distinctschema-correctedCPUcompletion+4unlaunchedGPUfirstapps frozen150s/app/900souter,0builds/4remaininglinks; reuseCPUlinkedviewer. raw/ui18-qualified-runs;18verified11open.

UI18 execution failure retained: AssertionError [ERR_ASSERTION]: false == true; inspect ui18-build.json/current/operations and logs. No unverified issue closure.18verified11open.

UI18CPUactual19fixtures COMPLETE168checks/166originaltrue/2fixedabsolute1e-4boundsprobesfalse adjudicatedsamecaptureddata withfloatrelative→originrebase envelope(ui18-cpu-rounding-adjudication.json),0CPUreruns. ShapeCastDebug nativekilometrecapsule requires7413.39m radius>unchanged1000m limit:explicitVIEW_LIMIT_ROUTING_ONLY,notwhole-fitpass;other18 actual8cornerfitpass. Remaining4GPUfirstapps/4links FROZEN150s/app730souter;0newcompiles/retries;18verified11open.

UI18 execution failure retained: AssertionError [ERR_ASSERTION]: false == true; inspect ui18-build.json/current/operations and logs. No unverified issue closure.18verified11open.

UI18 five19-scene fixtures terminal/852recordedchecks. CPU2arithmeticfalseprobes adjudicatedsame data; nativecombined1compositeRestartdefaultfalse cannotattributeomittedsignature,actualcameraunchanged. NEWfocusedTOIrestartboundary25s/0build/link/capture/physics recordsactualinputs/shapes/camera/world andpasses;receipt+originalfailure preserved. ui18-build PASS;supersededpoolretired/streamsconsumed. Closeout8grids/evidenceinspection pending;18verified11open.
