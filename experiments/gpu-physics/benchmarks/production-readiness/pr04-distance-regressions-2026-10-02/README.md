# Relevant regressions for the distance-clamp candidate

**242 applicable selected checks pass.** All 244 original results are retained,
including two actual mode-0 failures of the mode-1-only contact-order storage
fixture. The original runner's `all_assertions_pass=false` stays unchanged.
The production shader is restored; CPU-first affected-scene recording/review
is still required before retaining the clamp. PR04 and release readiness remain
open.

The [original four-case comparison](../pr04-distance-clamp-2026-10-02/README.md)
passed before this separate budget was frozen. It establishes the missing clamp
for the default rigid-distance fixture at absolute 1e-5, not every joint setting
or future-state repeatability. This campaign uses those same 107 candidate inputs
and unchanged existing assertions, with two test builds/four fresh processes.

| Backend / ordering | Actual selected results | Applicable checks |
| --- | --- | --- |
| Ordinary / 0 | 60 pass, one fail | 60 pass |
| Native cached / 0 | 60 pass, one fail | 60 pass |
| Ordinary / 1 | 61 pass | 61 pass |
| Native cached / 1 | 61 pass | 61 pass |

The copied group selected contact-order storage in mode 0 by mistake. This was
identified and recorded while the first process was running, without changing
its protocol or skipping a cell. The authoritative PR03 contract had already
frozen that fixture's **mode-1 prerequisite before the candidate**. Its original
baseline mode-0 failures also remain preserved. Neither new failure is called a
pass; applicability is assessed against that earlier criterion, without relaxing
an assertion or excluding any other result. Both unchanged mode-1 storage
controls pass here.

The 61-selector group retains all 28 numerical and 23 schedule/capacity selectors,
plus the existing slot-32768 regression and focused distance bounds, offset COM,
stored anchors, prismatic locks, revolute warm-start, joint wake/history and
reaction accumulator checks. Modes/backends use their original explicit cache/
scheduling overrides and original durations. Every individual selected test is
listed in the frozen protocol and stdout. All applicable assertions pass,
including schedule-equivalence/island/capacity checks; no new test source or
physics tolerance/default changed. This is selected regression evidence, not
final five-fresh-process/full-state qualification.

Protocol SHA `9bc5a3268462bf0138062ff1bf2eb5d8e391c87b5740d4902d185e2ac3669e1f`
freezes two test builds, four processes, 244 selected checks, zero new candidates,
library builds, retries or headline timing. Driver 7832 is terminal with exit 0;
its original failed test processes exit 101. Both compiler commands succeed and
preserve executable modes. The shader is staged only during builds, then restored
before all tests, verified against the original input hashes. No old campaign's
trace/cache file is written. Both builds match the exact prior candidate inputs
and all 455 generated native dependency hashes.

`raw/receipt.json` identifies source inputs, commands, toolchain, compiler
artifacts, executable hashes and NVIDIA RTX 4070 SUPER Vulkan banners. Source
bytes are archived; cached dependencies do not close PR06. All 21 indexed raw
files (2,930,412 bytes), original failures, logs and pre-candidate applicability
contract are portable. Test-duration clocks are incidental, excluded from charts.
Box3D/WASM and previous datasets remain unchanged.

```sh
python3 /home/firtoz/work/2026/box3d-wasm/experiments/gpu-physics/benchmarks/production-readiness/pr04-distance-regressions-2026-10-02/validate.py
```

[`acceptance.json`](acceptance.json) separates the applicable selected result
from the original runner's failed observations. Next is a finite precommit
build/recording protocol covering every affected supported scene with real CPU
first. Keep Rain, loaded dragging, strict ragdoll trajectory and all later release
requirements open. No production solver retention or performance gain is claimed.
