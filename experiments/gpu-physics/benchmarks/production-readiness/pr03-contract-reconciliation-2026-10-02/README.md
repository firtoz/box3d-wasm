# Frozen production qualification contract and baseline

PR03's contract/baseline gate is complete. **The engine is not qualified for
release.** PR04 starts with the recorded distance first-step discrepancy;
Rain residuals, loaded dragging and the strict ragdoll trajectory screen also
remain failed or incomplete. No production/default/scene change or new engine,
build, diagnostic, candidate, retry or headline timing run occurred here.

[`qualification-contract.json`](qualification-contract.json) reconciles 127
original process observations with their unchanged results, exact commands,
settings/protocol references and binary hashes. It links frozen source archives,
actual producer/link receipts, CPU oracle, toolchains and NVIDIA Vulkan evidence.
The invocation revision is context only. The production source milestone is
`abc0a54a3ce3b285c9984f2c7c6b5baefff00d1b`; its 107 archived Rust/WGSL/build inputs
are verified. The only current input difference is the independently qualified
`cfg(test)` sleeper-fixture correction, not production code.

## Required final matrix

Ordinary wgpu and native cached Vulkan each require ordering 0 and 1, at least
five fresh full-duration processes per selected configuration on PR06 binaries.
The manifest freezes 101 required Rust selectors, 13 trace recipes and 14 C/scene
recipes. Their original geometry/defaults, durations and numerical/physical
limits are in the hash-linked source fixtures, evaluators and
[fixed criterion table](../pr03-rust-baseline-2026-10-02/release-contract.md).
Rain and ragdolls require 600 steps; loaded dragging requires 3,060. Short,
interrupted, historical or pose-only captures cannot pass final qualification.

Two applicability corrections are explicit: the contact-order storage fixture
requires ordering 1; native replay requires the native backend. Their original
failed/inapplicable observations stay retained. Schedule/equivalence/island/
capacity tests remain required in both ordering modes. The existing body-slot
32768 regression was omitted from the baseline; it is now explicitly mandatory
in all four final configurations, at its original eight steps/four substeps.
The 30,000-cube insertion test cannot replace it.

Full future-state capture must include the new world registry/root and child
epochs, retired/exhausted slots and C metadata allocation/ownership/callback
state. Schema24 and the current public health CSV do not prove that coverage.
The generation-1 diagnostic setters and Rust/C world-index-range disagreement
are concrete PR05/PR07 audit inputs. Capture implementation, lossless/compact
negative controls, clean builds, five-run physical/state equality, performance,
floor/UI/raycast checks and desktop-only charts remain in their respective gates.

## Preserved baseline

| Family | Original processes | Result interpretation |
| --- | ---: | --- |
| Current C/Rust API | 52 | Functional assertions pass; not all physical behaviors |
| Rust matrix | 33 | 199 checks: 195 pass, four original failures retained |
| Distance | 2 | Both abort at step 0/lane 1 under absolute 1e-5 |
| Ragdoll / isolated / mesh | 24 | Physical/isolated/mesh pass; strict trajectory screen fails |
| CPU Rain / ordinary Rain | 2 | CPU 600 complete; GPU 900-second timeout |
| Drag / order controls / native Rain | 7 | Isolated and order-1 pass; loaded drag fails; Rain timeout |
| Corrected sleeper | 4 | Both backends × ordering 0/1 pass original 400+1 assertions |
| Ordinary Rain diagnosis | 3 | 30/30/180 complete; original 180-frame residual screens fail |

The complete CPU Rain record and all failed/incomplete observations remain in
the original reports; the 180-frame diagnosis has no recycling and cannot stand
in for 600-step qualification. Both older scheduling budgets remain exhausted,
with the mixed performance target unmet. No default policy changes here.

## Offline verification and experiment rules

From any directory, run:

```sh
python3 /home/firtoz/work/2026/box3d-wasm/experiments/gpu-physics/benchmarks/production-readiness/pr03-contract-reconciliation-2026-10-02/validate.py
```

The verifier checks every hash-linked reference, all 700 unique upstream indexed
raw files, source applicability, original result records, durations, selectors
and final configurations without a GPU or ignored executable. It does not turn
failed assertions into passes or replace the original source-linked evaluators.
[`acceptance.json`](acceptance.json) audits the PR03 requirements specifically.
Frozen copies of the older solver criteria preserve those limits when mutable
documentation changes; original datasets are unchanged. Metadata draft repairs
are disclosed in [`raw/metadata-repairs.md`](raw/metadata-repairs.md).

Before any later experiment, freeze its distinct hypothesis, exact source/build
identity, settings, order, finite process/build/diagnostic/candidate budget,
watchdog and stop/retain rule. Preserve every unfavorable result; no unchanged
retry, watchdog extension or selective budget expansion. Relevant code changes
invalidate affected evidence and require a recorded applicability/refreeze audit.
Box3D/WASM and old benchmark datasets remain unchanged.
