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
| Repairs | **26 open; 3 fixes verified locally** | issues.json; UI01–UI29 reports |
| Final PR10 acceptance | Open | Repairs, resource/compatibility checks and final criterion audit |

The 165 sample dispositions comprise 71 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 165 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping, UI02 wheel routing and UI08 Unix Replay basename are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size; UI29 Mesh Drop auto regeneration hides controls behind repeated GPU preparation and fails to switch off. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Full native control review complete: 342/342 active widget entries, 165/165 sample dispositions, 23/23 custom groups, 85/85 shared calls. The165 comprise71actual specific-control fixtures plus94explicit shared-code reuses, with defects/exclusions retained. PR10 remains unchecked;26open issues/3verified fixes. No product repair in this closing review.

Replay final review: replay-final-review-acceptance.json and replay-final-review.md. Query71394 CLOSED0/25.741s/75actions/3images; selection20018 CLOSED0/21.856s/52actions/5images. Closeout41496 completed its acceptance and cleanup outputs; process handle expired on recovery, not rerun. All8newCPUimages+4reusedGPUunsupportedimages inspected,40gridcells/12uniqueimages. Actual query index/filter/kinds/minhits/rebuild/pin/draw callbacks, body/shape/joint/contact details, FollowCOM/Zup, nativepick/gates/read-onlydrag, Focus body/shape/query/Home, resize/layout and exit restoration verified. GPU loadedplayer branches explicitly unsupportedUI09, not featurepasses. Earlier popup and availableTimelineTab all5route passes retained.

Three original false probe predicates retained/adjudicated: immediate text read did not prove stable completion; actual history/filter and narrow pre/postEnter probe preserve fullPR10-RAY. Two shape expectations incorrectly assumed creation order: actualnative linked-list slot1=firstsphere/id3, label/AABB/focus consistent, not an identity bug. No relaxed product criterion or unchanged fullcase rerun.

Latest cleanup: all4viewer/driverPIDsabsent/0cores; streams deleted 9248663 bytes; batch retained 3006303 bytes; raw total 872874706 bytes; sharedcache growth 870 bytes. All execution budgets closed/no livejob. Current mixedpool replay-observer-build.json = newCPU +4unchangedmutation-forceGPUviewers;207original inputpins verified. OldmutationCPU retired; pass mixedmanifest explicitly. Box3D/WASM/Rust/WGSL unchanged.

Required regression inputs: raw/replay-rich-corrected/fixture.b3rec (76546bytes/64frames/all7querykinds/bodyspawn-delete/joint/contacts); old28KB raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec for UI08/basicReplay. Retain current viewer pool only for UI repair regressions. Detailed prior evidence/closed budgets in history-replay-final-review-2026-10-16.md.

Watchlist: OBS01 combinedBodyCast pale/white unconfirmed (actualDrawHullRGBA/selection not retained); OBS02 GTKpathsubmission probe/environment unconfirmed. Both need final triage. Do not retry unchanged failed GTK, cold-start, solver, generalmesh or core-extraction campaigns. PR11query findings/PR02category/RainPR04 remain outside UI repair scope.

## Latest repair and current pool

UI08 [x] FIXED_VERIFIED_LOCAL: generated nativeReplaycopy chooseslastUnix/Windowsseparator/displayonly. 7exactgeneratedC++cases,46actualchecks/5routes/15newimagesPASS; dialog1412→363pxall5. CPU64frames/5bodies6shapes1joint/IDsposesexact/128MiB8policy preserved. Before/afterCPUfirst grids andall15new+5priorimagesinspected. UI09unsupportedLoadmessage remainsseparate/open.

Original65530diagnosticlinkfailure retiredobserver.o retained; correctedinputs4links79870PASS, no recompile/retryofunchangedfailure. Apps98682/CLOSEOUT83520terminal0; all10app/driverPIDsabsent/0cores/cachegrowth0. Streamsdeleted1581091, newdiagnosticretained2112827. Retiredold5viewersdeleted166316376bytes; oldmixedmanifest historical/unrunnable. **Currentpool ui08-build.json** with207sharedinputpinsunchanged; originalReplayobserver.o retainedfornextrelinks;generatedReplaycopy pinned. Raw875145255bytes. No livejob/allUI08budgetsclosed. Box3D/WASM/Rust/WGSLunchanged; existinglocalworkpreserved.

## Next actions

Continue repairs onecause atatime. UI09 next: validCPUfixture reachesGPU b3CreatePlayer unsupportedstub/null, sample collapsesallnullfailures into failed-to-open-file. Make generatednativeGPU/combined Replay UI explicitlyunsupported, preserve actualCPUload/errors. Trace source/inputs first; freezefinite budget beforeexecution. Remaining26issues/3verified, fullreview342widgets/165samples/23customgroups complete. OBS01appearance/OBS02GTKneedfinaltriage. GoalACTIVE/PR10unchecked/noPR10commitpushauthorization;precommitaffectedsceneclips/finalcriteria remainopen.

UI09 FROZEN:1generation10s/3CPP20s/5links25s/3nm3s/6apps60s/18screens/0Rustphysicsretries/600souter/1GiBwholeRaw192MiBnewbuild64MiBretained. SourcefirstfailureGPUb3LoadRecordingFromFileunsupportedNULL, notbadvalidfile. CPUvalid+missingerror andGPUhonestpopup/Close/R/exit.342widgets0left165samples0left23custom0left26open3verified.

UI09 generationPASS/sourcecheckstoppedbeforeanycompile/link/app: removing4GPUguardsleaves2addedblanklines, allnonblankCPUlinesexact. Originalbyteexactassertionpreservedui09-original-source-assertion.json. Correct sourceequivalenceassertiononly; continueunlaunchedoperations underoriginalfinitebudget, no generation/buildoperationrepeated.26open3verified.

UI09matrix terminal; readui09-matrix.json/receipts beforedisposition.342widgets0left165samples0left23custom0left26open3verified.

UI09build75045 CLOSED0/3compiles3symbolchecks5linksPASS; exactGPUobjectscontainnoLoadRecording/CreatePlayerreferences, CPUdoes. FourGPUonlyguards/CPUallnonblankcodelines+UI08preserved. UI09actualmatrix35103 LAUNCHED; pollsamehandle/no repeat. Currentcandidateui09-build.json, oldUI08poolretaineduntilverification.26open3verified.

UI09matrix35103 CLOSED1withCPU2+ordinaryGPU1complete;3ordinaryGPUtextpredicatesfalsebecauseexistingobserverdoesnotinstrumentTextWrapped. ActualpopupPNGshowsfullcorrectexplanation; source/calls/Close/R(newbaseworldgeneration)/exit allpositive. Preserveoriginalreceipt/predicates; displaycriterionrequiresmanualall3screens, notJSONabsence. Continueonly3UNLAUNCHEDGPUcases underremainingoriginal6appbudget, no build/CPU/ordinaryrerun.26open3verified.

UI09matrix terminal; readui09-matrix.json/receipts beforedisposition.342widgets0left165samples0left23custom0left26open3verified.

UI09displayinspectioncaught66charstatusclippedbyactualgfxTEXT_BUFFER_SIZE64; popupfullbutHUDtruncated. Samecausefollowup FROZEN46charconstantonly/1generation10s/2CPP20s/4links25s/2nm3s/4GPUapps45s/12screens/0CPUrerun/330souter/1GiBwholeRaw192MiBbuild64MiBretained. RetiredUI08pool166MB; keepcurrentUI09CPUandoriginallongmedia.26open3verified.

UI09shortbuild23950 CLOSED0/2compiles2nm4linksPASS; onlyGPUmessageconstantchanges/CPUprioractualpassesretained. ShortGPUmatrixrunning; poll tool handlefromlaunch, no oldcase reruns.25?NOTYET:26open3verified untilimages/disposition.

UI09matrix terminal; readui09-matrix.json/receipts beforedisposition.342widgets0left165samples0left23custom0left26open3verified.
