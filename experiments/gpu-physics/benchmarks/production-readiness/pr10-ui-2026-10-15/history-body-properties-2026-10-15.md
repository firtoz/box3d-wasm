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
| Sample widgets | **174 reviewed / 168 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **126 reviewed / 39 left** | sample-applicability.md/json |
| Custom input sample groups | **11 reviewed / 12 left** | Manifold, Shape Distance and Driving acceptance receipts |
| Repairs | **20 open; 2 earlier fixes verified locally** | issues.json; UI01–UI22 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 126 sample dispositions comprise 32 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 126 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest Driving/Top Down review is CLOSED: driving-controls.md and driving-controls-acceptance.json.13widgetentries/two samples plus Driving Keyboard group onallfive routes;381evaluations,371true (including the original host-only selection check),10UI20policyfailures. All25selected CPU-first image cells inspected; vehicle/terrain/room/grid preserved. Driving suspension updates allfour wheels and steering updates onlyfront wheels. W/S produce rear motor targets -40/+40 at Speed40, A/D produce front steering targets±pi/4, releases neutralize commands. Third-person checkbox/T press-release, camera-follow pivot, mode-specific picking exclusion and restart defaults checked. TopDown explosion sends source inputs, changes actual outward velocities, preservesIDs/pausedposes, staysfinite and restarts atrest. Zero new physicssteps: command/immediateimpulse qualification, not solver/trajectory qualification.

UI20 is confirmed onallfive routes: Driving MaxTorque25 leaves allwheel actual limits5, including while keys command motors, with zero runtime torque settercalls. UI19 remains specific to Wheel: related Driving steering damping correctly changesfront values to1.4 while rear values stay.7 onallfive routes. Both remain unfixed pending fullreview. Original CPUDriving12widgets/Keyboard checks were reused; original CPU TopDown waiter required disabled jointtracing and failed before any button effect. Only unfinished CPU TopDown and the eight unlaunchedGPU fixtures ran with corrected conditional waiter. Original failedreceipt and boundedtail retained.

Latest joint review remains CLOSED: joint-controls.md/acceptance, seven tools64entries/allfive routes,2355evaluations including10UI19policyfailures. Actual getters/setters/awake/ID/count recreation/restart and one configuredstep qualified UIeffects, not solveraccuracy. All70CPU-first imagecells inspected. Parallel Spring default10Hz versus normal drag range0..5 is an observation, not a confirmed missingeffect defect; defaults/textentry preserved.

All jobs terminal, no new runtimebudget. Joint build80617 closed0/14.340s; matrix5371 closed0/35apps; closeout13436 closed0. Driving original54621 closed1/CPU2only; correctedtail53792 closed0/9apps; closeout14342 closed0. All70joint-owned plus22Driving-owned app/driver PIDs absent; no matchingcores, sharedcachegrowth0. Original207engine/gfx/main/Sample inputs and sample_joint.cpp unchanged. Jointcleanup deleted263768751bytes; Driving streams consumed per-app anddeleted86330117bytes, preventing accumulation. Latest raw total486692130bytes; namedconsumer/evidence in cleanupreceipts.

Currentqualifiedpool: **joint-control-build.json**, five viewers under raw/joint-control-build/. Pass this buildFile explicitly toDriver; itsdefaultgeometrypool isdeleted. Retainuntil replacementqualifies, then delete. Existing raw/ui01-after, raw/ui02-after and other observerobjects remain actual link/preflightinputs. Keep raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec forremainingReplay controls. Selected screenshots/receipts/acceptance arePR10audit evidence. Raw islocallyignored; userwork/sharedcaches preserved.

Earlier bodymutation/collision/Manifold/geometry/Disable/shared reports remainvalid withknown defects andlimits. Collision tools use native C math, not GPU solverexecution; deliberatefaults arefindings, never featurepasses. Closed histories: history-driving-controls-2026-10-15.md, history-joint-controls-2026-10-15.md, history-body-mutation-2026-10-15.md, history-distance-2026-10-15.md andolderlinked histories. Do not resume closed/exhausted runs or coreextraction.

## Next actions

Continue **168remaining widgetentries and12custominputgroups across39samples**, usingTypeScript/Bun andNEW finite budgets beforeexecution. Readsource/deriveeffects first. Currentreadonly observer covers body creation/transform/velocities/explosion andseven jointtypes, with128jointref cap and64bodyexamples pluscomplete aggregates. APIlatest() reads64KiB tail; do not selectlargescenes whose single diagnosticrow exceedsit without a bounded-reader change. GearLift/Bridge may exceedjointref cap, so source/preflightbeforeenablingjointtrace. Driving per-frame setterstream was consumed per-app; reuse that cleanup strategy for chat/disk health.

Useful remaininggroups: collision no-body InitialOverlap/ShapeCast/MeshScale andcustomhandlers; bodylaunch/spawn/replace controls; remainingjoint/bodyproperties (Bridgegravity,MotionLocks,Doorconstrainttuning/Motor/GearLift) may needread-only observerextension; character/custominput; Meshcontrols; Replay/Viewer36entries. Review every remainingcontrol before fixingUI03–UI20. Keepgoalactive, PR10unchecked, stopafterPR10. No PR11/unrelatedsolver/coldcampaign/commit/push authorization.

Bodyproperties build budget FROZEN:body-properties-build-budget.json,1readonlyCPP30s/5links25s/0apps/retries0,220MiBtemporary8MiBretained. Consumer7remainingjoint/ragdollsamples28widgets plusDoorMouseDown; Motor/Filter/tuning/bodygravity/locks/impulse/targettrace, fullpropertyaggregates. Sourcecandidates:MeshRespawnleaves14kinematicanchors; DoorlimitUI membernotinitialized while jointsstartlimited. Actualconfirmationrequired, no newissue/fixyet. Previousgoalturn PROGRESS77widgets9samples/cleanup.

Bodyproperties build54561CLOSED0/14.727s. Earlier generatorpathpreflight failedbeforecompile/apps, corrected withsameunusedcompilebudget; no engineoperationrepeated. RuntimeFROZENbody-properties-controls-budget.json35apps CPU7first,180s/25ready/30scene,2screens160actions,64MiBAPI/2MiBboundedtail/core0/0retries,1GiBtemporary64MiBretained128MiBcache. Bridgejointtraceoff; othercap512. Locks<=2steps,Motor1,others0. Currentjointpooluntilall5replacementqualify. Per-app streams consumed andDriverlatest nowhonorsbudget.api_tail_bytes; priorconsumersdefault64KiBunchanged.

Bodyproperties9641 LIVE perexacthandle, CPU7andordinary7complete. UI21 Meshanchors29→43→57confirmedCPU/ordinary;Boxnoanchorscountstable. UI22DooruninitializedlimitUI andhardcodedrecreationconfirmed; ordinarycheckboxmaystaytrueafterclick, so not anunchecked-stimuluspass. Gearactual205bodies/274shapes, original273expectedpredicatewrong;sourcegroundextra supportshape auditpending, preserveoriginalreceipt/no rerun.20open/2verified; accepted174/168widgets126/39samples untilallroutesclose.

Bodyproperties matrix terminal; inspect body-properties-controls-matrix.json beforedisposition/cleanup.

Bodyproperties9641 terminal35apps; inspectmatrix. Doorcombinedleft Ctrl route ismappedCPUonly, standaloneGPU passed; missingcombinedrightGPUbranch nowseparateFROZENdoor-right-pane-budget.json2apps60s/1screen30actions/0steps/core0/retries0. Only3mousebranches onrightpane, no passedwidgets/leftactionsreplayed. Sourcehelper optionalpaneoffset archivedinitialsource. Gearcount274sourcebackwall resolvesoriginal273predicateoffline.

Door right-pane matrix terminal; inspect door-right-pane-matrix.json beforecustominput closeout.
