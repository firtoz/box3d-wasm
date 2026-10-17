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
| Repairs | **20 open; 9 fixes verified locally** | issues.json; UI01–UI29 reports |
| Final PR10 acceptance | Open | Repairs, resource/compatibility checks and final criterion audit |

The 165 sample dispositions comprise 71 actual specific-control fixtures plus 94 explicitly justified reuses of checked shared code on classes with no control/input/focus/layout overrides. They are **not 165 new runtime executions or feature passes**. Inherited widget definitions appear once per applicable sample in the 342-entry denominator. Five source widgets inside disabled Ragdoll/Pose `#if 0` are explicitly excluded; active native coverage remains 342.

## Findings

UI01 Info wrapping, UI02 wheel routing, UI04 CPU Profile width, UI05 hidden HUD layout, UI06 current body-type transparency/default material and combined CPU ownership, UI08 Unix Replay basename, UI09 honest Replay availability, UI19 Wheel steering damping and UI20 Driving torque are locally verified fixes. Open: UI03 missing GPU overlays; UI07 combined Frame Camera aspect; UI10 captured key-up leaves camera moving; UI11 combined Escape selection; UI12 Box Hull null render; UI13 unused Manifold controls; UI14 Hull-vs-Hull stale auto policy; UI15 invalid TOI menu; UI16 hidden Shape Distance radii; UI17 stale proxy cache; UI18 no-body preview framing; UI21 Mesh ragdoll anchor leak; UI22 Door limit state/recreation; UI23 zero mesh scale rejected; UI24 flat HeightField floor rejected; UI25 HeightField text edit loses trailing digit; UI26 failed Tree Load hides recovery controls; UI27 Mesh Scale sphere preview omits the cast start; UI28 Cast World hides active Box size; UI29 Mesh Drop auto regeneration hides controls behind repeated GPU preparation and fails to switch off. See individual reports and `issues.json`. The full review is complete; verified repair evidence follows.

UI12: real CPU fractional text `0.7` transiently applies `0`; eight hull points become coplanar, CreateHull returns null, original DrawHull dereferences null. GDB input/null boundary is in `geometry-null-debug.txt` and acceptance. GPU invalid input runtime has not been reproduced. Valid fractional `.7` spelling avoids the zero prefix for review; it is not a fix. Do not repeat core extraction: the 204,181,534-byte partial was deleted; the protected ~3 MB compressed original could not be deleted without sudo. Future owned apps use core limit 0.

UI13: four Manifold tools offer an unused Use cache checkbox; seven offer unused manual SAT radios. Exact affected lists and actual arguments are in `manifold-controls-acceptance.json`. UI14: Hull vs Hull selects auto but actual SAT input stays manual type 8 on all five routes. Closely related Triangle vs Hull correctly returns to automatic policy on all five routes; it is recorded unaffected. Both issues remain open for repair.

STARTUP01 belongs to PR05/08; Rain physical failures belong to PR04. Do not resume those campaigns.

## Current evidence and resources

Review COMPLETE:342widgets/165sample dispositions/23custom groups/85shared calls,0left. **20issues open,9fixes verified locally.** Verified:UI01/UI02/UI04/UI05/UI06/UI08/UI09/UI19/UI20. Goal ACTIVE,PR10unchecked.

UI06 [x]: [repair](UI06-repair.md)/[acceptance](ui06-acceptance.json). ActualnativeDrawShape cachedcreationtype2 while5realbodies becamekinematic1, so alpha1/transparent0vs5. Defaultpreset0 copied-input actualnativecalculation also roughness.40vs.55. CombinedCPUrawShape_GetBody returnedNULL/GPUGetType0; CPU prefix getters proved validactualowner/type. GeneratednativeDrawShape nowrefreshesexistingbodytype;2existingg_cpu_debug_draw guardbranches scopeShape_GetBody/Body_GetTypetoCPU. NormalGPUdispatch unchanged. Explicitoverrides sourceunchanged/no runtimefixture; nativepreset/default calculation distinctions documented. public_body_valid diagnostic isowner-validity,notproductIsValidAPIrepair. RenderFrame statsfirstCPUpaneonly; bothpanesmaterialrows/visualschecked.

All5routes PASS:48/48/48/61/61repairchecks in10.48/11.85/12.05/12.47/12.66seconds.22beforeboundary+9nativeprimitive+266repair=297true/0findings. BodyType type/opacity/defaultPBR/normalpreset/ID/pausedpose/restart/lifecycle,actualSingleBoxmousepickbothpanes,andCompoundSimpleparent/child/opacitychecked.0scriptedadvancingphysics; native5–6startupsteps.30individualbeforeafterimages+2CPUfirstgrids inspected.165applicabilityrowssharedsource reuse,not165newruns. SixGPU/nativecombinedbeforeimagesfromoriginalreview,build/startup/menu differences explicit. Seehistory-ui06-execution-2026-10-16.md for execution recovery; originalfailedwholearchive link retained inui06-original-whole-archive-link-failure.json. No qualifiedapprepeated.

**Currentpool ui06-build.json/link_plan PASS:**5viewers,16activeobjects (previous12Sample/Main/UI/HUD/Joint/Replay +2adapter+1bothDual+1materialobserver),1smalltaskAPIarchive forcombinedrelinks. Onlyarchiveboth_dual.c.o replaced; othermembersbyteexact,sharedarchivesnevermutated. Currentpins/alllinkinputsexist/hashverified. Preserve these namednext-relinkinputs; oldui05/04/20/trace/materialbaseline viewer manifests historical/unrunnable. Do not invoke theirbuildverifiers.

Cleanup:raw913512054B/free1708125020160B;selectedUI06diagnostics13287338B.174376968Bsupersededrepair/materialbuilds+33579515Bconsumedstreams+203108Boperationlogsremoved (tracebuildretirementalreadyclosed separately,avoid doublecount).47recordedPIDsabsent/0ownedcores/0measuredUI06cachegrowth;sharedcachepreserved. AllUI06budgetsclosed/no livejob. Default1GiBwholeRaw/64MiBretained/16MiBcacheperapp32MiBtotal,20GiBminimumfree. Freeze NEWbudget before nextexecution.

Box3Dsubmoduleclean/diff--checkPASS;WASM/Rust/WGSL/sharedarchivesunchanged. PriorUI01/UI02/UI04/UI05/UI08/UI09/UI19/UI20evidencevalid. Finalnative+WASMcompatibility/criterion audit/precommit affected-scene recordings remainOPEN. NoPR10commitpush authorization.

## Next actions

NextUI07 combined Frame Camera aspect. ReadUI07.md/body-view-controls-acceptance.json andscripts/sokol_split_injection.py +nativeSample::DrawUI/Camera::Frame. OnDrawUIwidenscamera.m_width800→1600forUI; actualmenuFrameuseswrongaspectwhileFusespaneaspect. Trace actualframebounds/aspect/fittingdistance and menu/key inputs at smallest calculation. Preserve fullUIlayout/paneprojection/picking. Freeze new finitebudget beforecompile/apps/captures; reusecurrentui06pool andbeforemedia, nofullsamplesweep.

Then remainingUI03–UI29 repairs/OBS01–02triage/finalaudit. No unchangedGTK/cold/generalmesh/core/solver retries; PR11queries/PR02category/RainPR04 separate. Continue ACTIVEgoal untilPR10criteria complete,thenstop. No nextbudget/livejob.

UI07traceFROZEN:1observerCPP30s/2links25s/2CPUfirstapps45s/4captures/200souter/64MiBnewbuild/1GiBwholeRaw64MiBretained16MiBcacheperapp/0productionphysicsretries. ActualCamera::Frame aspect/bounds/displayradius/distV/H/result Viewmenu vsF.20open9verified/review34216523complete.

UI07actualFrameboundary CLOSED/PASS:CPU9checks1.925s menu=keyaspect1.7778/radius47.1195;ordinarycombined9checks3.117s samebounds menuaspect1.7778/radius47.1602 vskey.8889/radius53.0552.4beforeimagesinspected. Originalobserverstatichelpercompilefailurepreserved/correctedread-only;0appsrepeated.2tracebinariesretired/cameraobserver nextrelink. RepairFROZEN1nativebeforelink/app75s/2screens then1generation10s/1CPP30s/5links25s/5CPUfirstapps75s/<=25afterimages/720souter/192MiBnewbuild/1GiBraw64MiBretained16MiBcache/app32MiBtotal/0physicsretries.20open9verified/reviewcomplete.
