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
| Sample widgets | **316 reviewed / 26 left** | inventory.json and linked per-widget evidence |
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

Latest replay-popup-acceptance.json:3newwidgetdispositions(Memorybudget/Mininterval/Cancel), ReplaystillPARTIAL10of36/26left. All17imagesinspectedCPUfirst(15cellgrid+2CPUextras). NativeSUPPORTED --replay input opensactualReplayViewer/Loadpopup. CPUCancel+R retainsactual512MiB/16framepolicy despitependingedits; CPUapply actual128MiB/8framepolicy/pregen64/returnsframe0 with5bodies6shapes1joint.4GPUactualpopupediting/Cancelworks; playerunsupportedandUI09misleadingfailed-openmessage remainsopen, neverGPUfeaturepass. UI08Unixfullpathmakesmodalwideall5 aspreviouslyrecorded. No productfix.

CPU-onlyobserver31947 CLOSED0: compile9.148s/link.221s. replay-observer-build.json isCURRENTMIXEDPOOL: newCPUatraw/replay-observer-build/cpu +4unchangedGPUsatraw/mutation-force-build.207originalinputs unchanged/read-onlyPlayer/callback/camera/Treeflags. OldmutationCPUandobserver.odeleted8486200bytes; oldmutationmanifestno longerfullyrunnable. PasscurrentbuildFileexplicitly.

Popup64519 CPUonlyCLOSED1/9.546s GTK8sdiscoverytimeoutbeforewidgets; corrected9624 CPUonlyCLOSED1 found/focusedownedOpenFile/pathsentbut8sawait-close timeout, beforewidgets. All4GPUapps ineachoriginalmatrixunlaunched/unusedbudgetclosed. Originalreceipts/streamtails preserved. OBS02GTKpathsubmissionPROBE_OR_ENVIRONMENT_UNCONFIRMED, notproductdefect; triagebeforefinalacceptance anddonotrepeatunchanged. Potentialtyping/readiness hypothesisunproved; priorPythonGTKpathpasses/sourceunchanged remainretained. Native--replay CLI actualsupportedalternativeforwidgetreview, notfakecontextinjection.

NativeCLI70915 CLOSED0/6apps(CPUcancel/apply+4GPUunsupported),84finalcheckkeys/17images; closeout8501 CLOSED0. All16viewer/driverPIDsfor8apps absent/0cores;2buildPIDsterminal0. LatestCLIcachegrowth399, retained2763445, streamsdeleted4033825, raw867540361; oldCPU/objectretired. Allbudgetsclosed/no livejob. Noengine/Rust/WGSL/Box3D/WASMchange.

ValidatednativeCPUfixture READY: replay-fixture-acceptance.json. **raw/replay-rich-corrected/fixture.b3rec**,76546bytes/64frames/5stablebodies+spawnframe9/deleteframe17 hole/namedshapes/revolute/contact/all7querytypes/stabletags/duplicateinstances/untagged. First5typespositive/miss; moverhitlists0infixture (CastMovernativeAPIalwayszeroentries/keepfractiondistinction), noqueryqualification. Originaldiagnosticcompile/positive-per-kindfailuresretained; finalread-onlyscanPASS/no recordingregeneration. Helpersdeleted3451352B/retained79773B/7PIDsabsent/0cores. Old28KBfixture retainedforUI08/basicReplayregression.

PreviousGhost3widgets/16images/all5PASS (source/data-backednonzero-to-zeroResetCounters, sourceheightthresholdmissesretained) remainsvalid; measuredsharedcachegrowth264543bytes, NOT0. Previousmutation/force13widgets/49images retainsUI29(all4GPUAutoofffails/repeatedPreparingSimulation), sourcecauseuntraced/UI25relationonlyhypothesis. Cast14widgets/79images retainsUI28andtwoPR11queryoutcomegaps. Allpriorchecks/sourceapplicability/historypreserved, no solver/cold/querycampaign. Detailedclosedreceipts inhistory-replay-popup-2026-10-16.md andlinkedacceptances.

Currentwatchlist: OBS01combinedBodyCastpale/whiteappearance UNCONFIRMED(actualDrawHullRGBA/selectionnotretained; projection/occlusionhypothesis), OBS02GTKprobeabove. Bothneedfinaltriage. UI12protected~3MBoldcorecannotdeletewithoutsudo;204MBpartialdeleted, neverextractagain. STARTUP01PR05/08, RainphysicsPR04, twoquerygapsPR11 andstandalonedefaultcategoryPR02 remainoutsideUIrepair. DoNOTresumeCapsulecoldstart/Benchmarkpost-ray100s/coreextraction/solver/querycampaigns.

## Next actions

Finish **26conditionalReplaywidgetentries +1custominputgroup** onactualCPU; GPUloadedplayerbranches explicitlyunsupported/UI09. Replaystilllast1sample/164done. ThenrepairdemonstratedPR10causesoneatatime andtriageOBS01/02. Current27UIissuesopen/2earlierverified.

Usecurrentmixedpool **replay-observer-build.json** andvalidated76KBfixture. SharedDriveroptionalr.start_args=['--replay',fixture] entersnativeactualLoadpopup; defaultSingleBoxargsunchanged. replay-ui.ts configure/state/node/click/choose supportallwindows(nativeoldDriveritemInfo-only). AvoidGTKforremainingwidgets untildistinctOBS02triage; actualoldOpenMenuchecksremainrecorded. RemainingShowTimeline/DrawAllQueries/FollowSelection/Zup/QuerySearchtext/kind/minhits/rebuild/resultselection, body/shape/joint/querytrees/details, speed/loop/substepspawns/workers/frameSlider andKeyboard/mouse/focus/layout.

CPUobserverrecordsactualPlayercreate/policy/workers/seek/step/pre-spawnstate, bodies/ordinals/querytags/bounds, DrawFrameQueriesindices/callbacks, nativecameraZup/rendertransform; Treeitemsincludeactualselected/hovered/clicked/toggledflags. Selectbody/shape/querytoverifyactualdetails andFollow/Focuscamera; nativeFrame0queriesempty, seekFrame1for9queries. Searchindex64frames/576rows; usefilters toboundUI, API/capture/receiptbytecaps. SplitremainingCPUcasesfinitely, reuseprior7basictransportpasses withoutreplayingunchanged. Freezenextbuild/check/app/capture/time/storagebudgetBEFOREexecution; nonecurrentlyfrozen/live. No fixesuntilreviewcomplete. GoalACTIVE/PR10unchecked/noPR10commitpushauthorization.

ReplayTimeline FROZEN1CPUapp90s/3screens/200actions/512publicstepcalls40seekcalls/3200recordedstepupperbound/0build/retry,24MiBAPI/1GiBraw64MiBretained.7widgets+Keyboardpartial;4GPUloadedbranchunsupported reusedfromcurrentpopupreceipts/sourceguard. Current316widgets26left164samples1left22custom1left27open2verified.

Timeline82077 CLOSED0/CPU36.835s with2harnessfindings: same-frame0editcorrectlydoesnotseek;4xwindowclipsat64, retainedactualPaction0→24over6nativeframesverifiesrate4. Allotheractualcontrols/Keyboardpass, originalsretained/no replay. GPUavailableTimelineTab-onlyFROZEN4apps25s/1screen/20actions/0world/replaysteps/build/retry/4MiBAPI/8MiBretained, loadedcontrolsunsupported.316widgets26left164samples1left22custom1left27open2verified.
