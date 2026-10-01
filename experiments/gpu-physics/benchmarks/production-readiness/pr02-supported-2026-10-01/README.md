# PR02 supported native API behavior — focused checks (2026-10-01)

All 16 linked C processes and 20 exact Rust checks pass on the frozen ordinary
and native cached builds, using the RTX 4070 SUPER / NVIDIA Vulkan desktop.
These checks cover existing controls, warm-start caches, external forces, shape
replacement, mass and joint queries. **PR02 remains open** for the remaining
sensor/compound/mesh diagnostic populations and actual viewer controls. This is
neither final physical/repeat qualification nor performance acceptance.

| Existing fixture | Processes / fixed budget | Evidence scope |
| --- | --- | --- |
| C API settings | 4 / 4 | Ordinary/native × GPU/combined; independent worlds, owned settings, recreation, warm-start and body controls; combined getter reads GPU state |
| Warm-start fixture | 4 / 4 | Contact and spherical joint, 240 steps each at substeps 1/4; runtime off/on; combined CPU/GPU 13-lane state within unchanged absolute 1e-5 |
| Substep external forces | 2 / 2 | Combined ordinary/native; 3,744 CPU/GPU scalar comparisons each, substeps 1/2/4/8, force/torque, zero steps, uploads, sleep, deletion/reuse |
| Shape replacement | 2 / 2 | Combined ordinary/native; identity, metadata, owned geometry, mass, queries, bounds/debug shapes, sleep, contact retirement and events |
| Joint reaction queries | 2 / 2 | Combined ordinary/native; 2,856 CPU/GPU scalar comparisons each across nine types; nonzero reactions, frames, motors/limits, sleep and lifetime |
| Joint separation queries | 2 / 2 | Combined ordinary/native; 2,686 CPU/GPU checks each across nine types; mutation and independent CPU divergence prove GPU-backed reads |
| Exact Rust selectors | 20 / 20 | Ten per backend; 18 GPU processes and two host-only accumulator checks, direct cache/reset/child coverage and analytical force/mass/settings assertions |

Every assertion and physics tolerance is unchanged. Standalone warm-start cases
check finite motion; only combined cases independently compare all 13 CPU/GPU
pose/velocity lanes. The fixture emits incidental mean clocks for its last 180
of 240 steps; these are retained verbatim and **are not timing results**.
The wheel angular-separation API is unsupported by the pinned upstream release;
its zero behavior is checked and is not promoted to angular-constraint coverage.
Optional reaction stress parameters are absent. Loaded/chaotic physics, Rain,
long-duration lifecycle and final five-process repeatability remain under later
roadmap gates.

The [pre-launch protocol](protocol.json) freezes the cases, hashes, settings,
order, budget and first-failure stop rule. No production candidate or timed run
is included. All builds complete before the serial processes. The
[receipt](raw/receipt.json) contains every result, actual link command, linked
input/executable hash and per-process environment. Two preflight test-list checks
selected the exact names without initializing a GPU. No run fails, is replaced,
or is added to the budget. The stopped earlier tab-only UI campaign and exhausted
diagnostic campaign stay closed.

## Compiled-source proof and portable evidence

Current engine sources and all linked archives match the prior diagnostic
builds exactly; [applicability](applicability.json) records the content audit.
Invocation HEAD `7f302ef671a7c1fbb06829a0a5d7036a7a421d7c` provides context, not
binary provenance. Ordinary/native engine build receipts and complete input
archives remain in the preserved
[diagnostic dataset](../pr02-diagnostics-2026-10-01/README.md); their hashes are
pinned in this protocol. Four C adapter receipts separately identify generated
wrappers, pinned CPU objects and the ordinary/native Rust archives. The new
[fixture source archive](raw/compiled-fixture-inputs.tar.gz) includes actual C
inputs and Box3D headers with matching before/after hashes in this receipt.
No source is reconstructed from checkout metadata alone.

The [raw index](raw-index.json) contains 73 losslessly copied files, including
all build/result logs, immutable protocol, runner and input archive. Large local
executables are identified by hashes rather than checked in. Each GPU process
reports the actual NVIDIA Vulkan adapter; the post-run
[host context](host-context.json) records driver 610.57.04, OS and compiler,
explicitly as context rather than a compilation-time observation.

An initial offline packaging audit incorrectly required equality with the whole
old link-input map, which also contained that campaign's different fixture
source. All selected archive hashes already matched. The corrected audit checks
each linked archive and independently verifies this campaign's source snapshot.
That packaging failure is recorded in applicability; no GPU process was repeated
and no raw result was changed.

Validate portable evidence without starting GPU work:

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr02-supported-2026-10-01/validate.py
```

Preserved runners document exact invocations; do not rerun this closed campaign.
Linked coverage remains 415 symbols with 39 explicit exclusions; this focused
contract evidence does not qualify every implemented API. Broader capability,
runtime, release-build and performance gates remain unchecked in the
[production roadmap](../../../../../docs/goals/gpu-production-readiness.md).
