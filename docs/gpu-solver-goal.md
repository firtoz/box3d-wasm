# GPU solver goal and recovery scratchpad

Updated: 2026-09-29. Status: resumed by explicit goal continuation, incomplete.
Workspace: `/home/firtoz/work/2026/box3d-wasm`.
Absolute scratchpad: `/home/firtoz/work/2026/box3d-wasm/docs/gpu-solver-goal.md`.
Engine working directory: `experiments/gpu-physics` relative to that workspace.

## Current work and recovery (2026-09-29)

The full goal is resumed. This section owns current process state; older pause,
bounded-task and live-job notes below are historical. Full objective and acceptance
matrix remain below. HEAD is `ff811a6` on `feat/gpu`; origin was fetched. Box3D is
initialized at its recorded clean revision. User-requested stage/commit and charts
completed; clean afterward, six commits ahead of origin. No additional commit or
push requested for this resumed solver work.
User authorizes continued work with unrestricted access and no routine prompts.
Both controlled performance batches are complete and independently verified.
Small session 88184 and large session 89242 exited 0; neither is running.
All 8 prewarms and 40 measured fresh processes validate. No production solver
change was made for these measurements.

### Completed controlled benchmark and charts

Five paired fresh runs per ordinary/native path and scene used 60 warmup plus
180 timed completed steps, alternating order, equal prewarming, identical cache
copies and frozen release binaries, without diagnostic capture or competing GPU
work. Large Pyramid has 5,050 dynamic cubes plus ground, no joints, sleep disabled.
Falling Ragdolls has 116 bodies and 112 joints, sleep enabled. Timing includes
queue completion, pose synchronization and bounded counters; it is not shader-only.
Physics uses NVIDIA RTX 4070 SUPER Vulkan; llvmpipe only renders the separate
320x240 display. Both modes execute the GPU solver, and mode 0 also uses paired normals.

| Scene / path | Geometric mode0/mode1 ratio | Two-sided 95% interval | One-sided 95% upper | Mean-speed screen |
| --- | ---: | --- | ---: | --- |
| Falling Ragdolls / ordinary | 0.912948 | [0.872564, 0.955200] | 0.945219 | Pass |
| Falling Ragdolls / native cached | 0.933863 | [0.882990, 0.987666] | 0.974905 | Pass |
| Large Pyramid / ordinary | 1.030844 | [0.989302, 1.074129] | 1.063921 | Fail |
| Large Pyramid / native cached | 1.048522 | [1.025471, 1.072090] | 1.066571 | Fail |

All reports, hashes, run matrices, means, p95/max and confidence bounds were
independently revalidated. Portable receipts are
`benchmarks/2026-09-29-small-scene-performance.json` and
`benchmarks/2026-09-29-large-scene-performance.json`. No selective extra repeats.
GPU-native remains opt-in, with CPU-compatible ordering the default: two of four
performance screens fail, tails are mixed, and physical/final-repeat gates remain
open. This evidence applies to the measured candidate, not future solver edits.

User-requested charts are rendered and visually checked: all five paired means,
ratios with uncertainty and individual pairs, and every run's p95/max.
`scripts/plot-solver-performance.py` produces PNG/SVG and an HTML index in
`artifacts/gpu-solver-qualification/desktop-baseline/performance-charts/` from the
two portable receipts. Charts delivered before starting the prepared Rain
experiment. Original failed performance receipts remain unchanged.

### Completed evidence relevant to the next step

- Nine receipt-runner regressions pass. The existing ordinary CCD metric regression
  passes; its initial sandbox adapter failure is preserved separately.
- The original failed performance attempt remains failed. One diagnostic repeat
  attributes all 16 stale metric rows exactly to deferred host CCD corrections;
  all 180 metric records match the original report. Source was restored byte-exact,
  and the rebuilt normal sample matches the original frozen executable SHA256.
  See `benchmarks/2026-09-29-performance-ccd-attribution.json` and the qualification
  report. No production solver change remains from that diagnosis.
- `scripts/solver_performance_validate.py` preserves scheduling-phase labels and
  checks known/current-step/topology/capacity guarantees, a truthful current flag,
  and the audited CCD revision transition. It is scoped to the two immutable,
  input-free benchmark scenes. Four focused tests and 15 real-report corruption
  controls pass; the original report was not rewritten or promoted to a pass.
- Small-scene session 88184 completed with exit 0. All four prewarms and 20 measured
  processes validate; five paired means were independently recomputed from hashed
  reports. Ordinary order1/order0 means: 25.3590/23.1647 ms, geometric ratio 0.912948,
  one-sided 95% upper 0.945219. Native: 24.5720/22.9559 ms, ratio 0.933863, upper
  0.974905. Both mean-latency screens pass on this candidate. Some maxima regress;
  no blanket tail acceptance or default switch. Portable receipt:
  `benchmarks/2026-09-29-small-scene-performance.json`.

All benchmark paths above are engine-relative unless prefixed with `experiments/`.
GPU-native remains opt-in: Rain/drag/final-repeat/delivery gates remain open.

### Rejected Rain coupled cone/point experiment and follow-up

CPU-only coupled cone/point controls passed 96 float64 cases: 46 active and 50
inactive cone constraints. Independent 4x4 and Schur solutions agree within
1.34e-15; stationarity/complementarity error is below 3.34e-14, momentum error below
3.56e-15, and hard zero-bias controls gain no energy. The uncoupled limit preserves
original softness. This is algebra evidence, not GPU or Rain acceptance; see
`benchmarks/2026-09-29-rain-coupled-block-controls.json`.

The artifact-only candidate is prepared under
`artifacts/gpu-solver-qualification/desktop-baseline/rain-coupled-cone-point-experiment/`:
protocol, original/candidate shader, build/replay/analyze scripts, and exact original
initial states. It couples three point rows and a unilateral cone row using Schur
elimination with refreshed angular frames, original softness, accumulated-impulse
clamping, and a near-singular sequential fallback. Fresh baseline diagnostic build
41615 exited 0, then both baseline/candidate ordinary fixtures linked successfully.
Candidate release build exited 0 and original shader source is restored byte-exact.
Session 27605 exited 0: all four ordinary replay arms completed with exact
initial state and complete finite records. Both baseline state streams exactly
match the prior angular-frame experiment. The candidate is REJECTED: hip target
anchor peak 5.908 -> 9.365 mm, severe target anchor 67.881 -> 76.285 mm, severe
all-joint upper-twist 0.032335 -> 0.051236 rad despite cone improvements.
Portable evidence: `benchmarks/2026-09-29-rain-coupled-cone-point.json`, including
candidate patch and hashes. No native expansion or five-repeat acceptance.
Normal baseline release rebuild 62190 exited 0; original shader is byte-exact
and target/release now contains the normal baseline. Restoration/hash receipt
is preserved. No live process remains from this experiment.

Follow-up capture audit session 21834 exited 0: both 120-step hip captures
reproduce their noncapture body streams byte-exact. End-state point-softness
reconstruction finds tail mean anchor 2.216/8.160 mm baseline/candidate versus
static estimates 1.830/8.736 mm. Candidate point speed remains 0.434 m/s and mean
vector residual 1.658 mm. This supports increased loaded compliance as a major
component, not static equilibrium or acceptance. The candidate remains rejected.
Portable receipt `benchmarks/2026-09-29-rain-coupled-anchor-audit.json` contains
analysis/run sources and hashes. No source change or live process remains.

Phase-consistency experiment 15139 completed: all four ordinary arms exit 0,
with exact initial getters, 5,040 finite body observations and 3,240 complete
joint observations each. Baselines reproduce prior state bytes. Candidate refreshes
angular axes at substep warm start and holds them through relaxation, preserving
original sequential rows/softness. It is REJECTED: hip anchor 5.908 -> 8.993 mm,
severe anchor 67.881 -> 72.540 mm, severe upper-twist 0.032335 -> 0.057534 rad,
despite reduced cone errors. Thus refresh timing alone is insufficient.
Portable receipt: `benchmarks/2026-09-29-rain-substep-angular-frame.json`.
Artifact directory: `desktop-baseline/rain-substep-angular-frame-experiment/`.
First arithmetic-confounded build remains unevaluated. A unit-inertia projection
control demonstrates a tangential residue, not energy injection; do not treat it
as proof of a cache-transport defect. No native/five-repeat expansion.
Original source is restored; normal baseline rebuild 29395 exited 0. Archive
SHA256 matches the prior normal baseline byte-exact. No live process remains.

New original-hip evidence closes two diagnostic branches. Existing 120-step
phase data and recent baseline state capture match exactly. All-body kinetic
phase accounting telescopes within 1.07e-14 J; kinetic plus gravitational energy
falls 143.7105 J overall and 6.6184 J over the last60steps. Stored compliant energy
is omitted, so this neither proves nor clears physical work balance. Portable
receipt/source: `benchmarks/2026-09-29-rain-phase-energy.json`.

Original hip extension to600steps (96979) and analysis (23733) both exit0.
42initial getters and first120body frames match exactly;25200finite body records,
16200joint records and600state frames validate. The predeclared stationary
compliance screen FAILS: tail120peak angular speed11.3438rad/s, peakcone.192977rad,
peak static-estimate difference.176926rad. Final cone.186425 versus predicted
.0133593rad, endpoint speed9.21768rad/s, both awake. This moving episode cannot
be classified as settled loaded softness. Receipt/source:
`benchmarks/2026-09-29-rain-hip-long-compliance.json`.
No production source change or live process remains.

Two-sweep discriminator74154 and600step extension6402 both exit0.
Both additional runtime passes were confirmed.120step baselines match original
bytes; all arms have exact initial getters and complete finite body/joint output.
Anchors and twist improve (hip5.908->5.182mm, severe67.881->63.238mm;
severe upper-twist.032335->.023802rad), but hip initialcone rises.314239->.318692.
Late hipcone improves.210057->.174631rad; severe latecone becomes0.
The600step candidate still fails stationary classification: peak tailangular
7.5088rad/s, finalcone.156916 versus staticestimate.0120632rad, endpoint speed
3.95137rad/s. All600states/25200body/16200joint observations validate and the
first120candidate frames match exactly. No production adoption or native repeats.
Receipts: `benchmarks/2026-09-29-rain-two-sweep.json` and
`benchmarks/2026-09-29-rain-two-sweep-long.json`.
Original sim.rs restored; baseline normal rebuild58446 exited0 and its archive
matches the prior baseline byte-exact. No live process remains.

Finite2x2 angular-refresh / extra-wave comparison completed. Final combined
runner61882 exits0, with all four ordinary120step arms complete and exact initial
getters,5040finite body/3240joint observations per arm. Both baselines match prior
bytes; extra-pass markers confirm execution. Combined hipcone improves to.110976
peak/.096286tail, but targetanchor worsens5.908->9.482mm. Severecone1.254127->
.591598rad, targetanchor67.881->58.787mm, uppertwist.032335->.027670rad improve.
Candidate is REJECTED under unchanged tradeoff rule. All four cells evaluated;
do not repeat them or start an iteration-count sweep. Original Rain baseline
is still unqualified, not accepted by elimination. Portable receipt:
`benchmarks/2026-09-29-rain-refresh-two-sweep.json`.
Both original sources restored; normal baseline rebuild43716 exited0 and its
archive matches the previous baseline byte-exact. No live process remains.

Current baseline state-source applicability now verified. All18dependency audit
hashes and211orderedGpuSim fields match. Twelve device owners/ten scratch regions
remain covered.21of23source files match consolidation+paired amendment exactly;
world.rs difference is precisely72test-only lines, and collide.wgsl difference
is the reviewed empty-manifold veto using already-captured contact count.
Selected flags ordinary262144/native393216 and completed-public-World boundary
remain mandatory; CPU-compatible trees/midstep/extra diagnostic modes excluded.
Three receipt corruptions (sourcehash,missingfield,wrongmode) reject. Source
contract applicability is CLOSED for baseline0940a4f, not final repeat/physics
qualification. Portable full audit bundle:
`benchmarks/2026-09-29-state-audit-applicability.json`. No production changes or
live processes. Final selected v22five-process repeat matrix remains open.

Hip row kinetic diagnostic build7466/replay4899 completed0. All physical
body bytes, dynamic phase words and aggregate phase summaries match original
exactly.1440targetpass observations telescope within7.16e-7J (<4.33% declared
float32bound). In tail60, cone bias+2.462693J/relax-2.170685J; point-.788935/
-.410249J; spring/motor dissipative on average; twistinactive. Whole targetjoint
bias+1.337268J/relax-2.724869J and warm-6.35835J on average. Thus targetjoint
updates net remove pairkinetic energy; do not call it the sole source of sustained
motion. Other joints/contact transfers and stored compliant energy remain open.
Receipt with sources/patches: `benchmarks/2026-09-29-rain-hip-joint-work.json`.
Both original shaders restored. Normal baseline rebuild89208 exited0 and its
archive matches previous baseline byte-exact. No live job remains. Diagnostic
bridge archives remain separate from the normal baseline.

Complete joint kinetic accounting build/replay31507 and analysis exit0.
All27spherical/12revolute updates included;3filter joints confirmed. Every physical
body byte, dynamic phase word and aggregate summary matches original exactly.
All1440pass records finite; aggregate row sum error<=1.75e-6J. Revolute live cached
axes preserved; carriers cleared eachphase so sleeping/skipped joints cannot
contribute stale work. Tail60phase sums: joints+11.2654J/substep, inferredcontacts
-11.6068J/substep. Warmjoint+18.8536J comprises spherical+6.1402/revolute+12.7134.
Biasjoint-2.0500, relaxjoint-5.5383. Thus net kinetic source is on the joint side
of this interleaved accounting, not contacts. This does NOT include compliant
stored energy or prove a warm-start defect/physical acceptance.
Receipt: `benchmarks/2026-09-29-rain-all-joint-work.json`.
Both original shaders restored; normal baseline rebuild7422 exited0 and its
archive matches prior baseline byte-exact. No live process remains.

Revolute basis hypothesis checked with current source and saved120step states:
1440hinge observations have current preparation tags;1428frame boundaries have
maxaxis component difference7.06e-7 and cached torque-impulse change7.51e-7.
Thus no large frame-boundary basis jump is demonstrated; do not add a transport
fix on that hypothesis. Receipt/source:
`benchmarks/2026-09-29-rain-revolute-basis-audit.json`.

Prospective passive warm-cache line-search controls pass96float64 cases:
15unchanged/33partial/48zero guesses, conserved pair momentum (<8e-15 error),
consistent cached-impulse/velocity scaling and nonpositive kinetic change within
7.11e-15roundoff. Given change L*alpha+Q*alpha², retain maximal alpha in[0,1]
with change<=0. Subsequent biased solve and physical coefficients stay intact;
active prescribed motors must be excluded. This is a numerical initial-guess
experiment, not proof positive joint work is physically wrong or an acceptedfix.
Receipt/source: `benchmarks/2026-09-29-warm-cache-line-search-controls.json`.
No production changes or live processes.

### Rejected passive warm-cache experiment and current recovery

Previous goal turn was progress: evidence/scripts committed and charts shown.
Build30828, runner51772 and normal rebuild76339 all exited0. No live process.
Artifact-only candidate: `desktop-baseline/rain-warm-cache-line-search/`.
Protocol frozen before execution; same original hip/severe120step states and
unmodified spherical_compliance_reference.cpp600step loaded-response screen.
Numerical guard used prepared solver inertia metric, not orientation-updated
physical energy. No inverse; prescribed motors/kinematic endpoints excluded;
actual impulse caches scaled consistently, live revolute axes untouched.

Candidate REJECTED under unchanged tradeoff checks. Hip targetcone .314239->
.127874 and anchor5.908->2.305mm improve, but hip all-joint lower/upper twist
.023374/.004442 -> .055346/.019265rad worsen. Severe targetcone1.254127->1.680792,
anchor67.881->75.008mm, lower/upper twist .385928/.032335 -> .885322/.051027rad.
All four Rain arms have exact initial getters,5040finite body/3240joint records;
both fresh baselines reproduce prior state bytes exactly. Original failures stay
failed. No native/five-repeat expansion or production adoption.

Both loaded-cone arms pass the original thresholds for both loads: last120
prediction errors1.74e-6/5.09e-6rad and angularspeed0. Independently validated
1200finite observations per arm. This narrow pass does not offset Rain failures.
Portable protocol/patch/sources/hashes/results:
`benchmarks/2026-09-29-rain-warm-cache-line-search.json`.
Original shader restored byte-exact; normal release archive SHA256 remains
`b3f2b64239d72c95c8e1746f9bd40a95152fb7a2c6ab373a6e2bd2986cf5265b`.
No production changes. Only new receipt and documentation changes remain uncommitted.

The regularized-row derivation and first resulting motor candidate are now
complete below. Original Rain/fullmatrix/v22repeats/recordings remain open;
performance fails two large-scene screens. GPU-native stays opt-in.

### Current motor residual investigation

Previous goal turn classified as progress: original Rain replay and analytical
loaded-response gates discriminated and rejected kinetic-only cache scaling.
Derived fixed-state softened-row residual r=w+b+Gamma*lambda with
Gamma=(impulseScale/massScale)*K.96bilateral/scalar-unilateral controls pass;
no sign/softness-coefficient defect demonstrated in these row equations.
Spherical torque-ball motor projection is different: current upstream/GPU radial
clamp is not generally the constrained minimizer for anisotropic K.96controls
show10objective increases by radial update;128float32 secular-solve controls pass
(maxrelative solution error3.17e-7). Saved120frame/27motor states show360of3240
hypothetical extra updates increase the objective, peak.04801J. These are NOT
observed shader-call inputs or causal attribution. Portable control source/results:
`benchmarks/2026-09-29-rain-motor-metric-controls.json`.

Candidate artifact `desktop-baseline/rain-motor-metric-experiment/` preserves warm
start and torque/speed/settings, replaces only saturated native-order spherical
motor projection with24fixed secular bisections and numerical objective guard.
Protocol frozen: original Rain120step tradeoff screens plus loaded-cone600step
screen, no tuning after results. Build72786, runner98522 and normal rebuild9671
all exited0. No live process. All four Rain arms have exact initial getters,
5040finite body/3240joint observations; fresh baseline streams match prior bytes.

Metric-motor candidate is REJECTED as a Rain fix: hip peakcone .314239->.319970,
anchor5.90776->5.98456mm, lower twist .0233740->.0234376. Hip finalcone improves
.183345->.156182, but tailpeak .210057->.214721. Severe peakcone1.254127->1.254525
is essentially unchanged; targetanchor67.8806->67.8776mm, lower/upper twist
.385928/.032335 -> .384739/.030620 improve slightly. Neither original episode is
resolved. Both loaded-cone arms still pass original600step/two-load criteria.
Portable patch/protocol/sources/results/hashes:
`benchmarks/2026-09-29-rain-motor-metric.json`.
Original source restored; normal archive matches baseline SHA256 above. Keep the
constrained-objective finding, but do not infer causal attribution or qualification.
Do not tune motor caps, secular iteration count or combine rejected candidates
without a new physical justification. No production code is changed.

The final-substep residual audit and resulting hinge-block discriminator are
complete below; neither local motor nor hinge correction resolves Rain alone.

### Current final-residual and hinge-block work

Previous goal turn was progress: motor constrained-objective controls and Rain
replay rejected isolated motor projection as a sufficient correction. New offline
final-relaxation audit covers all39joints/120frames (4680observations), with exact
published/phase velocity checks and no restitution/finalization velocity changes.
Remaining target-hip tail block-objective gaps average point.07497, spring.05897,
motor.02751, cone.16011 (not additive global energy). Adjacent right-knee hinge37
has the largest tail alignment gap .61537 at frame81; exact point correction
alone barely changes mean hinge alignment gap .02782->.02747 and worsens309/720
hinge observations. Portable audit/source:
`benchmarks/2026-09-29-rain-final-joint-residuals.json`.

A5row revolute alignment2/point3 coupled solve now passes96independent Schur/full
matrix controls, uncoupled-limit preservation and32hard zero-bias momentum/energy
controls. Maximum relative residual2.42e-15. This targets an untreated hinge block,
not another angular-frame/iteration change. Portable controls:
`benchmarks/2026-09-29-rain-hinge-block-controls.json`.
Candidate artifacts: `desktop-baseline/rain-hinge-block-experiment/`.
Build98892, runner23154 and normal rebuild99151 all exited0. No live process.
Four600step hinge load cases (0.01/0.1rad,0/0.4m lever), original60Hz/damping2,
pass CPU/baseline/candidate independently:7200finite complete observations,
tail angle error<=1.61e-9rad, anchor error<=5.97e-9m, speeds below original limits.
This validates the derived coupled steady load, not contact-loaded Rain.

Hinge candidate REJECTED under frozen Rain tradeoff checks. All four Rain arms
validate initial getters/5040body/3240joint observations; baseline streams exact.
Hip peakcone .314239->.316846, tailcone .210057->.200186, anchor5.908->5.887mm;
all-joint lower/upper twist .023374/.004442 -> .023733/.007383 worsen. Severecone
1.254127->1.254523 unchanged, lower twist .385928->.386411 worsens, tailcone
.007469->.027068. Both original loaded-cone arms pass unchanged. No native or
five-repeat expansion, production adoption or performance claim. Portable fixture,
protocol, patch, sources, manifests and results:
`benchmarks/2026-09-29-rain-hinge-block.json`.
Source restored byte-exact; normal archive SHA256 matches baseline above.
Only documentation/portable evidence changes remain; no further commit requested.

Next discriminating work: a residual-controlled convergence experiment across
complete interleaved joint/contact waves, rather than another local row variant.
Predeclare one fixed numerical convergence criterion covering velocities AND
persistent constraint impulses (or an equivalent full projected residual), with
an explicit work cap reported as nonconvergence. Do not use net body velocity
change alone: opposing row updates can cancel while caches keep changing. Do not
tune a fixed iteration count or loosen physical gates after seeing results.
The original43body replay uses `src/sim.rs::emit_wave` -> `solve_jointed_wave`
(single workgroup, all colors). This permits a bounded artifact-only discriminator;
larger worlds and native cached execution still require separate implementation
and full qualification if it works. Previously two fixed sweeps were insufficient;
that is not evidence of a converged solution. Preserve all original replay/loaded
screens and reject nonconvergence or failed physics; do not claim a faster/correct
production mode from a small-fixture convergence experiment.


### Current complete-wave convergence discriminator

Previous goal turn was progress: hinge-block and analytical response tests ruled
out another local correction as a sufficient Rain fix. Artifact experiment at
`desktop-baseline/rain-wave-convergence/` now implements a fixed convergence
criterion: max relative component change<=1e-5 for both endpoint velocities and
reusable joint/contact impulses, for two consecutive complete color waves, cap128.
Warm start/integration/restitution remain once per phase. The monotonic contact
`total_normal_impulse` accumulator is consumed only as zero/nonzero by restitution;
monitor that consumed state, not its increasing magnitude. No physical threshold
changes. Three unused filter-joint records carry8pass statistics per frame.

Control build40218 completed but runner43312 failed before simulation because
WGSL reserves the local name `active`; failed-parse-v1/ preserves the failed
receipt/archive/source. Renamed local to cv_enabled with no numerical change;
full control/candidate WGSL parses and validates using the existing Naga build.
Corrected control rebuild17915 exited0. Runner70256 stopped on a raw-state
comparison difference: occupied contact root append order in97hip frames only;
body stream exact. Raw failure remains in hip-control/validation.json. Existing
repository audited_storage_view confirms all120hip states semantically exact
except five declared filter-carrier fields; membership identities/history/solver
order remain exact. This is the existing audited storage contract, not a new
exclusion. Companion52327 exited0; severe120step body stream also byte-exact.
No-op controls complete. Candidate build51528 exited0; probe40187 child exited0
but driver exited1 for numerical nonconvergence:5of8passes hitcap. Biased passes
0/1/3 converge in72/108/94waves; bias2stillhasimpulsechange1.70e-5 despitevelocity
4.47e-7 atcap. All4relaxationpasses hit128withvelocitychanges .0785/.0239/.0438/
.0260 andimpulsechanges .01268/.00110/.00468/.00208. Initialgetters exact,
42finite body observations and8pass records validate. Expansion stopped asdeclared.
Normal rebuild84954 exited0; original shader and baseline archive SHA256 verified.
No live process remains.
Protocol wording correction: total_normal_impulse is also exposed by manifold
getters/state capture and used aspositive for hit events. Its magnitude is excluded
ONLY from stopping norm, not complete-state qualification. Repeated-wave artifact
changes its accumulation count; production accounting remains unqualified.
All original control/parse/raw-comparison failure receipts remain preserved.
Portable complete protocol/patch/source/hash/results:
`benchmarks/2026-09-29-rain-wave-convergence.json`.
No120step expansion, native runs or qualification followed the cap failure.

At the capped final state,15of27hypothetical isolated spherical motor updates
still increase their quadratic objective (max3.80e-6J); metric projection never
increases it. These are not actual row-call inputs or causal proof. A concrete
next discriminator is the already-derived torque-ball metric correction within
the same complete-wave experiment, unchanged1e-5/twoquiet/128cap and original
initial state. The previous one-wave metric test did not address convergence;
this new nonconvergence/row-objective evidence is the justification, not a blind
combination or parameter sweep. Preserve original failed probe, use a new artifact
arm and stop on cap failure. Even convergence would not imply physical Rain pass;
original replay/analytical/fullmatrix/performance/recordings gates remain open.


### Completed metric-motor convergence probe

Previous goal turn was progress: validated no-op controls and documented5of8
nonconverged complete-wave passes. New artifact arm:
`desktop-baseline/rain-wave-metric-convergence/`. It changes only the previously
controlled spherical metric motor projection relative to the failed wave arm.
Inputs,1e-5/twoquiet/128cap and expansion-stop rule unchanged; protocol frozen.
Full shader parse/validation passed; build61583 exited0 and restored the source.
Probe15025 completed: child0, driver1 for the declared numerical failure. Five
of eight passes remain nonconverged, including all four relaxation passes at
cap128. Their final velocity-change norms are .078098, .027773, .040634, .030479;
substep2 bias cache change is 1.87407e-5. Initial getters are exact and all body
outputs finite. Stop expansion: metric motor projection alone does not resolve
the complete-wave convergence failure. No production adoption.
Portable receipt: `benchmarks/2026-09-29-rain-wave-metric-convergence.json`.
Normal baseline rebuild36958 and archive restoration are verified before commit.
Next solver work must identify the remaining relaxation fixed-point obstruction
from existing evidence before another candidate; do not increase the iteration cap.
The full physical, repeat, and recording gates remain open.

### Current contact metric convergence discriminator

Previous turn made progress: combined motor/convergence failed and was committed
in ff811a6. Existing capped states now expose contact-row non-descent too.
`desktop-baseline/rain-contact-metric-audit/` reconstructs41active contact blocks
using prepared inertia. In the motor-corrected state,3tangent and9rolling isolated
next updates increase the fixed-bound quadratic objective, maxima4.0305e-5 and
2.30996e-6J. Metric minimizers do not increase it. These are hypothetical end-state
updates, not actual row inputs or causal proof. Prepared tangent inverse identity
error<=5.50e-7. One old tangent cache exceeds current bound by float roundoff.

256float32 controls (2D embedded tangent disks and3D rolling balls, including zero
bounds and infeasible-old caches) agree with independent eigensolve within2.35e-7
relative solution error; bound excess<=6.43e-8. Full shader validation passes.
Artifact `desktop-baseline/rain-wave-contact-metric-convergence/` adds only contact
metric projection to prior motor/wave arm. Friction coefficients/normal bounds,
input,1e-5/twoquiet/128cap and stop-on-failure are unchanged. Build56685 exited0 and
restored source byte-exact. Probe19284 child0/driver1:5of8nonconverged again.
All relaxation passes hit128, finalvelocity changes .075682/.045506/.048816/
.032721; bias2cache change1.99569e-5. Exact initialgetters/finite bodies validate.
Normal rebuild66786 exited0; source/archive byte-exact. Expansion stopped.
Portable receipts: `benchmarks/2026-09-29-rain-contact-metric-audit.json` and
`benchmarks/2026-09-29-rain-wave-contact-metric-convergence.json`.
No production adoption or physical qualification. No new parameter sweep.

Final-state joint residual reconstruction shows the metric motor correction
reduces the largest motor objective gaps substantially; after contact correction,
hinges27/39 alignment and point blocks dominate (largest gap1.09e-5J). These are
isolated counterfactual row gaps, not additive energy or actual in-wave inputs.
An initial exact-feasibility assertion fails on4.08e-10motor norm excess; retained
failure receipt and corrected diagnostic report signed gaps/feasibility explicitly.

Completed bounded diagnostic: `desktop-baseline/rain-wave-convergence-history/`
records16predeclared wave checkpoints and maximum-update labels in extra unused
filter fields, without changing calculations or cap. This will distinguish slow
residual decay from a plateau; finite-cap failure alone does not prove a cycle.
Require exact parent body stream and captured semantic state except declared
filter fields under the existing occupied-order audit. Shader validation passes;
build26428 exited0 and restored source. Probe67647 child0/driver1 (same numerical
failure); normal rebuild85691 exited0 with byte-exact baseline archive. No live jobs.
Validator confirms exact parent body stream and complete semantic state except
five declared filter fields, using existing occupied-order normalization. Two
corruption controls reject. History shows slow decay, not a demonstrated cycle:
wave64/96/112/128 velocity changes .062777/.045012/.037640/.032721 and cache
changes .005107/.004766/.002650/.002421. Hinge39 (bodies40/41) dominates late
velocity updates; ground contact1/41 dominates wave96. Largest-update labels
are quantized1/32767, while reported norms and convergence remain exact.
Local hard hinge alignment/point analysis gives isolated Gauss-Seidel spectral
radius .0970 for39, .1282 for27, max .1598 across12hinges. Thus isolated hinge
internal splitting is not a strong explanation for the much slower observed
network decay; do not blindly combine the earlier hinge-block candidate.
Portable history/no-op evidence: `benchmarks/2026-09-29-rain-wave-convergence-history.json`.
Next discriminating work: use saved fixed-geometry Jacobians/active rows to
identify the slow network mode and choose a coupled/preconditioned solve that
preserves original softness and bounds. Establish reference residual reduction
before another GPU candidate. Do not increase the cap or infer an infinite cycle
from finite nonconvergence. Full physics, native path, five-repeat matrix,
performance and recordings requirements remain intact.

### Current coupled reference calculation

Previous turn was progress: verified slow-decay history and weak isolated hinge
coupling changed the next action to a network reference. Artifact:
`desktop-baseline/rain-coupled-reference/`. Frozen protocol reconstructs all39
joints/41active contact manifolds (56normal points),413blocks/734impulse unknowns
from the last contact-metric capped state. Prepared inertia, final joint geometry,
prepared contact levers, original spring softness/targets and normal-dependent
friction/rolling/twist bounds are retained. Tiny inertia asymmetry is explicitly
symmetrized; delta poses are reconstructed, so this is float64 subproblem evidence,
not a bit-exact GPU trajectory. Contact normal/tangent prepared mass identity
errors are <=5.50e-7. Original production sources/build remain untouched.

Solve natural-map complementarity equations with an analytic generalized Jacobian
and SciPy LM, frozen max120function evaluations. Require24directional Jacobian
controls<1e-5 before solving, final normalized residual<=1e-8, original domain
feasibility<=1e-8scaled, direct primal/assembled agreement<=1e-9scaled and a full
sequential metric block sweep with velocity/cache changes<=1e-5. No larger budget
retry after failure. Session12723 exited1: all24Jacobian controls pass (<1.4e-9relative), but LM
reports xtol success with final natural residual5.20965e-4 and full-sweep velocity
change .0481242. Preserve the failure; optimizer status is not acceptance.
Runtime `/tmp/rain-coupled-reference-venv/bin/python`: SciPy1.18.1, NumPy2.5.2,
Clarabel0.11.1, BLAS/OMP threads1. No physical acceptance implied.

Structured reference8229 exited1 at unchanged32outer iterations/58QP calls:
fixed-bound convex QPs plus Anderson normal-bound fixed point reduce full residual
to7.81915e-6, still failing1e-8. Full-sweep velocity change8.09684e-5 also fails.
This preserves nonassociated contact normal equations; a single joint friction-cone
QP would introduce different normal KKT terms and is not used. All inner statuses
AlmostSolved and their independently measured residuals are retained.

Hybrid refinement33206 exits0: one full semismooth Newton step from the preserved
best structured state passes all original criteria. Natural residual4.81514e-11,
scaled feasibility4.34260e-11, direct primal/assembled agreement8.91e-15. A complete
sequential metric sweep changes velocities<=1.48475e-9 and caches<=7.70e-11.
Jacobian rank730of734, minimum singular1.07e-16, Newton linear residual1.65e-17.
Reference velocity differs from capped state by .0571418 maximum component and
cached impulses by .835058; largest impulse5.00817. This is a converged solution
of the reconstructed contact-metric phase, not a physical trajectory, bit-exact
shader solution, ordinary/native qualification, or production adoption.

GPU-oriented projected Anderson prototype62240 exits1: plain and accelerated
waves both fail the predeclared128evaluation budget from cold and capped starts.
Captured color/row order and paired normals are reconstructed; cache proposals
are projected to original domains and body velocities reconstructed consistently.
Memory4/weighted least squares/one proposal/0.9merit acceptance were fixed in
advance. Cold acceleration accepts7/rejects60; capped accepts0/rejects63.
Final natural residual plain/accelerated: cold .004037/.006566, capped
.000991/.001286. No GPU implementation of this failed accelerator is justified;
do not tune its memory or acceptance threshold. Initial states are reconstructed
cold/capped, not original GPU phase entry, so no128phase acceptance is claimed.

Portable `benchmarks/2026-09-29-rain-coupled-reference.json` embeds compressed
645691byte input trace, all scripts/protocols/results/failures and complete
reference impulse/velocity arrays. `verify_portable.py` reconstructs solely from
the receipt in a temporary directory and reproduces residual/sweep checks with
zero reference velocity difference. No raw external input or SciPy is needed for
that verification (NumPy only). All jobs terminal, production source/submodule
clean, normal archive unchanged. No additional commit requested.

Next: prototype a coupled generalized-Jacobian solve (e.g. matrix-free Newton/
Krylov with constraint-block preconditioning) against this strict reference,
with frozen work and original domain checks from cold/capped states before any
GPU candidate. Inspect the four numerical null directions and nonsmooth bound
activation when choosing the solve; a finite sweep failure does not prove no
solution. Avoid more local row variants or iteration-count/Anderson sweeps.
Full physical/defaults, original Rain screens, actual phase-entry GPU validation,
ordinary/native independence, complete-state five repeats, performance and
recordings remain mandatory; this reference closes none of those broader gates.

### Current matrix-free coupled prototype

Previous turn made progress: converged full reference plus failed projected
Anderson directs work to coupled Newton/Krylov. Artifact:
`desktop-baseline/rain-newton-krylov-prototype/`. Matrix-free natural residual,
forward Jacobian and adjoint use only constraint-local rows/body inverse mass;
1/2/3row blocks supply GMRES preconditioning. Dense matrices are controls only.
24controls pass (maxforward1.43e-14, adjoint7.43e-15, value3.23e-15, localinverse
6.44e-15); omissions of softness or normal-bound derivatives are rejected.
Four near-null directions at reference redistribute motor/limit impulses mostly
within joints9/13/35/41. Body velocity effects<=9.04e-14, so these are redundant
impulse allocations, not unconstrained physical motion. Smallest retained singular
value6.44527e-5, largest24.851. Initial import failure from prototype filename
operator.py is preserved; renamed nk_operator.py without numerical change.

Frozen run87145 exited1:16Newton-step limit, GMRES32x4cycles and160action/step
cap, original forcing/Armijo rules and final acceptance. Cold stops at11Newton
steps/532actions, capped at2steps/266actions, both failed line searches. Final
natural residuals .0124091/.00171665 and colored-wave velocity changes
.0643191/.0321590 fail. Capped GMRES reaches its128Arnoldi limit with true linear
relative residual .0894/.0578; no numerical pass or GPU adoption.

Direction check31992 and exact-linear obstruction check34804 both exit0.
Measured small-step merit slopes approach the predicted slopes (errors~1e-8);
there is no demonstrated branch-derivative bug. Failed-state Jacobians rank731,
unreachable residual fractions<=5.76e-14. Exact SVD Newton solves reach~2e-12
linear relative residual yet still fail all8original Armijo sizes. Thus better
Krylov accuracy alone would not repair the observed global step failure. No
larger line-search retry was performed.

Trust-controlled numerical-step variant70480 exits1. It solves shifted Newton
steps but always checks the original unshifted physical equations. Frozen sigma
updates/trial budgets improve residuals but both arms fail after32accepted steps:
cold42linear trials/950actions, natural .00109806, wave velocity .0228041;
capped40trials/1733actions, natural5.31593e-5, wave velocity .00208634. Preserve
all trials and both failures; do not sweep damping schedules. No GPU adoption.
Portable evidence: `benchmarks/2026-09-29-rain-newton-krylov-prototype.json`.

Exact bilateral elimination prototype25522 also completes1 under frozen final
checks, but its algebra controls pass. Artifact `desktop-baseline/rain-bilateral-schur-prototype/`
eliminates234unconstrained point/alignment/spring variables and retains500bounded/
unilateral unknowns. Original Gamma is retained, never recomputed from reduced
masses. H_ff is SPD (min eigen .0137474, condition482480);24free-residual/Schur-gradient/
velocity controls error<=9.10e-13. Reference reconstruction error2.83e-15 and
original residual4.82e-11. All transformed local blocks are full rank in this case.
Plain reduced waves still fail128budget: cold residual .00473656, capped
.000422357; original colored-wave velocity changes .0382096/.00430913. Domain
feasibility stays within8.61e-17. Exact elimination is validated, not full algorithm
convergence or physics. Serial prototype uses captured row/color order, but its
articulation-wide response means original GPU endpoint coloring cannot be reused
without a new ownership design. Portable:
`benchmarks/2026-09-29-rain-bilateral-schur-prototype.json`.

All processes terminal, production source/submodule and normal archive unchanged.
Next discriminating implementation: a coupled solve on the reduced500variable
system, using verified Schur action and local effective-mass scaling, with original
full residual/domain/sweep acceptance. Avoid another plain sweep, larger128cap,
Anderson-memory or damping-schedule search. Prefer a mature bounded trust-region
reference before a GPU implementation; verify any matrix-free reduced Jacobian
against the algebra controls. A successful algorithm still needs float32, actual
phase-entry/all8phase testing, original120/600step Rain/analytical controls and
all remaining full-goal gates. Do not mistake this fixed-geometry reduction for
an accepted production change.

## Bounded execution ended: friction, support and small-scene performance (2026-09-29)

User authorized all three candidates on commit d19a93d1a4b3fe51aa1de26222cf14b7560ded3f:
1. Existing sliding-friction240step fixture: five fresh runs per ordinary/native
   path, original physical assertions and exact raw v22captured-state equality.
2. Existing stack-support/energy600step fixture: same five fresh runs per path,
   original assertions and exact state equality.
3. Falling Ragdolls performance: five paired fresh runs per GPU path comparing
   CPU-compatible1/GPU-native0 ordering, fixed scene/config,60warmup+180timed,
   completed-step unpaced timing, no health/state capture or competing GPU work.
   Prewarm both orders equally, alternate order within pairs; report all run
   means/p95/max, paired ratios and log-ratio confidence bound. No default switch.

Stop each task on first failure, preserve/report it, continue independent tasks.
No fixes, solver edits, extra repeats, commit or push. Broader goal stays paused.
Evidence: experiments/gpu-physics/artifacts/gpu-solver-qualification/desktop-baseline/three-checks-d19a93d/.
Friction and support both PASS:20fresh processes, four exact raw comparisons,
240/600frames respectively. Physical driver77733 terminal0. Friction stopping
position0.39576817m; support overlap0.0009977221m, energy gain0, tail speed0.
Performance builds51274/2114 terminal0, feature-disabled capture libraries linked.
Performance driver97283 terminal1: stopped at first validation failure during
prewarm-ordinary-order1 (sample process exit0). At completed/submitted step87,
contact metrics current=false; snapshot_state88 versus current_state89. Metrics
are known, snapshot_step87 and capacity_loss=false. No root cause inferred.
No measured pairs ran; performance acceptance remains OPEN with no speed claim.
No fixes or reruns. Fixed protocol and original report/log retained under
performance/. Portable receipt:
experiments/gpu-physics/benchmarks/2026-09-29-friction-support-performance-d19a93d.json.

Acceptance: friction PASS (10/10); support PASS (10/10); performance STOPPED at
its first validation failure (0/10 measured pairs). The bounded execution and
reporting task has ended under its stop-on-failure rule; this is not a performance
qualification pass. No solver edits, commit or push. Broader goal remains paused.
Next action requires a new user request: resolve the stale-metrics validation
failure before attempting performance qualification. Do not automatically retry.

## Completed bounded goal: restitution requalification (2026-09-29)

User selected restitution only on commit `b45e86b4fa08f7edb73240bcc13e36d2ad6dcf7d`.
Run the existing `trace_restitution_matches_analytical_rebound` test in five fresh
processes on each ordinary/native cached GPU path, 240 steps each, GPU-native
ordering0. Preserve all original checks: rebound apices within0.05m of1.0/2.5m,
penetration<0.03m, energy<25.2J/kg, bounded lateral drift and inelastic settling.
Require exact complete v22captured-state equality within each path, no
normalization. Verify build/source/config identity and native cache policy.
Stop after result or first failure; no solver changes or broader investigation.
Broader solver goal remains paused. No additional recordings requested.

Both feature-enabled builds and driver61850 finish0. All10fresh processes pass
exactly the selected test and all original physical assertions. Both raw five-run
comparisons pass for all240v22frames, no normalization (2400total captured steps).
Source hashes/frozen binaries unchanged afterward; distinct process IDs and
per-frame native cache policy, GPU-native ordering and clean status verified.
Every run records rebound apices0.99096805/2.488538m, peak penetration
0.004999995m and peak energy25.017302J/kg, within original limits.

All bounded gates PASS on b45e86b. Evidence:
`experiments/gpu-physics/artifacts/gpu-solver-qualification/desktop-baseline/restitution-b45e86b-five/`.
Portable receipt: `experiments/gpu-physics/benchmarks/2026-09-29-restitution-b45e86b.json`.
No live jobs or solver changes. Stop here; broader solver goal stays paused.
The user authorized a combined local commit of all pending changes, including
the earlier README setup updates. No push or further solver work requested.

## Completed bounded goal: contact-island requalification (2026-09-29)

The user authorized only candidate1: requalify the existing73step
`api::world::state_trace::tests::trace_contact_island_wake_propagation` on commit
`791990c6428e641df5c254901eb3afdbbb7fd3ec`. The broader solver goal stays paused.
Run five fresh processes per ordinary/native cached GPU path with original
physical assertions and GPU-native ordering0. Require all10tests to pass and
all73v22state frames to compare exactly within each path, without normalization.
Verify committed source, freshly validated feature-enabled builds, frozen binary
hashes, native runtime settings and captured policy. Preserve original failures.
Stop after the result: on first failure, report it without solver edits or broader
investigation. No other fixtures or recordings are requested.

Both feature-enabled test builds85250/67977 and driver85409 finish0.
All10fresh processes pass exactly the selected test and its original physical
assertions. Both five-run73frame comparisons pass in raw mode with no field or
membership normalization. v22schema, frame sequence, GPU-native ordering,
paired-normal policy, clean sticky status and ordinary/native cache policy were
verified on every frame. All10process IDs are distinct. Source hashes and frozen
binaries still match after the runs. No source/test/default changes were made.

All bounded acceptance gates PASS on commit791990c: ordinary5x73,
native5x73, original physical checks, exact captured-state equality within paths,
source/build/config identity. Total730captured steps. Evidence directory:
`experiments/gpu-physics/artifacts/gpu-solver-qualification/desktop-baseline/contact-island-791990c-five/`.
Portable receipt: `experiments/gpu-physics/benchmarks/2026-09-29-contact-island-791990c.json`.
No live jobs. Stop here as requested; broader solver goal remains paused.
The user subsequently authorized a local commit of these results. No additional
tests, investigation, solver changes or push are authorized.
User README edits remain outside scope. Read this section first on continuation.

## Paused handoff (2026-09-29)

User requested stopping after diminishing returns. Do not resume experiments or
qualification until the user resumes the goal. No solver/build/repeat-runner jobs
remain in the process check at pause. Rejected angular-frame experiment is fully
reverted: solve.wgsl equals its saved original, and the ordinary release archive
matches baseline-rebuild-exit.json. Box3D submodule is clean. git diff --check
passes. Retained fixes, tests, documents and local evidence remain in place;
retained goal changes are being saved in a local checkpoint commit; pre-existing
user README edits remain outside it.
Cleanup did not delete evidence or switch default mode. The user subsequently
authorized a local checkpoint commit; no push is authorized.

Recent work isolated stale angular directions and rejected two inadequate fixes;
it did not close Rain acceptance. Avoid reporting a precise completion percentage.
Before further expensive runs, reassess the coupled angular-limit/anchor proposal
and set a bounded experiment with an explicit stop/retain decision. Outstanding
scope remains Rain/drag physical acceptance, final five-repeat matrix on both
paths, performance, recordings and default-mode decision. Detailed latest results
and rejected approaches follow below; older live-job notes are historical.

## Local checkpoint commit (2026-09-29)

User authorized saving retained work in a commit; goal remains paused, no push.
Release and native GPU sample builds succeed;9repeat-runner and13state-storage
helper tests pass. Pre-commit recordings cover19standard scenes at300steps on
GPU and real Box3D CPU, plus native lifecycle/ragdoll/Rain scenes. All46video
files have valid video streams. Seven native runs finish0; additional GPU Rain
was deliberately terminated with SIGTERM after at least138observed steps to
bound checkpoint work. Its partial visual clip is valid, its600step run is NOT
a pass, and original failure/stop receipts are preserved. The other native Rain
CPU and Falling Ragdolls runs finish600steps. CPU column is first in the grid.
No controlled performance or final qualification is claimed; capture timings
include rendering/encoding and concurrent work. Portable hashes, commands and
results: experiments/gpu-physics/benchmarks/2026-09-29-paused-solver-checkpoint.json.
Videos/raw reports remain local under recordings/ and artifacts/. All recording
processes are terminal; dedicated Xvfb was stopped. Pre-existing README changes
are excluded from this commit. Rejected angular experiments remain reverted.

## Read and maintain this file

Read this file at the start of each goal continuation and after compaction or restart. It contains the complete objective and current recovery context; no earlier conversation is required. Verify recorded facts against the worktree, artifacts and actual process state before relying on them. Update current evidence, decisions, jobs and next actions after meaningful progress and before handoff. Adapt implementation plans when evidence warrants it; do not silently narrow the objective, weaken acceptance limits or erase failures. User instructions take precedence.

Detailed evidence and historical investigations belong in [gpu-solver-qualification.md](gpu-solver-qualification.md). Generated evidence is under `experiments/gpu-physics/artifacts/gpu-solver-qualification/`; its `checkpoint.json` is a machine-readable evidence index. This file owns the current goal and recovery plan; the qualification document owns detailed results. Keep status summaries consistent.

## Portable checkpoint handoff (2026-09-28)

### Current desktop continuation

Workspace and authorization: HEAD/origin checkpoint `2f966088c006b5623b94b07e825477b5135ba1bb`, branch feat/gpu;
Box3D submodule remains clean at `30c67b5e6d0a3a66f0f506c69ce9e9e0587e3b7c`.
Pre-existing root/engine README edits belong to the user and are excluded from
the checkpoint commit. User authorized a local commit on2026-09-29 after pausing;
no push or resumed solver investigation is authorized. No subagents. Device confirmed by fixtures: RTX4070 SUPER, Vulkan,
NVIDIA610.57.04. Historical laptop artifacts are unavailable, not local passes.
All paths below are engine-relative under `artifacts/gpu-solver-qualification/desktop-baseline/`.

#### Latest recovery: phase replay and angular-frame experiment

Previous status-only turn was no progress; this continuation completed new evidence.
Phase bridge build22500 and replay37693 finish0. Artifact-only bridge reuses the
existing24-phase capture; no production instrumentation added. All120step body
outputs with capture off/on are byte-identical to the frozen ordinary hip baseline
(SHA2564d49e563a8b21a8ad1389560e6ca85ec2558c0b74e8d266a9d3a1de65f730a2d).
All43body phase slots validate against getters; max velocity rounding error5e-8.
Hip quaternion reconstruction from integrated phase velocities matches endpoints
within1.20e-7/component. Evidence: rain-hip-phase-replay/{equivalence.json,
phase-validation.json,hip-phases.json,analyze.py,protocol.json}.
In last60steps, mean biased cone rates along actual substep axes are
-6.442,-3.495,+.235,+3.418rad/s for substeps0..3; along the frozen prepared axis
all remain about-6rad/s. In54/60last substeps the frozen projection is inward
while actual projection is outward. Firstsubstep mean swing change-.021289rad
is undone by lastsubstep+.020620rad. This identifies stale-axis contribution in
this fresh-history replay, not complete Rain acceptance or per-constraint work.

Next distinct bounded experiment refreshes BOTH cone and twist angular frames,
Jacobians and effective masses together (prior rejected experiment refreshed cone
only). Prepared inertia/scalar caches remain approximations. No default changes.
Protocol: rain-angular-frame-experiment/protocol.json; require original-material
health480persistent and health514severe120step controls, reject cone improvement
that worsens anchor/twist failures. Ordinary first, native only if justified.
Build23608 and fixture/replay61749 finish0; all4ordinary arms validate exact
initial getters,5040finite body observations and3240spherical observations.
Hip baseline byte-identical to original. Candidate reduces hip tail60cone
.210057→.117623rad and final.183345→.089383, but target anchor peak grows
5.908→9.391mm. Severe cone peak1.254127→.766228rad, target anchor67.881→73.209mm,
and all-joint upper-twist peak.032335→.061229rad. Candidate REJECTED, no native
qualification justified. summary.json preserves all metrics/decision.
Original shader restored byte-exact (restoration.json). Baseline release-library
rebuild36986 finishes0, with restored-source and archive hash receipt in
baseline-rebuild-exit.json. Candidate archive is separate and rejected.
All jobs are terminal; no live job to resume.
Next solver work should address angular-limit/point-anchor coupling together;
do not repeat cone-only or cone+twist refresh or material/substep ablations.
A coupled cone/point row solve with original softness and unilateral cone bound
is the next concrete candidate to assess, with both original replay controls and
analytical block-solve controls before any full-scene qualification. This is a
proposal based on the rejected tradeoff, not a proven cause or accepted fix.
No production change retained from this rejected experiment. The later local
checkpoint commit is authorized; the goal remains paused.

#### Implemented and verified changes

- Repeat runner writes the exact child exit before Xvfb cleanup and preserves
  child/launcher failures separately; no failed receipt is rewritten. Nine runner
  tests and five actual Xvfb synthetic launches pass. All32Python helper tests pass.
- Xvfb21.1.24-1 was signature-verified and extracted without sudo to
  `artifacts/local-tools/xvfb/usr/bin`. Prepend that directory to PATH for sample
  launches. User must not send a sudo password; no device access is needed.
- Capture now includes `gpu_policy.contact_metrics` (all nine fields/null), which
  controls refresh and public metrics. Focused mutation and broader capture tests
  pass on both paths. Active timing-window counters remain captured separately.
- Analytical mass-mutation fixture covers six stages/18steps with nonzero impulse
  rejection for kinematic/massless stages. Five fresh runs per path have identical
  raw state (`mass-mutations-nonzero-five`). This predates later production fixes.
- Provisional CCD correction caps the old half-extent activation threshold by the
  unchanged20mm speculative range in host/motion/world/fast-proxy paths. Five240step
  restitution runs per path pass original limits with identical raw traces:
  penetration4.999995mm vs preserved49.311mm baseline failure; apices0.99096805/
  2.488538m. Related58checks per path pass (`restitution-ccd-five`,
  `restitution-ccd-coverage`). Wider final qualification/performance remain open.
- Mesh scratch lifetime bug reproduced on both paths: scratch reset a retired root
  counter1 to0, causing the same public handle to revive on re-entry. Corrected
  `collide.wgsl` preserves each physical slot's root counter through scratch stores,
  discards and child publication. Seven contact regressions and five fresh unread
  scratch-reuse tests per path pass. Frozen before binaries/failures, shader backup,
  hashes and successful evidence are in `mesh-generation-fix`.
- Five16step child/joint-history traces per path compare raw-identically after the
  lifetime fix (`mesh-generation-history-five`). Unread lifetime tests deliberately
  avoid intermediate harvesting; they are distinct from per-frame state evidence.
- Test-only child relocation now leaves counters at physical slots. Actual16step
  relocated probes pass on both paths; only child positions at2–3 and prior-touching
  storage at4 differ from controls. All counters/root owners/physical/public/motor
  history match (`mesh-generation-fix/relocation-counter-comparison.json`). Builds
  61709/85099 and probes92506/55538 are terminal0. No production change in this helper.

#### Current full-scene evidence

Current release diagnostic libraries and GPU sample executables include CCD, mesh-
generation, status and static-list overflow corrections (latest build below).
Independent ragdoll fixtures include CCD/mesh corrections only. Earlier logs are in mesh-generation-fix;
all library/sample/fixture build handles57423/89761/65489/85011/18420/75547 finish0.
Latest test binaries are ordinary `target/release/deps/gpu_physics-f3c11a04cf20bcdb`
and native `target/native-cache-build/release/deps/gpu_physics-e01a65b975ea9699`.
Native runtime must source scripts/native-samples-cache-env.sh.

Ragdoll single600step captures (`ragdoll-generation-candidate`) pass original
physical limits and have byte-identical pose/velocity/separation output to the
CCD-only predecessors. Same lifetime checker finds six reused tokens and377counter
decreases before the fix, zero of either afterward. Reports and hashes are in
mesh-generation-fix/{before,after}-scene-lifetimes-{ordinary,native}.json.

All five fresh600step runs per path now complete under `ragdoll-generation-five`.
Every child exits0 and all ten unchanged physical screens pass (physical-summary.json).
Aggregate runners71165/81966 retain exit1 for raw occupied-root append order at1.
Native whole-slot permutation audit7117 passes; ordinary85024 fails at141 because
child [[32,89],1] is in physical slot158/counter1 vs156/counter2. The physical
counter vector and root owners are nevertheless identical at that step.

The separate fixed-counter diagnostic keeps EVERY physical counter and every root
slot exact and compares only child placement as membership, in addition to the
previously audited occupied/previous-touching memberships. Both five-run600step
audits26645/82028 now finish equivalent=true (fixed-counter-audit.json). Controls
reject counter/root/public-state mutations and missing children while accepting
child relocation between unequal-counter slots. This is bounded diagnostic
storage evidence, not a full persistence/semantic qualification pass. Raw gates
remain failed; do not launch another broad batch without a specific reason.

#### Current Rain evidence and live work

- Frozen pre-CCD/pre-generation Rain baselines reach600steps on both paths.
  Exact child and launcher receipts are0; ordinary5275s/native5239s. Drivers29137/
  97952 now finish0 with complete.json, verified gzip/uncompressed hashes and all
  600frames validated. Raw files are replaced by state.jsonl.gz. Outputs
  rain-full/{ordinary,native}.
- Independent original CPU Rain reference finishes0, including600sequential frames
  and spherical validation (session5713 terminal0;224s). Frozen CPU library/binary,
  source and configuration hashes are in rain-full/cpu. It is a physical reference.
- CPU and ordinary absolute scans finish0 (23388/62744); each has8400creations,
  1948800joint observations, zero NaN/exploded/capacity-loss frames. Ordinary's
  longest >.05rad twist episode is369–463 (95frames,38asleep,endpoint2379), peak
  .104365408rad, final .0544822812rad. Peak cone1.26105142rad at516,endpoint5004.
- Ordinary creation-matched600step CPU screen22773 finishes1: identities/recycling
  pass, original residual screen fails (557anchor/3004angular/5150cone/1086lower/
  371upper observations). Native scan39195 finishes0; native CPU screen32912 finishes1 with exactly
  the same report, episode list and peaks as ordinary. Reports rain-full/MODE/{residual-episodes,cpu-relative-screen}.json.
- Existing health selector now accepts explicit source/output/windows; new sources
  require explicit identities. Ordinary selection35419 finishes0, validates whole
  report. Actual windows: twist368–463,base2353,target2379; cone514–530,base4999,
  target5004. Extraction90900/30134 finishes0, validating frozen ordinary full trace into
  rain-episodes/ordinary-{twist,cone}.jsonl. Diagnostic excerpts, not qualification.
  Selector controls pass valid input and reject missing body/truncation/missing
  explicit windows/bad sequence (selector-controls.json).
- Local twist compliance reconstruction completes96frames. At core426 (lastawake),
  observed .0544815634rad vs estimated .0544807131rad; core427 sleeps. This
  reproduces earlier correlation, not full substep load balance/acceptance.
  Independent incident-load reconstruction (excluding target twist cache) now
  predicts .4860739147N·m·s vs actual .4860729575 at426, relative1.97e-6;
  predicted soft error .0544808204rad. Awake410–426 max relative mismatch7.36e-5.
  Cache-zero and self-contact-removal controls verify independence/sensitivity.
  It supports loaded equilibrium but uses cached final-substep impulses/end-lever
  approximations, so does not claim full dynamic balance or Rain acceptance.
  Evidence rain-episodes/{reconstruct_load.py,ordinary-twist-load-balance.json,
  load-balance-controls.json}.
  Cone follow-through selection88170 finishes0 through health599: residual returns
  from .005660rad at525 to .159410rad at531, then .003718rad at599. Do not call
  the first recovery sustained. Evidence rain-episodes/ordinary-cone-followthrough.json.
- Current CCD/mesh-generation600step health-only captures7043/59391 now finish0
  on both paths: child/launcher0, ordinary5326.86s/native5321.51s,600validated
  frames and1252800spherical observations each. Outputs rain-candidate-health/MODE.
  Frozen fixtures predate status/overflow/capture corrections; no full-state trace
  or repeat/performance claim. CPU-relative screens32306/98170 finish1; absolute
  scans92515/28460 finish0. Both path screen reports are identical:501anchor,
  2444angular,5158cone,966lower-twist and327upper-twist failures.8400creations,
  1948800joint observations; noNaN/exploded/capacity-loss frames; identity/recycling
  passes. The previous95frame sleeping-twist episode2379 is absent from the
  >=5frame candidate episode list; none of those candidate episodes includes
  sleep. Longest is62awakeframes355–416,endpoint1693,peak.0555674881rad.
  Severecone5004 remains1.26105142rad at516; the515–522episode matches baseline,
  later recurrence529–534 peaks.15914005rad. Evidence comparison-summary.json.
  Prioritize severe-cone acceptance; old twist load reconstruction is historical,
  not evidence for this changed trajectory. No Rain capture/analysis jobs remain.

Severe-cone bounded follow-up: current ordinary health selection87465 finishes0,
validating whole600frame source. All42cell p/q/v/w/awake and target joint health
match baseline exactly at514–525; first cell/target divergence526. Evidence
rain-candidate-health/cone-health-equivalence.json. This does not prove equality
of hidden contact state. Baseline cone geometry reconstruction finds onset515
axis rotation72.4946degrees, endpoint angular speeds105.254/41.095rad/s and
final outward cone rates38.286rad/s on frozen axis vs129.342 on end axis. At520
axis dot isnegative (-.33146) and projected rate sign differs (-59.248 vs+21.618).
Both GPU and upstream CPU use step-start swing axis/mass but updated angle.
Two analytical-vs-central-difference controls pass below7e-11; float64 angle
reconstruction differs from health by about5.24e-5rad (not bit-exact).
Artifact rain-episodes/ordinary-cone-geometry.json. This identifies a concrete
fixed-axis approximation hypothesis, not physical acceptance or proof of cause.
The bounded axis/mass replay below tests this hypothesis and retains its tradeoff. Do not
reuse the baseline excerpt past525 as current-state evidence. No live jobs.

Cone-axis experiment completes; no solver change retained. Evidence:
desktop-baseline/cone-axis-experiment/{summary.json,restoration.json,*-solve.wgsl}.
Corrected CPU and ordinary/native baseline/candidate120step replays all finish0;
42initial getter vectors match float32 input bits,5040body/3240spherical records
validate. Baseline targetcone1.26054549rad/lower-twist.356199682rad; refreshed
axis/mass yields.983743727/.0900032818, but targetanchor peak grows.0697196573m
to.0835709944m and upper-twist.00343292952 to.00924003124. Ordinary/native metrics
match. Mechanical energy remains below initial in both; this omits motor work
and elastic storage and uses immutable source mass/inertia, so not an energy
acceptance proof. Thus fixed axis contributes, but refresh is not a sufficient
fix and no acceptance threshold changed. Do not repeat this same candidate.
Fresh contact/joint histories are an explicit isolation limitation.
Initial GPU baseline/candidate fixtures ALL failed signal-11 AFTER physics loop:
missing whole GPU samples API caused DestroyHuman to call CPU joint destruction.
Core/gdb backtrace and failed outputs retained; corrected linkage fixes cleanup.
Initial CPU link also needed DNDEBUG for Release archive. None of those failures
counts as a pass. Corrected links74090/4438 and runs44387/9293/86809/89211 finish0.
Original shader restored byte-for-byte; restored library builds48326/78456 finish0
and hashes recorded. Libraries now include v20/body-center validation; native
sample executables still predate them. No live or SIGSTOPped jobs remain.

#### Retirement/status alias fixed; capture follow-up

Source audit finds retire_contacts writes [a,b,mode] at query64–66 while query66
also stores sticky contact-drop reasons. Focused regression
contact_retirement_preserves_sticky_failure_reasons proves ordinary failure101:
[(shape,0,1),(pair,0,0),(shape,512,1),(pair,512,0)], with correct selected-root
retirement. Before ordinary build18298 and native build17465 finish0. Before
native regression66375 also finishes101 with identical four mismatches. Sources/hash
manifest and logs are in retirement-status-fix.
Fix moves command words to32–34 through matching Rust/WGSL constants,
keeping ray0–31 and sticky66 separate. After ordinary build20393 and native93510 finish0. All five matching retirement
checks61699/47763 pass per path (104.64s/101.31s), including new reason guard,
slot-reuse capture, hash retirement/reuse, child ownership and occupied publication.
Before/after binary/source hashes and exits are in retirement-status-fix/manifest.json.
This is five checks each, not five-process qualification. Test binaries include
this fix; current release rebuilds are recorded below.
The same-step follow-up also reproduced a real missed failure: direct GPU stats
showed broken-chain retirement failure while completed wait reused clean metrics
at unchanged physics_step. Before builds61597/15156 finish0; ordinary89087 and
native20494 fail101 with the same error. Frozen evidence: same-step-status-fix.
The correction tracks contact_status_dirty for retirement, shape remapping and
hash republication. Fresh status copy/map clears it; harvesting an older pending
slot does not. Explicit finish forces refresh while dirty, and capture rejects
pending or dirty status. The flag is captured in GPU policy.
After builds3437/58697 finish0; sequential test drivers47471/67857 finish0.
Both paths pass six retirement tests (including empty/older-pending same-step
cases), one undrained capture check and one metrics step-identity check. Manifest
records before/after hashes and exact exits. These are eight focused checks per
path, not five fresh-process full-state qualification.
The running600step Rain health fixtures predate both status corrections. Current
release rebuilds are recorded below; do not replace running frozen binaries.
Query audit records the refresh contract; clear/reset and simulator recreation
follow-ups are recorded below. Whole-state closure remains open.
The clear/reset regression clearing_unread_retirement_failure_cannot_rehabilitate_physics
now reproduces the failure on BOTH paths (before builds39647/51807 exit0; before
tests exit101). Real broken-chain retirement followed by clear before any status
harvest leaves physics incorrectly valid. Frozen evidence/hashes: clear-status-fix.
Candidate drains completed contact status before clearing host/device diagnostics,
marks the cleared status dirty for refresh, and immediately propagates simulator
invalidity to the world. After-build sessions56039/68148 finish0. First after suites87337/94498 exit101:
terminal invalidity and host-clear checks pass, but the test wrongly expected
last-step counters to clear as well. Preserve logs; corrected assertion now checks
only sticky device counters and first-failure step, matching this API contract.
Corrected-test builds83289/53206 finish0; production candidate unchanged.
Corrected retirement_ plus undrained-boundary drivers51199/67997 finish0:
seven retirement tests and one boundary test pass per path (105.72s/109.26s retirement).
Existing capacity_loss_ suites pass2tests per path (native48856 terminal0).
The regression covers empty and older pending slots and requires terminal invalidity
plus cleared host/device sticky counters after final readback. Ten focused checks
per path pass; manifests preserve all failures and before/after hashes. No full
qualification claim. Current release rebuilds are recorded below.
Growth regression contact_growth_preserves_unread_sticky_failure_reasons reproduces
loss on BOTH paths: atomic failure counters and first step survive, but reason0x40
becomes0 after body growth257. Before builds6747/63804 and frozen tests finish0/101.
Artifact growth-status-fix preserves sources/binaries/failures. Candidate copies
query66 alongside atomic sticky counters and marks the transfer dirty through the
shared submission helper, covering old failures not yet harvested. After-build
sessions62572/81416 finish0. Frozen after-test drivers6023/37250 finish0: new growth regression1, existing
contact-growth1, retirement7 and capacity-loss2 tests pass per path. Before/after
hashes and exact exits are in growth-status-fix/manifest.json. The test covers body growth257 and8193, with
unchanged/grown pair capacity, checking device and harvested host reasons and
terminal invalidity. Query audit now includes allocation transfer and distinguishes
component-query growth (already copies the entire old buffer). No full qualification
claim; the current release rebuild is recorded below.

Pair-generation/radix/allocation source audit is recorded in pair-workspace-state-audit.json.
Existing full-width radix and stale-compaction checks pass on frozen current
binaries (94678/31215 terminal0), covering u64 key order/digit boundaries and
poisoned compaction workspace including empty input and group boundaries. The concrete capacity omission found here is now fixed: collect_fat_statics had
ignored out>=pair_cap without recording loss.
Boundary regression fat_static_list_overflow_records_capacity_loss reproduces
silent loss on both paths at prior65536 (before tests exit101). Initial before
builds18484/83735 failed on a WGSL-only constant name; corrected builds75525/57125
finish0. All logs/binaries retained in static-list-overflow-fix. The test seeds the
already-filled prefix count, then runs one actual collector dispatch: last-slot
acceptance and no out-of-range writes pass; overflow falsely reports clean.
Candidate reports lost large-static entries through existing insertion-loss counters.
After-build sessions68168/66979 and frozen test drivers62021/95981 finish0.
Boundary1,80step matrix/grid mutation1 and capacity-loss2 tests pass per path.
Manifest preserves failed before cases and hashes. Full qualification remains open.
Release library plus sample rebuild drivers17836/74740 finish0, logs and source/
library/sample hashes in static-list-overflow-fix/production-builds.json. These
include all status and overflow fixes. Two-step sample Rain smoke checks52859/10114
finish0 with child/launcher0, two valid health frames and540spherical observations
each, zero NaN/exploded frames and empty GPU failure. Frozen fixture hashes match
the rebuilt samples (status-overflow-smoke/summary.json). Integration checks only,
not Rain physical acceptance/performance/repeats. The live600step Rain fixtures
remain frozen pre-status/overflow builds; do not overwrite or restart them.
The pre-fix80step matrix/grid mutation checks61652/96721 pass (exact pair/schedule
and p/q/v/w within1e-5 through filters, replacement, sleeping/waking and growth).


Pose readback audit now records staging/slot metadata exclusion only at the enforced
completed world capture boundary (pose-readback-state-audit.json). Logical pose
epoch/step, automatic policy and host-edit epochs remain captured. Existing boundary
rejection and policy-switch tests pass both paths (97840/76919 terminal0); no
production changes. This is not a whole-simulator or mid-flight capture proof.

Current source inventory regenerated locally:210GpuSim fields (101pipeline declarations),
with source hash in gpu-sim-field-inventory.json. Exporter reference presence remains
explicitly insufficient proof (54fields);12device-data owners are tracked for region/field
consolidation. Body, joint and separate CCD progress is recorded below. Additional source contracts classify general staging, completed wait
index and separate render-copy/export outputs in readback-output-state-audit.json.
These do not establish renderer correctness or arbitrary mid-flight capture.

SimParams source audit verifies all63meaningful fields against Rust/WGSL order,
unique capture keys and exact numeric encoding; the64thword is invariant padding
(sim-parameters-state-audit.json). Device body review found a concrete omission:
BodyColdGpu.local_center is dropped by the normal BodyGpu export while capture
used only the host value. Device-only mutation regression fails101 on BOTH paths
(before builds66561/47201 finish0, native test13809 terminal101). Frozen sources,
binaries and logs: body-center-capture-fix. Candidate reads actual device centers
in the existing extras readback and rejects any bit mismatch before appending.
After builds42383/94491 finish0; drivers75179/48283 finish0. Both paths pass
the mutation/NaN/recovery regression plus mass/center mutation, force clear and
body-slot reuse captures (four checks each). Hashes/exits are in manifest.json.
body-record-state-audit.json maps live hot/cold/extras fields; inactive slots and
island membership remain separate contracts. This is a capture validation change,
not a solver change. Release samples and frozen Rain
fixtures predate it. Prior captures lack this consistency proof.

Contact identity audit found all four feature/triangle lanes affect warm-start
mode selection even below point count4. Schema v20 now captures complete
feature_id_words/point_triangle_words; comparator validates lengths/u32s and
active-point consistency. Readers accept legacy v19 as separate historical
coverage; mixed schemas do not compare equal. Both builds72698/54574 finish0;
actual device-only inactive-lane mutation and3frame lifecycle tests pass/path.
The old contact projection hides both mutations; new fields detect them. The
capture helper also increments idle epoch, so no all-old-groups-equal claim.
All42Python checks pass (initial discovery found0tests/exit5; reran scripts
explicitly). Evidence contact-identity-capture-fix/manifest.json. Release samples
still predate body-center validation and v20. Full state coverage remains open.

Joint record audit now accounts for all43JointGpu fields:36direct,2body identity
references,2motor-only legacy lanes in _pad2 and4unused padding fields. Nine
constructors append complete records; destroyed slots become JOINT_NONE and
are excluded by consumers, with count/live slot identities retaining holes.
Evidence joint-record-state-audit.json. Existing active-motor and legacy-padding
controls pass both paths on frozen v20 tests. Separate ConvexCcd buffer audit
maps all semantic config/geometry/index/start-transform fields to actual readback;
unconsumed vector/padding/start-body lanes are excluded by explicit shader reads.
set_convex_ccd invalidates replay; normal/native sequences retain capture before
correction. Geometry corruption and reference validation pass2checks/path.
Evidence convex-ccd-state-audit.json, joint-record-audit/*logs. Four checks/path,
not fresh-repeat or physical CCD qualification. No new production change or jobs.
Retired-contact/reset and chain allocation contracts are now recorded in
contact-storage-state-audit.json. Four checks per backend pass on frozen v20
binaries: dirty range, lowest-slot/mesh free pool, deleted-body chain retirement
and corrupt-chain rejection before impulses. Driver35771 finishes0. Initial
wrong module filter selected0tests and was rejected/preserved separately.
Reset values matter: store_pair reads previous.color even for a reused inactive
root, while physical generation is deliberately preserved and captured. No
production code changed. Complete child-slot consumer equivalence, remaining
scene geometry and shared workspace remain open. Live assembled Contact mapping
now covers all47fields with count/reset contracts (live-contact-record-state-audit.json).

New discriminator: child_placement_changes_next_root_free_slot passes both paths
after correcting reserved WGSL identifier patch to piece. Identical root, logical
chain, high-water and counters10–14 yield next free root2 vs1 when child moves
from1 to4. Initial101logs preserved; builds93290/21606 and corrected tests finish0.
Evidence contact-storage-audit/placement-result.json. This proves the existing
fixed-counter/child-membership view omits a future allocator input; do not promote
it. Historical occupancy scan finds ordinary frame137/run2 differs at slots120/122.
Native first differs at139/run4, slots154/156. Both full600frame scans of five
runs finish0 (7031), with input hashes in contact-storage-audit/occupancy-result.json.
No live jobs remain. Need exact occupancy
at minimum and a full child-consumer proof, or deterministic child storage, before
claiming complete repeated semantic state. No production solver change.

Integrated child-compaction candidate now lives in src/contact_compaction.rs and
shaders/contact_compaction.wgsl. Four GPU passes snapshot payloads, scan child/free
counts, map logical children and publish canonical non-root destinations while
leaving roots and physical counters fixed. Encoder clears the work header first.
Production hook runs AFTER graph cleanup/retirement, before islands/preparation,
in both ordinary and native graph-cache exits; callback finish shares that boundary.
Mesh-free scenes skip it. Full native replay already excludes meshes. Snapshot
storage/binding cost is recorded; overhead remains unmeasured.

Error path now records query reason plus per-step/sticky contact loss and first
step from current GPU SimParams/pass_lut. Isolated tests pass both paths: spans
8/513/65537, distinct placements, exact payload/counters, idempotence and malformed
ownership; empty/high-water/live-above-high/orphan/cycle/truncated-chain controls
prove no partial publication and terminal host invalidity. Sources/reset contracts:
child-compaction-candidate/integrated-source-audit.json. Status tests37411/89900
finish0 (two tests per path). Final integration builds25337/5603 finish0.

Initial integration test47874 fails101 because it required an old root slot remain
empty; canonical child placement occupies it. Failure and old fixture retained.
Updated fixture requires non-root ownership and the exact original physical
counter; all original unread-handle validity/retirement checks remain unchanged.
Final driver27743 finishes0: BOTH paths pass11checks (isolation2,lifetime7,
16step joint-history1, convex callback-order1); all16 exported history frames per
path have canonical child slots. Inventory now211fields with transient compaction
owner/reset contract. Five fresh16step process repeats per path finish0 as
driver75841 under child-compaction-history-five: BOTH raw comparisons pass all
five. These are selected fixture repeats, not full ragdoll qualification.
See child-compaction-candidate/integration-result.json and integration-MODE-*logs.
No full repeat/physical/performance gate is claimed. Both release libraries now rebuilt (3886/66674 exit0); both sample executables
relinked (36788/40357 exit0), hashes in child-compaction-candidate/release-manifest.json.
Standalone ragdoll fixtures rebuilt (58248/70569 exit0), frozen hashes/source patch
and new implementation files in ragdoll-child-compaction-candidate/MODE.
All five600step fresh runs per path complete with child exit0. Aggregate runners
30729/32797 retain exit1 for raw occupied-root append order at frame1/run2
($.contact_allocation.occupied_order[124][0][0],50 vs20). Audited comparisons
71893/15489 finish0: all600frames/allfive runs pass independently on BOTH paths,
normalizing only the previously audited occupied-root/previous-touching memberships.
Physical slots, all physical generation counters, solver order and all other
captured fields remain exact. This closes this scene's child-placement repeat gap,
not the whole persistent-state audit. The old child-membership diagnostic is not used.
All ten original physical screens pass; check_completed.py records input hashes
and per-run reports, with physical-summary.json and MODE/audited-result.json under
ragdoll-child-compaction-five/. No comparison or GPU jobs remain live.

Mesh callback integration also passes one existing test per path (driver61900):
pre_solve_veto_disables_all_mesh_patches_and_can_reenable_at_rest. It verifies all
patches vetoed without events/motion, then re-enabled two-patch/one-public-pair
contact and begin event, exact API normals and query-in-callback completion.
Hashes/logs: child-compaction-candidate/mesh-callback-result.json. This closes the
selected integration check, not five-process full callback qualification.
Scene geometry/material source audit now maps all31ShapeGpu and9SurfaceMaterialGpu
fields, live device geometry arrays, exact mesh metadata and physical mixing-table
slots. Source hashes/contracts: scene-geometry-state-audit.json. Public shapes
always have >=1material; the count setter rejects zero, so shader max(count,1)
does not hide a reachable normal-path material. Color/reserved fields have no
physics consumers. Existing uploaded mesh-tree corruption, convex geometry/span
corruption, and slot-reuse/material/device-geometry controls all pass on BOTH
paths (three each, driver32543 terminal0; scene-geometry-audit/result.json).
No production/schema change. This is completed public-API capture scope, not
arbitrary malformed low-level simulator input or whole-state proof.
Command-cache/idle policy audit now classifies18more GpuSim fields, leaving36
reference-only policy entries and12device-region owners for final consolidation.
Source/key/invalidation contracts: command-policy-state-audit.json. Cached commands
retain resource owners and use current captured/reset buffers; cache keys and
binding validity are captured. Idle context,proof,chain,epoch and pending logical
step are captured, with completed capture requiring drained status.
Existing checks all pass (driver23306 terminal0): five idle tests per path,
native owner/queue lifetime and growing/shrinking reset-span tests. Five fresh
native48step full-replay/reentry traces compare raw-identically; each records42
actual replay hits after substep changes and transform upload. Frozen binaries,
source/trace/log hashes and result.json/replay-result.json are in command-policy-audit/.
No production/schema change and no live jobs. These close selected current-build
replay evidence and bounded policy contracts; they do not prove the remaining
workspace or Rain physical gates.
Host policy consolidation now maps the remaining36reference-only fields to their
exact capture keys/derived allocation contracts and consumers in
host-policy-state-audit.json. Inventory has no reference-only host entries; this
is not completion of the12device-region owners. The optional graph memo offset
is derived from captured shape_base_u32/36 actual body capacity and contact slots;
null versus invalid cache object distinguishes absence from allocated invalid cache.

Island/component workspace contracts are recorded in component-workspace-state-audit.json:
per-live-body island init before union/wake, ready reset before sleep, component
count/start/list regeneration, invalid-path guards and capacity-separated graph
memo. Existing three checks per path all pass (driver95338 terminal0):90step
merge/split with sleep enabled/disabled,120step small nonempty overflow,120step
large nonempty overflow. Their current-vs-global comparisons retain1e-5 limits,
actual overflow and component/sleep assertions. Evidence component-workspace-audit/.
Persistent history audit found a concrete omission: ordered_contact_transitions
reads contact_history_slots to select logical IDs, but v20 did not capture it;
record generations were reduced to generation_matches. A real device-mutation
regression fails101 on BOTH before exporters: lookup_distinct=false and distinct
stale generations also compare equal. Frozen before sources/binaries/logs and
hashes: history-capture-fix/manifest.json. Before builds51620/74107 finish0.
Schema v21 now captures complete physical slot-to-rank lookup through contact
capacity and each occupied history record's physical_slot/saved_generation.
Readers validate new fields and keep legacy versions distinct. Both after builds
20387/26578 finish0; mutation/restoration regression passes both paths. Five fresh
16step joint/contact-history traces per path compare raw-identically with v21
(history-capture-five/, driver80092 exit0). Python storage10/launcher9 checks pass;
initial synthetic fixture errors from replacing required policy keys are corrected.
No solver/default changes. Release/sample binaries and full ragdoll/Rain captures
still use v20 and do not establish the new history fields. No live jobs remain.
Joint schedule/remap review now recorded in joint-schedule-remap-state-audit.json.
Joint heads/counts/offsets/lists are regenerated before solving, with current
ordered schedule captured; persistent joint colors and contact history stay
captured separately. Remap mapping is fully written before tracked clear/remap/
prepare/publish submission; resulting pairs/hash and previous-touching protection
remain captured. Three existing checks per path pass (driver11933 exit0):24joint
exclusive ordered lists,80component fused-vs-general wave, first-joint contact
color inheritance. Logs/hashes joint-schedule-audit/. No code edits or live jobs.
Between-step retirement omission now reproduced on BOTH v21 exporters. Removing
the sole active candidate prevents selected retirement; restoring it enables
retirement, while v21 contact allocation remains identical. Both before tests
fail101 on the expected final capture assertion; before builds60360/2693 finish0.
Frozen before source/binaries/receipts: retirement-capture-fix/.
Schema v22 candidate adds exact candidate_unique_count,candidate_contact_count
and physical candidate_slots to contact_allocation. Length covers the larger
count, bounded by pair capacity; EMPTY entries retained. Readers validate these
fields and keep historical versions distinct. Python storage13/launcher9 pass.
After test builds22407/46169 finish0. Corrected retirement mutation regression
passes BOTH paths, preserving before failures and successful restored retirement.
Five fresh16step history traces per path compare raw-identically under v22
(retirement-capture-five/, driver96477 exit0). Complete source/binary/log hashes
and receipts are in retirement-capture-fix/manifest.json. No live jobs remain.
Release/sample binaries and full-scene traces remain v20; v21 history/replay
results also do not cover new candidate fields. Do not promote them to v22.
Fat-transform successful-path consumption review is now recorded in
fat-transform-state-audit.json with current source hashes. Every successful
ensure_sim reaches write_params, disabling the old batch before any new upload;
host pending commands and bound state remain captured. Epochs do not wrap, so
captured applied/initialized equality flags preserve reachable future behavior.
Zero-duration public steps return before consuming pending host transforms.
This proof is scoped to completed public boundaries, not direct GpuSim calls.
Failure-path review found an exception requiring a focused regression:
ensure_sim's caps.validate_allocation, GpuSim::new and pack_scene_bytes errors
set gpu_fail but not world/simulator physics_invalid; new-sim failure restores
the old simulator. step_gpu_inner checks physics_invalid after preparation,
so these errors do not establish the absorbing boundary assumed by the audit.
The bounded regression now reproduces rejection without invalidity on BOTH
before builds (75076/56924 finish0; tests exit101 on expected assertion).
All three error returns now mark world and any retained simulator invalid.
Both after builds28884/70944 finish0; rejected_scene_preparation_is_terminal
passes both paths, including capacity-error clear, smaller retry, simulator
creation with inherited invalidity, and unchanged physics_step=0. This directly
tests host heap rejection; construction/packing failure branches are source-
reviewed, not separately injected. Related initial-joint allocation (order=1)
and joint-filter body growth (order=0) pass each path; driver37724 terminal0.
Initial driver61668 failed because the order-storage test requires order=1;
its wrong-order failure is preserved. All evidence and frozen binaries are in
preparation-failure-fix/manifest.json. No live jobs remain. Test binaries now
include this failure correction; release/sample binaries remain older v20.
Device-region consolidation is now recorded in device-region-state-audit.json:
all12owners and10scratch partitions linked to individual consumer contracts,
with current source hashes. GpuSim inventory verified211fields, no additions or
removals. Source coverage is scoped to completed public World boundaries,
GPU-native order_enabled=0, selected flags0ordinary/131072native; no claim for
CPU-order tree state or additional diagnostic solver modes. v21/v22 history and
retirement fixes, exact child placement, filter validation, disabled body holes,
diagnostic output-only regions and preparation failure are incorporated.
No new tests or whole-goal pass inferred from consolidation. Older v20 full-scene
captures remain insufficient. Release library rebuilds36221/91946 and sample
links46988/60064 all finish0. Both libraries, sample executables and latest test
binaries are frozen with hashes in v22-candidate/manifest.json. Bounded Rain
startup capture check (five fresh2step processes, not600step qualification)
ordinary16427 completes all5runs with verified receipts: audited comparison
passes, raw comparison fails occupied_order at frame1 (aggregate exit1 retained).
Native50367 also completes all5runs: audited pass, raw occupied_order failure
at frame1, aggregate exit1 preserved. Every child/launcher receipt is0. Scope
checks verify all10frames per path are v22/order_enabled=0 with selected flags.
Outputs v22-startup-five/{ordinary,native}; logs v22-startup-{ordinary,native}.log.
No live jobs remain. This is startup validation, not600step Rain qualification.
Frozen v22 full Rain batches launched: ordinary63604 and native15392, each
five fresh600step runs. Both input sample hashes verified against v22-candidate
manifest. Outputs rain-v22-five/{ordinary,native}; driver logs
rain-v22-{ordinary,native}.log. They use separate frozen binaries, exact child/
launcher receipts and resumable runner. Poll these handles before replacement;
do not infer completion from partial frames. Concurrent runs are correctness
captures, not controlled performance measurements. Physical residual acceptance
remains independent of repeatability. No solver equations/default changes.
While those jobs run, a distinct contact-island discriminator now uses actual
resting frame65 geometry/mass in a float64 normal-only single sweep. Farthest
opposite-point pairs solved as two-variable complementarity blocks give zero
spin; scalar sequential gives0.148119rad/s biased /0.167582relaxation. Block
energies0.25003547/0.25J remain below initial0.5J, with exact linear momentum;
four active-set controls pass. Evidence contact-block-discriminator/{check.py,
result.json}. This is NOT a timestep, three-body/friction/warm-start replay or
physical pass. It is distinct from the rejected reversed-relaxation experiment.
Next bounded solver experiment: paired normal-contact blocks on full73step
contact-island fixture, keeping original spin/drift and conservation limits;
handle nonzero cached impulses, unilateral active sets and degenerate pairs,
then test resting support/friction before considering retention. No production
shader changed for this calculation. Full Rain batches remain live above.
Paired-contact GPU experiment now in progress: solve.wgsl temporarily adds a
two-point normal complementarity block, farthest-point pairing for4point
manifolds, accumulated-impulse softness, and scalar fallback for ill-conditioned
blocks. Ordinary test build96567 completed0. Backup/candidate shader, manifest and
runner in contact-block-experiment/. This is not retained or qualified; it
currently affects all ordering modes. Original73step spin/drift and all other
fixture limits are unchanged. Frozen Rain binaries remain unchanged and live
(63604/15392). Restore exact before-solve.wgsl if experiment fails; do not promote
this temporary source as the frozen v22 candidate.
Ordinary runner3474 finishes with exactly1passing test, childexit0, full73frames.
All original screens pass: peak spin7.45058e-9rad/s, transverse2.71538e-10m,
energy0.167527016J, momentum error5.96046e-8, angular momentum6.32216e-10,
axial overlap4.87566e-5m. This is one candidate run, not5repeat qualification.
Native build84300 and runner25211 finish0; exactly1passing test and73frames,
with all physical metrics identical to ordinary. Related driver30466 finishes0:
unchanged240step friction and600step support/energy fixtures each pass exactly
one test on both paths. Frozen experimental ordinary test hash44d5a7e214560dbd1e36917f57d55c11688bcc0959c801ad445c01b29aaed3a3;
nativee035b52302be3180f386f16a5434e1dee4e83ce9dd24f03522dfedcfa4964824.
Five fresh73step contact-island runs per path now all pass original assertions
and compare every raw v22frame exactly (contact-block-five/, driver97634 exit0).
These use the ungated experimental shader; default preservation and wider final
matrix remain open. New direct GPU paired_normals_cover_unilateral_cache_and_degenerate_cases
test covers independent analytic two-point results for four unilateral active
sets, retained warm impulses, softness, speculative release, fixed-point reentry,
coincident-point rejection and zero effective-mass rejection without partial
updates. Builds33350/73535 finish0; direct controls pass exactly1test on each
path (runners96786/50825), artifacts controls-{ordinary,native}*.
The candidate is now gated by SOLVER_PAIRED_NORMALS (1<<18), carried in captured
params.diagnostic_flags, enabled only when GPU_PHYSICS_LIVE_CONTACT_ORDER=0.
Default/unset and CPU-compatible1 retain scalar point order/solve. Diagnostic
overrides preserve this creation-time policy; controls now assert preservation.
Gated test builds65728ordinary/83592native and gating driver4998 completed0.
Both modes0 pass fixture/controls; each mode1/unset full73step trace is byte-
identical to frozen pre-block v22, preserving expected original symmetry failures.
Frozen gated binaries are contact-block-experiment/gating/{ordinary,native}/candidate;
gating/result.json has hashes and receipts. Final gated five-repeat campaigns
for contact-island73, friction240 and support600 all complete via
contact-block-experiment/repeat-gated-all.py, handle99006 exit0. Every one of30
fresh processes passes exactly1test and all original physical assertions;
all six5run raw v22comparisons pass at full duration. Outputs are
contact-block-gated-{island,friction,support}-five. Retain paired solving as the
GPU-native opt-in candidate for broader qualification; this closes the selected
contact-island original-screen/repeat gate on these frozen gated test binaries.
Ordinary fe4e6937eab5a6d4a5b5d5a29bc8652e4601cc9f2ee9689f3a2d0b164683909e;
native56c05f7899db29e8d55979c815f0fcbac049f4db00ba6a51e33d210765402fe8.
No experimental jobs remain. Release libraries/samples and live Rain v22 batches
still predate paired solving. Next: rebuild/freeze paired sample candidate and
assess Rain physical residuals on it, then remaining fixed matrix/performance.
Selected gated flags become262144ordinary/393216native; previous flags0/131072
and five-repeat results describe the ungated experiment/pre-block baseline.
Rain remains live
on frozen v22 binaries; production experiment has NOT become the qualified candidate.
Rain physical acceptance remains open; the selected contact-island gate is closed
on the gated test binaries above. The password/Xvfb reply revalidated working
user-local Xvfb but did not advance solver gates (no-progress for solver work).
Current continuation: paired release library builds69996ordinary/48507native and
sample links57470/98633 all finish0. New paired-v22-candidate/manifest.json freezes
six libraries/sample/test binaries plus source hashes and the amended device audit.
Its test hashes match the gated five-repeat binaries above. Paired sample hashes
are ordinary7f2759263b3bb3a4129a266313f44d21f45b37b6315288f5540915167364f1d6
and native79f671a6867bc23798a19d41b27f7838f11b67c92b35b843ac208a9b3d87c360.
New600step health-only Rain jobs26826ordinary/77334native use matching frozen
fixtures under rain-paired-health/{ordinary,native}; no state tracing or timing
claim. They preserve exact child/launcher receipts and original scene settings.
Paired restitution driver63634 completes0: five240step runs/path pass original
limits and exact raw v22comparison (paired-restitution-five). Apices0.99096805/
2.488538m, peak energy25.017302J/kg, penetration0.004999995m on both paths.
Follow-up19696 also completes0: motors120, mass mutations18 and joint-island73
each pass five fresh processes/path, all original assertions and raw v22 equality.
Outputs paired-{motors,mass,joint-island}-five; followup-exits.json records all0.
All40new processes pass completed-step/sticky-loss/invalid-state checks and
paired-policy checks (completion-policy-validation.json). An initial exact-flag
assumption failed because geometry-derived static-degree proof bit1024 is also
set; source set_topology_bounds confirms this runtime policy. Full comparisons
retain these bits exactly; they are not normalized. No solver/test changes made.
Frozen pre-paired Rain handles63604/15392 remain live; paired health26826/77334
also polled live. Next: remaining lifecycle/query/callback, native replay, CCD,
mass analytical/numerical and dragging/ragdoll final-candidate matrix, while
waiting for paired Rain health. Do not relaunch these completed four cases.

Next continuation made progress on the same frozen binaries: driver51035 exits0,
five history16/query8/callback12 runs per path all pass original assertions and
exact raw v22 comparisons; native replay five48step runs pass and all record42
actual hits. Outputs paired-{history,query,callbacks,replay}-five. All35processes
also pass completed-step, sticky-loss, invalid-state and paired-policy validation
(paired-v22-candidate/lifecycle-completion-validation.json).
CCD driver40428 exits0: five5step mutation captures/path pass original barrier/
reference assertions, raw v22 equality and completion/policy checks. Fixed
regression55676 exits0: per path28native_precision tests,27CCD tests, five
analytical mass/inertia tests, the400settle+1wake check, and five fresh unread
mesh-root lifetime runs pass (66test passes/path). These last unread checks
deliberately avoid intermediate harvesting; no per-step capture claim for them.
Evidence paired-v22-candidate/fixed-regressions-{ordinary,native}/results.json.
Paired ragdoll C fixtures now link the frozen paired libraries; builds20041/
24630 finish0 with manifests under ragdoll-paired-candidate/{ordinary,native}.
Single600step physical+state checks12785ordinary/35462native finish0, including
original physical screen. Both paths peak separation0.213310137m, tail0.00127359747m,
tail speed/angular0, peak angular103.627171rad/s within CPU-relative limit.
Five600step campaigns16267ordinary/36170native run under ragdoll-paired-five;
both finish their five child processes with exit0. All ten original physical
screens pass (physical-summary.json). Aggregate16267/36170 exit1 on raw occupied
append order at frame1; retain these failures. Audited comparisons1908/38749
both finish0/pass across all600steps/five runs, preserving physical slots/counters
and solver order. Completion/policy scan1571 finishes0 on representative runs;
comparison keeps these checked fields exact across each five-run group. Selected
paired ragdoll physical and audited five-repeat gates are therefore closed.
Dragging fixtures built85740ordinary/12347native exit0 against frozen paired
libraries and existing portable CPU/bridge archives; all input hashes recorded
in drag-paired-candidate/{ordinary,native}/build.json. Single original --ground
3060step state+physical checks44226ordinary/42316native finish1 on original
ground-drag limits. Ordinary maxposition0.3899m/quaternion chord0.351633,
heldposition0.147446m/heldvelocity3.08352m/s; native0.38982m/0.351611 and
heldposition0.147451m/heldvelocity3.08352m/s. Settled velocities0; keep failures.
No dragging five-repeat campaign started. Rain jobs remain live.
Discriminator: same fixture/bridge archives linked to frozen pre-paired v22
library, build46205/run52534, also fails (maxposition0.245738m, held0.152442m,
heldvelocity3.08363m/s). CPU-compatible order1 control52806 fails with identical
metrics. This is not solely a paired-solver/order regression. Both-engine trace
first exceeds1e-5m at frame227/body0: position difference0.016072364m while
velocity still matches; first-divergence.json retains values.
Diagnostic C-only wd.enableContinuous=false ablation (source copy under
drag-no-ccd-control/, production untouched), build50415/run87008, passes the
original gate: maxposition0.00602796m/chord0.00597248, heldposition0.00288055m,
heldvelocity0.047432m/s, peakvelocitydifference0.230405m/s, settled0. This isolates
CCD involvement, not qualification under disabled CCD. Evidence and5receipts in
drag-paired-candidate/control-summary.json. Next discriminating work: test the
earlier speculative-shell activation cutoff change directly and inspect first
clipped step (existing GPU_PHYSICS_TRACE_BODY diagnostic). Keep original dragging
and restitution checks; no limit/default changed or passing ablation substituted.
Current cutoff discriminator is isolated under ccd-cutoff-control/workspace/:
copied source changes only host activation from min(0.5*extent,speculativeDistance)
back to0.5*extent. No production source change. Library21984 and drag link19131
finish0; diagnostic44253scalar/order1 finishes0 with the original drag pass
(same0.00602796m maxposition as no-CCD control). Diagnostic61690paired/order0
aborts (-6) at frame2460 on original settled-body angular-speed<0.1 assertion:
body4 speed0.09601665m/s, angular0.13578371rad/s, y0.70661068m; other cubes asleep.
settling-failure.json records final complete state. Both kept continuous collision
enabled and both-engine traces. Restoring the old host cutoff alone is not an
acceptable fix: paired settling fails and original restitution coverage would
still require validation. This is diagnostic evidence, not retained code. GPU CCD
shader cutoff remains unchanged; this drag fixture uses host CCD (captured
convex_ccd=null). Frozen experimental lib and hashes are in ccd-cutoff-control/.
IMPORTANT: this copied-source build used shared target/; target/release library
outputs briefly described the diagnostic copy. Production-root rebuild24390
finishes0; restored static-library SHAaae48aa3232688f077b84839007c4f3706ca1cefb387497655a560da349036aa
matches frozen paired-v22-candidate exactly (production-restore.json). Use frozen paired-v22-candidate
libraries for qualification, or rebuild from production root before reusing
target/release. Production world.rs/solve.wgsl/sim.rs hashes still match frozen
paired manifest. Source defaults and acceptance thresholds were not changed.
Geometric first-impact check (unit cubes verified in fixture): at frame227/body0
CPU minY=-0.010831614m while GPU minY=+0.005000102m; both were+0.033612905m
at226. First-impact-clearance.json computes oriented-box support from actual
both-engine p/q, not a sphere approximation. Thus the initial position mismatch
is an earlier collision clamp with reduced penetration, not established tunneling
or instability. Later dragging divergence still fails preserved original screen.
Acceptance clarification asked asynchronously: objective permits different valid
CPU/GPU trajectories, while ground-drag screen requires25mm agreement through
impacts. User has not answered yet. Preserve existing required screens pending
clarification; no response is not authorization to waive them. Independent
physical checks continue; this is not a blocker for Rain/other work.
Old-cutoff paired release analysis finds +2.541764J/kg rigid-body mechanical
energy immediately after joint removal at2341, followed by decreasing energy and
near-edge toppling (angular speed rises0.0536 to0.1358 over final20frames). This
omits contact/joint stored energy, so is not a complete work-energy balance.
Comparison across existing runs finds roughly+4.01J/kg on drag1 even in the
passing scalar control. Both-engine scalar trace confirms CPU and GPU energies
match exactly across that release (8.66516597 ->12.67727927J/kg). Do not claim a
paired-only energy defect or treat CPU reproduction as physical acceptance.
Artifacts: ccd-cutoff-control/paired/release-energy.json and
drag-paired-candidate/release-energy-{comparison,cpu-control}.json.
Original contact-free drag fixture now passes on both paired paths (3296/77700
exit0):1320steps/ten grabs, position/quaternion/velocity differences exactly0.
Outputs drag-isolated-paired/{ordinary,native}; completion/policy validation23215
finishes0 for both full1320step captures (validation.json). This isolates picking/joint behavior from impacts and does not replace
failed ground or five-repeat gates.
Mesh recovery: the exact two critical triangle412 inputs (CPU169/GPU170) and
exposed-edge/covered-seam/fully-flat controls ARE tracked in native_precision.rs
and passed in the28-test run. Full historical45grid/12torus pose data remains
absent; don't confuse those direct shader controls with full-world57pose coverage.
Historical57mesh inputs (45grid/12torus) are absent locally; only probe source
c_abi/rain_frozen_contact.cpp and strict comparator are present. Searches of
artifacts, scripts and docs found no exact pose dataset. Do not count historical
results as a current candidate pass or silently substitute different inputs.
Portable local mesh baseline now completed independently. CPU and ordinary
Rain-cell172step captures generated right-calf682 poses166..171; this reuses the
documented frozen geometry protocol, not a new early-onset investigation. New
45grid inputs sample x/z=-1,0,1 and five yaw orientations at+5mm clearance, using
the actual upstream capsule geometry. Requirements were frozen before running:
exact contact/point counts, original1e-5normals/separations, allgridcasescontact
and upward normals. CPU and both GPU probes pass all57cases; grid maxerror
1.0245e-8, torus1.387e-7. Toruscase3 includes original exposed triangle412 point.
Inputs+construction/hash manifest are tracked under c_abi/fixtures/frozen-mesh/;
new scripts/check-frozen-mesh-reference.py freezes probes and preserves receipts.
Portable wrapper49135ordinary/16371native finish0 against committed inputs.
Evidence/buildcommands mesh-local-baseline/. Historical inputbytes remain missing;
this is explicit newlocalbaseline evidence under portability instructions, not
the historical runs. No solver/default/limit change, no repeat/dynamicRain claim.
Current evidence audit (paired-v22-candidate/coverage-audit.json) verifies all99
Rust/WGSL source hashes against frozen paired manifest, all25five-run case/path
groups with expected durations/receipts/results and test-binary hashes, plus
66fixed regression passes/path. It does not add a new physical pass or close
ground dragging/Rain/performance/delivery. Old pre-paired Rain first captures
last observed complete frames514ordinary/516native of600; handles63604/15392
and paired health26826/77334 polled live. Preserve these current captures; they
do not qualify paired solving. When first old-candidate runs finish and receipts
are safely complete, reassess the value of further obsolete-candidate repeats
before spending GPU time on them; do not interrupt useful nearly-complete files.
Next uncompleted work: ground-drag acceptance/fix and repeats, paired Rain
physical results and final repeats,
then controlled performance/recordings/default decision. Preserve all old fails.

#### Remaining acceptance and next actions

Latest continuation: saved paired ground-drag traces measured at every step.
Both3060frame captures pass original absolute settling limits at all50
cube/release endpoints, but minimum cube/floor clearance is about-44.60mm at
core527/body3 during held dragging. See qualification report and
drag-paired-candidate/clearance-settling.json; this is not a new acceptance gate.
Impact-window analysis finds empty manifolds at core526(+5mm) and527(-44.60mm),
then four points528; penetration lasts through540. Suspected empty-contact
recycling across the17.095mm CCD correction leaves discrete support absent,
while TOI skips its initial-contact shell. New test
ccd_landing_refreshes_empty_contact_inside_speculative_shell in world.rs tests
an unforced unit cube at22mm clearance/-2.66667m/s, second-step floor slop5mm.
Baseline build68818 finished0; regression78711 finished101, second-step center
y=0.4605555 (39.4445mm penetration), confirming the isolated defect. Frozen test
binary empty-contact-before-fixture and log empty-contact-before.log retained
under drag-paired-candidate/. Same binary with GPU_PHYSICS_AB=no-recycle runs
as8615 finished0, log empty-contact-no-recycle.log. Shader recycle_skip now rejects empty
manifolds: they contain no separation bound permitting safe reuse inside the
speculative shell. Patched build73933 finished0 and ordinary focused run3416
passes. Exact before/control receipts and frozen baseline hash are in
empty-contact-controls.json. Native test build77951 finished0; its frozen binary
empty-contact-fix/native-fixture runs the same regression driver as97716.
Ordinary library rebuild13563 finished0; frozen ordinary-libgpu_physics.a links
the unchanged original drag fixture via build-drag.py (44407 finished0).
Ground-drag runner36500 finished1; empty-contact-fix/drag-ordinary/ preserves
input hashes/build manifest, command, all3060steps and exact exit.
Native library build26975 and link21593 finish0; native-libgpu_physics.a frozen.
Native run-drag.py90117 also finishes1, with all3060steps in drag-native/.
Both retain original trajectory failures (maxposition .3899/.38982m), but
maximum geometric penetration falls44.60mm→18.49mm at1140/body5 and all50
release endpoints/path satisfy original absolute settling limits. Measured by
empty-contact-fix/measure-clearance.py, results clearance-settling.json.
Old bad frame527 now has four points and+0.0286mm clearance (ordinary), not an
empty manifold/-44.60mm. Remaining peak1140 has four points under a loaded
motor (vertical anchor error -0.68505m); release1141 clearance+2.835mm.
contact-motor-window.json records geometry/anchors; these are observations,
not a new physical pass. Next assess loaded motor/contact compliance rather
than revisiting the now-fixed empty-cache transition. Native focused test is
also confirmed passing within native/ccd.log. Both regression drivers23926/97716
finish0:28CCD+6recycling+3restitution passes per path. They ran selectors from
empty-contact-fix/check.py against frozen empty-contact-fix/ordinary-fixture;
per-selector logs and results.json retained under empty-contact-fix/ordinary/.
Next resolve loaded-drag compliance/physical acceptance and Rain. Frozen paired binaries remain
untouched; source hash audit predates these test and shader changes.
Do not dismiss failed ground dragging as trajectory divergence alone. Paired
Rain fixture PIDs882094/882144 verified running. Old pre-paired Rain first
captures now have child0/launcher0 receipts on both paths. No physics child
remained under drivers826815/826898; these superseded batch drivers were
intentionally terminated to suppress four obsolete repeats each (handles
63604/15392 terminal143). Existing receipts are unchanged. Dedicated
rain-v22-five/finalize-first.py validators1879ordinary/79885native now finish
validation/compression of the successful600step captures, without rerunning
physics; see superseded-batch-stop.json. No five-repeat Rain pass is claimed.
Health scans83827/77017 finish0 for these pre-paired captures:600frames/path,
zeroNaN/explosion/capacity loss, identical residual summaries. Severe cone
1.26105142rad remains at516/endpoints5003–5004; longest >.05rad twist episode
62awakeframes355–416 at1684–1693, matching prior baseline. Per-path
rain-v22-five/*/health-summary.json preserves full selections (not acceptance).
Independent saved CPU drag trace measures18.207mm penetration at1140 and
positive clearance at1141, versus patchedGPU18.491mm/positive at1141. CPU peak
over all15300bodyframes is22.358mm at526. Evidence
empty-contact-fix/cpu-clearance-reference.json. Similar loaded behavior narrows
the diagnosis to compliant motor/contact response, but is not a physical pass.
Patched drag state-health scans75281/39234 finish0: all3060steps/path have no
captured loss, invalid flag or completion mismatch. This does not erase the
physical-screen failures. empty-contact-fix/manifest.json freezes both test
binaries, both libraries and both drag binaries with99source hashes; only
world.rs(test addition) and collide.wgsl(empty-manifold veto) differ from the
previous paired candidate. source.patch preserves the current engine diff.
Persistent-state audit amendment: no new fields/caches; captured contact count
selects recycling eligibility and existing v22 geometry/history/order coverage
still applies. New trajectories require new repeats; prior passes do not transfer.
Cache-fix sample links49616ordinary/36282native finish0. Both sample executables
are now frozen in empty-contact-fix/manifest.json; source hashes and linked
library hashes verified unchanged. New600step health-only Rain captures
77061ordinary/10251native run under rain-empty-contact-health/{ordinary,native};
launched fixture hashes match the frozen executables. They use the existing
capture_candidate_health.py with clean runtime flags, exact child/launcher
receipts and lifetime health, no full-state/performance claim. Earlier paired
Rain26826/77334 remains live and predates the empty-cache fix; keep its results
as comparison evidence. Do not launch additional Rain batches while these run.
Latest completion: all six handles now terminal0. Old full-state finalizers
1879/79885 finish0 with verified600step gzip captures and completion manifests.
Paired health26826/77334 and cache-fixed health77061/10251 finish0, each600steps.
Absolute residual scanners now run: paired19415ordinary/80097native; cache-fixed
11118ordinary/51068native. Next inspect results and run unchanged CPU-relative
screens against rain-full/cpu, then choose the actual remaining episode on the
cache-fixed trajectory. No fresh-repeat or physical Rain pass is implied.
All four scanners finish0. Cache-fixed Rain peaks cone1.05444145rad at179/
33–34 (paired pre-cache-fix1.29449368rad at520/4961–4962), but now has a
persistent cone episode454–591 (138awakeframes), joint3942/endpoints3935–3943,
peak.360523075rad and final.20179686rad. Both paths have identical residual
summaries. CPU-relative scans47659/54383 finish1, preserving unchanged failures:
1194anchor/4449angular/15871cone/1656lower/1172upper across1948800matched joints.
Next diagnosis is this new persistent episode, not the old5004 trajectory.
Single ordinary bounded capture89833 is live under rain-empty-contact-cone/,
using frozen empty-contact-fix/ordinary-samples_gpu;600steps, core-state only
440..600 plus full health. capture_persistent_cone.py uses the existing hook,
no solver change, and preserves exact receipts. This is not a full-run repeat.
Selected health window449..591,42body cellbase3907,target3943 is being validated
by31512 under rain-empty-contact-episode/ using existing select_health.py;
selector finishes0 and confirms all42cell bodies across143selected frames.
Match identities and trajectory against completed health before trace analysis.
Extractor recovery: scripts/rain-episode-diagnostics/extract.py formerly assumed
line1=core1 and could not read the bounded440..600capture. It now uses recorded
frame IDs and validates contiguity. Synthetic full/trimmed inputs give identical
excerpts; missing/duplicate frames, incomplete windows, stale generations and
wrong poses all reject. Evidence: rain-extractor-window-check/results.json.
This is helper validation only, not a Rain pass. Capture89833/PID1216829 is
still live; core state begins440 and had complete frames through464 at the
latest inspection. Do not restart. Selected prefixes now validate against the
previous completed cache-fixed health: ordinary-onset-prefix.jsonl core450..454
and ordinary-impact-prefix.jsonl core450..460, all42cell bodies' q/v/w float32
bits and slot/generation identities match. Target core endpoint is4043, parent
4035, joint3942. Prefix extraction handles9367/11835 both finish0. These are
bounded diagnostic matches, not a complete run or fresh-repeat qualification.
impact-compliance.json reports core455 coneexcess0.360505rad (libm approximation),
stationary cached-impulse estimate0.117275rad, start/end swing-axis dot0.319209;
at460 they are0.169271/0.017178/0.617481. The cached stationary estimate alone
does not explain the moving impact. Next inspect later persistent frames as
they become available, then the complete449..591health window. Do not infer
that updated-axis-only is a valid fix from this impact diagnostic.
The next21frames core461..481 also match all42body identities/q/v/w exactly
(extractor24796 terminal0): ordinary-persistent-prefix.jsonl and manifest.
persistent-prefix-compliance.json shows coneexcess0.102552..0.204187rad,
stationary cached-impulse estimate0.005131..0.050761rad, start/end swing-axis
dot0.553345..0.956748. At481 values are0.192366/0.023576/0.553345.
Thus the effect persists after impact; stationary scalar compliance alone does
not account for it. Full contact-load/substep balance is not recovered by this.
Prior cone-axis experiment refreshed BOTH axis and effective mass, retaining
prepared inertia/scalar accumulation; it reduced cone but worsened anchor and
upper twist and was rejected. Do not duplicate it under a different name.
Next distinguish point/cone constraint coupling from that already-tested axis
refresh, using the current cached geometry; full449..591window still pending.
Point/cone coupling check now complete: rain-empty-contact-episode/
analyze_point_cone.py reconstructs a unit cone impulse followed by an exact hard
point projection using captured prepared inertia. Across core450..481, the
point solve removes9.14..23.98% of the cone-rate correction at start geometry,
or7.33..24.87% with end levers. Zero-lever and isotropic-unit-lever controls
recover0 and0.5 coupling; point-velocity residuals are below1e-9. This is a
bounded algebraic diagnostic, not an actual soft/substep solve. Own-point
coupling is not demonstrated as the explanation for the large persistence.
Next discriminator is launched as17223: rain-hip-load-replay/run.py, sequential
CPU/ordinary x collisions1/0,120steps each, from health480/base3907/target3943,
cellrow3/column9. Uses unchanged rain_cell_reference.cpp's existing collision
ablation, original joint/scene defaults,42body transforms/velocities and fresh
contact/joint history. protocol.json records scope and interpretation before
evaluation. Build93488 finishes0 for both fixtures; ordinary links the frozen
empty-contact-fix library. CPU arms both exit0; GPU arms pending. Preserve all
initial getter comparisons/body/joint records before interpreting recovery.
Replay17223 finishes0 for all four arms. Analyzer verifies42exact float32
initial getters,5040finite body records and3240spherical observations per arm.
With collisions, final targetcone CPU/GPU=.14410454/.183344901rad; tail60max
=.147624344/.210057288. Without collisions, both engines agree on final
.000124052167rad and tail60max.000211909413. Contact loading is necessary for
persistence in this fresh-history isolated replay; this is not acceptance.
summary.json retains anchors/twist/all-joint/energy diagnostics and raw hashes.
Captured target contact partners narrow further: at core470..481 its sole
active patch is against core4042, Human3942, its own left calf. At461..468
partners4014/4016 also occur, and469 retains4014. target-contact-partners.json
retains all21frames. Human thigh uses negativegroup but calfgroup0, so this
self-contact is allowed upstream. Next isolate this pair's contribution with a
bounded diagnostic before proposing any solver/default change. The original
full capture89833/PID1216829 is still live and must not be restarted.
Selective pair replay is complete: rain-hip-pair-replay/, build34912/run52431
both0. Artifact-only fixture adds the same force-free filter joint in both
arms, collideConnected true/false for Human3942↔3943. Per-step contact audit
finds120/120pair points in each sham and0/120 in each disabled arm. Exact42
initial getters and5040body/3240spherical records validate. Pair removal reduces
but does not resolve error: CPU/GPU tail60cone maxima .213215/.222990(sham)
vs .138892/.154004(disabled), finals .161817/.189009 vs .124223/.148645.
Sham trajectories differ from no-added-joint baselines, preserved explicitly;
do not claim this filter topology leaves ordering unchanged or that the pair
alone causes the problem. Only the matched added-filter arms are comparable.
Temporal-resolution discriminator also complete: rain-hip-substep-replay/,
build58550/run82934 exit0, CPU/ordinary x4/8/16substeps, same120worldsteps and
initial health480state. Original4substep body traces match prior originals
byte-for-byte on both paths. Tail60cone CPU=.147624/.197574/.220051 and GPU
=.210057/.194023/.219055; error does not vanish with refinement. All records
validate; summary.json retains anchor/twist/energy and receipts. Substeps are
diagnostic only; no scene/default/production solver changes. Do not repeat
these two failed simple explanations. Full capture still needs completion
and full449..591state matching; most recent complete frame observed528.
Material decomposition is complete under rain-hip-material-replay/:
build25250/run32689 exit0, CPU/ordinary x unchanged,zero-friction,zero-rolling,
zero-both,120steps from the same health480state.42material getters verify each
arm, all initial/body/joint records validate, and unchanged-material body traces
match prior originals exactly. With BOTH resistances zero (normal contacts still
enabled), target tail60cone max CPU/GPU=.00355715/.00345930rad, final
.00262496/.00326228. Zero-rolling alone remains .180940/.209618 tailmax;
zero-friction alone gives .130270 CPU but0 GPU. Original shams remain
.147624/.210057. Thus frictional loading supports persistence in the isolated
replay; pure normal-contact geometric impossibility is not demonstrated.
These changes are artifact-only diagnosis, not a proposed material/default fix
or original-material physical pass. summary.json/material-validation.json retain
all controls. Original-sham mechanical energy stays below initial945.864J over
120steps (max926.000CPU/913.371GPU, final803.848/802.153), but excludes stored
elastic/contact energy and motor work and is not a complete energy proof.
No more broad ablation arms are needed before analyzing the full captured load.
Capture89833/PID1216829 remains live at69m10s; latest inspected core551complete.
Cached friction-bound check on validated core450..481 excerpts passes:
1082active patch observations, no sliding/rolling/twist cap violations at
predeclared numerical allowance1e-5*max(1,cap). Normal cache is each rb.w, not
the accumulated total_normal_impulse field. Target thigh/calf at481 has normal
impulse1.055489Ns, sliding.63329344 vs cap.63329339, rolling.018998799 vs cap
.018998802Nms; both saturated within float rounding. Evidence:
rain-empty-contact-episode/check_friction_bounds.py and friction-bound-check.json.
This rules out cached over-limit resistance in those excerpts, not substep work
or original-material physical acceptance. Reuse the same check on the final
449..591excerpt. Capture remains live; last complete core564 observed.
Latest completion supersedes all live-capture notes above:89833 exits0, exact
child0/launcher0,4779.95s.600health frames/1252800spherical observations validate.
Full-window extractor85989 exits0: ordinary-full-episode.jsonl covers143frames
core450..592, all42cell q/v/w bits and identities match earlier completedhealth.
check_window.py/62226 exits0: all161core440..600frames have no sticky loss,
invalid state or completion mismatch. state-health.json stores fulltrace SHA256
e6cc054c0a0c4583e3caf6106c3afd2b0796e0bb1487bccb41f3672006b33c47.
No goal jobs remain live. This is one bounded diagnostic run, not five repeats.
Full episode analysis is full-compliance.json. At health500 coneexcess.087609
and stationary estimate.081915 nearly agree, but at591 they are.201786/.010188
with axisdot.591090 and angularspeeds1.30/6.77rad/s; do not apply a stationary
model to every moving step. Full friction-bound-check.json has6681activepatch
observations and22STRICT ideal-cap failures, all rolling; initial prefix pass
is preserved as prefix-friction-bound-check.json. All22are awake contacts, and
all satisfy the actual squared clamp deadband(max squaredexcess1.182890e-7 <=
FLT_EPSILON1.192093e-7), present in both solve.wgsl and upstreamcontact_solver.c.
rolling-deadband-explanation.json explains rather than erases the failed screen.
Target4042↔4043 has142observations and no strict cap failures. No acceptance
threshold/default change. Next must resolve physical acceptance under original
frictional loading; repeating the completed ablations or chasing rolling clamp
deadband as the demonstrated hip cause is not justified by current evidence.
Analytical cone-load reference now passes CPU/ordinary/native: build30657 and
run41403 exit0. Fixture retained at c_abi/spherical_compliance_reference.cpp;
evidence loaded-cone-reference/{protocol.json,results.json,baseline-*}. Static
anchor, unit isotropic inertia, coincidentCOM anchors, no contact/gravity/sleep,
default60Hz/damping2, four substeps. Torques1421.2229/14212.2295Nm predict
.01/.1rad cone excess through K=I*(2*pi*effectiveHertz)^2. Both600step loads
on all paths pass predeclared last120step angleerror<1e-4 and speed<1e-3;
actualworsterrors1.74226e-6/5.09452e-6rad, tailomega0. All1200records/path
validate. This closes the isolated stationary formula check, not moving Rain
acceptance or five-repeat qualification; no production solver/default change.
The existing compliance analyzer now also reports swing effective mass,
cached-impulse stationary residual, approximate libm cone excess and start/end
swing-axis alignment. A17frame historical cone excerpt exercises the fields;
an isotropic-tensor control verifies mass1 and0.02rad residual conversion.
Evidence: rain-extractor-window-check/{historical-cone-compliance.json,
isotropic-cone-check.json}. These are diagnostic checks only; await the current
capture, and do not mistake final cached impulse/h for a full torque balance.
Health-only motion analysis identifies this as the third Human's right hip:
3935pelvis→3943thigh_r (human.h14bone order, human.c right-thigh parent/frames).
Upstream cone10deg, twist[-30,60]deg, default base constraint60Hz/damping2;
no sample override. target-motion.json retains all143selected frame metrics.
At480 pelvis/thigh speeds .137/.183m/s and angular1.82/12.62rad/s; at591
.217/.299m/s and1.30/6.77rad/s, anchorerror1.956mm but coneexcess.2018rad.
Both remain awake throughout the138frame episode. It is not a sleeping-stall
case; next use the captured contact impulses/joint caches to distinguish load
compliance from angular-limit/point-constraint coupling. Do not repeat the
rejected historical updated-axis-only change without new evidence.
Angle measurement cross-check: independent composition of health body rotations
with normalized upstream thigh_r local frames gives the reported swing across
all143frames. Initial exact-atan2 check misses1e-5 tolerance (max1.8493e-5),
retained in angle-reconstruction.json. Source b3GetSwingAngle calls deterministic
b3Atan2's minimax polynomial, not libm atan2. Applying that documented polynomial
to the independently reconstructed angle gives max3.40054e-7 error at unchanged
1e-5 tolerance (angle-reconstruction-native-polynomial.json). Thus the persistent
cone error is real geometry, not frame/identity or measurement mismatch.
Next discriminating loaded-contact check is now implemented as test-only
constant_load_contact_compliance_matches_softness in world.rs. A rotation-locked
unit cube receives constant downward accelerations100/1000m/s² with unchanged
world contact defaults,600steps each. Four coplanar contact points each have
normal effective mass m, predicting equilibrium depth
(applied_acceleration-gravity_y)/(4*omega²), omega from the existing static-contact
hertz/clamp. Before evaluation, tail120steps require depth error<0.1mm and
speed<1mm/s. This tests the compliant model, not a replacement drag gate.
Ordinary test build51224 finishes0 and run60306 passes both600step loads at the
predeclared limits. Log empty-contact-fix/compliance-ordinary.log; frozen
compliance-ordinary-fixture plus SHA256 retained. Native test build14390
finishes0; test92484 finishes0 with the native runtime environment sourced.
compliance-native.log and frozen compliance-native-fixture plus SHA256 retain
the evidence. compliance-results.json records both successful paths, exact
exits, limits and executable/log hashes. Both600step loads pass on each path.
This validates
stationary force/compliance only, not moving off-center dragging acceptance. The
frozen sample/library binaries remain unchanged and source manifest predates
this test-only addition. No production force/softness/default modification.

1. Resolve dragging's CCD/solver interaction with the controls above; retain
   original drag, settling and restitution checks. A blanket cutoff rollback is
   insufficient. Both paired and cache-fixed600step Rain health runs are complete;
   inspect the current bounded cache-fixed core capture when its window completes.
   Keep original failed screens. New trajectories require
   new identity/residual analysis; reuse old cone windows only after verifying
   matches. Pre-paired v22 first captures and gzip finalization are complete;
   further obsolete repeats were deliberately stopped. Do not
   restart settled early-onset work or call CPU agreement physical acceptance.
2. Preserve the completed selected-boundary persistent-state audit, including
   v22 retirement candidates and the paired-policy amendment. The consumer audit
   covers the selected public World boundary/configurations; it does not qualify
   arbitrary midstep or low-level API states. Final repetitions must use the
   final cache-fixed candidate and retain exact physical storage checks.
3. Contact-island original spin/transverse failures are preserved as baseline;
   the gated paired candidate passes all original limits and five fresh73step
   raw-state repeats on each path. Friction240/support600 also pass five per path.
   Revalidate remaining fixed matrix cases on this candidate.
4. Freeze final candidate and finish fixed capability/repeat matrix. Then controlled
   small/large performance with no competing GPU work, snapshots/CPU column and
   default decision. GPU-native ordering remains opt-in; no performance run launched.

All evidence details, historical attempts, initial compile/selector failures and
source-consumer findings remain in gpu-solver-qualification.md and named artifacts.
Brief SIGSTOP of two repeat children during compilation was followed by SIGCONT;
those exact processes completed successfully and none remains suspended.

### Source-machine historical handoff

The user authorized committing and pushing this checkpoint. The solver goal remains incomplete. On the source machine, process inspection now confirms no qualification runners, samples or episode extractors remain. Old process handles below are historical. Ordinary Rain run1 has a `complete.json` receipt and compressed state; run2 was interrupted with only a partial raw trace. Native run1 remains a failed launcher attempt, not a completed repeat. Both ordinary episode extractions now have finalized output and manifests. No five-run Rain qualification is complete.

On another machine, resolve the repository root locally instead of using the source workspace path above. Read this file and the qualification report, inspect Git state and rebuild. `artifacts/`, `recordings/`, binaries, pipeline caches and session handles are local/ignored and do not transfer through Git. Historical results in the report are reported evidence, not independently reproduced results on the new machine. Missing raw evidence stays unavailable unless explicitly transferred or regenerated. Never count a missing artifact as a pass. The user-level goal-scratchpad skill also lives outside this repository; this file and the AGENTS.md pointer provide the necessary workflow without it.

Compact historical receipts and pre-commit recording provenance are tracked in `experiments/gpu-physics/benchmarks/2026-09-28-solver-checkpoint.json`. These do not substitute for missing raw captures or final qualification.

The Rain episode helper sources are now preserved in `experiments/gpu-physics/scripts/rain-episode-diagnostics/`. They select historical diagnostic windows; a new device may have different trajectories and event locations. Preserve same-device repeatability, not cross-machine agreement. First correct the runner's child-exit receipt handling around `xvfb-run` cleanup failures, with a bounded regression check, before launching another long batch. Do not rewrite failed receipts. Then finish the remaining physical criteria and persistent-state audit before freezing the final candidate and running the fixed matrix. Known restitution and contact-island symmetry screens still fail on both CPU and GPU; do not describe the full suite as green.

## Full objective

Make the GPU solver correct, stable, and repeatable using its own efficient constraint ordering. Exact CPU/GPU trajectories and resting poses are not required to match. Establish CPU-comparable physical capability within an explicitly documented qualification matrix, fix GPU defects, measure performance, and make efficient GPU ordering the normal mode only after qualification passes. Keep CPU-compatible ordering available for debugging and reference work.

### Determinism contract

Identical build, configuration, device, driver/backend, initial state, creation order, timestep/substeps and scripted inputs must produce identical semantic simulation results across fresh processes. Qualify ordinary and native cached GPU paths independently. No promise is made across devices, platforms, drivers, builds or configurations.

Compare relevant complete semantic state at every physics step, including poses, velocities, sleep state, contact geometry and impulses, joint configuration and accumulated impulses, lifecycle relationships, and persistent solver state that affects later results. Exclude padding, timestamps and incidental identifiers only with a documented audit. Preserve relational identities and any ordering that can affect future simulation. Pose checksums alone are insufficient.

### Physical correctness contract

Preserve upstream scene parameters, joint types, geometry and physical defaults. Validate joint constraints, collisions, friction, restitution, mass/inertia, sleeping/waking, motors/limits, CCD, dragging and body/shape/joint lifecycle operations. Detect missed collisions, persistent penetration, broken constraints, unexplained energy gain, invalid state and stale data after deletion/reuse. Preserve existing analytical, isolated, collision-free and numerical regression checks.

Use CPU as a capability and behavior reference, comparing constraint error, penetration, energy, settling, supported loads and scripted responses. Prefer analytical expectations when available. Define justified acceptance limits before evaluating a candidate fix. Allow different valid chaotic trajectories. Retain strict CPU comparisons where equivalent arithmetic and ordering make them meaningful. Unexplained instability cannot be called intentional divergence.

For each outstanding Rain failure, establish whether it is a GPU defect, behavior reproduced under equivalent CPU conditions, or an unresolved discrepancy. CPU reproduction alone is not a pass: explain whether behavior satisfies a physically justified criterion. Preserve failed original screens even when a better-founded criterion is proposed; do not broaden limits just because a candidate fails them.

### Qualification and practical execution

Start with Falling Ragdolls and Rain, including Rain joint errors and identity correspondence across recycling. Cover representative stacks, mechanisms, motors/limits, CCD, dragging, sleeping/waking and runtime creation/deletion/replacement.

Freeze the qualification matrix into named scenarios, commands, configurations, durations, physical limits and evidence requirements before another broad qualification campaign. The capability list below is the fixed scope; exact fixture selection and uncovered acceptance limits still need consolidation. Add tests only to cover an explicit missing requirement or reproduce a concrete defect, not speculative expansion.

Require at least five fresh runs per selected configuration. Make complete per-step comparison practical with compact lossless records or exact-bit hashes over audited semantic state, with detailed captures around discrepancies. Validate compact output against existing full captures and deliberate corruptions before relying on it. Hashing does not justify omitting state. Diagnose the earliest mismatch and fix its underlying race, ordering, initialization or stale-state cause.

Freeze candidate binaries and record hashes, configuration, device/backend and commands. Preserve completed runs across interruption; replace only incomplete runs. Do not mix different builds/configurations in one qualification batch. Existing partial traces are diagnostic evidence, not completed repeats. Avoid repeatedly launching multi-gigabyte full captures before addressing their runtime/storage cost.

Prioritize closure: close known failures, complete the fixed matrix on the candidate, measure performance, record comparisons, then decide the default. Add instrumentation only when it answers a specific unresolved requirement or discriminates a live hypothesis. Report closed gates and remaining failures; no unsupported completion percentages.

### Performance and delivery

Benchmark representative small and large scenes under controlled conditions, with both GPU paths and GPU-native versus CPU-compatible ordering. Exclude state/health/lifetime tracing and other diagnostic overhead from performance measurements. Record warmup, timed duration, repetitions, hardware/software configuration, completed-step timing and variability. Establish the performance acceptance decision before judging the candidate; no arbitrary speedup is promised without a baseline.

Publish the coverage matrix with explicit passes, failures and unsupported capabilities. Scope “equally capable” to that matrix. Document intentional trajectory differences separately from known defects. Record representative repeated GPU runs and CPU/GPU behavior comparisons. Do not hide failures behind averages, weaken checks or infer universal correctness from a few passing scenes.

Completion requires every required gate below to have current, directly applicable evidence, including performance, recordings and the default-mode decision. Unsupported capabilities must be explicit and consistent with the agreed scope; labeling a required capability unsupported does not complete it. Keep the goal active while required work remains.

## Acceptance gates and current evidence

All paths below are relative to the generated evidence directory unless stated otherwise. Historical passes must be checked for applicability to the final frozen build.

| Gate | Required evidence | Current status |
| --- | --- | --- |
| Fixed qualification contract | Named fixtures, limits, commands and durations for all capabilities | Fixed fixture selection and existing limits now recorded in qualification doc; acceptance gaps remain explicit (Rain/restitution, island wake, persistent state, performance decision) |
| Ragdoll physical behavior | 5 x 600 steps per path, unchanged stability limits | Current child-compaction candidate: all ten original physical screens pass; `desktop-baseline/ragdoll-child-compaction-five/physical-summary.json` |
| Ragdoll full-state repeatability | Every relevant semantic field identical over those runs | Current candidate passes five600step audited comparisons per path with exact slots/counters; only occupied/history memberships normalized. Selected v22source-contract applicability verified on0940a4f; final v22repeats remain open; raw append-order failures preserved |
| Rain recycling and joint behavior | Full 600-step identity/health checks, resolve excessive residuals | Open: 8,400 created identities, 4,200 reused slots and 1,948,800 joint observations mapped; residual screens still fail |
| Rain complete-state repeatability | 5 fresh full 600-step runs per path on fixed candidate | Open: run1 reaches600both; ordinary run1 complete/run2 interrupted, native launcher cleanup failed; no five-run qualification |
| Collision/support/energy | Analytical support, mesh contact and penetration cases | 57 frozen-pose cases and 5 x 600-step stack checks passed historically; final-build/wider matrix open |
| Friction/restitution | Analytical motion, rebound, settling, penetration and energy | Provisional desktop CCD correction passes all restitution assertions and5x240 captured-state repeats per path (5mm peak penetration); original49.31mm baseline failures retained. Friction5x240 passed historically and current single regressions pass; wider/final-candidate qualification open. |
| Mass/inertia, mechanisms, motors/limits | Analytical/reference fixtures plus lifecycle and scripted changes | Selected fixtures pass; matrix closure open |
| CCD | Sweeps, fast bodies and mutation with no missed barriers | Selected fixtures and 5 fresh mutation repeats pass; broader/final-build closure open |
| Sleep/wake | Settling, sleep-before-wake proof and island behavior | Latest gated paired-contact candidate passes every original contact-island screen and5x73step raw-state repeats on both paths. Baseline failures retained. Earlier sleep-to-wake/joint-island cases pass; remaining final-candidate matrix still open. |
| Dragging | Scripted drag/release, physical limits and per-step repeatability | 5 x 3,060-step runs per path pass historically; check final-build applicability |
| Lifecycle/query/events | Creation/deletion/reuse, stable semantics, no stale handles or duplicate ends | Several 5-run fixtures pass; full-scene/final-build closure open |
| Native replay | Actual cached-step reuse plus mutation/reentry and full-state repeats | Current child-compaction build: five48step raw-state repeats pass with42actual replay hits each, including substep/transform reentry; command-policy-audit/ |
| Numerical regressions | Preserve analytical/isolated numerical checks | All 28 precision tests passed both paths after latest production fix |
| Persistent-state coverage | Audited future-relevant fields and negative controls | Selected GPU-native completed-boundary source audit consolidated across211GpuSim fields and12device owners; v22 captures include demonstrated history/retirement omissions. Final frozen-candidate repeat evidence remains open; older v20 full-scene runs are insufficient. |
| Controlled performance | Small/large scene benchmarks, each path/order, no tracing | Both scenes: five paired runs per path validated on 0940a4f. Small screens pass; both large screens fail; tails mixed. Keep GPU-native opt-in; final-candidate applicability open |
| Visual delivery/default | Repeated GPU clips, CPU comparison, documented mode selection | GPU-native qualification recordings pending; GPU-native remains opt-in |

## Current implementation and findings

- Efficient ordering is `GPU_PHYSICS_LIVE_CONTACT_ORDER=0`. Default has NOT switched. Preserve CPU-compatible mode.
- Production fixes include collision flat-edge admission, contact handle/query ordering and duplicate end events, joint fallback execution, float32 recycling default, and most recently the joint collision-filter upload address during body growth.
- Latest filter bug: in-place scene updates uploaded joint filters using the old live body count, then changed the count used to calculate the shader address. `src/api/world.rs` now updates live parameters before scene/joint scratch uploads. Minimal two-to-three-body growth reproduces failure before and passes after on both paths.
- Latest native bounded Rain check passes the unchanged filter guard at frames80/81 and completes81 health frames. Those health results exactly match the earlier physical baseline: no Rain stability improvement demonstrated in this window. Evidence: `rain-filter-fixed/result.json`, `joint-filter-growth-*`, `filter-fix-{guard,lifecycle,precision}-{ordinary,native}.log`.
- Joint fallback fixes in `shaders/physics/solve.wgsl`: one global fallback writer; invalid-list small-wave path actually invokes fallback. Regressions pass both paths. Valid Rain component lists mean this does not explain the observed Rain episode.
- Rain matched isolated replays transplant all39 active joint caches (27 spherical,12 revolute;3 filter joints have no solver cache). CPU/GPU target cone errors closely agree; this narrows the investigation but does not clear full-scene failures. Evidence: `rain-{cpu,gpu}-all-joint-history/`.
- Initial Rain contact-history discrepancy is now reproduced on CPU as described below; other full-scene histories remain unmatched. At captured state122, target human bodies605–618 have6 active contact patches,3 with nonzero cached impulses. Terrain foot contacts and one self-contact are the main next comparison. See `rain-first-cone-full-state/precontact-cache-inventory.json` and `history.jsonl`.
- Extra CPU zero-time stepping is NOT a neutral way to seed contacts: immediate body getters match but subsequent trajectories change. Do not reuse that rejected control.
- CPU read-only `b3Solve` linker wrapper in `c_abi/reference_contact_boundary.c` observes after normal collision update. It passes the no-op control: all2,520 finite body-state rows across60steps byte-identical to baseline,60 hook calls,20 first-step contact patches. Evidence: `rain-cpu-solver-boundary/{result,manifest}.json`. The three nonzero-history target contacts now match CPU boundary1 to GPU collision geometry123: normals bit-identical, feature IDs equal, anchors/separations differ by at most2.98e-8m. Triangle mapping is now verified: GPU point IDs encode array index+1, so CPU406/241 map to GPUarray470/247 and encoded471/248; convex CPU-1 corresponds to GPU0. All512 torus triangles match bitwise under a bijection. An initial direct-index comparison falsely suggested adjacent triangles; that inference was corrected after inspecting the encoding. Evidence: `rain-cpu-solver-boundary/target-contact-comparison.json`. Native float32 friction conversion is prepared in `rain-contact-history-audit/friction-world.txt`; source audit confirms old tangent coefficients reconstruct world impulse, then new basis projection in CPU preparation/GPU collision. Same endpoints require no sign reversal; preserve rolling world vector/twist scalar and last-substep normal impulse, not total impulse. Evidence: `rain-contact-history-audit/audit.json`. Controlled CPU injection now implemented in the diagnostic boundary wrapper. It validates all3manifolds (ordered body/shape endpoints, exact normals, feature/triangle IDs and geometry within1e-6m) before changing only cached impulses. Three60step arms produce2,520finitebodyrows each; zero is byteexact baseline and baseline matches prior. Six malformed payloads reject with exit8. CPU restored onset cone error0.311083078 exactly matches fullGPU; first3steps exact and first5maxerror1.192e-7rad. Across18steps maxerror0.012300193rad as other histories/order remain unmatched. Evidence: `rain-cpu-contact-history/{result.json,full-gpu-comparison.json,manifest.json}`. This reproduces the early residual with equivalent joint/contact history; it is not physical acceptance or fullscene clearance. Next: use this evidence to classify the first episode, then target remaining persistent/severe residuals with the existing physical criteria rather than chasing later exact trajectories.
- Warm starting cannot currently be disabled on GPU; document this unsupported configuration, and do not use it as a matched CPU/GPU ablation.

## Latest capture-throughput correction

The v19 JSON writer used an unbuffered File. It now uses a 1MiB BufWriter and explicitly flushes each complete frame, propagating flush errors. No schema/state fields or capture guards changed. A three-repeat benchmark on a real20,026,862-byte Rain frame produced byte-identical raw/buffered files: median serialization24.63787s versus0.101028s (~244x for serialization only). This is not solver or end-to-end capture performance. Evidence: `trace-buffering/{bench.rs,timings.txt,result.json}`. The body-growth capture regression passes ordinary and native builds; native runtime was also checked with its configuration script sourced. Both sample executables are now rebuilt with buffering. Bounded end-to-end comparisons are complete; see results below.

Buffered end-to-end capture and interruption-safe batch recovery are now validated below. State coverage remains unchanged. Do not introduce hashing machinery merely to solve the already-identified tiny-write bottleneck.

## Capture and recovery validation completed

All jobs from this validation are terminal; no current live handle. `rain-buffered-capture/result.json` records ordinary10step wall time103.224s raw/12.185s buffered (8.47x) and native110.664s/13.243s (8.36x). Buffered81step captures finish in80.392s ordinary/75.956s native. Every81step physical health row matches the historical baseline on both paths. At10steps the only state difference is contact_allocation.occupied_order, with exactly equal membership on every frame (`{ordinary,native}/allocation-differences.json`). No other group differs. This is bounded diagnostic throughput/behavior evidence, not full determinism or solver performance.

`scripts/check-sample-state-repeats.py` provides locked, frozen-binary/configuration batches with preserved attempts, verified compressed traces and completed-process compression recovery. Five helper integrity tests pass (`scripts/test-sample-state-repeats.py`). An actual subprocess interruption/resumption test also passes: live duplicate rejected; completed run1 byte/mtime unchanged; interrupted run2 partial preserved; five runs complete after six total launches; second resume launches none; changed binary rejected. Evidence: `repeat-runner-recovery/{check.py,result.json,*log}`. It uses a synthetic fixture emitting one real captured frame, so its pass validates the runner, not physics. Full Rain repeats are still open.

Capture buffering and recovery are validated. The fixed fixture selection is consolidated; actual Rain repeats have now started as described below. Avoid further capture infrastructure without measured need.

## Live Rain qualification batches

`rain-buffered-five` run1 has reached600steps on both paths. Ordinary sample exit0 (2171.53s); batch session38633 is historical; run1 finalized with a completion receipt and run2 was subsequently interrupted. Native batch session25285 is terminal exit1: sample log reports600frames/0sokol errors, followed by `/usr/bin/xvfb-run: line185: kill ... No such process`. The launcher kills Xvfb under `set -e` before returning saved child status, so cleanup can mask that status. Do not claim the child's exact exit code from this evidence or edit exit.json. Native health is structurally validated through600sequential frames/statusok; its failed attempt remains diagnostic, not a qualified completed repeat. Native batch stopped before run2. Do not automatically restart the frozen failing runner. Next runner correction must separately record child and launcher exits, preserve this attempt, and validate cleanup-failure handling before replacement fresh runs. No ordinary process remains at handoff; preserve completed run1 and partial run2.

Both frozen sample hashes remain ordinaryc43c6e072de819aba88f0b4a379d786ec6c89b695ffaa732c05e47eed0d2111c and native6cb8d97496af49748ea88a4ac5220439b9b4d93cae06e2c439c990e58bbe3366. These are diagnostic captures, not performance measurements. Health reports are1.2GB each. `rain-later-full-state/select_health.py` reads the native writer's frame-per-line format, checks full600frame sequence/status/document end and hashes the entire source, then emits labelled42body-cell diagnostic selections. This avoids materializing the whole report while the ordinary runner finalizes. Both selectors32561/6652 are complete; ordinary health also validates600frames/statusok. Ordinary twist/cone state extraction completed with output and manifests; handles4646/92245 are historical.

`rain-residual-classification.json` consolidates evidence without changing limits. Early onset is reproduced with matched39joint+3contact caches; severecone1.26054549rad is reproduced by CPU/GPU with matched poses but fresh history and recovers to0; persistenttwist also reproduces in isolated CPU/GPU and remains when kept awake. The later fullscene cache/load explanation is still open. Once run1 reaches the relevant windows, extract persistenttwist health375–463 and severecone near516 using verified creation identities; candidate health identities2379/5004 must be reconciled with state identity mapping rather than assumed equal. Do not chase the already-explained earlytrajectory again.

## Verified health-to-state identity mapping

`scripts/health_identity.py::health_to_core_creations` maps Human creation IDs to core creation IDs using the same completed step, public body slot and generation. It validates live allocation membership and uniqueness, and rejects stale/deleted lifetimes. No fixed terrain offset is assumed. All15identity tests pass, including recycled-slot, stale-generation, deleted-slot and wrong-step controls. On both buffered81step captures, all68,880Human body observations per path map successfully and rotations/linear/angular velocities match float32 bits exactly. Evidence: `rain-health-core-identity/{check.py,result.json}`. Those bounded captures do not cover actual recycling; verify the later full-run episodes with their own health records once run1 completes. Current partial full-state captures alone cannot establish the Human creation mapping.

The later-window extractor is ready at `rain-later-full-state/extract.py` (run from engine cwd). It requires `--trace`, `--health`, `--out`, zero-based health `--first`/`--last`, and Human creation `--base`/`--target`. For persistent twist use first374,last463,base2353,target2379; for severe cone first514,last525,base4999,target5004. Use each current run1 attempt's completed health.json and state.jsonl.gz after completion. It rejects missing/stale mappings and requires exact q/v/w float32 bits for all42cell bodies. Output is explicitly a diagnostic subset, not a qualification trace. It retains connected external bodies, all cell joints/contact patches, cached state and step-start transforms. Controls pass: bounded health70–80 gives11frames with42bodies/42joints; historical contact-rich health122–130 gives9frames with42cell bodies plus1terrain body,42joints and71–72contact patches. Evidence: `rain-later-full-state/{bounded-control,contact-control}.manifest.json`. Current later-episode extraction still awaits run1 health completion; do not substitute historical health for the current run.

## Joint-island wake qualification

`trace_joint_island_wake_propagation` passes5fresh73step runs on both paths with exact raw v19state equality. Three1kg cubes connected by distance joints and a fourth disconnected cube naturally sleep for64steps. wake=false impulse leaves all asleep; wake=true1N·s centered impulse wakes all three connected bodies on the next step while the independent body remains asleep. Predefined limits pass: peak momentum error2.384e-7kg·m/s (<1e-4), energy0.166690504J (≤0.5001), length error0.000353443m (<0.01), no spin. All10runs agree on physical metrics. This closes the selected joint-island propagation/repeat case; contact-island coverage and complete persistent-state audit remain open. Evidence: `island-wake-five/{run.py,summary.json,ordinary/result.json,native/result.json}`. Builds require `--features replay-diagnostics`; an initial featureless selection ran0tests and is explicitly not evidence. Actual five-repeat runner requires exactly1passing test and exported73frame trace. Sessions25989/1259/78858/30510/90302 are all terminal. Test-only code changed; live Rain frozen sample binaries were not modified.

## Contact-island wake screen failure

Added `trace_contact_island_wake_propagation`, sharing the joint-island helper with touching cubes/no joints. Limits were recorded before execution. Ordinary run1 reaches frame66: both touching links established, all bodies slept, wake=false preserved sleep, wake=true woke all3connected bodies while disconnected body stayed asleep. It then fails the individual spin<0.001rad/s screen (peak0.0192691538). No five-repeat pass: `contact-island-wake-five/ordinary/run-1.log`, exit101. The runner correctly stops; no native run was launched. Both feature-enabled builds succeeded (sessions88291/15186 terminal).

New `c_abi/island_wake_reference.cpp` reproduces the same configuration on CPU and completes73steps, including wake/isolation assertions. CPU frame66 velocities/spins match GPU magnitudes with reflected transverse signs; peak spin0.0192691538 also fails the original screen. Evidence: `contact-island-reference/{manifest.json,result.json,cpu.txt,gpu-island-wake-true-539400.jsonl}`. This is shared multipoint-contact asymmetry, not evidence of a GPU-only wake failure. Physical acceptance is not yet established: retain the original spin screen failure and evaluate total angular momentum/energy through the full73GPU steps (the current test stops on first spin failure). If a better-founded criterion replaces individual ideal-symmetry screening, justify it from conservation/solver behavior and explicitly preserve this original failed screen; do not simply raise the spin threshold. The qualified joint-only fixture's limits are unchanged.

## Full contact-island diagnosis completed

The original spin and transverse-drift screens now run after complete73step capture/export, with unchanged limits; they still fail the Rust test (exit101). An intermediate full-window attempt stopped on transverse drift; preserved at `contact-island-full-five/ordinary/run-1.log`. The final diagnostic batch `contact-island-full-v2-five` requires the explicit symmetry-screen failure and complete capture for each run, then compares repeatability separately. All5fresh runs per path have exactly equal raw v19state. Wake, isolation, momentum, energy and penetration assertions pass through73steps; tests still report the original symmetry failure, not an overall physical pass.

`contact-island-full-v2-five/analysis.json` compares all73steps to CPU. All10GPU runs match CPU p/v/w exactly after Y reflection (polar p/v signs[+,-,+], axial omega signs[-,+,-]); maximum reflected error0. This is strong evidence of a symmetric numerical trajectory choice, not a GPU-specific wake defect. Identical metrics on both engines: peak linear momentum error5.97146e-8kg·m/s, angular momentum1.36406e-6kg·m²/s (<predeclared1e-4), energy0.167829651J (<0.5001), penetration8.08239e-5m. Original individual spin0.0192691538rad/s (>0.001) and transverse drift0.001521538m (>0.001) remain failed screens. Conservation and repeated-state evidence are established; reconcile the ideal-symmetry screen's role explicitly before marking the whole selected physical case accepted. Do not change the solver to force a CPU mirrored trajectory. All build/run sessions10062/35114/97486/82777/80973/55417 are terminal. No production solver changes this turn.

## Timing-window state audit correction

`GpuSim.metrics` is not wholly incidental instrumentation: steps/submitted select metric_step and suppress native full-step replay while the timing window is active. `diagnostic_policy_state` now exports optional `gpu_policy.timing_window={steps,submitted}`. No-window remains canonical field absence, preserving default v19 captures. Durations/query results remain excluded. Older active-window captures cannot be claimed covered; require independent no-window evidence for old captures. Source review found timing-window entry only through Rust API/headless/tests, with no C bridge/native sample call, so running frozen Rain captures are unaffected. `trace_captures_timing_window_replay_policy` passes ordinary and configured native: opening a window changes only this policy field, stepping increments submitted in full capture, finishing removes it. Evidence: `timing-window-state-audit.json`, `timing-policy-{ordinary,native}.log`. Inventory metrics entry refined; do not claim all209fields audited. Builds43824/26014 and native test are terminal. Only diagnostic capture code changed; no sample rebuild/restart needed.

## Force-clear and shape-identity audit

`step_force_slots` is represented by each live body's clear_force_next_step membership plus captured device loads. Source consumer drains old members with zero writes before any new load writes; order/duplicates therefore have no effect, and has_step_forces only checks emptiness. Uncaptured membership is rejected. `shape_identities` is the actual source of shape_storage_order, validated against live world shape generations; public shape slots are append-only. Added a generation-only corruption control to `trace_rejects_uncaptured_host_policy_and_force_membership`: stale GPU shape identity must reject capture without appending, then restore and confirm the existing force-membership corruption rejects. Both ordinary/configured native pass. Evidence: `state-identity-membership-audit.json`, `state-identity-membership-{ordinary,native}.log`. These two inventory entries are classified with consumer evidence; remaining explicit-consumer fields and device reset/persistence contracts remain open. Sessions77831/57578 and native test are terminal. Next persistent-state review candidate: contact_metrics boundary/current-step guarantees and output-versus-scheduling use; do not assume it is merely a metric by its name.

## Current full-scene Rain episode evidence

Native extraction completed: `rain-later-full-state/native-twist.jsonl` covers core375–464 (health374–463), corebody2479 verified as Human2379; `native-cone.jsonl` covers core515–526 (health514–525), core5104 verified as Human5004 including recycled slots. All42cell bodies on every selected frame match slot+generation and q/v/w float32 bits. Each window contains42joints and one external terrain body; contact patches retained. Extractor sessions92330/54117 are terminal. Evidence: manifests plus `native-health-validation.json` and `native-episode-summary.json`.

Persistent twist last-awake core426: measured lower violation0.05448156343rad, cached-impulse/stiffness estimate0.05448071309rad (difference8.5034e-7). First asleep core427 retains essentially the same error; finalcore464 error0.05448156626. `native-twist-compliance.json` uses `analyze_compliance.py`, an excerpt adapter of the existing reconstruction; it is not a full per-substep contact/joint torque balance. This supports loaded soft-constraint compliance, not a new sleep-created error or a physical acceptance pass. Severe cone still peaks1.26105142rad at health516, maxanchorerror0.0698773041m, recovering to0.00566038489rad at525. Oldscreens remain failed. Next: extract ordinary windows after selector completion, compare equivalent identity data, inspect target contact/load history for the severe transient and classify acceptance with physical evidence. Do not repeat early-onset work.

## Recovery after laptop restart

Verified 2026-09-28: no Rain fixture/runner processes remain. Prior session handles84200/38471 are historical, not live jobs.

`rain-filter-fixed-five/ordinary/run-1.jsonl` ends at complete frame156 (~2.72GB); native ends at158 (~2.76GB). No exit files, completed compressed traces or repeat results. Preserve these captures. Both crossed the old frame81 failure; neither qualifies a completed run. Older failed and interrupted batches also remain evidence; do not overwrite or count them as passes.

The latest ordinary/native sample binaries were rebuilt with the filter fix. Frozen copies and source/config manifests are in `rain-filter-fixed-five/{ordinary,native}/`. Verify their hashes before reuse. New capture-format code requires a new frozen candidate; do not mix its results with old binaries.

Build commands from the engine working directory:

```sh
cargo build --release --lib --features replay-diagnostics
cmake --build native-samples/build-gpu --target samples_gpu -j4
bash scripts/build-native-cache.sh build --release --lib --features replay-diagnostics
cmake --build native-samples/build-gpu-native-cache --target samples_gpu -j4
```

Native runtime requires sourcing `scripts/native-samples-cache-env.sh`; the build wrapper alone does not set runtime configuration. Sample binaries are under each build's `bin/samples_gpu`. Full qualification must leave `GPU_PHYSICS_STATE_TRACE_FIRST_STEP` unset; selected-frame captures are diagnostic only.

## Next actions

1. Close the explicitly listed acceptance gaps in the fixed fixture selection in gpu-solver-qualification.md. Existing fixture choices/limits are now consolidated; do not invent additional coverage beyond the stated scope.
2. Use the validated buffered capture and resumable batch runner for final frozen candidates. Preserve completed runs and failures; limit further infrastructure work to the demonstrated launcher child-exit receipt defect and any specific uncovered audit requirement.
3. Early Rain contact-history restoration is validated. Focus on later persistent/severe episodes from the preserved full captures and their physical/load criteria; use rain-residual-classification.json to avoid repeating settled onset diagnostics.
4. Run and close final fresh-repeat and capability gates on both configurations, investigate the earliest failures, and preserve original thresholds/results.
5. Run controlled performance measurements with no competing GPU work, then recordings and the documented default-mode decision.

## Authorization and repository constraints

User explicitly authorized commit and push of this checkpoint on 2026-09-28. Preserve unrelated dirty work. Do not modify the upstream `box3d/` submodule or grow the WASM library for this experiment. No subagent delegation currently authorized. Before committing solver/visual changes, repository instructions require affected scene snapshots and a real Box3D C comparison column; read AGENTS.md for the commands. Do not claim this scratchpad or a diagnostic hook completes the solver goal.

## Consequential decisions

- 2026-09-28: User requests a self-contained, file-backed goal maintained across compaction and restart, plus a reusable user-level skill. This file is the durable handoff.
- 2026-09-28: Retain physical correctness, same-configuration determinism and performance outcome. Tighten execution with a fixed matrix, practical complete-state comparison and interruption-safe batches. Stop reporting speculative percentage completion.
