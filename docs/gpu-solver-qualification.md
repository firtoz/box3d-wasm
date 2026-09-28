# GPU solver correctness and repeatability qualification

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

## Coverage and current evidence

| Capability | Required evidence | Current status |
|---|---|---|
| Falling Ragdolls repeatability | Five complete-state fresh runs per configuration, 600 steps | Five frozen-v19 runs complete on each path: physical outputs identical, but raw allocation/history comparisons fail; all other captured groups match over 600 steps. Later frozen five-run batches complete on both paths; standalone slot-permutation diagnostics pass all600steps, but raw qualification and final state coverage remain open. Binaries predate recycling-default correction |
| Falling Ragdolls stability | Existing separation, angular-speed and settling limits; finite complete state | All five frozen-v19 runs on each path pass unchanged physical limits over 600 steps; later frozen five-run batches also pass; wider physical coverage pending |
| Rain recycling | Persistent logical body/joint correspondence across creation/deletion/reuse; complete state and health checks | Creation mapping passes all 600 steps, 8,400 bodies and 1,948,800 joint observations across 4,200 reused slots; residual screening still fails |
| Rain joint correctness | Explain excessive residuals and qualify appropriate constraint behavior | Full 600-step spherical capture validates identities but fails residual screening: first cone discrepancy at frame130, before first anchor discrepancy at170. Matched-state replays reproduce severe cone and persistent twist behavior in both engines; sleep ablation does not eliminate isolated twist residual. Full-scene history/load diagnosis remains open |
| Collision response and penetration | Analytical isolated contacts plus persistent support/penetration checks in stacks and mesh scenes | Flat-edge admission corrected; 57 frozen-pose cases pass ordinary/native cached. Analytical three-cube support/energy fixture passes five600-step fresh repeats on each path with corrected defaults; every raw captured field matches independently. Wider behavior coverage pending |
| Friction and restitution | Existing analytical/reference fixtures with unchanged coefficients and tolerances | Analytical sliding/frictionless fixture passes five240-step fresh repeats per path, all raw captured fields equal; selected reference tests also pass. Vertical restitution passes rebound/energy/settling assertions on both paths but fails the unchanged30mm penetration screen (49.31mm peak, reproduced by CPU); wider material behavior remains pending |
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
| Performance | Controlled small/large scene measurements after correctness qualification | Ragdoll process timings available; representative sweep pending |
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
| Restitution | `trace_restitution_matches_analytical_rebound`,240steps | Rebound apices1.0/2.5±0.05m, penetration<0.03m, energy<25.2J/kg, settling assertions unchanged. Penetration screen remains failed, including CPU reproduction |
| Motors/limits | `trace_prismatic_motor_reversal_respects_both_limits`,120steps | Reverse at46; limits±0.5 with0.01m allowance, transverse drift<1e-5; stops reached/settled per source. Spherical/revolute behavior also covered by28 numerical fixtures and Rain; persistent Rain errors stay open |
| Mass/inertia | `capsule_mass_data_matches_compute_helper`, `capsule_compound_inertia_applies_parallel_axis_once`, `capsule_angular_impulse_uses_axial_inertia`, `mass_queries_preserve_shape_and_explicit_mass`, `replacement_reuses_storage_and_preserves_body_mass_and_metadata` | Preserve all existing analytical/source assertions; integration of mutation checks with per-step repeat traces remains a coverage gap |
| Sleep/wake | `high_resistance_sleeper_wakes_on_velocity`,400settle+1wake; `trace_joint_island_wake_propagation`,73steps | Require actual sleep before input. Three unit-mass cubes linked by distance joints must remain asleep for wake=false, then all wake on the first step after wake=true while the separate cube stays asleep. Axial impulse1N·s: momentum error<1e-4kg·m/s, energy≤0.5001J, spin<1e-6rad/s, length error<0.01m. Five fresh73step per-step repeats pass each path with exact raw captured state; contact-island coverage remains to assess |
| CCD | `convex_gpu_ccd_corrects_live_steps_and_rebuilds_after_mutation`, plus existing sweep/rotating-offset/exclusion fixtures | Keep source assertions and five-step mutation repeat evidence; validate barrier coverage against explicit fast-body requirement before closing |
| Lifecycle/events/query | Existing child/root joint-history16step, child-query8step, callback12step fixtures and generation controls | Retain reference/identity assertions and full-step five-repeat evidence; see existing manifests for exact test selectors. No new general lifecycle suite unless an uncovered requirement is identified |
| Native replay | `trace_covers_native_full_replay_and_reentry`,48steps | Require42 actual replay hits, mutation/reentry behavior and every-frame state equality |
| Dragging | Existing ground drag3060step fixture and its frozen manifests | Retain original physical gates and five-process state comparison; final-build applicability must be recorded |
| Numerical/mesh regressions | `native_precision::` (28tests), existing57frozen mesh contact poses | All unchanged assertions must pass; these do not substitute for full-scene behavior |
| Performance | Falling Ragdolls + Large Pyramid;5runs/path/order,60warmup+180timed | Use `performance-measurement-plan.json`: no tracing/health instrumentation, completed-step timing, no competing GPU work. Default-mode performance acceptance decision still needs definition before evaluating results |
| Delivery | Scene snapshots/CPU column, coverage and default documentation | Record after final candidate qualification; do not switch default with required gates open |

The fixture selection now identifies what to run and the known acceptance gaps.
It does not close the fixed-contract gate while Rain/restitution, island-wake,
persistent-state coverage and performance decision criteria remain unresolved.

## Acceptance rules

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
| Graph construction metadata | Both ordinary and cached graph builders run color reset and per-body metadata clear before classification on the non-rebuild path. Persistent memo state is separately captured; other graph branches still require their own contracts. |
| Pair-matrix scratch | The matrix path (at most 704 shapes, no mesh triangles) writes every live matrix word, merges valid previous pairs, writes every row count and prefix, then scatters bounded pairs. Ordinary and cached command sequences preserve this order; bases overwrite collision/prepare/radix indirect counts. Matrix scratch is not carried as physical history; the previous-contact inputs are captured separately. |
| Shared graph memo | `graph_assign_dynamic_memo` checks layout tag, body/edge counts, initial body masks and color counts, then ordered slot/endpoints/pair identities before reusing output. Rebuild restores initial inputs before the greedy walk; partial reuse reconstructs mask union and count maxima. Captured memo records retain all these inputs/outputs and stale entries; the omitted hit counter is not a solver input. |
| Batched graph memo | `graph_assign_dynamic_batched` clears every cached batch count on layout change, resets per-batch keys/counts, validates count/endpoints/initial masks, and reapplies relative local indices to current color starts. Capture includes each batch count and all six record words. Hit/miss counters are diagnostic only. |
| Active-body count | Query word 72 is explicitly cleared before counting on eligible steps; `idle_count_valid_step` is invalidated at step entry and set only for a produced count. Persistent idle proof metadata is captured. |

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
