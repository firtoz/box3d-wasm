# GPU solver correctness and repeatability qualification

Current2026-10-02 PR03 [fixture baseline](../experiments/gpu-physics/benchmarks/production-readiness/pr03-baseline-fixtures-2026-10-02/README.md): both current backends retain the original distance first-step discrepancy; unchanged600step ragdoll physical/isolated/57posemesh screens pass. Original strict ragdoll trajectory screen remains failed (0.063585767m versus0.006m), retained separately.26first-process observations are not final five-repeat/full-state/build qualification; no timing/source/default/tolerance change. Full current matrix/baseline remains open in the production roadmap.

This qualification uses efficient GPU ordering (`GPU_PHYSICS_LIVE_CONTACT_ORDER=0`).
CPU-compatible ordering remains available for reference work. The default must
not switch until the GPU-order qualification below passes. CPU trajectory
agreement is not a general correctness requirement for chaotic scenes.

Repeatability means identical semantic simulation state at every step across at
least five fresh processes using the same binary, input, configuration, device,
driver, backend and timestep/substeps. Ordinary and native cached configurations
are qualified separately. Cross-device, cross-driver and cross-build agreement
is not promised. Byte-identical pose traces alone do not establish full-state
repeatability.

Current production queue: [readiness roadmap](goals/gpu-production-readiness.md).
On2026-10-02 the named PR01/PR02 functional controls/API contracts pass on retained
source `abc0a54`, with [current API evidence](../experiments/gpu-physics/benchmarks/production-readiness/pr02-current-acceptance-2026-10-02/README.md).
[PR03 contract preparation](../experiments/gpu-physics/benchmarks/production-readiness/pr03-contract-baseline-2026-10-02/README.md)
records exact current selectors/source identities and unresolved original
physics/state/performance criteria; PR03 stays open pending a concrete executable
baseline and frozen evaluation budget. No new baseline simulation or timing has
run. The following older candidate/job notes are historical evidence, not current
process state or final-build qualification. Rain and loaded-dragging failures,
the distance first-step discrepancy and complete future-state gates remain open.

## Coverage and current evidence

Latest desktop candidate: `desktop-baseline/paired-v22-candidate/manifest.json`
freezes gated paired-contact libraries, sample executables and test executables.
Library builds69996/48507 and sample links57470/98633 completed0. Test hashes
match the earlier gated contact-island/friction/support five-repeat evidence.
The selected persistent-state consumer audit is complete for public World
completed-step boundaries and selected configurations, including v22 retirement
candidates and captured paired-mode policy; broader qualification remains open.
Older rows below retain historical evidence and do not imply final-build passes.

The current evidence cross-check is `paired-v22-candidate/coverage-audit.json`:
all99Rust/WGSL source hashes match the frozen candidate;25five-run case/path
groups have the expected durations, successful individual receipts and passing
comparisons, and Rust fixtures match the frozen binary hashes. Ragdoll raw
append-order failures remain separately recorded. The66fixed regression passes
per path are also verified. This consolidates existing bounded evidence; it
does not close ground dragging, Rain, performance or delivery requirements.

Paired restitution driver63634 completed0: five fresh240step runs per backend
pass all original assertions and exact raw v22comparison. Both paths report
first rebound apices0.99096805/2.488538m, peak energy25.017302J/kg and peak
penetration0.004999995m. Evidence: `desktop-baseline/paired-restitution-five/`.
Baseline49.31mm penetration failures remain preserved.

Follow-up driver19696 completes0: motor reversal120steps, analytical mass
mutations18steps and joint-island waking73steps each pass five fresh processes
per backend with all original physical assertions and exact raw v22comparison.
Evidence: `desktop-baseline/paired-{motors,mass,joint-island}-five/` and frozen
candidate `followup-exits.json`. All40processes across these and restitution also
pass every-step completion, sticky-loss and invalid-state checks. Policy checks
confirm paired solving plus native bounded-sort where selected; geometry-derived
static proof bits are retained exactly in comparisons. The initial assumption
that flags contained only the environment-selected bits was rejected and corrected
using `GpuSim::set_topology_bounds`; that was a validation assumption, not a solver
failure. Details: `paired-v22-candidate/completion-policy-validation.json`.

Paired Rain600step health captures26826ordinary/77334native are running under
`desktop-baseline/rain-paired-health/`; both fixture hashes match the frozen
candidate. These are single health runs, not per-step semantic repeats or timing
measurements. Separate frozen pre-paired v22 Rain repeats63604/15392 remain live.

The same frozen paired test binaries now pass lifecycle driver51035: five fresh
history16/query8/callback12 captures per path and five native replay48captures.
All seven raw comparisons pass; all35processes pass every-step completion,
failure-state and paired-policy checks. Every native replay log records42actual
hits. Evidence: `desktop-baseline/paired-{history,query,callbacks,replay}-five/`
and `paired-v22-candidate/lifecycle-completion-validation.json`.

CCD driver40428 also finishes0: five5step mutation runs/path pass unchanged
fast-body barrier/reference checks and raw v22 equality; all ten traces pass
completion and policy validation. Regression55676 completes0 with66test passes
per path:28numerical,27CCD, five analytical mass/inertia, one sleep-to-wake and
five fresh unread mesh-root lifetime runs. Those unread controls intentionally
avoid state capture, whose harvesting would change the scenario. Evidence:
`paired-ccd-five/` and `paired-v22-candidate/fixed-regressions-{ordinary,native}/`.
These are bounded selected checks, not universal CCD or lifecycle qualification.

New Falling Ragdolls fixtures link frozen paired libraries (builds20041/24630
exit0). Single600step physical/state checks12785ordinary/35462native finish0
under `desktop-baseline/ragdoll-paired-candidate/`. Both pass original limits:
peak separation0.213310137m, tail0.00127359747m, tail speed/angular0 and peak
angular103.627171rad/s within the CPU-relative limit. Five-repeat campaigns
16267/36170 now run under `ragdoll-paired-five/`; complete comparisons pending.

Dragging fixture builds85740/12347 finish0 using frozen paired libraries; exact
CPU/bridge/input hashes are in `desktop-baseline/drag-paired-candidate/`.
Single original --ground3060step physical/state checks44226/42316 fail on both
paths. Ordinary maxposition0.3899m/chord0.351633 and native0.38982m/0.351611 exceed
original limits; held position/velocity also fail, settled velocity remains0.
No dragging repeats were launched after this physical failure.

Saved paired traces were additionally measured geometrically using the unit
cube's rotated corners against floor top y=0. Both contain3060 completed frames
and50 cube/release endpoints; all endpoints satisfy the original absolute
speed/angular-speed <0.1 limits. Minimum corner clearance is -0.044598510m
ordinary and -0.044598563m native, both body3 at core frame527 during the second
held drag. This is descriptive evidence, not a newly chosen acceptance limit or
a replacement for failed trajectory checks. It identifies loaded-contact
penetration as the next physical investigation. Evidence and reproducible
measurement: `drag-paired-candidate/{clearance-settling.json,measure-clearance.py}`
under the desktop baseline. The measurement script runs from the repository root.

The ordinary impact window narrows the failure: core525 clearance+22.095mm,
526+5mm with an empty manifold, 527-44.599mm still with an empty manifold,
528-27.463mm with four points. Penetration persists through540, then release541
has positive clearance. `impact-window.json` preserves these values. Source
inspection identifies a candidate mechanism: empty contact recycling permits
up to20mm translation, so the17.095mm CCD correction into the speculative shell
can reuse the prior empty result. Host TOI deliberately skips an initial gap
within its5mm target plus tolerance. A focused unforced unit-cube regression
now tests this transition at22mm initial clearance and2.66667m/s descent;
it requires the second step to remain within the existing5mm linear slop.
Baseline build68818 finishes0; focused regression78711 fails101 with second-step
center y=0.4605555, i.e.39.4445mm penetration. The baseline binary is frozen as
`empty-contact-before-fixture`; log `empty-contact-before.log` retains the failure.
The captured drag contact also has lifecycle flags5 (ALIVE|RECYCLED), directly
confirming empty-result reuse at the failing frame. Shader `recycle_skip` now
rejects empty manifolds because no stored separation bound justifies their
reuse. Same-baseline disabled-recycling control8615 passes0. Patched build73933
finishes0 and ordinary focused test3416 passes0 with recycling enabled. Thus
the isolated defect has a before-fail/control-pass/after-pass result;
`empty-contact-controls.json` records baseline binary hash and exact exits.
Native test build77951 finishes0. Regression drivers23926 ordinary and97716
native remain live. Ordinary static-library rebuild13563 and unchanged drag
fixture link44407 finish0; original3060step ground-drag runner36500 is live.
`empty-contact-fix/drag-ordinary/` preserves all input hashes, command, state and
exact exit. Native static-library build26975 and link21593 finish0; the matching
ground-drag check is launched with evidence in `empty-contact-fix/drag-native/`.
Both ground-drag checks36500/90117 now finish1, preserving all3060states and
original CPU trajectory failures (maxposition0.3899/0.38982m). New geometric
scan finds18.4908mm peak penetration on both paths at1140/body5, versus44.5985mm
before; all50release endpoints per path meet original absolute settling limits.
At the old ordinary failure frame527, contacts now contain four points and
clearance is+0.0286mm. The remaining peak1140 also has four points, under a
loaded motor with vertical anchor error-0.68505m; release1141 has+2.835mm
clearance. Evidence: `empty-contact-fix/clearance-settling.json` and
`contact-motor-window.json`. These observations close the identified empty-cache
mechanism, not overall dragging acceptance or the loaded-compliance question.
Native focused regression also passes within native/ccd.log. Ordinary CCD
selector finishes0; both complete regression drivers23926/97716 now finish0,
with28CCD,6recycling and3restitution passes independently per path.

The superseded pre-paired Rain first captures now have exact child0/launcher0
receipts on both paths. Their batch drivers were stopped only after those
receipts and absence of physics children were verified; driver exits143 are
retained (63604/15392), with `rain-v22-five/*/superseded-batch-stop.json` explaining
the intentional stop of further obsolete repeats. Dedicated finalizers1879/
79885 use the existing runner validation/compression routine on the completed
600step raw captures. No child/launcher receipt is changed and no physics is
rerun. Finalization is pending; these remain older-candidate single captures,
not current-candidate or five-repeat qualification.
Their health scans83827/77017 finish0:600frames each, zeroNaN/exploded/capacity
loss, with identical selected residual summaries. Peak cone1.26105142rad at
516/endpoints5003–5004 and longest >.05rad lower-twist episode62awakeframes
355–416/endpoints1684–1693 reproduce the prior baseline. Full selections remain
in `rain-v22-five/{ordinary,native}/health-summary.json`; these absolute screens
do not establish physical acceptance.

Saved independent CPU drag geometry provides a further loaded-contact control:
all15300bodyframes were scanned; peak penetration22.358mm occurs at526. At
the patched GPU's worst loaded release1140, CPU penetration is18.207mm versus
GPU18.491mm; both return to positive clearance1141. Source hash and selected
measurements are in `empty-contact-fix/cpu-clearance-reference.json`. This
narrows the remaining question toward motor/contact compliance, but CPU
reproduction alone does not prove acceptance or replace failed original checks.
The patched drag state-health scans75281/39234 finish0 at3060steps per path,
with no captured capacity loss, invalid-state flag or completion mismatch;
per-path `state-health.json` explicitly excludes physical/full-state acceptance.
`empty-contact-fix/manifest.json` freezes both libraries, test executables and
drag fixtures plus99Rust/WGSL hashes. Relative to the previous paired candidate,
only the world.rs regression and collide.wgsl empty-manifold veto changed.
No persistent field was added: captured contact count drives the new eligibility
branch, and existing v22 contact geometry/history/order coverage remains needed.
No new normalization is authorized. Old fresh-repeat results do not qualify
the changed trajectories; samples and running Rain still use earlier binaries.
Subsequent sample links49616/36282 finish0, verified against the frozen library
and source hashes, and both new executables are added to the cache-fix manifest.
Fresh600step health-only checks77061ordinary/10251native now run under
`rain-empty-contact-health/`; their fixture hashes match the frozen samples.
The older paired health checks continue as comparison evidence. Neither batch
is a full-state five-repeat qualification or controlled performance result.

A bounded test-only loaded-contact experiment now checks the compliance model
under known force: `constant_load_contact_compliance_matches_softness` applies
100 and1000m/s² downward acceleration to a rotation-locked unit cube on a floor,
with unchanged contact defaults and600steps/load. Four coplanar points each
have effective normal mass m. The stationary soft-constraint equation predicts
total stiffness4*m*omega² and penetration(applied_acceleration-gravity_y)/(4*omega²),
using the existing static-contact frequency clamp. Before evaluating the test,
the tail120steps must remain within0.1mm of that prediction and below1mm/s.
Ordinary build51224 finishes0 and run60306 passes both600step loads with all
tail120steps within the predeclared0.1mm/1mm/s limits. The executable and SHA256
are frozen as `empty-contact-fix/compliance-ordinary-fixture`; output is in
`compliance-ordinary.log`. Native test build14390 finishes0; run92484 finishes0
with the native runtime environment, and its binary/SHA256 are frozen as
`compliance-native-fixture`. Both loads pass the original limits on each path;
`compliance-results.json` records exact exits and executable/log hashes. This
is a discriminating model check, not relaxed acceptance for the moving,
off-center drag scenario. No production defaults or solver code change.

All completed Rain captures are now accounted for: older full-state finalizers
1879/79885 finish0 with verified600step compressed state; paired health26826/
77334 and cache-fixed health77061/10251 finish0. Absolute scanners19415/80097/
11118/51068 finish0. Cache-fixed peak cone error1.05444145rad at179/endpoints33–34
is lower than paired1.29449368rad at520/4961–4962, but a new persistent episode
lasts454–591 (138awakeframes), joint3942/endpoints3935–3943, peak0.360523075rad
and final0.20179686rad. Both paths have identical residual summaries.
Unchanged CPU-relative scans47659/54383 both fail1 with1194anchor,4449angular,
15871cone,1656lower-twist and1172upper-twist failures across1948800matched joint
observations. Reports are in `rain-empty-contact-health/*/cpu-relative.json`.
No physical Rain pass is claimed.

The next bounded diagnosis targets that persistent episode. Capture89833 uses
the frozen ordinary cache-fix sample for600steps, saving core state only440..600
and full health under `rain-empty-contact-cone/`. Existing hooks and exact exit
receipts are retained; this is not full-run repeat qualification. Selector31512
validates health449..591/cellbase3907/target3943 under `rain-empty-contact-episode/`.
The fresh capture must reproduce the selected lifetime/trajectory before its
contact and joint caches are used to explain the persistent error.
Selector31512 finishes0 and validates all42bodies over143frames. The target is
the third Human's pelvis→right-thigh joint (bone indices0→8), verified against
`box3d/shared/human.h` and the right-thigh setup in human.c. Its upstream cone
limit is10degrees, twist[-30,60]degrees, and unchanged default base tuning is
60Hz/damping2. `rain-empty-contact-episode/target-motion.json` retains metrics
for each selected frame: at480 speeds0.137/0.183m/s and angular1.82/12.62rad/s;
at591 speeds0.217/0.299m/s and angular1.30/6.77rad/s, anchorerror1.956mm and
coneexcess0.2018rad. Both bodies remain awake throughout the138frame episode.
This excludes a sleeping-stall explanation but does not establish acceptable
load compliance. The bounded core capture remains live for contact/joint-cache
analysis; no further solver change is made from health data alone.
Independent swing reconstruction composes captured body rotations with the
normalized upstream right-thigh local frames. The initial libm atan2 comparison
fails1e-5 tolerance by a maximum1.8493e-5, preserved in
`rain-empty-contact-episode/angle-reconstruction.json`. Inspection shows
b3GetSwingAngle uses b3Atan2's deterministic minimax polynomial in
box3d/src/math_functions.c. Evaluating that polynomial on the independently
reconstructed angle agrees within3.40054e-7 across all143frames at the unchanged
tolerance; evidence is `angle-reconstruction-native-polynomial.json`.
This confirms the geometric violation and accounts for the initial comparison
failure without changing a solver parameter or acceptance threshold.
The diagnostic extractor now indexes by the recorded core frame rather than
line number, allowing this window to start at core440. It still rejects gaps,
duplicates, incomplete requested windows, stale generations and pose mismatches.
Synthetic full/prefix-trimmed inputs produce byte-identical excerpts; all five
negative controls reject as expected. Evidence is
`desktop-baseline/rain-extractor-window-check/results.json`. This is extractor
validation only; actual Rain trajectory matching awaits the live capture.
The existing compliance analyzer also reports swing effective mass, stationary
cached-impulse residual and start/end swing-axis alignment. A17frame historical
excerpt runs successfully; an analytical isotropic-tensor input recovers unit
effective mass and0.02rad residual (within1e-8). Outputs are in the same evidence
directory. Cone excess uses libm atan2 and is labeled approximate; these values
neither reproduce a complete substep torque balance nor establish acceptance.
The live cache-fixed capture has now reached the selected episode. Core450..460
matches all42cell bodies' float32 rotations and linear/angular velocities plus
slot/generation identities against the earlier completed health report. Target
core IDs are4035→4043, joint3942. The11frame excerpt and manifest are
`rain-empty-contact-episode/ordinary-impact-prefix.*`; the preceding5frame
onset excerpt is also retained. `impact-compliance.json` reports at core455
approximate cone excess0.360505rad, stationary cached-impulse residual0.117275rad
and step-start/end swing-axis dot0.319209. At460 those values are
0.169271/0.017178/0.617481. This confirms the captured impact trajectory but does
not establish a full torque balance or justify the previously rejected
updated-axis-only change. The later persistent portion and complete run remain
pending; the capture process continues unchanged.
The subsequent21frames core461..481 also match the earlier health exactly for
all42cell bodies (extractor24796 exit0). Evidence is
`rain-empty-contact-episode/ordinary-persistent-prefix.jsonl` and its manifest,
with `persistent-prefix-compliance.json`. Cone excess spans0.102552..0.204187rad,
cached stationary estimates0.005131..0.050761rad and swing-axis dot products
0.553345..0.956748. The discrepancy therefore persists after impact, but cached
end-step values are still insufficient to establish substep load balance.
The historical rejected relinearization already refreshed axis AND effective
mass; repeat that experiment only if a genuinely different mechanism is tested.
An independent unit-impulse calculation with captured inertia and joint levers
finds the hip point constraint removes9.14..23.98% of cone-rate correction at
start geometry,7.33..24.87% with end levers, over core450..481. Analytical
zero-lever and isotropic-unit-lever controls give0 and0.5; point residuals remain
below1e-9. `rain-empty-contact-episode/point-cone-coupling.json` and its generator
retain the results and limitations: this is a hard projection diagnostic, not
the actual soft solve or a complete contact/joint interaction model.
The existing isolated Rain fixture is now testing collision dependence from
health480, cellrow3/column9, base3907,target3943. The120step CPU/ordinary arms
repeat original collision filters or disable all Human collisions, retaining
upstream joints and using fresh history. `rain-hip-load-replay/protocol.json`
predeclares interpretation and exclusions; fixtures/manifests link the frozen
cache-fixed GPU library. Build93488 exits0; run17223 has successful CPU exits
for both arms and GPU arms pending. Full Rain capture continues independently.
All four replay arms subsequently finish0 (17223). The analyzer verifies42exact
float32 initial getter states,5040finite body records and3240spherical records
per arm. With collisions, CPU/GPU target cone final=.14410454/.183344901rad,
tail60max=.147624344/.210057288. Without collisions, both give final
.000124052167rad and tail60max.000211909413. This supports contact dependence
in the isolated fresh-history replay, not physical acceptance of the loaded
error. Full stats and hashes are in `rain-hip-load-replay/summary.json`.
The full-scene excerpt further shows the target's only active contact partner
at core470..481 is core4042 (Human3942), its own left calf. Earlier461..468 also
contact4014/4016;469 still contacts4014. `target-contact-partners.json` preserves
all21frames. Upstream calf filters use group0 whereas the thigh uses the
negative Human group, so this contact is allowed by upstream. A selective
diagnostic is needed to separate this pair from wider contact loading.
The selective diagnostic is now complete under `rain-hip-pair-replay/`.
An artifact-only fixture creates the same force-free filter joint in both
arms, with collideConnected true versus false for3942↔3943. All four CPU/GPU
runs exit0, pass initial/body/joint completeness checks, and report120pair
contact points in each sham versus zero throughout each disabled arm. Tail60
cone maxima CPU/GPU are .213215/.222990(sham), .138892/.154004(disabled);
finals .161817/.189009 versus .124223/.148645. Removing the pair does not
resolve the error. Sham traces differ from the no-added-joint originals, so
the added topology is not claimed to preserve ordering; this limitation and
exact contact audits are retained in pair-validation.json.
A separate artifact-only temporal-resolution check (`rain-hip-substep-replay/`)
runs CPU/ordinary x4/8/16substeps for120worldsteps from the same initial state.
All six exit0 and validate all records;4substep body traces exactly match the
earlier originals. Tail60cone maxima CPU=.147624/.197574/.220051 and GPU
=.210057/.194023/.219055. Refinement does not remove persistence. Full metrics,
hashes and default-control.json are retained. Neither experiment changes
production scene parameters, solver defaults or physical acceptance criteria.
The material decomposition (`rain-hip-material-replay/`) completes eight120step
CPU/ordinary arms: unchanged materials, zero sliding friction, zero rolling
resistance and zero both. Build25250/run32689 exit0; all initial/body/joint
records and42material getters per arm validate. Unchanged-material body traces
match earlier originals exactly. With both resistances zero and normal contacts
retained, tail60cone max CPU/GPU=.00355715/.00345930rad, final
.00262496/.00326228. Zero rolling alone remains .180940/.209618tailmax; zero
sliding friction alone yields .130270CPU/0GPU. This supports frictional loading
as sustaining the isolated persistent violation, without establishing physical
acceptance under original materials. No production material change is retained.
The original-sham mechanical energy maxima926.000CPU/913.371GPU are below
initial945.864J, finals803.848/802.153J; these omit elastic/contact storage and
motor work and cannot stand alone as an energy proof. Complete metrics, hashes,
controls and material checks remain in summary.json/material-validation.json.
An end-step cached-impulse invariant check covers1082active contact patches in
core450..481. Sliding<=friction*sum(normal), rolling<=rolling*sum(normal), and
twist<=friction*sum(lever*normal) all hold within the predeclared numerical
allowance1e-5*max(1,cap). It uses each rb.w normal cache, not the accumulated
total_normal_impulse field. At481, the target thigh/calf contact is saturated
in both sliding(.63329344Ns vs.63329339cap) and rolling(.018998799Nms vs
.018998802cap), with normal1.055489Ns. Evidence is
`rain-empty-contact-episode/friction-bound-check.json` and its generator.
This excludes an end-cache cap violation in those excerpts, not all substep
work defects or the open physical acceptance question.
The bounded Rain capture is complete:89833 exit0, child0/launcher0,4779.95s;
600health frames/1252800spherical observations structurally validate.
Extractor85989 validates all143selected frames core450..592 against the earlier
health report, including all42cell identities and exact float32 q/v/w fields.
`ordinary-full-episode.jsonl` and its manifest retain this evidence. The separate
core health checker62226 validates all161captured steps440..600 with no sticky
loss, invalid-state flag or completion mismatch; raw SHA256 is recorded in
`rain-empty-contact-cone/ordinary/state-health.json`. None of these checks is a
five-repeat or physical acceptance claim.
The full friction scan covers6681active patches and preserves22strict ideal-cap
failures, all rolling impulses on awake contacts. Every squared impulse excess
is <=1.182890e-7, below the existing FLT_EPSILON1.192093e-7 rolling-clamp deadband
in both WGSL and upstream scalar/SIMD CPU solvers. The failed strict screen
remains in friction-bound-check.json; rolling-deadband-explanation.json records
the source-consistent explanation. The initial pass is retained separately as
prefix-friction-bound-check.json. The target4042↔4043pair has142observations and
no strict cap failures. This does not establish all substep work or acceptance.
Full-compliance.json also shows the stationary estimate can closely match at
health500(.081915rad vs.087609measured), but fails as a general model for the
moving episode: at591 estimate.010188 vs.201786measured, axisdot.591090 and
endpoint speeds1.30/6.77rad/s. Original-material physical acceptance remains open.
The stationary formula itself is now independently tested with the retained
`c_abi/spherical_compliance_reference.cpp`. A unit-inertia dynamic body with a
static spherical anchor, coincident COM anchors and no contacts receives
constant world-X torque. Default60Hz/damping2 and four substeps imply
K=I*(2*pi*min(60,0.25/h))^2. Torques1421.2229/14212.2295Nm predict .01/.1rad
cone excess. Before evaluation, both600step loads require the final120steps
to have absolute prediction error<1e-4rad and angular speed<1e-3rad/s.
CPU, ordinary GPU and native cached GPU all pass with maximum errors
1.74226e-6/5.09452e-6rad and zero tail angular speed. All1200records per path
validate; build30657/run41403 exit0. `loaded-cone-reference/` freezes the
protocol, source, library/binary hashes, commands, raw traces and results.json.
This validates isolated stationary compliance, not the moving friction-loaded
Rain episode, and does not substitute for final five-repeat qualification.

Same-bridge pre-paired v22control52534 also fails (maxposition0.245738m, held
position0.152442m, heldvelocity3.08363m/s). CPU-compatible scalar-order control
52806 gives identical maxima. Its both-engine trace first diverges in position
at frame227/body0 by0.016072364m while velocity agrees. A diagnostic copy of the
C fixture with only enableContinuous=false, run87008, passes original limits:
maxposition0.00602796m/chord0.00597248, heldposition0.00288055m,
heldvelocity0.047432m/s and peakvelocitydifference0.230405m/s. This isolates CCD
involvement, not an acceptable default change or a substitute pass. Production
source and original failures remain untouched. Direct cutoff discrimination and
first-step collision analysis are next; preserve both drag and restitution gates.
Evidence: `desktop-baseline/drag-paired-candidate/control-summary.json`,
`drag-prepaired-control/`, `drag-scalar-order-control/`, `drag-no-ccd-control/`.

A copied-source host-cutoff discriminator now restores only the old
0.5*minimum_extent activation test (production sources unchanged). Library21984
and drag link19131 finish0. Scalar/order1 run44253 passes with continuous collision
enabled and the same0.00602796m maximum error as the no-CCD control, directly
isolating the new cutoff's effect. Paired/order0 run61690 instead aborts at
frame2460 on the unchanged settling assertion: body4 angular speed0.13578371rad/s
exceeds0.1; linear speed0.09601665m/s, center height0.70661068m. Other cubes sleep.
This is not an acceptable rollback or a production result. Original screens and
defaults remain unchanged. `desktop-baseline/ccd-cutoff-control/` contains copied
sources, library/fixture hashes, exact exits and `paired/settling-failure.json`.

At the first scalar divergence227, oriented unit-cube support computed from the
actual p/q gives CPU minY=-0.010831614m versus GPU+0.005000102m (both+0.033612905m
one frame earlier). This establishes an earlier clamp with reduced penetration;
it does not establish that the later strict comparison failure is instability,
nor does it close dragging acceptance. Evidence:
`drag-scalar-order-control/ordinary/first-impact-clearance.json`.

The original contact-free ten-grab fixture passes on both paired backends
(3296/77700 exit0):1320steps with exactly0 position/quaternion/velocity difference.
Evidence: `desktop-baseline/drag-isolated-paired/`. This is a single-run isolation
of picking/joint behavior, not a replacement for failed ground-contact gates.

Rigid-body mechanical-energy reconstruction for the old-cutoff paired case finds
a2.541764J/kg increase just after release2341, then decreasing energy and slow
near-edge toppling toward the settling failure. The reconstruction omits stored
contact/joint energy and is not a complete work balance. A scalar control shows
the same4.01211330J/kg release increase on CPU and GPU (8.66516597 to12.67727927),
so attributing this observation to paired solving would be unsupported. Evidence:
`ccd-cutoff-control/paired/release-energy.json` and
`drag-paired-candidate/release-energy-{comparison,cpu-control}.json`.

The user was asked to clarify strict trajectory agreement versus independently
justified physical acceptance for ground dragging; no answer yet. Existing
failed screens remain preserved and required pending clarification.

All ten paired ragdoll child runs now finish0 and pass original physical screens.
Raw aggregate16267/36170 exit1 on frame1 occupied append order. Existing audited
comparisons1908/38749 both finish0/pass for all600steps/five runs per path with
exact physical slots/counters and solver order. Raw append-order failures remain
preserved. Representative completed-state/policy validation1571 finishes0; these
fields remain exact in each audited comparison, closing selected paired ragdoll
physical/repeat gates under the documented completed-boundary capture scope.
The historical57mesh input poses are
unavailable on this desktop (probe/comparator source exists, exact45grid/12torus
dataset does not). Their old passes cannot establish current-build validation.

| Capability | Required evidence | Current status |
|---|---|---|
| Falling Ragdolls repeatability | Five complete-state fresh runs per configuration, 600 steps | Current desktop child-compaction candidate passes five600step audited comparisons per path, preserving exact physical slots/counters and solver order. Only occupied/history memberships normalized; raw append-order failures retained. Selected v22 source-contract applicability verified on current baseline; final v22 repeats remain open. |
| Falling Ragdolls stability | Existing separation, angular-speed and settling limits; finite complete state | All ten current desktop child-compaction600step runs pass unchanged original physical screens; wider physical coverage remains open. |
| Rain recycling | Persistent logical body/joint correspondence across creation/deletion/reuse; complete state and health checks | Creation mapping passes all 600 steps, 8,400 bodies and 1,948,800 joint observations across 4,200 reused slots; residual screening still fails |
| Rain joint correctness | Explain excessive residuals and qualify appropriate constraint behavior | Full 600-step spherical capture validates identities but fails residual screening: first cone discrepancy at frame130, before first anchor discrepancy at170. Matched-state replays reproduce severe cone and persistent twist behavior in both engines; sleep ablation does not eliminate isolated twist residual. Full-scene history/load diagnosis remains open |
| Collision response and penetration | Analytical isolated contacts plus persistent support/penetration checks in stacks and mesh scenes | Flat-edge admission corrected; 57 frozen-pose cases pass ordinary/native cached. Analytical three-cube support/energy fixture passes five600-step fresh repeats on each path with corrected defaults; every raw captured field matches independently. Wider behavior coverage pending |
| Friction and restitution | Existing analytical/reference fixtures with unchanged coefficients and tolerances | Analytical sliding/frictionless fixture passes five240-step fresh repeats per path historically. Desktop CCD-coverage candidate passes all unchanged restitution gates in five240-step fresh runs per path, with exact raw captured state; baseline49.31mm penetration failures retained. Current single-run friction regression passes each path; wider/final-candidate qualification remains open. |
| Mass and inertia | Existing lifecycle and analytical fixtures, including multiple shapes and center overrides | Selected ordinary/native GPU-order fixtures pass; wider qualification pending |
| Motors and limits | Isolated mechanisms, constraint errors and scripted responses | Existing Prismatic/twist fixtures pass prior qualification; wider matrix pending |
| CCD | Sweep/reference fixtures and fast-body scenes with no missed barriers | Selected ordinary/native GPU-order fixtures pass; five fresh five-step CCD mutation repeats match every raw v19 field per path; wider qualification pending |
| Warm-start configuration | Requested setting must be honored or explicitly unsupported | GPU warm starting is always enabled; `c_abi/samples_api.c` ignores `b3World_EnableWarmStarting` and reports true. Disabling warm starting is unsupported and cannot be used as a matched CPU/GPU ablation. |
| Sleeping and waking | Awake state, settling, island behavior and scripted wakeups | Awake/sleep capture and explicit sleep-to-wake fixture pass on both paths; broader island behavior pending |
| Dragging | Scripted interaction, constraint behavior and release/settling | Five 3,060-step ground dragging runs per path pass every raw captured field independently and all original physical gates. Prior strict comparison passes at 6.03 mm max |
| Creation/deletion/replacement | Lifetime, stale-state and identity-reuse checks, including contacts and warm starts | Five fresh captured-state repeats per path pass the 16-step child/root reuse and motor-history fixture; unread-handle and retired-generation controls also pass five processes per path. Broader lifecycle coverage remains open |
| Contact query ordering | Stable semantic enumeration and correct handle lifetimes | Handle allocation and child-patch enumeration defects fixed; five fresh eight-step captured-state repeats pass per path, plus unread handle-lifetime controls. Full-scene/current-build coverage remains open |
| Contact end events | Repeatable ordering, one end per logical transition, valid later begins | Duplicate refilter ends fixed; teleport/deletion/refilter and six previous-history permutations pass on both paths. Five fresh callback/lifecycle captured-state repeats and handle controls pass per path; wider event coverage remains open |
| Native full-step replay | Actual replay reuse, complete per-step state, mutation/reentry and five fresh repeats | New 48-step contact-stack control captures every step and records 42 replay hits, including reuse after substep and transform changes; five fresh repeats pass every captured field over 48 steps. Existing long tests inspect bodies at checkpoints only. |
| Performance | Controlled small/large scene measurements, with final-candidate applicability | Baseline five paired runs per scene/path complete: small mean-speed screens pass, large screens fail. Keep native ordering opt-in; changed candidates require new measurements. |
| Recordings | Repeated GPU runs and representative CPU/GPU behavior comparisons | Prior CPU-compatible clips exist; GPU-order qualification clips pending |

## Fixed fixture selection for closing qualification

This is the bounded fixture selection, not a statement that each gate passes.
Run each selected configuration independently on frozen binaries; retain five
fresh processes for repeatability fixtures. Numerical/reference assertions keep
their existing source limits. Any remaining coverage gap below stays open instead
of being hidden by a nearby passing test. Commands run from `experiments/gpu-physics`.

For Rust fixtures, select the named function with
`GPU_PHYSICS_LIVE_CONTACT_ORDER=0 cargo test --release --lib --features replay-diagnostics <selector> -- --nocapture`.
Require a nonzero selected-test count. Native builds use
`scripts/build-native-cache.sh test` with the same cargo arguments, with
`scripts/native-samples-cache-env.sh` sourced for runtime. Freeze test executables
for the five-process phase rather than rebuilding between repeats. Where supported,
set a unique `GPU_PHYSICS_TEST_TRACE` for each process and compare every captured frame.

| Capability | Fixed fixture / duration | Acceptance contract and remaining gap |
| --- | --- | --- |
| Falling Ragdolls | `c_abi/ragdoll_reference.cpp`, `scripts/check-core-state-repeats.py`,600steps | Existing `check-ragdoll-health.py` limits: peak separation<0.25m, tail<0.005m, tail speed<0.05m/s/angular<0.1rad/s; retain angular-speed/reference checks. Semantic allocation audit remains open |
| Rain | `Benchmark/Rain`, `scripts/check-sample-state-repeats.py`,600steps | Full creation identity/recycling correspondence; existing+0.05m/+0.05rad CPU-relative screens retained and currently failed. Physically justified residual classification must close before pass |
| Support/energy | `trace_stack_support_and_energy`,600steps | Overlap/lateral drift<0.025m, tilt quaternion-vector norm<0.01, energy gain<0.1J/kg; final120steps height within0.025m, speed<0.05m/s/angular<0.1rad/s |
| Friction | `trace_sliding_friction_matches_analytical_motion`,240steps | Stop distance0.4±0.04m, speed<0.05 after36steps; frictionless position/speed errors<0.005; height/lateral error<0.01m; retain all rotation assertions |
| Restitution | `trace_restitution_matches_analytical_rebound`,240steps | Rebound apices1.0/2.5±0.05m, penetration<0.03m, energy<25.2J/kg, settling assertions unchanged. CCD-coverage candidate passes five fresh runs per path with exact raw captured state; baseline49.31mm failures retained. Wider candidate qualification remains open. |
| Motors/limits | `trace_prismatic_motor_reversal_respects_both_limits`,120steps | Reverse at46; limits±0.5 with0.01m allowance, transverse drift<1e-5; stops reached/settled per source. Spherical/revolute behavior also covered by28 numerical fixtures and Rain; persistent Rain errors stay open |
| Mass/inertia | `capsule_mass_data_matches_compute_helper`, `capsule_compound_inertia_applies_parallel_axis_once`, `capsule_angular_impulse_uses_axial_inertia`, `mass_queries_preserve_shape_and_explicit_mass`, `replacement_reuses_storage_and_preserves_body_mass_and_metadata`; `trace_mass_mutations_preserve_analytical_impulse_response`,18steps | Preserve all existing analytical/source assertions. Mutation trace covers six stages (shape mass, doubled density, explicit mass/center/inertia, kinematic, dynamic, shape removal), three free steps each. Exact public mass; unit linear/angular impulse response within1e-5m/s and1e-4rad/s respectively; nonzero impulses must give zero response in the kinematic and massless/shapeless stages. Five fresh repeats per path pass with exact raw state equality; full-state audit and final-candidate applicability remain open. |
| Sleep/wake | `high_resistance_sleeper_wakes_on_velocity`,400settle+1wake; `trace_joint_island_wake_propagation`,73steps | Require actual sleep before input. Three unit-mass cubes linked by distance joints must remain asleep for wake=false, then all wake on the first step after wake=true while the separate cube stays asleep. Axial impulse1N·s: momentum error<1e-4kg·m/s, energy≤0.5001J, spin<1e-6rad/s, length error<0.01m. Five fresh73step per-step repeats pass each path with exact raw captured state; contact-island coverage remains to assess |
| CCD | `convex_gpu_ccd_corrects_live_steps_and_rebuilds_after_mutation`, plus existing sweep/rotating-offset/exclusion fixtures | Keep source assertions and five-step mutation repeat evidence; validate barrier coverage against explicit fast-body requirement before closing |
| Lifecycle/events/query | Existing child/root joint-history16step, child-query8step, callback12step fixtures and generation controls; `unread_root_handle_retires_after_mesh_scratch_reuse` | Retain reference/identity assertions and full-step five-repeat evidence; see existing manifests for exact test selectors. Mesh scratch reuse must not resurrect an unread retired handle; confirmed pre-fix failure on both paths; corrected regression passes five fresh processes per path |
| Native replay | `trace_covers_native_full_replay_and_reentry`,48steps | Require42 actual replay hits, mutation/reentry behavior and every-frame state equality |
| Dragging | Existing ground drag3060step fixture and its frozen manifests | Retain original physical gates and five-process state comparison; final-build applicability must be recorded |
| Numerical/mesh regressions | `native_precision::` (28tests), `scripts/check-frozen-mesh-reference.py` (45grid/12torus poses) | All unchanged numerical assertions and1e-5manifold comparisons must pass. Local inputs are now tracked in `c_abi/fixtures/frozen-mesh`; historical pose bytes remain unavailable. These do not substitute for full-scene behavior |
| Performance | Falling Ragdolls + Large Pyramid;5runs/path/order,60warmup+180timed | No tracing/health instrumentation, completed-step timing, no competing GPU work. Use the controlled-performance protocol and default decision below; measurements remain open. |
| Delivery | Scene snapshots/CPU column, coverage and default documentation | Record after final candidate qualification; do not switch default with required gates open |

The fixture selection now identifies what to run and the known acceptance gaps.
It does not close the fixed-contract gate while Rain/restitution, island-wake,
persistent-state coverage remain unresolved. The performance decision is specified
below before this machine's measurements; its evidence remains outstanding.

The portable mesh runner accepts CPU and GPU executables compiled from
`c_abi/rain_frozen_contact.cpp`, linking the same upstream Human/utility objects
and CPU/GPU sample bridge archives as the Rain-cell fixture. It freezes the
executables, verifies committed input hashes, preserves exact child receipts,
and compares contact/point counts, normal vectors and separation multisets at
the original1e-5tolerance. Every grid pose must produce contacts with upward
normals. Use an unused output directory:

```sh
python3 scripts/check-frozen-mesh-reference.py \
  --cpu-binary /path/to/cpu-rain-frozen-contact \
  --gpu-binary /path/to/gpu-rain-frozen-contact \
  --configuration ordinary --out artifacts/frozen-mesh-ordinary
# For a separately linked native-cache executable, use --configuration native-cached.
```

The committed45grid inputs sample x/z=-1,0,1 and five yaw orientations at5mm
speculative clearance. The12torus inputs are right-calf poses from CPU and paired
ordinary Rain-cell frames166–171; case3 retains the exposed triangle412 contact
from the original defect. Construction, source-pose hashes and input hashes are
in the fixture manifest. These are explicitly a new local baseline, not claimed
copies of missing historical inputs. Both paired paths pass (portable runner
49135/16371 exit0): grid max component error1.0245e-8, torus1.387e-7. Detailed
evidence/build commands are in `desktop-baseline/mesh-local-baseline/`. The
tracked exact critical triangle inputs and covered-seam/fully-flat rejection
controls also pass in the28numerical tests. No solver change was needed.

## Acceptance rules

Desktop mass-mutation evidence is under
`desktop-baseline/mass-mutations-nonzero-five/{ordinary,native}`. Each directory
freezes its executable, source/runner hashes, runtime configuration and command,
and retains five successful receipts with exactly one selected test and18frames.
All trace bytes are identical within each path; the comparator also validates
every frame's schema/state groups. The initial `mass-mutations-five` batch used
zero inputs in zero-mass stages and was insufficient to detect stale inverse
mass. It remains preserved but is superseded by the nonzero-input batch, whose
physical outputs match. This fixture adds analytical mutation and trace coverage;
it does not claim complete persistent-state coverage or replace the separate
capsule, compound-inertia and geometry-replacement assertions. No production
solver behavior or upstream sample parameters changed.

Preserve upstream physical defaults and existing isolated/analytical tolerances.
Use each existing fixture's checked-in limits as its initial acceptance contract;
record any missing contract before evaluating a candidate fix. Do not broaden a
limit simply because the implementation fails it. Soft constraints require
limits derived from their tuning, load and timestep, not arbitrary pose matching.

Ragdoll stability retains peak joint separation below 0.25 m, final-two-second
separation below 0.005 m, linear speed below 0.05 m/s and angular speed below
0.1 rad/s. Retain the existing peak angular-speed comparison and collision-free
precision gate. These checks alone do not prove collision or joint correctness.

`scripts/check-ragdoll-health.py` evaluates those physical limits independently
of CPU trajectory equality, while the older strict validator remains intact.
It requires all 67,312 finite, ordered frame/body records (frames 0–600, 112
bodies), reports the worst frame/body for each metric, and records input hashes.
Run it against the matching CPU and GPU fixture outputs:

```sh
python3 scripts/check-ragdoll-health.py --cpu <cpu.txt> --gpu <run-N.txt> --out <health.json>
```

The checker passes the preserved ordinary reference run; boundary/excess,
nonfinite, wrong-identity and truncation controls reject invalid input/results
(`ragdoll-health-historical-control.json`, `ragdoll-health-checker-controls.json`).
This historical positive control does not qualify a current run. A fresh CPU
fixture linked from the same upstream objects as the v19 GPU fixtures completed
600 steps and reproduces the previous CPU trace byte-for-byte
(`ragdoll-v19-five/cpu.txt`). Current GPU health results remain pending until
complete outputs are available. These metrics do not establish penetration,
energy conservation or complete constraint correctness on their own.

Rain's current +0.05 m anchor and +0.05 rad angular CPU-relative screening failures
remain open evidence. First establish corresponding joint types, endpoints and
lifetime, then inspect the actual constraint and incident loads. A comparison of
unrelated recycled IDs is invalid; a real excessive residual must be explained
and corrected, not relabeled as intentional divergence.

State comparisons must cover body position/orientation, linear/angular velocity,
awake/sleep state, live joint configuration and accumulated solver state, active
contact geometry/impulses and relevant persistent solver state. Canonicalize
identities through explicit lifetime mapping. Exclude timestamps, padding,
unused capacity and allocation-dependent identifiers, but retain relational
identity and solver ordering when those affect future simulation. Report the
first differing frame, object and field. A pose checksum is only a subset check.

The first frozen-v19 ordinary Falling Ragdolls run now completes all 600 steps
and passes the unchanged physical screen: peak separation 0.213310 m, final
window separation 0.00138940 m, and zero final-window linear/angular speed.
Peak angular speed is 101.964 rad/s, matching the CPU reference metric. Its
pose/velocity/separation output is byte-identical to the historical ordinary
reference. All 600 complete captures report no sticky capacity loss, zero loss
causes and no invalid-state flag (`ragdoll-v19-five/ordinary/run-1-health.json`
and `run-1-status-health.json`). The exporter calls an explicit GPU wait;
`finish_contact_status` refreshes the status if its previous asynchronous
readback predates the completed step. The second frozen ordinary run also completes 600 steps, passes the same
physical screen and has byte-identical pose/velocity/separation output
(`run-2-health.json`). Its richer state trace still contains the recorded
storage/allocation differences. Five-run semantic repeatability and native-path
physical results remain pending.

The second frozen-v19 ordinary ragdoll run exposes a captured-state mismatch
at frame 1: `contact_allocation.occupied_order` differs. A diagnostic comparison
that sorts only this membership list finds a further mismatch at frame 139 in
physical manifold-child slot allocation (`run-2-prefix-check.json` and
`run-2-occupied-diagnostic.json`). The original traces and strict comparator are
unchanged; this is a failed captured-state comparison, not a passed determinism
gate. `collect_occupied_contacts` appends through an atomic counter, and
`allocate_manifold_slot` also assigns free slots through an atomic counter.
The next investigation must distinguish incidental membership/storage order
from order affecting later allocation, contact processing or numerical state.
No shader ordering fix or trace canonicalization has been accepted yet.

A targeted perturbation test now reverses the actual GPU occupied list after
each captured step, with eight independent contacts and a teleport-away/return
sequence. It compares all state except the deliberately permuted membership
order. Initial same-process baseline/perturbation runs were confounded by a
global contact epoch difference at the first frame, before any reversal. The
corrected harness runs each case in a fresh process; it retains the epoch field
and all allocator/hash/history checks. Ordinary/native fresh-process controls
pass (`occupied-permutation-*-fresh.log`): six steps, eight contacts and
teleport-away/return preserve every other captured field. Failed original logs remain
preserved; they do not establish an occupied-list dependency.
The native five-by-600-step batch has now been dispatched independently from
the previously frozen native fixture, whose SHA-256 matches the build manifest
(`aefd518affad045992900846b90d6c37c56c263846dcbc58f9c2c0b0b8b1e137`). Its
first ten complete captures confirm NVIDIA GeForce RTX 4070 Laptop GPU,
Vulkan, NVIDIA driver 610.57.04 and native policy state
(`ragdoll-v19-five/native/run-1-startup-check.json`). The batch is still running;
this is a startup check, not a repeatability result. Both full-scene batches
use the frozen build preceding the host query/event ordering fixes. They run
concurrently for correctness evidence and must not be used for performance
claims. Native evidence is separate under `ragdoll-v19-five/native`; the output
runner refuses to overwrite it if the original sequential driver reaches the
same destination later.

The complete 600-step run-1/run-2 comparison is now audited at every captured
field. Only `contact_allocation` differs (528 frames, first at frame 1) and
slot-indexed `event_history` (three frames, first at frame 140). Body/joint
state, logical contacts, public queries/events and every other captured group
match throughout all 600 frames (`run-2-all-frames-differences.json`). This is
two-run diagnostic evidence from the frozen pre-query-fix binaries, not the
required five-run qualification or a justification to drop allocation state.
Run 3 has since completed 600 steps and passes the unchanged physical screen;
its pose/velocity/separation text is byte-identical to runs 1 and 2
(`run-3-health.json`, `first-three-pose-file-hashes.json`). Run 4 has also completed and passes the physical screen with byte-identical
pose/velocity output to run 1. Its full 600-step audit finds only allocation
(540 frames) and slot-indexed event history (three frames) differences; all
other captured groups match (`run-4-all-frames-differences.json`). Run 5 is active.
The third run's full 600-step comparison is now complete: only contact
allocation (543 frames, first at 1) and slot-indexed event history (two frames,
first at 141) differ from run 1. Every other captured group matches, including
body/joint state, logical contacts and public results
(`run-3-all-frames-differences.json`). The strict gate remains failed.

The frame-139 allocation difference is localized to one logical child manifold,
shape identities 92/118, chain ordinal 1: slot 148 in run 1 versus slot 145 in
run 2, with generation 2 following the live record and zero in the other free
slot. All other captured fields at that frame match after occupied membership
is sorted (`frame-139-allocation-diagnostic.json`). The slot relocation's effect
on future allocation/history remains to be tested; the strict batch remains
failed and its comparator has not been changed.
A diagnostic-only relocation helper now moves a live child into a free slot
below the current high-water mark, swapping hot/persistent/prepared records
and repairing incoming chain links. It asserts neither slot owns a logical
root-history entry. The mesh-corner fixture steps through contact, departure
and return; ordinary/native probes are pending in `child-relocation-probe.log`
and `child-relocation-native-probe.log`. Both probes now pass and confirm
relocation from slot 2 to slot 1. Fresh controls on both paths preserve all
captured state except physical slot bookkeeping and slot-indexed previous
contact history, converging again after re-entry (`child-relocation-ordinary-differences.json`,
`child-relocation-native-differences.json`). A stronger native variant creates
another body/contact while the old pair is absent. Physics still matches, but
a later root contact receives generation 2 versus 1 due to inherited slot
history (`child-relocation-native-reuse-differences.json`). This persistent
counter difference requires explicit handle/callback-lifetime testing before
excluding allocator history from the semantic gate. The production allocator
and strict trace comparator remain unchanged.

The relocation fixture now enables contact events and reads public contact
handles every step. Ordinary/native checks pass: the original root handle stays
valid through relocation, becomes invalid on departure, never aliases the
replacement, and all returned replacement handles are valid. Native fresh
control comparison preserves all published events, handle aliases and manifold
values; differences remain in slot allocation, previous-touching storage and
the later registry owner's generation (`child-relocation-native-events-differences.json`).
Expanding that fixture to three perpendicular mesh patches confirmed a real
public-query defect: relocating the last child from slot 3 to slot 1 preserved
the logical chain and physics but swapped returned manifolds at steps 2–3.
The original two-patch fixture could not expose this because the root remained
before its only child. The failing native control/perturbed traces are
`child-three-patch-native{,-control}.jsonl`.

`contact_api::update_registry` now traverses roots by shape-pair key and patches
by linked-chain ordinal, rather than enumerating physical slots. Traversal is
bounded and rejects malformed ownership, truncated/cyclic chains and orphan
children. This affects mirrored public contact queries, not the GPU solver's
constraint order. A regression checks decoded manifold order after relocation
and rejection of malformed links. Ordinary/native eight-step fresh control and
relocation probes now preserve every public manifold array, handle and event;
remaining differences are physical slot history and later internal owner
generations (`child-three-patch-ordinary-fixed-differences.json`,
`child-three-patch-native-fixed-differences.json`). This fixes an actual
observable ordering dependency without changing the strict trace comparator.
Five fresh eight-step runs of the strengthened relocation fixture pass the
unchanged comparator on each path independently, including every v19 captured
field (`child-query-five/{ordinary,native}/result.json`; binary hashes and
configuration in each manifest). Existing ordinary query-order and contact
retirement/reuse checks pass; all three native contact API regressions pass,
including end-event order. These short probes do not replace the full-scene
qualification. They do not establish that all allocator history is incidental
or qualify callback ordering; those remain open before broader canonicalization.

The event consumer audit found a second observable storage-order dependency:
`update_contact_events` selected the first patch when two patches had equal
maximum approach speed. A reducer regression relocates the last child across
another child's slot while preserving the logical chain; before the fix the
reported hit point changes from x=0.9997752 to x=1.4997752 at identical speed
10 m/s (`hit-slot-order-before.log`). The reducer now shares the bounded logical
root/patch traversal used by contact queries, stabilizing hit ties and begin
event ordering. Solver scheduling and physical state are unchanged. All four contact API regressions pass on ordinary and native paths
(`hit-slot-order-fixed-{ordinary,native}.log`). Five fresh eight-step runs of
the child-relocation fixture pass every captured v19 field on each path after
this change (`child-query-hit-fix-five`). The order-sensitive callback fixture
also passes five fresh 12-step repeats per path after the fix, comparing all
captured physics and callback state (`callback-state-hit-fix-five`). This does
not yet qualify every allocator/event dependency.

An unread-contact lifetime fixture now checks four separation/re-entry cycles
without harvesting intervening contact snapshots. It first leaves a live pair
unread for two steps and verifies that the public handle survives. It then
separates the same shapes for three steps and reunites them before reading:
the same GPU root slot is reused, its generation advances exactly once, and
every previously retired handle remains invalid. Snapshot-copy counters prove
that incidental host reads did not refresh the registry during either gap.
Both ordinary and native controls pass (`unread-root-generations-*.log`), as
do five fresh processes per path with four cycles each (`unread-root-five`).
Binary hashes and runtime configurations are preserved in the manifests. This
is a lifetime assertion fixture, not per-step full-state capture; serializing
the normal trace at intervening steps would defeat its unread-step premise.

The source audit of previous-touching history finds two writers (step reset
and topology-remap capture), both retaining pair keys until the latest event
read. `read_world_mirror` copies the array to the host; `update_contact_events`
consumes pair membership and deduplicates by public pair/compound child. Array
slots are not themselves used as event identities. Generation values have a
separate role: registry continuity across unread steps and solver color-history
validity compare slot/generation ownership. These relationships must remain
covered even if physical storage positions are eventually canonicalized.
The strict trace comparator remains unchanged.

A targeted end-event reducer test now exercises all six permutations of three
previous-contact pairs, with empty slots and duplicate patch entries. It checks
exact endpoint/handle associations and event order and ensures no begin/hit
events are invented. A control removes a real pair and requires one fewer end
event. Ordinary/native controls pass
(`previous-touching-permutation-{ordinary,native}.log`), followed by five fresh
processes per path, each exercising every permutation and the removal control
(`previous-history-five/{ordinary,native}/result.json`). These checks cover
history membership for existing public handles; they do not establish every
allocator/ownership relation as incidental or alter the raw trace gate.

The three-patch relocation fixture now has a 16-step joint-history variant:
a motor joint is added at step 7, removed at step 10, and re-added at step 12,
after the earlier child relocation and root-slot reuse. Connected collision
remains enabled; the joint's spring targets the current body position to
exercise constraint history without introducing an arbitrary large error.
The fixture requires live joints and nonempty persistent contact-color history
during joint-active phases, and no live joint in the removal interval.

Fresh control/relocation traces on ordinary and native paths preserve all body,
joint and logical contact state, complete contact-color history and public
results across all 16 steps. Every active history record's generation matches
its owner. Differences remain in allocation storage and raw registry owner
generations (`child-joint-history-*-differences.json`,
`child-joint-history-semantic-checks.json`). Five fresh perturbed runs per path
also pass every raw captured v19 field (`child-joint-history-five`). This covers
entry/re-entry into joint/contact history under the tested relocation; it does
not authorize omitting all allocator state or replace full-scene qualification.

`scripts/compare-core-state.py --audited-storage` now offers an explicit v19
comparison view for the two audited membership representations. It sorts unique
occupied-root identities and compares previous-touching pair membership after
removing empty slots/duplicate patches. It rejects duplicate occupied roots,
child identities in the occupied list and malformed history entries. Raw files
are never rewritten; the default mode remains raw, and result objects identify
the comparison mode. Slot positions/generations, allocation high-water marks,
history/owner records, hash layout, solver order, public result arrays and all
physical fields remain exact. This is a diagnostic view, not a completion gate.

`python3 scripts/test-core-state-storage.py` checks positive membership changes,
raw-data preservation, nine meaningful mutations and four malformed cases.
Real 12-step callback captures additionally verify that a storage permutation
fails raw comparison, passes the audited view, and still fails if a slot
generation changes (`audited-storage-real-trace-controls.json`). An existing
raw eight-step repeat remains green (`audited-storage-raw-regression.json`).
Applied to ragdoll runs 1/2, the audited view still fails at frame 139:
logical child [[92,118],1] occupies slot 145 in one run and another slot in the
other (`run-2-audited-storage.json`). No allocator/ownership field was excluded
to make this comparison pass.

Qualification treats external input as the ordered API-call sequence and all
supplied values, including creation/destruction, transforms, forces, world
settings and timestep/substeps. Concurrent callers must provide the same
serialized call order; wall-clock scheduling is not a deterministic input.
The current standalone ragdoll fixture has no application callbacks or
wall-clock inputs. Callback-bearing configurations additionally require the
same callback code and initial callback-owned state, and a recorded callback
input/output sequence or deterministic callback fixture. Pointer addresses do
not establish that equivalence. The trace records callback presence and
resulting engine state, but does not serialize arbitrary application memory;
stateful application callbacks are therefore unqualified until their external
input contract is supplied. This limitation does not excuse nondeterministic
engine callback invocation order or arguments.

A discrete-callback fixture now exercises eight independent box contacts with
order-sensitive custom-filter and pre-solve rejection, followed by body deletion
and same-slot reuse. It disables gravity, sleeping and CCD to isolate this
contract. Every step records callback shape identities/generations, point/normal
float bits, accept/reject decisions, invocation order and persistent rejection
counters alongside the v19 simulation state. Five fresh 12-step runs pass on
ordinary and native paths separately (`callback-state-five/{ordinary,native}`),
with binary/configuration manifests. Three corruption controls reject changed
callback order, decision and persistent counter state at the first frame.

The source audit finds that callback candidate pairs are sorted/compacted before
host filtering, and `find_existing_contact_slots`/`alloc_bind_slots` write root
slots at candidate-pair indices. The callback loop follows that array, not
physical child allocation order. This establishes discrete ordering coverage
for this fixture, not arbitrary application state or CCD/multi-patch callback
qualification. No production callback ordering or acceptance policy changed.

The new `trace_prismatic_motor_reversal_respects_both_limits` fixture tests a
zero-gravity slider with stops at ±0.5 m, motor speed ±2 m/s, reversal at step
46, four substeps and 120 captured steps. Its acceptance contract is fixed:
absolute travel at most 0.51 m (two linear-slop units beyond the stop), transverse
drift below 1e-5 m, and position within 0.01 m/velocity below 0.01 m/s during
upper-stop steps 30–45 and lower-stop steps 90–120. The default-density unit cube
has verified mass 1,000 kg; force is 100 times mass, giving 100 m/s² acceleration
and enough authority to reach the requested speed before either stop.

Initial ordinary/native controls used only 100 N and correctly failed the
arrival assertion: the body traveled 0.012604 m by step 30, consistent with
0.1 m/s² acceleration. Those logs remain preserved as
`prismatic-reversal-*-control.log`. Correcting the fixture's force, without
relaxing any acceptance assertion, passes on both paths
(`prismatic-reversal-*-final.log`). Five fresh 120-step trace repeats per path
pass all captured fields and physical assertions in `prismatic-reversal-five`
(`ordinary/result.json`, `native/result.json`). This is an analytical mechanism
fixture, not a modification to an upstream sample's defaults or a qualification
of all motor/joint types.

## Implementation checkpoints

The health-record extension adds body awake flags and generations, joint
generations/types, and endpoint body slots with their generations. These are
diagnostic fields, not yet a complete state snapshot or a cross-engine lifetime
mapping. `scripts/health_identity.py` matches joints only through an explicit
body-lifetime map and rejects reused slots, missing mappings, non-bijective maps,
topology mismatches and ambiguous parallel joints. Seven negative/positive
controls pass (`scripts/test-health-identity.py`).
Do not declare recycled joints matched solely from two reused body indices;
track body lifetimes and disambiguate multiple joints between the same bodies.

With `GPU_PHYSICS_LIFETIME_TRACE=1`, native samples record a monotonically
increasing creation ordinal for each Human bone body at the end of Human
creation. CMake generates a local instrumented copy of `human.c`; the upstream
submodule and scene parameters remain untouched. Health records carry the tag
alongside the full body handle. `creation_body_map` requires identical live
creation sets and rejects missing tags or duplicates before joint matching.
This currently covers Human bodies in matching fresh-process scene schedules,
not arbitrary scenes or cross-world identity. Ten identity controls pass.
A Rain smoke run matches all 420 bodies and 420 joints on both observed steps.
`scripts/compare-rain-lifetimes.py` streams complete health records, validates
lifetimes, records observed slot reuse, and screens corresponding joints with
the existing +0.05m/+0.05rad budgets. Its smoke result is not proof that recycling
or later residual failures are resolved; the complete 600-step result is described below.


The `replay-diagnostics` feature now exposes an opt-in core snapshot through
`gpu_b3_world_write_core_state`; the ragdoll fixture uses it when
`GPU_PHYSICS_STATE_TRACE` names an output JSONL path. Each post-step record
contains exact IEEE-754 float words for body state (including sleep), joint
configuration/impulses, and live contact geometry/prepared data/impulses. Contact
slots are normalized by shape pair and manifold-chain ordinal; unused point
capacity and true padding are excluded. The legacy contact `_pad_end` is restitution and is retained.

This is explicitly a **core-state** trace, not complete-state qualification.
The expanded trace includes logical contact history/free-stack order, contact
color lists, joint component processing order, fat bounds and epoch validity,
pending host forces/transforms, and physical step settings. Remaining allocator
relationships, command/event state, topology/geometry changes and other persistent
engine state still require a consumer-by-consumer audit. Schema v4 uses explicit per-world body creation ordinals and append-only
shape/joint creation ordinals; older schemas retain allocation-based identities. The initial 10-step ordinary-path probe passes: two core traces are identical,
and capture-enabled/disabled pose and velocity traces match each other and the
frozen GPU-order prefix. It captures 116 bodies (112 dynamic), 112 joints and
contacts on every step. Negative controls detect changes to sleep state, joint
impulses and contact impulses at the first changed frame, and reject truncation.
See `artifacts/gpu-solver-qualification/core-state-controls.json`. This initial
probe does not establish 600-step, five-run or cached-path full-state coverage.

The expanded ordinary trace passes two fresh 10-step runs without changing the
pose/velocity baseline. Corrupting logical contact history, contact color order,
joint component order or a fat-bound validity marker is detected at frame 3,
the first modified step. The preceding history/chain capture also completed a
200-step probe without invalid-reference failures. Evidence is in
`artifacts/gpu-solver-qualification/history-state-controls.json`; this is still
partial qualification, not the required five full-length repeats. The native
cached build separately passes two fresh 10-step expanded traces and the
capture-enabled/disabled pose/velocity control; see
`artifacts/gpu-solver-qualification/native-history-repeat-result.json`.

Ordinary and native cached builds each pass all 28 `native_precision::tests` with GPU ordering
selected (isolated shader/reference calculations, including two diagnostic
CPU-order algorithms). A selected set of 13 existing integration checks also
passes separately on both paths with unchanged assertions: support/stack reuse, sliding friction,
rebound, full mass lifecycle, prismatic/revolute/spherical constraints,
wakeup after velocity change, body/joint slot reuse, and CCD bounds/correction
persistence. These are partial capability evidence, not a full feature or
repeatability qualification. The audit found that `high_resistance_sleeper_wakes_on_velocity`
only checked the final awake state. It now also requires the fixture to be
asleep before the velocity change. The stronger test passes separately on ordinary and native cached paths. Evidence is in
`ordinary-precision.log`, `native-precision.log`, and the
`ordinary-integration-result.json` / `native-integration-result.json` reports under the new
qualification artifacts. The stronger transition evidence is recorded separately in
`ordinary-sleep-transition.log` and `native-sleep-transition.log`. The CPU Rain lifetime run has completed 600 health-valid steps,
covering 8,400 creation identities and 4,200 reused slots. The GPU run and
creation-matched comparison have now completed as described below.

The reverse contact-slot/history lookup is now normalized to its validated
logical history rank. A 10-step probe validates 2,453 mappings and leaves all
previously captured fields and pose output unchanged. The fresh-process runner
`scripts/check-core-state-repeats.py` freezes the executable, records its hash
and execution configuration, refuses to overwrite evidence, and requires at
least five runs. Five ordinary 600-step captures are currently running under
`ordinary-five-core`; their eventual result qualifies only the recorded state
until the remaining coverage audit is complete.

Schema v3 additionally reads the GPU body-extra storage: inverse-inertia
lower/upper off-diagonal terms, motion extents, sleep threshold, uploaded step
force/torque, and the host list controlling next-step force clearing. True
padding words are omitted. An integration control uploads nonzero loads, captures
them, steps again, and verifies they are cleared; this passes independently on
ordinary and native cached paths (`motion-state-load-test.log` and
`native-motion-state-load-test.log`). A 10-step Ragdoll capture preserves earlier
fields and pose output, and corrupted GPU force data is detected at the first
modified step (`motion-state-smoke-result.json`). The initial v3 smoke binary
called the upper inverse-inertia field `inertia_offdiag`; source now uses the
accurate name `inv_inertia_upper`. The ongoing five-run job is frozen on v2 and
does not qualify these newer fields.

Schema v4 gives bodies explicit feature-gated creation ordinals, including
when a slot is reused, and converts all body relationships to those identities.
Shape and joint storage is append-only; their creation ordinals are used instead
of generation/slot tuples. Body records are sorted by creation identity, while
`body_storage_order` separately retains processing order. Unknown island labels
are represented as null rather than inventing shared island membership.
A real create/step/delete/reuse/step test verifies distinct replacement identity,
stable survivor identity, shape correspondence and preserved processing order.
It caught and corrected a zero-based/internal versus one-based/public shape
mapping error in the new diagnostic code. All three trace unit/integration
controls pass separately on ordinary and native cached builds; see
`creation-state-tests-fixed.log` and `native-creation-state-tests-fixed.log`.
A 10-step full-scene v4 probe preserves all v3 physical fields after explicit
identity conversion and exactly preserves baseline pose output
(`creation-state-smoke-result.json`). Frozen v2 repeats do not establish v4
coverage. The first two frozen 600-step v2 traces match in every captured field
(`ordinary-first-two-result.json`). All five frozen-v2 ordinary runs have now
completed and match every captured field across all 600 frames
(`ordinary-five-core/result.json`). This does not qualify the newer v4 fields
or the remaining uncaptured persistent state, nor the native cached path.

The first frozen ordinary 600-step capture passes the existing Ragdoll stability
limits and exactly preserves the prior GPU-order pose/velocity trace. Peak joint
separation is 0.21331 m; final-two-second separation is at most 0.001390 m, with
zero recorded linear/angular speed in that tail. This is one-run stability
validation (`first-full-capture-health.json`), not five-run determinism evidence.

The creation-matched Rain comparison completed all 600 steps with both process
exit codes zero. It matches 1,948,800 joint observations, 8,400 created bodies and
4,200 reused body slots per engine. Identity matching passes. The unchanged
CPU-relative screen still reports 556 anchor and 3,019 angular joint-observation
failures. The first is frame 170, the right knee joining body creations 681/682:
GPU anchor error 0.0855421 m versus CPU 0.0017634 m. Peak GPU anchor error is
0.325546 m at frame 106 (CPU 0.325547 m for that joint), and peak GPU angular
error is 0.519477 rad at frame 328 (CPU 0.519512 rad for that joint). Comparable
peaks do not clear the remaining screening failures.

The frame-170 calf center-of-mass displacement differs vertically from a simple
trapezoidal integration of endpoint velocities by about 0.09366 m on GPU versus
0.02533 m on CPU. Endpoint velocities alone do not reconstruct all solver
substeps; this is a diagnostic lead consistent with a late position correction,
not proof of a CCD defect. Preserve and inspect a three-human Rain-cell impact
fixture before classifying this difference. Evidence: `rain-lifetimes-result.json`,
`rain-first-residual-window.json`, `rain-target-frames.json` and
`rain-first-residual-motion.json` in the qualification artifacts.

The latest v4 trace also retains local mesh material selection, joint reaction
frames, pending reaction frames, and force/torque event thresholds. Its 10-step
smoke probe preserves all earlier fields and baseline poses
(`joint-host-state-smoke-result.json`).

Schema v5 adds host shape definitions, filters, materials, mesh instances,
exact-content geometry tables and body allocation/free-stack order. Expanded
lifecycle tests pass independently on ordinary and native cached builds:
deleting a body preserves its survivor's identity, reusing its slot introduces
a new identity and geometry, and a friction update appears in the capture.
Two fresh five-step Rain impact replays match every v5 field and preserve the
earlier pose/velocity output byte-for-byte. Deliberately changing material,
geometry or allocation data at frame 3 is detected at frame 3. Evidence:
`shape-state-final-tests.log`, `native-shape-state-final-tests.log`, and
`shape-state-smoke/{repeat-result,negative-controls}.json`.

Schema v6 additionally reads collider and surface-material records back from
the GPU scene buffer. Host records alone cannot detect a stale upload. Uploaded
colliders retain semantic body/shape identities, physical fields, instance
transforms, initial ordering rank and resolved material values; visual color
and padding are excluded. The lifecycle test checks the uploaded survivor's
friction and material, replacement axis and new owner identity.
All three trace tests pass independently on ordinary and native cached builds
(`gpu-collider-state-tests.log`, `native-gpu-collider-state-tests.log`). Two
fresh ordinary five-step Rain replays match every v6 field, preserve all prior
v5 fields and poses, and detect a deliberately changed GPU material at frame 3
(`gpu-collider-state-smoke/{repeat-result,controls}.json`). These short probes
validate capture behavior; they do not replace full-length five-run checks.

Schema v7 reads the uploaded geometry and material-mixing regions in one GPU
readback, retaining live records and collider-relative array references.
Point/edge/vertex padding is excluded; plane offsets, triangle flags, BVH
integer metadata and the mixing table's occupied/empty probe layout are retained.
The latter affects its bounded lookup and must not be sorted away. Mixing keys
refer to the captured collider storage order. BVH metadata is decoded as integer
bits rather than incorrectly rejected as a non-finite coordinate.

The three trace tests pass independently on ordinary and native cached builds
(`gpu-geometry-mix-control-tests.log`,
`native-gpu-geometry-mix-control-tests.log`). A device-only hull-coordinate
change is observed while host geometry remains unchanged; NaN in unused point
padding is ignored. A real friction callback verifies uploaded mixing values
and removal of deleted-shape entries. Two fresh ordinary five-step Rain probes
match all v7 fields and preserve previous fields and poses. Frame-3 changes to
an uploaded vertex, BVH metadata or mixing entry are detected at frame 3
(`gpu-geometry-state-smoke/{repeat-result,controls}.json`). That probe exercises
84 hull points, 337 mesh vertices, 640 triangles and 158 BVH nodes; its hull
plane/edge/topology arrays are empty. The subsequent v8 convex fixture supplies
nonempty coverage for all four hull arrays and compares their uploaded values
exactly with its tetrahedron definition.

Schema v8 closes another omission found by following shader consumers:
`ShapeGpu._pad_filter` is not padding. Its two words select compound lifetime
bounds and public query results. The trace now records both as semantic shape
references. It also validates uploaded hull spans, mesh triangle/node spans and
triangle vertex references against live GPU array bounds. Four trace tests pass
independently on ordinary and native cached builds, including a device-only
invalid hull pointer that must fail capture
(`gpu-geometry-reference-controls.log`,
`native-gpu-geometry-reference-controls.log`). This does not yet validate every
BVH child or half-edge relationship.

The scene-buffer consumer audit now has the following coverage:

| Scene data | Capture coverage |
|---|---|
| Body cold data and motion extras | Existing body and v3 motion capture |
| Collider metadata and surface materials | Actual GPU readback in v6; v7 geometry references; v8 bounds/query references and geometry span checks; deeper topology checks pending |
| Hull points, planes, edges and topology | Host geometry in v5; uploaded live arrays in v7; nonempty exact-value runtime control passes in v8 |
| Mesh vertices, triangles and BVH nodes | Host geometry in v5; uploaded live arrays and packed metadata in v7 |
| Material mixing lookup table | Actual uploaded entries and probe layout in v7, lifecycle callback control passes |

This table covers scene storage only. Persistent broadphase/allocator state,
graph memoization, pending events/commands and other host state still need the
broader consumer audit. None of v5 through v8 is a complete-state qualification;
the five frozen-v2 repeats do not qualify their added fields.

Following `dynamic_graph_pipeline` and `collide_finalize_pass` shows that the
shared graph memo requires zero joints and zero mesh triangles; the batched path
has separate selection rules, including large body counts. These conditions
must be evaluated for each tested configuration, rather than treating all
graph cache memory as unused. Separately, `configure_idle_step`/`world_step`
can skip sleeping-world work based on host proof, epoch and context state.
Schema v9 now captures that idle proof/chain, logical revision contexts and
mutation epoch, eligibility, validity step, pending-context metadata and last
step decision. Capture starts with `b3_world_gpu_wait_with_mirror`, whose queue
wait drains contact status through `finish_contact_status`; this is a
synchronized step-boundary capture, not a snapshot of an in-flight submission.
Five idle-resident tests pass independently on ordinary and native cached paths
(`idle-state-controls.log`, `native-idle-state-controls.log`). They exercise
delayed proofs, buffer growth, velocity/force/transform/lifecycle mutations,
unchanged state across skipped steps and wake-up agreement with ordinary contact
history. Added assertions verify a sleeping proof exists and a waking impulse
changes its epoch and resumes physics.

Two fresh five-step ordinary Rain captures match all v9 fields, preserve prior
fields and poses, and detect an epoch change at frame 3
(`idle-state-smoke/{repeat-result,controls}.json`). Five fresh 120-step cold
Rain elbow replays now pass every captured v9 field and frame
(`idle-five-elbow/{manifest,result,coverage}.json`). All five processes exit
zero and their pose outputs are byte-identical. There are 28 sleeping bodies
at captured step 39 and all 42 dynamic bodies sleep by step 120. This is the
ordinary library with the native-sample shader policy, not native cached
qualification. Idle eligibility is false throughout: `can_submit_resident`
excludes worlds with live joints. Therefore these repeats cover settling, not
idle-proof reuse; the dedicated idle-resident tests supply that separate
coverage. The fixture starts with fresh contact/joint history, not a full-state
restore of the original Rain run.

Schema v10 adds actual graph-cache readback. Shared memo capture retains
input/output color counts, body masks and edge validation/color records.
Batched memo capture retains each batch's valid records, masks and choices.
Both retain stale entries that a future dispatch can inspect, along with
scheduling policy/hints; hit/miss counters and padding are excluded. Cache
indices are processing/allocator-slot references, not external object handles;
their relation to allocator lifetime remains part of the broader audit.
The readback rejects unknown layout tags and out-of-range counts/storage.

Four ordinary trace tests pass (`graph-cache-state-tests.log`). Three graph
mutation tests pass independently on ordinary and native cached paths
(`graph-cache-mutation-controls.log`,
`native-graph-cache-mutation-controls.log`), exercising shared, batched and
switching layouts with filtering, transforms, shape replacement, body-slot
reuse, body-type changes, sleeping/waking and buffer growth. The controls
require observing the relevant cache layouts and preserve the existing exact
contact schedules and numerical trajectory checks against canonical coloring.
The five v9 replays do not qualify these new v10 fields. Broadphase allocator
state, pending events/commands, remaining host/CCD state and complete-state
repeatability remain open.

Schema v11 preserves physical contact allocation relationships that the sorted
manifold records previously hid: the monotonic high-water mark, slot-to-logical
contact mapping including holes, generation counters surviving retirement, and
published occupied-root order. These are future allocation/reuse inputs rather
than external contact handles. Capture rejects live slots above the high-water
mark, duplicate occupied entries and child patches in the root list.
Five trace tests pass independently on ordinary and native cached paths
(`contact-reuse-state-tests.log`, `native-contact-reuse-state-tests.log`). The
new lifecycle control removes a touching body, verifies retirement without
resetting allocation history, then replaces it in the same body slot and checks
the new contact identity and advanced contact generation.

Two fresh ordinary five-step Rain probes match all v11 fields, preserve prior
v9 fields and poses, and detect generation/order mutations at frame 3
(`contact-allocation-smoke/{repeat-result,controls}.json`). Their high-water
mark reaches 51 slots with up to 19 holes, so this exercises actual retirement
and reuse rather than an empty allocator.

Schema v12 captures the contact hash as sparse bucket positions, semantic root
references and tombstones, with empty buckets implicit. It checks for duplicate
or invalid roots and mismatches between hash identity words and contact pairs.
Unreferenced identity words are excluded because `prepare_contact_identity`
overwrites them before publication. The previous-touching event region is
decoded through `previous_contact_shape_ids` and previous compound-child
ordinals, retaining deleted-shape identities with explicit liveness. Current
and previous event maps are both captured; decoding through current live shapes
alone would lose deletion/remapping history.

Five trace tests pass independently on ordinary and native cached paths
(`contact-hash-event-lifecycle-tests.log`,
`native-contact-hash-event-lifecycle-tests.log`). The deletion/reuse control
also verifies removal of retired hash lookups while the previous event map
retains the deleted shape. Two fresh ordinary five-step Rain probes match all
v12 fields, preserve every prior v11 field and pose, and detect hash-bucket and
previous-shape-validity changes at frame 3
(`contact-hash-event-smoke/{repeat-result,controls}.json`). This includes up to
14 tombstones and nonempty previous-touching history. Pending host event queues,
contact registry/handle relationships and remaining host/CCD state still need
capture and validation. This remains partial-state evidence.

The host-registry audit found an observable repeatability defect rather than
only another capture omission. `update_registry` allocated new handles while
iterating a randomized `HashMap`, then body/shape getters sorted by those
handles. Eight identical three-support worlds returned different pair orders,
including `[1,3,2]` and `[2,3,1]` (`contact-query-order-before.log`, exit 101).
New handles are now allocated in sorted public pair/compound-child order;
existing handles and their lifetime rules are preserved. This adds a host
registry-key sort during snapshot construction, not CPU-order emulation in the
GPU solver. Its performance cost is not yet measured separately.

The regression passes independently on ordinary and native cached paths
(`contact-query-order-after.log`, `native-contact-query-order-after.log`), and
the existing cache/pointer/lifetime/world-reuse test passes on both
(`contact-registry-lifetime-after.log`,
`native-contact-registry-lifetime-after.log`). These are repeated worlds in one
process, not the required fresh-process full-state qualification. The regression
has been moved into the normal contact-API test suite so it does not require
the diagnostic capture feature; ordinary and native cached reruns without that
feature pass (`contact-query-order-default-tests.log`,
`native-contact-query-order-default-tests.log`). Earlier frozen replay binaries do not contain
this new host-side correction. Registry and pending-event capture remain open.

The event audit found two further host defects. Iterating the previous-touching
`HashMap` published end events in different orders across identical worlds
(`contact-end-order-before.log`, exit 101). Sorting alone then exposed duplicate
ends on refilter: three deferred host transitions plus the same three transitions
reconstructed from GPU history (`contact-end-order-after.log` and
`native-contact-end-order-after.log`, exit 101). Deferred events now retain their
public pair/child key, are sorted at publication, and share per-step key
suppression with GPU-derived ends. Normal ends also use sorted logical keys.
The suppression resets each positive-duration step; it is not a permanent
filter on later valid transitions.

The final ordinary and native cached regressions each pass eight fresh worlds
for teleport, deletion and refilter, requiring exactly three ordered ends.
Restoring the filter must produce three begins and no stale ends. Late event
reads and compound/public-shape mapping with allocation holes also pass on
both paths (`event-key-final-*.log`, `native-event-key-final-*.log`). These are
host API regressions, not five fresh-process complete-state qualification.
Earlier frozen replay binaries also predate these fixes. Their performance
cost remains to be measured with the final candidate.

Schema v13 adds published contact-end endpoint order/validity, deferred end
payloads with logical pair/child keys, and the per-step suppression set. Deferred
queues are canonicalized by the publication key because enqueue order is
sorted before use; published event order remains observable and is preserved.
The deletion/reuse trace control enables real contact events and verifies a
queued end retains its deleted endpoint, publishes once, drains the deferred
queue and clears suppression on the replacement step. It passes ordinary and
native cached builds (`contact-end-state-final-test.log`,
`native-contact-end-state-final-test.log`). Seven synthetic comparator controls
verify acceptance of identical payloads, rejection of missing required lists,
and detection of changed published order, deferred endpoint validity and
suppression child keys (`contact-end-state-comparator-controls.json`). They are
comparator checks, not runtime repeatability evidence. Opaque contact-handle
alias relationships, registry state and other host event queues still need
capture; v13 is explicitly partial and has not received fresh-run qualification.

Schema v14 adds host contact-registry entries, sorted physical slot/generation
ownership sets, finite decoded manifolds, live-contact flags, body/shape getter
order, handle lookups and snapshot validity revisions. Begin/end/hit/deferred
contact events retain their endpoint identities and handle relationships; hit
positions, normals, speeds and material identifiers are included. Opaque handle
numbers are mapped to trace-local identities on first observation in semantic
order, with that mapping retained across frames. This allows different global
token numbers without hiding an event that references a new or retired handle.
Only meaningful manifold points are captured; unused capacity and pointers are
excluded. The diagnostic map exists only with `replay-diagnostics` enabled.

The lifecycle test verifies that the initial begin and query registry share a
handle, the end keeps it after deletion, and the replacement receives another
handle. Injecting a wrong begin-event handle while retaining the same shape
endpoints changes the capture. This passes ordinary and native cached paths
(`contact-registry-alias-control.log`,
`native-contact-registry-alias-control.log`). Other host event queues and
remaining host/CCD state still need audit; this is not complete-state capture.
Fresh-process lifecycle repeats are tracked under `contact-registry-five`. Four comparator controls
on the first actual v14 trace detect changed retired-handle identity, cached
normal impulse and getter membership, and reject a missing handle lookup
(`contact-registry-five/comparator-controls.json`). The default-feature library
check also passes (`contact-registry-default-check.log`).

The frozen v14 lifecycle batch passes five fresh processes independently on
ordinary and native cached paths, comparing all captured fields across each
fixture's three steps (`contact-registry-five/{ordinary,native}/result.json`).
Each path has its own frozen binary and recorded runtime policy.
These short deletion/reuse runs do not replace full-length Falling Ragdolls
or Rain qualification, and their binary does not include later capture additions.

Schema v15 adds sensor overlap lists, ordered sensor begin/end/deferred queues,
continuous sensor-hit pairs, body movement transforms/sleep transitions and
joint event identities. User pointers are excluded. A real sensor-overlap and
distance-joint fixture verifies begin roles, a deleted sensor's deferred end,
queue draining, body movement and a joint threshold event. It also rejects an
injected nonfinite movement transform. This passes ordinary and native cached
paths (`host-events-state-test.log`, `native-host-events-state-test.log`).
The full lifecycle exporter passes both paths (`host-events-full-trace-test.log`,
`native-host-events-full-trace-test.log`), and the native export preserves all
prior v14 fields across all three frames. Comparator controls detect a changed
movement position and reject a missing sensor queue
(`host-events-trace-controls.json`). Nonempty continuous-sensor-hit capture
still needs a targeted control. This remains partial-state evidence.

Remaining host-state audit items include CCD step-start body snapshots,
world definition fields not represented by GPU parameters (including the hit
event threshold), callback registration/configuration, execution-policy flags,
cached topology/capability proofs and query-index contents/validity. Their
meaningful state must be captured or explicitly justified as overwritten or
incidental before calling the trace complete. GPU cache/geometry audit gaps
and full physical/performance qualification also remain open.

Schema v16 adds the full host world definition and capacities, execution-policy
flags, cached topology/capability decisions, CCD cache key, substep/reaction
scales, synchronized failure/mirror flags, and the host query index with its
shape geometry, grouping, revisions and BVH nodes. Query geometry is interned
by exact content, not allocation addresses. CCD step-start storage retains its
slot order, length and consumed pose/origin fields. The CPU body mirror is
captured separately from GPU state, including host-write and snapshot epochs;
otherwise a stale mirror could be hidden by correct GPU poses. Callback presence
is recorded, but code identity and callback-owned context are external inputs
and are not serialized generically.

Targeted tests use the host CCD path on each compiled backend and verify
nonempty CCD history/query geometry/tree capture. Deliberately changed hit
threshold, CPU mirror velocity, CCD starting position, query geometry and a
cached topology decision all change the capture; a nonfinite CCD rotation is
rejected. The controls and full lifecycle exporter pass independently on both
paths (`{native-,}host-state-final-{controls,integration}.log`), and the default
library check passes (`host-state-default-check.log`). The native three-step
export preserves every prior v15 field; actual-trace comparator controls detect
changed hit threshold/mirror velocity and reject missing CCD history
(`host-state-trace-controls.json`). These tests do not establish fresh-run
repeatability for v16 or complete the GPU persistent-state audit. Remaining
geometry topology validation, nonempty continuous-sensor-hit capture, external
callback/input contracts and complete-state coverage review must precede the
full-scene repeats. Physical, performance and recording gates remain open.

The continuous-sensor capture gap now has a real crossing control: a box travels
from x=-1 to beyond x=0.5 in one step through a thin sensor, with neither endpoint
overlapping it. Both compiled paths report exactly one continuous hit/begin,
allow the body through, then clear the hit queue and emit one end on the next
step (`continuous-sensor-state-control.log`,
`native-continuous-sensor-state-control.log`). Sensor worlds use the supported
host CCD fallback; this is not evidence for a resident GPU sensor-CCD path.

Geometry capture now validates per-mesh BVH child relationships and triangle
leaf coverage, plus hull next/twin/vertex/face references and twin endpoint
consistency. This is structural validation, not a convexity or collision-quality
proof. Pure controls reject malformed children, duplicate/out-of-range leaf
spans and invalid hull references. A real uploaded 18-triangle mesh with a
branching tree captures successfully; corrupting only its GPU root child causes
capture to fail. The nine-test ordinary trace suite and separate device-mesh
control pass (`geometry-relationships-trace-tests.log`,
`mesh-tree-device-control.log`); all ten tests in the expanded native suite
also pass (`native-geometry-relationships-trace-tests.log`).

The next persistent-state review must explicitly classify remaining `GpuSim`
state, including host sleep-threshold/fat-geometry metadata, effective solver
policy/capacity parameters, native command/replay cache keys and convex CCD
buffers. Already captured graph memoization and idle proofs do not by themselves
cover every cache. Synchronization/metrics fields must be distinguished from
state consumed by future steps before complete-state qualification is claimed.

Schema v17 adds effective GPU parameters (including relative buffer-layout
indices and capacities, excluding the padding word), cached sleep thresholds
and fat-geometry keys, execution policy, synchronization epochs and sticky
capacity-loss metadata. Native captures include command-cache keys and whether
they reference the current bind group, plus the consumed replay-key bytes
without raw GPU handles or the parameter padding word. The native lifecycle
fixture populates radix, contact, graph and full-replay caches; ordinary builds
explicitly record no native cache state. This does not yet capture the separate
convex-CCD geometry buffers or prove command-cache construction correct.

Lifecycle export and controls pass independently on ordinary/native paths
(`gpu-policy-final-controls.log`, `native-gpu-policy-final-controls.log`).
Deliberately changed cached sleep thresholds, fat geometry and pair-policy
flags are observed, and nonfinite cached thresholds are rejected. The native
three-step export preserves every prior v16 field; comparator controls detect
changed capacity and replay binding relationship and reject missing fat
geometry (`gpu-policy-trace-controls.json`). The default library check passes
(`gpu-policy-default-check.log`). Five fresh lifecycle repeats per path completed under `gpu-policy-five`;
both binaries, configurations and source hashes were frozen before the first
run. This remains partial-state validation.

The frozen v17 ordinary and native lifecycle batches each pass all five fresh
processes with exact three-step traces (`gpu-policy-five/ordinary/result.json`
and `gpu-policy-five/native/result.json`). These predate the CCD and corrected
semantic-lane capture below; they are not current full-state qualification.

Schema v18 reads the separate convex-CCD device buffers: points, shape filters
and radii, body extents/centers and index spans, target/joint indices, configuration
and consumed start poses. Counts and references are validated before publishing
the trace. Unused shape fields/vector lanes/uniform padding are excluded.
Readback buffer usages and retained diagnostic references are feature-gated;
the normal buffers retain their prior usage. The trace also records the actual
selected adapter/backend/vendor/device and both driver strings, rather than
inferring adapter selection from host inventory.

Both compiled paths pass a control explicitly enabling GPU convex CCD: editing
only a device point changes the capture while an unused NaN lane is ignored.
Structural controls reject invalid body/config counts, shape/point/index
references and overflowing joint ranges (`ccd-state-final-controls.log`,
`native-ccd-state-final-controls.log`). Full lifecycle export passes on both
paths (`ccd-full-state-test.log`, `native-ccd-full-state-test.log`), and the native
export preserves every prior v17 field. Comparator controls detect altered CCD
points/indices and reject missing configuration (`ccd-state-trace-controls.json`).
The actual selected adapter is RTX 4070 Laptop, Vulkan, NVIDIA driver 610.57.04.
The default library check passes (`ccd-state-default-check.log`). This still
requires a final field-by-field persistence audit and full-scene repeats; these
controls do not prove the larger physical or performance gates.

Schema v19 corrects semantic data hidden behind legacy padding names. Motor
joints store two components of their accumulated angular spring impulse in
`_pad2`; these are now captured with the first component. Contact feature IDs
occupy `_tail[6..8]` and the raw bits of `_pad_ca/_pad_cb`, while `_tail[4..6]`
holds SAT cache data. The earlier trace mislabeled SAT words as feature IDs
and missed some feature/cache state. The new trace separates them, including
SAT data independently of manifold point count. Body allocation now also
captures free-slot generation history, which determines handles after reuse.
These are diagnostic corrections, not changes to solver arithmetic.

Ordinary and native controls pass for legacy-lane decoding, actual nonzero
motor spring impulses, and contact retirement/body reuse. A NaN in a consumed
motor lane is rejected; bit-pattern feature IDs are preserved as integers.
The native three-frame lifecycle export preserves every prior field except
the intentionally corrected feature IDs. Comparator controls detect changed
SAT data, feature IDs and generations, and reject missing SAT/generation data
(`semantic-state-trace-controls.json`). Runtime evidence is in
`motor-spring-state-control-final.log`, `semantic-state-integration.log` and
their `native-` counterparts; decoding evidence is in
`semantic-padding-state-control.log` and its native counterpart. Full-scene
current-schema repeatability, remaining persistence/reset contracts and the
physical/performance gates remain open.

Pass uniforms are rebuilt from effective `SimParams` by `upload_pass_lut` before
ordinary stepping and before cached full replay. Immutable program definitions
are tied to the recorded build. The completeness review must still verify
reset/overwrite contracts for remaining scratch/indirect regions and account
for external callback state as replayed input, rather than silently treating
all unrecorded data as irrelevant.

The scratch review has established the following reset/consumption contracts
in the ordinary dispatch and native tail/graph command builders. These are
source-audit findings, not a blanket proof that every scratch region is safe.

| Region | Reset/consumption contract |
| --- | --- |
| Island labels/wake/ready | `island_init` writes every current body before union/wake/prepare on both paths. |
| Component counters and offsets | `component_reset`, count, offsets, color offsets and scatter precede component solving. `component_offsets` writes query words 256–260, including all three large-component dispatch dimensions; the host/native builder copies words 257–259 immediately before indirect dispatch. |
| Component collection order | Small-component contacts sort by color and graph-local rank before solving. Large components use independent color waves and the established graph list for serial overflow. Body collection order only selects independent per-body integration work. Correct coloring remains a separate invariant. |
| Joint component lists | `prepare_joint_components` runs heads reset before compaction for nonserial joint worlds. Heads/count/list-valid are reset, then bounded component lists are rebuilt. |
| Graph construction metadata | For selected configurations with `DIAG_REBUILD_GRAPH` unset, both ordinary and cached builders reset all24color counts, per-body masks/degrees and fused flags/slots before classification. Paired histogram/bases/scatter overwrite current prefixes and bounded lists, including zero counts. Static degree finalization writes sort bounds/dispatches; degree-one and sorted higher-degree paths populate current color lists. Dynamic greedy/shared/batched paths consume those initialized masks and bounded lists, then overwrite final counts/dispatch arguments. Persistent memo inputs remain captured under their separate validity contract. Source hashes/dataflow: `desktop-baseline/graph-workspace-state-audit.json`. This does not classify diagnostic rebuild/proof modes or unrelated scratch regions. |
| Pair candidate/radix/allocation workspace | Ordinary and selected native=2/contact-cache=1 builders order reset/collection, candidate append, digit histogram/bases/prefix/scatter, unique compaction, existing-slot lookup, missing list and free-slot scan before consumers. Producers write current prefixes and consumers use clamped live counts; empty-input radix emits one zero histogram group and unique compaction scans zero groups. Missing-pair scatter completes before free scan reuses bases. Persistent contacts/hash/generations/occupied/event/fat-bound inputs remain separately captured. Source contracts and hashes: `desktop-baseline/pair-workspace-state-audit.json`. Full-width pair radix and poisoned-workspace compaction checks pass on both backend builds; these direct-dispatch checks do not by themselves validate native recorded sequence execution. The existing80step matrix/grid mutation check also passes on both paths: exact pairs/schedules and finite body p/q/v/w within1e-5 through filtering, replacement and capacity growth. The discovered static-list overflow omission is now reproduced/fixed and boundary-tested below. |
| Pair-matrix scratch | The matrix path (at most 704 shapes, no mesh triangles) writes every live matrix word, merges valid previous pairs, writes every row count and prefix, then scatters bounded pairs. Ordinary and cached command sequences preserve this order; bases overwrite collision/prepare/radix indirect counts. Matrix scratch is not carried as physical history; the previous-contact inputs are captured separately. |
| Shared graph memo | `graph_assign_dynamic_memo` checks layout tag, body/edge counts, initial body masks and color counts, then ordered slot/endpoints/pair identities before reusing output. Rebuild restores initial inputs before the greedy walk; partial reuse reconstructs mask union and count maxima. Captured memo records retain all these inputs/outputs and stale entries; the omitted hit counter is not a solver input. |
| Batched graph memo | `graph_assign_dynamic_batched` clears every cached batch count on layout change, resets per-batch keys/counts, validates count/endpoints/initial masks, and reapplies relative local indices to current color starts. Capture includes each batch count and all six record words. Hit/miss counters are diagnostic only. |
| Active-body count | Query word 72 is explicitly cleared before counting on eligible steps; `idle_count_valid_step` is invalidated at step entry and set only for a produced count. Persistent idle proof metadata is captured. |
| Pass uniform lookup | `upload_pass_lut` writes every bias/color row from captured `SimParams`, replacing `color_select` and `use_bias` with the row coordinates. Both normal stepping and a native full-replay hit upload it before execution. Prior lookup bytes are not a future-step input. |
| Indirect argument staging | `indirect` is a copy destination, not a shader-persistent store. Broadphase writes insertion/occupied counts before copying scratch words8–15; radix and allocation stages write their counts before copying words40–43 and48–51. Pair-matrix bases provide the corresponding initial collision/prepare/radix arguments. Graph finalization writes solve/prepare/island arguments before copying words8–11,40–47 and all color records from64 onward; static-degree finalization writes words59–62 before static sorting. The large-component path copies query257–259 immediately after component offsets. `write_group_indirect` writes all three dimensions plus padding, including zero-work branches. Jacobi does not consume the unproduced per-color arguments. |
| Native indirect copies | Cached broadphase/contact/graph command lists retain the same producer → `CopyArgs` → indirect consumer sequence. `RadixCache::record` barriers cover shader writes → transfer reads and transfer writes → indirect reads, and retain referenced buffers. Full-step replay re-executes the recorded producers/copies; it does not freeze old argument values. This classifies the indirect destination bytes, not every scratch producer's input. |
| Jacobi velocity accumulation | Each `dispatch_wave` in solver mode1 dispatches `jacobi_clear` for every current body and all six integer velocity/angular-velocity lanes, then `solve_jacobi`, then `apply_jacobi`. No accumulated lane from the previous wave is consumed. This region aliases graph workspace, whose separate initialization contract still applies before graph consumers. The source contract applies to this alternate solver path; it does not qualify its numerical behavior. |
| Per-step body deltas | `reset_deltas` writes zero translation and identity rotation for every current body before broadphase/preparation/substeps. Native reset commands contain the same dispatch; a full replay records and executes it. Idle steps omit physics under the separately captured idle proof. Deltas are also captured in body state; this overwrite does not justify omitting other body fields. |
| Query request/result workspace | Each ray request overwrites all32words (inputs, sentinels and zero result/overflow lanes), executes ordered reduce/pick/commit passes, then waits/maps its result. Contact retirement now overwrites separate command words32–34 before dispatch; completed-state capture waits for submission and rejects undrained status. These request/result bytes do not persist as future-step inputs. Sticky reason word66 is separate; host status masks are captured after explicit refresh. Retirement, shape remapping and hash republication mark status dirty; a fresh copy/map clears it, while an older pending harvest does not. Explicit finish refreshes dirty status even at an already-seen physics step; capture rejects pending or dirty status. The actual same-step retirement regression passes with empty and older pending slots on both paths. Metrics67onward, high-water and graph/component regions have their own contracts. Source hashes: `desktop-baseline/query-workspace-state-audit.json`. |
| Atomic header and spatial-hash workspace | `clear_broadphase` resets header words0–5/7 before all candidate paths, including matrix replay; occupied collection resets6 and allocation preparation resets15 before its free-list ticket consumer. Sticky words8–13 are copied/drained into captured failure causes/first-step/invalid state. Word14 has no reader/writer beyond declaration. Spatial hash buckets are cleared across max(HASH_BUCKETS,pair_capacity/4); the max(contact,hash,pair) dispatch covers that span. This is separate from the persistent contact-key hash, which remains captured. Source hashes/contracts: `desktop-baseline/atomic-header-state-audit.json`. |
| Joint device records | All43JointGpu fields are mapped:36direct exact fields,2body identity references, motor-only _pad2 lanes captured through motor_spring_angular_impulse, and4padding fields without consumers. read_joints copies the actual joint_count prefix. Constructors append initialized records; destruction replaces records with JOINT_NONE and marks scene dirty. Solver/filter/graph consumers skip none records; captured count and live slot identities retain holes. Active motor-state and legacy-lane checks pass both paths on current frozen v20 tests. Evidence: desktop-baseline/joint-record-state-audit.json. Host metadata/graph contracts remain separate. |
| Separate convex CCD buffers | Actual five device input buffers and start states are read. Config counts, all meaningful point/body/shape fields, indices and start pos/rot/origin/valid are captured; shader consumers use no omitted vector/padding/start-body lanes. Normal stepping captures the start prefix before reset/integration and corrects after integration; full replay records the same copy and dispatch. Replacing CCD buffers invalidates native replay. Device-only geometry and bad-reference controls pass both paths. Evidence: desktop-baseline/convex-ccd-state-audit.json. This is data coverage, not physical CCD qualification. |
| Retired contacts and chain allocation | Retirement resets hot/prepared/persistent records while preserving each physical generation; mesh scratch follows the same rule. This reset also matters because root publication reads previous.color. Capture retains holes/counters, topology through semantic ordinals and chain lengths, root ownership, hash/history and schedules. High-water/free-prefix and valid/malformed retirement/solver-chain checks pass four tests per backend on frozen v20 binaries. Evidence: desktop-baseline/contact-storage-state-audit.json and contact-storage-audit/. Initial zero-test selection was rejected and preserved separately. These direct-dispatch checks do not prove native recorded execution or complete child-slot permutation equivalence. |
| Live contact fields | All47assembled Contact fields mapped to exact capture or explicit count/reset contracts in desktop-baseline/live-contact-record-state-audit.json. Matching/recycling/solver accesses to old anchors and impulses are count-bounded; all-lane feature/triangle mode inputs are captured in v20. Temporary mesh pair.zw links are cleared before publication. This field mapping does not justify child placement normalization. |
| General readback and output transport | General staging is MAP_READ/COPY_DST only; each readback copies every decoded prefix/packed region and waits/maps/unmaps before return. Stale capacity tails are not decoded. At accepted completed-world capture, the retained submission index selects only an already-completed wait; logical completion state remains captured. Separate pose-export and private render-copy destinations receive body/cold copies and never feed the solver; their handles/semaphores govern output transport/timing. Source contracts: `desktop-baseline/readback-output-state-audit.json`. This does not prove renderer correctness or arbitrary standalone/mid-flight APIs. |
| Pose readback staging | Completed world capture first refreshes/supersedes staged poses and rejects pending != consumed. lock_pose_snapshot cannot read old staging or slot metadata afterward; the next kick overwrites the full required body prefix, current step and logical epoch before making a slot readable. Slot parity only selects a buffer; policy switches discard pending copies. Simulator pose_epoch/pose_step, world automatic policy and per-body host edit epochs remain captured. Maps/kicks are readback diagnostics with no physics consumer; capacity only controls allocation/wait. Boundary rejection and policy-switch regressions pass on both paths. Source hashes/contracts: `desktop-baseline/pose-readback-state-audit.json`. This excludes staging only at this boundary, not arbitrary mid-flight snapshots or other render state. |
| GPU timing ring and measured durations | Step timestamps add query writes/resolves and staging copies; readiness chooses a timing staging slot and the latest profiling result only. No physics dispatch policy consumes measured values or ring readiness. Harvesting invokes sticky-status harvesting unconditionally before its timing branch; completed-state capture separately drains that status. Exclude timing payloads, associated measurement-step metadata and mirror timings under the explicit timestamp exclusion. Active timing-window counters remain captured because they suppress full replay. Desktop source hashes/consumer summary: `timestamp-state-audit.json`. |
| Terminal simulator failure | For world-boundary captures, every simulator invalidation propagates to captured `host_state.physics_invalid` under the world lock before return. Callback-disable and fat-transform errors mark/propagate both states; material-mix failure marks both; simulation recreation inherits world failure. The world flag is absorbing and blocks further physics submission. Thus the simulator's duplicate invalid flag is not independent future state for this capture API. This argument does not cover standalone `GpuSim` snapshots. Source hashes/call sites: `desktop-baseline/terminal-failure-state-audit.json`. |

The pass/indirect, delta, timing and terminal-failure rows were re-audited on the desktop continuation at `2f96608`.
The existing `tiled_dispatch_direct_indirect_and_cached_cover_each_element_once`
test passes independently on both paths at0,1,65,535,65,536 and65,537workgroups,
with exact once-only writes and untouched padding; native also exercises the
cached direct/indirect command implementation. Logs are in
`desktop-baseline/indirect-dispatch-{ordinary,native}.log`. This validates grid
mapping and copy/barrier execution, not the whole physics reset audit. The
source argument above covers destination overwrite; remaining scratch/atomic
regions, semantic slot equivalence and complete-state qualification stay open.

The graph-memo source audit and source hashes are recorded in
`graph-memo-state-audit.json`. The existing batched-cache input-change and
slot-reuse regression passes ordinary/native with memoization enabled
(`state-audit-batch-memo-{ordinary,native}.log`). The broader greedy-color
reference regression, including holes, overflow, cache-prefix changes and
shared/batched size boundaries, also passes on both paths
(`state-audit-shared-memo-{ordinary,native}.log`).

Current v19 Falling Ragdolls qualification has started under
`ragdoll-v19-five`: both fixture binaries were frozen before any run, with
source/library/object identities recorded in `build-manifest.json`. The batch
requests five fresh 600-step processes independently per configuration. These
runs compare all current captured fields, but do not close remaining capture
coverage or physical acceptance gates. Ragdoll worlds contain joints, so the
full-step native replay eligibility gate excludes them; native substage caches
remain applicable. Separate no-joint coverage is required for full-step replay.

The native full-step replay control now exercises a three-box contact stack
for 48 steps with sleeping disabled and component TGS enabled. It captures each
step and asserts sustained replay before and after substep changes (steps 20
and 24) and a body transform upload (step 32). The first run passes with 42
replay hits (`native-full-replay-v19-control.log` and `.jsonl`). Five fresh
processes use a frozen test binary under `native-full-replay-v19-five`; all
five exit successfully and match every captured field over 48 steps
(`result.json`). This control proves the requested replay branch executes;
it is not a long-duration stack stability or performance benchmark.

The isolated upstream Rain cell now reproduces the frame-170 right-knee spike:
GPU CCD gives 0.0855092 m anchor error, versus CPU 0.0018373 m. With continuous
collision disabled, GPU gives 0.0003862 m, but earlier impacts also change, so
this ablation is diagnostic only. All four CPU/GPU CCD-on/off processes exit
zero with 8,148 finite body records each. The fixture computes anchor distances
directly from upstream Human local frames. These are ordinary-build captures
using the native sample shader policy, not native cached qualification runs.

A separate host-side CCD run also exits zero and matches all 8,148 GPU-CCD
pose/velocity/anchor records byte-for-byte. Its trace shows the calf's proposed
movement at step 171 (frame 170) clamped to fraction 0.3512181. The observed calf
contact begins at frame 170 on CPU and 171 on GPU. This narrows the investigation
to shared CCD/contact behavior; it does not yet distinguish a legitimate
trajectory-dependent threshold crossing from a missed contact or incorrect TOI.
Next compare collision generation at identical frozen poses before changing the
solver. Authoritative evidence is in `rain-cell-validated/result.json` and
`rain-cell-validated/host-ccd-result.json`; earlier `rain-cell/` trial captures
include a cleanup failure and stale API stub and are not qualification evidence.

The subsequent frozen calf/torus probe compares twelve identical input poses
from CPU/GPU frames 166–171, using fresh calf lifetimes, zero velocity/gravity,
disabled CCD and a 1e-6 second discrete step. Contact presence agrees in every
case: both engines report no contact at the GPU frame-169 pose. Thus the observed
one-frame onset difference is not itself evidence of a missed pair. However,
GPU omits one speculative point on torus triangle 412 at the CPU frame-169 and
GPU frame-170 poses (CPU separations 0.010835 m and 0.013216 m). The other
manifolds remain. Investigate this narrower discrepancy before classifying it
or attributing the knee spike to it. See `rain-frozen-contact/result.json` and
`cpu-features.txt`; this is a collision diagnostic, not a physical-quality pass.

The raw WGSL triangle routine does generate the missing point in both cases.
The mesh admission helper then rejects feature 2 with triangle flags 116.
This exposes a contextual filtering defect: GPU rejects a flat edge outright,
whereas upstream rejects a tentative flat-edge manifold only when an accepted
manifold already covers that shared edge (and separately rejects fully flat
triangles). The correction must retain suppression of covered seams while
allowing exposed edge contacts; simply disabling filtering is insufficient.
The initial helper records failed admission in
`rain-frozen-contact/triangle-helper.log`. A candidate production correction now
stages capsule triangle manifolds before clustering, establishes face coverage
using shared vertex-index pairs, filters tentative flat-edge contacts, and then
clusters accepted manifolds. Its isolated raw-manifold, exposed-edge,
covered-seam and fully-flat controls pass (`triangle-admission-fixed.log`).
Full frozen-pose, Rain-cell and manifold-lifecycle runs are in progress; broader
regressions, native cached validation and performance measurements remain
outstanding. In particular, the new staging allocates temporary contact slots
and scans local contact candidates, so capacity behavior and cost need checking.
This is a candidate correction, not yet an accepted qualification result.

Candidate runtime results: all twelve frozen torus cases now match CPU contact
and point counts, with maximum normal/separation component difference 1.49e-7.
The 210-step Rain cell completes with 8,148 finite records; frame-170 knee error
falls from 0.085509 m to 0.005417 m and peak anchor error from 0.299920 m to
0.202738 m. However, the unchanged CPU-relative +0.05 m screen increases from
two to four failing observations, starting at frame 191. These results do not
clear Rain. See the two `admission-fixed-result.json` files under
`rain-frozen-contact/` and `rain-cell-validated/`.

A separate 45-pose flat-grid control rejects the candidate: thirteen cases have
extra contact points, and several produce angled contact normals on the flat
interior surface. Evidence is `rain-flat-seams/result.json`. Its body creation,
deletion and fresh contact lifetimes also exercise recycling. A diagnostic
coverage-mask trace is running to distinguish incorrect coverage tracking from
incorrect triangle face classification. The fix remains unaccepted pending
this investigation; manifold-lifecycle tests are still running.

The coverage trace identifies an implementation error in the first candidate:
the filter rejects the unwanted fully flat triangle, yet assembly retains it.
The local Naga 26 WGSL frontend lowers both operands of binary expressions,
including mutating function calls. The candidate's `keep && append(...)` style
guard therefore did not prevent the append. Both mutating boolean guards now
use explicit branches. The 45-grid/12-torus probes and Rain cell are rerunning;
the existing lifecycle test process uses the earlier candidate and cannot
qualify this correction. `scripts/compare-frozen-contacts.py` now provides the
repeatable strict comparison of counts, normals and separation multisets at
identical poses, excluding different internal triangle/feature encodings.

The explicit-branch correction passes all 45 flat-grid and 12 torus poses;
maximum matched normal/separation component errors are 7.5e-9 and 1.49e-7,
respectively (`rain-flat-seams/guard-result.json`,
`rain-frozen-contact/guard-result.json`). Corrected Rain revalidation completes
with 8,148 finite records but restores the original 0.085509 m knee spike and
two CPU-relative screening failures. The earlier apparent stability improvement
was produced by the rejected candidate and is not evidence for the corrected
fix. See `rain-cell-validated/guard-result.json`.

The pre-guard manifold suite finished with 38 passes, one ignored test and one
failure: an old test invoked the three-argument `match_previous_points` helper
with two arguments, preventing its shader from compiling. The test now passes
the mesh/non-exact-feature argument explicitly; a current-build targeted rerun
is running. Native cached, broader current-build and performance qualification
remain outstanding.

Same-state impact replay now explains both isolated-cell screening failures.
The fixture creates the same upstream joints and terrain, transplants all 42
body poses and velocities from frame 169 or 197, and starts with fresh joint
impulses/contact history. Initial getter snapshots match between engines and
CCD settings. All sixteen five-step processes exit zero with finite output.
From the GPU frame-169 state, CPU and GPU both produce 0.085540 m knee error
with CCD, recovering to 0.001094 m by step five; their full pose/velocity/error
output is byte-identical in this case. Without CCD, both start at 0.000398 m.
From the CPU frame-169 state, both engines start at 0.001796 m with CCD.

From the GPU frame-197 state, both engines produce 0.078686 m for the other
flagged knee and recover to approximately 0.004903 m by step five. Without CCD,
the initial error is 0.000779 m. These two spikes therefore have reproducible
CPU counterparts from identical body state; the evidence supports transient
per-body CCD correction, rather than a GPU-specific joint failure, for these
events. This cold-history replay is not a complete historical-state transfer
and does not clear the remaining full-Rain observations. Evidence:
`rain-impact-replay/result.json` and `rain-impact-replay-197/result.json`.

All three stale helper calls in the mesh-history test have now been corrected,
and its current-build targeted rerun passes
(`rain-flat-seams/history-regression-all-calls-fixed.log`). All 28 current-build
numerical regressions pass independently in ordinary and native cached paths,
including the new admission controls (`ordinary-guard-precision.log` and
`native-guard-precision.log` under `rain-flat-seams/`). Full-world native cached
fixtures, broader lifecycle coverage and performance qualification remain open.

Current ordinary lifecycle validation now passes 39 tests; the sole ignored
test is an explicitly isolated GPU timing experiment, not a failed correctness
check (`rain-flat-seams/guard-manifold-regression.log`). Native cached full-world
validation also passes the 45 grid and 12 torus cases at the same 1e-5 limit;
its 210-step Rain-cell output matches the ordinary output byte-for-byte
(`native-contact-qualification/result.json`). These do not establish full-state
repeatability or qualify the complete Rain scene.

All full-Rain screening observations are now preserved as contiguous episodes:
556 anchor observations form 515 episodes, and 3,019 angular observations form
1,055 episodes. Anchor episodes last at most three frames. The longest angular
episode lasts 77 frames (387–463), joining body creations 2379/2380, with peak
0.065481 rad at frame 397. The preserved state shows the GPU elbow remains at
0.063880 rad after sleeping, and the bodies disappear at frame 464; deletion
must not be reported as recovery. This sustained case is the next physical
diagnostic target. See `rain-residual-events.json` and
`rain-longest-angular-window.json`. The event summarizer now additionally records
sleep state and distinguishes cleared screens from joint removal/capture end.

The sustained elbow episode now has a matched-state CPU counterpart. Replay
uses the verified upstream row-6/column-5 cell, group 65, and all three humans
(creations 2353–2394). From GPU frame-386 poses/velocities, CPU and GPU settle at
0.061662/0.061652 rad and both sleep at replay step 38. From CPU frame-386 state,
they settle at 0.009096/0.009115 rad and both sleep at step 41. Each run covers
120 steps with 5,040 finite body records; initial pose/velocity getter snapshots
are identical. This supports pose-dependent residual under the upstream joint
and contact configuration for this episode, rather than a GPU-only failure to
correct or sleep. Fresh contact/joint history and initially awake bodies remain
explicit replay limitations. Evidence: `rain-elbow-replay/result.json`.

The largest flagged angular episode (1847/1848, original frame 312) also has a
CPU counterpart. In the row-3/column-4 cell, both engines peak near 0.544230 rad
from a common frame-311 input and recover to about 0.0027 rad after 60 steps.
The original supplied GPU input exposed a 1.9-micrometre setter/readback position
difference in body 1841; that trial is retained and is not labeled identical.
A separate common-readback input gives identical initial getter snapshots and
the same spike/recovery finding (`rain-angular-peak-replay/result.json`). These
targeted explanations do not erase the original full-Rain screening results or
establish universal correctness from a few cases.

The largest remaining anchor-screen event (1829/1830, original frame 328)
now also has a matched-state CPU counterpart. Both engines use the upstream
row-3/column-4 cell and identical getter snapshots for all 42 bodies at frame
327. From GPU input, the first replay step peaks at 0.164867 m on CPU and
0.164864 m on GPU; step five drops to 0.013735/0.013734 m and step 60 to
0.000367/0.000358 m. From CPU input, peaks stay below 0.018 m on both engines.
All four processes complete 60 steps with 2,520 finite body records each
(`rain-largest-anchor-replay/result.json`). This supports input-dependent
transient stretch of comparable magnitude for this event. It does not qualify
all anchor episodes, change the original screen limits, or transfer the full
historical contact/joint state; fresh history remains an explicit limitation.

Rain cell construction now has an opt-in getter audit (`RAIN_AUDIT_JOINTS=1`)
using `c_abi/human_joint_audit.h`, shared with the existing Falling Ragdolls
audit. The release upstream `CreateGroup` and cell fixture both create three
Humans with friction torque 5, spring hertz 1 and damping 0.7, using the same
row/column collision group. Getter comparison passes for all 42 joints
(27 spherical, 12 revolute, three filter), 84 body/joint incidences and 1,377
configuration fields, including endpoint order, local frames, tuning, limits
and motors. The helper extraction preserves every existing Falling Ragdolls
joint audit record (`rain-joint-setup-audit/result.json`). This checks actual
cell creation state, independently of later pose matching; it does not provide
historical impulse equivalence or validate every full-Rain interaction.

All 19 Rain angular-screen episodes that ended at joint removal (five) or
capture end (14) now have 120-step matched-pose/velocity CPU/GPU cell replays.
All 38 processes exit successfully with 5,040 finite body records each; all
42 initial getter snapshots match in every pair. Every target pair sleeps by
step 120 in both engines. Maximum final angular-error difference is
0.000772041 rad; maximum per-step differences across the 19 target traces are
0.00077404885 m in anchor error and 0.0023186095 rad
in angular error. Full traces and worst identities/steps are preserved in
`rain-terminal-episodes-replay/results.json` and `summary.json`.

This broadens the evidence for CPU-comparable residuals under these upstream
poses, loads and tuning; sleeping does not mean a residual was corrected.
Fresh contact/joint history, initial waking and isolated-cell interactions
remain limitations. The original full-Rain screen stays failed. The other
515 anchor and 1,036 angular episodes that cleared the relative screen still
need physical qualification; screen clearance alone is not proof of correct
behavior (`rain-event-coverage.json`).

The remaining 1,551 screen-cleared Rain episodes are now grouped by error
field, joint type and child-bone ordinal. Selecting maximum residual and longest
duration in each of 16 groups yields 22 distinct episodes
(`rain-joint-extrema-replay/selection.json`). The batch uses the existing frozen
CPU/GPU cell binaries and 120 steps from each GPU pre-episode pose/velocity
state; all 42 initial body getters must match exactly. Diagnostic margins stay
at GPU minus CPU <=0.05 m for anchor error and <=0.05 rad for available angular
error. Every exceeding step is retained; no averaging or automatic full-scene
physical pass is applied. Replays remain limited by fresh contact/joint history,
initial waking and isolated-cell interactions.

The initial driver completed six revolute-anchor cases without diagnostic
exceedances, then stopped because spherical joints have no `replay-health`
angular line. The harness now obtains their already-exported anchor error from
the 16-column body trace and records angular/awake as unavailable, rather than
zero or an inferred value. Completed process logs are reused only after a
recorded zero exit, and initial getter/row/finite checks remain mandatory.
`driver.log` and `run-initial.py` preserve the original harness failure;
`driver-resume.log` tracks continuation. This also clarifies a coverage limit:
the existing Rain angular metric is revolute-axis alignment, not a spherical
swing/twist limit check. Those limits remain separately unqualified for full Rain.

The resumed extrema batch completes 20/22 cases without matched-state screen
exceedances, then correctly rejects case 21 (right elbow 1847/1848, frame 312):
initial body 1841 z differs by one float32 ULP. Input/CPU getter is -19.8252048;
GPU getter is -19.8252029 (`rain-joint-extrema-replay/initial-state-mismatch.json`).
Case 21 has no accepted comparison and case 22 remains pending.

This exposed a transform-setter precision defect for offset mass centers:
`b3_body_set_transform` previously cleared the origin cache and reconstructed
position by adding then subtracting the rotated local center. Upstream stores
the supplied origin directly. The GPU setter now writes that origin to the
existing cache while updating COM as before; `apply_deltas` invalidates the
cache when motion is integrated. The regression uses the actual upstream
thigh_l capsule and recorded Rain transform and fails before the fix with bit
patterns 3248396804 versus 3248396805 in z (`rain-transform-origin-before.log`).
All four origin/CCD precision tests pass on ordinary/native paths after the fix
(`rain-transform-origin-fixed-*.log`). The subsequent-motion check also passes on both paths: the retained setter
origin is replaced after stepping. A rebuilt native GPU replay now runs the
same 22 selected episodes in a separate `rain-extrema-origin-fixed` directory,
starting with the previously rejected case. Binary/source hashes and exact
configuration are recorded; pre-fix evidence is retained. No original Rain
screen limit changed.

The post-fix native batch completes all 22 selected cases, 120 steps each,
with exact initial getters for all 42 bodies in every case. No step exceeds
the predeclared matched-state diagnostic margins. Largest GPU-minus-CPU errors
are 0.0028195791 m anchor error (pair 4723/4724, replay step 41) and
0.0181889646 rad revolute alignment (pair 5275/5276, replay step 14).
`rain-extrema-origin-fixed/summary.json` retains values, identities and original
frames; full traces retain all observations. These are CPU-comparable isolated
responses across the selected extremes, not proof that all 1,551 episodes are
benign. Full Rain residual screening remains failed, spherical swing/twist
limits remain unqualified in this screen, and current-build full-scene repeated
state/physical qualification remains required.

Spherical limit diagnostics are now implemented in
`c_abi/spherical_limit_health.h` and enabled in the cell fixture with
`RAIN_REPLAY_LIMITS=1`. They decompose actual relative joint-frame rotation
using Box3D's Z-axis twist/swing convention and report enabled-cone excess,
lower-twist excess and upper-twist excess separately. Actual limit flags/bounds
and live engine angle getters are recorded too. The analytical control covers
allowed twist, cone violation, both twist bounds, combined swing/twist,
quaternion sign invariance and disabled limits; it passes against the release
reference library (`spherical-limit-health-controls.log`).

A new batch (`rain-spherical-limit-replay`) measures all 27 spherical joints
at all 120 steps of each of the same 22 selected cell replays. It requires exact
CPU/GPU limit configurations, finite unique observations and matching initial
body getters. The existing 0.05 rad CPU-relative diagnostic margin is applied
to actual limit excess, not permitted angular motion. At least one previously
uncovered exceedance has appeared: body creation 5273, cone limit, replay step
10 in the case starting from full-scene frame 587. CPU excess is 0.0744966418
rad and GPU excess is 0.129642427 rad, a difference of 0.0551457852 rad. This
remains under investigation; the formerly passing revolute-alignment screen
does not cover or excuse it. The batch completes all 22 cases and 71,280
spherical-joint observations per engine with exact limit settings and initial
getters. It finds exactly this one >0.05 rad GPU-minus-CPU excess; all raw
observations remain in the logs (`rain-spherical-limit-replay/summary.json`).
The independently derived angle versus engine-getter discrepancy peaks at
0.00004495 rad, far below the flagged difference.

The target's full 120-step trace is retained in
`rain-spherical-limit-replay/anchor-5276-588/body-5273-limit-trace.json`.
The diagnostic difference drops below the margin at the following step; at
step 119 CPU/GPU cone excess is 0.00101746619/0.00113323331 rad, with zero twist
excess in both. Recovery does not establish the cause of the transient excess
or clear the failed diagnostic. No production solver change accompanies this
diagnostic addition.

The cone exceedance now has a four-way cross-replay: both engines start from
CPU or GPU poses/velocities saved at replay step 9. All 42 initial getters
match in each pair. From CPU input both engines produce first-step cone excess
0.0735444725 rad; from GPU input both produce 0.106604904 rad. Across 30 fresh
history steps, maximum CPU/GPU cone-excess differences are 0.000004321337 and
0.000000923872 rad respectively (`rain-cone-cross-replay/summary.json`). This
points toward differing incoming state/history, rather than an isolated cone
formula mismatch. Original warm-start/contact history was not restored, so the
whole original discrepancy is not thereby explained or cleared.

Descriptive original-replay envelopes retain the individual extremes:
CPU/GPU peak cone excess is 0.272952735/0.260561138 rad; last indices above
0.05 rad are 22/17 and above 0.01 rad are 27/20. These are measurements, not
new acceptance limits (`body-5273-envelope.json` beside the target trace).
The GPU has neither a higher peak nor later recovery in this case, despite
the flagged pointwise difference. Five native fresh repeats of the original
case with all captured v19 state completed in `rain-cone-full-state-five`,
using the post-origin-fix binary. All five 120-step traces pass the narrowly
audited membership comparison (`audited-result.json`); raw comparison fails
at frame 2 on occupied-root list order (`raw-result.json`). Every field outside
the documented membership normalizations remains exact. This qualifies only
this isolated native case, not full Rain or final capture coverage.

A targeted cone-cache control now distinguishes the target's cached cone
impulse from other history. At step 10 the diagnostic fixture either calls
EnableConeLimit(true) twice (sham) or disables/re-enables it (clear). Both
engines clear only the cone impulse on the flag transition; both arms perform
the same number of setter calls, and the limit is verified enabled before
stepping. Six 30-step runs cover baseline/sham/clear on CPU and native GPU;
all starting getters and pre-intervention pose rows match.

The sham is byte-identical to baseline for all body output on both engines.
At the intervention step CPU cone excess changes 0.0744966418 -> 0.0699993223
rad and GPU 0.129642427 -> 0.116884977 rad. Clearing therefore reduces the
CPU/GPU difference from 0.0551457852 to 0.0468856547 rad; it does not explain
all of it (`rain-cone-cache-control/summary.json`). This is a diagnostic
perturbation, not an accepted solver setting or fix. The normal warm-start
behavior is unchanged, and the original flagged comparison remains recorded.
The global warm-start disable API cannot provide a matched control because
it is unsupported on GPU, as listed in the coverage matrix.

The frozen-v19 ordinary ragdoll batch has now completed all five 600-step runs.
All five pass the unchanged physical screen and have byte-identical pose,
velocity and separation output (`ragdoll-v19-five/ordinary/five-run-physical-summary.json`).
Comparing every captured group at all 600 frames against run 1 finds differences
only in allocation bookkeeping and slot-indexed event history in runs 2–5
(`run-N-all-frames-differences.json`). The raw gate still fails, and the audited
comparison still exposes child-slot differences at frame 139; no allocator
normalization or full-state pass is claimed. These frozen binaries predate the
host query/event ordering and transform-origin fixes. All five native runs also
pass the physical screen; the completed audit is recorded below.

A relationship audit of all 600 frames in ordinary runs 1 and 2 finds live
physical-slot differences on only three frames, first at frame 139: child
ordinal 1 of shape pair (92, 118) occupies slot 148 versus 145. Root slots,
live identity sets, per-identity live generations and free-slot counts match
throughout (`run-2-allocation-relations.json`). This narrows the observed
mismatch without normalizing it or qualifying all allocator histories.

The implementation explains why child slots can differ: `allocate_manifold_slot`
in `math.wgsl` uses an atomic ticket into a shared free-list snapshot.
`store_pair` assigns child generations from the parent root, while
`ordered_contact_transitions` in `solve.wgsl` skips children and validates root
slot/generation relationships. Public contact owners also contain roots only.
Free-slot generations can still affect a later root lifetime, so future-history
and stale-handle checks remain required; the raw/audited gates are unchanged.

A new `retired_generation_offset_preserves_unread_handle_lifetimes` regression
perturbs an empty GPU contact slot's generation by 17 before each reuse. It
preserves unread retirement/reentry, requires the same physical root slot and
the expected stored generation increment, checks surviving handle continuity,
and rejects every retired handle across four cycles. The test-only mutation
asserts the selected slot is empty and changes no live contact or public handle.
Ordinary/native controls and five fresh processes per path pass all four cycles
(`retired-generation-five/{ordinary,native}/result.json`); the original
unperturbed unread-lifetime test also still passes both paths. This verifies
bounded handle behavior under differing free-slot generations; it is not a
full-state comparison and does not broaden allocator normalization.

The corresponding post-origin-fix ordinary five-run cone-case capture also
passes all 120 steps in audited-membership mode (`rain-cone-ordinary-five/audited-result.json`).
Raw comparison fails at frame 2 on occupied-root order. Both configurations
therefore pass this bounded captured-state comparison independently; full Rain
and final capture coverage remain open. Concurrent correctness runs provide no
timing evidence.

The generation-offset control now also covers the 16-step mesh contact and
motor insertion/removal/reinsertion fixture. It changes the retired original
root's stored generation by 17 after step 4; the replacement root inherits that
offset at step 5. Ordinary and native-policy controls match at all fields except
slot/registry-owner generation integers across all 16 steps, including physical
state, public events/handles, solver order, warm state and logical history.
All 60 inspected history records per path retain valid generation relationships
(`retired-generation-state-semantic-checks.json`). This is a controlled
perturbation result, not a broadened production comparator. The first native
mutation capture accidentally omitted the native cache environment; its policy
difference was detected and preserved in `retired-generation-state-differences.json`.
Only the corrected `retired-generation-state-native-policy.jsonl` is compared
with the matching native-policy control for this result.

After the transform-origin fix, all 28 existing `native_precision::` numerical
regressions pass independently in ordinary and native cached configurations
(`post-origin-numerical-regressions.json` and corresponding per-path logs).

Full native-sample health capture now records `spherical_limits` for every
joint: null for other joint types; enabled flags, cone/twist bounds, measured
angles and three separate violations for spherical joints. CPU and GPU use the
same `spherical_limit_health.h` measurement routine, independently of each
engine's reported constraint error. This closes a measurement omission in the
full Rain capture without changing the solver or the existing alignment metric.

`compare-rain-lifetimes.py --require-spherical` rejects missing measurements,
nonfinite values, inconsistent enabled flags/violations and mismatched CPU/GPU
limit settings. Cone/lower-twist/upper-twist screens each retain the established
0.05 rad CPU-relative diagnostic margin; violations are reported individually,
not averaged away. Without the flag, older captures remain readable and report
zero spherical observations, which is not spherical-limit qualification.
Five checker tests cover valid/disabled limits, eleven malformed/missing
cases, six paired coverage/parameter mismatch controls, and episode endings
through clearing/deletion/capture end (`spherical-full-health-controls.log`). The initial three-step full Rain
smoke matches 1,260 joint observations, including 810 spherical measurements,
with no diagnostic exceedances (`rain-spherical-full-smoke-comparison.json`).
The CPU capture validates all 1,948,800 joint observations, including 1,252,800
spherical measurements (`rain-spherical-full/cpu-validation.json`). Its observed
peak cone/lower-twist/upper-twist violations are 0.982344/0.656990/0.227094 rad.
These are diagnostic observations, not proposed acceptance thresholds or a
claim that the residuals are harmless.

Both CPU/native GPU 600-step captures completed under `rain-spherical-full`,
with frozen binaries and recorded environments. Their recorded scene, seed,
sleep, worker count and step-window settings agree (`header-comparison.json`).
Creation matching passes all 1,948,800 joint observations, with 8,400 created
bodies and 4,200 reused slots. All 1,252,800 spherical measurements are valid,
but the unchanged CPU-relative screen fails: 557 anchor, 3,004 alignment,
5,150 cone, 1,086 lower-twist and 371 upper-twist observations. The complete
4,826 episodes are retained in `episodes.json`, with extrema and endings in
`episode-summary.json`; none are discarded or averaged away.

The earliest cone discrepancy is joint509–510 at frame130: GPU violation
0.182287276rad versus CPU0.0908877254rad, preceding the first anchor flag170.
The largest cone violation is1.26105142rad at frame516 on5003–5004; its episode
clears the screen after frame518. A lower-twist episode2370–2379 lasts89frames,
including sleep from426 until deletion464. These findings need diagnosis;
pointwise CPU differences alone do not establish a solver defect.
`rain-first-cone-replay` extracts both engines' frame129 cell505–546 for
four120-step cold-history replays, preserving each input's body poses/velocities
and checking initial getters. All four replays completed with exact initial
getters. For each input, both engines produce exactly the same first-step cone
violation:0.159254253rad from CPU input,0.238866389rad from GPU input. Maximum
GPU-over-CPU cone excess over120steps is0.0330601037rad and0.00058728456rad
respectively (`rain-first-cone-replay/summary.json`), below the unchanged0.05rad
diagnostic margin. This does not reproduce an isolated cone-formula mismatch.
Other-cell contacts and accumulated impulses/history are not restored, so the
original full-scene episode remains unresolved rather than silently cleared.
They do not provide complete-state repeatability or performance evidence.
`summarize-rain-residual-events.py --require-spherical` now shares validated
residual definitions with the comparator and retains every spherical episode,
its peak, sleeping observations and whether the screen cleared, the joint was
removed, or capture ended. Synthetic controls verify all three endings and
identity preservation; the three-step smoke has no flagged episodes.

Frozen native ragdoll runs 2, 3 and 4 have now each been compared against run 1
at every captured group over all 600 frames. Only contact allocation bookkeeping
and slot-indexed previous-touching history differ; every other captured group is
exact (`ragdoll-v19-five/native/run-{2,3,4}-all-frames-differences.json`). This supports
the same bounded diagnosis as ordinary runs; it does not remove the raw failure
or qualify the unfinished five-run/current-build coverage.

The exporter now enforces its completed-step contract through
`validate_diagnostic_boundary`: completion must match the current physics step,
contact-status readback must be drained, and no callback phase may be open.
`b3_world_gpu_wait` already waits for completion and calls `finish_contact_status`;
the check makes this prerequisite explicit before any frame is published.
The new ordinary/native regression deliberately holds status harvesting,
verifies rejection without a partial file, releases it, and verifies successful
capture (`capture-boundary-{ordinary,native}.log`). The existing trace suites passed 13 ordinary and 14 native tests, with one
shared failure in a stale pre-v19 expected body-allocation object. That test
now explicitly checks retained generations after deletion and the replacement
and survivor generations after reuse; targeted reruns pass on both paths
(`capture-boundary-lifecycle-{ordinary,native}.log`).
This closes the pending-status boundary assumption; it does not
claim arbitrary in-flight state capture or finish the persistent-state audit.

The existing `convex_gpu_ccd_corrects_live_steps_and_rebuilds_after_mutation`
regression now optionally emits every tested GPU-CCD step through
`GPU_PHYSICS_TEST_TRACE`. Its original physical checks remain: fast offset
sphere/capsule/box bodies stay above the ground, the first three steps agree
within 0.002 m with the host CCD reference, bullet mode forces fallback, and
clearing bullet mode plus teleport/velocity reset re-enters GPU CCD above the
sphere clearance bound. This fixture compares CCD implementations inside the
GPU engine; its reference is not an independent Box3D C world.

Ordinary/native capture-enabled controls pass, and all five emitted steps show
CCD presence `[true,true,true,false,true]`
(`ccd-mutation-state-{ordinary,native}.jsonl`). Five fresh repeats per path pass every raw v19 field over all five steps in
`ccd-mutation-five/{ordinary,native}/raw-result.json`, with frozen binaries and
configuration recorded separately. No storage normalization was needed.
This is bounded fast-impact and mutation coverage, not a universal CCD claim.

The existing C `both_drag_test.cpp` fixture now optionally exports GPU core
state after every completed step via `GPU_PHYSICS_STATE_TRACE`, failing the
process if the diagnostic exporter is unavailable or rejects a frame. Normal
fixture runs are unchanged. Fresh ordinary/native binaries retain the real
Box3D C comparison, independent picking rays, ten sequential grabs, sparse
joint slots, release and settling checks from the original `--ground` fixture.
The existing limits remain: total position/quaternion chord 0.025, held position
0.005 m, held quaternion chord 0.01, held velocity 0.1 m/s and settled velocity
0.02 m/s. The stricter all-frame 0.5 m/s velocity diagnostic remains available
through `--ground-strict`; this batch uses the existing ground physical gate.

`drag-ground-state-five` now runs five fresh 3,060-step captures per path.
Binaries, configuration and output directories are frozen separately, and
initial frames verify ordinary versus native-cache policy. The first 3,060-step
run passes on each path with identical printed results: maximum position
error 0.00602796 m, quaternion chord 0.00597248, peak velocity difference
0.230405 m/s, held velocity difference 0.047432 m/s and settled velocity
difference zero. The optional stricter peak-velocity diagnostic also passes
(`drag-ground-state-five/first-run-physical.json`). Five-run state results remain
pending. This is scripted input replay for this fixture, not arbitrary mouse
input/callback-state serialization. Concurrent qualification runs are not
performance evidence.

Further full-Rain diagnostics replay the severe cone event (body 5004, input
frame 514) and the persistent lower-twist event (body 2379, input frame 374).
Each case runs both engines from each engine's extracted 42-body state for
120 steps, with exact initial getter agreement and finite output throughout.
The frozen replay binaries include the transform-origin correction. Their
manifests and all three angular metrics are retained in
`rain-{cone-severe,twist-persistent}-replay/metric-summary.json`.

From the severe case's GPU input, both engines reach exactly the same
1.26054549 rad peak cone excess; the maximum pointwise GPU-minus-CPU cone
excess is 0.007599116 rad. From its CPU input, both peak at 0.69729197 rad.
For the persistent twist case's GPU input, CPU/GPU lower-twist peaks are
0.110814810/0.110814765 rad and final residuals are
0.0466963947/0.0466984063 rad. Maximum pointwise GPU-minus-CPU lower-twist
excess is 0.0000028312 rad. No target angular metric in these eight runs
exceeds the unchanged 0.05 rad diagnostic margin.
These results do not isolate a GPU joint-formula discrepancy. They also do
not clear full Rain: the replays reset impulses/history, wake bodies and omit
other cells' contacts. In particular, a nonzero final replay residual does
not prove that the replay body slept; the frozen logs do not capture that flag.
Full-scene contact history and the observed sleep-time residual remain open.

All five frozen-v19 native Falling Ragdolls runs have now completed with exit
zero and pass the unchanged physical screen. Their pose/velocity/separation
dumps are byte-identical (`ragdoll-v19-five/native/five-run-physical-summary.json`).
The raw state gate still fails at frame 1 on occupied-root append order.
The all-frame audit of native run 5 also finds only allocation and slot-indexed
event-history differences, matching runs 2–4 (`run-5-all-frames-differences.json`).
These binaries predate the later host-query ordering and transform-origin
fixes, so this is bounded evidence, not current-build qualification.

`ragdoll-current-five` starts the corresponding five 600-step runs per path
with the current production fixes and completed-boundary guard. The current
Rust archives use `external-c-shim`; the initial fixture links accidentally
omitted that shim, created CPU worlds and were rejected by GPU state capture
at the first step. Those terminal failures are retained in `ordinary`/`native`.
The corrected build explicitly includes `shim.o`, with commands and dependency
hashes in `corrected-build-manifest.json`; fresh outputs use
`ordinary-corrected`/`native-corrected`. Both first frames confirm schema v19,
116 bodies and the expected separate graph-cache policy. Qualification is
pending; no failed attempt is counted as a repeat and these overlapping runs
are not performance measurements.

The persistent twist replay now has explicit awake-state diagnostics for both
spherical-joint endpoints and a strict optional `RAIN_REPLAY_SLEEP=0|1` control
in `c_abi/rain_cell_reference.cpp`. Absent the option, upstream sleep defaults
are preserved. Existing spherical-limit log columns remain unchanged.

Four 180-step runs in `rain-twist-sleep-control` compare Box3D C and native GPU,
with sleep enabled and disabled, from the same GPU-derived 42-body state.
Initial getters match in all four runs, every output is finite, and the enabled
runs reproduce all previous 120-step pose/velocity rows byte-for-byte. Both
endpoints sleep at replay step 51 on both engines. Disabling sleep keeps both
awake throughout but leaves lower-twist residuals of 0.0466937721 rad (CPU)
and 0.0466936976 rad (GPU), versus 0.0466963947/0.0466984063 with sleep enabled
(`summary.json`). Thus sleep is not the cause of this isolated residual. The
result supports examining the loaded constraint equilibrium and contact
history; it neither proves that equilibrium is acceptable nor clears the
original full-Rain sleep-time failure. No physical acceptance limits changed.

All five ordinary ground-dragging runs have now completed successfully and
match every raw captured v19 field over all 3,060 steps; no normalization is
needed (`drag-ground-state-five/ordinary/raw-result.json`). Each also passes
the original physical checks against Box3D C; exits, output hashes and final
metrics are retained in `five-run-physical-summary.json`. Native remains live.

The persistent-twist drive control adds explicit optional target-only spring
or motor disabling to the diagnostic cell fixture, checking the requested
setting through getters. Each arm makes both setter calls; the sham preserves
both settings. Normal runs leave the original definitions untouched.
Six 180-step Box3D C/native GPU runs keep sleep disabled, identical initial
getters and fresh history (`rain-twist-drive-control/summary.json`). Both sham
outputs exactly reproduce the prior sleep-disabled baseline. Removing only
the target spring leaves CPU/GPU lower-twist residuals
0.0469249338/0.0469185412 rad; removing only the target motor leaves
0.0448780060/0.0448783487 rad. All endpoints remain awake and output is finite.
Neither target drive alone explains the residual. This narrows the isolated
case to other connected constraints/contact loading and the limit response;
it does not validate the full Rain trajectory or relax any acceptance limit.

The native ground-dragging batch is also complete: five fresh runs independently
match all raw v19 fields at all 3,060 steps, and all five original physical
checks pass. Together with ordinary this closes this fixture's repeatability
gate on both configurations (`drag-ground-state-five/native/raw-result.json`
and `five-run-physical-summary.json`). It does not close wider matrix entries.

The native sample harness now optionally calls the core-state exporter from
`gpu_sokol_bench_note_world`, immediately after each actual physics step and
before the optional health scan. `GPU_PHYSICS_STATE_TRACE` requests capture;
missing exporters and unsupported CPU/both builds fail explicitly. The exporter
itself enforces the completed-step boundary. This enables full upstream Rain
state capture rather than substituting isolated cells. Build/smoke validation
is pending: the first native link found the current archive used external C
shims, whereas this target expects embedded shims; an appropriately configured
Rust rebuild is in progress. Existing failed build evidence is retained.

The native full-Rain state-capture smoke now passes all three requested steps
with 520 bodies and schema v19 (`native-full-state-smoke/result.json`). The
native link uses a rebuilt diagnostic archive with its embedded C shim.
`compare-core-state.py` now accepts losslessly gzipped JSONL as well as plain
JSONL. A comparison of the same three frames in both formats passes; mutated
frame numbering and truncated compressed input are rejected
(`compression-controls.json`). The comparison rules themselves are unchanged.

`rain-full-state-five` now runs five fresh 600-step native captures of the full
upstream scene, including health and lifetime mapping. Each finished trace is
compressed and its decompressed SHA256 verified before removing the redundant
plain file. The binary, source hashes, environment and each command are retained.
Ordinary build preparation is separate. These are qualification runs, not
performance measurements; full-scene repeatability and physical results remain
pending.

Both full-Rain five-run batches are now live. Their first captures independently
verify schema v19, 520 bodies and ordinary versus native graph-cache policy
(`rain-full-state-five/{ordinary,native}/startup-check.json`). Supplemental
provenance records upstream revision, benchmark/human sources, native compile
flags and link commands. The upstream `BenchmarkRain::Step` calls `StepRain`
before `Sample::Step`, so spawning/recycling for that step precedes the state
snapshot. Rain's column counters are deterministic scripted application state
under this fixed schedule; the engine capture is not an application checkpoint
or a claim of arbitrary-input replay. All 600-step comparisons remain pending.

The persistent-state audit found that broadphase's joint collision-filter
lookup table was not validated against the captured joints. Capture now
reconstructs its exact expected contents from GPU joint records and checks the
actual scratch table when the shader consumes it, plus the overflow flag.
No additional schema fields are needed for this derived relationship. Empty
joint worlds and explicit scan/overflow paths do not consume table contents.
An initial unconditional check rejected unused storage after a body-count
change; the existing lifecycle fixture exposed that and the guard was corrected
using the actual broadphase branch. The initial failure logs remain retained.

The revised corruption regression uses a live motor joint, verifies a valid
capture, corrupts device-only filter storage and requires rejection without
appending a partial frame. It passes on ordinary and native, as do both 16-step populated-table lifecycle
reruns covering joint creation, removal and recreation (`joint-filter-*-v2.log`). Current frozen full-scene
batches predate this exporter guard. This closes one audit item only after
verification; it does not establish complete persistent-state coverage.

The first current-build Falling Ragdolls run on each path now passes the
unchanged 600-step physical screen. Both pose/velocity/separation outputs are
byte-identical to their frozen-v19 predecessors
(`ragdoll-current-five/first-run-health-summary.json`). This is one completed
run per configuration, not yet the required five-run state result.

`scripts/check-core-state-health.py` now checks every captured frame for sticky
capacity loss and its cause bits, invalid physics, open callbacks and mismatch
between completed and physics step. It requires all requested sequential v19
frames and reads plain or gzipped input. Ten negative controls reject each
failure class and missing status; real three-step full-Rain traces pass in
both formats (`core-state-health-controls.json`). Both 600-step current ragdoll status scans pass. This is a status screen, not a substitute for finite
physical data, joint/collision quality or complete-state comparison.

The persistent-twist contact control now completes four 180-step runs from
the same saved state with sleep disabled and all joint/gravity settings intact.
`RAIN_REPLAY_COLLISIONS=0|1` is an explicit diagnostic-only cell-fixture option:
it sets all human shape masks/group overrides to disable collisions, or repeats
the unchanged filter setter in the enabled sham arm, verifying getters.
Both enabled-arm pose outputs exactly match the previous no-sleep baseline.
With collisions disabled, CPU and GPU both peak at 0.0481669605 rad lower-twist
violation and both finish with zero violation; all endpoints remain awake and
all output is finite (`rain-twist-contact-control/summary.json`). With contacts
enabled both retain approximately 0.04669 rad. Thus contact loading sustains
this isolated residual. It is not an inability to correct the unloaded joint,
and sleeping or either target drive alone did not explain it. This does not
establish acceptable loaded compliance or explain the larger full-scene
residual; the original Rain screening failures remain open.

`gpu-sim-field-inventory.json` inventories all 209 named `GpuSim` fields,
including conditional build variants, with a source hash. It distinguishes
103 immutable pipeline declarations, exporter references, device-buffer regions,
derived uniforms, test-only controls and fields still needing consumer review.
Exporter reference presence is explicitly not treated as proof of coverage.
Persistent scratch/atomic/query regions and pose/readback synchronization still
need a complete consumer classification before claiming complete state.

The loaded-twist source audit also identifies a measurement limitation:
CPU and GPU spherical reaction-torque getters sum spring/motor impulses and
project twist-limit impulse onto the **current body-B twist axis**. The solver
instead applies that impulse through its **prepared twist Jacobian**, including
the swing correction (`spherical-reaction-load-audit.json`). Therefore matching
public torque values cannot by themselves establish the actual applied torque
balance or justify a residual acceptance limit. Continue with component
impulses and preparation-frame transforms for the load analysis; this is a
shared API measurement limitation, not evidence of a new GPU solver defect.

`rain-twist-impulse-state` is capturing 180 steps of the loaded isolated case
using the same frozen native binary, initial state and settings as the completed
contact-enabled sham. Completion will require byte-identical pose output.
The analysis uses captured `host_state.step_start_bodies` and local joint frames
for the prepared twist Jacobian, rather than spherical `reaction_frames`
(which the host only populates for parallel/revolute joints). It separates
cached twist, swing, spring, motor and point impulses. Reconstructing cached
impulse/h is not a full per-substep force balance; capture and analysis remain
pending and no compliance bound has been inferred from them.

The loaded impulse replay completes all 180 steps with pose output exactly
matching the sham baseline. Captured start rotations and full inverse-inertia
lanes reconstruct the target's prepared twist Jacobian and effective mass.
The independent reconstructed angle agrees with fixture health within 1e-5 rad
throughout (`rain-twist-impulse-state/summary.json`).

For a stationary active lower limit, setting the soft solver's impulse increment
and relative angular velocity to zero gives
`abs(C) = lambda / (h * effectiveMass * omega^2)`, where
`omega = 2*pi*min(hertz, 0.25/h)`. At step 180 the captured effective twist mass
is 0.0150461688, stiffness is 2138.39594 and lower-limit impulse is 0.416040272.
The equation predicts 0.0466937193 rad versus the fixture's measured
0.0466936976 rad, with endpoint angular speeds about 2.1e-6 and 2.5e-6 rad/s.
This provides quantitative evidence of loaded soft-limit compliance in the
isolated case, consistent with both engines and with the unloaded control
clearing its violation. It does not excuse transient instability or establish
the cause of the larger full-Rain residual. Apply the same check to actual
full-scene impulses before classifying those failures; no limits changed.

The current-build ragdoll run-2 prefix comparison now covers 140 completed
frames on each path (`ragdoll-current-five/{ordinary,native}-corrected/run-2-prefix140.json`).
Raw comparison first differs on occupied-root append order at frame 1; audited
membership comparison first differs on a physical child slot at frame 137 for
shape pair (92,112). Only allocation bookkeeping and slot-indexed event history
differ through frame 140; all other captured groups match. The earlier frozen
batch's frame-139 observation is not a universal first-difference frame. These
prefix diagnostics do not replace the full 600-step/five-run gates.

`scripts/analyze-spherical-compliance.py` generalizes the loaded-joint analysis
for plain or gzipped v19 traces. It requires an explicit **captured body creation
identity** (not a recycled slot or native health creation tag), exact input frame
count and a unique spherical joint ending at that body. It validates sequential
frames and reports the component impulses, prepared-axis reconstruction,
effective mass, measured residual, endpoint speeds and sleep flags. All 180
isolated diagnostic rows reproduce the original analysis. Sleeping endpoints
may retain impulses prepared before the current snapshot; the output explicitly
warns that it is not a per-substep torque balance or an acceptance gate. Full
Rain identity mapping must be verified before selecting its target.

The completed current ordinary run-2 allocation audit changes the scope of the
storage discrepancy: live slots differ on 464 of 600 frames, and root slots on
463 frames starting at 138. Live identity sets, per-identity generations,
free-slot counts and high-water marks remain exact throughout
(`ragdoll-current-five/ordinary-corrected/run-2-allocation-relations-600.json`).
This is broader than the earlier frozen batch's brief child-only relocation;
do not reuse that earlier classification without checking later root reuse.
Run 2 passes the unchanged physical screen. The native 600-frame allocation audit now reports the same counts and preserved
relationships. The full captured-group comparison remains running; no
normalization has been added.

The current ordinary run-1/run-2 comparison now covers every captured group
at all 600 frames. Only contact allocation (586 frames) and slot-indexed
previous-touching history (450 frames) differ; all other groups, including
solver/contact data, public handles/events and cache state, match
(`ordinary-corrected/run-2-all-groups.json`). The native second run also passes
the unchanged physical screen; its full-group audit is in progress.

A separate diagnostic checks whether each frame's allocation records differ
only by permutation, preserving every live and free generation value and
record count. It now finds identical record multisets at all 600 ordinary
frames (`ordinary-corrected/run-2-slot-multiset.json`), and does not modify either
comparison mode. A future exclusion of incidental locations would still need validated
contact ownership, lifecycle relationships, capacities and all semantic state;
it must not simply discard allocation failures.

The native current run-1/run-2 audit is complete: over all 600 frames, only
contact allocation (584 frames) and previous-touching storage (36 frames) differ.
Every other captured group matches, and native run 2 passes the unchanged
physical screen (`native-corrected/run-2-all-groups.json`, `run-2-health.json`).
Five-run results and storage-equivalence qualification remain pending.

The pose-readback consumer audit follows `ensure_pose_snapshot` through its
host-epoch checks and `lock_pose_snapshot`, and full mirroring through
`accept_body_mirror_as_pose_snapshot`. Once a full mirror has been applied,
`consumed == pending` prevents a later getter from applying an older staged
pose, including poses preceding host CCD correction. Capture now explicitly
requires either no pending staged pose or that superseded relationship.
The new boundary regression captures a valid world, reopens consumption in a
test-only control, requires boundary rejection, restores the full-mirror marker
and verifies a subsequent pose query preserves the position. Ordinary/native boundary controls and the existing 16-step
creation/deletion/recreation fixture both pass (`pose-boundary-*.log`). This is a capture guard, not a
production solver change; frozen ongoing binaries remain unchanged.

`scripts/audit-contact-slot-permutations.py` is a separate storage-equivalence
diagnostic, leaving both existing comparison modes unchanged. Before comparing,
it validates every original frame with the existing schema checker. It requires
unique live identities, exact allocation/contact correspondence, contiguous
child ordinals with a root, matching root/child generations and matching
occupied-root membership. It then sorts complete allocation records, retaining
all free and live generation values, high-water marks and all other captured
state. The earlier audited membership treatment is reused only for its already
reviewed arrays. A positive slot permutation and eight negative controls cover
changed retired generations/impulses and malformed spans, children, duplicates
and memberships (`slot-permutation-controls.json`). The native 600-frame audit completed successfully: current runs 1 and 2 are
equivalent under this diagnostic (`native-corrected/run-2-permutation-audit.json`).
This is diagnostic evidence, not a five-run or complete-state qualification claim.

The full Rain captures now cover the first cone discrepancy. In both paths,
state frame130 matches the old health-frame129 replay input exactly in float32
rotation, linear velocity and angular velocity for all42 bodies in cell505–546.
Their captured identities are605–646; target510 is body610, joint509 from
body609. State frames130–132 reconstruct cone excesses of0.300528,0.182272
and0.056586rad, with identical selected results on ordinary/native paths.
The endpoints initially rotate at23.73/22.99rad/s and remain at10.24/5.10rad/s
at the last selected frame: this is a transient recovery, so the stationary
soft-compliance relation is not an acceptance test. The double-precision angle
reconstruction uses libm atan2, whereas the original health report uses Box3D's
float32 polynomial b3Atan2; these reconstructed numbers are not a bit-exact
health comparison. Evidence and extraction/analysis scripts are retained in
`rain-first-cone-full-state`. The first CPU-relative failure at health130 is
not the beginning of the actual cone violation; trace the preceding collision
and loaded solver history next. Full-scene correctness remains unresolved.

The first-cone history extraction now retains frames100–132, including all
cell joints, incident contacts and the bodies at their other endpoints
(`rain-first-cone-full-state/history.jsonl`). The target has no end-of-step cone
excess through122, then0.31106rad at123, coincident with a direct contact
between bodies610 and608 carrying a total normal impulse of115.51. That episode
clears by127; a second episode begins129 without a direct endpoint contact.
These observations locate onset but do not establish causality or correctness.
Both matched-input CPU/GPU60-step replay pairs from health121 and127 complete
with42 exact initial getters per pair and finite outputs
(`rain-cone-precontact-replay`, `rain-cone-second-onset-replay`). Maximum
GPU-minus-CPU cone excess is7.8976e-6rad and0 respectively. The first replay
peaks at~0.215603rad in both engines, below the original full-history episode;
the second peaks at exactly0.249558389rad in both. CPU reproduces the transient
responses from identical inputs, but these cold replays do not establish that
the full-scene history and load transmission are correct.
Current ragdoll run3 completed600steps on both paths and passes unchanged physical
screens; its pose/velocity/separation SHA matches the preceding runs. The native
run1/run3 slot-permutation diagnostic also passes all600frames. Full-state
five-run qualification remains pending.

The incident-contact audit for full Rain frames100–132 finds397 contact-point
observations. The only external body is terrain13, with zero inverse mass
(54 point observations over122–126); there are no contacts with dynamic bodies
outside the42-body cell. Missing neighbouring ragdolls therefore do not explain
this onset-window replay difference. Persistent history, ordering and equivalent
terrain/contact setup remain to be checked (`external-contact-audit.json`).

The selected GpuSim consumer audit maps actual shape identity order to
`shape_storage_order`, opaque submission handles to the enforced completed-step
boundary, and adapter identity to captured metadata. Capture now also checks
sim/world automatic-pose-policy equality and rejects force-clear entries outside
emitted live bodies. Force clearing writes zeros to every previous target before
new loads, so ordering and duplicate zero writes are incidental; the emitted
membership retains the relevant state. Intentional policy/membership corruption
is rejected without appending partial frames on both paths. The existing real
force/next-step-clear and16-step lifecycle tests also pass both paths
(`policy-force-{capture,load,lifecycle}-{ordinary,native}.log`). These are
diagnostic guards, with no schema or production solver change; frozen long runs
predate them. Source hashes and evidence are retained in
`gpu-sim-selected-consumer-audit.json`; this is not a full buffer coverage proof.

A new analytical support fixture is specified before its first execution: three
unit cubes, density1, zero restitution, default gravity and solver tuning,
initially aligned with0.01m gaps above a fixed floor. Over600 steps at60Hz with
four substeps, require finite states, at most0.025m vertical overlap and lateral
drift, and at most0.01 quaternion-vector magnitude (about1.15degrees tilt).
Over the last120steps, center heights must be within0.025m of0.5/1.5/2.5m,
linear speeds below0.05m/s and angular speeds below0.1rad/s. Total mechanical
energy per unit dynamic mass may exceed its initial value by at most0.1J/kg
(the potential energy of a1cm lift under10m/s² gravity); this allows small
contact correction while rejecting sustained energy injection. Uniform cube
inertia is analytically m/6 about each axis. These are bounded stack-support
criteria, not universal pile or restitution qualification. Capture every step
and repeat five fresh processes separately per configuration.

The first600-step analytical stack run passes on ordinary and native paths with
identical reported extrema: maximum vertical overlap0.0012832284m, no mechanical
energy gain above the initial value, and zero final-window linear speed.
All predefined position, tilt, angular-speed and finite-state assertions pass.
Five fresh600-step captured-state repeats per path are now running under
`stack-support-five`; the initial checks do not establish repeatability.

The isolated first-cone60-step capture leaves all pose output byte-identical to
its uncaptured replay. Full terrain13 and isolated terrain1 have identical
transforms and corresponding shape properties, materials, filters and host
geometry arrays. Actual solver settings expose one difference: isolated GPU
contact-recycling distance is0x3d4ccccd versus full scene0x3d4ccccc. Upstream
`physics_world.c` initializes this with `10.0f * B3_LINEAR_SLOP`; the full Sample
also sets that value explicitly. GPU WorldInner/SimParams used literal0.05,
which rounds one float32 step higher. Both defaults now use the shared
`CONTACT_RECYCLE_DISTANCE = 10.0 * LINEAR_SLOP` expression. Post-change stack
checks are running on both paths. Frozen earlier default-world repeats and
isolated replays predate this correction; full Rain already uses the correct
explicit value. This does not establish a cause for the Rain failures. Evidence:
`rain-first-cone-full-state/{isolated-capture-summary,terrain-comparison}.json`.

Both frozen stack batches complete: five600-step runs per path match every
raw captured field and pass physical assertions. Their captured recycling value
is consistently0x3d4ccccd, so they are baseline evidence, not corrected-default
qualification. The post-correction single600-step checks pass on both paths
with unchanged extrema. Separate corrected five-run batches are running in
`stack-support-corrected-five` and explicitly verify0x3d4ccccc in each trace.
The two Rain onset cases are also rebuilding/rerunning against the corrected
native library with actual-state capture (`recycle-default-rain-replays`).

After the default correction, all28 numerical precision fixtures pass on each
path (`recycle-default-precision-{ordinary,native}.log`). Both60-step Rain onset
replays finish with exact initial getter correspondence, finite outputs,
byte-identical CPU/GPU pose output relative to their own preserved baselines,
and unchanged angular traces. Their captured solver settings now match full
Rain exactly apart from the intentionally different step counter. Maximum
GPU-minus-CPU cone excess remains7.8976e-6rad and0. Thus the one-ULP default bug
is corrected but does not explain these episodes
(`recycle-default-rain-replays/comparison-summary.json`). Frozen ragdoll run4
also completes600steps and passes physical checks on both paths; it predates
the default correction and full-state qualification remains open.

The corrected-default stack batches also complete: five600-step fresh runs per
path pass every raw captured field and all physical assertions. Actual recycling
settings are verified in each trace; results are retained in
`stack-support-corrected-five/summary.json`. This qualifies the specified stack
fixture, not arbitrary piles or full persistent-state coverage.

The full Rain cell and corrected isolated replay also match exactly across
all42 bodies and42 joints for the explicitly enumerated captured physical
fields: masses, local centers, inertia tensors including GPU off-diagonal lanes,
shape extents, material response, damping, joint types/endpoints/local frames,
limits, springs, motors and tuning. Joint matching uses both endpoints and type,
because multiple distinct joints may share a child body. The comparison excludes
evolving velocities/transforms, solver colors and accumulated impulses; it does
not assume history equivalence. Evidence and executable comparison are in
`rain-first-cone-full-state/physical-setup-comparison.json` and
`compare-physical-setup.py`.

Device terrain comparison now uses full Rain stateframe122, the impact onset,
and corrected isolated replay frame1. Independently bounded collider ranges
resolve to exactly matching337 vertices,640 triangles,158 mesh-tree nodes and
all remaining collider properties, including filters, materials and instance
transforms. Only verified identity references and heap offsets are mapped.
Thus host and uploaded terrain both agree for this case; no terrain upload
mismatch explains its different full-history response. The executable audit
and results are in `rain-first-cone-full-state/compare-device-terrain.py` and
`device-terrain-comparison.json`. Contact generation/order and accumulated
solver history remain distinct and require investigation.

The fullscene frame123/replay frame1 contact comparison verifies the preceding
42-body rotation/velocity input exactly at fullframe122. All six terrain contact
patches and the target human's three active self-contact pairs have exactly equal
normals, persistent anchors/separations, feature IDs and triangle IDs. Solved
normal-impulse sums differ substantially for the target human's feet (about1048
versus361 and1019 versus359), while the other two humans' foot responses are
close. Some other cell contact memberships differ, so this is not global contact
equivalence. The target discrepancy is localized past matched narrowphase
geometry to preserved solver history/order; these impulse totals alone are not
an acceptance metric. Evidence: `first-step-contact-comparison.json` and
`precontact-input-check.json` under `rain-first-cone-full-state`. A controlled
preserved-history/cleared-cache comparison is the next diagnostic.

A replay-diagnostics-only contact-impulse control now targets a verified live
creation-identity range. It reads and uploads the same contact buffer in sham
and clear modes; clear changes only accumulated normal, friction, twist and
rolling impulses on contacts incident to the selected bodies. Geometry, point
features, allocation, colors and joint history are preserved; both arms invalidate
the idle proof equally. Ordinary/native tests verify byte-identical sham contact
state, exact impulse-only changes, unchanged unrelated contacts and rejection of
invalid body ranges (`contact-impulse-control-{ordinary,native}.log`). The full
native Rain140-step baseline/sham/clear experiment is starting, applying this
control after step122 to bodies605–618. Sham trajectory equivalence must be checked
before interpreting the clear result (`rain-full-contact-control`). This control
is not a solver fix or a new normal-mode behavior.
The ordinary ragdoll runs1–4 storage-permutation diagnostic also completes:
all600frames are equivalent under that explicit diagnostic; existing qualification
gates remain unchanged.

The new full-scene impulse-control baseline completes140steps and matches the
historical GPU run's body and joint-health fields exactly throughout that prefix
(`rain-full-contact-control/baseline-prefix-check.json`). Sham and clear remain
in progress. `cargo check --lib` also passes without replay-diagnostics, confirming
the new opt-in control does not require diagnostic code in the default build.

Controlled performance measurement is prepared for `Determinism/Falling Ragdolls`
and release `Benchmark/Large Pyramid` (5050 dynamic cubes, upstream sleep disabled).
The plan uses five fresh runs per ordering/configuration,60warmup and180timed
steps, completed-step latency, alternating ordering sequence, fixed rendering
settings and no health/state instrumentation. Actual order-disable/capacity
failures invalidate a measurement. Run only after competing GPU qualification
jobs drain; these are planned measurements, not performance evidence
(`performance-measurement-plan.json`).

The source-machine plan file is unavailable on this desktop. The following
protocol is the recoverable measurement contract, fixed before measurements:
use five paired fresh-process runs of each ordering for each named scene and
each compiled path. Alternate which ordering runs first in each pair. Freeze
binary hashes, native runtime settings, adapter/driver/backend, upstream scene
defaults, timestep/substeps and rendering settings. Use60warmup and180timed
completed steps, and retain every timing row, exit status and configuration.
Run no competing GPU job. Prewarm pipeline compilation identically outside the
measured runs; record that procedure and cache identity. State, lifetime, health,
phase capture and optional profiling windows must be disabled. A measurement
with incorrect completed-step sequencing, capacity loss, fallback ordering,
process failure or environmental interference is invalid, retained with its
reason, and replaced as a whole paired block. Slowness alone is not a reason
to discard a run. Verify the harness's completed-step timing boundary before
using its timing field; host enqueue time is insufficient.

Source inspection establishes the current harness boundary:
`scripts/inject-sokol-bench.py` marks `physics` immediately before
`b3World_Step`, invokes `b3World_Wait` when `--completed-step` is set, then
records world counters before the `profile` mark ends that interval.
`src/c_abi.rs::gpu_b3_world_wait` waits and synchronizes completed poses into
host getters; `GpuSim::wait_completion` waits on the submitted queue index.
Thus `physics_ms` in this mode measures completed application physics latency,
including pose synchronization and bounded counter collection, not pure shader
time. Health/body/joint traversal is skipped without `--health-scan`.
Require `--completed-step --unpaced`, record this cost boundary, and keep it
identical for both orderings. The report's completed-step labels alone are not
proof of completion: the actual wait call is the source evidence.

The default decision requires correctness and repeatability gates first.
For performance, calculate the arithmetic mean completed-step latency within
each run, then each paired log ratio `log(GPU-order / CPU-compatible-order)`.
Report all five ratios, their mean, sample standard deviation and the one-sided
95% Student-t upper confidence bound (`mean + 2.131847 * sd / sqrt(5)`, df4).
Require this upper bound below zero independently in all four scene/path cells
before claiming a demonstrated performance reason to switch the default.
This is a per-cell small-sample screen, not a simultaneous confidence guarantee
or a promised speedup. Also report every run's p95 and maximum latency; a
regressing tail requires investigation before a switch, even if means pass.
An inconclusive or failing screen leaves GPU ordering opt-in and must appear
explicitly in the default decision. Do not pool paths/scenes or add repeats
selectively until a significance threshold passes. Measurement evidence and
the decision are still open.

An analytical sliding-friction fixture is specified before execution: two
unit cubes on a level floor, both launched at2m/s with zero restitution and
damping. Floor friction0.5 combines with cube friction0.5 or0 to give effective
friction0.5 or0. With gravity10m/s², Coulomb sliding predicts0.4m stopping
distance and0.4s stopping time for the frictional cube. Over240steps at60Hz,
allow0.04m stopping-distance error (slightly over two initial full-step travel
distances), require speed below0.05m/s after0.6s, and require the frictionless
control to remain within0.005m/s of2m/s and0.005m of2t. Require both heights
within0.01m of0.5m, lateral drift below0.01m, finite values, and angular speed
below0.1rad/s after1s. This is a flat-support kinetic-friction check, not a
static-friction or arbitrary contact-material qualification.

The full Rain contact-cache experiment completes all three140-step arms.
Baseline matches the historical prefix; sham matches every recorded body and
joint-health field for all140frames. The control selects6patches,3with nonzero
cached impulses. Clear remains identical through health121, then reduces the
first cone spike at122 from0.311083078 to0.248825252rad. However its later peak
at130 is0.362470627rad versus baseline0.182287276rad, exceeding the original
whole-window maximum0.311083078rad. Therefore clearing contact history is not a
fix; it changes the response and worsens the later event. Joint history and
solver ordering remain to be examined (`rain-full-contact-control/analysis.json`).

The analytical sliding-friction fixture passes240steps on both paths with equal
reported results:0.39577165m stopping distance versus0.4m analytical, and maximum
frictionless position error6.198883e-6m. All predefined support, stopping-speed,
angular-speed and finite-state limits pass. Five fresh captured-state repeats per
path are starting (`sliding-friction-five`); this initial result is not yet
repeatability qualification.

The sliding-friction batches finish: five fresh240-step runs per path pass all
physical assertions and every raw captured field independently. The ragdoll
batches also finish all ten600-step runs with physical gates passing and identical
pose/velocity/separation hashes. Their raw comparison still fails occupied-contact
order; five-run slot-permutation diagnostics are running on both paths. These
ragdoll binaries predate the contact-recycling default correction.

For the target Rain human, all14 joint colors match between fullscene frame123
and the corresponding isolated replay, while substantial preceding joint impulses
remain at fullframe122 (`rain-first-cone-full-state/target-joint-history.json`).
This excludes a difference in those color assignments alone, but does not establish
complete component/dispatch order or eliminate the effect of warm joint history.

The diagnostic history experiment now has an independent spherical-joint arm.
It clears only cached point, spring, motor, cone and twist impulses on selected
spherical joints, preserving tuning, local frames, flags/colors, other joint
kinds and contact state. Sham performs the same readback/upload; it deliberately
does not rebuild joint topology or filter tables. Ordinary/native tests verify
byte-exact sham preservation and exact impulse-only changes while retaining an
unselected joint; the prior contact-control tests also pass. Four full Rain arms
are building/starting at the same step122 boundary: baseline, spherical sham,
spherical clear, and spherical+contact clear (`rain-full-spherical-control`).
This is diagnostic only; no default solver behavior changes.

An analytical restitution fixture is specified before execution: three radius0.5m
spheres dropped from center height2.5m onto a level frictionless floor, with
restitution0,0.5 and1, no damping, default gravity10m/s² and continuous collision
enabled. Their first rebound center heights should be1.0m and2.5m for the two
bouncing spheres, from h_rebound=e²×2m; allow0.05m error for discrete stepping and
contact correction. Require observed descent and a completed first apex, at most
0.03m floor penetration, lateral displacement below0.01m, finite values, and no
specific mechanical energy above25.2J/kg (initial25J/kg plus a2cm lift allowance).
The zero-restitution sphere must settle within0.01m of center height0.5m and
below0.03m/s after1.5s. Run240steps at60Hz/four substeps and capture every step.
This covers vertical sphere/floor impacts, not all restitution geometries.

The current ragdoll standalone slot-permutation audits now complete all five
600-step runs independently on both paths. They report equivalent semantic
records under that diagnostic; raw qualification still fails occupied slot order.
These frozen binaries predate the recycling-default correction.

The spherical-cache Rain experiment completes all four140-step arms. Baseline
matches the historical prefix and sham matches every body/joint-health field.
Clearing spherical history first changes health122 and reduces the window's peak
cone excess from0.311083078 to0.24056679rad; clearing contacts as well gives
0.28682518rad. This implicates warm joint history in the response but does not
establish a faulty cache or justify clearing it in production. See
`rain-full-spherical-control/analysis.json`.

The restitution fixture fails its original30mm penetration screen on both GPU
paths atstep115 (38.137mm). A matching240-step C reference reproduces that event
and reaches49.311mm atstep192, matching GPU. Maximum CPU/GPU pose/velocity scalar
difference is9.5e-7. First rebound apices are0.982001245m and2.47957182m, within
the predefined limits; peak specific energy is24.99652729J/kg, below initial25.
The penetration screen remains failed. Upstream solver.c activates CCD only when
motion exceeds half the minimum extent; continuous enabled does not imply every
impact receives a sweep. Further diagnosis must establish the relevant motion
and contact cadence. The test now defers only its penetration assertion until
all240steps are captured, retaining the original threshold and failing trace.

After interruption, the old full-Rain session handles are missing and process
inventory confirms no remaining capture jobs. Both incomplete run1 files are
preserved. A native five-run batch restarts from the exact saved binary under
`rain-full-state-recovery`; its manifest retains the original source provenance.
Ordinary recovery is pending. Interrupted prefixes are not completed repeats.

Prior evidence is under `experiments/gpu-physics/artifacts/ragdoll-gpu-order`,
`rain-contact-order`, and `ragdoll-impact-goal/qualification`. New qualification
artifacts belong under `artifacts/gpu-solver-qualification` with binary/source
identity, exact environment and hardware/driver details. All open matrix entries
must be resolved or explicitly classified as unsupported before claiming the
solver is equally capable within this matrix.

The revised ordinary restitution diagnostic captures all240steps and passes
its rebound, finite-state, settling, lateral-drift and energy assertions, then
fails the unchanged penetration screen at0.049310952m (exit101). Its sampled
first-apex maxima are0.98304284m and2.4806135m (the reference summary above
reports the last positive-velocity samples). The full trace is retained in
`restitution-reference/gpu-restitution-complete-ordinary.jsonl`.

Native restitution also completes240steps and fails only the retained30mm
penetration screen, with the same49.311mm peak. Full failed traces are retained.
The ordinary trace shows the first failed step starts66.377mm above the floor,
outside the20mm speculative range, and moves104.514mm, below the250mm CCD
activation threshold. There are no manifold points during that step; one appears
on the next step and reverses velocity. Four-substep free flight predicts the
failed-step height within1.73e-8m. The restitution1 sphere exceeds30mm only at
steps115 and192, each for one step (`restitution-reference/contact-cadence-analysis.json`).
This explains a shared discrete-contact limitation; it does not pass or relax
the original screen.

The desktop CCD-coverage candidate closes that gap without changing the scene,
material coefficients, timestep, speculative distance or physical assertions.
It caps the former half-minimum-extent activation threshold at the existing
20mm speculative range. Smaller bodies retain their smaller cutoff. Host CCD,
the GPU motion classifier, convex device CCD and fast/proxy flags use the same
bound. This changes when the solver performs continuous collision work; it
does not change geometry or increase the allowed penetration. Static, sleeping,
disabled, kinematic and bullet routing/exclusion rules remain in place.

Five fresh240step runs independently pass on ordinary and native cached paths,
with byte-identical captured state within each path. All ten runs report
peak penetration0.004999995m, rebound apices0.99096805m/2.488538m and peak
energy25.017302J/kg; every original settling/finite/lateral assertion passes.
Evidence: `desktop-baseline/restitution-ccd-five/{ordinary,native}` and
`summary.json`, with frozen binaries, exact commands/configuration, source diff,
hashes, receipts and complete traces. Original failed baseline traces remain
under `desktop-baseline/{ordinary,native}`. Neither CPU reproduction nor a
relaxed tolerance was used to pass this screen.

The classifier regression retains endpoint ordering, rotation, flags and the
65-body workgroup boundary, and now tests the20mm cutoff, just beyond it, the
former extent-only gap and the smaller-body threshold. Both paths independently
pass27CCD tests,28numerical tests,600step support/energy,240step friction and
18step mass mutation (58checks each). Logs and receipts are in
`desktop-baseline/restitution-ccd-coverage/{ordinary,native}-regressions`.
This is a
provisional production correction: full Rain/ragdoll behavior, remaining
persistent-state qualification and controlled performance must still be checked.
The ongoing frozen Rain baselines predate this candidate and are diagnostic
history for the prior behavior, not final-candidate qualification.

The candidate also completes the full upstream600step Falling Ragdolls case
once on each path and passes `check-ragdoll-health.py` unchanged against a fresh
independent CPU fixture. Both paths report peak separation0.213310137m,
final-window separation0.00138041785m, zero final-window linear/angular speeds
and peak angular speed103.3440986rad/s within the1.2×CPU screen. Build inputs,
library/source hashes, CPU/GPU pose files, complete state, receipts and physical
reports are in `desktop-baseline/ragdoll-ccd-candidate/`. Fresh five-process
batches are running in `desktop-baseline/ragdoll-ccd-five/`; these single-run
results do not close repeatability or the persistent-state audit.

The first spherical warm-start source audit finds matching cache signs, base-axis
preparation and per-substep application. GPU retains base body rotations until
final apply_deltas and rotates lever arms by the current delta; CPU uses the
same structure. No production change follows from this inspection. A fullscene
cached-history comparison is still needed to distinguish valid warm-start
response from an implementation defect.

A native full-Rain solver-list audit passes captured frames100–132, spanning
the first cone episodes. Every live dynamic joint appears exactly once; writable
endpoints belong to one component; scheduled contact identities are unique;
nonoverflow colors have no writable endpoint conflict across joints and contacts.
Duplicate, missing and cross-component joint corruption controls are rejected
(`rain-first-cone-full-state/solver-ownership-audit.json`). This checks captured
schedules, not actual dispatch execution or transient races.

The existing80-component regression passes on both current test binaries: the
small joint/contact wave and general color dispatch retain exclusive ownership,
expected ground-contact colors and final poses within1e-5 over10steps. Logs are
`joint-wave-general-{ordinary,native}.log`. It is a focused dispatch-path check,
not five-run qualification or a full-Rain warm-history comparison. Source review
also identifies a robustness question in the invalid-component-list fallback:
local lane0 can exist in multiple dispatched workgroups. Audited Rain lists are
valid, so this cannot explain those captured episodes; a deliberate fallback
control is needed before that path can be considered qualified.

Invalid joint-list fallback diagnosis reproduces a small-wave omission. With
a forced invalid list and zero component count, a spherical joint's cached unit
impulse leaves the unit-mass body's x velocity at-2 instead of-1. The existing
multi-workgroup branch also selects local lane0 per workgroup, permitting races,
although1/2/64-workgroup tests did not expose a visible duplicate impulse on this
device. The candidate uses global invocation0 and explicitly enters fallback
from the small-wave path when its list is invalid. The regression covers both
paths, unchanged static endpoints and zero angular response. Before-fix evidence:
`joint-fallback-wave-before.log`; ordinary/native candidate tests are running.
This is fallback robustness, not a claimed cause of Rain's valid-list episodes.

The native Rain recovery capture also stopped during interruption: its handle
is missing and process inventory confirms no remaining capture process. The
partial319MB trace is preserved; no completed repeat is claimed. Full qualification
must resume after the fallback change is validated, with binary provenance kept
separate from prior frozen builds.

The fallback fix passes its forced-invalid-list regression independently on
ordinary and native cached builds. Each tests general dispatch with1,2 and64
workgroups plus the small wave, requiring exactly one unit impulse, zero torque
and an unchanged static body. The existing80-component valid-list small/general
path regression also passes both builds after the change. Logs are
`joint-fallback-{ordinary,native}.log` and
`joint-fallback-wave-regression-{ordinary,native}.log`. No Rain qualification
batch is currently live; sample binaries must be rebuilt before new final-build
repeats. The upstream submodule remains clean.

Both ordinary and native sample archives/binaries rebuild successfully with the
fallback fix (`fallback-sample-{ordinary,native}-build.log`). New frozen five-run
600-step Rain batches are live under `rain-fallback-fixed-five`; manifests retain
source hashes, binary hashes and independent runtime settings. Each reaches its
first captured frame. Comparing that frame to its earlier frozen build finds
only contact-allocation bookkeeping differences; all other captured groups match.
Both allocation records are retained in `startup-prefix-comparison.json`. This
is a cross-build startup diagnostic, not repeatability qualification.

A CPU-only diagnostic bridge now supports exact spherical cached-impulse
transplants without editing upstream sources. The isolated cell receives all27
spherical caches from full Rain state122; point, spring, motor, lower/upper twist
and swing impulses are copied and checked bitwise. Endpoint identities and
complete spherical-joint coverage are validated. All42 initial body getters match
across cold, zero-cache and transplanted arms. Zero-cache output is byte-identical
to cold; cold is byte-identical to the prior CPU replay. Missing, duplicate, wrong
parent and nonfinite payload controls each reject with exit6.

Over60steps, target510 peak cone excess rises from0.215602487rad cold to
0.400575578rad with captured spherical history; the first-step excess rises from
zero to0.186978012rad. Thus the same spherical history materially changes CPU
response too. Contacts and revolute/distance caches remain fresh, so this does
not reproduce fullscene history or clear the GPU implementation. Evidence and
provenance: `rain-cpu-spherical-history/{result,controls,manifest}.json`. The bridge
is linked only into diagnostic CPU fixtures; normal solver behavior is unchanged.

The matching GPU spherical-cache transplant bridge is feature-gated behind
replay diagnostics. It validates live joint generation, finite input and a
completed simulation boundary, updates only the selected joint's12 cached
impulse floats, reads back exact bytes and invalidates the idle proof. It does
not rebuild joint topology or filters. Ordinary/native controls confirm all
other joint bytes remain unchanged and reject stale IDs/nonfinite values without
mutation. The native cold/zero/captured-cache60-step comparison is running under
`rain-gpu-spherical-history`; the full-Rain frozen binaries do not invoke this
optional diagnostic bridge.

The default build without replay diagnostics passes `cargo check --lib` after
the transplant addition (`spherical-transplant-default-check.log`).

The matching native GPU spherical-history replay completes all three60-step
arms. All42 initial body getters and all27 cache payloads match CPU exactly;
zero-cache poses are byte-identical to cold on each engine. Warm target510 peaks
at0.400575578rad CPU versus0.400574982rad GPU; maximum per-step cone-excess
difference is9.506941e-6rad. Cold/zero maximum cone difference is3.66569e-5rad.
All2520 frame/body identities and finite values match per arm. Across all bodies
in the warm arm, maximum component differences reach3.071mm position,0.00745038
quaternion component,0.051636m/s velocity and0.479692rad/s angular velocity; close
target-cone agreement is not global trajectory equality. Results are diagnostic,
not new acceptance limits or fullscene clearance. See
`rain-gpu-spherical-history/{result,all-body-comparison}.json`. Full contact and
remaining joint history remain unmatched.

After the fallback fix and diagnostic additions, all28 numerical native_precision
regressions pass independently on ordinary/native cached paths (exits0), recorded
in `post-fallback-precision-{ordinary,native}.log`.

The cell contains27 spherical,12 revolute and3 filter joints; there are no
distance joints in this captured setup. The CPU diagnostic now transplants all39
active joints' accumulated impulses. Revolute point/perpendicular/spring/motor/
lower/upper impulses are copied exactly; per-step axes are prepared normally
from the matched poses, rather than copied as stale derived state. All12 hinge
records have exact readback. Missing/duplicate/wrong-parent/nonfinite controls
reject; the zero-hinge-cache arm remains byte-identical to the prior spherical
replay, including all42 initial body getters.

The all-joint CPU replay peaks at0.36046648rad versus0.400575578rad for spherical
cache alone. Its first-step cone excess0.248825252rad exactly matches the full
GPU contact-clear experiment. Over the first five steps the maximum target-cone
difference is0.00020313263rad; over the18-step overlap
it reaches0.0025977493rad. Other humans' contact
history remains unmatched, so this does not establish fullstate equivalence.
Evidence: `rain-cpu-all-joint-history/{result,controls,full-contact-clear-comparison}.json`.
A matching GPU hinge-cache transplant and the original contact-history
investigation remain outstanding.

The GPU diagnostic now also restores the nine revolute impulse values. Ordinary
and native controls compare complete joint bytes, preserving prepared axes,
settings and an unselected joint; stale/nonfinite/wrong-kind inputs reject without
mutation. Both controls and the default build pass. The native archive rebuilds
and the three-arm60-step comparison with all39 active joint caches is starting
under `rain-gpu-all-joint-history`. Full-Rain capture binaries remain frozen;
this optional diagnostic addition does not alter their solver behavior.

The all39-joint native GPU replay completes all three60-step arms. All initial
body getters and cache payloads match CPU; zero hinge history is byte-identical
to spherical-only. With all caches, target510 peaks at0.36046648rad CPU versus
0.360465467rad GPU; maximum per-step cone difference is7.8678131e-5rad. Across
all2520 matched finite body records, maximum component differences are0.416mm
position,0.002869 quaternion,0.019161m/s velocity and0.160769rad/s angular velocity.
This supports the joint-history implementation in this isolated replay but does
not qualify full Rain. Evidence: `rain-gpu-all-joint-history/{result,all-body-comparison}.json`.

At fullstate122 the target human has six active contact patches; three have
nonzero cached impulses: terrain/feet612 and614, and selfcontact608/606. The
foot normal caches are165.189 and157.922 impulse units, corresponding to
normal-only warm-start speed changes17.449 and16.681m/s at their actual masses.
These are contributions before subsequent solving, not net motion or an energy
acceptance measure. Friction, rolling, feature and point data are retained in
`rain-first-cone-full-state/precontact-cache-inventory.json`. A matched contact
history replay remains necessary. CPU zero-time stepping updates collisions
without velocity solving, providing a possible way to seed manifolds; its
state-preservation effect must be verified before using it as a control.

CPU zero-time contact seeding fails the stronger no-op control. It preserves
all42 body position/rotation/velocity/awake readbacks bitwise immediately, but
alters every subsequent body row across60steps. First difference is frame0/body505
(max scalar1.81e-5); the largest scalar difference is0.15787339 atframe59/body526.
The ordinary baseline remains byte-identical to the prior all-joint replay.
`rain-cpu-zero-step/{result,manifest}.json` and manifold logs preserve this result.
Do not use this extra collision pass as a neutral preparation stage.

The public pre-solve callback cannot serve as a supported cache setter: it has
no mutable manifold argument and explicitly forbids world mutation. A diagnostic
linker wrapper around b3Solve is the next candidate boundary: physics_world.c
calls it after the normal collision update and before constraint preparation.
A no-op wrapper must reproduce the baseline exactly before cache injection;
feature matching and friction/normal reprojection must also be audited. No
contact-cache transplant is implemented or qualified yet.

Both fallback-fixed full Rain batches terminate at state81 with the same
validation error: GPU joint collision filter differs from captured joints. The
last complete frame is80, with1360 bodies and1260 joints. This is a captured-state
failure, not an interruption or completed repeat. Ordinary fixture exits134;
native wrapper exits1 after the fixture abort. Original traces/logs are retained.

The guard now preserves expected/actual filter tables and host/device joint
endpoints/kinds/flags in a sibling filter-failure JSON before returning the same
error. An optional sample trace-start frame supports bounded diagnosis; it is
unset for full qualification and never replaces every-step repeat checks. A
native81-step reproduction capturing80/81 is starting in `rain-filter-failure`.
The CPU solver-hook work is deferred until this newly observed failure is
classified and corrected.

The bounded native run reproduces the frame81 filter failure. All12288 actual
filter words are zero at the expected address; host/device joint endpoints, kinds
and collision flags agree. The scene now has1780 bodies/1680 joints. Source
inspection identifies an in-place update ordering defect: write_joints uploads
the filter using the previous live body count, then write_params changes the
count and hence the shader's filter address. The small body-growth regression
is running before applying a correction. Failure data remain in
`rain-filter-failure/state.filter-failure.json`; no acceptance guard was relaxed.

A minimal regression reproduces the same failure by growing from two to three
bodies while body/joint capacities remain unchanged (`joint-filter-growth-before.log`
and `joint-filter-growth-before.json`). The candidate updates live parameters
before in-place scene remapping and joint uploads, ensuring scratch addresses
use the new body count. Both-path regression runs are pending. This is a real
collision-filter state defect; its effect on Rain trajectories still needs
measurement, and previous valid-joint-list audits do not cover this table.

The body-growth filter regression now passes independently on ordinary/native
after the update-order fix. Native bounded Rain81-step verification is rebuilding
and starting in `rain-filter-fixed`; no fullscene stability improvement is claimed
until that and the broader qualification runs complete.

The corrected native Rain run completes all81steps and passes the unchanged
filter guard at captured frames80/81. All81 body/joint health frames are exactly
identical to the historical baseline (`rain-filter-fixed/result.json`): this
proves the table correction, but demonstrates no physical improvement in that
window. The stale-table rejection test, lifecycle fixture and all28 numerical
regressions pass independently on both GPU paths after the fix. Both sample
binaries have been rebuilt. Fresh five-run600step qualification batches are
being launched in `rain-filter-fixed-five`, capturing every step from1; prior
failed batches remain preserved and are not counted as completed repeats.

The CPU solver-boundary observation now passes its no-op control. An explicitly
linked diagnostic wrapper around b3Solve observes the normal collision output
and forwards to the real solver exactly once. Across60steps, all2520 finite
body-state rows are byte-identical to the uninstrumented baseline, which also
matches the prior all-joint-history replay. All60 calls are observed, including
20 contact patches on the first step. Logs retain body/shape endpoints, manifold
normals, feature IDs, anchors, separation and accumulated impulses for matching
against GPU history (`rain-cpu-solver-boundary/{result,manifest}.json`).
This validates read-only observation for this fixture; contact cache injection
and its feature/orientation/reprojection checks remain pending. Upstream sources
are unchanged. The corrected full Rain batches remain running on both paths;
no completed repeat is claimed yet.

The capture-throughput investigation identifies unbuffered serde_json file
serialization as a concrete bottleneck. The writer now buffers1MiB and explicitly
flushes each complete frame, preserving error propagation, schema and all fields.
Three serialization trials on Rain frame121 (20,026,862bytes) have median raw
24.63787s versus buffered0.101028s; all six output SHA256 hashes match exactly.
This ~244x improvement applies only to serialization, not physics or total capture.
The growth/filter capture regression passes ordinary and configured native paths.
Evidence: `trace-buffering/{bench.rs,timings.txt,result.json,*test.log}`.
End-to-end measurement and sample rebuild remain pending; no full-repeat pass
is inferred from this diagnostic optimization.

Buffered capture end-to-end evidence: ordinary10steps take103.224s unbuffered
and12.185s buffered (8.47x diagnostic wall-time ratio). Both output91,152,233bytes
and identical physical health. Every captured group matches except contact_allocation
over10steps; this difference remains explicit. Buffered81steps complete in80.392s
and all81 health frames exactly match the earlier baseline. Native comparisons
remain running. Evidence: `rain-buffered-capture/ordinary/comparison.json`.
The new sample repeat runner preserves attempts, validates frozen configuration
and completed compressed evidence, and recovers completed processes interrupted
during compression. Five helper controls pass; full process-resumption validation
is pending before qualification use (`rain-buffered-capture/recovery-tests.log`).

Both buffered comparison paths are complete. Native10steps take110.664s raw
versus13.243s buffered (8.36x diagnostic wall time); native81steps finish75.956s.
Both paths match every81step physical-health frame of the prior baseline. The
only10step state difference is occupied_order, with identical membership on
every frame; no other captured group differs. Results and field-level differences
are in `rain-buffered-capture/result.json` and each mode's allocation-differences.json.
This does not establish complete-state determinism or solver performance.

The resumable sample runner passes actual subprocess interruption/restart controls
in addition to its five helper tests (`repeat-runner-recovery/result.json`). A
live duplicate is rejected, completed evidence stays byte/mtime identical, the
interrupted attempt is retained, and only its replacement is launched. Five
completed synthetic runs use six launches; another resume launches none. A binary
change is rejected. The synthetic fixture replays one real captured frame; this
qualifies runner recovery, not GPU physics. To start or resume a frozen Rain batch
from the experiment directory, use the same arguments on each invocation:

```sh
python3 scripts/check-sample-state-repeats.py --binary native-samples/build-gpu/bin/samples_gpu --out artifacts/gpu-solver-qualification/rain-buffered-five/ordinary --configuration ordinary --steps 600 --runs 5
python3 scripts/check-sample-state-repeats.py --binary native-samples/build-gpu-native-cache/bin/samples_gpu --out artifacts/gpu-solver-qualification/rain-buffered-five/native --configuration native-cached --steps 600 --runs 5
```

These commands are documented, not yet executed. After rebuilding, resume only
with the matching saved fixture or create a new candidate batch; never mix builds.
The runner reports raw and audited-membership results separately and retains a
failed raw comparison as failure. Physical acceptance and persistent-state
coverage remain separate required gates.

CPU solver-boundary matching now verifies the three target-human contacts with
nonzero prior caches. CPU boundary1 and fullGPU persistent collision geometry123
have bit-identical normals, equal feature IDs and anchor/separation differences
at most2.98e-8m. Four of five points match exactly. Mesh triangle IDs differ
(CPU406/241 versus GPU471/248); convex no-triangle uses-1 versus0. Geometry
matching supports a common contact configuration, but triangle mapping and
friction reprojection must be audited before injecting old caches. No cache
restoration or stability pass is claimed. Evidence:
`rain-cpu-solver-boundary/{compare-target.py,target-contact-comparison.json}`.

Triangle identity audit is complete for the three target contacts. GPU contact
triangle values are array-index+1 (0 is no triangle); they must be decoded before
indexing mesh arrays. All512 CPU torus triangles map bijectively to bit-identical
ordered GPU vertices. CPU406/241 map to GPUarray470/247, encoded471/248, exactly
the observed IDs. An initial direct-index diagnostic falsely suggested adjacent
triangles; that inference is withdrawn. CPU-1 maps to GPU0 for convex contact.

The source audit and native float32 probe establish the friction payload
conversion: reconstruct world impulse from old GPU tangent coefficients, then
let CPU preparation project into the new normal basis. GPU collision performs
the corresponding reprojection. Ordered endpoints match for all three contacts;
no sign flip is required. Preserve rolling world vector, twist scalar, and
last-substep normal impulse rather than total accumulated impulse. The self-contact
friction coefficients change from approximately(-9.19126,-13.66360) to
(-9.18339,-13.63740) after projection, illustrating why raw scalar copying is wrong.
Evidence: `rain-contact-history-audit/{audit.json,triangle-mapping.json,friction-world.txt}`.
Controlled history injection and the resulting replay are still pending.

Controlled CPU contact-history restoration is now implemented only in the
explicitly linked diagnostic b3Solve wrapper. The payload covers the3audited
target-human contacts, validates allrecords before mutation, and requires ordered
body/shape endpoints, exact normals, unique features/decoded triangle IDs and
geometry within1e-6m. Only cached normal/friction/twist/rolling impulses change.
Three60step arms (baseline,zero,warm) each produce2520finitebodyrows. Zero output
is byte-identical to baseline; baseline matches the prior reference. Missing
records, wrongfeature, wrongtriangle, nonfiniteimpulse, wronggeometry and duplicate
manifold controls all reject with exit8 before solving.

Restoring these3contact caches alongside all39joint caches makes CPU reproduce
the fullGPU targetjoint's onset cone excess0.311083078rad exactly. The next2steps
also match exactly; first5steps differ by at most1.192e-7rad. Over18steps the
maximum discrepancy is0.012300193rad, with other contacts/globalordering still
unmatched. This identifies missing contact history as the cause of the initial
matched-state replay discrepancy and demonstrates that CPU reproduces this early
residual. It does not establish physical acceptance, fullscene equivalence, or
clear later persistent/severe episodes. Evidence:
`rain-cpu-contact-history/{result.json,full-gpu-comparison.json,manifest.json}`.

The validated resumable Rain commands are now launched under
`rain-buffered-five/{ordinary,native}`, requesting5fresh600step runs perpath.
The frozen sample hashes match the bounded buffered comparison exactly; each
batch retains provenance and allattempts. Both run1 processes are live at this
checkpoint; no completed repeat or physical pass is claimed. They are concurrent
diagnostic captures, not performance measurements.

`rain-residual-classification.json` separates the resolved early replay-history
discrepancy from later unresolved fullscene episodes. Existing matched-pose,
fresh-history replays reproduce severecone and persistenttwist behavior on CPU,
but fullscene cache/load correspondence and physical acceptance remain open.
The new full captures are needed for those later windows, particularly persistent
twist375–463 and severecone near516, after reconciling creation identities.


Health-to-core identity mapping now resolves each instrumented Human body through
its public slot and generation at the same completed physics step, rejecting
stale/deleted/duplicate identities and mismatched steps. This avoids assuming a
constant offset between the Human-only and whole-world creation counters.
All15identity tests pass. Both bounded81step buffered captures pass mapping for
68,880body observations each; quaternion, linear velocity and angular velocity
float32 bits agree exactly. Evidence: `rain-health-core-identity/{check.py,result.json}`.
Actual recycling and later episode extraction still require the corresponding
full-run health records. No new physical acceptance or repeatability pass is claimed.


The later Rain diagnostic extractor (`rain-later-full-state/extract.py`) is now
validated on11bounded frames and9historical contact-rich frames. It resolves the
42cell bodies through same-step slot/generation mappings and checks their q/v/w
float32 bits against health. It retains external contact partners and step-start
transforms: the contact-rich control includes1terrain partner and71–72patches,
alongside42joints. Outputs are labelled diagnostic subsets, not complete-state
qualification evidence. Current persistent/severe windows await completed run1
health; controls do not clear those failures. Evidence:
`rain-later-full-state/{bounded-control,contact-control}.manifest.json`.


The island-wake fixture adds the explicitly missing multi-body joint propagation
check. Its limits above were fixed before execution: momentum and energy follow
from a centered1N·s impulse on a1kg cube with no external forces; the1e-4 numerical
allowance is distinct from the1cm constraint sanity screen. It records all73steps,
including64quiet steps, one wake=false probe and8steps after wake=true. The fourth
body is disconnected and must stay asleep. Compilation/qualification is pending;
no pass is claimed from adding the fixture. Sample binaries used by the running
Rain batches are frozen and this change is test-only.


Joint-island wake qualification now passes on both paths:5fresh processes each,
73captured steps with raw state equality. All ten runs pass the fixed wake,
isolation, momentum, energy, spin and length assertions. Peak momentum error is
2.3841858e-7kg·m/s, energy0.166690504J, length error0.000353443m, spin0.
Evidence: `island-wake-five/{summary.json,ordinary/result.json,native/result.json}`.
The runner records frozen test binaries/configuration and requires one passing
test plus the exported trace, excluding an initial featureless zero-test probe.
This covers joint-connected wake propagation, not all contact-island behavior or
the remaining persistent-state coverage audit. Rain captures continue unchanged.


Before executing the remaining contact-island check, the same73step fixture is
extended with three touching1m unit-mass cubes and no joints. Both touching links
must exist after64quiet steps. The same wake=false/wake=true, separate-body sleep,
momentum<1e-4kg·m/s and energy≤0.5001J checks apply. Contact-specific screens are
penetration<0.01m, transverse relative drift<0.001m and spin<0.001rad/s; these allow
small multipoint solver asymmetry while detecting loss of support or spurious
motion. Five fresh runs per path and raw per-step state comparison are required.
Fixture: `trace_contact_island_wake_propagation`; results pending.


Contact-island ordinary run1 passes actual sleep, both touching links, wake=false,
wake=true propagation and independent-island isolation, then fails the spin screen
at step66:0.0192691538rad/s versus0.001. No repeated/native qualification is claimed.
CPU reference (`c_abi/island_wake_reference.cpp`) completes73steps with wake checks
passing and reproduces the same spin magnitude; frame66 transverse velocities
and spins reflect signs while magnitudes match GPU. The original screen fails on
both engines and remains recorded. Shared behavior does not alone establish
physical acceptance. Next assess total angular momentum/energy over the fullGPU
window, preserving this screen if a conservation-based criterion is justified.
Evidence: `contact-island-reference/{manifest.json,result.json,cpu.txt}` and
`contact-island-wake-five/ordinary/run-1.log`. This does not invalidate the
separate passing joint-island case.


For full-window contact-island diagnosis, the unchanged individual-spin screen
is evaluated after step73 and trace export; it still fails the test. A diagnostic
five-repeat runner requires that exact failure plus complete traces and reports
repeatability separately from failed physical screening. Before inspecting the
new full captures, total angular momentum is screened at1e-4kg·m²/s around its
initial zero value (centered axial impulse, no external torques). Compute
Σ(position×mass×velocity + world_inertia×omega); unit cubes have isotropic inertia
1/6. This tests conservation rather than assuming each cube's spin must be zero.
Original momentum/energy/penetration/transverse/spin limits remain recorded.


Full contact-island diagnosis (`contact-island-full-v2-five`) completes5fresh
73step captures per path. Each Rust test still exits101 for the original symmetry
screens; the diagnostic runner requires that specific failure and a full trace,
then independently reports raw state repeatability (passes both paths). It does
not relabel failed tests as passing. Wake/isolation, linear momentum, energy and
penetration assertions pass throughout the window.

All10GPU captures match CPU p/v/w exactly under Y reflection across all73steps:
polar p/v signs[+,-,+], axial omega signs[-,+,-], maximum reflected error0.
Conservation metrics are identical: peak linear-momentum error5.97146e-8kg·m/s,
angular momentum1.36406e-6kg·m²/s (predeclared limit1e-4), energy0.167829651J,
penetration8.08239e-5m. Spin0.0192691538rad/s and transverse drift0.001521538m
fail the original ideal-symmetry screens on both engines. This identifies a
reflection-equivalent numerical trajectory rather than a GPU-only wake failure;
it does not erase those failed screens or silently substitute a new acceptance
gate. Evidence: `contact-island-full-v2-five/{analysis.json,ordinary/result.json,native/result.json}`.

A bounded desktop ordering experiment reversed contact-point traversal during
the relaxation pass only in GPU-native ordering, retaining the forward biased
pass and the same iteration count. Both paths completed73frames and failed the
unchanged symmetry screens more severely: peak spin0.028980222rad/s and
transverse drift0.0015639528m, versus baseline0.019269153/0.001521538.
Wake, isolation, momentum, energy and penetration assertions still passed.
The candidate was rejected and the original shader restored byte-for-byte;
both test builds were rebuilt after restoration. Artifacts, frozen binaries,
full traces, failed receipts, candidate patch and rejection decision remain in
`desktop-baseline/contact-symmetric-sweep/`. This rules out this simple sweep
change as the fix; it does not relax the original screens or qualify the case.


Persistent-state audit correction: timing windows affect native replay eligibility
through MetricsRes.steps/submitted. These counters are now captured as optional
`gpu_policy.timing_window`; no-window is canonical absence. Recorded durations,
query results and workload-report data remain observational. Both GPU paths pass
`trace_captures_timing_window_replay_policy`, including unchanged other policy
fields, full-state capture of the advancing counter, and removal on finish.
Older active-window v19 captures remain unqualified for this omission. The Rain
C sample path has no timing-window bridge/call, so its ongoing frozen captures
retain applicability; no binaries were replaced. Evidence:
`timing-window-state-audit.json`, `timing-policy-{ordinary,native}.log`.
This resolves one inventory entry, not the whole persistent-state audit.


Two additional GpuSim inventory fields now have explicit consumer classification.
step_force_slots is captured as live-body clear membership plus device loads;
set_step_forces clears all old slots before writing new loads, making the order
and repeated zero writes incidental. shape_identities supplies actual captured
shape storage order, with live-generation validation and append-only public
shape slots. A new generation-only corruption control rejects capture without
appending; restoration followed by uncaptured force-member corruption also
rejects. Both paths pass the extended existing test. Evidence:
`state-identity-membership-audit.json`, `state-identity-membership-{ordinary,native}.log`.
The rest of the persistence/reset audit remains open.


First buffered Rain sample runs reach600steps on both paths. Ordinary sample
exit0; runner finalization remains live. Native launcher exits1 after the sample
reports600frames/0renderer errors: xvfb-run fails its cleanup kill before returning
its saved child status. Preserve this failed attempt and do not infer exact child
exit0. Native health parses as a complete600frame/statusok report; it is diagnostic
evidence, not a qualified repeat. Runner needs separate child/launcher receipts
before replacement fresh native attempts.

Native later windows now have verified lifetime mapping and exact q/v/w bits for
all42cell bodies. Persistent twist(core375–464) maps Human2379→core2479; severe
cone(core515–526) maps Human5004→core5104 across recycled public slots. The last
awake twist error0.05448156343rad matches cached-impulse/stiffness estimate
0.05448071309rad within8.5034e-7, and remains when sleeping begins at427. This
supports loaded compliance but is not a full contact-torque balance or acceptance
pass. Cone still peaks1.26105142rad and anchorerror0.0698773041m, recovering to
0.00566038489rad at health525. Evidence:
`rain-later-full-state/{native-episode-summary.json,native-twist-compliance.json,native-health-validation.json}`.

## 2026-09-28 portable checkpoint

Desktop continuation at checkpoint `2f96608`: the sample repeat runner now
records the sample's exact subprocess status inside the Xvfb session before
launcher cleanup. The launcher status remains separately recorded. Both must
be zero, and the child receipt must exist, before finalization; a cleanup failure
with successful sample completion remains a failed attempt with explicit cause.
No historical failed receipt is changed. Completion manifests include the child
receipt hash. `python3 scripts/test-sample-state-repeats.py` passes nine tests,
including bounded real subprocess cases for cleanup failure, sample failure,
missing child execution with zero/nonzero launcher exits, failed-attempt resume
preservation, and child-receipt integrity. These tests require no GPU/display and
do not count as solver qualification. Historical raw artifacts are unavailable
on the new desktop; new hardware evidence must be generated independently.

Local baseline is now recorded under `desktop-baseline/`: both diagnostic
builds succeed, with28/28 numerical precision tests and the body-growth filter
regression passing independently. Selected adapter evidence confirms RTX4070
SUPER/Vulkan/NVIDIA610.57.04. One complete contact-island and restitution run
per path reproduce the unchanged failed screens, with exit101 preserved.
Spin/transverse drift are0.019269153rad/s and0.001521538m; restitution penetration
is0.049310952m. All73 contact-island frames are retained; measured momentum error
is5.971459e-8kg·m/s, total angular momentum1.364062e-6kg·m²/s and peak energy
0.167829651J. New independent CPU runs reproduce all292 contact-island body rows
exactly after Y reflection; the720 restitution rows differ by at most9.536743e-7.
`baseline-summary.json`, `cpu-comparison.json`, per-process receipts and frozen
binary manifests retain the evidence. These are single-run baseline diagnostics,
not accepted failed screens or five-repeat/full-state qualification. The CPU
fixtures use the complete original CPU library, not the portable GPU fallback
archive (an initial link against that incomplete archive failed before running).

The next consumer audit closes one additional capture omission: `contact_metrics`
was absent even though its step controls `finish_contact_status` refresh and its
counts/revisions are public API output. `gpu_policy.contact_metrics` now retains
all nine fields (or null before a snapshot exists). Both paths pass a focused
test that compares the export with the public snapshot, mutates only a cached
candidate count, observes the public change, and requires the capture to retain
exactly that change. Initial native current=1 expectation was removed because
the API explicitly permits stale revisions; the test retains the actual
revision and freshness inputs rather than asserting a scheduling assumption.
This is diagnostic-only code. Earlier captures lack this state and cannot close
its coverage gate; fresh candidates must include it. Full persistence/reset and
semantic-storage audits remain open. Evidence: `desktop-baseline/contact-metrics-
{ordinary,native}-final.log`; initial compile/test failure logs are preserved.
Both broader `trace_captures_` selections also pass5tests each. All32Python
helper tests pass (9runner,15identity,3storage,5spherical health). A local,
signature-verified Xvfb package runs without sudo; the actual launcher completes
five synthetic fixed-frame processes with zero child and launcher statuses and
matching full traces (`desktop-baseline/runner-xvfb-control`). This qualifies
the bounded runner integration, not five fresh physics runs.

User requested commit/push and a new-machine handoff. The active goal remains
incomplete. Process inspection found no surviving qualification jobs. Ordinary
Rain run1 has a completion receipt; run2 is partial. Native run1 retains the
launcher cleanup failure. Both later ordinary episode extractions finalized.
The relevant historical receipt and compact analysis summaries are preserved in
`experiments/gpu-physics/benchmarks/2026-09-28-solver-checkpoint.json`; raw traces
remain local and unavailable through Git. These summaries are historical
evidence, not new-machine qualification. Rain episode helper sources are now
tracked under `scripts/rain-episode-diagnostics/` in the engine directory.

At checkpoint preparation, all28Python helper tests passed (15identity,5runner,
3storage,5spherical health). This does not supersede the known failing physical
screens or complete the persistent-state audit. See the goal scratchpad for
portable recovery and the next discriminating work.

Pre-commit recording finished: `record-snapshot.sh` produced all19core GPU
clips (300frames each, GPU-native ordering) and its5-repeat metric pass;
`record-box3d-oracle.sh` refreshed the real C CPU comparison column. The rebuilt
paired native viewer also captured Shape Replacement300, Offset Kinematic300,
Falling Ragdolls600 and Rain120steps, all with0Sokol errors. CPU is left and GPU
right. All38core clips were checked for300video frames; paired cropped clips
were checked for nonempty960x1080video. The Offset paired image was visually
checked for correct scene and pane placement. Software OpenGL rendered the
paired clips while Vulkan ran GPU physics. These are bounded visual evidence,
not controlled solver performance or final physical/repeatability passes.
Clip hashes, exact binary identities, reports and metrics are preserved in
the tracked checkpoint JSON; videos themselves remain ignored/local.


### Desktop CCD candidate: ragdoll repeats and lifetime checks

`desktop-baseline/ragdoll-ccd-five/physical-summary.json` records all ten
600step physical passes (five fresh processes per path) with the unchanged
CPU-relative limits. All children exit0; the aggregate runners exit1 because
raw state comparisons fail. The standalone slot-record permutation audits
also fail at frame139: ordinary runs2/4 and native run5 assign generations6/1
to roots [[48,62],0]/[[50,55],0] oppositely from their respective run1. Free
generation histograms match at that frame. No comparator was relaxed.

`compare_groups.py` compares each top-level group exactly through every600frame
trace, requiring sequential frame indices and no trailing content. The prior
slot audits validate every original trace with the existing schema checker.
Per-path `group-differences.json` shows differences only in contact_allocation
and event_history; all other captured groups match across five processes.
Allocation differs in520–542ordinary and537–575native frames per comparison;
event history differs in2–5frames. This localizes the mismatch; it does not
close complete semantic-state repeatability or establish future equivalence.

On the current candidate, all six contact_api::determinism_tests pass on each
path, including unread root reuse, retired-generation perturbation and event
ordering. Logs `contact-lifetime-{ordinary,native}-selected.log` retain the
actual six-test counts and process exits0. The earlier wrong-selector logs
ran zero tests and are excluded. Source consumers compare root slot/generation
identity in public registry continuity and solver history validity; mere numeric
renaming needs those relationships preserved. Next audit must address these
root lifetime relationships, not simply sort slot records or discard generations.


### Confirmed mesh scratch contact lifetime defect

The desktop lifetime audit finds six reused same-pair/slot/generation tokens
after retirement in every600step ragdoll run, despite matching lifetime episode
patterns. `ragdoll-ccd-five/{ordinary,native}/lifetime-relations.json` retains
all1544episodes/136334root observations per run; first-reused-owner-window.json
shows the earliest token returning at166 after its slot counter was cleared.

A new `unread_root_handle_retires_after_mesh_scratch_reuse` regression reproduces
public-handle aliasing on both paths without counter injection: create/read a
box contact, move it away without harvesting contacts, stage a separate three-plane
mesh collision through the freed slot, then reunite the original pair. The slot's
generation changes from1 to0 during scratch use; re-entry produces1 and the old
ContactId incorrectly survives. Both pre-fix tests exit101 on the stale-handle
assertion. An initial child-storage assumption failed earlier and is preserved
as a fixture-development failure, not substituted for the actual reproduction.

The candidate fix adds a mesh-only store helper preserving the physical slot's
root generation through temporary writes and discards. Published children also
retain that slot counter instead of copying the parent's. Existing root birth
increment and lifetime consumers remain unchanged. Every shader generation
consumer uses roots; child slots must preserve their prior root history for
future reuse. The diagnostic permutation validator correspondingly removes its
obsolete child-counter-equals-parent assumption, while retaining counters exactly.
Controls verify counter mutations still compare unequal and negative counters
reject (`mesh-generation-fix/slot-counter-controls.json`). This is a production
lifetime fix under verification, not permission to omit allocation state.

The candidate passes all seven contact regressions on both paths, including
the formerly failing public-handle test. Actual scratch readback preserves
generation1, then the replacement contact gets a distinct handle and the old
handle remains invalid. Both suites exit0; `mesh-generation-fix/manifest.json`
records before/after binary and shader hashes, test count and exits. Five-process
contact/joint-history checks remain running. Two repeat processes were briefly
suspended during memory-intensive shader compilation and resumed unchanged
after the contact suites finished; no attempt was replaced or counted complete.

Both follow-ups now pass: five fresh unread scratch-reuse tests per path
(`mesh-generation-fix/lifetime-five-{ordinary,native}`) and five fresh16step
child/joint-history traces per path (`mesh-generation-history-five`). All five
raw trace hashes match within each path. The unread tests deliberately avoid
intermediate contact harvesting, so they establish public lifetime behavior,
not per-step full-state equality. The16step fixture supplies the separate
contact/motor-history capture comparison. Full-scene applicability, the complete
persistence audit and remaining physical gates are still open. All these test
processes are terminal with successful receipts; no suspended child remains.


The rebuilt independent Falling Ragdolls fixtures complete600steps on each path
after the generation fix. Both unchanged physical checkers pass; poses, velocities
and joint separation outputs are byte-identical to their CCD-only predecessors.
The same full-trace lifetime checker rejects the pre-fix controls with six reused
root tokens and377slot counter decreases on each path, then passes the post-fix
captures with zero of either. Every frame also passes captured-status checks.
Evidence: `desktop-baseline/mesh-generation-fix/{before,after}-scene-lifetimes-
{ordinary,native}.json`, `full-scene-physical.json`; source/binary/library hashes
are in `ragdoll-generation-candidate`. These are one fresh full-scene run per
path, not five-run qualification. Five-run batches are now in progress under
`ragdoll-generation-five`, using these exact frozen executables.

All five post-generation-fix600step ragdoll processes now finish successfully
on each path, and every physical screen passes (`ragdoll-generation-five/
physical-summary.json`). Aggregate runners retain exit1 for raw frame1
occupied_order differences; no raw-repeatability pass is claimed. The existing
slot-permutation diagnostic is evaluating all five traces per path and keeps
every generation counter, physical group and public-state field. Its outcome
remains separate from complete-state coverage and semantic qualification.


The post-fix native slot-record permutation diagnostic passes all five600step
traces. Ordinary fails at141 on child [[32,89],1], which occupies slot158 with
counter1 versus156 with counter2. At that frame the entire physical-slot counter
vector and all root owners match, as do every other audited group. A child no
longer owns or inherits that counter after the lifetime correction, so sorting
combined child/counter records is not the right equivalence for this case.

The separate fixed-counter diagnostic retains all physical counters in their
original index order and every root slot exactly, and compares child placement
as membership. It changes no qualification comparator. On real frame141 data,
controls permit a child move between unequal-counter slots while rejecting any
counter change, root move, public-state change or missing child. Source and
controls are under ragdoll-generation-five; five-run/full600step checks are running.

The test-only relocation helper was aligned with production counter preservation.
Actual16step relocation probes pass on both paths and compare with frozen
unperturbed controls: only child positions at2–3 and previous-touching storage
at4 differ. All counters, root owners, physical/public state and motor/contact
history remain equal (`mesh-generation-fix/relocation-counter-comparison.json`).
This is bounded causal evidence for child placement, not complete allocator or
persistence qualification. Both probes/builds are terminal successful.

The fixed-counter diagnostic completes all600steps across all five runs on each
path with equivalent=true. Entire per-physical-slot counter vectors and root
owner positions remain exact, with only child placement and the previously
audited membership arrays normalized. Results: ragdoll-generation-five/
{ordinary,native}/fixed-counter-audit.json. This resolves the observed post-fix
storage discrepancy under that bounded view; complete persistent-state coverage
and the final semantic comparison gate remain open. No raw failure is erased.

A CPU-only native sample build is being established on the desktop because the
local reference inventory contained ragdoll CPU poses but no full CPU Rain health.
CMake uses GPU_SAMPLES=OFF/BOTH_SAMPLES=OFF and the existing disconnected dependency
cache. The planned capture uses the same original Benchmark/Rain scene,0warmup,
600timed steps and spherical health instrumentation, with separate child/launcher
receipts and source/binary provenance. It is a physical reference, not a GPU
repeatability run. Configure/build logs: desktop-baseline/rain-cpu-*.log.


### Desktop Rain baseline completion and current-candidate check

The independent original CPU600step Rain run now completes with child/launcher0,
all600sequential health frames and spherical measurements validated. The frozen
pre-CCD/pre-generation GPU baselines also complete all600steps with exact child
and launcher0; trace finalization/compression remains in progress. Health scans
confirm8400body creations and1948800joint observations per report, with no NaN,
exploded or capacity-loss frames in the CPU/ordinary reports. These are diagnostic
baselines and the CPU physical reference, not final-candidate repeat qualification.

Ordinary creation-matched comparison validates4200reused slots and all joint
identities, but the original +.05m/+.05rad screens fail:557anchor,3004angular,
5150cone,1086lower-twist and371upper-twist observations. Peak cone is1.26105142rad
at health516 for endpoint5004. The longest absolute >.05rad twist episode spans
369–463 (95frames), endpoint2379: peak.104365408rad, last.0544822812rad,
38frames with both endpoints asleep, last awake425. Source hashes and episode
records are under desktop-baseline/rain-full; neither CPU agreement nor successful
run completion clears these physical failures.

The existing Rain health selector accepts explicit source/output/window arguments;
new sources require explicit windows to prevent accidental reuse of historical
identities. It validates the full source before publishing excerpts. The actual
ordinary report selects twist368–463/base2353/target2379 and cone514–530/base4999/
target5004. State extraction uses the existing public-slot/generation and float32
physical correspondence checks. Two single current-candidate health-only runs
are also underway under rain-candidate-health to determine whether the CCD and
mesh-lifetime corrections change these residuals before analyzing older load
histories further. No new full-state qualification batch or performance claim.


Native baseline comparison now also finishes with the exact same identity/screen
report, peaks and absolute episodes as ordinary. All600steps map correctly; the
residual screen remains failed on both paths. Both ordinary state extractions
complete with verified health/core correspondence. Selector controls accept a
complete synthetic report and reject a missing cell body, truncation, missing
explicit windows and a broken frame sequence (rain-episodes/selector-controls.json).

Local twist reconstruction reproduces the earlier correlation: core426's measured
lower-limit violation.0544815634rad versus.0544807131rad from cached impulse and
soft stiffness. Both endpoints sleep at427. This is still a cached-final-impulse
estimate, not independent full-substep torque balance or physical acceptance.
Extending the selected severe-cone health window to599 finds recurrence after
initial recovery: .005660rad at525 rises to.159410rad at531, before reaching
.003718rad at599. Thus the early recovery must not be described as sustained.
Detailed rows: rain-episodes/ordinary-cone-followthrough.json; excerpt and source
hashes, lifetime mappings and compliance rows remain alongside it. Candidate
health checks continue with confirmed live processes; no new acceptance is claimed.


### Desktop loaded-twist independent static balance

The selected last-awake twist episode now has a second reconstruction using all
incident contact and joint cached impulses on both endpoints, excluding the
target twist-limit cache from the inferred load. It includes normal/friction,
rolling/twist contact terms, point-joint lever torques, other spherical/revolute
constraints, springs/motors and gravity. Projection through the prepared endpoint
inverse inertias and twist Jacobian gives the balancing target impulse.

At core426 it predicts0.4860739147N·m·s versus cached0.4860729575N·m·s,
relative difference1.97e-6. Its corresponding static soft-limit error is
.0544808204rad versus measured.0544815634rad. Across awake410–426, maximum
relative inferred/cache impulse difference is7.36e-5. This supports the loaded
soft-equilibrium explanation independently of substituting the target's own
cache into its softness equation. It uses final cached substep impulses and
end-rotation lever approximations; it is not a full dynamic substep ledger or
blanket Rain acceptance. Original screens remain failed.

Controls zero the target twist cache without changing the inferred load, and
remove self-contact[[2569,2579],0], changing the required impulse to.4165008126.
The reconstruction follows solve.wgsl's actual prepared twist Jacobian rather
than the public reaction-torque getter's different axis. Source, per-load vectors,
residual body impulses and controls are in desktop-baseline/rain-episodes/
{reconstruct_load.py,ordinary-twist-load-balance.json,load-balance-controls.json}.

The graph workspace source audit also now covers static compaction/sorting and
canonical dynamic graph construction for the selected no-rebuild configurations.
All relevant counts, masks, degree metadata, prefixes and bounded list outputs
are produced before consumption, with ordinary/native command order checked.
Memo persistent records remain captured; unrelated scratch and child-storage
semantics stay open. This changes no solver code or comparison acceptance.


### Contact-retirement command/status alias

The query-workspace audit found an actual state corruption: retire_contacts wrote
[a,b,mode] at query64–66, while query66 also held sticky contact-drop reasons.
Normal shape retirement replaced clean status with reason bit0; both shape and
body-pair retirement overwrote existing reason bits before they were harvested.
The ordinary before-fix regression reproduces exactly[(shape,0,1),(pair,0,0),
(shape,512,1),(pair,512,0)], exit101. It separately verifies that shape retirement
removes only the selected root and body-pair retirement then removes the other.
Thus this failure is status corruption, not a failure of its fixture selection.

The candidate moves the three command words to32–34, outside ray state0–31,
sticky status66 and counters67onward. Both host and shader use named matching
constants; word66 remains exclusively the contact-failure reason accumulator.
Before native and after-fix verification are in progress. Frozen before binaries,
source hashes and logs are in desktop-baseline/retirement-status-fix. Current
Rain health captures predate this correction and remain diagnostic baselines;
no ongoing process or receipt was replaced.


Native before-fix regression also finishes101 with the same four corruptions.
Both corrected binaries build successfully; the five retirement-related checks
are running per path. These include state capture through slot reuse, sticky
reason preservation, full-width hash retirement/reuse, child ownership and
occupied-list publication. Baseline Rain finalizers now also finish0: both
complete.json receipts verify all600frames, original/compressed trace hashes,
health and child/launcher receipts. Raw traces were replaced by verified gzip.
These baseline binaries predate the current retirement/status correction.


The correction now passes all five related retirement tests on both paths,
ordinary104.64s/native101.31s. The before-fix failures and binary/source hashes
remain preserved; manifest.json records exact exits and corrected binary hashes.
This is focused regression validation, not five fresh full-state qualification.
Production sample/library builds still predate this correction.

The same-step status follow-up reproduced another actual failure on both paths.
The regression seeds a broken contact chain, submits real body-pair retirement
and first verifies failure directly on the GPU. Completed wait incorrectly reused
clean metrics from the same physics step. Both frozen before executables exit101.

The correction marks status dirty on retirement, shape remapping and contact-hash
republication. A fresh status copy/map clears the flag; draining an older pending
slot does not. Explicit completion refreshes dirty status regardless of cached
metrics step. Diagnostic capture rejects pending or dirty status, and GPU policy
includes the flag. Both after builds finish0; six retirement tests plus the
undrained capture and metrics identity checks pass on each path. The new regression
covers both an empty status slot and an older pending clean copy. Evidence, source
and binary hashes, and exact exits are in desktop-baseline/same-step-status-fix.
These eight focused checks per path are not five-process qualification. Production
libraries/sample executables and running Rain fixtures predate both status fixes.
The query audit records this contract; explicit clear/reset is covered by the follow-up below, while simulator recreation
transitions still require review before whole-state coverage is claimed.


### Clearing unread contact failure (desktop continuation, 2026-09-29)

The status-transition audit found that clearing counters could hide a failure
before its first host harvest. The new regression submits real broken-chain
retirement, calls clear with no intervening status readback, then requires
terminal invalidity while sticky diagnostics stay cleared. Both frozen before
executables fail with physics incorrectly valid (exit101). It tests both an
empty status slot and an older pending clean copy.

The candidate drains completed status before clearing host/device sticky fields,
marks the clear dirty for a subsequent status refresh, and immediately propagates
simulator invalidity to the world. Existing capacity-loss retention and stopped-
submission tests pass on both paths (two tests each). First after suites reached
the terminal-invalid and cleared-host assertions, then failed an overbroad test
assertion requiring last-step counters to clear too. The API only clears sticky
counters; the corrected test checks sticky loss, first-failure step and reason
bits. Those original after logs/binaries remain preserved. Corrected suites now pass on both paths: seven retirement checks (ordinary
105.72s/native109.26s) and one capture-boundary check, in addition to the two
capacity-loss checks with identical production code. Evidence and hashes live in
`desktop-baseline/clear-status-fix`. This is focused correctness evidence, not
final candidate qualification. Production samples and live Rain captures still
predate these status corrections.


### Failure reasons across simulator growth (desktop continuation, 2026-09-29)

A concrete persistence omission remained in allocation transfer: atomic sticky
failure counters and their first-step word were copied, but query word66 containing
contact failure reasons was not. The new regression emits a real GPU contact drop
without harvesting host status, then grows the simulator. Both before binaries
exit101: the counters survive while reason0x40 becomes0 at body count257.

The correction copies word66 with the other contact state and records the transfer
as a contact mutation requiring fresh status. The regression passes for body
counts257 and8193, covering unchanged and increased pair capacity; it verifies
device/host reason bits, counters, first-failure step and terminal invalidity.
Both paths pass eleven focused checks: the new regression, existing contact-growth
check, seven retirement checks and two capacity-loss checks. Evidence, exact exits,
and before/after source/binary hashes: desktop-baseline/growth-status-fix.
Component-query-only growth already copies the entire old query buffer. These
results close this transfer omission, not full persistent-state qualification.
Production sample/library binaries and live Rain fixtures predate all status fixes.


### Large-static list overflow (desktop continuation, 2026-09-29)

The pair-workspace audit found that collect_fat_statics incremented its count past
pair capacity but silently discarded excess descriptors. These large statics bypass
the spatial hash, so omission could lose candidate collisions without a failure.
The regression seeds the already-filled prefix count and dispatches one real large
static proxy. It verifies the last valid slot is accepted, a guard beyond the list
is unchanged, and the next insertion records sticky loss. Both before executables
fail101 at prior65536 with capacity_loss=false. This is an exact boundary fixture,
not a full many-static scene stress run. An initial compile error referred to the
WGSL-only SCR_STATIC_N constant; corrected builds and original logs are retained.

The candidate records the discarded descriptor as broadphase insertion loss using
the existing per-step/sticky counters. Both paths pass the boundary test,80step
matrix-versus-grid mutation test and two existing capacity-loss tests. The mutation
check preserves exact candidate pairs/contact schedules and p/q/v/w within1e-5.
Evidence, before/after hashes and exact exits: desktop-baseline/static-list-overflow-fix.
Release libraries and sample executables now rebuild successfully with this and
the status corrections (drivers17836/74740 exit0). Source patch, library/sample
and build-log hashes are in static-list-overflow-fix/production-builds.json. Two-step
Rain sample smoke checks now finish0 on both paths with child/launcher0, two valid
health frames and540spherical observations each, no NaN/exploded frames and empty
GPU failure. Frozen fixture hashes match rebuilt samples; evidence is in
desktop-baseline/status-overflow-smoke/summary.json. Live600step Rain fixtures
retain their original frozen builds. These focused checks do not establish final
full-state or physical qualification.


The desktop continuation regenerates the unavailable historical GpuSim inventory
from current source:210named fields, including101pipeline declarations. The
source-hashed artifact is desktop-baseline/gpu-sim-field-inventory.json. Its routing
categories deliberately distinguish exporter references (54fields) from semantic
coverage proof, and twelve device-data owners still require final consolidation
of existing field/region contracts. Pose/readback/output contracts narrow the
remaining review; they do not establish whole-state qualification.


### Device body-center capture omission (desktop continuation, 2026-09-29)

The parameter audit verifies all63meaningful SimParams fields against Rust and
WGSL declaration order, unique export keys and finite bit-preserving float/exact
integer encoding. The64thRust word is zero-initialized padding with no shader
field. Evidence: desktop-baseline/sim-parameters-state-audit.json.

BodyColdGpu.local_center is consumed by collision geometry/origin calculations,
but decode_body_gpu drops it. The trace previously recorded only host.local_center
without validating device bytes. A device-only finite mutation is invisible to
capture on both paths: the new regression fails101 with capture incorrectly
successful. Before builds66561/47201 finish0; native test13809 finishes101.
Frozen sources/binaries/logs are in desktop-baseline/body-center-capture-fix.
The candidate reads device centers with the existing extras readback and rejects
any bit mismatch with captured host values before appending a frame. Its test
also covers NaN, unchanged output on rejection and restored-value success.
After builds42383/94491 finish0; drivers75179/48283 finish0. Four checks pass on
each path: finite/NaN corruption with unchanged-output rejection and restored-value
success,18step mass/center mutation, external force clear and body-slot reuse.
Manifest records binary/source/log hashes and exact exits. The live body field
mapping is recorded in desktop-baseline/body-record-state-audit.json; inactive
slots and island membership are outside this bounded audit. Existing traces
cannot establish this device/host invariant retroactively. No physical solver
change, no replacement of frozen live Rain fixtures and no full audit claim.


### Inactive contact identity lanes (desktop continuation, 2026-09-29)

The contact record audit found that match_previous_points checks every lane of
point_triangles and all four feature IDs to select mesh/feature matching mode.
pack_point_features also checks all feature lanes before generating IDs. Trace
v19 saved active points only, so inactive identities were missing future inputs.
Schema v20 now records feature_id_words and point_triangle_words as full four-word
arrays in addition to active point fields. Comparator checks exact u32 values,
array sizes and active-point consistency. Legacy v19 remains readable but has
weaker coverage and does not compare equal to v20. Health/repeat/episode readers
accept both; new qualification requires a newly frozen v20 candidate.

Both builds72698/54574 finish0. Actual device-only mutations change feature lane2
and triangle lane3 on a one-point sphere contact; both GPU paths capture the
changes while active points stay exact. The old contact projection is unchanged,
but the host test helper also increments idle epoch, so this is not an assertion
that every old group stays equal. Both3frame contact retirement/reuse captures
also pass and validate as v20. All42Python helper tests pass, including missing,
malformed, inactive-change, active-consistency and legacy-schema controls. Initial
unittest discovery found no hyphenated modules and exited5; explicit script runs
are the evidence. Artifacts, hashes and exact exits:
desktop-baseline/contact-identity-capture-fix/manifest.json. Solver arithmetic is
unchanged. Release samples remain pre-v20; full persistent-state coverage is open.


### Completed CCD/generation Rain health comparison (desktop, 2026-09-29)

Both frozen health-only600step captures complete0 (7043/59391), with child and
launcher0: ordinary5326.86s/native5321.51s. Each validates600frames and1252800
spherical observations. Full scans count8400body creations,1948800joint
observations and noNaN/exploded/capacity-loss frames. These fixtures predate the
later status/overflow/capture fixes and are single runs, not final qualification
or performance measurements.

CPU-relative screens32306/98170 finish1 with identical reports:501anchor,
2444angular,5158cone,966lower-twist and327upper-twist observations exceed the
original budgets. Identity/recycling passes. Absolute scans92515/28460 finish0.
The baseline95frame sleeping-twist episode ending at creation2379 does not
appear among candidate episodes lasting at least5frames; none of the retained
candidate episodes includes sleep. The longest is62awakeframes355–416 at
endpoint1693, peak0.0555674881rad. This does not assert that every shorter
sleeping violation is absent, nor that the original residual screen passes.

The severe cone peak is unchanged:1.26105142rad at health516,endpoint5004. The
515–522episode matches the baseline report; recurrence529–534 now peaks
0.15914005rad rather than0.159410328. Severe-cone physical acceptance remains
the priority. Prior loaded sleeping-twist reconstruction is historical and must
not be assumed to explain the changed candidate trajectory. Evidence:
desktop-baseline/rain-candidate-health/comparison-summary.json and each mode's
validation, CPU screen and residual episode reports. All these jobs are terminal.


### Severe-cone geometry discriminator (desktop, 2026-09-29)

The current ordinary health selector87465 completes0 with whole-document
validation. Every selected42body position, quaternion, velocity, angular velocity
and awake flag is decimal-output identical to baseline over health514–525; the
target joint health also matches. Both diverge at526. This bounds reuse of the
baseline excerpt for the initial transient, without assuming hidden contact
history equality. Evidence: rain-candidate-health/cone-health-equivalence.json.

A float64 reconstruction from the existing captured float32 rotations measures
72.4946degrees between the step-start and end cone axes at onset515. Endpoint
angular speeds reach105.254/41.095rad/s. Final outward cone-rate projections are
38.286rad/s on the frozen axis and129.342 on the actual end axis. At520 the axes
have dot product-.33146: frozen projection-59.248rad/s suggests closure while
end-axis projection+21.618 suggests opening. Cached swing impulse is0 there.
GPU and upstream CPU both prepare axis/mass from step-start orientation while
using updated orientations for the angle. This is a concrete shared fixed-axis
approximation hypothesis; it is not yet a causal demonstration or acceptance.

Two central-difference controls independently validate rate/sign calculations
within7e-11. Reconstructed angle differs from the health implementation by at
most about5.24e-5rad; calculations are diagnostic, not exact shader replay.
Source hashes and all17rows: rain-episodes/ordinary-cone-geometry.json, generated
by analyze_cone.py. Final velocities/cached impulses do not recover all substep
work. Next discriminating experiment is a bounded isolated replay with a
consistent updated-axis/mass treatment, preserving baseline/defaults and checking
anchor/twist/energy as well as cone error. No solver change made in this follow-up.


### Bounded cone-axis experiment (desktop, 2026-09-29)

Frozen120step replays use the42body cell state from health514 (base4999,row9,
column1), original defaults and fresh contact/joint history. The candidate
recomputes the cone gradient and effective mass from current delta-rotated joint
frames, retaining the prepared inertia tensor and scalar impulse accumulation.
It is an approximate relinearization experiment, not a full nonlinear solve.

Corrected CPU, ordinary/native baselines and ordinary/native candidates all exit0.
Each validates exact initial float32 getter agreement,5040finite body records and
3240spherical observations. Both GPU modes give identical metrics. Baseline
cone/lower-twist peaks1.26054549/.356199682rad become.983743727/.0900032818rad,
but target anchor error increases from.0697196573 to.0835709944m and upper twist
from.00343292952 to.00924003124rad. Maximum mechanical energy is23318.405J
baseline vs23022.228J candidate, below common initial24744.718J. These energy
numbers omit motor work/elastic storage and use captured immutable mass/inertia;
they are not a complete energy-acceptance argument. The severe violation remains,
so the candidate is not retained. No threshold/default is changed. The outcome
supports a contribution from frozen-axis linearization, not a complete causal
explanation or full Rain acceptance. Do not repeat this same axis refresh.

Initial GPU fixtures reached the end of the physics loop but all crashed in
cleanup (signal11); stdout retained a partial buffered tail. Gdb on the preserved
baseline core shows DestroyHuman calling CPU b3DestroyJointInternal with GPU
handles because this ad-hoc fixture omitted libgpu_samples_api.a. Corrected
whole-archive GPU API linkage fixes all reruns. Initial CPU linkage separately
needed DNDEBUG to match its Release archive. Failed outputs remain intact and
are excluded from successful evidence. Corrected candidate/baseline links and
runs74090/4438/44387/9293/86809/89211 finish0. The temporary pause of initial
candidate children for compilation memory was undone; all jobs are terminal.

Original solve.wgsl is restored byte-for-byte and ordinary/native release
libraries rebuilt successfully (48326/78456) from restored source, now including
v20 capture/body-center validation. Sample executables are unchanged. Frozen
sources, executable/library hashes, commands, configurations, failures, complete
summary and restoration proof: desktop-baseline/cone-axis-experiment. No commit.

## Child placement affects future allocation (desktop continuation)

The new direct-dispatch `child_placement_changes_next_root_free_slot` passes on
both configured backend builds. With identical physical counters10–14, root slot0,
logical root/child chain and high-water5, placing the child at1 yields next free
root slot2; placing it at4 yields slot1. `alloc_prepare_keys` and `alloc_bind_slots`
consume that same free-list prefix. This is a counterexample to treating arbitrary
child placement as irrelevant merely because roots/counters/membership match.
It is an allocator fixture, not a full-step physical scene test. Both initial
checks failed because `patch` is a reserved WGSL name; those logs remain preserved.
Renaming it to `piece` and rebuilding produced one passing test per path.
Evidence: desktop-baseline/contact-storage-audit/placement-result.json.

The separate historical600step occupancy scan is recorded in
desktop-baseline/contact-storage-audit/occupancy-result.json. Ordinary runs first
differ at frame137/run2, slots120/122; native first differs at139/run4, slots154/156.
Both scans verify all600frames of all five runs and hash the complete inputs
(driver7031 exits0). Thus the fixed-counter diagnostic omitted
an input that actually differs in the captured batch. Its previous equivalent=true
result remains historical diagnostic evidence; it is not eligible for promotion
to full semantic qualification. Full raw qualification was never changed.

## Isolated GPU child-compaction candidate

A test-only candidate (`src/contact_compaction.rs`,
`shaders/contact_compaction.wgsl`) stages complete contact records, scans root
child counts and non-root destinations, maps root/chain order, then publishes
children without moving roots or physical generation counters. No production
stepping path calls it. Predeclared gates and hashes are in
desktop-baseline/child-compaction-candidate/{acceptance.md,result.json}.

Both configured backends pass one test containing spans8/513/65537, two
placements and malformed ownership at each span, with two applications per case.
Expected full hot/persistent/prepared words match, counter vectors remain fixed,
outputs are idempotent, and malformed ownership leaves all contact buffers
unchanged while setting the chain-reason bit. The largest case has257groups,
covering multiple group summaries per scan lane. This is an isolated transform
check, not fresh-process scene qualification or native cached execution.

Before production integration the error path must also update atom per-step/
sticky contact loss and first-step to match record_contact_drop; query reason
alone does not set the host physics-invalid state. Empty/high-water/cyclic/orphan
inputs, lifecycle integration, complete ragdoll repeats and overhead measurement
remain open. Mesh-free paths should not execute or allocate this pass.

## Integrated child-compaction status and lifecycle checks

The candidate is now called after graph cleanup/retirement and before islands
and constraint preparation, including the native graph-cache exit and callback
completion. This placement keeps old graph cleanup ahead of child relocation.
Mesh-free scenes allocate and execute no compaction pass. Temporary buffer/reset
contracts and memory cost are recorded in
`desktop-baseline/child-compaction-candidate/integrated-source-audit.json`;
GpuSim inventory now has211fields, including the transient compaction owner.

Failure now records per-step/sticky contact loss, query reason and the first
failing step using current GPU parameters. Both backends pass empty, oversized
high-water, live-above-high, orphan, cyclic and truncated chain controls, with
unchanged input buffers and verified terminal host invalidity. The full-payload
and prefix-boundary checks also remain green.

Integration driver27743 finishes0:11checks per path (two isolation tests, seven
contact lifetime/order tests, one16step mesh/joint trace and one convex callback
order/reuse control). Both actual16step traces have canonical child placement
in every frame. This is not mesh-specific callback coverage or full repeat
qualification. Release libraries/sample executables have not been rebuilt yet.

The initial ordinary integration fixture47874 fails101 because its old setup
required a retired root slot stay empty. Compaction correctly occupies it with
a child. The preserved replacement fixture requires non-root occupancy and the
exact original physical generation; its original unread-handle validity and
retirement checks remain intact. Old fixture and failed log are preserved under
child-compaction-candidate/. No physical threshold or scene default was changed.

The selected16step mesh/joint fixture additionally passes five fresh processes
per backend with exact raw v20 state equality (driver75841 exits0). Frozen test
binaries, configuration/source manifests, all child exits and traces are in
`desktop-baseline/child-compaction-history-five/`. This does not close the full
ragdoll or broader persistence gate.

Both release libraries and sample executables are rebuilt/relinked; hashes are
in child-compaction-candidate/release-manifest.json. Standalone ragdoll fixtures
are frozen under ragdoll-child-compaction-candidate/ including the new source
files omitted by ordinary git diff for untracked files. Five600step batches are
running under ragdoll-child-compaction-five/ (ordinary30729/native32797); no
full-scene comparison or physical pass is yet claimed. Preserve raw failures and
check exact slots/counters in any audited membership comparison.

## Full ragdoll child-compaction results

All ten fresh600step children finish0 and pass every original ragdoll physical
screen. Aggregate runners30729/32797 retain exit1 for raw occupied append order
at frame1/run2 (50 vs20 at occupied_order[124][0][0]). Separate audited comparators
71893/15489 finish0: five600step v20 runs per path match after only the existing
occupied-root/previous-touching membership normalization. Physical slots, physical
generation counters, solver order and all remaining captured fields stay exact.
This resolves the selected scene's child-placement repeat gap. It does not promote
the historical arbitrary-child-membership comparison or close incomplete schema
coverage. Reports, hashed physical inputs and preserved failures:
`desktop-baseline/ragdoll-child-compaction-five/`.

Existing mesh callback regression also passes on both current binaries (61900):
all patches vetoed with no events/motion, then two patches re-enabled as one public
pair with begin event, exact contact normals, and query-in-callback completion.
Evidence: `desktop-baseline/child-compaction-candidate/mesh-callback-result.json`.
This is one integration check per path, not a five-process callback qualification.

## Scene geometry and material capture audit

`desktop-baseline/scene-geometry-state-audit.json` maps all31ShapeGpu fields and
all9SurfaceMaterialGpu fields to capture or explicit non-physics consumers. Actual
device geometry is captured: xyz for points/directions/vertices, xyzw planes,
integer topology/triangles, bounds plus raw BVH metadata, and physical mixing-table
slots including occupied tags. Material slots resolve to ordered values; GPU code
only reads them and host scene updates replace the packed scene. Public shape
construction guarantees at least one material; mesh material count rejects zero.
Thus the shader's max(count,1) fallback exposes no omitted normal-path material.
Reserved ordering lanes, padding and visual color have no physics consumer.

Three existing tests pass on each current backend (driver32543 exit0): uploaded
mesh-child corruption, convex geometry/span validation, and body-slot reuse with
material callback mixing and device-only geometry mutation. Binary/log hashes and
exact test counts are in `desktop-baseline/scene-geometry-audit/result.json`.
No production or schema change was needed. Scope is completed public World API
capture; arbitrary malformed low-level GpuSim inputs and other policy/workspace
regions are not established by this audit.

## Cached command and idle policy audit

`desktop-baseline/command-policy-state-audit.json` records source hashes and
producer/consumer contracts for18GpuSim fields: radix/contact/graph/tail/full-step
command caches and their toggles, plus idle proof/chain/context/epoch and logical
step state. Cache keys and current-binding validity are captured. Cached Vulkan
commands retain every buffer/pipeline owner and read current buffer data; they do
not carry a separate uncaptured physics payload. Direct dispatch selection follows
the recorded keys and fixed build/device policy; indirect contents have separate
producer/reset contracts. Idle proof uses exact captured epochs/contexts and
completed status, while capture rejects outstanding or dirty status.

Driver23306 finishes0. All five existing idle regressions pass per backend:
unchanged idle state/submissions, wake/contact history, delayed proof with mutations,
growth, and mutation recovery. Native owner/queue lifetime and reset dispatch
span2/130/65/2 checks also pass. Five fresh current-native48step full-replay runs
compare raw-identically; each has42actual replay hits with substep and transform
reentry. Frozen binary/source/input hashes and receipts are in
`desktop-baseline/command-policy-audit/`. The inventory now leaves36reference-only
policy entries and12device-region owners for consolidation. No production/schema
edit; remaining shared-state and physical acceptance gates stay open.

## Host policy consolidation and component workspace

`desktop-baseline/host-policy-state-audit.json` maps the remaining36host policy
fields to exact capture/derived-allocation contracts and source consumers.
GpuSim inventory now has no reference-only host entries. This classification does
not close the12device-region owners or the full persistence gate. Graph memo
allocation presence is distinguished by null versus an invalid-layout object;
its immutable offset derives from the actual capacity encoded by captured
shape_base_u32 and contact slots.

`desktop-baseline/component-workspace-state-audit.json` records initialization
and count-bounded consumption of island label/wake/ready lanes and component
metadata, starts, lists and indirect/invalid header. Each active step regenerates
these values before use on ordinary, cached-tail and full-replay paths. Persistent
graph memo storage sits beyond the capacity-sized component region and remains
captured separately. This does not classify joint schedules/history, fat bounds,
remapping or previous-touching storage as transient.

Driver95338 exits0; three existing tests pass per path. The90step merge/split
comparison exercises both sleep settings, actual large-to-small transitions and
return to sleep. The two120step overflow comparisons exercise actual nonempty
small/large overflow and retain1e-5 physical comparisons on every observed step.
Logs and hashes: `desktop-baseline/component-workspace-audit/result.json`.
No new production code, schema or acceptance-limit change. These are source and
bounded regression checks, not five-process qualification of all component states.

## Persistent history lookup capture correction

`ordered_contact_transitions` consumes the independently persistent physical
slot-to-logical-rank lookup before allocating IDs and assigning colors. Previous
v20 export omitted it and reduced saved generations to a match/mismatch boolean.
The new real-device mutation regression fails101 on both old exporters: zeroing
one live lookup leaves captured history identical, as do two different saved
generations that both mismatch the current contact. Restoring device words restores
the original history. Frozen before binaries, source and failures are preserved in
`desktop-baseline/history-capture-fix/`.

Schema v21 exports slot_to_rank for the entire current contact capacity and each
occupied record's physical_slot and saved_generation. Comparator validation rejects
missing/malformed new fields and retains v19/v20 as separate historical coverage.
Both corrected mutation tests pass. Five fresh16step joint/contact-history runs
per backend compare raw-identically under v21 (`desktop-baseline/history-capture-five/`).
All four test builds finish0; runner80092 finishes0. Ten Python storage checks and
nine launcher checks pass, including omission/corruption and version-separation
controls. The initial synthetic fixture lost required GPU policy keys; correcting
that test setup resolved its two errors. Manifest includes source/binary/log hashes.

This changes capture only. Existing v20 ragdoll/replay passes retain their stated
scope but cannot prove equality of newly included history fields. Release/sample
binaries still require a v21 rebuild after the remaining workspace audit; no
full-scene rerun or complete-state qualification is claimed here.

## Joint schedule and remap region review

`desktop-baseline/joint-schedule-remap-state-audit.json` records current source
contracts. Joint heads/counts/offsets/list prefixes are regenerated before their
solver consumers, and the actual ordered schedule is captured. Joint color flags
and contact history remain persistent captured inputs. Shape-remap mapping is
written completely before a tracked clear/remap/prepare/publish submission;
completed capture observes the resulting contact pairs/hash and protected previous-
touching state, rather than an unexecuted mapping command.

Three existing checks pass per path on current v21 tests (driver11933 exit0):
24exclusive ordered joints,80component fused-vs-general color waves, and preserving
contact color when the first joint is added. Logs/binary hashes are under
`desktop-baseline/joint-schedule-audit/`. No solver or capture edit this review.

An explicit remaining gap is the between-step retirement consumer:
retire_body_pair_contacts reads SCR_UNIQUE_N and scr_active_contact. A next-step
reset alone cannot exclude these values from persistent-state coverage. Current
capture has neither their exact contents nor a derivation check against captured
roots. Resolve that with a mutation/retirement discriminator before completing
the scratch audit; device fat-transform command consumption also remains open.

## Between-step retirement candidate capture correction

The active-contact list is a future input even after a physics step completes:
retire_body_pair_contacts reads SCR_UNIQUE_N and scr_active_contact before the
next broadphase reset. A real-device test now removes the sole candidate, verifies
selected retirement is prevented, restores it and verifies retirement succeeds.
The original v21 exporter reports identical contact allocation across that
mutation. Both before tests fail101 at the expected final capture assertion;
those failures, source and binaries remain under `desktop-baseline/retirement-capture-fix/`.

Schema v22 records candidate_unique_count, candidate_contact_count and the exact
physical candidate_slots prefix through the larger count, bounded by pair capacity.
EMPTY entries remain explicit. Comparator checks counts/length/u32 values and does
not normalize this list. Both corrected GPU tests pass. Five fresh16step history
runs per path compare raw-identically with v22 (`retirement-capture-five/`, driver96477
exit0). Thirteen Python storage and nine launcher controls pass, including missing,
malformed and version-separation controls. All four test builds finish0. Full hashes,
commands and receipts: `retirement-capture-fix/manifest.json`.

No solver equations or scene defaults changed. Older v20 full-scene and v21
history results remain limited to their recorded schemas. Device fat-transform
consumption and final region consolidation still precede new full-scene qualification.

## Transform command consumption and preparation failure boundary

`desktop-baseline/fat-transform-state-audit.json` records the current source
contract. At successful public steps, ensure_sim always reaches write_params,
which disables the old batch. A new upload writes the entire body head table
and command payload before publishing its count and checked-increment epoch.
The sole shader reader is update_fat_bounds. At the completed, callback-closed
capture boundary, previous command bytes therefore cannot affect the next
successful step. This does not depend on assuming every prior shape was dispatched.
Pending host command lists and six raw bound words remain captured. Initialized
and applied equality flags suffice for reachable epochs: the bounds epoch is
constant and the command epoch cannot wrap. Public zero-duration steps preserve
pending host commands. Existing native replay/reentry evidence includes transform
changes but remains v20 evidence, not a new v22 qualification run.

Reviewing early returns found a separate unclosed boundary: rejected allocation
validation, failed GpuSim construction and failed scene packing set gpu_fail but
do not mark the world/simulator physics_invalid. Construction failure restores
the previous simulator; step_gpu_inner subsequently checks physics_invalid,
not gpu_fail. Thus a failed preparation can continue toward physics submission.
The successful-path command proof is not a blanket failure-path guarantee.
The bounded rejected-preparation regression now fails on both original paths
at the missing-invalidity assertion (exit101), before any large device allocation.
The correction marks the world and any retained simulator invalid in all three
error returns. Both corrected regressions pass: clearing the error, reducing
the hint and preparing a simulator cannot rehabilitate physics; after another
step request and wait, physics_step remains0 and both invalid flags are set.
This directly exercises host heap rejection. Construction and packing error
returns have source-reviewed matching handling, without independent injected
failures. No solver equations or physical defaults changed.

Initial-joint allocation with CPU-compatible order=1 and joint-filter body
growth with GPU-native order=0 also pass on each backend. The first related
driver incorrectly selected order=0 for the order-storage test; its assertion
failure is preserved, and the corrected configuration passes. This is no claim
that order-storage exists in GPU-native mode. All four builds completed0;
source/binary/log hashes and exact receipts are preserved in
`desktop-baseline/preparation-failure-fix/manifest.json`. The twelve device-buffer
regions still require final consolidation; existing full-scene v20 evidence
does not cover the latest v22 fields or this failure correction.

## Consolidated device-region coverage for the selected candidate

`desktop-baseline/device-region-state-audit.json` consolidates all twelve device
owners and ten scratch partitions, with current source hashes and links to the
individual consumer audits. The current GpuSim declaration still has exactly
211 inventoried fields; none were added or lost during consolidation. This is
source-contract closure for completed public World boundaries with GPU-native
order_enabled=0 and the selected ordinary/native flags (0/131072). It does not
establish coverage for CPU-compatible tree state, arbitrary intermediate GpuSim
calls, or additional diagnostic solver modes.

| Device owner | Persistent representation / exclusion contract |
| --- | --- |
| pass_lut | Captured parameters regenerate every color/bias row before submission |
| bodies | Live hot state captured; deltas reset; dead slots disabled and rewritten before reuse |
| body_cold | Cold fields, inertia, loads, bounds, colliders, materials and referenced geometry captured; device centers validated |
| convex_ccd | Device config, geometry, indices and consumed start-state lanes captured |
| contacts | Assembled hot fields plus exact physical allocation and generations |
| contact_persistent | Persistent fields including all identity lanes; unused point data only omitted under bounded consumers |
| contact_prepared | Valid prepared lanes captured; transient vectors reset before preparation |
| joints | Live fields/colors/history captured; deleted/padded records cannot enter the solver |
| scratch | Captured history, candidates, allocation, schedules and bounds; filter table validated; remaining regions reset or output-only |
| atom | Contact hash and sticky state captured; spatial, Jacobi, island and graph workspace reset under the selected schedule |
| query | Requests overwritten; status/hints/high-water captured; component workspace rebuilt; persistent memo captured separately |
| indirect | Every selected argument has an ordered current producer before copy/dispatch, including native replay |

The consolidation explicitly incorporates v21 reverse history lookup and saved
generations, v22 retirement candidates, exact child placement after compaction,
and terminal preparation failure. It retains only the independently audited
occupied-root and previous-touching membership normalizations. The phase
diagnostic region has no physics consumer; joint-filter storage is validated
against captured device joints rather than assumed correct. Body holes are
packed as disabled/static/sleep records on topology upload; reuse overwrites the
slot before stepping.

Final fresh-process qualification is still open. Existing full-scene v20 captures
cannot prove v21/v22 fields. Rebuild and freeze the current candidate, enforce the
above configuration scope for qualification, and preserve all original physical
failures. Rain/contact-island acceptance, performance and recordings remain open.

Both current release libraries and sample executables now build/link successfully.
Frozen libraries, samples and test binaries plus complete Rust/WGSL source and
qualification-script hashes are in `desktop-baseline/v22-candidate/manifest.json`.
A bounded Rain startup check completes five fresh two-step processes per path
with all child/launcher receipts0. Both audited comparisons pass; raw comparison
fails occupied-root append order at frame1 on both paths and aggregate exit1 is
preserved. Outputs are in `v22-startup-five/{ordinary,native}`. These checks verify
the new candidate's capture startup and selected configuration, not600step Rain
repeatability, recycling or physical acceptance. No long qualification run is
in flight.

## Frozen v22 Rain campaign and contact-island discriminator

The full Rain campaign now runs from verified frozen v22 sample binaries:
five fresh600step processes independently per path, under
`desktop-baseline/rain-v22-five/{ordinary,native}`. Runner sessions63604/15392
were launched with separate outputs and exact child/launcher receipts. They
are correctness captures, not controlled performance measurements. Partial
frames do not establish completed runs or physical acceptance.

An offline contact-island discriminator uses the desktop resting frame65
four-point contact geometry and unit-body mass/inertia. At zero separation and
zero initial contact cache, it applies a centered unit impulse and compares one
normal-only sequential sweep with two opposite-point blocks. The two-variable
complementarity solve checks all four active sets; controls cover no contact,
either single active point and both points. Pair selection uses greatest point
distance, with source-order tie breaking.

Sequential spin is0.14811899rad/s with captured biased mass scaling and
0.16758220rad/s in relaxation. Both block sweeps yield zero spin; energies are
0.25003547/0.25J versus initial0.5J, and linear momentum stays1kg·m/s. This
float64 calculation does not reproduce a timestep: it omits friction, integration,
the third cube, nonzero-cache transport and GPU rounding. It supports a concrete
alternative to point-order reversal, not acceptance. Source/script hashes and
results are in `desktop-baseline/contact-block-discriminator/`. The next bounded
experiment must retain the complete73step fixture's original spin/drift and
conservation limits and validate cached impulses/degenerate pairs; a smaller
one-sweep torque alone cannot justify keeping a solver change.

## Paired normal-contact GPU experiment

The temporary shader candidate solves pairs of accumulated normal impulses as
a two-variable complementarity system. It retains each point's speculative/
biased softness terms, includes nonzero old impulses in the right-hand side,
enumerates unilateral active sets and leaves state untouched for scalar fallback
when the block is poorly conditioned. Four-point manifolds pair the first point
with its farthest point, followed by the remaining pair. No material, timestep,
substep count or acceptance limit changes. This experiment currently affects all
ordering modes and is not yet retained as the production candidate.

The ordinary full73step contact-island fixture passes every original assertion:
peak spin7.45058e-9rad/s, transverse drift2.71538e-10m, energy0.167527016J,
momentum error5.96046e-8kg·m/s, angular momentum6.32216e-10kg·m²/s and axial
overlap4.87566e-5m. Baseline spin0.019269153rad/s and drift0.001521538m remain
recorded failures. The candidate run exits0 with exactly1passing test; complete
v22state and metric reconstruction are preserved in
`desktop-baseline/contact-block-experiment/ordinary/`. These are single-run
results, not repeat qualification. Native runtime verification now also passes
exactly1test/73frames with identical physical metrics. The unchanged240step
sliding/frictionless and600step support/energy fixtures each pass once on both
paths (`contact-block-experiment/related-result.json`, driver30466 exit0).
Active-set/cache/degeneracy controls, CPU-compatible preservation and fresh
repeats remain required before retention. Both candidate builds completed0.
Frozen v22 Rain batches use their unchanged binaries and continue independently;
they do not test this new block candidate.

The ungated paired-contact candidate now also passes five fresh73step processes
per path, including every unchanged physical assertion and exact raw v22state
comparison (`desktop-baseline/contact-block-five/`, driver97634 exit0).
Direct GPU controls independently verify all four unilateral active sets,
nonzero warm caches, softened accumulation, speculative impulse release,
fixed-point reentry, coincident-point rejection and zero effective mass. Rejected
blocks must not partially modify velocities or cached impulses. Both backends
pass the9case test; source builds33350/73535 and tests96786/50825 finish0.

The subsequent gated candidate selects paired solving only for explicit
GPU_PHYSICS_LIVE_CONTACT_ORDER=0. Its creation-time policy uses captured bit18
in params.diagnostic_flags; diagnostic overrides preserve that bit. Unset/default
and CPU-compatible1 keep scalar point order/solving. New selected flag values
are262144ordinary/393216native, so prior flags0/131072 and ungated repeat evidence
remain separately labelled. Gated builds65728/83592 both finish0. Verification
compares entire73step default/CPU-compatible traces against the frozen pre-block
v22candidate, preserving their expected original symmetry failures, then checks
the opt-in fixture and direct controls. This verification must pass before
claiming default-path preservation.

Gating verification4998 now finishes0. On both backends, all73default and
CPU-compatible frames are byte-identical to the pre-block v22binary; their
original symmetry failures are preserved. Both opt-in runs and direct GPU
controls pass. Frozen gated binaries, hashes, exact exits and comparison results
are in `contact-block-experiment/gating/`. Five gated repeats per backend of
contact-island73steps, friction240steps and support600steps are now running
under driver99006. The earlier ungated five-repeat pass is not substituted for
these final mode/configuration checks.

Driver99006 now completes0: all30fresh processes pass exactly1selected test and
every original physical assertion. All six five-run raw v22comparisons pass at
their full73/240/600step durations. Evidence is under
`contact-block-gated-{island,friction,support}-five/`; gated binary hashes and
test/source manifests are retained. This closes the selected contact-island
physical and repeat case on the gated candidate, with original baseline failures
preserved. Keep paired solving in the GPU-native opt-in candidate for broader
qualification. It does not close Rain, the other final matrix fixtures,
performance or recordings. Existing release/sample binaries and the running
Rain v22campaign predate paired solving and cannot be reported as its results.


### Persistent hip phase replay (desktop, 2026-09-29)

Existing24-phase velocity capture is exposed through an artifact-only Rust/C
bridge in desktop-baseline/rain-hip-phase-replay. Build22500/replay37693 exit0.
Capture off/on and original frozen baseline have identical120step body output
SHA2564d49e563a8b21a8ad1389560e6ca85ec2558c0b74e8d266a9d3a1de65f730a2d.
All43slots validate against final public velocities (max5e-8 decimal rounding);
independent double quaternion integration reconstructs hip endpoint rotations
within1.20e-7/component. Reports retain all480substep observations.

Tail60steps show mean biased cone rates along current axes of -6.442,-3.495,
+.235,+3.418rad/s across the four substeps, while projections onto the prepared
axis stay near-6rad/s.54/60fourth substeps have inward prepared but outward actual
projection. Mean first-substep swing reduction.021289rad is nearly undone by
last-substep increase.020620rad. This supports an outdated angular Jacobian
under rotating load; it neither proves individual-constraint work nor clears
physical acceptance. No production change retained by this diagnostic.

A separately frozen bounded candidate is being built under
rain-angular-frame-experiment: refresh cone AND twist frames/Jacobians/masses
together, unlike the earlier rejected cone-only refresh. Prepared inertia and
scalar accumulation remain. Predeclared comparison covers original-material
health480persistent and health514severe120step replays and preserves anchor/twist
failure checks. It is not yet evaluated or accepted.


Angular-frame experiment build23608 and four ordinary replay arms61749 now
finish0. All arms have exact42initial getters,5040finite body rows and3240joint
observations. Hip baseline is byte-identical to its existing empty-contact-fix
control. Candidate hip tail60cone peak.210057→.117623rad, final.183345→.089383;
target anchor separation peak5.908→9.391mm. Severe cone peak1.254127→.766228rad,
target anchor67.881→73.209mm; all-joint upper-twist peak.032335→.061229rad.
The candidate is REJECTED: angular improvement trades for larger other errors.
No native/five-repeat/full-Rain acceptance inferred. Both source variants,
original/candidate archives, manifests, exact exits and summary.json are retained
under rain-angular-frame-experiment. Original solve.wgsl restored byte-exact.
Baseline release rebuild36986 still pending; do not confuse its earlier
candidate target/release artifacts with the restored source.

Baseline release rebuild36986 finishes0 from restored original source;
baseline-rebuild-exit.json records its archive hash and restoration check.
All experiment processes are terminal. No production change retained.


### Paused checkpoint recordings and commit checks (2026-09-29)

See benchmarks/2026-09-29-paused-solver-checkpoint.json under the engine for
portable hashes, settings, completion receipts and the partial-run limitation.
The19standard scenes complete300step CPU/GPU recordings. Native Shape
Replacement/Offset Kinematic complete120steps each on CPU/GPU; Falling Ragdolls
completes600each; CPU Rain completes600. Additional GPU Rain video is a bounded
partial snapshot: intentionally stopped with SIGTERM after at least138observed
steps, FFmpeg finishes0, sample exit-15 and wrapper exit1 preserved. It is NOT
a600step physical/repeat pass. All46clips contain valid video streams; CPU
comparison column is first. Actual clips remain local/ignored. Concurrent
rendering/encoding and single-run standard metrics are not performance evidence.
Release/native sample builds,9repeat-runner tests,13state-storage tests and
diff checks pass. This is an incomplete checkpoint, with goal still paused.


### Contact-island requalification on791990c (2026-09-29)

Bounded user-selected goal completed: existing73step
trace_contact_island_wake_propagation passes five fresh processes each on the
ordinary and configured native cached GPU paths, all10with original assertions.
Both exact/raw v22state comparisons pass for all73frames (730total captured
steps). No normalization, relaxed thresholds, solver changes or extra fixtures.
Source matches791990c6428e641df5c254901eb3afdbbb7fd3ec; feature-enabled builds,
frozen binary hashes, runtime settings and per-frame native cache policy verified.
Builds85250/67977 and driver85409 finish0. All10process IDs are distinct; source
and frozen executable hashes remain unchanged after execution.

Evidence: desktop-baseline/contact-island-791990c-five/{summary.json,ordinary/,
native/,build-*}; portable receipt benchmarks/2026-09-29-contact-island-791990c.json
under the engine. This closes this selected contact-island gate on the committed
build only. The broader Rain/drag, matrix and performance goal remains paused.


### Restitution requalification on b45e86b (2026-09-29)

The bounded240step trace_restitution_matches_analytical_rebound fixture passes
five fresh processes each on ordinary/native cached GPU paths. All10retain the
original bounce, penetration, energy, lateral drift and settling assertions; both
raw v22state comparisons pass all240frames, with no normalization. Total2400
captured steps. Source matches b45e86b4fa08f7edb73240bcc13e36d2ad6dcf7d; frozen
binaries, feature-enabled builds, runtime settings and per-frame cache policy
verified. All10process IDs are distinct; sources/binaries unchanged afterward.

Every run reports first rebound apices0.99096805/2.488538m (targets1.0/2.5m,
error<0.05m), peak penetration0.004999995m (<0.03m), and peak energy25.017302J/kg
(<25.2). Driver61850 finishes0. Evidence: desktop-baseline/restitution-b45e86b-five;
portable receipt benchmarks/2026-09-29-restitution-b45e86b.json under the engine.
This closes the selected restitution gate on the committed solver only; no solver
changes or broader investigation. Full solver goal remains paused.


### Friction, support and stopped performance attempt on d19a93d (2026-09-29)

The bounded sliding-friction and stack-support/energy tasks both PASS on
`d19a93d1a4b3fe51aa1de26222cf14b7560ded3f`. Each fixture passes five fresh
processes per ordinary/native cached GPU path, retaining its original physical
assertions. All four raw v22 captured-state comparisons pass without
normalization: 240 steps per friction run and 600 per support run, 20 processes
and 8,400 captured steps overall. Source/build identity, frozen executable and
trace hashes, frame sequence, ordering and native policy were verified.

Friction stopping distance is 0.39576817 m (target 0.4 m, tolerance 0.04 m);
peak frictionless-control position error is 0.000006198883 m. Stack peak overlap
is 0.0009977221 m, measured peak energy gain is 0 J/kg and tail speed is 0 m/s.
All other original finite-state, drift, orientation and settling checks pass.
Physical driver77733 exits0; both feature-enabled builds exit0.

Performance builds51274/2114 exit0 with diagnostic capture disabled. The fixed
Falling Ragdolls protocol intended five pairs per path, alternating order1/0,
60 warmup plus 180 timed completed steps, 60 Hz/four substeps, sleep enabled,
eight workers and 320x240 llvmpipe rendering. Timing includes GPU completion,
pose synchronization and bounded counters. Mode0 also enables paired normals;
this would compare current modes, not isolated ordering cost.

Performance STOPPED at its first validation failure during the first ordinary
order1 prewarm. The sample exits0, but driver97283 exits1: at submitted/completed
step87, contact metrics are known but current=false (snapshot_state88 versus
current_state89). Snapshot step and topology match; capacity_loss=false. This
records the failed predicate only, without a root-cause claim. No measured pairs
started, so there is no valid timing ratio or performance conclusion. No fixes,
weakened checks or reruns were performed. Performance acceptance remains open;
the broader solver goal remains paused.

Local evidence: `experiments/gpu-physics/artifacts/gpu-solver-qualification/desktop-baseline/three-checks-d19a93d/`.
Portable receipt: `experiments/gpu-physics/benchmarks/2026-09-29-friction-support-performance-d19a93d.json`.
The bounded execution/reporting task ended under its first-failure stop rule;
only the two correctness gates passed. No solver changes, commit or push.


### Performance scheduling-metric diagnosis on0940a4f (2026-09-29)

Read-only analysis of the preserved failed prewarm finds16stale rows out of180.
All180retain known metrics, equal snapshot/completed steps and topology revisions,
and no capacity loss; every stale row differs by exactly one state revision.
The existing CCD-metric regression passes on the ordinary frozen test executable
(one selected test, RTX4070 SUPER/Vulkan). It explicitly requires scheduling metrics
to become stale after host or GPU CCD changes the completed pose state. Source
`mirror_gpu_state` advances query_state when deferred host CCD corrects bodies;
`b3_world_contact_metrics` preserves the scheduling snapshot revision.

Thus current=true is not an unconditional completed-step invariant. These facts
support CCD invalidation as the next hypothesis to verify for step87; the old
report does not expose per-frame correction attribution. No original receipt or
performance validation was changed, and no timings are accepted. Next establish
explicit attribution before defining a phase-aware validator, retaining all
unknown/old-step/topology/capacity rejection. Nine receipt-runner tests also pass.
Portable evidence: `benchmarks/2026-09-29-performance-metrics-diagnosis.json` under
the engine. No solver change, performance rerun, or new native qualification.


### Exact attribution of stale performance metrics (2026-09-29)

One diagnostic ordinary Falling Ragdolls run repeats the original240step prewarm
configuration with a temporary print only at the deferred host CCD correction
branch. For every correction it logs physics step, topology, and state revisions
before/after the existing increment. The240step process exits0. All16stale rows
match exactly one logged correction at the same step/topology/revisions; no
current row has a correction. All180metric records equal the preserved failed
prewarm report exactly. Stale steps:87,117,118,123,142,161,162,164,165,166,167,
169,173,174,181,214. This establishes CCD attribution for that failure, beyond the
previous source-consistency hypothesis. Scheduling snapshots correctly remain
labelled stale after CCD modifies the completed body poses.

The original failed attempt remains failed and unchanged. This run's timings
include diagnostics and are excluded from performance evidence. A revised
performance validator must enforce the actual scheduling-phase contract and
completed-step/capacity guarantees, with negative controls, before a new batch.
It must not relabel pre-CCD counts as post-CCD current contacts. No native path or
five-repeat qualification is inferred from this one ordinary diagnostic run.

Source restored byte-exact; normal release build and sample link exit0. Restored
normal executable SHA256 matches the original frozen performance executable
exactly (eda4900438f712335f1ca8879b5e68061744d1469881ad78502bffab8d044087).
Local protocol, temporary original-source backup, build logs, frozen diagnostic
binary, run script, report, attribution log and receipts are under
`desktop-baseline/performance-ccd-attribution/`. Portable receipt under the engine:
`benchmarks/2026-09-29-performance-ccd-attribution.json`. No production changes remain.


### Phase-aware performance validator and fresh batch (2026-09-29)

`solver_performance_validate.py` validates measurement integrity only for the two
fixed, input-free scenes on an isolated display and the audited Step/Wait/note_world
sequence. It retains scheduling labels and rejects unknown metrics, old steps,
topology changes, capacity loss, false current labels, revision gaps outside0/1,
and unexpected between-step revisions. It also checks the fixed measurement
window, completed steps, scene counts, rendering dimensions, diagnostic absence
and finite positive timings. It does not establish physical correctness or permit
arbitrary mutating scenes. Current=false after CCD remains visible in results.

Four focused tests pass, including missing-field/invalid-state cases, plus15
corruptions of the real failed prewarm report are rejected. Input bytes remain
unchanged. This is validator evidence, not reclassification of the failed run.
A separate controlled Falling Ragdolls batch is started at
`desktop-baseline/phase-aware-small-performance/`, with identical original frozen
release binaries. Five paired runs per ordinary/native path follow equal prewarming;
all original timing/configuration controls remain. Results are pending.


### Small-scene controlled measurements complete (2026-09-29)

Falling Ragdolls phase-aware batch completes0: four prewarm and20measured fresh
processes, all with exit0 and valid180row completed-step windows. Five paired
means are independently recomputed from hashed raw reports; frozen source/binary
hashes remain unchanged. No health/state capture or competing GPU work. Results:

| Path | Order1 mean ms | Order0 mean ms | Geometric ratio | Two-sided95% interval | One-sided95% upper |
| --- | ---: | ---: | ---: | --- | ---: |
| Ordinary |25.3590|23.1647|0.912948|[0.872564,0.955200]|0.945219|
| Native cached |24.5720|22.9559|0.933863|[0.882990,0.987666]|0.974905|

Both predefined small-scene mean-latency screens pass. This compares current modes,
including paired-normal policy for order0, not isolated sorting cost. Per-run
p95/max and paired ratios remain in the portable receipt and local raw reports.
Ordinary p95 improves in allfive pairs; maxima regress in two, so do not claim
uniform tail improvement or approve a default switch from these means. Large
scene, final-candidate physics/repeatability and tail review remain required.
Original failed benchmark receipt remains unchanged. Portable evidence under the
engine: `benchmarks/2026-09-29-small-scene-performance.json`; local raw evidence:
`desktop-baseline/phase-aware-small-performance/`.

Large Pyramid completed afterward in `desktop-baseline/phase-aware-large-performance/`
with 5,050 dynamic cubes plus ground, no joints and upstream sleep disabled.
All four prewarms and 20 measured processes exited 0 and passed validation. The
finalizer independently rechecked all raw reports, hashes, means, p95/max, pair
ratios, confidence bounds and the complete fresh-process matrix.

| Path | Order1 mean ms | Order0 mean ms | Geometric ratio | Two-sided 95% interval | One-sided 95% upper |
| --- | ---: | ---: | ---: | --- | ---: |
| Ordinary | 26.9057 | 27.7275 | 1.030844 | [0.989302, 1.074129] | 1.063921 |
| Native cached | 43.3259 | 45.4259 | 1.048522 | [1.025471, 1.072090] | 1.066571 |

Neither large-scene cell demonstrates a mean-speed improvement; the native cached
path is slower with its two-sided interval entirely above 1. Tails are mixed and
remain visible for every run. No selective repeats were added. Portable evidence:
`benchmarks/2026-09-29-large-scene-performance.json`.

**Default decision: retain CPU-compatible ordering; GPU-native remains opt-in.**
Only two of four predefined mean-speed screens pass, and physical qualification
and the final repeat matrix remain incomplete. These measurements use NVIDIA RTX 4070 SUPER Vulkan physics and a separate
320x240 llvmpipe display renderer. They compare two GPU solver modes, not GPU
versus the Box3D CPU engine. Mode 0 includes paired
normals, so the comparison does not isolate ordering cost. Latency includes queue
completion, pose synchronization and bounded counters; it is not pure shader time.

The user-requested standalone charts show all paired means, geometric ratios with
two-sided intervals and one-sided upper bounds, and all p95/max values. PNG/SVG
outputs and an HTML index were visually checked under
`desktop-baseline/performance-charts/`. Reproduce from the engine directory:

```sh
python3 scripts/plot-solver-performance.py \
  --small benchmarks/2026-09-29-small-scene-performance.json \
  --large benchmarks/2026-09-29-large-scene-performance.json \
  --out artifacts/gpu-solver-qualification/desktop-baseline/performance-charts
```

### Coupled cone/point analytical controls (2026-09-29)

Between timing batches,96CPU-only float64 cases verify the proposed4constraint
block (three point rows and one unilateral cone row). The matrix is J M^-1 J^T;
softness adds Kpp*zp/yp to the point block and Kcc*zc/yc to the cone diagonal,
retaining original row softness in the uncoupled limit. Two active sets enforce
nonnegative accumulated cone impulse. An independent bordered/full4x4 solve agrees
with the Schur result within1.34e-15.46active/50inactive cases satisfy stationarity,
complementarity and the impulse bound within3.34e-14. Momentum error<3.56e-15;
hard zero-bias controls show no energy gain relative to pre-warm initial state.

Protocol and implementation precede GPU evaluation. These are algebra/physical
controls, not proof of float32 behavior, rotating impulse transport or full Rain
acceptance. The next bounded candidate must retain original severe/persistent
replay initial states, finite/complete checks and cone/twist/anchor acceptance;
relative cone improvement cannot hide another worsened constraint. No production
change made. Portable receipt: `benchmarks/2026-09-29-rain-coupled-block-controls.json`.


### Coupled cone/point candidate rejected (2026-09-29)

Fresh diagnostic baseline build and candidate build succeed; all four ordinary
120-step replay arms exit 0. Each has 42 exact initial body getters, 5,040 finite
body observations and 3,240 finite spherical observations with complete frame and
joint identity sets. Both baseline body streams match the previous angular-frame
experiment byte-for-byte. No upstream setup, timestep or softness was changed.

| Check | Baseline | Candidate |
| --- | ---: | ---: |
| Hip target tail-60 cone peak, rad | 0.210057 | 0.107070 |
| Hip target anchor peak, mm | 5.90776 | 9.36527 |
| Severe target cone peak, rad | 1.254127 | 0.722365 |
| Severe target anchor peak, mm | 67.8806 | 76.2845 |
| Severe all-joint upper-twist peak, rad | 0.032335 | 0.051236 |

The candidate is rejected under its predeclared rule: reduced cone error still
trades for larger anchor errors and upper-twist error. The four-row block does
not resolve the problem exposed by angular relinearization. Algebraic block
correctness is insufficient for physical acceptance. No native expansion or
five-repeat qualification follows this failure. Original source is restored;
normal baseline release rebuild 62190 exited 0. Source equality and normal
archive hash are recorded in `baseline-normal-rebuild.json`.

Raw evidence is in `desktop-baseline/rain-coupled-cone-point-experiment/`.
Portable receipt `benchmarks/2026-09-29-rain-coupled-cone-point.json` includes
results, protocol, candidate patch, archive/script/initial-state hashes and exact
baseline comparisons. Keep the original residual screens failed. Before another
solver variant, use the retained output and existing joint-state/compliance
analysis to distinguish load redistribution from unresolved point/twist coupling;
do not simply add stiffness or tune scene defaults to hide these errors.


### Rejected candidate anchor/compliance audit (2026-09-29)

Two existing frozen ordinary hip fixtures were rerun for 120 steps with the
existing complete state capture enabled, without shader or configuration changes.
Both exit 0 and reproduce their original noncapture body streams byte-for-byte.
All 120 captures per arm are complete. Float64 reconstruction of the target world
anchor separation agrees with the native getter measurement within 20 micrometers
at every frame. The static estimate uses C = -Kpoint * lambda / (h * omega^2),
with prescribed joint frequency clamping, captured start inertia, end levers and
cached final point impulses. It is not a full per-substep equilibrium identity.

| Tail-60 mean | Baseline | Rejected candidate |
| --- | ---: | ---: |
| Anchor separation, mm | 2.216 | 8.160 |
| Static point-softness estimate magnitude, mm | 1.830 | 8.736 |
| Vector difference from static estimate, mm | 1.175 | 1.658 |
| Relative anchor point speed, m/s | 0.169 | 0.434 |

The increased separation is broadly consistent with increased loaded point
compliance; it cannot be classified solely as a failure to solve the local point
block. Significant motion and a nonzero vector residual prohibit claiming static
force balance or physical acceptance. No criterion changed, and the coupled
candidate remains rejected. The next solver investigation must account for the
angular-axis/cached-impulse update and coupled load response, rather than blindly
increasing point stiffness or adding another partial block.

Evidence: `desktop-baseline/rain-coupled-anchor-audit/`, portable
`benchmarks/2026-09-29-rain-coupled-anchor-audit.json` (includes the analysis/run
sources, capture hashes and exact-body no-op control). No live process remains.


### Substep angular-frame consistency candidate rejected (2026-09-29)

Source audit confirms warm/bias/integrate/relax order per substep. A bounded
unit-inertia projection control shows changing the row axis before relaxation
leaves a tangential component, but does not itself prove energy injection: the
projection reduces kinetic energy relative to the pre-relax state. The candidate
therefore tests phase consistency without claiming an established transport bug.
It refreshes cone/twist axes at warm start, holds them through biased solve and
relaxation, and retains original sequential rows, gyro arithmetic and softness.
Artifact-only scratch uses fields otherwise unused by spherical joints; it is not
a production state-layout design. An early build with an accidental arithmetic
helper substitution was preserved unevaluated and corrected before replay.

All four ordinary 120-step arms exit 0 with exact initial getters, 5,040 finite
body observations and 3,240 complete joint observations each. Both baseline state
streams remain byte-identical to the previous controls.

| Check | Baseline | Candidate |
| --- | ---: | ---: |
| Hip tail-60 cone peak, rad | 0.210057 | 0.091263 |
| Hip target anchor peak, mm | 5.90776 | 8.99274 |
| Severe target cone peak, rad | 1.254127 | 0.766278 |
| Severe target anchor peak, mm | 67.8806 | 72.5400 |
| Severe all-joint upper-twist peak, rad | 0.032335 | 0.057534 |

Rejected under the unchanged predeclared anchor/twist tradeoff check. Refresh
timing alone is insufficient; do not repeat this or the previous angular-refresh
variants. No native/five-repeat qualification follows. Raw artifacts:
`desktop-baseline/rain-substep-angular-frame-experiment/`. Portable result with
candidate patch, hashes and controls:
`benchmarks/2026-09-29-rain-substep-angular-frame.json`.
Original source is restored; normal baseline rebuild 29395 exited 0 and its
archive SHA256 exactly matches the prior normal baseline. No live job remains.

Next Rain work should close a physical load/work criterion using the original
persistent/severe evidence and prescribed softness before another solver variant.
The static estimates alone cannot close this, and a local velocity projection is
not a full work balance. Preserve failed original screens and prior rejected
candidates. Final-build applicability of the finite capability/repeat matrix
remains required, including earlier completed ground-dragging evidence.


### Original hip energy accounting and 600-step compliance screen (2026-09-29)

Reused existing phase captures after verifying byte-identical body output against
the recent baseline state capture. The ledger covers all 42 dynamic bodies and
24 phase observations over each of 120 steps. Reconstructed final orientations
agree within 1.48e-7 per component; final velocities agree exactly. Kinetic phase
increments telescope within 1.07e-14 J. Distinct captured inverse-inertia lanes
are retained; their maximum upper/lower difference is 1.53e-5 from float rounding.
An initial strict symmetry assertion failed and was replaced by a recorded
asymmetry measurement plus positive-definite symmetric-part checks, without
changing inputs. Quadratic kinetic forms discard antisymmetric components.

Whole-cell kinetic plus gravitational energy decreases 143.7105 J over the replay,
and 6.6184 J over the last 60 steps. Tail mean phase changes per substep are
+0.3182 J velocity integration, +10.7974 J warm start, -3.3955 J biased solve,
about -4.68e-7 J orientation/inertia update, and -7.7432 J relaxation. This does
not prove runaway energy, but also omits stored compliant contact/joint energy
and individual-constraint work, so it is not a physical acceptance proof.
Evidence and reproducible source: `desktop-baseline/rain-hip-phase-energy/`,
`benchmarks/2026-09-29-rain-phase-energy.json`.

A predeclared unchanged 600-step extension then tests whether the original hip
enters the already validated stationary loaded-cone regime. The criterion retains
angular speed <1e-3 rad/s and absolute measured/predicted residual difference
<1e-4 rad for every final-120 observation; stationary-axis agreement is required.
It is a residual-classification test, not a substitute for original Rain screens.

The ordinary replay exits 0, reproduces all original 42 initial getters and the
first 120 body frames exactly, and validates 25,200 finite body observations,
16,200 complete joint observations and 600 state captures. The stationary screen
FAILS: tail angular speed peaks 11.3438 rad/s, cone error peaks 0.192977 rad,
and estimate error peaks 0.176926 rad. At step 600, measured cone error is
0.186425 rad versus static estimate 0.0133593 rad, endpoint speed 9.21768 rad/s,
and start/end axis dot 0.638437. Both endpoints are awake.

This rules out explaining this original persistent replay as settled loaded
compliance over the tested duration. It does not establish a total-energy source
or full-scene equivalence. Do not repeat static-compliance classification for
this moving episode. Next bounded solver work should address joint/contact
convergence together, retaining original timing, material and joint defaults,
rather than another isolated angular-axis refresh. Full Rain/native/five-repeat
acceptance remains open. Evidence: `desktop-baseline/rain-hip-long-compliance/`,
`benchmarks/2026-09-29-rain-hip-long-compliance.json`. No source edits or live jobs.


### Two whole joint/contact sweeps: improvement, still unqualified (2026-09-29)

The bounded ordinary discriminator executes exactly two complete biased and two
relaxation waves per original substep. Warm start/integration remain once;
original angular preparation, timestep, four substeps and physical defaults stay
unchanged. Runtime markers prove both extra passes execute. All four 120-step
arms exit 0 with exact initial getters, 5,040 finite body rows and 3,240 complete
joint observations each. Baseline streams remain byte-identical to prior controls.

| Check | Baseline | Two sweeps |
| --- | ---: | ---: |
| Hip initial cone peak, rad | 0.314239 | 0.318692 |
| Hip tail-60 cone peak, rad | 0.210057 | 0.174631 |
| Hip target anchor peak, mm | 5.90776 | 5.18150 |
| Severe cone peak, rad | 1.254127 | 1.137679 |
| Severe target anchor peak, mm | 67.8806 | 63.2383 |
| Severe all-joint upper-twist peak, rad | 0.032335 | 0.023802 |
| Severe tail-60 cone peak, rad | 0.007469 | 0 |

The early hip cone regression is retained; this is not a uniform improvement or
acceptance. A predeclared 600-step candidate extension preserves its entire first
120-step body prefix and all initial getters; 25,200 finite body rows, 16,200 joint
observations and 600 states validate. It still fails the unchanged stationary
classification: tail angular speed peaks 7.5088 rad/s and cone error 0.167700 rad.
Final error 0.156916 rad versus static estimate 0.0120632 rad accompanies speed
3.95137 rad/s, with both endpoints awake. Extra global convergence alone is
insufficient. No native/five-repeat expansion or production adoption follows.

Portable evidence: `benchmarks/2026-09-29-rain-two-sweep.json` and
`benchmarks/2026-09-29-rain-two-sweep-long.json`; raw artifacts use corresponding
`desktop-baseline/rain-two-sweep-experiment/` and `rain-two-sweep-long/` directories.
Original sim.rs is restored; normal baseline rebuild 58446 exited 0 and its
archive matches the prior baseline byte-exact. No live process remains.
The complementary missing discriminator is refreshed angular rows together with
the extra whole-wave pass: isolated refresh and unchanged-axis extra iteration
have each failed. Evaluate at most that missing combination before broadening
this experiment, retain all original error checks and do not claim default or
performance acceptance from this diagnostic.


### Finite angular-refresh / whole-wave comparison complete (2026-09-29)

The final combination uses the original full angular-refresh candidate plus
exactly two complete biased and relaxation joint/contact waves. No block/scratch
variant, scene tuning or timestep/substep change. Build and all four ordinary
120-step arms exit 0. Initial getters, 5,040 finite body rows and 3,240 complete
joint observations per arm validate; runtime markers confirm extra waves. Both
baseline streams exactly reproduce earlier controls.

| Check | Baseline | Combined candidate |
| --- | ---: | ---: |
| Hip cone peak, rad | 0.314239 | 0.110976 |
| Hip tail-60 cone peak, rad | 0.210057 | 0.096286 |
| Hip target anchor peak, mm | 5.90776 | 9.48150 |
| Severe cone peak, rad | 1.254127 | 0.591598 |
| Severe target anchor peak, mm | 67.8806 | 58.7868 |
| Severe all-joint upper-twist peak, rad | 0.032335 | 0.027670 |

Rejected: the hip anchor tradeoff remains. All four cells (baseline, refreshed
axes alone, two waves alone, combined) are now evaluated. No physical acceptance,
native expansion, selective repeats or production adoption is inferred. Do not
expand this into an iteration-count search. The original baseline itself remains
physically unqualified for Rain; rejecting candidates does not qualify it.

Raw evidence: `desktop-baseline/rain-refresh-two-sweep-experiment/`. Portable
receipt with both source patches and hashes:
`benchmarks/2026-09-29-rain-refresh-two-sweep.json`. Source restoration is verified;
normal baseline rebuild 43716 exited 0, and its archive matches the previous
baseline byte-exact. No live process remains.

The finite study identifies a consistent loaded-anchor tradeoff when angular
constraints respond more strongly. Existing end-state compliance estimates show
much of that separation corresponds to increased point impulse, while original
600-step motion rejects a stationary-softness explanation for the baseline cone.
These facts still do not provide a coupled force/work acceptance argument. Next
work must address that physical criterion and preserve the original screens,
rather than keep recombining the rejected variants.


### Current-baseline state audit applicability verified (2026-09-29)

The consolidated completed-public-World source contract was rechecked against
0940a4f after all rejected experiments restored the baseline. All 18 recorded
audit dependencies match their hashes; the declaration still has all 211 fields
in the recorded order, with no additions/removals. The 12 device owners and ten
scratch partitions remain covered. The selected flags include the paired solver:
ordinary 262144 and native cached 393216, with GPU-native ordering enabled.

Of 23 source files covered by consolidation plus the paired-solver amendment,
21 match exactly. The remaining changes are fully accounted for: world.rs adds
exactly two functions/72 lines inside cfg(test), whose removal reproduces audited
bytes; collide.wgsl adds the reviewed empty-manifold recycling veto, selected by
already-captured contact count without new persistent storage. Geometry, history,
physical placement and ordering retain the existing capture contract. Existing
membership normalizations are unchanged.

Changed-source, missing-field and wrong-mode receipt corruptions all reject.
An initial guard incorrectly expected a blank line between cfg(test) and its
module; that formatting assertion failed before output and was corrected while
retaining byte-exact equivalence and scope checks. No physics test was rerun or
claimed here. Portable `benchmarks/2026-09-29-state-audit-applicability.json`
contains all 18 dependency audit documents, field inventory, source hashes,
reviewed diffs and verification source. This removes dependence on local ignored
files for reading the audit's contracts; missing raw test captures remain missing.

The source-contract applicability gate is closed for this baseline and selected
completed World boundary. It excludes CPU-compatible tree state, arbitrary
midstep/low-level GpuSim states and extra diagnostic modes. Final v22 five-process
qualification on each selected configuration remains open, as do Rain physical
acceptance and recordings. Rejected candidates are not covered by this receipt.


### Hip row kinetic accounting validated (2026-09-29)

A bounded diagnostic records pair kinetic changes after spherical spring, motor,
twist, cone and point rows. It uses current delta orientation for inertia and
otherwise unused spherical weld fields plus the existing ground diagnostic lanes.
No new production storage or algorithm is retained. Build7466 and120step replay/
analysis4899 exit0. All physical body output bytes, every dynamic-body phase word
and every aggregate phase-summary word match the original phase replay exactly.
The1440pass observations telescope within7.16e-7J, at most4.33% of the predeclared
float32 roundoff bound. Target endpoints/slot are verified at every phase.

Tail60mean kinetic changes per target hip update:

| Update | Bias J | Relaxation J |
| --- | ---: | ---: |
| Spring | -0.216815 | -0.069313 |
| Motor | -0.119675 | -0.074622 |
| Twist | 0 | 0 |
| Cone | +2.462693 | -2.170685 |
| Point | -0.788935 | -0.410249 |
| Total | +1.337268 | -2.724869 |

Warm starting removes6.35835J on average for this target pair in the same tail
window. These local changes are evaluated where the joint actually executes,
not reconstructed from final cached impulses. The target joint's complete
updates therefore remove pair kinetic energy on average, while the cone bias
row adds kinetic energy and relaxation removes most of that addition. Springs
and motors are dissipative on average in this record; twist is inactive.

This does not account for compliant stored energy or energy exchanged through
other joints and contacts. It rules out claiming the hip joint alone is a net
kinetic source in this replay, not a coupled-work or physical-acceptance pass.
The next discriminating accounting step is complete spherical/revolute joint
contributions versus the already measured whole-cell phase changes, retaining
the same exact no-op controls; infer no contact contribution until all joint
kinds are included. Do not alter the solver based on a single positive row.

Raw evidence: `desktop-baseline/rain-hip-joint-work/`. Portable protocol, patches,
sources, hashes and results: `benchmarks/2026-09-29-rain-hip-joint-work.json`.
Both original shaders are restored. Normal baseline rebuild89208 exited0 and
its archive matches the prior baseline byte-exact; no live job remains. Diagnostic
archives remain explicitly separate.


### Complete joint/contact kinetic exchange (2026-09-29)

The same original120step replay now records every27spherical and12revolute joint
update;3kind5filter joints have no solver impulse. Revolute weld fields are live
cached axes and were explicitly preserved; its otherwise unused swing_impulse
carries the diagnostic total. Spherical diagnostic carriers are cleared after
each capture to avoid counting stale skipped/asleep updates. No production change.

Build/replay31507 and analysis exit0. Physical body bytes, all dynamic phase words
and all aggregate phase summaries exactly match original. All1440pass observations
are finite; joint counts match every phase. Spherical aggregate row sums telescope
within1.75e-6J under the predeclared aggregate float32 guard. Only ground diagnostic
lanes differ. The existing whole42body kinetic ledger supplies total phase changes;
subtracting all joint contributions gives net contact contribution.

Tail60mean J per substep phase:

| Phase | Spherical | Revolute | All joints | Contacts (inferred) | Whole cell |
| --- | ---: | ---: | ---: | ---: | ---: |
| Warm start | +6.1402 | +12.7134 | +18.8536 | -8.0563 | +10.7974 |
| Biased solve | +1.6034 | -3.6534 | -2.0500 | -1.3456 | -3.3955 |
| Relaxation | -4.6977 | -0.8406 | -5.5383 | -2.2050 | -7.7432 |
| Sum | | | +11.2654 | -11.6068 | -0.3414 |

Contacts net remove kinetic energy in this accounting; they are not evidenced as
a net kinetic source. Joint updates add energy overall, concentrated at warm
start, especially revolute joints. This is pass-wise kinetic attribution under
interleaved solver order, not a complete stored-energy work balance or proof of
a warm-start defect. The original moving Rain residual remains unqualified.

Next bounded check: inspect revolute cached-axis/impulse preparation and warm-start
transport using existing state and phase evidence. It must distinguish a genuine
cache/basis defect from valid release of stored compliant energy; do not simply
disable warm starting, change physical defaults or attribute positive row work
to a defect. No further cone-refresh/iteration search follows these results.

Raw evidence: `desktop-baseline/rain-all-joint-work/`. Portable protocol, sources,
patches, hashes and results: `benchmarks/2026-09-29-rain-all-joint-work.json`.
Both original shaders restored; normal baseline rebuild7422 exited0 and its
archive matches prior baseline byte-exact. No live process remains.


### Revolute basis continuity and prospective cache-reuse control (2026-09-29)

Source review confirms each alignment solve updates the cached perpendicular axes
used by the next warm start; first use each physics step prepares them from start
geometry, following upstream preparation. Independent float64 reconstruction over
all12hinges gives1440updated-axis observations with no stale step tag. Across1428
frame boundaries, the maximum axis component difference is7.06e-7 and the maximum
change in reconstructed cached perpendicular torque impulse is7.51e-7, versus
maximum cached norm1.40216. This rules out a large boundary basis discontinuity
in this original replay; no projection/transport fix is justified by that hypothesis.
It does not establish substep work conservation. Evidence/source:
`benchmarks/2026-09-29-rain-revolute-basis-audit.json` and
`desktop-baseline/rain-revolute-basis-audit/`.

A prospective numerical cache-reuse control now has96float64 pair-impulse algebra
checks. For a passive warm-start guess with kinetic change L*alpha+Q*alpha^2,
retain the maximum alpha in[0,1] giving nonpositive change, scaling cached impulses
and applied velocity increments together. This leaves already-dissipative warm
updates unchanged and retains the original subsequent biased spring/motor/limit
solve. Positive prescribed motor motion must be excluded from a candidate.

The controls cover15unchanged,33partially scaled and48zero-cache guesses. Linear
and angular momentum errors stay below1.78e-15 and7.95e-15; maximum numerical
energy increase is7.11e-15. Independent impulse reconstruction agrees within1e-12.
This is not proof of float32 behavior, full-solver convergence, retained loaded
response or Rain acceptance. Stored compliant energy can legitimately release
kinetic energy, so this is a numerical initial-guess experiment, not a claim that
all positive joint work is invalid. No production code or defaults changed.

Protocol/source/results: `desktop-baseline/warm-cache-line-search-controls/`,
portable `benchmarks/2026-09-29-warm-cache-line-search-controls.json`. A bounded
candidate must preserve cache/velocity consistency, active motor behavior, all
original physical screens and CPU-compatible debugging. Do not turn this into
blanket warm-start disabling or accept algebra controls as physics qualification.


### Passive warm-cache line search: rejected (2026-09-29)

The predeclared artifact-only candidate scales passive spherical/revolute cached
impulses and their applied updates together. It chooses the largest factor in
[0,1] with nonpositive quadratic change in the **prepared solver inertia metric**.
This differs from the orientation-updated physical kinetic ledger and excludes
stored compliant energy. Prescribed nonzero motors and kinematic endpoints are
excluded; original physical coefficients, bias/relaxation solving and live hinge
axes remain unchanged. CPU-compatible ordering is guarded out. This is an initial
guess experiment, not a physical rule forbidding positive joint work.

Build30828, replay/analysis51772 and restoration76339 exit0. All four ordinary
120step Rain arms validate exact initial getters, complete finite 5040body and
3240spherical observations. Both fresh baseline body streams reproduce earlier
baseline bytes exactly. Results under the unchanged tradeoff checks:

| Metric | Baseline | Candidate |
| --- | ---: | ---: |
| Hip target peak cone (rad) | 0.314239 | 0.127874 |
| Hip target peak anchor (mm) | 5.908 | 2.305 |
| Hip all-joint lower twist (rad) | 0.023374 | 0.055346 |
| Hip all-joint upper twist (rad) | 0.004442 | 0.019265 |
| Severe target peak cone (rad) | 1.254127 | 1.680792 |
| Severe target peak anchor (mm) | 67.881 | 75.008 |
| Severe all-joint lower twist (rad) | 0.385928 | 0.885322 |
| Severe all-joint upper twist (rad) | 0.032335 | 0.051027 |

The candidate is **rejected**, with five explicit anchor/twist tradeoff failures
and increased severe cone error. Reduced hip angular speed and cone residual do
not qualify it. Original Rain screens remain failed; no native expansion or final
five-repeat qualification follows this result.

The unmodified spherical_compliance_reference.cpp independently passes baseline
and candidate at both 0.01/0.1rad constant-torque loads:600steps each, last120
maximum prediction errors1.7423e-6/5.0945e-6rad, angularspeed0. All1200observations
per arm are finite and complete. Original thresholds remain1e-4rad and1e-3rad/s.
This isolates preservation of the already-settled analytical response; it does not
offset the coupled contact-loaded Rain regression.

Kinetic-only cache selection is insufficient. A subsequent proposal must derive
its objective from the actual regularized constraint residual, including bias and
compliant impulse terms, before another solver change. Do not tune this rejected
cap or restart the completed angular-refresh/iteration-count study.

Portable protocol, patch, build/run/analysis sources, manifests, output hashes,
rejection reasons and restoration evidence:
`benchmarks/2026-09-29-rain-warm-cache-line-search.json`.
Raw artifacts: `desktop-baseline/rain-warm-cache-line-search/`.
Original source restored byte-exact. Normal release archive matches the preceding
baseline SHA256 b3f2b64239d72c95c8e1746f9bd40a95152fb7a2c6ab373a6e2bd2986cf5265b.
No production solver change or live process remains.


### Regularized residual and spherical motor projection (2026-09-29)

For effective inverse mass K, current relative velocity w, bias b and accumulated
impulse lambda, the existing softened row update is
`delta = -m K^-1 (w+b) - i lambda`, where m+i=1. With m>0, define
`Gamma=(i/m)K`. The update zeros `w' + b + Gamma lambda'`; scalar unilateral
projection enforces complementarity.96random bilateral and96scalar unilateral
controls pass (relative residual6.96e-15, KKT error2.14e-13). These controls do not
demonstrate a sign or softness-coefficient error. They also explain why positive
kinetic work alone is not a valid rejection criterion for a biased joint update.

A concrete constrained-row discrepancy exists in spherical motor saturation.
The target-relative objective is
`F(lambda)=0.5 lambda^T K lambda + (w-K lambda_old)^T lambda`, under the existing
Euclidean torque bound `||lambda|| <= h*maxTorque`. Its constrained minimizer
satisfies `(K+mu I)lambda=-g`, mu>=0, and boundary complementarity. The current
radial clamp of the unconstrained solution is exact for isotropic K, but generally
not for anisotropic K. Source review finds the same approximation upstream; this
is a numerical solve improvement proposal, not a GPU transcription bug.

Of96synthetic cases,10radial updates increase F relative to the feasible old
impulse, maximum5.67005. The metric solution satisfies all declared KKT/objective/
bound controls; isotropic solutions agree within1.67e-16. A separate128case
float32 emulation of24secular bisections agrees with an independent float64 eigen
reference within3.17e-7 relative solution error, with no torque-bound excess.

In3240frozen end-state hypothetical motor updates (27motors across120original
hip frames),360radial updates increase F, maximum0.0480096J; the metric solution
never increases it. Maximum radial excess over the constrained minimum is
0.0617148J. Input prepared-inertia upper/lower asymmetry is at most2.82e-6 and is
symmetrized for the objective. These are **not actual per-row shader inputs**:
contacts/other rows would change velocities before a live update. The audit
establishes a concrete row-objective discrepancy, not causation or a predicted
Rain improvement. Full physics acceptance remains open.

Protocol, analysis source, hashes and controls:
`benchmarks/2026-09-29-rain-motor-metric-controls.json`.
The resulting artifact-only candidate was screened against unchanged original
Rain and loaded-cone checks. It leaves warm start, target speeds, torque caps,
physical settings and unsaturated/CPU-compatible paths intact. No production
adoption follows from these algebra controls.


### Spherical motor metric candidate: insufficient for Rain (2026-09-29)

The artifact-only saturated native-order motor implementation uses24secular
bisections, a final radius guard and a feasible-old-impulse fallback if roundoff
increases the quadratic objective. It changes no warm cache policy or physical
setting. This is a bounded test of the demonstrated constrained-row issue,
not another warm-start cap.

Build72786, run98522 and normal rebuild9671 exit0. Four120step ordinary Rain arms
validate exact initial getters and complete finite5040body/3240joint records each;
both new baseline streams match previous bytes exactly. The candidate is rejected
under the predeclared hip anchor/lower-twist tradeoff checks and does not resolve
the original cone failures:

| Metric | Baseline | Candidate |
| --- | ---: | ---: |
| Hip target peak cone (rad) | 0.314239 | 0.319970 |
| Hip target tail60peak cone (rad) | 0.210057 | 0.214721 |
| Hip target final cone (rad) | 0.183345 | 0.156182 |
| Hip target peak anchor (mm) | 5.90776 | 5.98456 |
| Hip all-joint lower twist (rad) | 0.0233740 | 0.0234376 |
| Severe target peak cone (rad) | 1.254127 | 1.254525 |
| Severe target peak anchor (mm) | 67.8806 | 67.8776 |
| Severe all-joint lower twist (rad) | 0.385928 | 0.384739 |
| Severe all-joint upper twist (rad) | 0.032335 | 0.030620 |

The unmodified600step loaded-cone fixture passes both loads on both arms;
1200finite complete observations per arm independently verified. Prediction
errors remain1.7423e-6/5.0945e-6rad with zero tail angularspeed. Original criteria,
failed Rain screens and earlier candidate rejections remain intact. No native
expansion, five-repeat qualification or performance claim follows this result.

The row-objective discrepancy is real, but correcting this isolated motor
projection is insufficient for the coupled Rain failure. Next inspect existing
end-state joint residuals including point/angular coupling and complementarity;
do not tune this motor cap or start another iteration-count sweep.

Portable evidence: `benchmarks/2026-09-29-rain-motor-metric.json`.
Raw artifacts: `desktop-baseline/rain-motor-metric-experiment/`.
Original shader restored byte-exact, normal archive SHA256
b3f2b64239d72c95c8e1746f9bd40a95152fb7a2c6ab373a6e2bd2986cf5265b unchanged.
No production changes or live process remain.


### Final joint residuals and coupled hinge discriminator (2026-09-29)

Offline analysis uses the original120frame hip capture and matching phase trace.
All39active joints give4680complete observations. Phase21 velocities are the final
relaxation outputs; phase23 matches the published capture exactly, and there is
no restitution/finalization velocity change in this replay. Prepared inertia,
current end geometry, saved impulse caches and pass-specific softness/bias rules
reconstruct each independent block's projected residual. Unviolated scalar limits
can have large positive slack with zero projected residual; raw residual norm is
not interpreted as error in those cases.

The reported gap is the decrease available from an isolated block minimization.
It is not physical energy gain, and summing gaps does not bound a coupled global
objective. Target-hip tail60mean gaps are point0.07497, spring0.05897, motor0.02751,
cone0.16011. Adjacent right-knee hinge37 (endpoints38/39) reaches alignment gap
0.61537 at frame81. For all720tail hinge observations, an isolated exact point
correction changes mean alignment gap only0.02782->0.02747 and worsens309cases.
Thus the remaining residual is not restricted to motor projection.

Portable protocol/analysis/hashes/results:
`benchmarks/2026-09-29-rain-final-joint-residuals.json`.
No additional GPU instrumentation or physical acceptance claim was introduced.

The next bounded candidate couples the hinge's two alignment and three point rows,
retaining each original softness regularizer and the physical cross terms. Axial
spring/motor/limit solving and warm-start geometry remain unchanged.96float64
Schur solutions agree with independent full5x5 solutions within7.41e-16 relative
error; residual2.42e-15. The uncoupled limit preserves the original row updates.
All32hard zero-bias controls remove kinetic energy and conserve pair momentum
within6.04e-16. Controls: `benchmarks/2026-09-29-rain-hinge-block-controls.json`.

A new independently prescribed loaded-hinge fixture was frozen before evaluation.
With unit mass/inertia, static endpoint A, constant force F and rotated lever r,
the point equilibrium is `C=Kpp*F/omega^2`. Transverse angular equilibrium at theta
requires `tau=2*omega^2*tan(theta/2)` and applied torque `r cross F + tau`.
Four600step cases use theta0.01/0.1rad and lever0/0.4m, original60Hz/damping2.
CPU and both ordinary GPU arms pass the original last120limits: angle/anchor
error<1e-4, linear/angular speed<1e-3. All7200finite ordered observations validate.
Maximum tail angle error1.61e-9rad, anchor error5.97e-9m; maximum speed/angularspeed
2.34e-8/5.84e-8. This closes the isolated coupled-load formula check only.

Build98892, runner23154 and normal rebuild99151 exit0. Original Rain120step arms
all have exact initial getters and complete finite5040body/3240joint observations;
both new baseline streams match earlier bytes exactly. Candidate results:

| Metric | Baseline | Candidate |
| --- | ---: | ---: |
| Hip target peak cone (rad) | 0.314239 | 0.316846 |
| Hip target tail60peak cone (rad) | 0.210057 | 0.200186 |
| Hip target peak anchor (mm) | 5.90776 | 5.88728 |
| Hip all-joint lower twist (rad) | 0.023374 | 0.023733 |
| Hip all-joint upper twist (rad) | 0.004442 | 0.007383 |
| Severe target peak cone (rad) | 1.254127 | 1.254523 |
| Severe all-joint lower twist (rad) | 0.385928 | 0.386411 |
| Severe target tail60peak cone (rad) | 0.007469 | 0.027068 |

The candidate is rejected under three predeclared twist tradeoff failures. Both
existing loaded-cone arms pass unchanged; no native/five-repeat expansion follows.
Original severe and persistent Rain failures remain unresolved. A local hinge
block with correct isolated loaded response is insufficient for this coupled case.

Next examine convergence of complete interleaved joint/contact waves with one
predeclared residual/complete-state stopping criterion and an explicit failure
cap. Net velocity change alone is insufficient because constraint impulse changes
can cancel. This is a convergence discriminator, not an iteration-count sweep or
permission to weaken physical screens. Larger-world and native cached execution
remain separately required if a small-cell experiment succeeds.

Portable protocol, candidate patch, new analytical fixture source, build/run
sources, receipts and hashes: `benchmarks/2026-09-29-rain-hinge-block.json`.
Raw artifacts: `desktop-baseline/rain-hinge-block-experiment/`.
Original shader restored byte-exact; normal release archive remains
b3f2b64239d72c95c8e1746f9bd40a95152fb7a2c6ab373a6e2bd2986cf5265b.
No production changes or live process remain.


### Complete-wave convergence discriminator: nonconverged (2026-09-29)

A bounded artifact-only experiment repeats complete interleaved color waves for
bias/relaxation on the original43body Rain cell. Every joint/manifold update
contributes endpoint velocity and reusable impulse changes, normalized componentwise
as `abs(new-old)/(1+max(abs(new),abs(old)))`. Require both maxima<=1e-5 for two
consecutive complete waves, with a fixed128wave cap. Warm start, integration and
restitution remain once per scheduled phase. No tolerance/cap tuning after results.

Three solver-skipped filter-joint records carry8pass counts and final change
norms per step. A one-wave control first verifies noninterference. Initial control
runner43312 aborted before simulation due to the WGSL reserved name `active`;
failed source/archive/receipt remain in failed-parse-v1/. Renaming the local to
cv_enabled changes no numerical behavior. Both full shaders then pass Naga parsing
and validation; corrected build17915 exits0.

Control70256 produces the original hip body stream byte-exact. Its raw captured
state comparator fails because occupied contact root append order differs in97of
120frames. That failure remains preserved. The repository's **existing**
`audited_storage_view` confirms all120complete captured states equal after its
previously audited membership-only normalization and the five declared filter
carrier fields. Membership identities, allocation generations, owner/history,
solver order, contacts, impulses and physical state remain exact. This is not a
new semantic exclusion. Companion52327 completes severe120steps with exact original
body bytes. Two corruption controls retain rejection of membership/physical changes.

Candidate build51528 exits0. Probe40187 executes one original hip step with exact
initial getters and42finite body records; all8pass records validate. Child exit0
means execution succeeded; driver exit1 records numerical nonconvergence:

| Substep | Phase | Waves | Last velocity change | Last impulse change | Converged |
| --- | --- | ---: | ---: | ---: | --- |
| 0 | Bias | 72 | 8.62e-6 | 2.57e-6 | Yes |
| 0 | Relax | 128 | 0.07853 | 0.01268 | No |
| 1 | Bias | 108 | 8.52e-6 | 1.20e-6 | Yes |
| 1 | Relax | 128 | 0.02393 | 0.001096 | No |
| 2 | Bias | 128 | 4.47e-7 | 1.70e-5 | No |
| 2 | Relax | 128 | 0.04384 | 0.004676 | No |
| 3 | Bias | 94 | 3.37e-6 | 1.37e-6 | Yes |
| 3 | Relax | 128 | 0.02601 | 0.002083 | No |

All four relaxation phases remain far from the fixed numerical criterion. The
third biased phase illustrates why net/body velocity alone is insufficient:
velocities are quiet but reusable impulses are still changing. Expansion stops
as predeclared; no120step Rain/loaded-response/native/five-repeat pass is claimed.
Additional passes alone are not demonstrated to produce a converged solution.

Scope correction to the initial accumulator comment: contact total_normal_impulse
is a monotonic per-solve accumulator, used by restitution as zero/nonzero and hit
events as positive/nonpositive; its magnitude is also publicly exposed by manifold
getters and captured state. It is excluded **only from the fixed-point stopping
norm**, never complete-state qualification. This artifact preserves its existing
accumulation at each repeated solve, so magnitude changes with the repeated work.
A retained production algorithm would need explicit accounting semantics and
validation. No claim of unchanged public manifold magnitudes is made.

A fixed-end-state motor check finds15of27hypothetical isolated radial motor updates
still increase their quadratic objective, maximum3.80e-6J; metric projection never
increases it. These are not actual row-call inputs and do not prove causation.
They justify one targeted next probe: use the already-derived torque-ball metric
correction inside the same convergence experiment, with identical criterion/cap
and inputs. The earlier single-wave metric experiment did not answer convergence.
Do not raise the cap, tune tolerances, discard failures or infer physical acceptance
from convergence alone.

Portable protocol, patches, driver/analysis sources, comparator source, all failed
receipts, hashes and results: `benchmarks/2026-09-29-rain-wave-convergence.json`.
Raw artifacts: `desktop-baseline/rain-wave-convergence/`.
Normal rebuild84954 exits0. Original shader restored byte-exact; release archive
SHA256 remains b3f2b64239d72c95c8e1746f9bd40a95152fb7a2c6ab373a6e2bd2986cf5265b.
No production changes or live process remain; GPU-native stays opt-in.


### Metric motor projection within complete-wave convergence (2026-09-29)

The frozen follow-up changes only the previously controlled spherical motor
metric projection relative to the failed adaptive-wave diagnostic. Original input,
1e-5 velocity/cache threshold, two consecutive quiet waves, and cap128 are unchanged.
Build61583 succeeds; probe15025 child exits0 and driver exits1 for numerical failure.
Initial getters match exactly, all42 output bodies are finite, and all8 pass
records validate. Five passes still fail convergence: all four relaxation passes
and substep2 bias. Relaxation final velocity norms are .078098, .027773, .040634,
.030479; the failed bias cache norm is 1.87407e-5. Thus metric motor projection
alone does not resolve this fixed-point failure. Expansion stops as predeclared.

No production change is adopted. The parent diagnostic's public normal-impulse
accounting limitation still applies; this is not physical acceptance. Original
shader and normal release archive are restored byte-exact before commit.
Portable protocol, patch, scripts, validation and restoration evidence:
`experiments/gpu-physics/benchmarks/2026-09-29-rain-wave-metric-convergence.json`.


### Contact metric projection discriminator (2026-09-29)

Forty-one active contacts in each saved capped state were reconstructed with
prepared inertia. The motor-corrected state has3tangent and9rolling hypothetical
next radial updates increasing their fixed-bound quadratic objective, maxima
4.0305e-5/2.30996e-6J. Metric minimizers do not increase it. The prepared tangent
inverse agrees with reconstructed mass to5.50e-7. These are end-state probes,
not actual row inputs or proof of nonconvergence causation. One old tangent cache
is outside the reconstructed bound by float rounding.256float32 disk/ball controls
agree with an independent eigensolver within2.35e-7relative solution error,
maximum bound excess6.43e-8.

The artifact candidate replaces tangent/rolling projections only, atop the prior
motor/wave arm. Original friction coefficients and normal-dependent bounds remain;
tolerance1e-5, two quiet waves and cap128 are frozen. Build56685 exits0. Probe19284
child0/driver1 again reports5nonconverged passes. All four relaxation passes hit
128with velocity changes .075682,.045506,.048816,.032721; substep2bias retains
1.99569e-5cache change. Exact initial getters and all42finite output bodies pass.
Expansion stops; no physical acceptance or production adoption. Normal build66786
and source/archive restoration are byte-exact. The repeated-wave public impulse
accounting limitation remains unresolved. Portable receipts:
`benchmarks/2026-09-29-rain-contact-metric-audit.json` and
`benchmarks/2026-09-29-rain-wave-contact-metric-convergence.json`.


### Slow-wave localization with a verified no-op capture (2026-09-29)

The contact-metric candidate was repeated with only16predeclared checkpoints and
quantized maximum-update labels written to five additional unused filter fields.
No numerical solve or stopping rule changed. Build26428 exits0; probe67647 child0,
driver1 preserves the same5of8nonconverged passes. The body stream is byte-exact.
Every captured semantic state field matches the parent except declared filter
carriers under the existing occupied-order audit; two corruption controls reject.
Normal restoration85691 returns the exact baseline archive.

Final relaxation history is still decaying: velocity update .0627774 at64waves,
.0450119 at96, .0376403 at112, .0327210 at128. Cache changes respectively
.0051070,.0047658,.0026504,.0024213. This is insufficient convergence within
the fixed cap; it does not establish an infinite-time cycle. Hinge39 (40/41)
dominates checkpoints64,112,120,124,128; ground contact1/41 dominates96. Labels
rank changes at1/32767resolution; the recorded norms use the original exact max.

Independent isolated hard hinge alignment/point matrices have Gauss-Seidel
spectral radius .0970 for39 and .1282 for27, maximum .1598 across12hinges.
Their internal two-block splitting therefore does not explain the observed slow
network decay by itself. This argues against blindly adding the previously rejected
local hinge block. A network-level conditioning/reference solve is the next
discriminator; no physical defaults or acceptance limits change.

Portable history, comparison, patches and scripts:
`benchmarks/2026-09-29-rain-wave-convergence-history.json`. The related final-state
joint rankings and isolated hinge matrices are included in
`benchmarks/2026-09-29-rain-contact-metric-audit.json`. An initial diagnostic assertion
that old motor caches are exactly feasible failed at4.08e-10norm excess; that
analysis failure is preserved and signed objective gaps/feasibility excess are
reported explicitly. None of these diagnostics qualifies Rain or the full solver.


### Coupled relaxation reference and rejected acceleration (2026-09-29)

A fixed-geometry float64 model reconstructs all39active joints and41contact
manifolds (56normal points),413blocks and734impulse variables from the last
contact-metric capped phase. Prepared step-start inertia, final joint geometry,
prepared contact levers, spring softness, targets, speculative biases and original
normal-dependent friction/rolling/twist bounds are retained. Tiny captured inertia
asymmetry is symmetrized and delta poses reconstructed; this is explicitly not a
bit-exact GPU replay. Normal/tangent prepared mass identity errors are below
5.50e-7. Twenty-four directional generalized-Jacobian checks pass at1.38e-9
relative error (<1e-5limit).

The generic LM solve12723 exits1 under the declared checks despite optimizer
xtol success: natural residual5.20965e-4, full-sweep velocity change .0481242.
Structured solve8229 also exits1 after32outer iterations/58QP calls, with natural
residual7.81915e-6 and sweep velocity8.09684e-5. It solves fixed-bound convex
problems, then their normal-bound fixed point; it does not substitute associated
friction-cone normal equations. Every AlmostSolved inner status and independently
measured residual is preserved. Neither failed solver is relabelled a pass.

A predeclared finite hybrid refinement33206 succeeds after one full Newton step:
SVD minimum-norm correction, rank730/734, linear residual1.65e-17, no backtracking.
The final natural residual is4.81514e-11 (limit1e-8), scaled domain feasibility
4.34260e-11 (limit1e-8), and direct primal/assembled agreement8.91e-15
(limit1e-9). A full metric block sweep changes velocities<=1.48475e-9 and cached
impulses<=7.70e-11 (both limits1e-5). Thus the reconstructed coupled phase has a
verified fixed point. This does not establish physical Rain acceptance or a
production algorithm. Reference/capped velocity maximum component difference
is .0571418; maximum impulse difference .835058, largest absolute impulse5.00817.

A separate bounded projected Anderson prototype62240 fails all four arms at128
complete-wave evaluations. Captured native color/row order, paired normal blocks
and convex/mesh friction order are reconstructed. Original domains are projected
and affine body velocities reconstructed consistently. Two starts (cold and
saved capped), memory4, weighted least squares, one candidate per state and
0.9residual-reduction acceptance were frozen in advance. Cold acceleration
accepts7/rejects60proposals; capped accepts0/rejects63. Final natural residuals:

| Reconstructed start | Plain waves | Projected Anderson | Outcome |
| --- | ---: | ---: | --- |
| Cold | .004037 | .006566 | Both fail |
| Saved capped | .000991 | .001286 | Both fail |

These are not actual original phase-entry starts and do not qualify any GPU
phase. No accelerator is adopted or parameter sweep started. The next candidate
needs a coupled solve/preconditioner; another isolated hinge correction is not
supported by the current evidence.

Portable receipt `benchmarks/2026-09-29-rain-coupled-reference.json` embeds the
compressed full input frame, scripts, frozen protocols, every result/failure and
reference arrays. Reconstruction solely from that receipt in a temporary
directory reproduces residual/sweep checks and has zero difference from stored
reference velocities, using NumPy without the optimizers. Source/submodule and
normal release archive remain unchanged. All jobs are terminal. Broader physical,
ordinary/native, final-repeat, performance and recording gates remain open.


### Matrix-free coupled prototypes and exact bilateral reduction (2026-09-29)

Twenty-four constraint-local/body-response Jacobian and adjoint controls agree
with the assembled natural-map derivative: forward1.43e-14, adjoint7.43e-15,
local-preconditioner6.44e-15. Dropping softness or normal-bound derivatives fails
independent controls. Four near-null directions at the verified root redistribute
motor/limit impulses, mainly joints9/13/35/41, with body velocity effects below
9.04e-14. The smallest retained singular value is6.44527e-5, largest24.851.
An initial Python import failure from naming a prototype operator.py is preserved;
renaming it nk_operator.py changed no numerical equations.

The fixed matrix-free Newton/GMRES prototype87145 fails its original line search:
cold11steps/532Jacobian actions, capped2steps/266actions. Final natural residuals
.0124091/.00171665 and colored-wave velocity changes .0643191/.0321590 fail.
Diagnostics31992/34804 confirm the small-step directional derivatives, while
exact SVD Newton directions also fail all8original Armijo sizes. Failed-state
Jacobian rank731 and negligible unreachable RHS (<5.76e-14relative) rule against
an inconsistent linear system as the explanation. These are diagnostic evaluations,
not expanded line-search acceptance or a rewritten failed receipt.

A separately frozen trust-controlled shifted-step prototype70480 also fails.
The shift only limits numerical steps; final tests use original equations. Both
arms reach32accepted-step cap. Cold:42linear trials/950actions, residual .00109806,
wave velocity .0228041. Capped:40trials/1733actions, residual5.31593e-5, wave
velocity .00208634. No damping schedule is tuned or GPU candidate adopted.
Complete controls, failures, direction evidence and sources are preserved in
`benchmarks/2026-09-29-rain-newton-krylov-prototype.json`.

Exact block elimination then removes234unconstrained bilateral variables
(point, alignment and springs) and leaves500bounded/unilateral unknowns.
For H=J M^-1 J^T+Gamma, recover lambda_f=-H_ff^-1(g_f+H_fr lambda_r) and use
the Schur complement for remaining rows. Original Gamma is preserved; softness
is not recalculated from the reduced masses. H_ff is SPD: minimum eigenvalue
.0137474, condition482480. Twenty-four algebra controls have maximum free-row/
reduced-gradient/velocity errors9.10e-13; the independent root is recovered within
2.83e-15, retaining original natural residual4.82e-11. All local reduced blocks
are full rank in this fixture.

The plain reduced-wave prototype25522 still fails its128wave budget from both
starts: cold/capped natural residuals .00473656/.000422357, original colored-wave
velocity changes .0382096/.00430913. Domain feasibility remains within8.61e-17.
Thus elimination is verified but is not sufficient by itself. Serial captured
color/row ordering is used for this reference only: reduced impulses affect an
entire articulation, requiring a different GPU ownership strategy. No parallel
safety or speed claim follows from these CPU algebra results.

Portable algebra, protocols, input dependency, states and scripts:
`benchmarks/2026-09-29-rain-bilateral-schur-prototype.json`. Both new receipts
reference the hashed self-contained coupled-reference bundle. The next algorithm
must solve the reduced coupled system while passing the original full equations.
All processes are terminal; normal archive and production sources are unchanged.
Physical, actual-phase GPU, ordinary/native, repeat, performance and recording
requirements remain open.


### Reduced coupled references (2026-09-29)

Dense trust-region reference53832 exits1. Its24directional Jacobian controls
pass (2.42e-9relative), but both cold/capped starts fail original full criteria.
Cold reaches120evaluations with residual .0175937 and colored-wave velocity
change .729374. Capped terminates on xtol after52evaluations, residual .000406218,
wave velocity .0872617. Optimizer success does not establish convergence.

The next frozen reference26361 uses the same exact234variable elimination and
original Gamma with fixed-bound convex QPs, normal-bound fixed point and full
semismooth Newton refinement. It preserves nonassociated normal equations.
Budgets match the earlier full structured reference:32outer/128QP calls,
100inner iterations, Anderson memory5; then8Newton steps and8Armijo sizes.
Cold and capped starts require33/58QP calls and two Newton steps each.
Both structured stages retain NoConvergence; only the complete hybrid passes.

| Start | Original natural residual | Scaled feasibility | Colored-wave velocity change | CPU reference seconds |
| --- | ---: | ---: | ---: | ---: |
| Cold | 5.26e-14 | 5.96e-14 | 2.55e-12 | 5.66 |
| Capped | 5.62e-16 | 8.92e-17 | 1.49e-14 | 9.34 |

Independent reconstruction exclusively from portable receipts reproduces all
original acceptance checks and saved velocities exactly. Two deliberately altered
impulse states reject. Cold/capped resulting velocities differ by6.51e-13 despite
redundant multiplier directions. Quantizing only the saved caches to float32 gives
natural residual1.60e-7, feasibility3.38e-8 and wave velocity<=5.36e-7 with float64
arithmetic; this fails the strict reference1e-8test and is not float32/GPU evidence.
No numerical tolerance or physical acceptance screen changes.

Portable receipts `benchmarks/2026-09-29-rain-reduced-trust-reference.json` and
`benchmarks/2026-09-29-rain-reduced-structured-reference.json` preserve protocols,
sources, outcomes, states and hashed parent input. Production sources/submodule
and normal archive are unchanged. This closes a reconstructed-phase numerical
reference gate from two starts, not actual phase-entry fidelity, GPU parallel
ownership, physical trajectory, ordinary/native repeat or performance gates.
The seconds-long CPU optimizer is not a production algorithm. Next implementation
must preserve coupled constraints with explicit articulation ownership, validate
float32 arithmetic and actual phase inputs, and retain the full physical matrix.


### Actual next-wave reference fidelity and separation precision (2026-09-29)

A diagnostic witness executes one final relaxation wave after the unchanged
128-wave failed convergence report. It is not an expanded convergence budget.
Build97223/probe60349/normal86909 complete0. Initial getters, body poses and
all eight original convergence reports match. Comparing734impulse/258velocity
components against the endpoint float64 reconstruction fails the predeclared
normalized2e-5fidelity screen: impulse3.328e-4, velocity .00149316.
Localizing with preceding actual cache increments gives affine velocity agreement
4.793e-7, but two scalar contact normals (15/19 and19/30) differ markedly.

These contacts are near68m in world coordinates. Subtracting rounded final and
initial positions loses small displacement precision and changes speculative bias.
A second artifact captures56exact pre-wave normal separations into eight unused
fields of three filter joints. Build7773/probe62246/normal34696 complete0. Body
output is byte-exact to the first witness; every semantic state field matches
except those declared fields under the existing occupied-membership audit.
Raw occupied-order variation is preserved, and deliberate body corruption rejects.
An initial analysis-only chain_length==1 assertion failed: standalone convex
manifolds export0. It was corrected to<=1 plus no child identity, retaining failure.

Replacing contact biases with captured max(separation,0)/dt reduces maximum
impulse/velocity errors to1.2325e-5/2.6977e-5. The original2e-5fidelity screen
STILL FAILS. The previous endpoint-model root has corrected-equation residual
.000468122, so its internal convergence does not qualify the actual GPU phase.
With actual prior impulses, all local rows now agree within5.63e-6; affine
velocity agreement stays4.80e-7. Remaining largest rows are joint29motor,
hinge37upper and joint20motor. Float64 metric minimizers can differ from the
shader's float32 safeguarded steps; exact local geometry/prepared masses also
need faithful evaluation. Do not broaden the fidelity limit or infer a production
solver defect from the remaining reconstruction error alone.

The conditionally prepared corrected coupled-reference rerun was not launched.
Both portable receipts retain full compressed witnesses, patches, sources,
protocols, failures, dependency hashes and exact normal archive restoration:
`benchmarks/2026-09-29-rain-reference-wave-fidelity.json` and
`benchmarks/2026-09-29-rain-reference-exact-separation.json`. No production edit
is retained. Next use the saved witness to implement faithful float32 local replay
before another coupled GPU candidate. All wider qualification gates remain open.


### Exact pose and deterministic joint-angle reference corrections (2026-09-29)

A bounded hybrid model replaces ideal ball projections with24-bisection float32
projection/clamp/objective safeguarding.24SPD controls pass with maximum solution
error2.49e-7, but whole-wave fidelity fails: velocity7.60e-5, impulse1.23e-5.
Actual-prior local error stays5.63e-6. This partial cast is not a complete float32
shader replay; no parameter retry is justified.

One-step diagnostic build52559 retains dp/dq after apply_deltas while still
computing normal published poses, flags and sleep metrics. Probe52424 and normal
rebuild61040 complete0. Physical output bytes match the exact-separation witness,
and all audited state matches except retained body dp/dq and their explicitly
verified identical host mirrors. Initial comparison omitted duplicate host fields;
failed-analysis-v1 is preserved. A second step with unreset deltas is prohibited.
Source and normal archive restore exactly. No production behavior is changed.

Reference levers now follow two float32 rotations; frames use prepared frame then
substep quaternion multiplication. This alone leaves velocity mismatch2.69661e-5
and impulse1.20838e-5, failing the original2e-5fidelity criterion. Source inspection
then finds a separate model defect: Python libm atan2 differs from the engine's
b3Atan2/gyro_atan2 deterministic polynomial. The corrected scalar float32 polynomial
matches the original C implementation bit-for-bit on263axis/quadrant/seeded cases;
maximum difference from libm is2.76566e-5rad. Hinge37upper local error falls from
5.50e-6 to4.11e-9. The prior geometry-only model/result remains preserved.

With both corrections, whole-wave impulse error is7.91241e-6 but velocity error
remains2.69661e-5, still FAIL. Actual-prior local error is5.63e-6 (motor29), with
affine velocity agreement4.68e-7. The next discriminating work is complete native
float32 inertia/body-update/motor-objective arithmetic using the saved exact inputs;
additional capture is not needed by present evidence. No corrected root solve or
GPU algorithm is qualified by these results. Limits and broader gates are intact.

Portable protocols, sources, compressed capture, original failures, C polynomial
controls and dependency/restoration hashes:
`benchmarks/2026-09-29-rain-reference-float32-projection.json` and
`benchmarks/2026-09-29-rain-reference-exact-pose.json`.


### Native float32 update replay and objective cancellation (2026-09-29)

Using saved exact geometry, CPU native-float32 tensor construction and body-update
order improves actual-prior motor replay:6of27motors bit-exact, maximum relative
motor error3.54e-6, final velocity error4.72e-7. Stored accumulated-cache differences
cannot recover every applied impulse: cache half-ulp uncertainty alone propagates
to2.36e-6absolute velocity, excluding other arithmetic errors. This is diagnostic
input substitution, not free-running prediction or determinism qualification.

The separate predicted wave computes raw impulses and applies them before rounded
cache storage where the shader does. It uses original prepared masses, exact
separations and corrected joint angle polynomial. It still fails the predeclared
2e-5fidelity bound: maximum impulse7.91e-6, velocity2.69732e-5. No limits change;
some bias/Jacobian setup remains mixed precision, not whole-shader bit conformance.

An independent float64 quadratic audit evaluates the exact float32 candidates,
old caches and tensors at reconstructed local inputs. Comparing rounded total
objectives falsely rejects2improvements and accepts5tiny increases. Motor29's
rounded difference+4.66e-10 has exact quadratic delta−2.91e-10; motor18 gives
+7.45e-9 versus−5.64e-9. Direct and delta-form double algebra agree within1e-14.
This demonstrates cancellation for those inputs; it is not proof of the actual
GPU branch inputs or the cause of Rain's large coupled residual.

The next bounded correction is the algebraically identical delta objective,
 d·(cdot+K(feasible_old−old))+0.5 d·K d. It needs independent near-stationary,
conditioning and original-domain controls before any GPU candidate. No physical
parameters, convergence budget or acceptance thresholds are changed. Production
source and normal archive remain untouched. Portable protocols/sources/results:
`benchmarks/2026-09-29-rain-native-float32-updates.json`.


### Direct objective-change safeguard discriminator (2026-09-29)

Both metric-ball safeguards are changed only in an artifact candidate to compare
 d·(cdot+K(feasible_old−old))+0.5d·Kd directly.512seeded2D/3D controls cover zero
bounds, infeasible old caches, near-stationary inputs and condition up to1e6.
Independent float64 quadratic checks pass the predeclared32eps operation-scale
bound and4eps domain-bound excess.487decisions lie outside uncertainty; the25
remaining are explicitly unresolved. Maximum error/bound .04572. An initial report
serialization error on NumPy scalars is retained; JSON conversion alone was fixed.
Full WGSL parses and validates; candidate build21800 exits0.

Original hip one-step probe34272 exits child0/driver1. Initial getters match and
all42body outputs are finite, but5of8phases remain nonconverged under unchanged
1e-5velocity/cache, twoquiet and128cap. All four relaxation phases hit128 with
velocity changes .0756853/.0455106/.0488159/.0326764; substep2bias cache change
1.92216e-5 also fails. Expansion stops; no native or wider physical acceptance.
Normal rebuild75557 exits0 and restores the exact baseline archive. Source and
submodule are unchanged.

The numerical cancellation correction does not resolve the large coupled residual.
It is not adopted and does not justify more local safeguard/iteration sweeps.
Corrected coupled-system/reference work remains necessary; the earlier2e-5whole-
wave fidelity failure stays unresolved. Portable controls, preserved serialization
failure, patch, run scripts/manifest/phase failures and restoration evidence:
`benchmarks/2026-09-29-rain-delta-objective.json`.


### Corrected coupled numerical target and rejected fixed-bound ADMM (2026-09-29)

The corrected exact-separation/dq/polynomial model passes24directional derivative
checks (1.85e-9) and24Schur controls (1.82e-12gradient). The unchanged structured
QP/normal fixed-point/Newton budget converges from both starts. Cold33QP calls
plus2Newton steps gives full natural residual7.78e-15, feasibility8.70e-15,
colored-wave velocity4.45e-13. Capped48QP calls plus1Newton step gives1.63e-12,
1.72e-12 and5.47e-11respectively. Intermediate NoConvergence stages are retained.
CPU times5.85/7.94seconds are numerical reference cost. Independent NumPy-only
reconstruction solely from portable receipts reproduces stored velocities exactly,
rejects2impulse corruptions and finds cold/capped velocity difference5.39e-11.
This numerical target does not close the failed2e-5GPU next-wave fidelity screen,
actual-phase physics, float32 or ownership gates; none is promoted to a pass.

A separate fixed-bound inner-QP ADMM prototype reuses Cholesky(S+D), choosing D
from existing local effective-mass scales, with no overrelaxation/adaptive penalty.
All24linear-system controls pass. Under the frozen1024iteration/1e-8residual
criteria, cold fixed bounds converge in426iterations (natural9.82e-9), while
captured fixed bounds fail at1024(natural3.32308e-5, primal5.69e-6). Domain
feasibility remains9.57e-17. Same-bound independent Clarabel solves confirm cold
objective gap−1.26e-13and velocity difference1.05e-7; capped gap1.38412e-5and
velocity difference .000650554. Capped Clarabel AlmostSolved/fixed-bound residual
2.11e-8 are reported explicitly. No outer/GPU expansion or parameter sweep.

Portable protocols, source, failed configurations, complete numerical states and
verification: `benchmarks/2026-09-29-rain-corrected-coupled-reference.json` and
`benchmarks/2026-09-29-rain-reduced-admm.json`. Production sources and normal
archive remain unchanged. The next coupled method should avoid repeated expensive
fixed-bound QPs while preserving nonassociated normal equations and original
final acceptance; no broader solver qualification is claimed.


## Rain interior continuation numerical reference (2026-09-29)

The original twelve-Newton-solves-per-stage continuation fails both starts in its
first stage (natural residuals0.607219/0.590548). Its failure is preserved in
`benchmarks/2026-09-29-rain-interior-continuation.json`, including the initial
zero-radius initialization failure. Twenty-four Jacobian controls pass3.57e-7;
omitting normal-dependent radius derivatives fails0.5334.

One explicitly recorded scheduling revision shares the same156total solve budget
across stages. Cold/capped need130/133 dense barrier solves and6.60/6.74seconds CPU.
Both barrier schedules stop on line-search failure; one original-equation Newton
polish per arm then passes natural2.44e-13/2.46e-15, feasibility8.20e-14/8.20e-16
and reconstructed-wave velocity7.95e-11/8.00e-13. The hybrid succeeds; continuation
alone did not finish. Receipt-only NumPy reconstruction verifies both saved
solutions and rejects two corruptions. Cold/capped velocities differ9.80e-11.
Evidence/source: `benchmarks/2026-09-29-rain-interior-global-budget.json`.

This establishes a second route to the corrected numerical reference. It does
not close actual GPU-wave fidelity, physical acceptance, float32 feasibility,
GPU ownership, repeatability or efficiency. Production sources are unchanged.


### Direct float32 barrier port rejected (2026-09-29)

The previous turn progressed by verifying the portable shared-budget numerical
reference. This turn tests a necessary condition for GPU use: float32 solves of
every scaled barrier Jacobian along the unchanged successful float64 path,
without feeding diagnostic directions back into that path. Session99808 exits0;
both saved float64 solutions/velocities remain bit-identical to the originals.
The predeclared normalized linear-residual1e-5 screen fails16/130 cold and23/133
capped directions. First failures occur at mu1, step0 (6.16e-5/1.85e-5); worst
cold residual4.29e9, worst capped1.85e-5. All directions are finite. Rounding
accepted states loses strict interior20/129 and27/132 times, first at mu1e-9
step2 on both arms. This rules out the unchanged direct float32 port; it is not
a failure of every possible coupled method.

Rounding final impulses alone produces natural1.35e-7 and feasibility1.72e-8
on both arms, failing the original1e-8 numerical-reference gates. Reconstructed
wave changes still pass the independent1e-5 stopping screen: velocity7.13e-7/
5.16e-7, cache1.70e-7. Preserve this distinction; do not weaken numerical gates
or claim actual GPU fidelity from it. Receipt includes protocol/source/all263
directions and261rounding checks:
`benchmarks/2026-09-29-rain-interior-float32-audit.json`.

Normal archive hash and production source cleanliness reverified; no live job.
Next implementation work should avoid late-stage strict-interior subtraction
in float32. Use the converged reference to assess an active-set or proximal
coupled formulation with explicit articulation ownership; do not port the dense
barrier routine unchanged or begin precision/iteration parameter sweeps. All
physical/final-state-repeat/performance/recording gates remain in the full goal.


### Articulation factorization and ownership (2026-09-29)

Previous turn progressed by ruling out the unchanged float32 barrier port.
Session82969 completed0: bilateral dynamic-body support partitions the234free
rows into three78row articulations, each14dynamic bodies. Fixed ground does not
join them. All cross-component Hff entries are exactly zero; deliberately
injecting a cross-component coupling is rejected. Independent component float64
Cholesky matches the global reference in26cases (24seeded reduced states plus
both final roots): normalized free residual1.66e-14, velocity difference8.22e-14.
Dense factor storage falls54756→18252entries and cubic work proxy12812904→1423656.
These are structural counts, not measured GPU savings.

Remaining constraints include271blocks owned by one articulation and52spanning
two. Thus original endpoint-body colors cannot safely schedule condensed
articulation updates. This is concrete ownership evidence for any future coupled
solver. Unscaled float32 Cholesky fails2/26 screens (max velocity error2.22e-5);
diagonal-equilibrated float32 fails4/26 (max residual1.14e-5, velocity2.95e-5).
Both reference endpoints pass1e-5, but the predefined random controls remain
failed. Source/results: benchmarks/2026-09-29-rain-articulation-factorization.json.

One square-root alternative avoids explicitly forming the normal matrix in
float32: QR of transpose [Jfree sqrt(M), sqrt(Gfree)] with float32 triangular
solves. Session23580 completed0; float64 assembly matches Hff7.73e-12. It fails
6/26 unchanged screens, max residual1.62e-5 and velocity3.89e-5. Original RHS
formation and velocity reconstruction remain float64 diagnostic references, so
this is not a complete float32 algorithm. Both final endpoints pass, but no
variant is promoted based on selective endpoints. Source/results:
benchmarks/2026-09-29-rain-articulation-square-root.json.

No production edit or live job remains. Exact articulation decomposition is now
established for this fixture; approximate float32 factors may still serve as
preconditioners in a residual-corrected coupled method, but are not demonstrated
exact replacements. Next: formulate bounded coupled correction using articulation
ownership and original residual checks, avoiding barrier boundary cancellation.
Do not start a factorization/precision parameter sweep. Full physical, actual GPU
fidelity, independent-path repeat, performance and recording gates remain open.


### Residual correction and projected continuation (2026-09-29)

Previous turn progressed by establishing articulation ownership and preserving
failed float32 factor variants. This turn's one-correction inverse-action check
(session22314, exit0) uses the same unscaled Cholesky factors plus float32
Jfree*(M*(Jfree.T*x))+Gfree*x residual and float32 update. It still fails3/26
unchanged cases (max residual1.34e-5, velocity1.65e-5), versus2/26 before
correction. No additional corrections or precision sweep. RHS formation remains
float64 cast once; no complete float32 algorithm claim. Evidence/source:
benchmarks/2026-09-29-rain-articulation-correction.json.

A different bounded nonlinear method avoids strict-interior barriers:
R(y)+lambda*(y-seed)=0, original projected natural map with seed fixed per arm;
lambda1,.1,.01,.001,.0001,.00001,.000001,0, at most16Newton solves per stage,
eight Armijo sizes, least-squares rcond1e-12. Temporary numerical term vanishes
at the required final stage; original Gamma/domains/final gates unchanged.
Session72001 completed1 overall. Cold fails the lambda.001 stage budget after
34total Newton solves: natural7.19e-4, reconstructed wave velocity.01915/cache
.001979. Preserve this failure; no stage-budget retry.

Captured-capped start completes all eight stages in38Newton solves,6.95seconds
CPU. Final original natural2.03e-14, feasibility2.27e-14, primal5.06e-15, wave
velocity1.18e-12/cache2.90e-14. Independent receipt-only NumPy reconstruction
verifies this endpoint, matches saved velocities exactly and rejects an impulse
corruption. Overall method remains unqualified because cold fails. Receipt/source:
benchmarks/2026-09-29-rain-projected-continuation.json.

This is a barrier-free route from the captured state to the numerical root, not
a GPU implementation, a full physical pass or a universal initialization result.
Next bounded implementation step: assess iterative/matrix-free linear solves on
this successful continuation path with original residual checks and articulation
ownership; do not erase the cold failure or increase its budget. No production
changes or live processes remain; all broader acceptance gates remain intact.


### Matrix-free continuation directions (2026-09-29)

Previous turn progressed by independently verifying capped projected continuation,
with its cold-start stage failure preserved. Frozen-path Krylov audit48173 exits0:
all38Newton systems pass the original unpreconditioned residual screen using
GMRES restart32, fourcycles, rtol1e-6/atol1e-12 and actioncap160. Total1612actions,
maximum71per system. Twenty-four Schur action controls agree1.81e-13; every
Jacobian action matches assembled D within1e-9. Replay endpoint is bit-identical
to the original dense path. Source/results:
benchmarks/2026-09-29-rain-continuation-krylov.json.

Action uses J M J^T+G and three bilateral component solves; local projection and
normal-dependent-radius derivatives are sparse. Remaining-row owner tuples
partition preconditioner blocks into117,135,157,35,56rows. Their pseudoinverses
still come from dense diagonal blocks of assembled D; no GPU setup/timing claim.

Actual driven solve48179 completed1 overall with unchanged nonlinear schedule,
stage budgets, line search and final gates. Captured-capped completes38Newton
steps in7.00seconds CPU, natural2.04e-14, feasibility2.28e-14, primal4.92e-15,
wave velocity1.17e-12/cache2.91e-14. Receipt-only NumPy reconstruction independently
verifies the endpoint, stored velocities and impulse-corruption rejection.
Cold stops on a linear-budget failure after19accepted Newton records, natural
.004251, feasibility.002045, wave velocity.06872/cache.009272. Across both arms
2589operator calls, maximum132, one linear failure. Original cold dense stage-cap
failure remains separate. No tuning or budget retry. Source/results:
benchmarks/2026-09-29-rain-continuation-krylov-driven.json.

This removes dense global Newton least-squares from the successful capped path,
but remains float64 CPU with dense local preconditioner setup and an unqualified
cold start. Next implementation gap: float32 operator/direction accuracy and
preconditioner construction/cost on the actual driven path, before GPU adoption.
Do not equate the captured-phase root with Rain physical acceptance. No production
source edit or live process remains; full independent-path physical/repeat,
performance and recording gates are still open.


### Float32 continuation precision screens (2026-09-29)

Previous turn progressed by driving the capped continuation with matrix-free
float64 directions and independently verifying its endpoint. Two predeclared
float32 diagnostic formulations now preserve that driven trajectory exactly.

Condensed experiment88283 exits0: all38 random Jacobian action screens pass1e-5,
maximum8.69e-6, but26/38GMRES directions fail the unchanged original float64
linear-residual limit max(1e-12,1e-6*RHS norm), with restart32/fourcycles/cap160.
J/M/G actions and bilateral Cholesky are float32; projection derivatives/active
branches and local pseudoinverses were prepared in float64 and cast. Thus even
this partial float32 path fails, not a demonstrated complete float32 solution.
Receipt: benchmarks/2026-09-29-rain-continuation-float32.json.

Uncondensed experiment92073 exits0: keep all734rows, free rows scaled by reciprocal
Hff diagonal, bounded rows use identical natural projection derivatives/steps.
Original elimination reproduces the500row Jacobian within1e-9 at every state.
Float32 action has no bilateral inverse, but27/38GMRES directions still fail the
same residual screen. Owner-tuple preconditioner sizes195,213,235,35,56; local
pseudoinverse setup remains float64 cast. No parameter retry or physical equation
change. Receipt: benchmarks/2026-09-29-rain-continuation-full-system.json.

Both saved float64 replay endpoints are bit-identical to the prior driven
endpoint. Diagnostic runner exits0 mean completed audits, not float32 success.
No production change/live process remains. The verified reference is retained;
neither direct float32 formulation qualifies under the predeclared linear screen.
Next work must address numerical error in the coupled correction, not increase
Krylov budgets or relabel these failures. Distinguish the strict intermediate
linear screen from the original nonlinear/physical gates; no inference that all
possible inexact coupled methods are ruled out. Full goal remains unchanged.


### Inexact continuation reaches both numerical roots (2026-09-29)

Previous turn progressed by preserving both float32 linear-screen failures.
This turn explicitly revises the conservative all-linear-systems-must-pass rule:
finite float32 Krylov outputs become approximate search directions, accepted only
by the unchanged eight-size nonlinear Armijo check. Strict1e-6/1e-12 linear
screens remain failed where applicable; they are not relabeled. Original final
natural/feasibility1e-8,primal1e-9,wave1e-5 and lambda0 requirements are unchanged.
No Krylov tolerance/budget increase. Initial source-extraction SyntaxError occurred
before numerical controls; source/log preserved and extraction corrected.

Session42775 completed1: with16steps/stage, cold still hits lambda.001 cap after
34Newton steps (natural7.19e-4,wave velocity.01915). Capped passes38steps with
natural2.21e-14, feasibility2.50e-14,wave velocity1.19e-12. Independent receipt-only
verification passes capped and rejects corruption. Original linear screens fail
22/34cold and26/38capped directions. Evidence:
benchmarks/2026-09-29-rain-inexact-continuation.json.

One documented scheduling revision shares the SAME128maximum Newton attempts
(8*16) across stages. It revises the earlier no-stage-budget-retry plan because
cold's late stage history shows residual decrease and accepted full steps,
without an established stall. No schedule/arithmetic/line-search/final threshold
change. Session65664 completed0. Cold needs52Newton steps, including18at lambda
.001,15.54seconds CPU; capped38steps,10.78seconds. Cold natural/feasibility6.10e-11,
wave velocity1.68e-10/cache6.10e-11; capped unchanged2.21e-14/2.50e-14,
wave velocity1.19e-12/cache3.32e-14. Both complete lambda0.

Independent receipt-only NumPy reconstruction verifies both final states, matches
saved velocities exactly, rejects two impulse corruptions, and finds cold/capped
velocity difference2.01e-12. Strict intermediate linear screens STILL fail35/52
and26/38directions;4629/3180operator actions. This is successful inexact nonlinear
iteration, not improved linear-inverse accuracy. Receipt:
benchmarks/2026-09-29-rain-inexact-global-budget.json.

The first barrier-free both-start numerical result with float32 directions is
now established for this captured phase. Nonlinear state/residual and local
preconditioner SVD setup remain float64; no full float32 or GPU claim. Next:
remove dense SVD setup and assess float32 outer-state/residual behavior, keeping
the numerical-reference gates distinct from the existing runtime wave stopping
screen. Actual GPU fidelity, Rain physical acceptance, both-path full-state
repeats, performance and recordings remain open. No production change/live job.


### Float32 LU replaces preconditioner SVD (2026-09-29)

Previous turn progressed by independently verifying both roots with inexact
float32 directions. Session52842 now completed0 with no dense pseudoinverse
preconditioner setup in the Newton loop. Each owner-tuple diagonal block is cast
to float32 and factored with partial-pivot LU after adding sigma I ONLY to the
preconditioner, sigma=n*eps32*max(norm_inf(D32),tiny32). Recorded shifts range
.000122049→.003193923. This numerical formula is fixed, not tuned compliance;
original operator/residual/Gamma/final gates remain unchanged. Factors and solves
are checked float32/finite. No pivot failure or budget retry.

Both starts pass: cold52Newton steps/14.68seconds, captured38steps/10.44seconds.
Cold natural/feasibility6.13e-11,wave velocity1.68e-10/cache6.13e-11; captured
natural2.51e-13,feasibility1.14e-14,wave velocity1.98e-12/cache1.57e-13. Independent
receipt-only reconstruction verifies both, matches saved velocities exactly,
rejects two corruptions, and finds endpoint velocity difference3.42e-12.
The original strict linear screen fails63directions across the90attempts;
those failures remain explicit under the unchanged inexact nonlinear policy.

Source/protocol/states/results:
benchmarks/2026-09-29-rain-inexact-lu.json. CPU timings are diagnostic, not controlled
production speed evidence. Preconditioner block entries still come from assembled
float64 D before casting; nonlinear state/residual and projection setup remain
float64. Next: assemble local preconditioner blocks directly from float32
projection derivatives and reduced operator blocks, avoiding full float64 D as a
setup dependency, then address outer-state/residual precision. Production GPU,
actual-wave fidelity and full Rain/independent-repeat/performance/recording gates
remain open. No production edit or live job.


### Direct float32 local setup passes; derivative extension fails (2026-09-29)

Previous turn progressed by replacing SVD preconditioning with verified float32
LU. Local-preconditioner session7915 completes0: reduced preconditioner columns
and affine term formed via float32 Schur/free solves; projection branches/radius
derivatives and local matrix assembly all float32. No assembled D64 dependency
for preconditioner setup. Original nonlinear equations, shifted-LU formula,
budgets and final gates unchanged. Twenty-four states/all5owner tuples give120
float64 algebra controls, max9.74e-17. Initial source-extraction82849 exit1 after
controls is preserved; only source extraction changed before rerun.

Cold52/capped38Newton steps pass (15.45/10.91seconds CPU). Natural2.76e-12/1.65e-13,
feasibility3.08e-12/2.87e-14, wave velocity1.60e-10/2.65e-12. Independent receipt-only
reconstruction passes both, exactly reproduces saved velocities and rejects two
corruptions. Strict diagnostic linear screens fail61directions and action screen
fails1; these remain failures under the inexact nonlinear method, not inverse
accuracy passes. Evidence: benchmarks/2026-09-29-rain-local-preconditioner32.json.

Derivative extension12466 completes1: operator projection branches, ball/box
derivatives and radius derivatives now also use float32 reduced gradient and
state. Both arms fail original Armijo before completing the required schedule.
Cold52steps: natural1.36e-8,feasibility1.52e-8,wave velocity7.90e-7; it fails strict
numerical checks though wave limit passes. Capped32steps stops at lambda1e-5:
natural2.87e-6,feasibility3.02e-6,wave velocity6.10e-5 (>1e-5),cache4.39e-6.
Original linear screens fail63directions, action screens27. No threshold/budget
retry. Evidence: benchmarks/2026-09-29-rain-projection-derivative32.json.

Current successful baseline is direct float32 LOCAL PRECONDITIONER with float64
operator projection preparation/nonlinear state/residual. Full derivative32
extension is rejected. Next investigate cancellation/branch error in the reduced
gradient used by projection derivatives, preferably an algebraically centered
physical-velocity representation; do not weaken final checks or infer GPU
qualification from the mixed result. Actual GPU fidelity, Rain physical matrix,
both-path full-state repeats, controlled performance and recordings remain open.
No production change or live process remains.


### Centered derivative gradient: cold pass, capped branch failure (2026-09-29)

Previous turn progressed by verifying direct float32 local setup and preserving
the failed full derivative extension. New centered expression uses captured
velocity/old impulses: c=J*vcap+bias+G*old; solve free delta from c+H*delta, then
evaluate the remaining gradient. Session1606 completed1 overall. Float64 identity
passes26states with max2.68e-13; maximum normalized float32 gradient error decreases
7.61e-5→6.82e-5 over random/failed endpoints. At the prior cold/capped endpoints,
error decreases7.05e-6→1.78e-6 and6.01e-6→3.74e-6. No acceptance threshold changed.

Cold now PASSES52steps: natural3.71e-12,feasibility4.14e-12,wave velocity2.18e-10.
Independent receipt-only reconstruction verifies cold, matches saved velocity
exactly and rejects impulse corruption. Capped still FAILS Armijo at lambda1e-5
after34steps: natural2.87e-6,feasibility3.02e-6,wave velocity6.10e-5/cache4.39e-6.
A progress message initially misread this as both arms failing; corrected after
reading complete result. Source/protocol/states:
benchmarks/2026-09-29-rain-centered-gradient32.json.

Branch audit identifies exactly ONE mismatch at the capped failure: joint35motor.
Exact projected argument-0.020833336464 lies outside radius0.020833334420;
float32-0.020833328366 lies inside radius0.020833333954. Gradient error3.95e-8 flips
the derivative from saturated to interior. At the successful cold endpoint there
are3branch mismatches (joint9upper,13motor,35motor), so disagreement alone is not
proof of failure causation; the capped mismatch is a concrete correction target.

Successful both-start baseline remains local-preconditioner32 with float64
operator derivative preparation. Centered derivative32 is only cold-qualified.
Next address numerical branch selection near motor/limit bounds while keeping
hard physical projection and original final checks; do not tune physical limits,
increase budgets or pretend the mixed prototype is GPU-qualified. Broader
physical/fidelity/independent-repeat/performance/recording gates remain open.
No production edit or live job.


### Stored-bound derivative passes both starts (2026-09-29)

Previous turn progressed by verifying centered cold start and locating capped
motor35branch disagreement. Symmetric joint-scalar secant experiment31563 exits1:
nine scalar boundary/interior controls pass, but both solves exhaust128Newton
steps. Cold natural7.19e-4,wave velocity.01915; capped natural.001415,feasibility
.001440,wave velocity.03173. All256direction action/linear diagnostics fail.
No secant-width or budget retry. Preserved source/results:
benchmarks/2026-09-29-rain-scalar-secant.json.

A separate no-tolerance active-bound policy targets the observed stored impulse:
for JOINT BOX derivatives, saturate when either projected argument is outside OR
stored float32 impulse is at/outside the exact float32 bound. Original hard
projection/residual unchanged; this is an approximate Newton derivative. Matching
policy used in local preconditioner; contacts/other domains unchanged. Five
interior/exterior/stored-bound/zero-radius controls pass. Session93387 completes0.

Cold52Newton steps/15.61seconds, capped38steps/11.02seconds pass the original final
gates. Cold natural3.70e-12,feasibility4.14e-12,wave velocity2.17e-10; capped
natural1.03e-13,feasibility5.97e-14,wave velocity3.32e-12. Independent receipt-only
reconstruction verifies both, matches stored velocities exactly, rejects two
corruptions, and finds endpoint velocity difference3.59e-12. Strict direction
diagnostics still fail68linear and21action checks; approximation accepted solely
through unchanged nonlinear Armijo and final criteria. Source/protocol/states:
benchmarks/2026-09-29-rain-bound-state-derivative.json.

Current both-start prototype now has float32 centered projection preparation,
local preconditioner assembly/LU, and Krylov directions. Outer state/residual and
line-search evaluation remain float64; original reference geometry/operators are
prepared then cast. Next address outer-state/residual precision, preserving the
strict reference checks and separately tracking the existing runtime wave
stopping criterion. No GPU/physical/repeat/performance claim; full objective and
all remaining gates intact. No production edit or live job.

### PR01 speculative world controls: functional acceptance, release gates remain open (2026-10-01)

The production-readiness roadmap now owns the continuation queue. Its first
control implementation gates experimental hull–mesh speculative contacts per
world, preserves endpoint flags and default behavior, and routes the combined
setter to both mapped worlds. An actual preceding solid CCD hit retains the
existing 6.25 mm landing support shell. Fast tangent/no-hit gaps remain disabled.
The pinned CPU world flag has no collision consumer; CPU shape flags provide the
independent isolated control. No CPU world-toggle parity is claimed.

The [portable PR01 report](../experiments/gpu-physics/benchmarks/production-readiness/pr01-speculative-2026-10-01/README.md)
retains exact compiled-source/build/library/executable receipts, all fixed
protocols, failures, raw v23 captures and charts. The zero-shell variant tunnels
and is rejected. The broader fast-body guard passes its CCD checks but adds
positive-gap contacts without a hit and is rejected on semantic scope. The
current handoff candidate passes 20 fresh C confirmations, four stricter guards,
ten 110-step raw-state repeats with every-step health, and 63 frozen regressions
per backend. Fourteen storage/comparison controls pass. Schema v23 adds required
world/pending-policy booleans; old schemas remain readable. This is control
fixture evidence, not the final whole-matrix qualification.

Real CPU and native GPU viewers each complete 300 steps of s&box Ghost Collisions
with no ghost launches or unhealthy completion. Both fail an additional 5 mm
floor screen at the same initial landing height, 0.908670902 m on step 6 versus
0.9094 m. Maximum CPU/GPU height difference over the recording is 0.00143975 m;
agreement does not erase the failed screen. The separately frozen one-run
pre-PR01 baseline diagnostic times out during startup after 180 seconds and
receives no qualification credit or selective retry. Offline float32 integration
independently matches both engines’ first six heights and vertical velocities
exactly; the unchanged native/GPU mesh glancing filter leaves this discrete
landing. The added all-window5mm bound is not the separate150m/s normal-impact
fixture’s assertion applied to equivalent motion. Its failure remains retained;
no bound is raised and the original0.495m isolated CCD check remains passing.
The source applicability review closes PR01’s original functional contract,
without introducing a final physical tolerance for this scene. PR03/PR04/PR07
retain complete physical qualification and PR05 owns current runtime/startup
checks. All20standard pre-commit scene pairs are delivered as40portable clips
with compiled-source receipts/hashes; first/last visual review is complete.
No timing, Rain acceptance, default-policy change or production-readiness claim
follows this control milestone.
