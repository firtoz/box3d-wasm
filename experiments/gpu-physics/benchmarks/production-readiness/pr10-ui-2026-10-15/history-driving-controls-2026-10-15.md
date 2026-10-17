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
| Sample widgets | **161 reviewed / 181 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **124 reviewed / 41 left** | sample-applicability.md/json |
| Custom input sample groups | **10 reviewed / 13 left** | manifold-controls-acceptance.json; 27 overridden mouse methods exercised |
| Repairs | **18 open; 2 earlier fixes verified locally** | issues.json; UI01–UI20 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 124 sample dispositions comprise 30 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 124 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest joint review is CLOSED: joint-controls.md and joint-controls-acceptance.json. Seven tools /64widgetentries on allfive routes: Distance10, Prismatic10, Revolute10, Spherical12, Parallel Spring2, Weld4, Wheel16.2355evaluations:2345true and10policyfailures grouped into UI19. Every conditional field and Spherical vector component was exercised. Actual setters/getters, wake paths and retained joint/body IDs were checked. Distance Count retires old IDs and recreates three dynamic bodies/joints with the edited policy. One configured step checks finite output and response where driven. All samples restart to original actual settings/counts. This is control qualification, not solver accuracy/constraint or trajectory qualification. All70selected CPU-first image cells were inspected; geometry/floors/material conventions preserved.

UI19 is confirmed on allfive routes: Wheel steering damping UI1.4 calls the suspension setter with1.2; actual steering damping remains about.7. It is unfixed. Driving uses the correct source steering-damping call but its runtime controls remain unreviewed. Source preflight also finds Driving Max Torque stored only in a field/constructor, with no runtime setter; this is a candidate requiring actual input before adding an issue. Parallel Spring source default10Hz versus drag range0..5 is an observation, not a confirmed missing-effect defect; original defaults and text-entry behavior are preserved.

All joint jobs are terminal: build80617 closed0/14.340s, matrix5371 closed0/35apps, closeout13436 closed0. All70owned app/driver PIDs absent, no matching cores; sharedcachegrowth0. Original207engine/gfx/main/Sample inputs and original sample_joint.cpp hash unchanged. joint-controls-cleanup.json deleted263768751bytes of consumed streams and superseded body viewers. Latest raw total470935586bytes. Initial driver source is retained in joint-controls-initial.ts; future fixtures only quieted console passes and observed the same one step with the already-frozen30s bound. No failed/repeated joint runtime or product repair.

Current qualified pool: **joint-control-build.json**, five viewers under raw/joint-control-build/. Pass this buildFile explicitly; Driver default points at a deleted older geometry pool. Retain until a replacement qualifies, then delete. Existing raw/ui01-after, raw/ui02-after and other observer objects are actual link inputs. Keep raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec for remaining Replay controls. Selected screenshots, receipts and acceptance are PR10 audit evidence. Raw is locally ignored; user work and shared caches preserved.

Earlier body mutation, collision, Manifold, geometry, Disable and shared evidence remains valid with its known defects/limitations. Body mutation acceptance records414evaluations and12adjudicated harness predicates, with original CPU duplicates/JSsin/screenshot failures retained. Collision review records388evaluations,40policyfailures and9deliberate negativefaults (TOI5SIGSEGV/cache4GPUSIGILL); CPUinvalidcache survives. Five framing-pan assertions omittedAlt and were rejected; F/menu failure independently confirms UI18. Those no-body tools use native C math, not GPU solver execution. Closed recovery histories: history-joint-controls-2026-10-15.md, history-body-mutation-2026-10-15.md, history-distance-2026-10-15.md and older linked histories. Do not resume exhausted runs/core extraction.

## Next actions

Continue **181remaining widgetentries and13custominputgroups across41samples**. Use TypeScript/Bun and freeze a NEW finite budget before execution. A useful next batch uses the current joint/body observer for Driving12widgets/Keyboard group (including source-correct steering damping versus UI19 and unused-torque candidate), plus Top Down Friction Explode1. Read all input/camera source before that batch. Driving invokes wheel setters every render frame even paused, so bound API output and consume per-app full streams after extracting receipts. No new budget is active yet. Other remaining groups include character/custom input, creation/replacement, collision previews, remaining joints and Replay/Viewer36entries.

Finish all review before repairing UI03–UI19. Keep goal active, PR10unchecked and stop afterPR10. No PR11, unrelated solver, cold-start campaign, commit or push authorization is active.

Driving budget FROZEN driving-controls-budget.json:0builds/10apps CPU2first,150sapp25ready30scene,3screens130actions/0physicssteps/core0/retries0,96MiBAPI perapp (source emits setters every render evenpaused),1GiBtemporary64MiBretained128MiBcache. Parent consumes/deletes per-app streams after terminal receipt/PID validation; selected states preserved, failed tails bounded. Driving12widgets/KeyboardT+WASD andTopDownExplode1; no solvercampaign.

Driving matrix terminal; inspect driving-controls-matrix.json before disposition/cleanup.

Driving original54621CLOSED1: CPUDriving12widgets/Keyboard/T/WASD complete withUI20 MaxTorqueno setter; CPUsteeringdamping passes(relatedUI19 unaffected). CPU TopDown hostselectioncount passed butstate waiter wrongly required disabledjointtrace; no button effect tested. Both streams consumed withcompactfailuretail, ownedPIDsabsent. AllGPUfixturesunlaunched. DistincttailbudgetFROZENdriving-controls-tail-budget.json9apps:unfinishedCPUtopdownthen4GPU×2,same150s/3screens/0steps/core0/storagecaps. state waiter now requiresjointframesonlyDriving. PreservepassedCPUDriving, no replay.18open/2verified; accepted161widgets181left/124samples41left unchanged.

Driving matrix terminal; inspect driving-controls-tail-matrix.json before disposition/cleanup.
