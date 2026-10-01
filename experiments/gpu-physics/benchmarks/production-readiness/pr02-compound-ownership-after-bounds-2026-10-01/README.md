# Combined compound ownership after qualified bounds

The combined adapter now creates each CPU compound exactly once. Private GPU
sphere, capsule and transformed-hull children use helpers with an explicit
`mirrorCpu=false` argument; public constructors pass `true`. There is no mutable
creation mode. Mesh creation, parent mapping, materials, render callbacks and
the accepted source-local bounds import are preserved.

All **14 first correctness processes pass** on the desktop NVIDIA Vulkan
adapter: four ownership processes, followed by ten existing regression
processes. This is a focused PR02 result, not final physical, repeatability,
viewer-control or performance qualification. No timing process ran.

## Fixed protocol and retained history

[Protocol](protocol.json) SHA-256
`455bc93f52eb4d653f89aed28d5e62f6d46179fce7c3781c65ee5c96f9658b98`
freezes one C-only candidate, zero new baseline processes, four ownership and ten
regression processes, zero Rust builds/trials, zero timing and zero retries.
Builds precede all trials; any build/input/assertion/600-second timeout stops and
restores the candidate. No run was retried or tolerance changed.

The [earlier ownership attempt](../pr02-compound-fix-2026-10-01/README.md) remains
rejected: independent baseline counts exposed duplicate primitive CPU colliders
and accumulating orphans; the candidate reached correct counts and then failed
the original bounds check. Its budget/results are unchanged. The prerequisite
is now materially different: [AABB repair and original-limit validation](../pr02-compound-aabb-validation-2026-10-01/README.md)
were independently delivered in `776577937bce5e9a265264f327c524f4f1e4cc4f` before
this frozen campaign. The same ownership helper delta is evaluated against those
qualified bounds. This does not replace the failed result or resume its budget.

## Coverage and outcomes

| Process group | Order | Result |
| --- | --- | --- |
| Original ownership fixture | ordinary, native, native, ordinary | 4/4 pass |
| Independent sensor/compound/three-plane mesh populations | ordinary, native | 2/2 pass |
| Original API settings | ordinary, native | 2/2 pass |
| Original shape replacement | ordinary, native | 2/2 pass |
| Original substep forces | ordinary, native | 2/2 pass |
| Independent compound AABB contract | ordinary, native | 2/2 pass |

The ownership fixture is unchanged, SHA-256
`bb75e0d3313746d92695a0c53500e4ac6f0bdbfc5df1f628de92ee6fb7e10277`.
It checks sphere/capsule/hull/mesh compounds through three create/delete cycles,
exact independent mapped-CPU and public-GPU body/shape/joint counts, single
parent identity, native owned bounds at the original absolute `1e-5` limit,
public sphere/capsule/hull mirroring and stale parent invalidation. No baseline
flag is passed. All original regressions/assertions remain unchanged and enabled.
The combined population check that previously failed now passes on both paths.
Compound scalar getters and actual viewer controls remain open under PR02.

## Source and binary provenance

[Terminal receipt](raw/receipt.json) records the exact order, commands,
environments, adapter/driver, toolchain, executable hashes and stdout/stderr
hashes. The sole production candidate is `c_abi/both_dual.c`, SHA-256
`0b0ed1f953620b60735d66eca215728727d25872647906d8a3b47a9ef811d086`;
[baseline](raw/baseline-both_dual.c), [candidate](raw/candidate-both_dual.c) and
[patch](raw/candidate.patch) are portable. Invocation checkout metadata is context
only. Every fixture is newly linked with `-O2 -std=c++17 -ffp-contract=off` and
assertions enabled.

Fresh combined C adapter receipts and archived sources identify the 107 relevant
compiled translation units, object hashes and five linked archives per backend.
The unfiltered CMake inventory also includes one shared cached NFD GUI object;
it is outside these builds and absent from every fixture link, so it is explicitly
excluded from C fixture provenance. The first offline validator failed when it
tried to find that unlinked source in the archive. Its original script, failure
and corrected applicability filter are retained; no build or GPU process repeated.

Rust/WGSL inputs and libraries are byte-exact reused from the
[compile-only bounds repair](../pr02-compound-aabb-compile-2026-10-01/README.md):
ordinary `096d9a9bfad7ba87677723ba37f3b6ee413f7ed266682b53927b7ebe3cc897a0`,
native `f5d3c3856536d53b8f8bfef51655cd062aed414adfeb68e6be80905be1d6d366`.
Before/after source maps match those build receipts. Production C inputs also
match before/after throughout this campaign. Box3D/WASM, shaders, physics
settings and scheduling defaults are unchanged.

Run the offline audit from any working directory:

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-ownership-after-bounds-2026-10-01/validate.py
```

It validates 61 portable raw files and all14 first processes without launching
physics. [Raw index](raw-index.json) pins every retained raw file.

Separate [CPU-first combined-viewer precommit captures](../pr02-compound-ownership-captures-2026-10-01/README.md)
complete all12 first300-step health checks and all six visual reviews. Village
retains a demonstrated existing shared-renderer capacity failure: nearby GPU
ground/buildings are absent. It is recorded underPR09, not counted as visual
parity or silently discarded. Renderer sources and private GPU callbacks are
unchanged by this ownership delta. PR02 remains open for compound getters and
actual viewer controls; final release qualification is still required.
