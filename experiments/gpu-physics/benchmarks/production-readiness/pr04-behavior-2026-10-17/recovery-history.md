# PR04 behavior and throughput recovery

Read this FIRST on continuation/compaction/restart, then AGENTS.md,
docs/goals/gpu-production-readiness.md and the goal-scratchpad skill. Before
solver edits read docs/gpu-solver-goal.md and relevant rejected experiments in
docs/gpu-solver-qualification.md. Verify live handles, not stale state files.

## Full objective and authority

Complete PR04 in /home/firtoz/work/2026/box3d-wasm on feat/gpu: plausible physical
behavior and useful completed physics steps/second on ordinary/native NVIDIA
Vulkan. Ragdolls may fall/settle differently from real Box3D CPU; no cross-engine
bit/pose/scene-trajectory parity. Require articulation, collision/support, finite
healthy states without unexplained explosion, severe stretching, persistent
jitter or energy gain. Preserve original applicable physical/function/calculation
limits, geometry/defaults, identities/picking, user work, Box3D and WASM.
Use TypeScript/Bun new automation. Stop after PR04, not the full roadmap.
The six-gate/pivot/stop contract in the roadmap remains authoritative.

App goal is ACTIVE. Published checkpoint8d18dc8b6 is pushed tofeat/gpu.
Previous turn was PROGRESS: verified/published checkpoint. That publication
authorization is consumed; no further implementation commit/push inferred.
This continuation qualifies retained source; Rain/drag budgets remain spent.

## Current status — 2 done / 4 left

| Gate | Status and evidence |
| --- | --- |
| A acceptance/baseline | OPEN. acceptance.md classifies original101Rust/13trace/14C recipes and freezes targets. Matched headless336 CPU/ordinary/native timing exists; full current Rain600 physical baseline and source-applicability audit unfinished |
| B Rain/articulation | OPEN. Current FallingRagdolls600 physical pass onall4GPUconfigs; fullRain600 remains unqualified and historical severe/persistent joint residuals remain |
| C dragging/CCD | CLOSED at current physical scope: drag-physical-summary.json;8scene cells,17,520combinedsteps,175,200independentbody observations,8focused landing/mutation checks. Bothbackends/order0&1, original1320/3060windows. Limits/applicability below; broader29CCD belongsD |
| D related regressions | CLOSED. related-qualification.json reconciles 400requiredRustcells (286currentpasses/114explicitreuse),50trace-fixture cells and52non-RainCcells (46currentpasses/6explicitreuse). Originalnativewhole36worldreaction nowpasses bothorders at120s confirmationcap; original60stimeout preserved. Rain4Ccells belongB and remainopen |
| E throughput/latency | OPEN/FAIL. Rain2.9SPS baseline/candidate4.6–4.7 vs60target; native isolateddrag47–48SPS and large hitches; do not present physics pass as speed pass |
| F reconcile/cleanup/refreeze | OPEN. Qualification phases terminal andconsumedoutputs cleaned; PR06sourceapplicability remainsreopened for finalreleaseinputs. Contact-policy repair retained; frozenPR06 oldsource untouched and applicability reopened |

Baseline runtime442689de1, current publishedHEAD8d18dc8b6. Broadphase shader was restored byte-exact after its experiment; that cullcandidate
remains unpromoted. The later contact-policy source repair is retained; shared
libraries now contain it and testpaths mustbeidentified by compilerreceipt/hash.
Frozen PR06 release candidate remains20files/367,035,504bytes under
experiments/gpu-physics/artifacts/pr05-pr06-release-candidate-2026-10-17. Consumer:
focused qualification and PR06 applicability/refreeze. Preserve it and sharedcaches.

## Current process/source state and next actions

All sessions TERMINAL. Relatedregression gateD CLOSED: related-qualification.json.
Latestnativewholeconfirmation86414 PASS8/8jobs: order0 89.441s/order1 88.863s,
all36worlds/9types/4configs/2856scalarchecks perorder inoriginalunfilteredfixture.
Perkind89411 completedall21physicalruns/13940scalarchecks butfinalduplicate-label
metadataFAILED; separate9401finishedonlymissingmetadata. Originalfailedreceipts
andold60swholefixturefailure retained, no newperformancepass. All400Rustcells/
50trace-fixture cells/52non-RainCcells accounted withsourceapplicability explicit.
No ownedGPU/build/viewer jobs orrawoutputs remain. Current2gatesclosed(C,D),4left.

**Retained production source repair:** src/sim.rs now always selects existing
paired normal solving in native builds, independent ofcontactorder. SHA256
6b4aeaf97f94ed856b11ddcf4b0d7b6d0e19d3c8296ec0b3ff46d0f6768d97dd.
contact-policy-sim-candidate.rs is exactverifiedsource; originalbeforeimage
retained for historicalinput audits. Both Cargo manifests restored byte-exact,
empty experimentalfeature absent fromproduction. Broadphasecull remainsartifact
only; productionbroadphase originalf49df7557960f55ce5b64ac518b57e09249bf33aa2ac91b27fbc4d3ad533d747.
Current shared libraries are the contact-policy candidate, compiledwith anempty
isolationfeature; old culltestbinaries remain at their originalpaths andnewpolicy
tests have distinctcompiler-selected names. Never treat a targetpath assource
identity. Exact paths/hashes are in the actual compilerlogs/protocols.

Next priorities:
1. A/B/E remain incomplete: matchedfullRain600 physicalbaseline, Rainresiduals/
   recycling andfrozen60SPS/p95/max targets. Investigationbudgets Rain/performance
   3/3anddrag3/3are spent. No automaticnewexperiment orrepeatofhistoricaltrials.
2. Preservequalifiedsource/developmentarchives forfinalsource/release applicability.
   F/PR06 remainsreopened: old20releaseartifacts do notcontainretainedpolicy.
   Do not refreezea releasewhilefurther requiredRainrepairs wouldchangeit again.
3. Promisinguntried directions (NOT executed/qualified): validatebulkpose-event
   cache toavoidfullhealth O(N²) gettercloning; coupled/articulation/data-movement
   approaches need distinctmechanism review andexplicitnewbudget, nottransfer
   ofunusedcontact-symmetrybudget. Allcredibleapproachesarenot provedexhausted.
4. Thereis no remainingunlaunchedDqualification. Complete reviewofexistingA/F
   evidence beforedeclaringbudgetblocked; do notmanufacturemore passingtests.
   StopafterPR04; no implicitPR07/08campaign orcommit/pushauthorization.

## Investigation budgets and decisions

Per blocker max3×45min,<=2hypotheses and2variants/hypothesis/batch. Freeze exact
builds/runs/watchdogs/progress/storage before execution. Pivot after2unproductive
candidates or1batch without causal/physical/performance gain. Do not repeat
historical exhausted trials or silently renew budgets. Defaulttemporary1GiB,
retained64MiB/free20GiB, bounded/reaped ownedgroups/core0, cleanup both outcomes.
Credible approaches exhausted allows unsuccessful stop; ceiling with promising
unfinished work means honest partial outcome, not success or silent extension.

Rain/performance3/3 consumed,0left. Drag physical investigation3/3 consumed,0left;
its last batch required one finite observer handle-routing correction, recorded
separately without changing physics. No further experiment on these blockers
without explicit newbudget. There is promising unfinished Rain work; do not claim
all credible approaches exhausted. Related original-regression verification is
separately frozen and terminal. Contact-symmetryblocker RESOLVED in1/3batches: existingpairednormal policydecoupled
fromcontactpriority, originalphysicalbounds passbothbackends/modes. Unused2batches
do notrenewRain/drag budgets. Verificationphase114checks completed, no newcandidate.

Rain batch1: normal GPU viewers both hit180s caps even without fullhealth. Headless
ordinary with actualsetters reached407/600 before90s; loadedsteps~400ms while
setters~.0013ms. Viewer cost alone cannot explain stepping. Indirect setter/cache
invalidation not fully falsified (GPUengine-only arm unlaunched). CPU600 botharms
passed finite/valid endpoint with same187d68e48d2ebcef digest. These are not joint/
energy acceptance. Normal viewer records8workers, headless explicitly1worker;
do not present CPU390SPS viewer versus90SPS headless as matched settings.

Rain batch2: originalupstream336steps through firstrecycling, worker1,dt1/60×4,
recycle.05,CPUfirst/bothGPU. All3complete. Loadedfinal20 CPU14.054ms/71.153SPS;
ordinary341.040ms (GPU238.900,broadphase186.325,solver49.174,fetch98.564),
native340.817ms (GPU238.037,broadphase185.881,solver49.451,fetch99.548).
Timestamps exactly identify currentstep. Fetch includes mirror/map+hostCCD/events,
not merely DMA. BothGPUendpoint f177d8f0fa03811c.

Rain batch3 cause/candidate:300humans×14distinct jointpairs=4200 >4096lookupslots,
forcing scan fallback. Broadphase invokes it even for nonoverlapping terrain
pairs. Move pure AABB/material filter rejection before expensive joint test in
hash/static pair discovery; accepted predicate unchanged. Two libraries/two test
builds, two336profiles andfour original joint-filter checks PASS, exactendpoint
bothbackends. ordinary341.040→215.421ms/2.932→4.642SPS(+58.31%), native340.817→
210.914ms/2.934→4.741SPS(+61.59%). Broadphase~186→56ms both. StillFAIL60SPS;
no full600joint/support/energy acceptance. Source restored; artifact retained.
Actualpeaknewdata61,978,088B under explicit1.5GiBbuild-growthexception.

Drag batch1: originalisolated1320 passes independentphysical+originalnumerical
checks. Ground3060 meets support/finite/originalrelease endpoints but new
unconditional.20m targettracking screen fails bothCPU.899216/GPU.919589m. Preserve
that screen and all original loadedtrajectory FAIL results; no GPU-only defect.

Drag batch2: source confirms actualmotor7.5Hz/damping1.0, not historical5Hz
assumption in acceptance.md; drag-acceptance.md corrects it. First>.20m frame232/
t172 during lowering; worst1139/t179 CPUtargety.344374/anchor1.003620/GPU1.029429,
floorupwardimpulses~2990/3249 plusothers. Free-servo model inapplicable under that
load; diagnosis does not by itself pass physical behavior.

Drag batch3: same.20m bound for lifting/constanttarget/free motion; only lowering
(t>=135) with actual upwardnormal.y>.5 and positiveimpulse can be floor-loaded.
Check every violatingframe. Preserve unrestrictedpeak/FAIL, exemptedcounts and
originaltrajectoryFAIL. The printed free_peak is an upperbound on unexempted
tracking (may include floorloadedframes below.20, which need no exception).
Allfinite/no-runaway/support-.025/release<.1 checks stillapply; per-held-step raw
mousebody/joint validity. First observer abort wrongly used combined mapping for
an internalGPU-onlyjoint; corrected to actualgpu_b3_joint_is_valid, failedprefix
retained. All8scene cells+8CCDchecks PASS; original windows/no ground resets.
Worst groundCPUcorner-.022358m/GPU-.018491m, within original.025 screen. Source
scene libraries are exact frozenPR06; focusedRustCCD binaries include cull change,
not finaldelivery tests. Full29CCD is D, not claimed by these2selectors alone.

Timing is combinedCPU+GPU completed-step walltime with health observations
excluded, not pure GPUkernel. Ordinary~511–546SPS but groundmax.67–.73sFAIL250ms;
nativeisolated47–48SPS/max1.49s, ground~100SPS/max2.13s. These new measured hitches
remain unresolved E failures; do not hide first-use work inside a timing pass.

## Cleanup, evidence and limitations

Prior phase receipts (FAILED never rewritten as PASS):

| Phase | Result | Elapsed | Last job | Deleted temporary bytes |
| --- | --- | --- | --- | ---: |
| baseline | FAILED | 203.027s | run-rain-ordinary | 3123517 |
| baseline-native-remaining | FAILED | 180.559s | run-rain-native | 286 |
| rain-batch1-headless | FAILED | 105.908s | run-ordinary-viewer-setters | 16811128 |
| rain-batch2-profile | PASS | 130.716s | caches-after | 33650888 |
| rain-batch3-cull | PASS | 268.848s | sizes-after | 32574880 |
| drag-batch1-physical | FAILED | 12.994s | ordinary-order0-ground | 33530248 |
| drag-batch2-tracking | PASS | 9.634s | caches-after | 33535992 |
| drag-batch3-qualified | FAILED | 4.094s | ordinary-order0-isolated | 33532112 |
| drag-batch3-handle-correction | PASS | 152.228s | caches-after | 68870912 |

Prior phases removed255629963bytes; raw was empty beforeD. All recorded prior
PIDs/groups absent and141+distinct input/log/protocol pins matched checkpoint-audit
beforeD launch; rerun after terminal. No raw binaries/objects/cachecopies/fullscene
dumps retained. Currentraw is empty; all owned testtemporary directories were consumed/deleted. Keep
small protocol/source/failedlogs and summaries, not consumed artifacts. Shared
libraries/tests/cache nextconsumer=qualification; no releasecandidate overwritten.

Important records: acceptance.md (frozen targets/classification, correctedmouse
frequency via drag-acceptance.md), rain-batch2-profile-summary.json,
rain-batch3-cull-comparison.json, drag-physical-summary.json,
drag-tracking-applicability.md, checkpoint-audit.json. Numeric performance targets:
60completedSPS/1simulatedsps, p95<=33.333ms,max<=250ms, originalscene/defaults;
Rainfull600+loadedtail120, ragdollactivefirst240 (full600also reported), dragoriginal
1320/3060. Never lower after results. Physical tolerances remain original.

Historical rejections: angular/substep refresh, extra waves and combinations,
coupled cone/point andhinge blocks trade cone improvements for anchor/twist failures;
passivewarmcache line search/motorprojection alone insufficient; mesh-unilateral
fallback left actual18anchor/27angular/249cone failures unchanged. Read later
coupledCPU prototypes before proposing another coupled algorithm. No exhausted
historical campaign resumed. Rain physical failures remain, oldstrict ragdoll
trajectories deferred separately from its historicalphysicalpass.

Relatedcheckphase CLOSED/FAILED ordinaryorder1physical:7pass1fail contact-islandwake spin.019269153>.001 andtransverse.001521538>.001. Ordinaryorder0 all65checks pass. Sourceonlycullcandidate, no productionpromotion. Distinctremainingphase FROZEN beforelaunch: only8unlaunchedgroups/187checks,120s each/1010sphase,0builds/candidates/retries; predictednativeorder1same1failureexplicitlyretained, statusCOMPLETED_WITH_RETAINED_FAILURESnotPASS. No completedcellrepeated;originalfailedphysicallimits unchanged. Drivercleanedtesttempfiles/PIDsabsent audited; 1/6gatesclosed. RemainingphysicalfailurejoinsRain/performance asunresolved; nottrajectorywaiver.

Relatedremainingverification CLOSED completedwithretainedfailures:258/260originalchecksPASS, twooriginalcontact-island spin/transversefailures onordinary/nativeorder1. Sourcecausalboundary sim.rs diagnostic_flags_from_env couplesSOLVER_PAIRED_NORMALS toorder0evenwhenpriorityitselfisindependent. Mode0original8physicalchecks passbothbackends;mode1fails oneonboth. Newdistinctcontact-symmetryblocker batch1/3 FROZEN contact-policy-batch1-protocol.json,1algorithm-policycandidate alwaysuseexistingpairednormals, no physicalparameterchange,0Rainruns/budgetrenewal.2libs+2testbuild180/2CPP15/2link25/4physicalsuite90=32checks/2original3060ground60/cache15/1350sphase,default1GiBtemporary+sharedgrowth/raw128MiB/retained64MiB/free20GiB. Temporaryemptyfeature isolates Cargoartifacthashes so priorpinnedtestbinariesarenotoverwritten;no cfg consumer,manifests+simrestorefinally. Contactsymmetry1/3allocated,2left;Rain3/3spent,drag3/3spent;1/6gatesclosed.

Contactpolicy followupverification FROZEN beforelaunch: contact-policy-order1-verification-protocol.json, onlychangedorder1 CCD29+precision28 perbackend=114checks,4runs120/cache10/510sphase,0builds/newhypotheses/candidates/retries/Rainruns,defaultstoragecaps. Newfeatureisolatedactualcompilerbinariespinned. Do notlaunchuntil86254terminalandrestorationcomplete. This verifies candidate applicability, notanotherphysicsattempt; unchangedmode0old65checks andnew32physicalcontrolchecks remainseparateevidence.

## Retained contact-policy repair result

contact-policy-acceptance.json verifies actual retainedsource and source-identity
transition. Before: originalcontact-island mode1 spin.019269153rad/s vs.001 and
transverse.001521538m vs.001 onbothbackends, all other originalchecks pass.
Firstcause: diagnostic_flags_from_env tiedSOLVER_PAIRED_NORMALS toorder0. Repair
alwaysselects existingpairednormalalgorithm; priority/world/body/material/joint
parameters andWASM initializer unchanged. No constant/iteration sweep.
Candidatebatch1 all17jobs PASS265.247s;32originalphysicalchecks allpassbackend×
order0/1, including oldsymmetrybounds; twochangedmode1ground3060confirmationsPASS.
Postcandidate4groups all114originalorder1CCD/numericchecks PASS atoldlimits.
Mode0normalpolicy unchanged; its previousCCD/numeric controls retain source
applicability withpurebroadphasepredicate equivalence, while newphysicalcontrols
also exercisedmode0. FullCfixture/source reconciliation andB remainOPEN.

source/srcsim andbothmanifests restored aftertrial, then onlyverifiedsim change
retained (normalpolicy); 125,807,400Bpeaknewbuilddata. No newproductionfeature.
Anemptyfeatureisolatedbuildartifactnames so priorpinnedtestexecutableswerenot
overwritten. Sharednewcandidate artifacts nextconsumer=remainingphysical
qualification/PR06applicability, notclaimedfinaldelivery. docs/gpu-physics.md and
qualification/roadmap updated. PR06 current-source checkbox REOPENED; historic
442689de1 buildmilestone and20artifacts preserved. No other roadmap item reopened.
policy-scene-dispositions.tsv lists all165native registrations as current-policy
UNREVIEWED individually; no extrafullreviewcampaign. Sharedfixture evidence is
not165newbehaviorpasses. Current1/6gatesclosed,5left, goalACTIVE, no newcommit/push.

Continuation: previous goal turn was PROGRESS (retainedverifiedpolicyrepair,1/6closed). No livejobs found. JointCqualification FROZEN: joint-c-qualification-protocol.json, originalninekindreaction/separation CPUrealprefix +ordinary/native mode1;2CPP15/4links25/4runs60/cache10/freezeverify15/450sphase,0enginebuilds/retries/captures. Default1GiBtemp/64MiBdiag/raw128MiB/free20GiB; explicit196MiBretainedbinaryexception foronlytwo currentpolicy libraries, ignoredartifacts directory; consumers=C/mesh/articulatedqualification+PR06refreeze. Old20PR06artifacts preserved; rootmutablebuildoutputs consumed withfrozenhashes, immutablecandidatecopiesverifiedbefore/after. No Rain/dragbudgetrenewal.1/6closed5left.

JointCfirstphase terminalFAILED dueharnessmarkeronly: ordinaryreaction code0/full2856sourceassert comparisonsPASS, butdriver incorrectlyrequireduppercasePASS. Markerreconciliation preservesfailedreceipt/no physicsfailure. joint-c-remaining protocolFROZEN only3unlaunchedfixtures withexactoriginal2856reaction/2686separationmarkers (separationcount=(16poses+3mutations)*8configs+6dynamic/sleepinspections perkind;8kinds2fields+wheel1field). Recompile2cleanedCPPobjects15s each,3links25/3runs60/cache10/350sphase;0enginebuilds/retries/newcandidate. Alreadycompletedordinaryreactionnotrepeated. Copiesfrozen87,507,106+94,830,834B; unchangedsource/Box3D/WASM.1/6closed.

## Publication checkpoint requested by user, 2026-10-17

User explicitly requests commit/push and reusable PR04 goal text emphasizing
expected physical behavior, completed-step performance, innovative pivots and
permission to stop unsuccessfully. This is checkpoint publication, not PR04
completion or renewal of Rain/performance3/3 or drag3/3 exhausted budgets.

Joint C remaining phase40257 terminated FAILED at65.352s. Ordinary separation
completed2686 unchanged checks; previously completed ordinary reaction2856 is
reused, not rerun. Native reaction reached60.136s and was killed by its frozen
watchdog, without a final comparison marker; preserve this as timeout/incomplete,
not a demonstrated incorrect reaction. Native separation never launched. Raw
cleanup68,829,048B recorded; exact receipt/logs retained. Earlier marker-only
harness failure remains reconciled separately.

qualification-fixture-tools.ts is a prepared, unexecuted Bun adapter. Its
mesh-ragdoll protocol does not yet exist; it establishes no fixture pass and
must be reviewed with a finite protocol before use. No mesh/ragdoll campaign
launched during publication. Gate status stays1done/5left; PR04 unchecked and
PR06 applicability reopened.

Publication-only budget frozen before execution:0 engine builds/GPU runs/captures,
1 offline checkpoint audit25s,1 syntax scan of local TS tools25s, Git diff/staging
checks, fetch45s (completed0ahead/0behind), commit120s and push60s. No retries of
physics/timeouts, no new hypothesis budget. Keep existing196MiB exception for the
two named development libraries; no extra binaries/caches. Audit will verify
source transitions, receipt integrity, recorded process groups absent and raw0.

Publication verification PASS: checkpoint-audit.json verifies179 distinct pinned
inputs (one explicit historical-to-retained sim source transition), all recorded
process groups absent, raw0, approximately4.03MB retained diagnostic data and
1.70TB free. Sixteen TypeScript tools transpile; this is syntax-only, including
the unexecuted adapter. git diff --check passes; Box3D submodule clean and no WASM
paths changed. Fetch confirms0ahead/0behind before checkpoint commit. Publishing
under title `fix(gpu): decouple contact normals from ordering policy`; resolve its
commit hash with Git after publication. No gate or budget status changed.
Staged source/docs whitespace check passes. Verbatim receipt-pinned test logs
contain original trailing blank lines; preserve those bytes/hashes rather than
rewrite evidence for a cosmetic whitespace check. The checkpoint publishes only
small diagnostics/source fixtures, not the ignored development libraries.

## Continuation after published checkpoint8d18dc8b6

Previous turn PROGRESS: committed/pushed retained repair and unfavorable evidence;
Git verified clean/0ahead0behind. No new commit/push authorization inferred.
Goal remains1/6closed5left. The earlier publication authorization is consumed.

Affected-source qualification FROZEN before launch:
mesh-ragdoll-qualification-protocol.json,55jobs/25input pins. OriginalCPP fixtures,
3sharedC compiles+2CPP15s,6links25s,8actualCPU reference runs30s first,32GPU cells
(ordinary/native×order0/1×8modes); fullragdoll60090s, others30s;2libraryverify15s,
caches10s/1750sphase.0Rust builds/newalgorithms/Rain experiments/captures/retries.
Stopfirst failure. Original isolated4×120,noContacts60,grid45/torus12 andfull600
physical bounds unchanged; old chaotictrajectory screens deferred explicitly.
Bun adapter preserves compactCPU reference inputs (<2MiB budget) for all pending
GPU comparisons rather than fullscene dumps; consumers=unlaunched GPU cells,
delete afterconsumption. Raw128MiB/retained64MiB/default1GiB including drivercache
/free20GiB, prior196MiB two-library exception unchanged. Allobjects/exes deleted
on success/failure. No native-reaction retry or budgetrenewal; native-separation
stillunlaunched. Necessary qualification is distinct from exhausted investigations.

Affected-source qualification83664 CLOSED/PASS:55/55jobs,40/40CPU/GPU cells.
Original fullragdoll600 physical checks, exacttyped112jointsetup, firststep and
frame2contact calculations passbothbackends/order0&1 afteractualCPUfirst. All
4isolatedspherical120/noContacts60/frozenmesh45grid+12torus pass unchanged limits.
See mesh-ragdoll-qualification-summary.json and40compactqualification-results.
Objects/exes cleaned; compactCPU references consumed byall32GPUcells anddeleted
withsize manifest. No Rain600 qualification or acceptedperformance claim.

Originalunlaunched nativejointseparation FROZEN separately:6jobs compile15/link25
/run60/verify15/cache10/145sphase,0enginebuilds/retries/captures/newalgorithms.
Exact2686sourcechecks/unchangedlimits. No repetitionof native-reactiontimeout;
oldfailedreceipt staysfailed. Defaultstoragecaps/cleanup unchanged.1done5left.

mesh/ragdoll qualification actual82.457s; peakGPUanchor.116392836m<.25 andtail
.00298925536m<.005; all GPUtailvelocities0. CPUpeak.213310137/tail.00139051978.
Sharedcachegrowth0;68,840,992Braw+461,743BconsumedCPUrefs deleted. Summary written
before anempty-directory BunrmSync EFAULT; verifiedallreferencefilesremoved and
emptydirectoryremoved via rmdirSync. Cleanupreconciliation explicit, no engine
rerun/data loss. Finalhelpercorrected directoryprimitive, no qualificationlimit
changed. joint-native-separation74373 CLOSED/PASS6/6jobs,2686checks; native
reaction remains incompleteatits60s ceiling, not retried.

RemainingRustqualification FROZEN beforelaunch:36 original selectors minus65
alreadycovered; exactrequiredapplicability yields34/35ordinary+35/36native=140
checks. Four120s groups/cache10s/520phase,0Rustbuilds/candidates/Rain/drag
experiments/retries/captures. Alloriginalsourceassertions, no testrewrites.
TMPDIRowned/raw128MiB and consumedaftereachgroup, default1GiB/64MiB/free20GiB;
currentcompiler-selectedbinaries pinned. No source/policychange.1done5left.
Live remainingRust session91770: check exacthandle/receiptPID, no restart on
observation timeout. Originalmeshgrid/torus input SHA matchesPR03baseline retained
rawcopies exactly (7bbaf3de… /3abbc014…); no new substituteinput/historicalpass.

Distance qualification FROZEN (launchonlyafter91770terminal): originalunchanged
CPU-prefixed fixture copiedfrom originalOct2reference,4cases240steps each,
13lanes1e-5/warmstarttoggles20/60/100/160 and1/4substeps.1CPP15/2links25/4runs60
/verify15/cache10/350sphase,0Rustbuilds/candidates/retries/captures. ActualCPUfirst
insideeachcombinedstep. Defaultstoragecaps/objects+2exescleanupbothoutcomes.

Distance50395 CLOSED/PASS10/10jobs:16original240step CPU/GPU comparisoncases
(4cases×2backends×2orders),13lanes1e-5 andoriginalwarmstartassertions unchanged.
Objects/executablesconsumed. Frozen-input meshhashesalsoexactoriginalPR03copies.

RemainingRust91770 CLOSED/FAILED atoriginal120saggregategroupcap, after18explicit
`ok`results, while19th creationidentitytest stillrunning. No assertionfailure;
allotherconfigurationsunlaunched. remaining-rust-partial-disposition.json pins
exactlog/18completed/1incomplete/15unattempted. No retryofincompletecase.

Distinctunlaunched-only verification FROZEN: rust-unlaunched-qualification,
121previouslyunattempted selectors (17groups<=8 originalchecks/120s,2cache10s,
2080sphase).18completed reused;1interrupted creationidentity remainsincomplete.
0builds/candidates/retries/Raininvestigations. Smaller groups prevent aggregation
fromconsuming awholeconfiguration's ceiling, without extending interruptedtest.
Alloriginalassertions andbackend/order applicability preserved. Default1GiBtemp
inclsharedcache/raw128MiB/64MiBretained/free20GiB, ownedTMPDIRconsumedaftergroup.
Runningexactsession20101 (rust-unlaunched qualification): atlastverified snapshot,
ordinary0two groups andordinary1five groups passed;native0firstgroup is live.
Do notrestart on observationtimeouts; poll exacthandle/receiptPID. Original
ordinary0incompletecreationidentity remainsnotretried. All165scene dispositions
retained; onlyDeterminism/FallingRagdolls upgraded to physicalfixtureverified
forcurrentpolicy, explicitlynot anewviewer/visual/performancepass. Other164
remainindividuallyunreviewedforcurrentpolicy.

UnlaunchedRust20101 CLOSED/PASS19/19jobs,121checks/324.974s; alltemporarytest
outputsconsumed. Matrix400requiredcells:285currentbuildPASS/114explicitreused
mode0CCD-numeric/predicateequivalence/1incompleteordinary0identity;4applicability
exclusionsretained. No testassertionfailureincompletedprefix/continuation.

One corrected standaloneverification FROZEN: identity-standalone-qualification,
only originalunfinishedordinary0creationidentity,30s once/cache10s/60sphase.
Evidence distinguishes aggregatecap exhausted by18earlier tests fromanindividual
fixturetimeout. Previous17groups remainedunlaunched-only; no oldreceipt rewrites.
This distinctcheck corrects batching, not a solver/performance hypothesis or a
renewalofRain/drag investigation.0builds/candidates/captures/furtherattempts;
originalassertions unchanged/defaultstoragecaps/ownedtracecleanupbothoutcomes.

Standaloneidentity58200 CLOSED/PASS1.299s/fullsourceassertions. Originalaggregate
FAILED receipt remainsunchanged; matrixnow286currentpasses/114explicitreuse/
4applicabilityexclusions across400requiredcells, withhistory evidence preserved.
Continuationqualification.json inventories all6phases. Taskownedtemporarydata
173,420,479BdeletedincludingconsumedCPUreferences; observedsharedcachegrowth
~2.12MB, cachespreserved. No newproduction/Box3D/WASM edits or releasebuild.

Finalofflinebudget:1checkpointintegrityaudit25s,1syntax/sourcewhitespacecheck25s,
0GPU/build/capture/retries. Nextsafeaction afterhandoff is source-only remainingC
reaction/applicabilityaudit; no renewing spentRain/performance3/3 ordrag3/3.
Thisgoalturn is PROGRESS: current600step articulation proof andoriginalregression
coverage, notonlyplans. Gatecount1done5left; no newcommit/push authorized.
Final integrity audit52385 PASS:207distinctpinnedinputs,1explicitretainedsource
transition, allrecordedownedprocessgroupsabsent,raw0,~4.98MBretaineddiagnosticdata,
~1.706TBfree. All23TS tools transpile andsource/docs whitespacecheckpasses;
Box3Dclean, no Rust/WGSL/bridge/WASM productiondiff thiscontinuation. Twofrozen
currentlibraries retain196MiB exception forsourceapplicability/audit/PR06refreeze.
No jobsleft andno additionalexperimentbudgetallocated. FullPR04notachieved;
goalremainsACTIVE andPR04unchecked. Budgetceilingisnot allideasexhausted: source
applicability reconciliation cancontinue; furtherRain/performance experimentation
needs explicitnewbudget. Do not resume orrerun historicaltimedoutjobs implicitly.

## C-fixture source audit continuation

Previousgoalturn PROGRESS:400requiredRustcells accounted/40CPU-GPUmesh-ragdoll
cells/otherCverification; no livejobs. Historicalbaseline confirms originalnative
reactionPASS1.720s atoldsource/order1, butthisalone isnot current-source proof.
Sourceaudit: retainedpolicy bit1<<18 onlyconsumed bysolve_contact; reactionfixture
hasone sphere shape/no collisionpartner andsupports existingGPU_REACTION_KIND.
Its finalnonzero-force/torquechecks honor thatselector. Perkindcounts derivefrom
13inspect×6lanes×4configs=312; distance adds2inspect/config=48, total360;
360+8×312=2856 unchangedchecks. Thisallows bounded independentqualification of
alloriginal36world/configcases withouta60s aggregateprocesslimit.

FROZEN beforeexecution: joint-kind-qualification-protocol.json,31jobs,2CPP15s,
4links25s,18nativekindruns20s (bothorders),3unlaunchedwholefixtures30s
(ordinaryorder0reaction/separation,nativeorder0separation),2verify15/cache10s/
650sphase. Total13940neworiginalscalarcomparisons; currentphysicalCPUfirstvia
combinedbridge andunchangedassertions.0Rustbuilds/newsolver/capture/performance
hypotheses/RainorDragrenewal/automaticretries. Stopfirstfailure. Reconciles one
incompletefixture andnever-runorderingcases; oldnative60stimeoutstaysFAILED.
Default1GiBtemporaryinclcachegrowth/raw128MiB/64MiBretained/free20GiB; deleteeach
fixtureexe afterlastconsumer (rawpeakone~35MB),objectsfinallyonbothoutcomes.
Oldtwo-library196MiBexceptionunchanged. No stage/commit/push authorization.

Ckind89411 TERMINAL/FAILED due duplicate finalverification label/output collision;
all29launchedjobs/21physicalruns PASS,13940originalscalarcomparisonscomplete.
Do not rewritefailedreceipt/protocol/helper pins. Missingfinal2metadatajobs ONLY
frozenas joint-kind-final-audit (verify15/cache10/35s,0test/build/capture/retry).
Batchedlabelerror isautomationonly; source/productassertions allpassed.
Observednativeperkind4worlds~10.7–11.1s, all9types/order0sum97.924s; original
36world60scapinsufficientisnow a measured explanation, not anincorrectjoint.
Decompositiondoesnot byitself qualify sharedprocess/crosskind lifecycle: whole
unfilteredconfirmation must stillpass beforeclosing Cscope. Plan2×120s native
order0/1wholefixture basedon observed~98s, no claimofspeedpass/coldstartupfix.
Finalmetadata9401 PASS2/2; libraryhashes intact/cacheaftercollected. Wholefixture
confirmation FROZEN as joint-whole-confirmation-protocol.json with8UNIQUElabels,
observed9kind sums97.924/97.832s.1CPP15/link25/2nativewhole120/2verify15/cache10/
350sphase;0Rustbuilds/candidates/captures/furtherattempts. Unfilteredoriginal
CPP36worlds/9types/4configs,2856checks perorder includescrosskind/sharedprocess
lifecycle. Previous60sfullrun remainsFAILED; no targetlowered orperformancepass.
Boundedfullconfirmation distinctfromperkinddiagnosis, no Rain/dragbudgetrenewal.
Defaultstoragecaps; delete1object1exeafterbothconsumersoronfailure.

Wholeconfirmation86414 CLOSED/PASS8/8jobs: actualnativeorder0 89.441s/order1
88.863s,2856unchangedscalarchecks each inoriginal36world/sharedprocessfixture.
Old60sFAILED receipt unchanged. All9jointkindssucceededindividuallyat~11s each;
thatcost justifiedfinite120sfullconfirmation, not a relaxedphysical/performance
limit. Perkindpassisdiagnostic; fullunfilteredpasspreserves lifecycle scope.

RelatedgateD CLOSED: related-qualification.json;52non-RainCcells46currentpasses/
6explicitreuse,50trace-fixturecells48currentpasses/2explicitreuse,400requiredRust
cells286currentpasses/114explicitreuse,4originalapplicabilityexclusions. The
originaltracefunctions write/validate/consume temporarysemanticstates during
assertions; archivalcopyoptional, no new5process/futurestateclaim. Rain4Ccells
remainB, notsilentlywaived. All165scene dispositions retained. Now2done4left.

ThisgoalturnPROGRESS: Dclosed byactualremainingCresults/sourceaudit; A/B/E/F
remainincomplete. No newproductioncode, no Rain/drag/performancebudgetrenewal.
Finalofflinebudget1checkpointaudit25s+1syntax/whitespace/cache/cleanupsummary25s,
0furtherGPU/build/capture/retries. No additionalcommit/pushauthorized.
Finalaudit70511 PASS:211distinctpinnedinputs, retainedsource-transitionverified,
allrecordedprocessgroupsabsent/raw0.26TS tools transpile; source/docs whitespace
checkPASS; no Rust/WGSL/Box3D/bridge/WASM productiondiff thisturn. Newrelated
qualification removed172,953,728Btemporaryobjects/executables; peakraw35.36MB,
observedcachegrowth10,583B (sharedcachespreserved). Retaineddiagnostics~5.39MB,
free~1.706TB, existing196MiBtwo-library exception unchanged. Related-checkpoint.json
recordsmeasurementsandlimitations. No stage/commit/push andno ownedjobsleft.
CurrentturnPROGRESS: gateDclosed;2done4left(A,B,E,F). Nextsafeaction is reviewof
existingA/Facceptance/releaseapplicability, notnew solver/performance trials.
Rain/performance3/3spent0left/drag3/3spent0left; promisingunfinishedapproaches
arenot provedexhausted. FullPR04unchecked, appgoalACTIVE. No blockedstatusclaimed:
thisturnmadeconcreteprogressandthereisstillfiniteindependentauditwork.

## Remaining A/F review continuation

PreviousgoalturnPROGRESS: relatedgateDclosed;2done4left. Thisread-onlyreview has
0builds/GPUruns/captures/newhypothesesexecuted. Frozenofflinebudget:1release/
baselineaudit25s and1finalintegrityaudit25s; outputscompactsource/artifactstatus
andremaining-requirementmatrix <=1MiB; no temporarybinaries/cachecopies.

Freshsourceaudit falsifies anoldrecovery lead: with_world calls
ensure_cpu_mirror_world, which gates synchronization on gpu_mirror_stale,
events_pending orpost_ccd_pending. Cleangetters skip sync_world_mirror entirely.
The arraycollect in sync_world_mirror_parts' cleanfastpath isnot anunconditional
per-gettercopy. callback_shape_reads::pending alsogates snapshotcapture on
stepping/postCCD plusregisteredcallbacks. Do not implement orbenchmarkthe
previously proposedclean-getterclonefix. Thiscorrects sourceattribution, not
proofthatallhealth/querycostsarecheap. Sourcepins/evidence recordedinaudit.
