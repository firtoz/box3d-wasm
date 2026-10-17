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
| Sample widgets | **89 reviewed / 253 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **113 reviewed / 52 left** | sample-applicability.md/json |
| Custom input sample groups | **10 reviewed / 13 left** | manifold-controls-acceptance.json; 27 overridden mouse methods exercised |
| Repairs | **16 open; 2 earlier fixes verified locally** | issues.json; UI01–UI18 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 113 sample dispositions comprise 19 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 113 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Collision review CLOSED: distance-controls.md, distance-controls-acceptance.json and CPU-first distance-controls-grid.html. Three zero-body tools /11widgets all5routes; actual GJK/TOI/draw values and ShapeDistance mouse effects.388recorded checks include40policy failures;9deliberate negative faults (TOI5SIGSEGV, GPUcache4SIGILL), never cleanquit/featurepasses. CPUcache invalidinput survives. Original unfinished CPU DistanceDebug label-hit failure retained; only unfinished work repeated.

UI15 TOI wrong labels/invalid fourth enum, UI16 hidden active radii, UI17 stale proxy cache confirmed all5routes, unfixed. UI18 TOI preview framing also confirmed all5routes: initial/F/menu all moving capsules outside. empty-world-framing-acceptance.json preserves actualcamera/drawposes. Five pan false predicates rejected because harness omittedAlt; source requiresAlt+MMB. No pan defect claimed, no rerun needed to prove independent F/menu failure. Other zero-body tools have source-applicable focus paths but are framing-runtime-unreviewed.

All jobs terminal: distance builds68715/49450, originalmatrix18783exit1, tail84026exit0, framing59753exit0. No live viewer or runtime budget. All owned PIDs absent and no PID-matching system core files. Original engine/gfx/main/Sample207input hashes unchanged. distance-controls-cleanup.json:188991776bytes deleted (full consumed API/observer/timing streams, superseded Manifold5viewers); sharedcachegrowth0; raw379553648bytes. Shared caches/user work preserved.

Current qualified pool: **distance-control-build.json**, five viewers under raw/distance-control-build/. Pass this buildFile explicitly to native-ui-driver; default old geometry binaries were deleted. Retain current pool for remaining controls until replacement qualified, then delete. Existing link objects in raw/ui01-after, raw/ui02-after and other observer objects retained as actual link inputs. CPU Replay fixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec retained for Replay. Selected before/change screenshots are interaction states, not repaired before/after.

Prior Disable/geometry/Manifold reports and acceptance remain valid; source-callsite-audit accounts allactivecalls plus5disabledRagdollPoseexclusions. See history-distance-2026-10-15.md for original closed budgets and earlier recovery state; history-manifold-2026-10-15.md/history-controls-2026-10-15.md/history-2026-10-15.md for older closures. Do not resume exhausted runs.

## Next actions

Continue253remainingwidgets and13custominputgroups across52samples, usingTypeScript/Bun and a NEW finite budget before builds/apps. Read source and derive intended effects first; likely bodymutation/explosion/joint controls. Review probes leave engine/Rust/WGSL unchanged. Use current viewer pool until new instrumentation needed, then replace after qualification. Finish review before repairing UI03–UI18. Keep goal active, PR10unchecked. No PR11/solver/coldcampaign or commit/push authorization.

Bodymutation buildbudgetFROZEN:body-mutation-build-budget.json,1readonlyCPP20s/5links25s/0apps/retries0/210MiBtemp8MiBretained. Consumer4tools8widgetentries(Weeble3,Explosion2,Restitution2,FarStack1loopfivepresets),actualmutations/explosion/creation/identity/geometry. No new productchange.

Bodymutation build9051CLOSED0/11.807s originalinputsunchanged; new20appbudgetFROZENbody-mutation-controls-budget.json:4CPUfixturesfirstthen4perGPUroute,120s/app25ready30scene,3screens100actions32MiBAPI,1GiBtemporary64MiBretained128MiBcache/core0/retries0. OneWeeblestepmax;BenchmarkExplosionpaused APIeffects only/no meshshadersteps. Currentpooldistanceuntilnewall5routesqualify.

Bodymutation matrix terminal; inspectbody-mutation-controls-matrix.json beforedisposition/cleanup.

Bodymutation66160CLOSED1/CPU4only,GPUunlaunched. HarnessregistryretainedwarmbodyIDsacrossworldreplacement; actualIDtriplesreuseworldslot, duplicatesinvalidatescounts/aggregates. InitialJSsinreferencealsowrong(nativeb3ComputeCosSinapproximation); screenshotSpherefilenamecollidedbeforeRestitutionrestart. Originalvalues/receipts preserved.165098464approxunqualifiedbinariesdeleted(exactreceiptbody-mutation-stage1-binary-cleanup.json). NewlifetimebuildbudgetFROZEN1CPP20/5links25/0apps/retries0; clearregistryonactualWorldDestroy, nativequatreference. Reusepassedmutation/lifetime/mouseeffects;newCPUbaseline/restartonlythenunlaunchedGPUcases. No confirmednewproductissue.

Lifetimebuild86024CLOSED0/PASS. DistincttailbudgetFROZENbody-mutation-controls-tail-budget.json17apps:CPU1baseline+unfinishedRestitutionrestartonly,4GPU×4fixtures,120sapp/25ready/30scene/3screens100actions/core0/0retries; samestoragecaps. OriginalCPUpositiveeffects kept, duplicatecounts resolvedoffline viauniqueID/lastcreation records, nativequatcomparedtooriginalactualsetter. No passedactionsreplayed.

Bodymutation matrix terminal; inspectbody-mutation-controls-tail-matrix.json beforedisposition/cleanup.
