# Public compound bounds: confirmed API gap

On both ordinary and native-cached NVIDIA Vulkan backends, the public GPU
`b3Shape_GetAABB` returns six zeros for every tested compound. The mapped real
CPU result matches independent `b3ComputeCompoundAABB` geometry plus upstream
speculative padding within the unchanged absolute `1e-5` tolerance in all eight
cases per process. PR02 remains open; diagnostic completion is not physics or
API acceptance.

The [protocol](protocol.json) freezes exactly two diagnostic fresh processes,
zero production candidates and zero timing. Both builds complete before the
ordinary then native serial runs, each exits0. Assertions validate independent
CPU geometry, finite observations and native visitor ownership/status. GPU
discrepancies are printed and retained as defects, not made into a passing
acceptance criterion. This addresses a distinct public AABB requirement exposed
by the [rejected ownership candidate](../pr02-compound-fix-2026-10-01/README.md);
that stopped budget has not been rerun or extended. The production adapter is
still exactly its pre-candidate baseline.

| Compound case | Public GPU result | Maximum native extended vs CPU parent error, both backends |
| --- | --- | --- |
| Sphere, unrotated | All six lanes zero | `1.1920929e-7` |
| Sphere, body rotated0.2rad | All six lanes zero | `0.108435869` |
| Capsule, unrotated | All six lanes zero | `1.1920929e-7` |
| Capsule, body rotated0.2rad | All six lanes zero | `0.0722910166` |
| Hull, unrotated | All six lanes zero | `0` |
| Hull, body rotated0.2rad | All six lanes zero | `0.0404696465` |
| Mesh, unrotated | All six lanes zero | `2.38418579e-7` |
| Mesh, body rotated0.2rad | All six lanes zero | `2.38418579e-7` |

Full [six-lane observations](observations.json) preserve CPU, independent CPU,
public GPU, extended GPU and absolute errors. Each fixture uses two spatially
separated children, existing world/material defaults and static body creation;
no solver step, sleep-policy modification or scene scaling is performed. Hull
children retain their own0.2rad transform and mesh children their0.1rad transform
and1.2/1/0.8 scale in both body-orientation cases. This matrix diagnoses named
constructor/query semantics, not arbitrary compound geometry or lifecycle.

## Cause and next action

Public GPU queries obtain the zero-sized placeholder parent through
`query_shape`; compound geometry resides in the children. The ordinary/combined
C wrapper routes to that GPU query, so consulting a native merged diagnostic
cannot prove the public API is correct. Native diagnostics do merge child
world-space AABBs. Box3D instead transforms the compound's local enclosing tree
box and adds speculative padding. For rotated primitive/hull compounds these
are different boxes, even though both can enclose the geometry. The explicit
public API needs Box3D-compatible parent semantics; tighter child union output
must not be presented as that result.

Next, implement parent compound bounds from independent local child bounds and
the body transform, using the upstream geometry/scale/padding rules and existing
absolute1e-5 comparisons. Share the semantic result between public and native
ownership diagnostics. Freeze a finite candidate protocol before checking the
change; preserve complete child coverage and verify mutation/lifetime. Do not
change the tolerance or accept zero parent geometry. The separate CPU duplicate
child defect remains unresolved; its rejected edit is preserved only as evidence.
Viewer control qualification must use refrozen binaries after retained fixes.

## Portable evidence and provenance

Run `python3 validate.py` here for offline validation. [Raw index](raw-index.json)
hashes all retained build/run logs, receipt, protocol, runner and compiled-fixture
source archive. The receipt pins actual link commands, all linked archives,
compiler, binary hashes, before/after source inputs, environment and NVIDIA
adapter logs. The immutable protocol links exact prior adapter/engine build
receipts and their hashes. Rust/WGSL and baseline adapter archives are reused
byte-for-byte; invocation revision is not compiled-source proof. Driver610.57.04
and host details are separately preserved by the preceding
[environment receipt](../pr02-compound-fix-2026-10-01/environment.json).

Both processes retain CPU-compatible ordering and exact ordinary/native cache
settings in their receipts. No ownership candidate, visual solver change or
benchmark has been kept. Box3D, WASM and earlier evidence remain unchanged.
Historical commands/scripts are retained for audit; this closed two-process
protocol must not be launched again.
