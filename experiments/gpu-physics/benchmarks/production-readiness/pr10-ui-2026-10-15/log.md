# PR10 UI review — recovery log

Read this FIRST on every continuation and after compaction/restart, then `AGENTS.md` and `docs/goals/gpu-production-readiness.md`. Apply the goal-scratchpad skill. This is the current state; dated histories contain closed experiments, not next actions.

## Objective and authorization

Finish **PR10 UI/widgets end to end** in `/home/firtoz/work/2026/box3d-wasm` on `feat/gpu`. The user activated this goal on 2026-10-15. Applicable PR10 acceptance completed on 2026-10-16; stop here. No later roadmap item is activated. No token budget was requested. Each execution batch still requires finite resource limits.

Review applicable controls for all 165 native samples: 342 per-sample widget entries, 85 shared calls and 23 samples with custom input handlers. Compare real Box3D CPU first, ordinary/native GPU and both combined viewers. Exercise actual mouse/keyboard input, displayed state, intended geometry/physics/render effects, identity and lifecycle. Shared implementation may reuse exercised cases only with explicit per-sample applicability. Source registration, a screenshot or input acceptance alone does not prove an effect. Unsupported features require honest UI and explicit dispositions. Preserve previous passes and original failures; do not repeat PR09's completed default visual sweep.

Finish the remaining review **before fixing newly recorded issues**. Then trace the smallest failing operation and fix one cause and its affected cases at a time. Verify applicable routes, retain applicable verification evidence and existing before/after screenshots, mark [x] only after verification, and share screenshots with each repair report. Report controls/samples done and left and issue counts in every progress update. Finish with a criterion audit; PR10 stays unchecked until its actual criteria pass.

Use **TypeScript/Bun** for new automation. Freeze build/check/capture/process/runtime/storage budgets before each batch. Enforce reasonable timeouts and output caps, disable owned viewer core dumps, and reap owned child processes. Default limits: 1 GiB temporary / 64 MiB retained diagnostic evidence per batch; budget exceptions beforehand. Measure and preserve shared caches. Delete consumed task-owned binaries/traces/copies after extracting minimal evidence, including failed runs. Keep a named consumer for retained large inputs. Never stage `raw/`; it is locally excluded.

Keep Box3D/WASM unchanged and preserve geometry, physics defaults, body identity, picking and other local work. Review probes also leave engine/Rust/WGSL inputs unchanged. No unrelated solver repairs, PR11 query qualification, cold-start campaigns or exhausted experiments. Exact pixels and chaotic CPU/GPU trajectory matching are outside scope. Integrate remote changes safely if needed. User authorized PR10 commit and push on 2026-10-16; recordings/comparison grids are optional and are not commit prerequisites (user direction2026-10-17). PR09 is already pushed as `51ff8b06b4964d359386f08605d835ac6e2ea23d`.

## Final state — complete

PR10 is verified: native inventory165/0left, shared controls85/0, active samplewidgets342/0, sampledispositions165/0, custominputgroups23/0, repairs29verified/0confirmedopen. The dispositions comprise71actualspecific-controlfixtures and94explicitshared-code reuses; five inactive Ragdoll/Pose widgets are excluded. [Reports](README.md), [acceptance](final-acceptance.json), [portable media](portable-media.json).

Five fresh native builds/startup checks, full workspace/scripts typechecking and demo build passed. Unchanged-source WASM built with repository-pinned Emscripten6.0.2; official SingleBox frames0,1,2 dump passed. The temporary SDK/generatedWASM/source/build were removed and original absent-artifact state restored. No Box3D/WASM/Rust/WGSL source changes. All original failures and limits remain in the reports.

OBS01 BodyCast appearance remains unconfirmed underPR11. OBS02 nativeGTKacceptance reuses actual retained evidence; later helper timeouts are not product fixes. OBS03 CPU TOI comboEscape follows native disabled keyboard-navigation policy; original event remains untraced. Rain physical failures and remaining releasegates stay open.

## Publication and policy —2026-10-17

The user authorized commit/push, then explicitly removed recording/comparison prerequisites: do not generate fullcatalog media merely to commit. AGENTS.md, the roadmap, native README and related goal guidance now reflect this. Current direction supersedes the historical mandatory capture wording. Existing repair screenshots and checks remain useful evidence. No additional capture/inspection gate applies.

An overbroad background publication matrix had already generated817shortclips plus8preexistingunavailable dispositions. The user said these were unnecessary; offline inspection was stopped and all new publication clips/sheets/orchestrators/scratch were deleted. No visual acceptance is claimed from them. publication-cleanup.json records143585624bytes deleted,2465ownedPIDsabsent and0remainingjobs. Shared caches/engine archives/userwork are preserved.

Five additional delivery-source native builds passed; commands, source/library hashes and compiled-unit identities are retained in publication-checks.json. Current runnable viewer pools/buildtrees are retired. ui03-build.json is historical RETIRED_AFTER_FINAL_VERIFIED_ACCEPTANCE; do not rerun its old protocols.

Selected original screenshot/replay evidence941files/261882755logicalbytes was made portable outside ignored raw/ using exact hardlinks; portable-media.json maps historical media paths. Never stage raw/. No current process/job needs polling. This publication is scoped to PR10 and revised guidance on feat/gpu. Git records its commit identity; check HEAD and origin/feat/gpu on recovery. The local post-push receipt is at the Git path pr10-publication.json (fresh clones use Git history/remote). This milestone must stop after PR10.

## Next action

After publication, PR11 query behavior/appearance is the next recommendation; it is not activated. Read AGENTS.md and docs/goals/gpu-production-readiness.md before substantive work. Use existing evidence and targeted checks. Do not resume old coldstart/GTK/Rain/solver/scheduling campaigns. This is not full production-readiness certification. Detailed earlier continuation: history-final-completion-2026-10-16.md.
