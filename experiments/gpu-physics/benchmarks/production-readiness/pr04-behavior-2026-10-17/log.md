# PR04 current recovery — read this first

Read this file FIRST at continuation/compaction/restart, then AGENTS.md,
`docs/goals/gpu-production-readiness.md` and the goal-scratchpad skill.
Before solver work read `docs/gpu-solver-goal.md` and relevant rejected
experiments in `docs/gpu-solver-qualification.md`. Historical next actions and
receipts do not authorize reruns or renew budgets.

## Full objective and authority

In /home/firtoz/work/2026/box3d-wasm on feat/gpu, complete PR04: plausible physical
behavior and useful completed physics SPS on ordinary/native NVIDIA Vulkan.
Ragdolls may fall or settle differently from real Box3D CPU. They must remain
articulated, collide/support correctly, and avoid unexplained explosions, severe
stretching, persistent jitter or runaway energy. Preserve applicable physical,
functional and independent calculation checks at their original tolerances.
No cross-engine bit/pose/chaotic trajectory parity. Preserve geometry/defaults,
body identity/picking, user work, Box3D and WASM. New automation uses TS/Bun.
Stop after PR04; PR07/08/full roadmap campaigns are not implicitly activated.

App goal BLOCKED pending the explicit Rain/performance budget decision; PR04 unchecked. Published checkpoint8d18dc8b6 is on origin/feat/gpu.
The user has now explicitly authorized a new checkpoint commit and push. This
authorizes publication of verified work; it does not renew experiment budgets. Previous goalturn was NO_PROGRESS at the same budget impasse. The current
third consecutive impasse observation meets the blocked audit threshold.
No new experiment is authorized; no completion or progress is claimed.

## Current status — 3 done / 3 left

| Gate | Status / authoritative evidence |
| --- | --- |
| A acceptance/baseline | OPEN: targets/classification frozen in acceptance.md; matched336-step timing exists, full current-source Rain600 physical baseline/applicability does not. See remaining-requirements.json. |
| B Rain/articulation | OPEN: FallingRagdolls600 passes all4GPUconfigs; full Rain600 residual/support/identity/recycling acceptance remains unqualified. Historical severe/persistent residuals are not waived. |
| C dragging/CCD | CLOSED at physical scope: drag-physical-summary.json,8scene cells/17,520combinedsteps/175,200body observations/8focusedCCD checks. Bothbackends/order0&1, original1320/3060windows. |
| D related regressions | CLOSED: related-qualification.json,400requiredRustcells (286currentpasses/114explicitreuse),50trace-fixture cells,52non-RainCcells (46currentpasses/6explicitreuse);4original configuration exclusions. Rain4Ccells remain B. |
| E throughput/latency | FAIL/UNQUALIFIED: Rain2.93SPS baseline/artifact4.64–4.74 vs60; nativeisolateddrag47–48SPS and long hitches; ordinaryground hitches also fail. |
| F cleanup/release | CLOSED: refreeze-acceptance.json/refreeze-report.md.2emptyRust+4freshnative builds,CPUsource carry,5CPUfirst consumers/maxerror0/4callbacks/9viewer controls;20current-source artifacts frozen. PR06 refrozen for retainedpolicy; laterenginechanges invalidateaffectedinputs again. |

## Budgets and pending decision

Rain/performance3/3 spent,0left. Drag investigation3/3 spent,0left. Contact-symmetry
resolved in1/3; its unused2 batches do not transfer. No additional investigation
is authorized by an automatic continuation or by writing a proposal.

Default ceiling per blocker:3×45min, at most2hypotheses and2variants per hypothesis
per batch. Freeze exact builds/runs/timeouts, progress and storage before launch.
Pivot after2unproductive candidates or1batch without causal/physical/performance
progress. Do not repeat exhausted historical experiments. Budget exhaustion with
promising unfinished work permits an honest partial outcome, not success or an
automatic extension. All credible ideas have NOT been proved exhausted.

One text-input question is pending: authorize ONE additional45min Rain/performance
batch or keep the limit and stop experiments. The reviewable scope/counts/storage
are in next-batch-proposal.md (NOT authorized or launched). It tests a demand-sized
joint-filter lookup:4200unique pairs exceed4096slots, causing full scans. The old
artifact only rejected geometry earlier. This is not permission to run it.

No new Rain/performance experiment while that decision is pending. Independent
F qualification is now complete. Remaining A/B/E depend on work beyond the
spent Rain/performance budget. This productive turn is the first reviewed
post-F budget impasse; do not count earlier productive turns as blocked.

## Retained source and build identities

Only production change since runtime442689de1 is sim.rs native normal policy:
paired normals now independent of contact priority. Current SHA256
6b4aeaf97f94ed856b11ddcf4b0d7b6d0e19d3c8296ec0b3ff46d0f6768d97dd.
Exact before/candidate images are contact-policy-sim-{before,candidate}.rs.
Both Cargo manifests restored; no production experiment feature. Broadphase
original SHA f49df7557960f55ce5b64ac518b57e09249bf33aa2ac91b27fbc4d3ad533d747;
its cull optimization remains artifact-only, not promoted.

Two current development libraries under ignored artifacts/pr04-contact-policy-
candidate-2026-10-17 have an explicit196MiB retained exception. Exact hashes/bytes
are current-policy-libraries.json. Their empty build-isolation feature has no cfg
consumer; they are qualified development inputs, not clean PR06 release outputs.
Keep for source/applicability audit and PR06 refreeze; never infer identity from
mutable target paths. Actual compiler-selected test paths are pinned in receipts.

Old ignored artifacts/pr05-pr06-release-candidate-2026-10-17 contains20files/
367,035,504B. All hashes still match the original clean-build receipts. Audit
compared259production inputs: onlysim.rs differs.14C/CPU artifacts unaffected;
The6affected oldfiles remainhistorical. Their2Rustarchives/4viewers have now
beenrebuilt andqualified inthenew20artifact candidate (refreeze-acceptance.json).
Do not overwrite or call theoldoutputs current.

## Key results and corrected hypotheses

- Contact symmetry repaired: original mode1 spin.019269153>.001 and transverse
  .001521538>.001 on both backends. Existing paired solver passes unchanged
  physical limits on both backends/orders;32physical+114changed-mode CCD/numeric
  checks and two3060step ground-drag confirmations pass.
- FallingRagdolls600:40CPU-first mesh/ragdoll cells pass. GPU peakanchor.116392836
  <.25m, tail.00298925536<.005m and tail velocities0. Original isolated4×120,
  noContacts60 and frozen45grid/12torus checks pass; input bytes matchPR03.
- Native reaction: old60s whole-fixture timeout retained. Perkind4worlds~11s
  predicted~98s; final unfiltered36worlds/9types/4configs/2856checks pass both
  orders in89.441/88.863s with120s caps. This is no speed/cold-start pass.
- Drag: correctmotor7.5Hz/damping1, superseding historical5Hz note in acceptance.
  .20m free-tracking bound retained. Only lowering with actual upward support
  impulse may be floor-loaded; every violatingframe checked. Old unconditional
  tracking and loaded trajectory FAILs remain. No scene resets in ground mode;
  original release<.1/support-.025/finite/identity checks preserved.
- Rain timing: matchedworker1/original336steps throughfirstrecycle; loadedfinal20
  CPU14.054ms/71.15SPS, ordinary341.040ms/2.932/native340.817ms/2.934.
  GPU~238ms (broadphase~186,solver~49), fetch~99ms includes hostCCD/events.
  Ordinary earlier reached407/600 at90s; normal viewers timedout180s. Viewer8worker
  records and headless1worker are separate, not matched CPU observations.
- Cull artifact: pure AABB/material rejection before joint scan gains58–62%,
  final20~211–215ms/4.64–4.74SPS, stillfails60SPS; only336steps, notRain600 health.
- CLEAN-GETTER LEAD FALSIFIED: with_world calls ensure_cpu_mirror_world, which
  gates sync on stale bodies/pending events/postCCD. Clean getters do not collect
  the fullbody array. callback snapshots are also conditional. Do not implement
  the old proposed unconditional-copy fix. This does not prove all read costs cheap.

Original failed receipts are immutable. Marker-only rejection, duplicate final
metadata label, and aggregate120s timeout are reconciled by separate evidence;
no failed run rewritten as PASS. Original standaloneidentity passed1.299s after
an aggregate interruption. Full chronology is recovery-history.md (exact prior
log SHA256 0925d59fec19bb337257d1d72332546709d9406f72ceddc579ab3f255bdbcd5f); detailed evidence remains in protocols/receipts/reports.

## Current process / cleanup / next action

All F sessions TERMINAL/PASS: Rust94049/native32641/consumer43677/viewers32584;
finalrefreeze verifier72967 PASS. Allbuildtrees/ownedconsumer inputs/executables/
viewer CWDs consumed/deleted; no GPU/build/viewer job orrawoutputs remain.
Last integrity audit passed211pins/allrecorded processgroups absent/raw0; the
latest source/release audit validates all20old artifacts unchanged. Shared caches
preserved. Defaulttemporary1GiB includingcache growth/raw128MiB/retained64MiB/
free20GiB; name consumers/deletion points and clean on success/failure. Retained
small diagnostics~18.3MB; the new367MB release candidate uses the explicit512MiB
allowance andcurrentqualification/PR07/refreeze consumers. Two development
libraries andtheold release candidate keep their recorded audit consumers. Do not create new backup/cache trees.

Next: F is complete for current source. Rain budget decision remains pending
and does not authorize a new experiment. Inspect actual input before
any dependent action. An answer preserving the ceiling means stop experiments
with this partial result. A renewed budget must be recorded explicitly, then
freeze exact commands/source restoration before execution. Do not automatically
resume any old handle, spent experiment, PR07/08 campaign or publication.

Final review audit32322 PASS:211pinnedinputs/allrecorded processgroups absent/raw0.
No new production/Box3D/WASM change.20oldreleaseartifact hashes match;14unaffected/
6requiringrefreeze. Newreport/plan/history are small text; no binaries/cachecopies
created. GoalstillACTIVE2done4left, budgetquestionpending/no additionalexecution.

F releasequalification budget FROZEN beforeexecution: refreeze-rust1350s (2empty
Rusttargets600s each,originalreleasefeatures/j2,source/dependencyreceipts),
refreeze-native1400s (4emptyCMakeconfigure45/build240/receipt30; onlypinnedsource
cache reused,dependencybuilds isolated),refreeze-consumer950s (original5CPUfirst
externalconsumers and4callbackcontrols),refreeze-viewers1150s (original9CPUfirst
cacheon/off pause/step/switch controls120s).0newphysics/performancehypotheses,
0Rainruns/captures/retries. Explicit6GiBtemporary/512MiBnewcandidate exception,
64MiBdiagnostics/free20GiB; candidateconsumers=Fverification andPR07/refreeze.
Buildtrees deletedaftereachreceipt andonfailure; no sharedcache/treecopies.
Stableignorednewcandidate path artifacts/pr04-release-candidate-2026-10-17
keepsprotocolinputpins valid afterrawcleanup. Old20artifacts/qualifieddevlibs
remainuntouched. Newdriver tolerates transientcompilerfile ENOENT inbytecounts,
retains othererrors andcandidate512MiBcap. Newproduction sourceunchanged.

F live session94049/refreeze-rust: currentreceiptPID verifiedlive, ordinaryclean
releasebuild underway. Poll exacthandle; do not restart on observation timeout.
Builds use2compilerjobs andemptyownedtargets; no shared compiledobjects reused.
F Rust94049 TERMINAL/PASS6/6jobs: ordinary189.705s/native208.147s, bothempty
Rusttargets/source-dependencyreceipts/verification PASS. Cleanbuildtrees consumed
anddeleted; twoqualified-build archivesatstablecandidate path awaitconsumer/UI
verification (notfinalrelease acceptanceyet). Refreeze-native protocol prelaunch
finalization adds source-directory overrides/freshdependencybuild paths and
ownedconsumerCPP cleanup onfailure; no engine/source edits ornewphysicsbudget.

F live native session32641: ordinarygpu cleanbuild+receipt PASS/buildtreegone;
ordinaryboth nowbuilding. ExactreceiptPID checkedlive; do not restart after
observationtimeout. Preflight xdpyinfo5s PASS. All3source dependencies reusedonly
assource withbuildobjects inownedraw; no globalcompileddependencytree overwritten.

F native32641 update: ordinarygpu/ordinarybothbuilds andreceiptsPASS, bothbuild
treesconsumed/deleted; nativegpuconfigurePASS/currentbuildlive, nativebothpending.
Continueexactsession, thenconsumer950s/viewers1150s frozenphases.2done4left.
F native32641 TERMINAL/PASS14/14jobs:4independentempty nativebuilds andcompiled
input/generated-source receipts; source/dependency bytes unchanged. All4buildtrees
consumed/deleted; newstablecandidate has20artifacts inclunchangedcarriedCPU2.
Consumers/viewerlifecycle notyetqualified; FstillOPEN2done4left.
F consumer43677 TERMINAL/PASS21/21jobs: actualCPUfirst5publicAPI consumers,
original1e-5/maxerror0,create/step/read/ray/counts/teardown and4callbackcontrols
PASS. Objects/executables/copiedCPPfolderconsumed/deleted. NativeCproducer hashes
andfeature provenance remainrecorded. Next exactfrozen viewerphase9cells,
CPUfirst/cacheon-off/pause/singlestep/sceneswitch; no Rain/performanceexperiment.

F CLOSED: new20artifact/current-source candidate validated; see refreeze-report.md.
2Rustcleanbuilds/4nativecleanbuilds,5CPUfirst publicAPI consumers at1e-5/max0,
4callbacks and9viewercontrols PASS. Actualreleasefeatures/noexperimentalfeature
verifiedfromCargocompilerartifacts; source/lock/dependency snapshots match.
CompatibilityWASM/TS/demo evidence reusedonly atunchanged-source scope.
Now3done3left(A,B,E); no Rain/performance budgetrenewal orpublicationauthority.

FinalF integrity89383 PASS: 350pinnedinputs,allrecordedprocessgroupsabsent/raw0.
Temporarydata removed2502968344B; observedpeakraw989299999B.
Newcandidate367034636B within512MiB; stableignoredarchive consumers
currentqualification/PR07/refreeze. Oldcandidate/devlibraries remainforhistorical
input/auditconsumers, no sharedcachesdeleted. refreeze-checkpoint.jsonrecords
cachegrowth andcleanup. Now3done3left(A,B,E), no newRain/performancebudget.

Budgetimpasse revalidation:3done3left(A,B,E), qualifiedsourceunchanged,
0active receipts/rawfiles, no newhumanbudgetauthorization. Previousgoalturn
PROGRESS(Fclosed); currentNO_PROGRESS. Consecutivebudgetimpasse observations2,
firstpure no-progress turn1; blockedthresholdnotyetmet, goalstaysACTIVE.
No build/GPUrun/repeatedquestion/commit/push. Existingbudgetquestion remains
pending; do notsilentlyextend3/3usedRain/performance ordraglimits.

Thirdconsecutivebudget-impasse observation verified: remainingA/B/E unmet,
source stillmatchesqualifiedrelease,0activephases/rawfiles, no newhumanbudget
authorization. C/D/Fandallindependentwork complete. Initialimpassewasrecorded
afterFcompletion; previous/currentcontinuations revalidatedunchangedcondition.
SetappgoalBLOCKED ratherthancontinue status-onlyautomaticturns. Fullobjective
preserved/PR04unchecked; resumeonlywithnewhumanbudget/constraint direction.
No runs/builds/commit/push, no alternativequalificationscope orcriterionrelaxation.

Publication request, 2026-10-17: commit/push the verified qualification and release-refreeze evidence, then supply a practical PR04 goal proposal. Publication budget: one read-only integrity audit (30s), one TS syntax/JSON validation (30s), whitespace/staging checks, commit (60s), push (90s); no builds, GPU runs or captures. Only existing small text reports, protocols, generated-source provenance and logs are selected; binaries/cache/raw trees stay excluded. Fetch completed: HEAD/origin 0 ahead/0 behind. New goal text is a proposal only; current app goal remains BLOCKED and existing investigation limits unchanged.

Publication verification PASS: 350 distinct pinned inputs match, all recorded process groups absent, raw bytes0; 94 JSON files parse and18 TS helpers transpile, authored-source/documentation whitespace check clean. Selected checkpoint647 text files/15,311,793bytes before this note; no binaries/cache/raw additions. Existing physical qualification reused at its recorded source/window scope; no new physics/performance result claimed.

Staged whitespace audit identified existing trailing whitespace/blank EOF lines in exact generated-source snapshots and captured command logs. Those bytes remain unchanged because receipts pin their hashes; the authored-source/documentation check excluding those two provenance directories passes. The checkpoint preserves failed receipts and makes no whole-PR04 success claim.
