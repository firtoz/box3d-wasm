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
| Repairs | 27 verified / 2 open |
| Final PR10 acceptance | Open |

The 165 dispositions comprise 71 actual specific-control fixtures and 94 explicit shared-code reuses, not 165 new runtime/feature passes. Five inactive Ragdoll/Pose widgets are excluded; 342 active entries remain the denominator. See inventory.json and sample-applicability.md/json.

Verified: UI01, UI02, UI04, UI05, UI06, UI07, UI08, UI09, UI10, UI11, UI12, UI13, UI14, UI15, UI16, UI17, UI18, UI19, UI20, UI21, UI22, UI23, UI24, UI25, UI26, UI27, UI29. Open: UI03, UI28. Individual reports preserve original findings, probes, limitations and evidence. issues.json is authoritative.

## Current evidence and resources

UI27 [x]: [repair](UI27-repair.md), [acceptance](ui27-acceptance.json). Generated native MeshScale hit/miss sphere transforms add m_start with b3OffsetPos, preserving actual query inputs/results/mesh/defaults/body+shapeIDs. FiveCPUfirst routes/40nativepreview cases (35hits5misses)/40independentDrawSolidSphere comparisons/230passingchecks. Offsethit/miss/restoration/UI23zeroScale/raymode/restart pass; no querynumerics/solver matching required.

All three CPU-first before/after/regression grids inspected: miss sphere nowat endpoint[2,1,1], hit sphere follows ownactualnativefraction; zero-scale/default/restart preserved.

Current pool: **ui27-build.json/link_plan PASS**; 5 viewers, 39 direct task objects plus four C modules, three API carrier archives, the shared loading module, two generated main modules and chained input diagnostics. Exact required inputs are in ui27-acceptance.json.resources.active_task_objects; next consumer is the next PR10 relink. Older manifests are historical and may have retired files/stale pins. Verify only the current pool.

Cleanup: raw 881831581 B; free 1707594489856 B; selected evidence 5666747 B. Removed 166893934 B of superseded builds, 8066141 B of consumed streams and 375 B of logs. 17 recorded PIDs absent, no owned cores, measured shared-cache growth 0 B. Shared caches preserved. No live job; this batch is closed.

Freeze a new finite budget before execution: 1 GiB whole raw tree, 64 MiB retained evidence, 16 MiB cache growth/app and 32 MiB/repair, at least 20 GiB free. Replace viewers sequentially; audit/remove consumed task outputs before approaching the cap. Preserve media, current inputs and user work.

Box3D/WASM/Rust/WGSL/shared archives unchanged. CMake generates native UI copies with Bun; full fresh configure/build/native+WASM compatibility, final affected-scene recordings and criterion audit remain OPEN. PR10 unchecked; goal ACTIVE. No PR10 commit/push authorization.

## Recovery cautions

UI12 original CPU leading-0 crash/GDB evidence is sufficient; do not repeat crash/core extraction. Its three wrong analytic-zero probes were adjudicated from the same data against native minimum half-width 0.02. UI14 two wrong cache-equality probes included the hit flag; native fresh hit=0/reused hit=1 is correct. Original receipts/flags and adjudications remain preserved. UI11 two registry-vs-world-counter probes and UI10 observer symbol-chain/pin corrections remain in their reports. Never call original false flags all true.

STARTUP01 is PR05/08, Rain physics is PR04, query qualification is PR11, category truthfulness is PR02. No exhausted core/cold/solver/GTK campaigns. OBS01 combined Body Cast pale appearance is unconfirmed; OBS02 GTK dialog timeouts are preserved, not a reason to rerun unchanged GTK.

## Next actions

Next UI28 CastWorld hides active Box size. Expose same existing m_castRadius as Half-width for Box, Radius for Sphere/Capsule, hidden forRay; preserve defaults/geometry. Read UI28/nativeCastWorld/actualproxyinput observer and freeze NEW finite budget. Current ui27-build.json:5viewers/39directtaskobjects/currentCollision+Tree/loading/API modules/3carriers nextPR10relink. ThenUI03/OBS01–03/freshcompatibility/finalaffected recordings/criterion audit; stopafterPR10.

UI28 FROZEN:1Bungeneration/1CPP25s/5links25s/5CPUfirstapps60s/0physics/retries/530souter/raw1GiB/retained64MiB. SharedCastWorldHalf-width/Radius slider andactualnative8point/radius effects;IDs/poses/defaults unchanged. Current ui28-build.json/current when launched.27verified/2open.

UI28 stopped: AssertionError [ERR_ASSERTION]: false == true; inspect ui28-build/current/operations.27verified/2open; no closure.

UI28 CPU complete11.89s/all8actualproxy sizecases pass;1Ray predicate wrong API name (CastRayClosest vsnativecallbackCastRay). FreshCPU RayUI confirms hidden labels; query/IDs snapshot notsaved, inherited originalactualCPU unchangedRay coverage explicitly. ui28-cpu-ray-adjudication.json preservesfalseflag. Remaining4firstGPUlinks/apps continuation FROZEN/noCPUrepeat/productchange; correctedCastRay name and retainRay state.27verified/2open.
