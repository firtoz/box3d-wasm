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
| Sample widgets | **342 reviewed / 0 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **165 reviewed / 0 left** | sample-applicability.md/json |
| Custom input sample groups | **23 reviewed / 0 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **24 open; 5 fixes verified locally** | issues.json; UI01–UI29 reports |
| Final PR10 acceptance | Open | Repairs, resource/compatibility checks and final criterion audit |

The 165 sample dispositions comprise 71 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 165 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping, UI02 wheel routing, UI08 Unix Replay basename, UI09 honest Replay availability and UI19 Wheel steering damping are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size; UI29 Mesh Drop auto regeneration hides controls behind repeated GPU preparation and fails to switch off. See individual reports and `issues.json`. The full review is complete; verified repair evidence follows.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain open for repair.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

The full review is complete: 342 active sample widgets, 165 sample dispositions, 23 custom input groups and 85 shared calls; 0 left in each denominator. The 165 comprise 71 actual specific-control fixtures and 94 explicit shared-code reuses, with defects and unavailable features preserved. PR10 is unchecked: **24 issues open, 5 fixes verified locally**. The app goal remains active.

UI19 is now [x]: see ui19-acceptance.json and UI19-repair.md. The native Wheel callback used the suspension setter and suspension value for the steering slider. Generated project code now uses the steering setter/value. All five routes have actual setter-input/getter, independent damping, wake, joint/body identity, paused pose and default-reset checks. Driving front/rear damping and its auxiliary Parallel joint remain correct. There are 267 recorded checks and 20 new captures plus 5 before images; both CPU-first grids and all captures were inspected, with the five repaired-control images also inspected at full resolution. Other original before-widget settings differ; numeric traces prove the repair. No advancing physics steps or solver qualification are claimed.

Passed cases were preserved: CPU Wheel six transitions/reset come from one partial run, CPU Driving from a separate completed run. Ordinary combined's first four Wheel transitions were retained after the storage guard stopped it; a distinct follow-up established .4/.3, checked .7 restores/reset/Driving. Other three GPU routes ran the full case once. Original missing-checkbox timeout, Parallel-filter miscount, source-include assertion and missing Replay-object link failure are retained and adjudicated in the linked reports/history. None is passed off as a product defect. The project source delta is one Wheel setter/value replacement; an existing declarations-only split-step include was explicitly accounted for.

Current build is **ui19-build.json**. Use its **link_plan** for subsequent relinks: it contains the five correct commands and all six active Replay/Joint object paths. Keep those objects (small required inputs); earlier cleanup deleted active Replay dependencies and required three small recompiles. The new manifest pins these objects, generated sources and unchanged original207 shared inputs plus3 flag files. Do not reuse commands pointing at retired ui09 Replay objects. Box3D/WASM/Rust/WGSL/shared archives and existing local work remain unchanged.

All jobs are terminal: dependency build44418, original CPU setup93215, corrected CPU75909, completion48250, remaining81811 and closeout86150. Owned app/driver PIDs were checked absent, cores disabled, streams consumed; resource receipts are linked from ui19-acceptance.json. Storage guard stayed at1GiB. Retired first3 old viewers freed86303496bytes; final2 old viewers/generated unused main freed80131340bytes. Duplicate frame/draw snapshots in receipts were consumed, retaining selected states, all damping/wake events, API counts and original hashes; this freed36868054bytes. Current raw size890098653bytes; free1708133658624bytes. See ui19-final-cleanup.json. No live job or open execution budget. Filter diagnostic events before writing future receipts.

Earlier UI01/UI02/UI08/UI09 verified fixes remain valid. GPU Replay remains explicitly unsupported; its honest Close/status UI is a usability repair, not a player implementation. Keep raw/replay-rich-corrected/fixture.b3rec (76546bytes/64frames) and the old28KB recording fixture for final compatibility checks. Preserve selected before/after media and minimal evidence. Never stage raw/. Old mutation/replay-observer/UI08/UI09 pools are retired; their manifests are historical.

## Next actions

Continue one demonstrated cause at a time. Next is UI20: Driving Max Torque updates only a variable and never applies the changed value to its rear wheel motors. Read UI20.md, its actual input/getter evidence and native source first. Patch the generated Joint copy, preserve defaults and front-wheel behavior, and verify all five routes with CPU-first before/after evidence. Freeze the finite budget first; use ui19-build.json/link_plan and retain the active objects. Do not repeat completed UI19 checks wholesale.

Then repair the other open UI03–UI29 causes and triage OBS01 (combined BodyCast pale/white, actual RGBA/selection not retained) and OBS02 (GTK path submission probe/environment unconfirmed). No unchanged GTK/cold/general-mesh/core/solver retries. PR11 queries, PR02 category gap and Rain PR04 failures remain separate. Final PR10 criterion audit, native/WASM compatibility and applicable pre-commit affected-scene recordings remain open. No PR10 commit/push authorization. Continue the active goal until PR10 is complete, then stop.

PreviousgoalturnPROGRESS: UI19fixed/verified and resourcescleaned. UI20FROZEN:1generation10s/3CPP30s/5links25s/5apps75s/15captures/0physicsenginebuildretries/750souter/1GiBwholeRaw192MiBnewbuild64MiBretained. Build/check/consume/retireone routeatatime; existinglink_plan/Replayobjectsverifiedpresent. Beforeactualall5UI25→rearcap5/no setter. Actual25/0/5+W/S/IDs/defaultreset+Wheelrelated; filterreceipttracebeforewriting.342widgets0left165samples0left23custom0left24open5verified.

UI20batch9933LAUNCHED: interleaved compile/link/check/consume/retire perroute, existingReplayobjects reused/currentcandidateui20-build.json. Pollsamehandle; no launchesbasedonobservationtimeout.24open5verifiedpendingactualevidence.

UI20batch9933 CLOSED1afterCPUcompile/link: actual25cap/W/S/unchangedfront+IDs/poses PASS; mouse-checkboxoffprobe failedbecauseToggleThirdPersonlocks pointer. Tshortcutnative/sourcepreviouslychecked. Originalreceipt/2captures/13positivechecks retained, streamsconsumed/PIDsabsent. DistinctcompletionFROZEN2GPUCPP30s/4links25s/5apps75s/15captures/0regenCPUbuildphysicsretries/650souter: CPUtailTunlock+0/5/reset/Wheelrelated,4GPUfirstfullchecks. ReuseCPU25/W/Spasses, no rerun.24open5verified.

UI20completionbatch launched; use exact tool handlefromlaunch and ui20-build.json.current. No regeneration/CPUcompilelink; routes interleaved/oldbinsretiredonlyafterpass.24open5verified.

UI20completionbatchhandle49594; pollsamehandle/inspectcurrentmanifest ifinterrupted.24open5verifiedpendingcaptures.
