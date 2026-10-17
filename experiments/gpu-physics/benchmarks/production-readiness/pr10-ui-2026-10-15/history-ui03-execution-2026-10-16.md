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
| Repairs | 28 verified / 1 open |
| Final PR10 acceptance | Open |

The 165 dispositions comprise 71 actual specific-control fixtures and 94 explicit shared-code reuses, not 165 new runtime/feature passes. Five inactive Ragdoll/Pose widgets are excluded; 342 active entries remain the denominator. See inventory.json and sample-applicability.md/json.

Verified: UI01, UI02, UI04, UI05, UI06, UI07, UI08, UI09, UI10, UI11, UI12, UI13, UI14, UI15, UI16, UI17, UI18, UI19, UI20, UI21, UI22, UI23, UI24, UI25, UI26, UI27, UI28, UI29. Open: UI03. Individual reports preserve original findings, probes, limitations and evidence. issues.json is authoritative.

## Current evidence and resources

UI28 [x]: [repair](UI28-repair.md), [acceptance](ui28-acceptance.json). Generated CastWorld shows same size binding asHalf-width forBox/Radius forSphere+Capsule, hidden forRay; default.5/range.1–2/geometry unchanged. FiveCPUfirst routes/40nativeproxy cases/25eightpointBoxcases verifyactualcorners/radius plus10sphere IDs/poses/mass/counts/restart.175recordedchecks,1wrongCPU RayAPI predicate preserved with explicitunchangedCPU Raycoverage reuse;0CPUrepeats.

All three CPU-first before/after/regression grids inspected; actualHalf-width2/.25 visible/editable, CapsuleRadius/Rayhidden/defaultrestart shown. Original32mixed-body before versusfocused10sphere after population explicitly recorded; caster size/type matched.

Current pool: **ui28-build.json/link_plan PASS**; 5 viewers, 39 direct task objects plus four C modules, three API carrier archives, the shared loading module, two generated main modules and chained input diagnostics. Exact required inputs are in ui28-acceptance.json.resources.active_task_objects; next consumer is the next PR10 relink. Older manifests are historical and may have retired files/stale pins. Verify only the current pool.

Cleanup: raw 891914951 B; free 1707575394304 B; selected evidence 13046189 B. Removed 166901207 B of superseded builds, 21655007 B of consumed streams and 475 B of logs. 17 recorded PIDs absent, no owned cores, measured shared-cache growth 0 B. Shared caches preserved. No live job; this batch is closed.

Freeze a new finite budget before execution: 1 GiB whole raw tree, 64 MiB retained evidence, 16 MiB cache growth/app and 32 MiB/repair, at least 20 GiB free. Replace viewers sequentially; audit/remove consumed task outputs before approaching the cap. Preserve media, current inputs and user work.

Box3D/WASM/Rust/WGSL/shared archives unchanged. CMake generates native UI copies with Bun; full fresh configure/build/native+WASM compatibility, final affected-scene recordings and criterion audit remain OPEN. PR10 unchecked; goal ACTIVE. No PR10 commit/push authorization.

## Recovery cautions

UI12 original CPU leading-0 crash/GDB evidence is sufficient; do not repeat crash/core extraction. Its three wrong analytic-zero probes were adjudicated from the same data against native minimum half-width 0.02. UI14 two wrong cache-equality probes included the hit flag; native fresh hit=0/reused hit=1 is correct. Original receipts/flags and adjudications remain preserved. UI11 two registry-vs-world-counter probes and UI10 observer symbol-chain/pin corrections remain in their reports. Never call original false flags all true.

STARTUP01 is PR05/08, Rain physics is PR04, query qualification is PR11, category truthfulness is PR02. No exhausted core/cold/solver/GTK campaigns. OBS01 combined Body Cast pale appearance is unconfirmed; OBS02 GTK dialog timeouts are preserved, not a reason to rerun unchanged GTK.

## Next actions

Next UI03 debug overlays: native GPU renderer consumes only drawShapes and returns early when shapes hidden. Source-audit actual available direct GPU body/joint/contact getters and required native callback semantics; implement supported overlays using actual GPU data, preserve allflags/defaults/culling/IDs/picking. Any genuinely unavailable data needs explicit honest UI/capability disposition, not CPU-backed fake outputs. NEW finite build/check/capture/storage budget before execution. Current ui28-build.json:5viewers/39taskobjects/currentCollision+Tree/loading/API modules/3carriers nextPR10relink. Then OBS01–03/fresh native+WASMcompatibility/finalaffected recordings/criterion audit; stop afterPR10.

UI03 source-first diagnosis/design saved in UI03-plan.md (read before new budget). Existing directGPU body/joint/contact APIs can support actual overlays without Rust/engine change; sleepVelocity/perconstraintcolor/island membership unavailable via current Cdiagnostics, require honest UI dispositions. Preserve qualified API module sources/carriers and callback provenance. No UI03 execution budget/build/app frozen yet; currentpool ui28-build verified, all prior jobs closed.28verified/1open.

UI03 FROZEN before product edits (ui03-repair-budget.json):4C/2sampleCPP+1readonlyobserver/2objcopy/<=45AR/5links+CPUfirst5apps120s/2scriptsteps/10screens each/raw1GiB/retained64MiB/build330s+check760s. ActualGPU getters and independent renderer sinks; supported overlays plus honest3 unavailable metric controls. Nativejoint drawScale default/metadata andactualStepdt forrenderonly. Box3D/WASM/Rust/WGSL/physicalfields/sharedarchives unchanged. Currentpool ui28-build valid beforechanges; ui03-build/current authoritative afterlaunch.28verified/1open.

UI03 stopped: AssertionError [ERR_ASSERTION]: void gpu_samples_on_world_created(b3WorldId world, const b3WorldDef* def) __attribute__((w

0 !== 1
; see ui03-build.json.28verified/1open; no closure.

UI03 check stopped AssertionError [ERR_ASSERTION]: mes[1].p});const scale=b.inputs[0].jointScale;for(const xf of frames){out.push({op:'point',p:xf.p,size:5});for(const v of[[.3*scale,0,0],[0,.3*scale,0],[0,0,.3*scale]])out.push({op:'line',a:xf.p,b:add(xf.p,rotate(xf.q,v))});}}return out;}
23 | function expectedContacts(b:any,meta:any){const result:any[]=[],seen=new Set<string>(),flags=b.inputs[0],shapes=new Map<string,number[]>();for(const body of meta.worlds.find((w:any)=>w.owner==='public').examples)for(const s of body.shapes)shapes.set(JSON.stringify(s.id),body.id);for(const m of b.inputs.filter((x:any)=>x.op; inspect ui03-build/current/receipts.28verified/1open.

UI03 check stopped AssertionError [ERR_ASSERTION]: ordinary-both READY 1.440
ordinary-both TERMINAL FAILURE_RETAINED
21 |  save(){write(join(this.dest,'receipt.json'),this.r)}
22 |  emit(s:string){console.log(this.label,s)}
23 |  state(){return tail(join(this.dest,'observer.jsonl'),16384).at(-1)}
24 |  latest(api:string){return tail(join(this.dest,'api.jsonl'),this.budget.api_tail_bytes??65536).findLast(x=>x.api===api)}
25 |  guard(){assert(this.app&&this.app.exitCode===null&&this.app.signalCode===null,`Viewer exited ${this.app?.exitCode}/${this.app?.signalCode}`);assert(performance.now()-this.start<this.budget.; inspect ui03-build/current/receipts.28verified/1open.

UI03 continuation2026-10-16: production supported-overlay repair implemented c_abi/debug_overlays.h +samples_api.c; actualStepdt andjointdrawScale metadata inshim/dual; honest3 unavailablemetric menus inBun/CMake. Original4C/3CPP/5links complete. CPU29overlayspass, finalpickingtrace envmissing→timeout; explicitunchangedUI24CPU picking reuse. FirstordinaryGPU bounds/mass/joints pass; wrong7names and metadata IDorder assertions adjudicated fromactualsaved8names/2contactpoints, no productfault. NativeGPU46checks PASS/10images/picking. FirstordinaryBOTH baseline diagnostic timedout: sharedtemplate lastrequest suppressedsecondpane; nooverlayproductfail. Separate readonly budget1CPP/5links done, onlyper-owner request bookkeeping changed; oldobserver/source/manifest/historypreserved. Current ui03-build DIAGNOSTIC_RELINKED; raw/ui03-observer-fix/5viewers current. Three remaining apps launched via ui03-observer-fix.ts check under90s/app/310souter: ordinaryremainingcontrols supplement, ordinaryBOTH continuation, firstnativeBOTH. ExactcurrentPID in ui03-build.json; toolsession recorded bycurrentturn. Allbeforemedia retained. Newstepbudgets2/app;noengine/Rust/WGSL/WASM edits. Afterterminal:consumeallstreams, verifyactualinput/sink/math+media,cleanupoldAPIcarriers/modules/generatedsamples,recordUI03beforeafter; thenOBS01–03/freshcompatibility/finalrecording/criterion audit.28verified/1open,342/0widgets165/0samples23/0custom.

UI03 allroutes have applicable positive evidence; see ui03-build.json and initial/corrected/supplement receipts. CPU picking reused; firstordinary names/contact ID selector errors adjudicated; two-pane readonly counter fixed. Media/acceptance/currentpool cleanup pending;28verified/1open.
