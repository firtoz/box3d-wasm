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
| Repairs | **17 open; 12 fixes verified locally** | issues.json; UI01–UI29 reports |
| Final PR10 acceptance | Open | Repairs, resource/compatibility checks and final criterion audit |

The 165 sample dispositions comprise 71 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 165 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping, UI02 wheel routing, UI04 CPU Profile width, UI05 hidden HUD layout, UI06 current body-type transparency/default material and combined CPU ownership, UI07 combined Frame Camera, UI08 Unix Replay basename, UI09 honest Replay availability, UI10 captured key release, UI11 combined Escape selection, UI19 Wheel steering damping and UI20 Driving torque are locally verified fixes. Open: UI03 missing GPU overlays; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size; UI29 Mesh Drop auto regeneration hides controls behind repeated GPU preparation and fails to switch off. See individual reports and `issues.json`. The full review is complete; verified repair evidence follows.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain open for repair.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Fullreview COMPLETE:342widgets/165sample dispositions/23custom groups/85shared calls,0left. **17open,12fixesverified locally.** Verified:UI01/UI02/UI04/UI05/UI06/UI07/UI08/UI09/UI10/UI11/UI19/UI20. Goal ACTIVE,PR10unchecked.

UI11[x]:[repair](UI11-repair.md)/[acceptance](ui11-acceptance.json). ActualClearSelectionGPU[2,1,1]→NULL butcombinedownerCPU[2,1,0]/GPU[2,1,1]remained;nextdrawrestored both. Newboth_clear_selection clearsauthoritative selected[2]/renderers, generatedBOTHmaincallsitonlyatEscape selectionlayer beforeoriginalClearSelection. Mouse/grab/joint/world/physicsuntouched.25before+86repair evaluations=109originalpositive+2wrongregistryprobes adjudicatedsameactualdata. CPU10/ordinaryGPU10/nativeGPU10/ordinaryBOTH26positive+2adjudicated/nativeBOTH28positive;2.92/4.01/4.16/6.21/6.28s. PopupEscapepreservesID, plain/repeatedEscape clears, overlayEscape→spliton staysclear, activeCtrlgrabmouse/jointIDs retained/counter3→3→2untilmouseup, IDs/pausedposes/defaults/restart/lifetime/picking preserved.0scriptadvancingphysics/5–7startupsteps.25individualimages+2CPUfirstgrids inspected;165sourceapplicabilityrowsnot165newruntimes.

Originalordinarycombined2falseprobes readmutationcreationregistrycount2 forGPUinternalmousebody; nativeactualcounter3 andCPUregistry3/realownerIDs showcorrectgrab. ui11-original-grab-probes.json preservesflags/data;ui11-grab-probe-adjudication.json validates sameactualdata,no rerun/productrevision. Nativecombinedfirstcasecorrectcounterpassed. Do NOTclaimall111originalflags true/0originalfindings. ControlsEscape sourcebranchunchanged/notruntime-repeated; actualpickerEscapechecked. Seehistory-ui11-execution-2026-10-16.md.

**Currentpool ui11-build.json/link_plan PASS:**5viewers(3standaloneUI10binaries reused unchanged),20task-ownedlinkobjects plusnewDualmember/smalltaskAPIarchive;allnamednextPR10relinkconsumers. Exactobjects inui11-acceptance.json.resources.active_task_objects. Onlyarchiveboth_dual.c.o changed/map+passthroughbyteexact/sharedarchivesnevermutated. Currentpins/alllinkinputshash/existenceverified; headeronlynewprototype/typesunchanged. Oldui10/ui07/etccombinedviewer manifests historical/unrunnable afterretired2combinedviewers/oldBOTHmain/Dual/taskarchive. ThreeUI10standaloneexecutables remaincurrent throughUI11manifest. Do notverifyhistoricalmanifests.

Cleanup:UI11supersededbuilds168488180B/streams4771004B/logs212656Bremoved;raw933382473B/free1707997126656B/selected8964891B.32recordedPIDsabsent/0ownedcores/0measuredUI11cachegrowth/sharedcachepreserved. AllUI11budgetsclosed/no livejob. FreezeNEWfinitebudget before nextbuild/app/capture; default1GiBwholeRaw/64MiBretained/16MiBcacheapp32MiBrepair/20GiBfree. Atcurrentraw933MB, sequentialviewerreplacement fits; audit consumedoldtaskoutputs before anylaterbatch wouldapproach1GiB, preservingmedia/currentlinkinputs/sharedcaches.

Prior12verifiedfixes retainperreport evidence/limitations. Box3Dclean/diff--checkPASS;WASM/Rust/WGSL/sharedarchives unchanged. Finalnative+WASMcompatibility/criterion audit/precommit affected-scene recordings OPEN. NoPR10commitpush authorization.

## Next actions

NextUI12 nullHull nativepreviewcrash. ReadUI12.md/geometry-null-debug.txt+geometry-controls-acceptance.json, nativeDrawHull andactualBoxHull sampleinput/CreateHull. OriginalCPU fractionaltext0.7 transient0 createscoplanar8points/CreateHullNULL→DrawHulldereference. ExistingGDBboundary evidence available; DO NOTresumeexhaustedcoreextraction/reproducecrash justfornewcore. KeepBox3Dclean; fixgeneratednative rendering/caller atsmallestnulloperation preservingvalidgeometry/defaults. Verifyactualfractionaltext0.7, zero/degenerateboundary, validrestore/relatednativeHull callers onapplicableroutes; freezeNEWfinitebounds beforeCC/apps/captures. UseTS/Bun automation/currentui11pool; existinggeneratorPython maybemodified/invokedthroughTS. No solver/Rust/WASMchanges.

ThenUI13–UI29/UI03/OBS01–02/finalaudit. NoGTK/cold/generalmesh/core/solver retries;PR11queries/PR02category/RainPR04 separate. ContinueACTIVEgoaluntilPR10complete,thenstop. No nextbudget/livejob.

UI12preflight sourceDrawHull isgfx/draw.c (notdebug_adapter); currentwrapper safelyrecordsvalidfalse thenrealDrawHull dereferencesnull. ExistingCPU GDB coplanar8points/RDI0 cause reused, no crash/core rerun. RepairFROZEN5ar5s/2filteredonefilegenerations10s/1C30s/5links25s/5CPUfirstapps60s/30screens/550souter/192MiBnewbuild/1GiBraw64MiBretained16MiBcacheapp32MiBtotal/0physicsRustretries. CPU/BOTHqualifiedDrawunits haveidenticaloldobjectSHA; verifyall5archives beforeoverride.17open12verified.

UI12CPUcompleted12s actualleading0.7 NULLsafe/restores1.4volume;zeroaxesNULLsafe/restore/picking passed.3falseanalyticvolume0probes: nativeb3MakeScaledBoxHull clamps halfwidthminimum.02, actualXYZvolumes.02/.04/.08/bounds matchoriginalnative rule. Originalflags/source preserved; ui12-zero-probe-adjudication.json same-dataPASS. NoCPUrerun/productchange.4unrunroutes remaining continuation frozen0compile/gen/archive/4links/apps underoriginaltotalbudget. Currentpartialui12-build.json; oldpoolsourcepins stale aftercapacityguard.17open12verified.
