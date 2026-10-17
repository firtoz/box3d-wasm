# PR04 practical acceptance audit and frozen development targets

Frozen before any new production candidate on2026-10-17. No tolerance in the
original physical/function/calculation contract is increased. Source contract:
`../pr03-rust-baseline-2026-10-02/release-contract.md`; exact selectors/recipes:
`../pr03-contract-reconciliation-2026-10-02/qualification-contract.json`.
Current planning baseline6cd6956a7/runtime442689de1. Gate A remains OPEN until
matched baseline evidence and the current physical failure audit are complete.

## Assertion classification

| Original check | Classification and disposition |
| --- | --- |
| Ragdolls first60 corresponding-body positions within.006m | Cross-engine scene trajectory; DEFERRED, archived failure remains failed, not a physical repair target |
| Ragdolls600 finite rows, peakanchor<.25m, tail480..600anchor<.005m/linear<.05/angular<.1, GPUpeakangular<=1.2CPUpeak | Physical, REQUIRED unchanged. Historical physical pass is separate from trajectory failure; needs applicable current-source verification |
| Isolated spherical twist/negative/tilted/negative120,14fields at1e-5 | Targeted independent calculation, REQUIRED unchanged; not a chaotic full-scene comparator |
| Ragdolls-no-contacts60,112positions at5e-5 | Retain as targeted isolated-joint integration regression; no general visual pose parity claim |
| Rain600 +.05m anchor/+.05rad angular/cone/twist relative screens | Physical residual screens, REQUIRED unchanged. Match creation identity, enabled typed/oriented parameters and load; explain/correct excess under actual softness. Different falling direction does not excuse broken constraints |
| Rain finite/no runaway, support, identity/lifetimes/recycling | Physical/function, REQUIRED for entire600steps, including recycled IDs; timing-only endpoint digests are insufficient |
| Drag `Errors::add`, held/total/released positions, quatchords and velocities, including settledVelocity | CPU/GPU scene-trajectory deltas, DEFERRED as parity gates in loaded mode; preserve original failures/diagnostics. In particular heldPosition is not mouse-anchor error and settledVelocity is not either body's speed |
| Drag independently selected body IDs, valid joint creation/destruction, finite states | Function/physical, REQUIRED. Original noarg1320 and ground3060 windows/input sequence retained |
| Drag release endpoint each engine speed<.1 and angularspeed<.1 | Physical, REQUIRED unchanged for all5bodies after each of10releases; no resetting loaded scenes |
| Drag low-speed CCD first-impact classification/fraction divergence | Diagnosis, not automatically a defect. Original frame227 GPU correction16.993mm versus CPU discrete penetration is not by itself unacceptable. Independently verify landing, support, no barrier misses and finite stable release |
| Original isolated no-ground drag numerical bounds | Retain as controlled integration regression at original limits; no tolerance enlargement, no borrowed loaded-scene pass |
| Support600 | Physical REQUIRED: overlap/lateral<.025m, tilt<.01, gain<.1J/kg; tail120height±.025/linear<.05/angular<.1 |
| Friction240/restitution240/motor120 | Analytical/physical REQUIRED unchanged: stop.4±.04m, nofrictionerrors<.005, speed<.05after36; apices1/2.5±.05,penetration<.03,energy<25.2J/kg; reversal46/limits±.5±.01/transverse<1e-5 plus sourceassertions |
| Distance-original240×4 | Retained independent calculation regression,13lanes1e-5, original toggles/substeps. Existing timestep frequency-clamp repair already retained; no renewed experiment without new failure |
| Nine joint reaction/separation queries,57frozen mesh poses,28native_precision cases | Independent input/calculation/functional, REQUIRED unchanged at original1e-5 and source-specific tolerances |
| CCD29selectors and5step mutation; mass18;sleep400+1/joint-contactislands73; callback/identity/retirement/capacity/schedule tests | Physical/function/same-input schedule equivalence, REQUIRED unchanged. Preserve order1-only contact-order-storage applicability and native-only replay selection |
| Five-process complete future-state repeat matrix and finalpairedperformance | PR07/PR08, separate final campaigns. PR04 fixes cannot waive them or call pose digests full state |

The original C source `c_abi/both_drag_test.cpp:23` subtracts corresponding CPU/GPU
poses and velocities in `Errors::add`; lines190..199 separately test true endpoint
speeds. Do not rebrand its5mm held trajectory delta as a5mm spring residual.
An independent physical mouse-joint residual/force/work screen still needs to
be specified from its actual5Hz tuning, damping and maximumforce before a repair;
static gravitational spring deflection alone is about10mm at5Hz.

All101 named Rust selectors are retained, including their existing source
applicability. None is waived by scene-trajectory deferral. The13trace recipes and
14C recipes retain their required physical/function assertions, setup, durations
and bothbackend/order0&1 coverage. Cross-engine bodytrajectory diagnostics are
explicitly separated in the C cases above; no original result is rewritten.
New current-source runs must use matched geometry/defaults/inputs and disclose
CPU worker count. Current normal viewer records8workers/headless1worker are
separate observations, not matched performance comparisons.

## Performance acceptance frozen before candidates

Qualified desktop only, ordinary/native separately, dt1/60/foursubsteps/upstream
scene size/defaults, actual completion and required mirror synchronization.
Measure physics and end-to-end host cadence separately. No health tracing inside
timed samples; use separately verified physical runs. No pure submission clocks.

| Workload | Required useful rate | Latency guard | Window |
| --- | --- | --- | --- |
| Falling Ragdolls116bodies/112joints | >=60completed SPS and>=1simulated second/second | p95<=33.333ms,max<=250ms | active first240 steps after setup; report complete600 and sleeping tail separately |
| Original Rain4300bodies/4200joints at full occupancy | >=60completed SPS and>=1simulated second/second | p95<=33.333ms,max<=250ms | full600 and loadedfinal120, including normal recycling; no omission of expensive spawn steps |
| Original controlled/ground dragging | >=60completed SPS and>=1simulated second/second | p95<=33.333ms,max<=250ms | original1320/3060 completed steps; setup excluded, all held/released steps included |

Rationale: interactive60Hz needs an average16.667ms step; p95allows two ticks and
max250ms is an explicit development hitch limit. Rain is a stress case but its
matched one-worker CPU control completes the loadedfinal20 at about70SPS;60SPS
is a demanding useful target rather than permitting the observed2.5SPS. No GPU
speedup-over-CPU ratio is required. Pipeline/world startup is reported separately;
first-use pipeline work within stepping is reported, not silently removed. These
are PR04 development guards, not PR08 statistical qualification. Do not lower
these limits after candidate results. A miss remains a failure.

For each candidate, retain meaningful physical regressions first and timing
at matched windows. A predeclared20% throughput gain, or20% reduction of the
specific worst physical residual without worsening any required original limit,
is meaningful candidate progress. A falsified mechanism/first-cause attribution
also counts as investigation progress; a build/report alone does not.

## Historical approaches and current gaps

Reviewed: angular-axis refresh, substep-phase refresh, extra-waves and combined
refresh/waves, coupledcone/point andhinge5row blocks, warm-cache line search and
motor projection. Some improve isolated algebra or cone errors while worsening
anchors/twist; they remain rejected, not candidates to rerun. Later coupledCPU
prototypes must be read before proposing another coupled method. The newer
mesh-unilateral scalar fallback changed native arithmetic but left actualRain
18anchor/27angular/249cone failure counts unchanged, so it was not promoted.

Known unresolved physical issue: Rain severe/persistent joint residuals, including
moving latehip behavior and severecone about1.25rad. Loaded dragging has archived
trajectory failures and independently passing release endpoints; current full
physical confirmation is still required. No new explosion is inferred solely
from the word ragdoll or a trajectory failure. Source applicability and required
regressions remain OPEN until actual sources and completed evidence establish them.
