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
| Repairs | 23 verified / 6 open |
| Final PR10 acceptance | Open |

The 165 dispositions comprise 71 actual specific-control fixtures and 94 explicit shared-code reuses, not 165 new runtime/feature passes. Five inactive Ragdoll/Pose widgets are excluded; 342 active entries remain the denominator. See inventory.json and sample-applicability.md/json.

Verified: UI01, UI02, UI04, UI05, UI06, UI07, UI08, UI09, UI10, UI11, UI12, UI13, UI14, UI15, UI16, UI17, UI18, UI19, UI20, UI21, UI22, UI23, UI24. Open: UI03, UI25, UI26, UI27, UI28, UI29. Individual reports preserve original findings, probes, limitations and evidence. issues.json is authoritative.

## Current evidence and resources

UI24 [x]: [repair](UI24-repair.md), [acceptance](ui24-acceptance.json). Native flat heightfields use equivalent GPU minHeight0/heightScale0/scaleY1 encoding instead of passing rejected scaleY0. Original data/render scale/holes/materials/winding/combined CPU input preserved. Five routes/20 actual native cases/16 accepted Rust create boundaries/12 flat GPU AABBs versus real CPU, 207 passing checks; 131072 header decoded-vertex equivalences. Paused/defaults/restart/picking pass; no Rust/Box3D/WASM/shared-archive changes.

All three CPU-first before/after/regression grids inspected: GPU floors restored with matching holes; positive restoration/default restart/fresh picking visible. Real CPU first, combined GPU tint intentional.

Current pool: **ui24-build.json/link_plan PASS**; 5 viewers, 34 direct task objects plus four C module objects and three small API carrier archives. Exact required inputs are in ui24-acceptance.json.resources.active_task_objects; next consumer is the next PR10 relink. Older manifests are historical and may have retired files/stale pins. Verify only the current pool.

Cleanup: raw 847855412 B; free 1707650760704 B; selected evidence 10841152 B. Removed 160283549 B of superseded builds, 9467397 B of consumed streams and 616 B of logs. 39 recorded PIDs absent, no owned cores, measured shared-cache growth 0 B. Shared caches preserved. No live job; this batch is closed.

Freeze a new finite budget before execution: 1 GiB whole raw tree, 64 MiB retained evidence, 16 MiB cache growth/app and 32 MiB/repair, at least 20 GiB free. Replace viewers sequentially; audit/remove consumed task outputs before approaching the cap. Preserve media, current inputs and user work.

Box3D/WASM/Rust/WGSL/shared archives unchanged. CMake generates native UI copies with Bun; full fresh configure/build/native+WASM compatibility, final affected-scene recordings and criterion audit remain OPEN. PR10 unchecked; goal ACTIVE. No PR10 commit/push authorization.

Reversible whitespace compaction saved166655909B across308 terminal receipts; exact original bytes/hashes reconstructable (storage-receipt-compaction-plan.json and storage-receipt-compaction-apply.json). Current raw847855412B. Original before/after screenshots and all native values preserved.

## Recovery cautions

UI12 original CPU leading-0 crash/GDB evidence is sufficient; do not repeat crash/core extraction. Its three wrong analytic-zero probes were adjudicated from the same data against native minimum half-width 0.02. UI14 two wrong cache-equality probes included the hit flag; native fresh hit=0/reused hit=1 is correct. Original receipts/flags and adjudications remain preserved. UI11 two registry-vs-world-counter probes and UI10 observer symbol-chain/pin corrections remain in their reports. Never call original false flags all true.

STARTUP01 is PR05/08, Rain physics is PR04, query qualification is PR11, category truthfulness is PR02. No exhausted core/cold/solver/GTK campaigns. OBS01 combined Body Cast pale appearance is unconfirmed; OBS02 GTK dialog timeouts are preserved, not a reason to rerun unchanged GTK.

## Next actions

UI25: trace actual character events, ImGui active ID/value and loading-frame transitions for OverlapRecovery .25→.2 and HeightField .75→.7. UI29 may share a cause but is unproved. No unchanged negative retries: use a new finite read-only trace budget before any build/app. Current pool ui24-build.json has5viewers/34directtaskobjects/4Cmodules/3localAPIcarriers, named nextPR10relink consumers. Continue UI25–29/UI03/OBS01–03/fresh compatibility/final affected recordings/criterion audit until PR10 complete, then stop.

UI25/UI29 read-only trace FROZEN:1CPP/2objcopy/2links/CPU+ordinaryGPU90s each, 0physics/retries,280s outer/raw1GiB/retained64MiB. ui25-trace-build.json/current authoritative when launched; UI24 current pool preserved.23verified/6open.

UI25/UI29 shared cause CONFIRMED (ui25-ui29-cause.json): loading-only frame on main ImGui context consumes queued final char/off mouse and clears active widget. CPUcorrect;ordinaryGPU .25→.2/.75→.7 andAutooff only1activation reproduced with newboundarytrace. Product unchanged/noissueclosure. DiagnosticGPUviewer/logs consumed;CPUtraceviewer+3smallchainedobjects named nextrepairverification inputs. Freeze new repair budget before context isolation implementation.23verified/6open.

UI25/UI29 repair FROZEN before product edits: isolate loading-only ImGui context/sharedfont+style, keep main queuedinput/activeID intact;3CPP/4links/4GPUapps90s/0CPUreruns/620s outer/raw1GiB/retained64MiB. ReadonlyCPU trace reused; no worker/world access duringloading. ui25-build.json/current authoritative next execution.23verified/6open.

UI25/UI29 stopped: AssertionError [ERR_ASSERTION]: false == true; inspect ui25-build/current/operations.23verified/6open, no closure.

UI25 link-plan guard caught harness map-key mismatch (active_main_object keys cpu/gpu/both) before any link/app. Corrected to actual keys; original failure retained ui25-link-plan-guard.json. Existing frozen4link/4app allowance unused, no app retry/new compilation.23verified/6open.

UI25/UI29 stopped: AssertionError [ERR_ASSERTION]: false == true; inspect ui25-build/current/operations.23verified/6open, no closure.

UI25 ordinaryGPU complete .25/.75/actualgeometry/defaults/picking/maininput queue retention passed. UI29 two assertions invalid: diagnostic saved pre-on buttonY757, actual button movedY853; original fullreview fresh buttonY853 bothclicks remains valid. ui29-trace-target-adjudication.json preserves all flags and qualifies cause limits. Distinct current-target1ordinaryGPU60s FROZEN; no repeatedUI25. Remaining3GPUroutes unused underoriginalbudget will use fresh button d.click. No issueclosure;23verified/6open.

UI29 corrected-current-target ordinaryGPU app PASS: two native Button returns/actual Auto on recreation and off stops IDs/creation/destruction; same productbinary. AllUI25ordinary results reused without rerun;2false stale-target predicates preserved/adjudicated. Focusedconsume initiallycalledinside ownprocess rejectedlive-driver beforedeletion;external afterterminal consumption PASS. Remaining3firstGPUrouteapps+3links FROZEN (ui25-continuation-budget.json),0compiles/productchanges/retries; execute stagecontinue.23verified/6open.
