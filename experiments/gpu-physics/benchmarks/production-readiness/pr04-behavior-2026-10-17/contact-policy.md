# Contact ordering no longer disables the paired normal solver

Retained PR04 source repair,2026-10-17. This closes the demonstrated contact-island
symmetry defect in the selected physical checks, not PR04 or the release.

## Defect and first cause

The original73step contact-island fixture has three touching unit-density cubes
in zero gravity and a fourth isolated cube. All sleep before a centered1N·s
impulse at step66. Wake/isolation, momentum, energy and penetration checks pass,
but CPU-compatible ordering fails the independent ideal-symmetry limits:
spin.019269153rad/s versus.001, transverse.001521538m versus.001 on both backends.
The existing CPU oracle also exhibits this asymmetry; its old mirrored trajectory
and conservation evidence remain archived in docs/gpu-solver-goal.md. Matching
that CPU numerical path is outside this practical behavior goal.

Source trace: `GPU_PHYSICS_LIVE_CONTACT_ORDER` enters
`sim.rs::diagnostic_flags_from_env`; only value0 suppliedSOLVER_PAIRED_NORMALS.
Default/value1 therefore reached `solve.wgsl::solve_contact` with the bit clear,
selecting sequential normal updates instead of the existing coupled pair solve.
The shader's paired path selects opposite points for four-point manifolds,
solves unilateral normal constraints together, and retains the existing scalar
fallback for ill-conditioned pairs. Ordering priority is a separate parameter;
its selection did not need to disable this already-qualified physical solver.

The repair always initializes the paired-normal policy in native builds.
Contact ordering itself, world/body/shape/joint parameters, geometry, timestep,
material defaults, IDs/picking and Box3D/WASM are unchanged. The normal solver
implementation and its mathematical tolerances are unchanged. WebAssembly's
separate flag initializer remains0. CPU/GPU scene trajectories may change.

## Actual verification

Frozen contact-policy-batch1-protocol.json, actual compiler logs and receipts:

- One algorithm-policy candidate, ordinary/native libraries and test binaries.
-32unchanged original physical checks:8perbackend/order0&1. Contact-island
  spin<.001 and transverse<.001 now pass in all four configurations, along with
  original wake, isolation, momentum<1e-4, energy<=.5001 and penetration checks.
  Support600, friction240, restitution240, motor120, mass18, joint/contactwake73
  and callback12 assertions also pass. No threshold was raised.
- Two original3,060step ground-drag physical confirmations, one per backend in
  changedmode1, pass identity/joint validity, support, finite states, free tracking
  and original release-speed limits. Original trajectory/globaltracking failures
  remain printed and qualified as explained in drag-tracking-applicability.md.
- Post-candidate contact-policy-order1-verification: all114original CCD/numerical
  checks pass (29CCD+28precision perbackend). Mode0's previous114CCD/numerical
  checks remain source-applicable: the returned mode0 solver flags are identical;
  broadphase candidate only reordered pure rejection predicates. New32physical
  checks include freshmode0controls. No final five-repeat/fullstate claim.

Build isolation used a temporary empty Cargo feature to keep prior pinned test
executables intact. No Rust cfg consumes that feature; the exact temporary
manifests, compiler-selected executable paths and hashes are retained. The wrapper
restored both manifests and sim.rs byte-exact after the trial; then the verified
sim.rs candidate alone was retained. No isolation feature is in production.
No engine or CAND archive was modified by restoration. Current shared libraries
contain the policy candidate; the old frozen PR06 archives remain old-source
baselines and do not qualify the new policy. PR06 applicability/refreeze is open.

## Scope and remaining work

This algorithm-policy change potentially affects native contact scenes with two
or more contact points under default/order1. It applies to both backends. Bodies
without such contacts retain their original normal-update path; order0 already
used paired normals. No rendering/UI/query/geometry source changed. Reuse the
completed165sample catalog for enumeration; do not claim all165physically reviewed
on this policy. policy-scene-dispositions.tsv explicitly lists current review scope.

Rain600 joint/recycling behavior, ragdoll600 current-policy physical acceptance,
full C-fixture applicability and final-build repeats remain open. Rain throughput
is still far below60SPS. Native dragging topology hitches and cold maxima still
fail frozen performance targets. The separate broadphase-cull optimization remains
an artifact, not part of this retained repair or its compiled test binaries.

All owned trial objects/executables were consumed/deleted; test traces went to
ignored task TMPDIRs and were deleted after assertions. Source/manifest restoration
and peaknewdata125,807,400B are recorded in contact-policy-batch1-restoration.json.
This is a verified local repair; no new implementation commit/push was requested.
