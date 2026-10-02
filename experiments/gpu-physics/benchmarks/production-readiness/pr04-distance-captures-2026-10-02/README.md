# Retained rigid-distance clamp: CPU-first scene review

The narrow timestep frequency clamp is retained after the [original four-case
physical comparison](../pr04-distance-clamp-2026-10-02/README.md),
[242 applicable regressions](../pr04-distance-regressions-2026-10-02/README.md)
and this precommit scene review. The two actual mode-0 storage failures remain
failed in the regression report; the earlier contract requires that selector
only in mode 1. No tolerance, scene default or scheduling policy changes.
**PR04 remains open for strict ragdoll, loaded dragging and Rain failures.**

All three upstream distance-joint creation scenes were captured: Distance Joint,
Motion Locks and Joint Events. Real Box3D CPU completed all three first, followed
by ordinary GPU, native cached GPU, ordinary combined and native combined.
All 15 fresh processes completed 300 paced steps each with finite bodies,
no capacity loss, no sticky GPU failure and submitted/completed/rendered step
counts equal. Both combined views put CPU left and GPU right.

Protocol SHA `01020053c79e3a7296c2e0a1b8e66baa51467eb02e68e65244bdfc8663184344`
freezes three CPU and twelve GPU/combined clips, zero retries and timing runs.
CPU driver 95833 and GPU driver 38632 completed exit 0. The four
[artifact viewer relinks](../pr04-distance-viewers-2026-10-02/README.md)
reuse unchanged actual C objects with the exact tested candidate Rust archives;
these are not PR06 clean release builds. Receipts identify actual compiled
inputs, library and executable hashes. Invocation HEAD alone is not provenance.

Settings: upstream geometry/material/world defaults, dt 1/60, four substeps,
sleep enabled, no warmup, 300 completed/rendered steps, hidden UI, 1280×720.
Actual RTX 4070 SUPER NVIDIA Vulkan compute; Mesa/Xvfb diagnostic graphics.
Clips encode at 30 FPS and include startup/shutdown. Neither that encoding rate
nor incidental health clocks establish rendering FPS or headline step latency.
The later fresh desktop comparison must use actual desktop graphics.

## Visual review and limitations

CPU is first in each sheet, then ordinary/native GPU and both combined viewers:

- [Distance Joint](distance-clamp-joint-review.png): ground and suspended sphere
  visible, bounded swing. Original framing makes the sphere very small.
- [Motion Locks](distance-clamp-motion-locks-review.png): floor, four boxes and
  shadows visible; expected constrained motions remain bounded.
- [Joint Events](distance-clamp-joint-events-review.png): floor and three boxes
  visible. The leftmost free-joint body position differs between CPU and GPU;
  this review does not qualify that unrelated joint or event callback parity.

Four animation stages per clip were visually inspected. Timestamps are approximate
callback-cadence estimates, not exact synchronized physics checkpoints. Initial
299-step sampling selected black shutdown frames; those derived images remain
archived and corrected late-window 275 samples were reviewed. All original full
clips remain in `raw/clips/`. Combined purple GPU floor/blue body coloring is
intentional; split projection differs from the single viewer. No sample-wide
floor/widget/raycast acceptance follows; those remain PR09–PR11.

The repository comparison grid adds these three rows with the real CPU pinned
first. New clips/manifests preserve every existing dataset. `capture.json` was
written before the health assertion and retains its original `running` field;
`raw/capture/receipt.json` records the authoritative terminal pass for all cells.

## Retention and verification

`source-applicability.json` verifies all 107 retained inputs against the actual
candidate producer. Sole production difference: rigid-distance hertz clamp,
shader SHA `22fe506b97b97c24f724a09b2dad2efcd52db6b71b95d892bc3bc31c4726ea31`.
API, registry/layout, C routing and scene sources are unchanged. PR01/PR02 named
functional contracts remain applicable through this bounded source audit and
relevant candidate regressions; their old whole-source hashes remain historical.
General spring/zero-frequency/cache equivalence is not established by this fix.
Future-state, lifecycle, clean-build, final-repeat and performance gates stay open.

Verify portable records without a GPU or original binaries, alongside the linked
viewer/physical/regression reports:

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr04-distance-captures-2026-10-02/validate.py
```

`prepare-portable.py` archives existing artifacts and samples videos; it never
launches physics. Reproduction launch commands, selected environment overrides,
empty initial settings, ffmpeg logs/probes, health records, original scene/recorder
bytes and immutable protocol are in `raw/`. Driver caches and binaries are omitted.
Box3D/WASM remain unchanged. No performance claim or default switch is made.
