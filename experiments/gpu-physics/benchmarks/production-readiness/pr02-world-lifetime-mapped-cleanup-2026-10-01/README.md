# Combined cleanup after deliberate GPU root retirement

Two first processes pass42 observations: ordinary and native combined adapters
retain the genuine CPU world, mapping and owned geometry after only the GPU root
object is deliberately retired, then release all three through public
`b3DestroyWorld`. Recreation gives distinct GPU root/child identities; stale
cleanup preserves the replacement's CPU world and geometry. Final teardown
releases ownership.

This is a required adapter-lifetime case following the
[world-lifetime repair subset](../pr02-world-lifetime-repair-2026-10-01/README.md).
It is **not actual GPU device-loss or full runtime qualification**. The seven-file
production repair remains local/uncommitted, pending actual viewer controls and
CPU-first precommit recordings. PR01/PR02 stay open. No further production edits,
Rust/C adapter rebuild, older process repeat, physics step or timing run occurs.

The frozen budget is two fixture links and two first processes, ordinary then
native, with a600s watchdog and stop/retain/restore on first failure. Both fixtures
build before execution. Box3D/world/shape/material defaults remain unchanged;
only an opaque userdata tag and the deliberate raw GPU-root retirement drive the
lifetime check. Real CPU validity/mapping are checked directly; invalid CPU
mutators/destructors are never invoked. Actual NVIDIA Vulkan, RTX4070SUPER,
driver610.57.04 is selected in both processes.

`raw/receipt.json` pins all reused linked archive hashes and compile commands.
These match the actual changed/unchanged C member and freshly compiled Rust
source proof in the preceding report; there is no new binary-provenance claim
from checkout revision. The fixture and a precompile linkage-audit draft,
immutable protocol, exact source/binary hashes, environments and raw stdout/
stderr/build logs are portable here. The driver is terminal, exit0. No failure
occurred in this campaign; earlier campaigns' unfavorable outcomes remain intact.

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-mapped-cleanup-2026-10-01/validate.py
```

Do not rerun the archived driver. Continue with fresh viewer build receipts,
actual remaining GPU controls and affected scene recordings before committing
the production source. Laptop validation and final desktop timing/rendering
charts remain later roadmap gates.
