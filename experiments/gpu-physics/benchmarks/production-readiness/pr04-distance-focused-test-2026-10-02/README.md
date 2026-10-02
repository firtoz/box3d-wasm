# Durable one-substep distance regression

`gpu_invariants::distance_joint_one_substep_matches_cpu_reference` passes on
ordinary/native NVIDIA Vulkan, each in ordering 0/1. All four fresh processes
print GPU Y=-1.000284910, the same float32 value as the independent Box3D reference.
The original absolute 1e-5 limit is unchanged. The unclamped historical GPU
Y=-1.00004232 remains archived and would fail that limit.

The test mirrors the first step of the [original C comparison](raw/reference/distance-original.cpp):
static anchor without a shape, default-material radius-0.5 dynamic sphere centered
at [0,-1,0], length-1 default rigid-distance joint, gravity -10, damping zero,
sleep/CCD disabled, one dt 1/60 step with one substep. It checks 13 finite
pose/velocity lanes, valid state, no sticky GPU failure or capacity loss. The
reference comes from the independently recorded CPU value, not a reconstruction
of the shader formula. Full 240-step/four-case comparison evidence remains in
the [retained clamp report](../pr04-distance-clamp-2026-10-02/README.md).
The existing four-substep loose-bounds test remains unchanged; it would not
catch the concrete defect addressed here.

Protocol SHA `ffdf136068a06c89eb4500b4f169d4a070d3d802c651a4231b79ad4449126821`
freezes two test builds and four single-selector processes. All cells run once,
zero new solver candidates, separate production-library builds, retries or timing
runs. Driver 27540 is terminal exit 0. A preparation path error before freezing
launched no builds/engine; its host-only failure is retained. No outcome was
replaced, and no old campaign was repeated or extended.

Only the cfg(test) invariant module changes versus the tested clamp producer.
All 107 test compilation inputs are archived and hash-verified; all 455 native
backend generated inputs match that producer. Actual compiler artifact records,
executable hashes, commands, selected overrides, adapter banners and logs are
in `raw/receipt.json`. Invocation HEAD is context only. Test builds reuse cached
dependencies and do not satisfy PR06 clean release builds. Ordinary executable
SHA `0ad42ffac5bf0b9ad1597b780ec1266f3a6d995918f87cd742a8e79bd344dab4`;
native SHA `5c6c05b7536269554fbc933eb821331a0d2fcc08d4800c820cb7accae2e59381`.
Driver 610.57.04/RTX 4070 SUPER Vulkan applies to this desktop only.

Production solver, API, viewers, scenes, Box3D and WASM remain unchanged. Prior
physical, regression and CPU-first scene evidence retains its bounded production
applicability; the changed test-module hash is explicit and is not claimed to
match the older complete-source receipt. PR04's strict ragdoll, loaded dragging
and Rain failures remain open. Final PR07 qualification must include this new
selector alongside the frozen original matrix, with all existing limits retained.
No final-release, repeatability or performance acceptance follows from four
one-step checks.

Verify portable evidence without a GPU or original executables:

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr04-distance-focused-test-2026-10-02/validate.py
```

Portable references include the original failed baseline receipt and exact C
fixture plus prior Rust producer/regression protocol. Every raw file is indexed;
binaries and driver caches are omitted. Preserve all earlier datasets.
