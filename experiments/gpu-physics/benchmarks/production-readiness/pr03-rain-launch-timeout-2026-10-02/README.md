# Current Rain baseline: CPU complete, ordinary GPU incomplete

The real Box3D CPU viewer completes the original 600-step Rain window. The
ordinary GPU viewer reaches its frozen 900-second watchdog without writing its
health record. This is an incomplete GPU observation, not a physical pass or a
performance comparison. The GPU production-readiness gate remains open.

| Case | Outcome | Evidence |
| --- | --- | --- |
| Original launcher | Failed before any engine launch | `xvfb-run` unavailable; zero engine cells consumed |
| Real Box3D CPU Rain | Complete, child/wrapper exit0,600frames/0Sokol errors |1,948,800body and1,948,800joint observations; original record/spherical checks pass |
| Ordinary GPU Rain |900second watchdog timeout | Actual RTX4070SUPER Vulkan; no child-exit receipt or health file; incomplete |
| Native Rain / four dragging / two order1 controls | Unlaunched in this stopped campaign | Separate remaining-cell protocol runs only these seven cells; no CPU/ordinary Rain repeats |

The [original protocol](raw/original-launcher/protocol.json) freezes nine first
processes, zero builds/candidates/retries/headline timing, with stop/retain on
setup/input/adapter/timeout failures. The [display dependency repair](raw/display-repair/protocol.json)
uses the previously successful bundled X server and explicit viewer/server exits.
It retains the failed setup and changes only launcher/output paths; all engine
binaries, scenes, defaults, settings and evaluator limits remain unchanged.
Driver70354 exits1 after the ordinary GPU timeout. No watchdog extension or
replacement attempt is inferred. Every original failed/complete/unlaunched
outcome is preserved in the [receipt](raw/display-repair/receipt.json).

Rain uses its original scene/default geometry,dt1/60,four substeps,sleep enabled,
eight CPU workers,zero warmup/600timed steps,unpaced640x360 diagnostic presentation.
A fresh empty settings file prevents persisted UI settings from changing setup.
Creation/lifetime and spherical-limit capture are enabled. Full GPU semantic-state
capture is not enabled. Mesa/llvmpipe Xvfb graphics are diagnostic; GPU physics
selects the actual NVIDIA Vulkan adapter, desktop driver610.57.04. These traces,
process elapsed times and viewer clock fields must not feed headline charts.

Compiled engine source remains `abc0a54`; ordinary viewer SHA
`5f62cad9c9a0b1cfee92cc569cefa7728f8e14d03807d4943a438b4e65229bea`,
real CPU viewer SHA`6c65fabab6fbd048a321061c8f9b2bb061afdcfa450d57b1bbc5ff96d5e6d14e`.
The [portable receipt references](raw/portable-references.json) link exact producer,
source-applicability,actual119CPU/124GPUtranslation-unit/link receipts and the
pinned display dependency. The previous [viewer build report](../pr02-world-lifetime-viewer-builds-2026-10-02/README.md)
and [CPU source-applicability report](../pr02-world-lifetime-scene-captures-2026-10-02/README.md)
retain their source archives,generated compiled main bytes and cached-dependency
limitations. Checkout metadata alone is not provenance; these are not PR06
clean builds.

The44indexed raw files include protocols/drivers,original evaluators,all output
and display logs,CPU record checks,settings and the exact complete CPU health
record. Its1,203,549,772original bytes, SHA
`48918163f059dcbca3c5f8c067191442581eb9cb8510170c9ea3171a25879cf6`,
are losslessly compressed to251,743,429bytes in six≤48MiBparts. Concatenate the
parts in manifest order, then decompress the single gzip stream to recover the
original JSON. No frames, fields, float digits or unfavorable observations were
removed. Streamed decompression verifies the original byte count/SHA; the offline
validator rechecks all600frames with the original complete-record and spherical
limit checks. Ignored executables,objects,libraries and driver pipeline-cache
blobs are excluded; older datasets remain intact.

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr03-rain-launch-timeout-2026-10-02/validate.py
```

This validates complete CPU capture and retained failures without local binaries
or GPU execution. It does not establish GPU Rain residual health,repeatability,
FPS,completed-step latency or release readiness. Do not rerun the stopped engine
drivers. The production roadmap records the remaining-cell protocol and next
source-supported diagnosis; all original physical limits remain in force.
