# Compound AABB API contract: focused validation passes

All twelve fresh processes pass against the exact repaired candidate inputs:
four C bounds-contract processes, four C regressions and four Rust suite
processes. Bounds retain the original absolute `1e-5` independent CPU tolerance.
This closes the tested compound public/body/native bounds contract. The [precommit scene review](../pr02-compound-aabb-captures-2026-10-01/README.md)
passes/reviews twelve CPU-first captures; the source repair is retained. PR02 remains open for
combined ownership and actual viewer controls. This is not final physical,
repeatability, platform or performance qualification.

| Evidence | Ordinary / native coverage |
| --- | --- |
| C compound contract | Two fresh processes each, alternating ordinary/native/native/ordinary |
| Independent bounds | Sphere/capsule/hull/mesh; unrotated/rotated body; two parent generations each; creation, body transform, disable and enable; five queried bounds vs independent CPU geometry at1e-5 |
| Stale parent IDs | CPU/GPU parent invalid after deletion; public stale bounds return six zeros; next generation remains valid |
| C settings/replacement regressions | One unchanged settings and replacement process per backend |
| Rust query suite |18 unchanged tests per backend, zero failed/ignored |
| Rust diagnostic contract | One unchanged test per backend, zero failed/ignored |

Each bounds process validates320 six-lane query results: mapped CPU shape,
public GPU shape, native owned public shape, public GPU body and mapped CPU body.
The reference uses real Box3D `b3ComputeCompoundAABB` plus its speculative
padding and the requested body transform; it is not computed from the GPU result.
The native visitor must return the correct public parent identity and status.
Four processes therefore validate1,280 bounds results, alongside stale handles.
Old duplicate CPU primitive colliders remain a separate known defect; this
fixture deliberately does not claim their ownership counts pass.

## Behavior and implementation

Native compound creation now imports the exact source local tree AABB before
transformed hull baking can lose its enclosing-box semantics. Existing traced
parent center/half fields retain that box, with no borrowed native pointer.
Public bounds resolve compound children and transform the enclosing local box
once, then add speculative padding. Body bounds group by public owner and use
the same result. Native diagnostic snapshots likewise use public parent semantics
and still invoke C visitors after unlocking. Disabled direct queries remain valid.
Noncompound public bounds retain their previous calculation.

Direct programmatic Rust parents without an imported source box derive local
bounds from their children. Existing stale-parent query tests remain unchanged.
GPU proxy boxes are still overwritten from collider-derived topology bounds in
`ensure_sim`; the imported host query box does not replace solver/broadphase
bounds. No WGSL, scheduling setting, timestep, material/default or CPU ownership
routing change is applied. Existing semantic captures already include parent
center/half, so no unrecorded future query metadata is introduced.

## Budgets, failures and provenance

The [immutable validation protocol](protocol.json) freezes one compiled variant,
C contract4/C regression4/Rust suites4, baseline0/timing0. All cases complete in
the fixed order. No result is repeated/replaced, tolerance lowered or budget
extended. The [initial compiler rejection](../pr02-compound-aabb-fix-2026-10-01/README.md)
and its launcher sequencing deviation remain failed. The
[compile-only repair](../pr02-compound-aabb-compile-2026-10-01/README.md) produces
exact frozen libraries/tests before this first API validation. Those earlier
closed budgets are not resumed, and no GPU trial was discarded.

[Raw index](raw-index.json) hashes all retained fixtures, build/run logs and
receipts. CMake freshly builds both combined C adapters and stateless/mapped CPU
archives. Before/after input hashes, source archives, generated adapters, actual
built translation units/object hashes/commands and linked archive hashes identify
what was compiled. Candidate Rust archives/tests have separate full compiled
source snapshots, toolchain/build receipts and byte hashes. All C fixtures build
before any trial. Invocation checkout metadata is not binary provenance.

Actual NVIDIA RTX4070SUPER Vulkan and driver610.57.04 are recorded; precise
ordinary/native cached settings and CPU-compatible ordering1 are in every run
receipt. These are diagnostic/correctness runs, not timing data. Timestamp values
or incidental clocks in unchanged regressions cannot be used as performance
measurements. Box3D/WASM and prior datasets remain unchanged.

Run `python3 validate.py` for offline evidence checks. It launches no physics.
The tested source has completed the required scene and caller review; its
verified milestone is recorded in the authoritative readiness roadmap. Any subsequent production change requires an
applicability review/refreeze; these results do not automatically qualify future
ownership fixes or final release builds.
