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
| Repairs | **25 open; 4 fixes verified locally** | issues.json; UI01–UI29 reports |
| Final PR10 acceptance | Open | Repairs, resource/compatibility checks and final criterion audit |

The 165 sample dispositions comprise 71 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 165 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping, UI02 wheel routing, UI08 Unix Replay basename and UI09 honest Replay availability are locally verified fixes. Open: UI03 missing GPU overlays; UI04 CPU Profile clipping; UI05 hidden HUD overlap; UI06 stale type/transparency; UI07 combined Frame Camera aspect; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI19 wrong Wheel steering-damping setter; UI20 unused Driving torque slider; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size; UI29 Mesh Drop auto regeneration hides controls behind repeated GPU preparation and fails to switch off. See individual reports and `issues.json`. No new product fix was made in the latest review.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain unfixed pending review completion.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Full control review COMPLETE:342active widgets/165sample dispositions/23custom input groups/85shared calls,0left in each denominator.165=71actual specific-controlfixtures+94explicitshared-code reuses, not165newruntimefeaturepasses. Replay loaded-player GPUbranchesremainunsupported; defects/exclusions are preserved. Latest review replay-final-review-acceptance.json. GoalACTIVE/PR10unchecked;25openissues/4verifiedfixes.

UI08 [x]: ui08-acceptance.json/UI08-repair.md,7exactgeneratedC++filenamecases+46actualchecks/all5/15newcaptures. Modal1412→363px; CPUoriginalrecording/IDsposes/policy preserved. GeneratedReplaycopy/CMakewiring keepsBox3Dclean. Oldlinkfailure retiredobserver.o preserved; correctedinputspassed, no unchangedretry.

UI09 [x]: ui09-acceptance.json/UI09-repair.md. FirstnulloperationGPUb3LoadRecordingFromFile unsupportedstub; genericbad-filemessagewrong. GeneratedGPU/combinedpopup nowexplainsunsupportedplayback/offersClose/hidesineffectiveLoadpolicy; constructor/RskipunavailableAPIcalls. CPUvalid64frames/5bodies6shapes1joint/IDsposesexact andmissingfilegenuineerror/Load/Cancel checked. Final46charstatusfitsactualgfx64bytebuffer; all4GPUClose/R(newbaseworldgeneration)/exit normalSingleBoxcontrols verified. Nativeplayerimplementation remainsoutofscope/unavailable.

UI09 initialbuild generation/sourceassertion stoppedbeforecompile:4GPU-onlyguardsleave2blanklines, everyCPUcodelineexact; preservedoriginalassertion, no generationrepeat. ThreefirstordinaryGPUtextpredicates searcheduninstrumentedTextWrapped; originalreceipts kept, actualmodal/closed/restartimages adjudicate. Visual reviewcaughtinitial66charHUDtruncation; distinct46charconstant-onlyfollowup2CPP/4links/4GPUapps, CPUactual2passesreused. Final41currentchecks. All28new+5priorimagesinspected; currentCPUfirstfinalgrid ui09-short-after-grid.png. No requirement/tolerance relaxed. CombinedInfostatusbelowcurrentclip, fullmodal andHUD visible.

All viewer/driver jobs terminal/absent and streamsconsumed:35103initial3apps,62810remaining3,9677short4; builds75045/23950, closeouts24702/9187closed. Latest12+8ownedapp/driverPIDsabsent/0cores; cachegrowth0. UI09streamsdeleted2678851, diagnosticretained4349538. RemovedoldUI08pool166414968bytes andlongGPUcandidate158663168bytes afterreplacementchecks. CurrentRaw879638137bytes/free1708161732608. All budgetsclosed/no livejob.

**Currentmixedpool ui09-short-build.json**: CPUraw/ui09-build/cpu +4raw/ui09-short-buildGPUviewers. Original207sharedinputpins plus3flagfiles preserved; activeoriginalReplayobserver.o requirednextrelinks. GeneratedReplaycopies pinned; productgeneratorhash current. Oldmutation/replay-observer/UI08/longUI09 manifests historical/unrunnable (retiredbinaries). Usecurrentmanifest explicitly. Box3D/WASM/Rust/WGSL/sharedarchives unchanged; localUI01/UI02 andotherworkpreserved.

Keep raw/replay-rich-corrected/fixture.b3rec76546B/64frames andold28KBraw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec forremainingregressions/finalaudit. Keepselectedbefore/after media/receipts. Deletecurrentviewersonlyafterreplacement/finalqualification;neverstageRaw. Detailedclosedbudgets/history in history-ui09-repair-2026-10-16.md.

## Next actions

Continue remaining25UIissues onecauseatatime. Suggested nextsmallproven causeUI19: Wheel steeringDamping slider invokesSuspensionDampingRatio instead ofSteeringDampingRatio; readUI19.md/joint-controlacceptance/source first, preservedefaults, patchgeneratedJointcopy andverifyactualsetter/getter+unaffected suspension/all5routes withCPUfirstbeforeafter. Freezefinitebudgetbefore execution; currentpoolabove. ThenUI20unusedDrivingtorque/remainingUI03–29; do notmarksharedseparatecausespassedimplicitly.

OBS01combinedBodyCastpale/white unconfirmed(actualRGBA/selectionnotretained) andOBS02GTKpathsubmissionprobe/environmentunconfirmed needfinaltriage. NeverretryunchangedGTK/cold/generalmesh/core/solver campaigns. PR11queries/PR02category/RainPR04 remainseparate. FinalPR10criteria/normalbuildcompatibility/precommitaffectedsceneclipsremainopen. NoPR10commitpushauthorization. Continueactivegoal/stoponlyafterPR10complete, notroadmap.

Finalrecoverycheck:diff--checkPASS/Box3Dsubmoduleclean/shared207pinschecked;UI19sourcepreflightconfirmsWheel1463wrongSuspensionsetter+variable, Driving2478–79correct/unaffected. NextpatchscopedgeneratedJointcopy, freezesmallall5setter/getter/identity/capturebudgetbeforeexecution. No livejob/nextbudget. GoalACTIVE/25open4verified/full34216523reviewdone.

PreviousgoalturnclassifiedPROGRESS: fullreviewclosed/UI08UI09verified/currentpoolcleaned. UI19FROZEN3CPP30s/5links25s/1generation10s/5apps90s/20screens/0physicsenginebuildretries/1GiBwholeRaw192MiBnewbuild64MiBretained/750souter. Existingactual5routebeforetraceprovesSteering1.4→Suspension1.2/Steering.7; Drivingcorrectunaffected. Scopeonesetter/inputplusactualrelatedregression.342widgets0left165samples0left23custom0left25open4verified.

UI19 generation/sourcecheck stoppedbeforecompiles: existingcurrentgeneratoraddsBOTH-onlysokol_split_step.hdeclarationincludevsoldgeneratedartifact. Headerdeclarationsonly/noexecutedcode; all5sourcesotherwiseexactoneWheelsetter/inputdelta. Originalsourceassertionretained; correctexplicitincludeallowance/continueonlyunlaunched3compiles5links/originalbudget, no regenerationrepeat.25open4verified.

UI19sourceassertionadjudicated/3JointcompilesPASS;initialCPUlinkfailedretiredReplay.o dependency (cleanupconsumererror), noUIapps. Originalfailurepreserved/alloldunspentbuildbudgetclosed. DistinctcorrecteddepsFROZEN3ReplayCPP20s/5links25s/0Jointcompilegenerationengineappsretry; exactcurrentqualifiedReplaysource. Retaincurrent6translationunitobjects+linkplansfornextrepairs, subMiBneededinputs, ratherthanrepeatingthiscleanupmistake.25open4verified.

UI19correcteddeps44418 CLOSED0/3Replaycompiles5linksPASS;6activeobjects+link_planretained/currentcandidateui19-build.json. Actual5routechecks launchedunderoriginalbudget; pollnewexacttoolhandle. No engine/Rust/WGSL/Box3D/WASMedits.25open4verified.

UI19actualmatrixhandle93215; pollsameiflive, readmatrixifterminal.

UI19matrixterminal; readui19-matrix.json/receipts andinspectcaptures beforechecking[x].25open4verified/34216523reviewcomplete.

UI19matrix93215 CLOSED1/CPU90s timeout/0dampingcases beforeconditionalwidgets: sourceconstructorSuspensionSpringfalse/Steeringfalse; setupomittedcheckboxclicks. Originaldefaultcapture/receipt/failuretailpreserved; streamsconsumed/allPIDsabsent. DistinctinputpreconditionsFROZEN5apps90s/20captures/0buildstepsretries/same1GiB64MiB/480souter; actualcheckboxclicks+jointenablegetterproof, defaults unchanged. FourGPUfirstlaunch, CPUdistinctstimulus.25open4verified.

UI19matrixterminal; readui19-matrix.json/receipts andinspectcaptures beforechecking[x].25open4verified/34216523reviewcomplete.

UI19correctedactualmatrix75909LAUNCHED; samequalifiedui19-build.json/checkboxenablepreconditions. Pollsamehandle, neverrepeatbasedonobservationtimeout.25open4verifiedpendingevidence.

UI19correctedmatrix75909 CLOSED1:CPU6Wheeldampingtransitions/all39?checks/3capturesPASS, jointenable/wake/IDsposes/resetallpositive; Drivinginitial3!=2filtermistake countedParallel(nonWheel undefinedspin). NativeactualDriving1Parallel+4Wheel/2front2rear. Originalreceiptkept/streamsconsumed. DistinctcompletionFROZENCPUDrivingonly+4GPUfullfirstattempt/5apps75s/20screens/0buildstepsretries/400souter/samecaps; kindfiltercorrect/Parallelunchanged. ReuseCPUWheelpasses/noCPUWheelrerun.25open4verified.

UI19completionactualmatrix48250LAUNCHED; CPUDrivingonly+4GPUfull; pollsamehandle/currentui19-build.json. CPUWheelpassesreused.25open4verified.

UI19matrixterminal; readui19-matrix.json/receipts andinspectcaptures beforechecking[x].25open4verified/34216523reviewcomplete.

UI19completion48250 CLOSED1/resourcecap:CPUDriving+ordinaryGPU+nativeGPUPASS;ordinaryboth4Wheeltransitions/2imagesPASSthenwholeRaw1GiBguardstopped/streamcleanup2.3MB, nativebothunlaunched. Retired3alreadyqualifiedoldviewers frees86303496bytes; unchangedhardcap. DistinctremainingbudgetFROZEN2apps75s/8captures/0buildstepsretries/170souter: ordinarybothrestore.4/.3setup→.7/.7+R+Driving; nativebothfirstfull. Reuseallcompletedpasses/images.25open4verified.

UI19matrixterminal; readui19-matrix.json/receipts andinspectcaptures beforechecking[x].25open4verified/34216523reviewcomplete.

UI19remainingcombinedmatrix81811LAUNCHED; onlyordinarybothrestores/Driving andnativebothfull, after86MBretiredpoolcleanup. Pollsamehandle, reusepriorpasses.25open4verifiedpendingfinalevidence.
