# PR02 current native API acceptance — 2026-10-02

The named PR02 API and truthful-control gate passes on retained source
`abc0a54a3ce3b285c9984f2c7c6b5baefff00d1b`. This closes the accumulated functional
API work; physical correctness, runtime safety, final builds, repeatability and
performance remain PR03–PR08. The engine is not production ready.

[Acceptance matrix](acceptance.json) reconciles every PR02 requirement to
current or explicitly applicable retained evidence. All **32 C world tests**
and **20 Rust tests** pass their unchanged assertions: **16 Rust GPU tests and
four host calculations**. All GPU processes select NVIDIA RTX 4070 SUPER/Vulkan,
driver610.57.04. There are no production edits, new engine/viewer builds,
performance trials or repeated completed processes in this campaign.

## Results and scope

| Contract | Current result |
| --- | --- |
| World/body settings and warm-start control | Four standalone/combined/backend cells each; independent per-world state, CPU forwarding and cache behavior pass |
| Counters/profile/capacity/bounds | Four cells each for diagnostics and sensor/compound/mesh populations; availability and invalid-world errors pass |
| Compound properties | Four independent CPU/GPU cells,1016 observations/cell, zero mismatches at the original1e-5 limit |
| Applied forces | Both backends,3744 independent CPU/GPU scalar comparisons/cell,1/2/4/8 substeps |
| Joint reaction queries | Both backends,2856 independent CPU/GPU comparisons/cell across nine joint types |
| Joint separation | Both backends,2686 checks/cell; wheel angular query retains the documented upstream zero fallback |
| Replacement/ownership/AABB | Both backends; original mass, metadata, identity, events, geometry and lifetime assertions pass |
| Rust contracts | Ten exact selectors/backend; original force/cache/body-state/mass/replacement and warm-start assertions pass |

Fixture-specific tolerances and settings are unchanged; not all checks use the
same epsilon. Warm-start stdout contains incidental clocks, retained verbatim
and excluded from performance claims. Ordinary wgpu and native cached Vulkan
remain separate configurations; no automatic scheduling/default change.

The [exact inventories](api-inventory-ordinary.json) ([native](api-inventory-native.json))
list all **415 stateful header operations per backend/linkage**:376 implemented
candidate operations and39 explicit exclusions. Each row identifies its current
C/Rust definition and source hash. The underlying current linked audit finds
zero missing/stub/known-placeholder/duplicate/CPU-only passthrough entries.
“Implemented” is not universal physical or runtime qualification. The named
contracts above are tested; remaining release behavior is qualified by the
roadmap's full physical/runtime matrix. Macro-generated definitions use a
finite source-index expansion; their capped line locations are not independent
compiler proof. Actual build/unit/archive receipts supply compilation identity.

All156 current excluded C definitions across the four cells match the successful
compiled error contracts, including the idempotent closed-stderr save repair.
The39 recording/player/CPU-worker/static-tree operations publish ENOTSUP and a
sticky thread-local operation name; creation returns NULL and saving returns
false without creating a file. The [original report](../pr02-api-2026-10-01/README.md)
retains every earlier error-harness and link failure. No native recording support
or new exclusion is inferred.

Current [PR01 controls/CCD/repeat checks](../pr01-current-acceptance-2026-10-02/README.md),
[world/child lifetime checks](../pr02-world-lifetime-repair-2026-10-01/README.md),
[mapped cleanup](../pr02-world-lifetime-mapped-cleanup-2026-10-01/README.md),
[actual four-viewer controls](../pr02-world-lifetime-atomic-apps-2026-10-02/README.md)
and [52 CPU-first scene clips/26 reviewed pairs](../pr02-world-lifetime-scene-captures-2026-10-02/README.md)
remain directly applicable to these exact sources. Original failures remain
preserved. Village's rendering omission, narrow widget layout and sample-wide
query appearance remain PR09–PR11; this gate does not claim visual parity.

## Frozen budget and retained harness stop

[Original protocol](protocol.json) freezes32 first links,32 C processes and20
Rust processes,1200s watchdog/process, stop on first failure, zero retries,
new engine/viewer builds, candidates and timing. Source/geometry/defaults and
original assertions remain fixed. Compiler logs and every actual result are
portable; large executable/archive binaries remain local with recorded hashes.

The driver stops after the nineteenth Rust process exits0 with one passing test:
it wrongly expects an adapter banner from the pure host
`reaction_impulses_use_solver_accumulators_for_every_joint_kind` selector. The
[stopped receipt](raw/receipt.json), original driver and protocol remain unchanged.
The [source review and remaining protocol](raw/remaining-host/protocol-before-work.json)
show that this test only constructs `JointGpu` data and evaluates host impulse
arithmetic; it creates no device/world. Only its previously unlaunched native
counterpart is run. The original ordinary result is retained, never replaced.
Total Rust consumption remains20; the truthful classification is16 GPU+4 host,
correcting the original18+2 classification. C reaction fixtures separately test
actual GPU worlds against the independent CPU for all nine types. No failed
physics result or tolerance is reclassified as success.

## Provenance and offline review

[Raw index](raw-index.json) hashes197 files: all compiler/process logs, both
protocols, receipts, exact fixture/header inputs, reviewed generated C/Rust
sources, inventories and the preserved classification failure. Source identity
comes from [origin receipts](raw/origin-receipts/viewer-parent.json), actual
compiled units/linked archives and the [post-campaign source review](raw/post-campaign-source-review.json).
All107 Rust inputs and723 viewer/build inputs still match the producers.
Invocation `c2fd69f` is checkout context, not proof of binary provenance.
Cached dependencies and viewer instrumentation are disclosed in those receipts;
final clean consumer builds remain PR06.

Run the offline validator from any checkout:

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr02-current-acceptance-2026-10-02/validate.py
```

It hashes portable bytes, verifies all actual outputs/protocol consumption and
source receipts, and launches no GPU/compiler process. For a future release,
first freeze a new applicable build/matrix/budget. Do not rerun this closed
campaign or substitute its named API passes for Rain, long lifecycle,
complete future-state repeats or final desktop measurements. The bounded mixed
scheduling target remains unmet and its original exhausted datasets remain intact.
