# PR02 corner populations — standalone passes, combined defect

The independently linked CPU reference and standalone ordinary/native GPU
processes pass the sensor, compound and three-plane mesh diagnostic contracts.
The ordinary combined process then fails at its mapped CPU topology assertion.
The [campaign](protocol.json) stops there: **one CPU and three of four GPU
processes consumed; native combined unlaunched**. PR02 remains open.

| Process | Result | Observed public contacts / manifold buckets |
| --- | --- | --- |
| CPU reference | Pass | Sensor: 0; compound: 2 IDs, one manifold each; mesh: 1 ID, three manifolds |
| Ordinary GPU | Pass | Same diagnostic populations and buckets |
| Native cached GPU | Pass | Same diagnostic populations and buckets |
| Ordinary combined | Fail (-6) | GPU sensor/compound counts correct; mapped CPU topology differs at compound |
| Native combined | Unlaunched | No evidence |

The new C fixture retains default materials/shape/body settings except the named
zero-gravity, sleep-disabled controls. Each population takes four steps at dt
1/60, four substeps. The mesh uses the existing 12-vertex, six-triangle corner
from `state_trace::run_child_manifold_relocation`, with the same half-meter box
at `[.49,.49,.49]` and >=3 manifold criterion. No physical tolerance changes:
analytical/CPU bounds and sensor response use absolute 1e-5, topology and IDs are
exact. Sensor overlap identity is checked without public contacts or impulses;
disabled shapes appear in bounds but not enabled proxy peaks. Compound children
share one public shape ID while retaining distinct contact IDs; parent deletion
retires all children. Kinematic bodies contribute to dynamic occupancy. Profile
mask/NaN/step availability and zero-step semantics are checked separately from
occupancy/reservation quantities. These scoped checks are not final physical or
repeatability qualification.

## Demonstrated combined-creation defect

[Source diagnosis](diagnosis.json) identifies the cause in the actual linked
`both_dual.c`: `b3CreateBakedCompoundShape` creates its private GPU sphere/capsule/
hull children through the public **dual-world** constructors. Each adds a CPU
child collider. It then creates the entire CPU compound on the same body,
duplicating those colliders. The GPU parent/children are correct; the combined
CPU comparison is not an independent equivalent scene for these compounds.

The failed assertion checks mapped CPU body/shape/joint topology. It does not
print the exact CPU values, so a numeric CPU excess is a source inference, not
a claimed raw observation. The next fix must create private GPU children without
CPU mirroring, preserve public individual constructors/materials/transforms,
and verify parent deletion/recreation and affected native scenes. Do not alter
Box3D or WASM. The failing fixture is retained as a regression reproducer;
this stopped campaign is not reused for a candidate.

Two earlier CPU-only harness failures are retained:
[compound identity](../pr02-populations-2026-10-01/README.md) and
[coplanar grid](../pr02-population-identities-2026-10-01/README.md).
Both stopped with CPU1/GPU0. Source inspection independently justifies the new
child-ID contract and the different existing corner stimulus before any GPU
evaluation. Neither failure is replaced; the multiple-patch criterion is
strengthened to the existing >=3 fixture, with no physical tolerance relaxed.

## Provenance and reproduction

The [receipt](raw/receipt.json) preserves actual commands, compiled fixture input
hashes before/after, toolchain, all five linked binaries/archives, settings and
every process exit/log hash. The [raw index](raw-index.json) retains 17 files;
the earlier datasets each retain 12. Frozen CPU build/library hashes resolve to
the [PR01 compiled-source receipt](../pr01-speculative-2026-10-01/raw/handoff/native-scene-cpu/build.json).
Ordinary/native engine and C adapter hashes resolve to the preserved
[diagnostic dataset](../pr02-diagnostics-2026-10-01/README.md), whose compiled
sources match this campaign. Invocation HEAD is context, not binary proof.
Each launched GPU log identifies RTX 4070 SUPER and actual NVIDIA Vulkan.
No candidates, timing, benchmark flags, source changes or budget extension occur.

Audit the portable data without launching GPU processes:

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr02-corner-populations-2026-10-01/validate.py
```

Do not rerun the preserved runners. The
[roadmap](../../../../../docs/goals/gpu-production-readiness.md) keeps the combined
defect and viewer qualification open. The independent held-input proof passes,
but its [viewer campaign](../pr02-viewer-controls-2026-10-01/README.md) stops before
any app pending the required adapter fix/refreeze. No production-ready or
performance claim is made.
