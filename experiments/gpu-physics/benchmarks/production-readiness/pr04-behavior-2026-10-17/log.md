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

App goal is ACTIVE. Last publication6cd6956a7 was explicitly requested and pushed;
the latest user request explicitly authorizes committing and pushing this checkpoint.
Previous turn was PROGRESS: verified/published checkpoint. This continuation has
causal profiles, a tested artifact candidate and a closed physical dragging gate.

## Current status — 1 done / 5 left

| Gate | Status and evidence |
| --- | --- |
| A acceptance/baseline | OPEN. acceptance.md classifies original101Rust/13trace/14C recipes and freezes targets. Matched headless336 CPU/ordinary/native timing exists; full current Rain600 physical baseline and source-applicability audit unfinished |
| B Rain/articulation | OPEN. No current GPU600 physical pass. Historical severe/persistent joint residuals remain; no solver repair |
| C dragging/CCD | CLOSED at current physical scope: drag-physical-summary.json;8scene cells,17,520combinedsteps,175,200independentbody observations,8focused landing/mutation checks. Bothbackends/order0&1, original1320/3060windows. Limits/applicability below; broader29CCD belongsD |
| D related regressions | OPEN. Originalrelatedchecks258/260pass exposed2symmetry failures; verifiedcontact-policyrepair nowpasses32physical+114changed-modeCCD/numeric. RemainingoriginalC/sourceapplicability notclosed |
| E throughput/latency | OPEN/FAIL. Rain2.9SPS baseline/candidate4.6–4.7 vs60target; native isolateddrag47–48SPS and large hitches; do not present physics pass as speed pass |
| F reconcile/cleanup/refreeze | OPEN. All prior phases cleaned/audited; runningD stillneedsclosure. Contact-policy repair retained; frozenPR06 oldsource untouched and applicability reopened |

Baseline runtime442689de1, current publishedHEAD6cd6956a7. Broadphase shader was restored byte-exact after its experiment; that cullcandidate
remains unpromoted. The later contact-policy source repair is retained; shared
libraries now contain it and testpaths mustbeidentified by compilerreceipt/hash.
Frozen PR06 release candidate remains20files/367,035,504bytes under
experiments/gpu-physics/artifacts/pr05-pr06-release-candidate-2026-10-17. Consumer:
focused qualification and PR06 applicability/refreeze. Preserve it and sharedcaches.

## Current process/source state and next actions

All sessions are TERMINAL. Last candidate86254 PASS; post-candidate41602 PASS.
Joint remaining40257 FAILED: ordinary separation passed all2686 checks; native
reaction hit its60s watchdog (SIGKILL) without a final comparison marker. This is
an incomplete qualification, not an established physics assertion failure.
Native separation was not launched. No automatic retry or further scene run.
Prior related12960 FAILED and remaining61155 completed with retained failures.
No liveGPU/build/viewer jobs, raw directory empty. Audit again after source/docs
updates; do not launch from any oldreceipt/handle or reuse a spent budget.

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
1. Finish gateA/D source/C-fixture applicability, especially current-policy
   ragdoll600 and original frozenmesh/joint fixtures before claiming allDclosed.
   Reuse checks whose code/input consumers are unchanged; no newcatalogsweep.
2. Rain/performance budgets are spent. Any further investigation needs explicit
   newbudget, nottransfer ofunusedcontact-symmetrybudget. Necessary affected-scene
   qualification must be separately frozen and clearly distinguished from a new
   solver/performance hypothesis. No silentextension or old900shealth rerun.
3. Useful source-only qualification-adapter lead: original fullhealth repeatedly
   calls bodygetters; with_world unconditionally drops sync_world_mirror's returned
   Vec, whose clean fastpath clones everybody evenwhen already current. This
   explains a potentialO(N²) diagnostic readpath, notyetmeasured as a newrepair.
   Existing b3World_GetBodyEvents provides bulkawake/fell-asleep transforms; a
   validated creation/generation-awareposecache plus bulk world_health_scan may
   qualify Rain joint anchors/cone/twist without everybodygetter eachstep. Validate
   eventcache versusactualgetters andexactjointmath first; do not invent a pass
   fromfinite-only endpointdigests. No suchadapter or newgetterchange exists yet.
4. E stillFAIL Rain60SPS and native topology/first-usehitches. F/PR06 refreeze
   reopened for newsource; do not refreeze a release that will immediatelychange
   again. StopafterPR04, no implicitPR07/08campaign. Current checkpoint commit/push
   is explicitly authorized; future implementation commits still need authorization.

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
