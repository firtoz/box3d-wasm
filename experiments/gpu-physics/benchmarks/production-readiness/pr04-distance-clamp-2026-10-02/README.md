# Distance-joint timestep clamp: artifact candidate

The missing rigid-distance frequency clamp explains the recorded first-step
mismatch. One artifact candidate passes the complete original comparison on
ordinary/native NVIDIA Vulkan, each with ordering 0 and 1. **The production
shader is restored; the candidate is not committed as a solver change.** Relevant
regressions and CPU-first recordings must pass before retention. PR04's other
physical failures and the release qualification gates remain open.

## Cause and bounded change

Box3D's `b3PrepareJoint` clamps the constraint frequency to `0.25 * inv_h`.
At a 1/60 step its default 60 Hz becomes 15 Hz. The GPU rigid-distance branch
previously used 60 Hz directly. A float32 reconstruction of the first biased
pass predicts Y=-1.000042319 at 60 Hz and Y=-1.000284910 at 15 Hz, matching the
original GPU/CPU discrepancy. The archived CPU `joint.c`, `solver.h` and
`distance_joint.c` establish the formula and phase semantics.

The candidate adds only the timestep clamp in that rigid-distance branch.
Existing damping/frequency floors, spring/limit/motor equations, impulse caches,
warm-start rules, scheduling and scene defaults are unchanged. It addresses
this default rigid-distance fixture; it does not establish general spring,
zero-frequency or cache equivalence. The original shader and sole candidate
are preserved byte-for-byte in `raw/`, with all 107 candidate compiled inputs.

| Configuration | Original four-case comparison |
| --- | --- |
| Ordinary, ordering 0 | PASS |
| Native cached, ordering 0 | PASS |
| Ordinary, ordering 1 | PASS |
| Native cached, ordering 1 | PASS |

Each fresh process runs sphere-ground and distance, each at one/four substeps,
240 steps at dt 1/60, sleep/CCD disabled as in the original fixture. It checks
13 finite pose/velocity/angular-velocity lanes against independent real Box3D
CPU at absolute 1e-5, with the original small forces and warm-start toggles at
20/60/100/160. Total: 3,840 completed steps and 49,920 lane comparisons, all
within the original limit. Max errors are not printed by that unchanged fixture;
no exact trajectory equality or complete-state repeat claim follows.

## Provenance, budget and verification

Protocol SHA `c037ce0026875e3d8ac87e3b78a77a813e5db01c440763899f4183190898c2b5`
freezes one candidate, two Rust library builds, two C++ fixture links and at most
four correctness processes. All four execute once; ordering-1 cells were gated
on both full ordering-0 passes. Zero retries, test/viewer builds or headline
timing. Driver 72839 is terminal with exit 0. Both builds/links pass; source is
restored before any engine process, verified against all original input hashes.
A host preparation path error before protocol freezing ran zero builds/engine
processes and is retained in `raw/preparation-launch-failure.log`.

Actual NVIDIA RTX 4070 SUPER Vulkan (driver 610.57.04) is verified in every run.
`raw/receipt.json` records commands, toolchains, candidate source archive and
actual library/link/executable hashes. `raw/native-generated-inputs.tar.gz`
retains 455 generated/patched native dependency inputs. C fixtures reuse the
exact frozen combined wrappers and prefixed independent CPU archive; their
linked hashes and original producer receipts remain hash-linked. Invocation
commit `da44722` is context only, not binary provenance. These reuse cached
build dependencies and do not satisfy PR06 clean-build acceptance.

All 38 portable raw files (4,299,081 bytes), original failed baseline observations,
source, build and launcher logs are retained. The fixture's incidental `mean_ms`
output is diagnostic and cannot enter performance charts. Box3D/WASM and earlier
benchmark datasets are unchanged. Verify offline without a GPU or executable:

```sh
python3 /home/firtoz/work/2026/box3d-wasm/experiments/gpu-physics/benchmarks/production-readiness/pr04-distance-clamp-2026-10-02/validate.py
```

Next: freeze relevant unchanged regression selectors for the same candidate
sources, verify both backends/modes, then record all affected supported scenes
with real CPU first. Restore a rejected candidate and keep every unfavorable
result. Rain, loaded dragging, strict ragdoll trajectory, future-state capture,
clean builds, final repeats and performance stay separate required gates.
