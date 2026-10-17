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
| Sample widgets | **97 reviewed / 245 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **117 reviewed / 48 left** | sample-applicability.md/json |
| Custom input sample groups | **10 reviewed / 13 left** | manifold-controls-acceptance.json; 27 overridden mouse methods exercised |
| Repairs | **17 open; 2 earlier fixes verified locally** | issues.json; UI01–UI19 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 117 sample dispositions comprise 23 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 117 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest body mutation review is CLOSED: body-mutation-controls.md, body-mutation-controls-acceptance.json and four CPU-first image grids (45 cells inspected). Eight widget entries across four samples were reviewed on all five routes: Weeble teleport/explosion/magnitude, Benchmark Explosion magnitude/button, Restitution shape radios, and all five Far Stack presets (one loop call site). No new product finding or repair. There were 414 recorded evaluations, including 12 invalid harness predicates explicitly adjudicated. All 16 GPU fixtures pass.

The original observer duplicated warm-world body IDs after world replacement. Complete small states were recovered using unique IDs and latest creation definitions. One CPU recovery checked corrected baseline counts, the native quaternion reference and the unfinished Restitution restart; no passed action was replayed. JS Math.sin was an invalid exact reference for native b3ComputeCosSin. The actual native expression matches the original setter at the unchanged 1e-5 limit. The duplicate Sphere screenshot filename failure is retained. Far Stack lifetime uses the world generation token; body triples may reuse world slots. Original CPU Explosion aggregate counts/sums are invalid quantitatively because they double-counted warm IDs. Individual velocities, zero/nonzero effects, directions and finite outputs remain valid. Benchmark Explosion stayed paused; no mesh shader/solver campaign resumed. Weeble moved on one actual step. No trajectory equivalence claim.

Prior collision review is CLOSED: distance-controls.md, distance-controls-acceptance.json, its CPU-first grid, and empty-world-framing-acceptance.json. Eleven widgets across three tools plus Shape Distance mouse gestures were reviewed on all five routes. Its 388 check evaluations include 40 policy failures and nine deliberate negative faults (five TOI SIGSEGV, four GPU-viewer cache SIGILL); CPU invalid cache input survives. UI15–UI18 remain confirmed/unfixed. Five false pan assertions were rejected because the harness omitted Alt; initial/F/menu framing independently leaves the preview outside. Other no-body tools have source-applicable focus paths but runtime framing remains unreviewed. These tools use native C math, so this is not GPU solver qualification.

All jobs are terminal. No next runtime budget is active. Body build9051 and lifetime build86024 closed successfully; original CPU batch66160 retained its failure; tail56300 and closeout16887 closed successfully. All 42 owned app/driver PIDs are absent, with no matching core files. Shared cache growth was zero. All 207 original source/engine/gfx/main/Sample input hashes are unchanged. body-mutation-controls-cleanup.json records 209,194,983 bytes deleted (consumed streams and superseded distance viewers); an earlier cleanup removed 164,805,776 bytes of unqualified body viewers. The preceding collision cleanup removed 188,991,776 bytes. Latest raw total:427,299,771 bytes. Original manifests, sources and receipts remain. A small TypeScript parse error before launching tail apps was corrected without consuming an app run.

Current qualified pool: **body-mutation-lifetime-build.json**, five viewers under raw/body-mutation-lifetime-build/. Pass this buildFile explicitly to Driver; its default geometry pool was deleted. Retain the current pool for remaining controls until a replacement is qualified, then delete it. Existing objects under raw/ui01-after, raw/ui02-after and other observer directories remain actual link inputs. Keep raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec for remaining Replay controls. Selected screenshots, receipts and acceptance are PR10 audit evidence. Raw is locally ignored. User work and shared caches are preserved.

Previous Disable, geometry, Manifold and shared reports remain valid. source-callsite-audit.json accounts for every active call plus five excluded, disabled Ragdoll/Pose widgets. Closed histories: history-body-mutation-2026-10-15.md, history-distance-2026-10-15.md, history-manifold-2026-10-15.md, history-controls-2026-10-15.md and history-2026-10-15.md. Do not resume closed or exhausted experiments.

## Next actions

Continue the **245 remaining widget entries and 13 custom input groups across 48 samples**, using TypeScript/Bun and a NEW finite budget before execution. Read source and derive intended effects first. The current observer handles body/shape/explosion mutations and can be extended read-only for joint setters or query controls. Useful remaining groups include joints, creation/replacement controls, collision preview handlers, character/custom input, and Replay/Viewer (36 entries). Shared implementations may reuse checked cases only with explicit per-sample applicability. Registration does not prove effects.

Finish the review before repairing UI03–UI18. Keep the goal active and PR10 unchecked; stop after PR10. No PR11, unrelated solver, cold-start campaign, commit or push authorization is active.

Joint observer build budget FROZEN: joint-control-build-budget.json, 1 readonly C++ compile25s/5links25s,0apps/retries0,220MiB temporary/8MiB retained. Consumer7jointtools64entries; actual joint setter/getter/identity/awake state. Wheel steering-damping wrong setter is a source candidate, not yet runtime-confirmed. Previous turn classified PROGRESS (19widgets+7samples, evidence/cleanup).

Joint build80617 CLOSED0/14.340s, original207inputs unchanged. Runtime budget FROZEN joint-controls-budget.json:35independentapps (CPU7first),180s/app25ready30scene,2screens200actions/1configuredstep/core0/retries0,1GiBtemporary64MiBretained128MiBcachegrowth. Sevenjointtools64entries; actual conditional fields/components, parameters/awake/IDs/recreation/restart; no solveraccuracy claim. Currentbodypool retained until replacement qualified.

Joint matrix5371 LIVE at last observation: CPU7 fixtures completed; GPU routes in progress, exact ownedchild in joint-controls-matrix.json. UI19 Wheel steering-damping wrong setter confirmed on CPU; GPU pending.17open/2verified; accepted97widgets245left/117samples48left unchanged until allroutes close. Future unlaunched fixtures use quiet console and bounded30s direct single-step observation; initial driver archived in joint-controls-initial.ts. No new runtime budget/retries or productchange.

Joint matrix terminal; inspect joint-controls-matrix.json before disposition/cleanup.
