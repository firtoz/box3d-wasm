# PR10 UI review — recovery log

Read this FIRST after continuation/compaction, then `AGENTS.md` and `docs/goals/gpu-production-readiness.md`. Detailed earlier decisions/failures remain in [history-2026-10-15.md](history-2026-10-15.md); do not restart closed budgets.

## Objective and authorization

Finish PR10 only: inventory all native sample widgets and shared controls; actual mouse/keyboard review on real Box3D CPU, ordinary/native GPU and both combined viewers; verify displayed state and intended effect/lifecycle. Group identical shared implementation with explicit applicability; do not infer sample-specific passes. Keep a simple log, done/left counts, finite budgets, CPU-first before/after evidence for fixes, and cleanup. Preserve geometry, defaults, identity/picking and local work. Box3D/WASM/Rust/WGSL unchanged. Stop after PR10; no PR11/solver/cold campaign. No new commit/push authorization for PR10. PR09 is already published as `51ff8b06b4964d359386f08605d835ac6e2ea23d` on `feat/gpu`.

## Counts and current findings

- Inventory: 165 mapped / 0 left; 342 sample widget call sites in 67 samples; 85 shared calls (84 upstream CPU +1 generated combined-only split checkbox); 23 custom keyboard/mouse sample handlers.
- Shared call dispositions: **85 reviewed / 0 left**, including defective Bounds/Mass/transparency/framing calls. Sample-specific widgets: Jenga's two radio controls checked on all five routes; BodyType four controls also reviewed on all five routes; Replay7basiccontrols also reviewed; other329call sites remain open. **Full interaction reviews: 0 complete / 165 remaining.**
- [UI01 Info wrapping](UI01.md): [x] verified locally, 100before+100afterchecks/all5routes/90screens; native7–9clippedrows→0. Scoped generated Info wrap; defaults/identities preserved.
- [UI02 wheel/camera routing](UI02.md): [x] verified locally,35before+55afterchecks/all5routes/30screens. Actual stale capture causes panel zoom10→11.1 and swallowed outside wheel. Own-event-position helper repairs both; held gesture/popup/keyboard checks pass.
- [UI03 missing debug overlays](UI03.md): [ ] confirmed/unfixed. Actual Bounds/Mass flags reach draw; CPU2Bounds/2Transforms, GPU0/0, combinedCPUleftpositive/GPUright0.44checks/20screens. Other12overlay outputs have source gaps/routing checks and remain unqualified. Do not fix UI03 until review phase completes.
- [UI04 CPU Profile label clipping](UI04.md): [ ] confirmed/unfixed; tree interaction works, but indented constraints text clips at the fixed first-column edge. GPU native Profile has no CPU tree by policy.
- [UI05 hidden HUD overlap](UI05.md): [ ] confirmed/unfixed all5routes; sample text and paused HUD both use y=3fontSize. Both combined viewers also keep Comparison view visible over the footer when UI is hidden.
- Closed harness hypothesis (one-detent actual CPU control PASS7checks): retained CPU target oscillates y885↔725 because two wheel detents scroll160px across a132px clip. Do not call it a UI defect. GPU Profile is intentionally native timestamps only, with explicit unavailable CPU-substage text; CPU plots/reset/tree are absent by policy.
- STARTUP01 remains PR05/PR08 latency limitation; earlier visible/resizable loading window could take~70s on a pipeline miss. No startup or solver repair here. Rain original physical failures remain PR04, out of scope.

## Closed validation and evidence

[Solver sliders](solver-controls.md): 100checks/all5routes, actual120Hz7substeps public/GPU/mappedCPU arguments,3.5cm→.035m setters, Workers recreates world, defaults restored60Hz4substeps/5cm/8workers. Standalone GPU automatic scheduling and recording unavailable are explicit. Four shared call sites closed.

[Render controls](render-controls.md):12 actual renderer settings change/restore, all5debugmodes/all5routes, selected Exposure/normal image effects,20 selected images. Mouse Combo changed tier1→2→1. Native ImGui keyboard navigation is not enabled; use mouse with actual popup geometry. CPU partial receipts remain failures with valid checked subsets, not retroactively passed; negative argument parsing/half-stop contrast/weak same-tier assumptions are documented. Sixteen more shared calls closed. Actualmean157.87→55.33 with Exposure-6; defaults/identities preserved.

`menu-controls-routing.json`:305checks/all5routes prove menu input/state/actual flags only. Do not call missing GPU overlays passed based on those flags. All UI01/UI02/slider/Render/output jobs are terminal.

## Current build, terminal results and next inputs

All jobs terminal. Overlay matrix session69663 CLOSED0: CPU36.481s, ordinary37.435s, native38.18s, ordinarycombined38.335s, nativecombined38.944s. All47CPUchecks pass; eachGPUroute14missingoutputs extendsUI03; combinedCPUpositive. Shared sourcecalls85reviewed/0left, includingdefects/exclusions. Fullsample0/165, samplewidgets13/342reviewed329left, customhandlers23unreviewed. SevenUIissuegroupsopen, twoearlierfixesverifiedlocally. No newfix/commit/push.

Current qualified diagnostic pool overlay-callback-build.json, five binaries raw/overlay-callback-build/{cpu,ordinary-gpu,native-gpu,ordinary-both,native-both}. Original engine/gfx/main/Sample link inputs pinned unchanged. Counts/digests wrap9actualdrawcallbacks, originalcallback+contextonce; combinedCPUfirst/GPUinneroutputs separate. Old body-metadata and render-popup binary pools and consumed streams deleted after extraction. Main/Sample candidate objects under raw/ui02-after and raw/ui01-after still needed for possible later diagnostic link. Retain one28432byteCPUrecording raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec forremainingCPUReplaycontrols, deleteafter. No livejob; no next runtimebudget frozen. Never restart exhaustedbudgets or removed binaries.

## Storage and cleanup

`raw/` is locally excluded before producers; never stage full raw trees. No cache copies/videos or engine builds. Normal shared WGPU/NVIDIA caches are measured before/after each GPU batch and preserved for their consumers; no causal claims about concurrent shared growth. Current raw bytes and free space are appended below. Retain reports, selected images, minimal inputs and actual state/API/callback windows. Delete full world/API streams and superseded viewers after extraction/verification on success or failure. Preserve current live inputs. Cleanup receipts: `ui02-cleanup.json`, `solver-controls-cleanup.json`, `menu-controls-routing-cleanup.json`, `ui03-output-cleanup.json`, `render-controls-cleanup.json`, `navigation-metrics-cleanup.json`, `catalog-controls-cleanup.json`.

## Next actions

1. Next review remaining View controls (camera frame, transparency, draw distance, scales), recording/replay availability and actual picking/dragging under new finite budgets. Body Type is a small source-derived fixture for type/enable and dynamic/kinematic transparency; its actual setters/IDs/effects need read-only observation. Do not repeat passed shared sliders/Render/navigation or closed experiments.
2. Review remaining shared catalog/navigation, frame/culling/transparency/scales, recording/replay policies, picking/dragging and layouts with actual inputs under new finite budgets. Source-derived menu inventory should cover all165 registrations; shared case reuse must be justified. Pending GPU overlay output fixtures remain explicit.
3. Exercise all remaining sample-specific widgets/custom handlers, log each outcome and full-sample done/left counts. Do not fix newly found issues until the review phase is complete. Then fix one cause at a time with CPU-first before/after evidence, affected-scene checks and [x] only when verified.
4. PR10 stays unchecked until its full criteria pass. Before any future authorized visual commit, record every affected scene with real CPU first; current representative UI evidence is not that complete pre-commit grid. No commit/push instruction is active.

Last resource snapshot: raw 272975266 bytes; free 1709420457984 bytes. All apps/jobs terminal. Shared58reviewed/27left;2UI fixesverified/3UI issuesopen;full0done165left.

Body/View closure: 201 evaluations/190 true (defects and corrected harness assumptions stay explicit). Deleted 174195739 bytes; raw 289748616 bytes, free 1709201903616 bytes. Next remaining shared recording/replay and actual picking/dragging; sample-specific336widgets/23handlers and pendingoverlayfixtures remain.

Recording/Replay budget frozen in recording-replay-budget.json:0builds/5CPUfirstapps120s/ready25/dialog8/8screens160actions/0retries; two10step2bodyCPUrecordings consumedbyCPUload+4GPUexclusion checks,thenremove. Matrix CLOSED/session26792 (5failed harness receipts):CPU3headerchecks pass; textedit appendeddue groupedfocus/CtrlA/type assumption. GPUall4explicitunavailable+absentactions+GTKcancel pass; validloadunlaunchedmissingCPUfile. Correctedbudget frozen recording-replay-corrected-budget.json;CPUremainingfirst;GPUloads gatedonproducedfile;no productfix orrepeatedpassedchecks; rawstreams16MiB/app,record16MiB/file,1GiBtemp64MiBretained128MiBsharedcachegrowth. No new fixes.

Recording CPU corrected session25739 CLOSED1:9checks pass, both10step2bodyrecordfiles saved, RecordNowworldstable/RecordRestartnewworldstep0. GTKvalidOpen didnotclose8s. No repeatedrecording; next finite replay-load-tail-budget.json usesexistingtinyCPUfile, up to2GTKEnter eventswithactualbefore/afterGTKscreens,CPUloadfirst;GPUvalidloadstests gatedonCPU. No liveapp.

Replay focused CPU load CLOSED1/session98260; actual GTK screenshot path /home/firtoz/firtoz/work differs from intended /home/firtoz/work (harness typing/completion), no actualfileload; newreplay-gtk-path-budget frozen, clearGTKlocation/fast1mstyping;CPUfocusedonly. Previously; app receipt raw/replay-load-tail-runs/cpu/receipt.json.

Replay GTKpath session12729 CLOSED1: actualfileload succeeded; CPUtransport andframe0, >1,>2,<1 pass. Endbutton reaches actualrecordingendpoint but harness hardcoded10 failed3s. Preservevalidsubsets; deriveactualendpointfromUI/header before anytail. GPUloadtests maynowproceed usingactualCPUvalidfile.

Replay CPUtransport tail execsession64218 launched despite preparation error (budget variable shadowed); separatebudgetreceipt retrospective, not pre-frozen. Existingdriver hard120s/API16MiB/raw1GiB/screenshotcaps retained. Record deviation; do not repeat it. Exactterminal inraw/replay-transport-tail-runs/cpu/receipt.json. FourGPUbudget replay-gpu-exclusion-budget.json frozen successfullybeforepreparationerror; GPUapps stillunlaunched.

CPUtransport session64218 CLOSED1 beforeloading; actualGTK screenshot shows extra m/ inlongpath. No additionalproductfinding. Newreplay-short-path-budget FROZEN:0builds/CPUremainingtransport+4unlaunchedGPUloads,120s/25ready/8dialog/6appscreens+2GTKscreens/0retries. Oneverifiedtinyrecordcopy/tmp/p10-*/a.b3rec perapp,consumerGTKLoad,deletionfinally. This avoidslong-pathcompletion whilekeepingactualkeyboardinput. CPUprerequisitefailure preventsGPUlaunch. No app live yet.

Replay short-path matrix session12253 terminal; actualCPUtransporttail andall4GPUvalidload paths reviewed, exactstatuses inreplay-short-path-matrix.json. Temporary/tmp tinyrecordcopiesremovedbyfinally. Needsacceptance/inventorymergeandconsumedstream+producerfilecleanup.

Recording/Replay CLOSED:7sharedcallsreviewed→73/85,12left(pendingUI03overlayoutputs). CPUheader3+record9+loadforwardback5+transporttail6 plusall4GPUunavailable/cancel andvalidloadexclusion reviewed. UI08Unixbasename andUI09nativeReplaymessage confirmed/unfixed→7issuesopen/2fixedverified. Samplewidgets13/342reviewed329left;fullsample0done165left. replay-short-path matrix session12253CLOSED0/all5apps~5.1–5.3s;actualtemporaryfixturecopiesremoved. Cleanupdeleted1956551bytes;raw296581920free1709259894784. Only28432byteCPUrecordingretainedforremainingReplaycontrols. No app live; no next budget frozen.

Nextbuildbudget FROZEN overlay-callback-build-budget.json:1readonlyCPP15s/5links20seach/0apps/0screens/210MiBtemp8MiBretained/0retries. Observe9actualdrawcallbacks withscalarinputdigests; originalcallback/contextonce, splitCPU/GPUprovenance separate. Originalengine/gfx/main/Sample inputs unchanged. Consumer12remainingoverlayoutputchecks, thenreplaceoldqualifiedpoolafterall5runtime. No repair. Originalbuildsession3819CLOSED1 compileonlymissingheaders(cstring/worldgetter),0links/apps; failuremanifest/logpreserved. Correctedfinitebudget overlay-callback-build-corrected-budget.json frozen1CPP+5unusedlinks, requiredheadersaddedonly.

Overlay correctedbuild execsession50632; poll exacthandle/manifest overlay-callback-build.json before anyapp.

Overlay correctedbuild session50632CLOSED0/10.326s;1readonlyCPP+5links originalinputpinsunchanged. Runtimebudget FROZEN overlay-controls-budget.json:0builds/5CPUfirstapps240s/25ready90scene+firststep/8screens160actions/API32MiB/1GiBtemp64MiBretained128MiBsharedgrowth/0retries. BodyType twoactualsteps thenfrozenoverlays andJoint/Forcescale callbacks. CPUprerequisitefailurestopGPUexpectations. No app yet.

Overlay matrix LIVE execsession69663; exactcurrentchild inoverlay-controls-matrix.json.

Overlay closure: 263 checks/207true;56GPUeffectfailures are14checks×4routesUI03,not56independentbugs. Deleted185398653bytes;raw315751362free1709180690432. Nextactualpicking/dragging/camera sharedinputs, thenremaining329samplewidgets and23handlers. Shared85done0left;fullsamples0done165left.

Pointer/camerabudget FROZEN picking-camera-budget.json:0builds/5CPUfirstapps240s/25ready90firstdragstep+scene/8screens160actions/API32MiB/1GiBtemp64MiBretained128MiBsharedgrowth/0retries. ActualSingleBoxIDs/defaults/drag/release/worldfocus/pane mapping/camera controls. Allheldmousebuttons released in finally (notonlyLMB). Newfindingslogged, nofix. No app yet.

Pointer/camera matrix CLOSED1/session69848/7.931s:CPU13checks passinclactualdrag/originalID andpanelrelease; heldworldgestureWantCaptureMousefalse so no droppedMouseUp confirmed. Onlyerror missingxwininfoCLI beforefocuscase,4GPUunlaunched. Correctedbudget frozen samecaps, CPUfocus/cameratail+4unlaunchedGPUfull, useavailablelibX11readonlyrootID. No newproductfinding/fix.

Pointer/camera corrected matrix CLOSED1/session16310/8.079s:CPU14focus+camera checks pass, noissues; groupedCtrlclickdidnotactuallyentertextedit(want_text_inputfalse), no keyrelease conclusion. Newtext-capturebudget frozenCPUkeyonly+4unlaunchedGPUfull, holdControl/mousedownacrossrealframes, verifyactualTempInput thenrelease,restoreSubsteps4.

Pointer/camera actualtext-capture matrix CLOSED1/session41850/1.466s:SliderdidnotactivateunderheldRMBworldgesture;no keyrelease reached/no productfinding. Newpicker-capturebudget frozen:actualCtrlO givesexplicittextfocus whileRMB/W held;verifywanttext+picker beforekeyup. CPUremainingkeyonly+4GPUfull, no repeats of27CPUcompletedchecks/defaultsedits.

Pointer/camera picker-capture matrix LIVE execsession57345; exactchild in picking-camera-picker-capture-matrix.json.

Pointer/camera CLOSED/session57345:CPUkeytail2.368s,ordinary16.695s,native17.446s,ordinarycombined18.096s,nativecombined19.097s. CPU13+14previousvalidchecks+focusedkey4;GPU31/31/36/36checks reviewed. UI10keyupunderactualpickertextcapture/all5 andUI11combinedEscapeconfirmed→9openissues/2previousfixesverified. Actualdrag/panelrelease/focuscleanup/allcameraWASDQE pass; no MouseUpdropissueconfirmed. Sourcebasecontrolsqualifiedwithnameddefects,custom23handlersstillopen. Deleted9237836streambytes;raw325446380free1709264257024. Alljobs terminal, no nextbudget frozen. Next329samplewidgets+23customhandlers; fullsample0/165 remainsconservative untilsourceapplicability reconciliation. Shared85done0left, samplewidgets13done329left.

Nextgeometrybuildbudget FROZEN geometry-control-build-budget.json:1readonlyCPP15s/5links20s/0apps/0screens/210MiBtemp8MiBretained/0retries. ActualDrawHullbounds/volume/pointsdigest originalonce; source-derivedSliderFloat3componentrects actualinputvalidationpending. Consumer4geometrytools18widgets;noengine/gfx/main/Sample edits. Buildnotyetlaunched.

Geometry stage1buildsession68578CLOSED0/10.647s,0apps. Refreshsamegeometry needsactualfactoryrebuild count, notonlyunchangedrenderoutput. Supplementfactorycounterbudget geometry-control-factory-build-budget.json FROZEN1CPP+5links/0apps,samecaps; originalAPIs calledonce unchanged. Fiveunlaunchedstage1binariesdeleted164495016bytes before replacement,stage1manifest/logspreserved.

Geometryfactorybuildsession53719CLOSED0/10.598s. Runtimebudget FROZEN geometry-controls-budget.json:0builds/5CPUfirstapps240s/25ready90scene/8screens320actions/API32MiB/1GiBtemp64MiBretained128MiBsharedgrowth/0retries. 4no-bodygeometrytools18samplewidgets, actualHullfactory/drawinputs plusanalyticbounds/volume andrestore; BoxHullFloat3all3components. CPUfailurestopsGPU. Noapp yet.

Geometry matrix session90287CLOSED1/CPU6.226s: BoxHullh:0 change/restore actualvolume/bounds passed; typing h:1 causedSIGSEGV2158015 inDrawHull→BoxHullRender. GPU4unlaunched. Existingtasksystemcore2.8MiBcompressed; coreinspectionbudget geometry-crash-budget.json FROZEN0newapps/0builds/1extract10s≤256MiB/1gdb30s/128KiBtext/0retries. Futureapps mustsetRLIMIT_CORE0; no corecollection producer requested. No repair.

Existingcore extraction -25(sizecap),partial204181534bytesdeleted; GDBnotlaunched, no largerextractionretry. Newdistinctminimaldebugbudget geometry-null-debug-budget.json FROZEN0builds/1CPUappunderGDB45s/1screen30actions/32MiBtemp1MiBretained/0cores/0retries: h:1=0prefix, captureCreateHull8points andstopbeforeDrawHull(null). No repair.
