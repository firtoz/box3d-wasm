# Fixed release qualification contract

This reconciles the original fixed fixture matrix with the current PR03 baseline.
It does not certify PR03 complete, any physical failure resolved, or PR06/PR07
release-build qualification. The production roadmap is authoritative; detailed
historical results and rejected hypotheses remain in `docs/gpu-solver-qualification.md`.
Keep original assertions and failed screens. No candidate may change a limit to
fit its result. Missing or inapplicable evidence stays open.

## Configurations and evidence identity

Required ordering configurations are efficient0 and CPU-compatible1, independently
for ordinary wgpu/NVIDIA Vulkan and native cached/NVIDIA Vulkan. Efficient0 is
not promoted to the default until physical, future-state repeatability and the
existing small/large performance gates pass. Explicit global/component/automatic
scheduling overrides remain separate and unchanged. Source-selected test-specific
modes are recorded, including the live-order-storage fixture's order1 prerequisite.
The current primary physical baseline uses efficient0; unmeasured1cells remain
unqualified rather than borrowing0results. Full scene/reference1checks must be
selected and frozen before their final PR07 campaign.

Common physics is dt1/60, four substeps and upstream gravity [0,-10,0], original
sleep/warm-start/CCD/material defaults. Preserve documented fixture-specific
overrides (zero gravity, disabled contacts, fast velocity, variable substeps,
tiny frozen-pose steps, analytical masses, scripted inputs). Performance settings
are separately frozen, including sleep disabled on the three scheduling fixtures.
Every executable must have actual producer source/object/archive/link receipts,
source bytes/hashes, build settings/toolchains and binary hashes. Checkout HEAD
alone is insufficient. Source-relevant changes require an applicability audit,
refreeze and final-build evidence; cached dependencies are not clean-build proof.

## Fixed capability matrix

| Capability | Fixture / duration | Acceptance and required evidence |
| --- | --- | --- |
| Speculative/warm-start/API controls | PR01/PR02 current named fixtures and exact415-symbol inventory | Original per-world/geometry/default/transition/ownership assertions and explicit39-operation errors. Reuse applicable current receipts; inventory does not prove every behavior. Full physical gates below still required |
| Ragdoll support/joints/mesh impact | `ragdoll_reference.cpp ragdolls 600` | All67312 finiteordered rows; originalpeak separation<.25m,tailframes480..600separation<.005m/linear<.05/angular<.1,GPUpeakangular≤1.2CPUpeak. Exactjoint setup/bodyendpoint/type/material/default parameters; retain original first-step/frame2contact/first60step/visual trajectory screens and diagnose failures |
| Isolated spherical joints | `twist`, `twist-negative`, `tilted`, `tilted-negative`,120each | All14state/separation fields within1e-5 of independentCPU; original geometry/twistlimit/default settings |
| Collision-free ragdolls | `ragdolls-no-contacts 60` | Originalsetup with sourcefilter-only override,112bodypositions within5e-5 ofCPU; same full-state qualification obligations |
| Rain articulated recycling | `Benchmark/Rain`,600steps | Complete original scene, geometry/joint defaults, currentsource CPU/GPU matched positivecreationordinals, slot/generation lifetimes and typed/orientedjoint endpoints/parameters across recycling. Finite/no runaway/sticky loss. Capture enabled cone/twist parameters and true excesses. Retain +.05m anchor/+.05rad angular/cone/twistCPU-relative screens and contiguousevent/load/sleep diagnostics; explain/correct each required residual under its tuning/load/timestep before physical acceptance. CPU reproduction/isolated roots alone do not pass |
| Support/energy | `trace_stack_support_and_energy`,600 | Overlap/lateral<.025m,tiltquatvectornorm<.01,energygain<.1J/kg;tail120height±.025m/linear<.05/angular<.1; all source assertions |
| Friction | `trace_sliding_friction_matches_analytical_motion`,240 | Stopdistance.4±.04m,speed<.05after36;frictionlessposition/speed<.005,height/lateral<.01;all rotation assertions |
| Restitution | `trace_restitution_matches_analytical_rebound`,240 | Apices1/2.5±.05m,penetration<.03m,energy<25.2J/kg,originalsettling/CCD/barrier checks |
| Motors/limits/all supported joints | `trace_prismatic_motor_reversal_respects_both_limits`,120; original nine-kind reaction/separation fixtures, Rain/isolated/numerical controls | Reverse46,±.5limit with.01allowance,transverse<1e-5,originalreach/settle asserts; allnine-kind original1e-5 independentCPU/reaction/ownership checks applicable to retainedsource. Those API fixtures do not excuse loadedRain failures or unresolveddistance prototype |
| Distance first-step | Preserved `distance-original.cpp`, originalsphere-ground/distance×1/4substeps,240each | All13pose/velocity lanes within1e-5; original20/60/100/160warm-start transitions/forces. Abort is a failed/incomplete case; no unreachedduration/substep branch counts |
| Mass/inertia/replacement | Original fourcapsule/compound/impulse/query fixtures plusshape replacement; mutationtrace18 | Existing analyticalsource asserts; sixstages×3steps,unitlinear1e-5/angular1e-4;kinematic/masslessnonzeroimpulse giveszeroresponse. Preserve shape/mass-center/inertia/material/lifetime behavior |
| Sleep/wake | HighResistance400settle+1wake; joint/contactisland73each | Demonstrateactualvalidbody sleeps beforeinput,allconnectedwake/isolatedremainasleep,wake=falseunchanged. Joint axial1N·s momentumerror<1e-4,energy≤.5001,spin<1e-6,length<.01. A fabricated/stale handle cannot prove sleep or wake |
| CCD | All29current CCD-selected names and mutation5 | Preserve every source assertion, no missedbarrier, rotatingoffset/kinematic/filter/joint exclusions/meshglancing/posehistory/mutations. Trace actualGPUCCD cohort/rebuild/fallback as source requires; countallselected/nonignored tests |
| Query/contact history/events/lifecycle | Child-joint16,child-query8,callbacks12,retirement3,creationidentity4internal,unread-root | Originalhandlevalidity/stale rejection,child/rootrelocation and cache/history/identity assertions; callbacks include actualordering/decisions/persistent counts. Record public/source-supported identity rather than assumedgeneration1 |
| Native full replay | `trace_covers_native_full_replay_and_reentry`,48 | Actual42hits including substep/transform reentry,sourcehit lowerbounds andnonemptycontacts; full per-stepsemantic comparison later |
| Drag/picking/release | Original `both_drag_test.cpp`: noarg1320,`--ground`3060 | Independentrays/hitbody andmousejoint lifecycle. Isolatedtotalposition/quatchord≤.0005/velocity≤.005. Groundtotalpos/quat≤.025,heldpos≤.005/quat≤.01/velocity≤.1,settledrelativevelocity≤.02;allreleaseendpoints bothengines speed/angular<.1. Retain printedstrictallframevelocity≤.5diagnostic andstrictfixture semantics; no result reset inloaded mode |
| Frozen meshes | Original45grid/12torus poses, twofreshprocesses perengine | Exacttrackedpose/geometry hashes,count/normal/separationmultisets1e-5;allgridposescontact/upnormal. CPUresults reused forbothGPU comparisons; not57repeatedprocesses/fullscene proof |
| Numerical | Exact28current `native_precision` names | Every unchangedCPU-reference arithmetic/geometry/softness/inertia/joint assertion; host cases honestly classified |
| Islands/schedules/capacity | Original4complete-component+5mixed/automatic schedule tests, originalcapacitytests/30000insertioncontrol | Sourceoriginal1e-5scheduleequivalence/exactrepeats,fullconstraint/island coverage,explicitoverrides/topology/transitions,stickycapacity/growth/bounds. Existing32768slot regression must be retained in final PR07 selection; it is not claimed covered by30000insertioncontrol |
| Runtime/consumer/loss | PR05/PR06 fixedconsumer/lifecyclematrix | C/RustABI layout/ranges,root/childgenerations/growth/exhaustion,reentrancy/concurrency/asyncinvalid/loss/replayinvalidations,actualUI lifecycle/startup/quit; clearing errors never healsfailedworld. Minimalexternalcreate/step/read/destroy andunchangedTS/demo workflow |
| Performance/default decision | PR08, originalsmallRagdolls/LargePyramid + three schedulingfixtures | Fivepairedruns perbackend/scene/order; one-sided95%Student-tupperbound pairedloglatencyratios<0inallfourcells,originalp95/maxscreens; no traces/recordings/competingGPUwork. Schedulingthreefixedfixtures explicitglobal/component/currentautomatic90warm240timed/foursubsteps/dt1/60/sleepoff,matchsettings/freshprocesses/alternateorder. Freeze new finitebudget; exhaustedprior55run dataset remains targetunmet |
| Final presentation | PR09–PR13 | GPUfloors,actualsamplewidgets,querynumeric/rayvisualqualification andCPU-firstrecordings; fresh exactdeliverydesktop-only realCPU/GPU renderingFPS separatelyfromstepms/throughput/p95,raw/provenancereceipts,scope/limitations/finalaudit/commitpush |

The numerical/CCD/schedule/capacity command names are exact in the associated
frozen Rust protocol. C fixture commands/binary/link/source receipts are exact in
the preceding current fixture report. Rain/drag/order1commands and their separate
launcher repair preserve exact settings in their frozen protocols. Any remaining
selected configuration without current completed evidence stays OPEN.

## Full future-state and repeat contract

PR07 requires at least five fresh processes per selected configuration on each
backend, every required frame over600Rain/ragdoll,3060loadeddrag and the original
fixture durations above. Every repeat has the same build/device/driver/backend,
initialstate/creationorder/configuration/dt/substeps/scripted inputs. Interrupted,
pose-only, earlier-build or shortened runs do not count. Required semantic state:

- Bodyposes/velocities/sleep/motion locks/mass/inertia/forces and pending commands.
- Complete active contactgeometry/impulses, SAT/manifold ownership/identity,
  warm history/materials/filters, root/child allocations/generations/free-stack/
  high-water state, hash placement and numerical/constraint processing order.
- Joint definitions/endpoints/types/limits/motors/tuning/accumulated caches and
  reference/history fields influencing later solving.
- Worldregistry root/child epochs, retiredchild generation ceilings and exhausted
  slots; C ABI per-world metadata/generation routing/counters and sticky error/
  invalid/reentrancy state. Schema24's presence does not prove these newfields.
- Host/body/shape/geometry storageorder/allocation/lifetime relationships,
  queryindexes/published/deferredevents, callbackpresence/inputs/decisions/order/
  persistent external counters, host/device mirrors and completed-boundary state.
- Sleep/island/graph/cache/replay policies and future-relevant cached command,
  CCDorigin/geometry, pipeline-bound resource versions, allocator/reservation
  and eligibility state, including native-cache mutations/reentry.

Audit actual source producers/consumers against this list, including the current
WorldRegistry and C metadata, before trusting any existing schema. Preserve raw
meaningful order/relational identities and bit-exact float values. Normalize only
previously audited occupied/history memberships with a source consumer audit and
actual perturbation controls; do not broadly sort arrays or discard free slots.
Exclude timestamps/padding/incidental handles only with a recorded audit. Report
first differing frame/object/field. Pose hashes are a subset check.

Compact/lossless capture must reproduce every audited semantic field and ordering
of full records. Validate exact decoded bytes or semantics against full captures,
plus truncation/corruption and changedcontact/history/generation/callback/order/
replay controls before using it for five-run qualification. Existing
`compare-core-state.py` compares captured schema fields; it explicitly does not
certify complete coverage. Existing host storage/comparator negative controls are
required, but cannot substitute for new registry/ABI field coverage. Final-build
source changes invalidate affected evidence and trigger applicability/refreeze.

Every campaign freezes a finite process/build/candidate/diagnostic/timing budget,
configuration order, watchdog, provenance, limits and stop/retain rule before
execution. Preserve allfailed/unfavorable/unlaunched results; no favorable-only
extension or reuse of a closed budget. Build/setup dependency repairs are distinct
from physics trials and must retain their original stopped attempts. No new
criterion is invented here to relax an existing physical/assertion limit.
