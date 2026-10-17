# PR11 query qualification — recovery log

Read this FIRST at continuation and after compaction/restart, then AGENTS.md, docs/goals/gpu-production-readiness.md and the goal-scratchpad skill. This is the current state; dated batch histories are closed evidence, never next actions.

## Objective and authorization

Complete one roadmap item, PR11, in /home/firtoz/work/2026/box3d-wasm on feat/gpu. User activated it on2026-10-17. Reproduce and repair query behavior/appearance discrepancies using real Box3D CPU and ordinary/native GPU, identical frozen inputs, existing1e-5 limits and calculation-first traces. Preserve scene geometry, stepping defaults, identity and picking; keep Box3D/WASM unchanged. No unrelated solver repair, full sample resweep, coldstart/GTK/scheduling replay or recording campaign. New automation uses TypeScript/Bun. Recordings/grids are optional. User authorized a PR11 commit on 2026-10-17, then continuation with PR02. Push was not requested.

## Final state — complete,7 done /0 left

PR11 is verified locally. Starting and fetched HEAD/origin are d03ab8cc2530b7fc6871e6c6a40b7a719ea23c5a,0ahead/0behind. Current worktree contains this uncommitted PR11 change. PR02 is now activated by the user; no other roadmap item is activated. Full result and original limitations: [report](README.md), [final acceptance](final-acceptance.json).

| Gate | Status and evidence |
| --- | --- |
| A1 Original failures and first operation | Done: baseline-corrected CPU/ordinary/native receipts; mesh culls occur before GJK |
| A2 Verified repair and related boundaries | Done: repaired-comparison.json; both exact scene constructions plus two-sided/stationary controls |
| A3 Independent query contract | Done: contract-comparison.json,601records/backend,180ray inputs,explicit BodyCast transforms; encroach-comparison.json,216additional mesh cases |
| A4 Filters,callback semantics,lifetimes,picking identities | Done: default/bit2/bit63 filters,nearest shape/body ownership,teleport/destruction; callbacks-result.json,4nested response modes/backend |
| A5 Height Field numeric watch | Done:14,278frozen inputs/13,340hits; actualCPU selected-target arithmetic passes1e-5;93adjacent-facet and2cyclic-anchor conventions explicit |
| A6 Actual display/camera and BodyCast appearance | Done:14checks/combinedroute,actual draw/query/camera inputs,same-process scene changes and8inspected images |
| A7 Regressions,docs,source audit and cleanup | Done:25query tests/backend,diagnostic/workspace typechecks,unchangedBox3D/WASM,final-acceptance.json |

## Findings and decisions

- F1 Initial Overlap zero-translation cast originally CPUcallback1/GPU0; fixed by removing inappropriate mesh directional culling. Native initial-overlap normal remains zero.
- F2 Shifted Overlap World box originally CPUcallback1/GPU0; fixed by removing mesh center/backface culling. Overlap and shape casts are two-sided; ray casts keep native sidedness.
- F3 Standalone C default categoryBits1 differed from CPU UINT64_MAX; corrected and mask-tested. Standalone world shape-cast child-1 also corrected toCPU0 without changing compound child indices. PR02 whole gate is not automatically certified.
- F4 Long Height Field queries used rounded world triangles/restarted simplex. Repair uses local first-vertex frame,relative pose advancement and cached simplex,preserving witness normals. CPU frame trace isolates the cause. Of95residual cases,93choose edge-sharing near-tied facets;2have cyclic vertex-order/anchor rounding. ExactCPU selected-triangle outputs pass1e-5,max1.49e-8. Rotating just the2CPU input orders reproduces CPU whole-grid outputs exactly. No whole-grid point/normal identity or tolerance increase claimed.
- F5 BodyCast OBS01 resolved without palette change: actual white miss sphere at[-6.5,2.5,.5],drawRGBA[1,1,1,1]. Frozen shifted CPU/GPU queries all miss. Capsule overlap colour is separate.

## Evidence and limits

[Before/after index](README.md),[media map](media.json),[affected caller inventory](affected-scenes.json),[original frozen Height Field differences](heightfield-before-failing-pairs.json),[triangle frame trace](triangle-trace.jsonl),[selected-facet oracle](facet-oracle-comparison.json),[vertex-order trace](vertex-order-comparison.json),[test logs](checks/).

Affected scene source applicability is recorded explicitly; only InitialOverlap/ShapeCast/BodyCast are fresh native viewer checks, and exact frozen scene fixtures also cover OverlapWorld/BenchmarkHeightField. Shared default-filter correction applies to callers using those defaults. This is not another165sample sweep. Combined CPU geometry is actual prefixedBox3D on the left; Step query overlays are shared GPU-result draws,with independentCPU numeric fixtures as reference. Info panel obscures some right-pane objects; recorded draw/query/camera inputs verify placement. Exact pixels/chaotic trajectories,other hardware/OS,mover solver and broad performance/physics release claims remain outside this milestone. Tested Manjaro/RTX4070SUPER/driver610.57.04/Vulkan.

## Closed budgets and cleanup

Every execution batch was frozen before launch with time/output/process limits,core0 and cleanup. [Original batch history](history-batches.md) retains all budgets,failures and refinements: wrong harness CPU body lookup,stale output-file tail,obsolete UI schema/missing state hook/include and TS7 CLI flag. None is a product fix or an active experiment.

Final audit PASS: task-owned336398948bytes of raw builds,viewers,generated driver and duplicate outputs removed;about5.1MiB of selected evidence retained before final prose updates. Compact heightfield-cpu-reference.json is consumed by src/fixtures/mesh_casts.rs. Shared Cargo/driver/pipeline caches and previous local work preserved. Both qualified viewer processes exited0; no owned jobs remain. raw/ is locally excluded and absent. Build identities are historical and retired; do not poll or replay them. Reproduction uses a fresh GPU_PR11_CHECK_OUT directory.

## Next action

Commit this verified PR11 milestone, then audit PR02 under a new finite protocol. Do not resume closed PR11 budgets or historical solver campaigns. Push was not requested.
