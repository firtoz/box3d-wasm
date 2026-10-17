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
| Sample widgets | **323 reviewed / 19 left** | inventory.json and linked per-widget evidence |
| Sample interaction dispositions | **164 reviewed / 1 left** | sample-applicability.md/json |
| Custom input sample groups | **22 reviewed / 1 left** | Manifold, Shape Distance, Driving and Door acceptance receipts |
| Repairs | **27 open; 2 earlier fixes verified locally** | issues.json; UI01–UI29 reports |
| Final PR10 acceptance | Open | Remaining review, repairs, resource/compatibility checks and final criterion audit |

The 164 sample dispositions comprise 70 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 164 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping and UI02 wheel routing are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI08 Unix Replay basename; UI09 unsupported Replay message; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size; UI29 Mesh Drop auto regeneration hides controls behind repeated GPU preparation and fails to switch off. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Latest replay-timeline-acceptance.json:7newwidgetdispositions. ActualCPUShowTimeline/Timeline/Speedall5values/Looptruefalse/SubstepSpawns/Workers1,2,8/FrameSliderboundaries/spawn-delete. NativeCPUcomma/Shift5/CtrlAltgating/frame0clamp reviewed, customgroupstillPARTIAL pendingmouse/focus/layout. ActualGPUavailableTimelineTab Profile→Timeline/fileline/worldidentity testedall4. Other6loadedplayerwidgetsunsupportedGPU/UI09, neverfeaturepass.

Timeline82077 CLOSED0/CPU36.835s/120actions/3screens/84totalfinalcheckkeyswith4GPUtabapps; retained2harnessfalsepredicatescorrectedsource/datawithoutreplay: samevalueframe0editdoesnotSeekagain;4xlatewindowclipsatEOF64, independentnativePactionframe1272→1278/replay0→24 proves4x6 beforeend. Allothernativeinput/effects pass. FinalcounterspublicStep290/Substep2/Restart2/Seek22/Workers3 underfrozencaps; no solver/determinism/queryqualification. GPUtab12379 CLOSED0/4apps. Closeout29761 CLOSED0. All7newcaptures+4priorunsupportedimagesinspectedCPUfirst;3freshCPU+4freshGPUtab+4reusedunsupportedimages make15gridcells/11uniqueimages, no newGPUspawn/loopcaptures claimed. FullCPUEOFimagefloorvisible/nativeasleepbluecolors, no floorissue.

All10ownedviewer/driverPIDsabsent/0cores/cachegrowth0. Newstreamsdeleted7456807/retained2328042/raw869868403/free1708277428224. Allbudgetsclosed/no livejob. CurrentMIXEDPOOL **replay-observer-build.json**: newCPUraw/replay-observer-build/cpu +4unchangedGPUraw/mutation-force-build.207originalpinsunchanged; oldmutationCPU/observer.oretired, oldmutationmanifestnotfullyrunnable. PasscurrentbuildFileexplicitly. Noengine/Rust/WGSL/Box3D/WASM/productfix.

Earlierreplay-popup-acceptance.json3newwidgets/all5/17images remainsvalid: CPUCancel+R actual512MiB/16persistedpolicy despitependingedits; CPUapply128MiB/8/pregen64/5bodies6shapes1joint.4GPUpopupedit/Cancelmechanicsactual, nativeplayerunsupported/UI09failed-openmislabel; UI08Unixpathmakesmodalwide. CLI70915sixappsPASS/84checks, retained2763445; CPUobserver31947 compile9.148/link.221PASS; oldCPUobjectretired. Seehistory-replay-timeline-2026-10-16.md forclosedpopup/GTK/previousbatches.

ValidatedactualCPUfixture **raw/replay-rich-corrected/fixture.b3rec**,76546B/64frames, replay-fixture-acceptance.json:5stablebodies+spawnframe9/deleteframe17 hole/namedshapes/revolute/contact/all7querytypes/stabletags/duplicateinstances/untagged. First5typespositive/miss; moverhitlists0infixture, CastMovernativeAPIzeroentrieskeepsfractionseparate. Noqueryqualification. Originaldiagnosticcompile/coveragefailuresretained/finalread-onlyscanPASS/no recordingregeneration. Old28KBfixture stillneededUI08/basicReplayregression.

Watchlist: OBS01combinedBodyCastpale/white UNCONFIRMED(actualDrawHullRGBA/selectionnotretained; projection/occlusionhypothesis); OBS02GTKpathsubmissionPROBE_OR_ENVIRONMENT_UNCONFIRMED. GTK64519timedout8sdiscovery,9624found/focusedownedOpenFile/pathsentbut8sawait-closefailed, allbeforewidgets/terminals/streamsconsumed; no productdefectinferred, no unchangedretry. PriorPythonGTKpasses/sourceunchanged retained; typedelay/readiness hypothesisunproved. NativeSUPPORTED--replayactualCLIletswidgetsreviewcontinue. BothOBSneedfinaltriage.

PreviousGhost3widgets/16images/all5PASS remainsvalid (measuredcachegrowth264543B, correctedfrom0); mutation/force13widgets/49images retainsUI29repeatedGPUPreparingSimulation/Autoofffailure/sourcecauseuntraced/UI25relationhypothesis; Cast14widgets/79images retainsUI28/twoPR11querygaps. UI12protected~3MBoldcorecannotdeletewithoutsudo,204MBpartialdeleted;neverextractagain. STARTUP01PR05/08/RainPR04/queriesPR11/defaultcategoryPR02outsideUIrepair. DoNOTresumeCapsulecold/Benchmarkpost-ray100s/core/solver/querycampaigns.

## Next actions

Finish **19conditionalReplaywidgetentries +1custominputgroup**, lastnative sample remains164done/1left. ThenrepairdemonstratedPR10causesoneatatime, triageOBS01/02. Current27UIissuesopen/2verified.

Usecurrentmixedpool **replay-observer-build.json**, nativeCLIr.start_args=['--replay',validatedfixture] andactualLoadbutton. replay-ui.ts configure/state/node/click/choose supportallwindows; currentDriverdefaultSingleBoxargsunchanged. No nextbudget/livejob. Freeze nextfinitecheck/app/capture/time/storage budgetBEFOREexecution.

Remaining19: DrawAllQueries,FollowSelection,ZupView; QuerySearchheader/text/Kind/Minhits/Rebuild/resultSelectable;5TreeNodecalls(body/shape/joint/Queriesheader/queryleaf);Body/Shape/Contacts/Joint/Querydetailheaders. SuggestedtwoCPUcases: querysearch/draw/Queries/Querydetails (about10entries), selection/body-shape-joint/Follow/Zup/details (about9entries)+custommouse/focus/layout. AllloadedbranchesGPUunsupported alreadyobservedsourceguard/currenthash, nofakefeatures.

SeekFrame1 for9recordedqueries (Frame0empty). QuerySearchbuildindex64frames/576rows;filter toboundUI. CPUobserverreadsactualPlayercalls/frames/ordinals/queries/drawindices/callbacks/nativeZup/rendertransform andTreeflags(selected/hovered/clicked/toggled). NativeBody/Shape/Jointsselection bycreationordinal, detailtextsactualIDs/type; FollowusesCOM/queryboundscenter, FocusF usesselectedshape/queryAABB orbody; Homefitsrecording. Actualviewportplainclickselectsshape/read-only; Ctrl/Shift/rightgated, MouseMove/Upempty/no physicsmutation. NeedactualMouse/Focus/layout/HasSolverControls beforecustomgroup23complete. KeepallbodyIDs/defaults/geometry native. No fixesuntilreviewcomplete. GoalACTIVE/PR10unchecked/noPR10commitpushauthorization.

ReplayQuery FROZEN1CPUapp120s/3screens/170actions/400publicstep30seekcalls/2400recordedstepupperbound/0buildretry,32MiBAPI/1GiBraw64MiBretained.10widgets; actualindex/filter/pin/nativecallbackdraw;4GPUloadedbranchunsupportedreuse.323widgets19left164samples1left22custom1left27open2verified.

Query71394 CLOSED0/CPU25.741s/75actions, all10effects pass exceptunexplainedimmediatetextpredicateuppercase (no actualwrongvalueretained); nativechangehistoryfullPR10-RAY and64rows preserved, sameframe/calls/identities. No productdefectinferred. Selection+textobservability FROZEN1CPUapp140s/5screens/220actions/400step30seekcalls/2400recordedstepupperbound/0buildretry,32MiBAPI/1GiBraw64MiBretained.9remainingwidgets/custommouse/focus/layout/exit plusonlytextprobe, GPUloadedunsupportedreuse.323widgets19left164samples1left22custom1left27open2verified pendingreviewcloseout.
