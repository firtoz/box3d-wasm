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
| Sample widgets | **297 reviewed / 45 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **160 reviewed / 5 left** | sample-applicability.md/json |
| Custom input sample groups | **22 reviewed / 1 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **26 open; 2 earlier fixes verified locally** | issues.json; UI01–UI28 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 160 sample dispositions comprise 66 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 160 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest cast-controls-acceptance.json:14widgetreviewdispositions/5samples/3mousegroups all5routes. CastWorld actual4types/4callbackmodes/radius/contextflags/5spawntypes/destroy/ring64/CtrlMouse/restart; LongRay inputlength/nativecone; ShapeCastflag/ShiftZ/ControlY/release; OverlapWorldShiftZ/release. All79capturedimages inspected: fiveCPU-first15cellgrids and4additionaltailimages. UI28 confirmedall5: Boxusesm_castRadius foractual8pointproxy[±2,±1,±.5] afterRadius2, buthides theactivecontrol. No productfix.

Twoqueryoutcome gaps areexplicitPR11releasefindings, notGPUfeaturepasses: InitialOverlapzero-translationcastflagreachescontexttrue, CPUaccepted1/all4GPU0; OverlapWorldShiftZlast8pointproxy identicalallroutes, CPUcallback1/returnsfalse(stop)/red, all4GPUcallback0/green. Inputs/bodydigests/source/scenes in cast-pr11-query-findings.json andtwo reports. CausesUNTRACED; noquerycalculationcampaign/repair inPR10. Widget/proxyinput effects observed separately.

Source-derived CastContext copiedwithmemcpy onlyforfourcallbacks whitelistedfromcurrentexecutable symbols, PIE-normalized; othercontextsneverread. Callbackoriginal/context/returnforwardedonce. NativeCPUlong-rayreference ratios used because b3MakeQuatFromAxisAngle usesintentionallyapproximate deterministicb3ComputeCosSin (math_functions.c216), notJavaScriptMath.cos. Fiveidealcosinepredicates perroute andthree repeated-cursororigin-changepredicates perrouteareharnesserrors, originalvaluesretained/no replay. Positiveactualcallbackpoliciesall4modes passall5.

Build13285 CLOSED0/15.521s,5nmreadsPASS. Main35349 CLOSED1/25apps:23complete,2combinedCastWorldstopat16MiBAPIcap aftertypes/radius/4modes. Tail76445 CLOSED0/2apps checksONLYpreviouslyunreachedflags/negativeMouse/destroy/ring64/restart+missingsortedcapture; nofullcasereplay. Closeout68140 CLOSED1 after5montages atretainedbudget check(71438098>64MiB). Losslesswhitespace-onlycompaction5CastWorldJSONreceipts saves27005424bytes/preservesallvalues/failures; original/compacthashes+structuralequality in cast-receipt-compaction.json. Resume94885 CLOSED0 withzeronewcompositions/apps/builds.

All54viewer/driverPIDs plus5nmPIDsabsent/0cores/cachegrowth0. Cleanup deletes295569212stream/binarybytes plus27005424whitespacebytes, retained original44432674/tail9580356; raw795402497, free1708281446400. Oldplane5viewers/object removed166936888bytes. Currentqualifiedpool **cast-controls-build.json**, raw/cast-controls-build;207originalinputs unchanged. PassbuildFileexplicitly; oldplane/scene/host/geometrypoolsdeleted. Currentobserverhasquerycontext/overlap/cameraprojections pluspriorplane/force/bodyhelpers; retainforremainingcontrols. NoRust/WGSL/Box3D/WASMchanges.

Prior plane-controls-acceptance.json oneSolve/3mousegroups/3samples/45images; character4widgets/2Keyboardgroups/30images; query-input7widgets/2samples/30images; scene11widgets/2samples/37images; host13widgets/3samples+VillageT/60images remainvalid withknownfindings/limits. Linkedhistories preservefailureadjudications andclosedresources. OBS01 combinedBodyCastpale/whiteappearance remainsUNCONFIRMED: retainedinputs showmissesall5, so nothit-vs-missdifference; combinedworlddragtransform differswithviewport, projection/occlusion isonlyhypothesis. ActualDrawHullcolor/selectionnotretained; triagebeforefinalacceptance.

Keep earlier UI24 flat HeightField scaleY0 floor loss, UI25 HeightField.75→.7 and combined Benchmark post-ray100s performance exclusions open. Read geometry-controls-acceptance.json and linked histories. Do NOT resume Capsule cold startup, Benchmark post-ray, core extraction, solver or PR11 campaigns. UI12 protected~3MB old core could not be deleted without sudo;204MBpartial already deleted. Owned viewers use core0. Original207pins and CPUReplayfixture raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec retained for remaining Replay. Preserve shared caches/user work; raw ignored.

## Next actions

Continue **45widgetentries/1custominputgroup across5samples**: FallingTrees3, MeshDrop5, GhostCollisions3, ReplayViewer29remainingconditionalwidgets+handlers/focus/layout, Wind5. ThenrepairdemonstratedPR10causesoneatatime. Current26UIissuesopen/2earlierverified, OBS01unconfirmed, twoadditionalPR11queryfindingsoutsideUIrepairscope.

Nextsource-firstmutation/forcegroup. MeshDropautoGenerate whilepausedregenerateswheneverbodymoveCount0; observingon/off canverifyactualgenerationwithoutworldsteps, but1024bodydestroylogsperUIframewouldwasteoutput. Addread-onlysummary-mode mutationobserver ifneeded, preservingrealcreate/destroy calls/counts/IDs, beforethatbatch. Needactualshape-filterinput/getter forCollide. FallingTreesgeneratedscene variants needsourcecounts/bounds. WindApplyWind runsafterrealworldstep; budgetactualboundedsteps/time ifneeded, no skippedphysics/fakeinput/coldcampaign. GhostResetCounters needsnonzeroactualstate/clearproof; choosefiniteactualinteraction, donotdeclarezero→zerofullfeaturepass. ReplayneedsCPUfixturewithjoints/queries/spawn/delete toreach29conditionalcontrols; GPUplayerunsupportedexplicitly, earlierbasictransport7entriesalreadyreviewed. No budgetfrozen orlivejob. KeepOBS01forfinaltriage. Finishreviewbeforerepair. GoalACTIVE/PR10unchecked; noPR10commit/pushauthorization.

Continuation audit: PROGRESS. Derived297widgets/45left,160sampledispositions/5left=66actualspecificfixtures+94sharedreuses,22custom/1left.26UIissuesconfirmedopen/2earlierverified;2PR11queryfindings;OBS01unconfirmed. git diff --check PASS, feat/gpu, box3d submoduleclean, noownedcast viewers/drivers. Cleanup295569212stream/binarybytes+27005424losslesswhitespacebytes; retainednewruns54013030bytes/raw795402497bytes;59PIDsabsent/cachegrowth0/free1708281446400bytes. Allbatchbudgetsclosed; currentcastpoolnamedremainingconsumer. No newbudget/livejob/commit/push. Goal remainsACTIVE/PR10unchecked.

Mutation/force observer build FROZEN: one C++ compile30s/five links25s,230MiB/log1MiB,0 engine builds/apps/retries. Summary-only observation forwards every original call once; filters and counts retained for all bodies,8 examples; destroy records retired immediately. No product fix. Counts297/45widgets,160/5samples,22/1custom,26open/2verified.

Mutation/force build21924 CLOSED0/15.785s all5links. Actual control batch FROZEN15apps/90s each/20s transition/3screens/100actions/12MiB API/4MiB observer;4 real Wind steps/app and0 MeshDrop/Trees steps,0retries/engine builds/1500s outer,1GiBraw/64MiBretained/cachegrowth128MiB. Current297/45widgets,160/5samples,22/1custom,26open/2verified. Driver mutation-force-controls.ts launches once after freeze.

Mutation/force matrix terminal; inspect receipts and images before dispositions.

Main mutation/force64015 CLOSED0/15terminalapps. Harness enum/default-filter/AABB assumptions require source/data adjudication, no replay. UI29 confirmed50msAutooffclick lostall4GPU(originalButtonactivatesoncevsCPUtwice), actualcreationcontinues; causeUNTRACED/possibleUI25relation.27open/2verified. Supersededcastpooldeleted 167031008bytes/currentmutation-force-build.json qualifiedfive routes;207originalpinsunchanged. Counts297/45widgets,160/5samples,22/1custom pendingvisualcloseout.

MeshDrop off-only duration variant FROZEN: 4 previously failedGPUroutes/45s each/1screen/30actions/0steps/builds/retries,12MiBAPI/16MiBretained. Distinctactualmouse-down heldacross2UIframes; original50msclickfailures preserved. Not a productfix.

Mutation/force closeout terminal:13widgetdispositions/3samples pending49imageinspection/inventorywrite. All38app/driverPIDsabsent/0cores/cachegrowth0. Mainretained60602936/tail1645569,streamsdeleted67756711; castpool167031008deleted. 207originalpinsunchanged. UI29causeuntraced,helddurationfailsall4GPU,CPUoriginaloffpasses. Counts297/45widgets,160/5samples,22/1custom,27open/2verified.
