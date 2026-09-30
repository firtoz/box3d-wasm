# CPU/GPU cube scaling across machines

Each machine contributes its own `.json` dataset and `.raw.json.gz` bundle. The
bundle contains the original per-trial measurement JSON, direct-render cadence
samples and logs. The publisher rechecks successful measurements against raw
files; the plotter verifies hashes and recomputes the reported metrics before
rendering. Add a new machine/run ID instead of replacing another machine's files.

## Shared protocol

- Falling cubes: 100, 1,000, 5,000, 10,000, 25,000, 50,000, 100,000, 150,000 and 200,000 dynamic cubes, plus a static floor.
- Three fresh-process trials per path/count, 90 warmup and 240 timed steps, dt 1/60 s, four substeps, sleeping disabled, eight CPU workers.
- CPU and GPU **completed physics steps/s**, plus CPU and GPU physics in the **same direct instanced renderer** at 1280×720, unpaced. Rendered FPS includes frame/presentation work and is not inferred from physics time.
- Native cached Vulkan backend, global-color solver (the published large-cube setting). Build and shader startup are outside the timed window. Raw manifests identify the runner checkout, binary hashes, backend settings and machine CPU/platform; GPU physics results identify the actual adapter. Preserved binaries require separate build provenance; the runner checkout is not proof of their source revision.
- The 10 FPS early stop is disabled: slow paths still attempt 200,000 cubes. Invalid results, capacity limits and timeouts remain explicit stops. No missing point is extrapolated.
- Rate is 1000 divided by the median of trial mean milliseconds. Bands show the full trial range. p50/p95 are medians of per-trial percentiles, not pooled percentiles.

CPU and GPU rendering both use the graphics hardware on that machine. CPU curves
measure Box3D C; GPU curves measure the experimental engine. They are the same
workload, not the same solver implementation. Keep other CPU/GPU-heavy jobs stopped
while measuring. Eight workers is the fixed comparison setting, not necessarily
maximum CPU throughput on machines with more cores.

## Collect on another machine

The current collection helper targets the project's Linux native Vulkan backend.
Use the normal project toolchain (Rust/Cargo, CMake/C++ compiler, Python 3,
matplotlib for plotting, Vulkan driver and a working desktop display). Driver and
Git access must be available to the agent. Never substitute software rendering
or report physics throughput as FPS when the display is unavailable.

From the repository root, after pulling `feat/gpu` and initializing submodules:

```sh
CUBE_ADAPTER=nvidia experiments/gpu-physics/scripts/collect-cube-machine.sh \
  my-cpu-my-gpu-2026-09-29 'My CPU / My GPU'
```

Choose a unique lowercase ID and an accurate label. The helper builds the CPU
oracle, CPU renderer bridge and GPU executable before starting the sequential
sweep. An interrupted run can resume with the same ID, binaries and settings.
It publishes both data files and rerenders all checked-in machines. If the native
backend is unsupported, report that limitation; do not silently change backend.
An explicitly labelled ordinary-backend experiment can use the low-level runner
with `--backend ordinary --gpu-binary ...`; differing protocols require review
and `--allow-protocol-differences` when plotting.

Commit the new machine's two files and regenerated `charts/` files. Do not commit
local executables or raw working directories under `artifacts/`.

## Rerender or validate anywhere

### Desktop baseline and bounded regression investigation

The original component-TGS sweep is preserved under `component-baseline/`.
It contains all 27 CPU trials through 200k and completed GPU/renderer trials
before the user redirected work to 50k. It is not a global-solver comparison.
The historical fast cube charts used global-color scheduling; selecting component
TGS for that comparison was a measurement-configuration error. The collection
helper now selects global explicitly, without changing any engine defaults.

The user has now requested the full global-solver sweep through 200k. Omit
`CUBE_COUNTS` for that sweep; `CUBE_COUNTS=50000` remains available for a bounded check. Different count lists can share a chart when their
measurement conditions match; unmeasured points are never inferred.

### Portable chart generation

No GPU, binaries or ignored artifact directories are needed to rerender:

```sh
python3 experiments/gpu-physics/scripts/plot-cube-machines.py --validate-only
python3 experiments/gpu-physics/scripts/plot-cube-machines.py
```

Outputs: `charts/cube-scaling.png`, `.svg`, `.csv`, and `charts/README.md`.
Different measurement conditions (including solver choice) are rejected by default. Count lists may differ. Missing GPU or renderer
results remain visibly missing; `partial` datasets are not full CPU/GPU comparisons.

To add separately collected paths to an existing machine/run, use the same
protocol and ID:

```sh
python3 experiments/gpu-physics/scripts/publish-cube-machine.py \
  experiments/gpu-physics/artifacts/machine-scaling/additional-paths \
  --machine-id my-cpu-my-gpu-2026-09-29 --label 'My CPU / My GPU'
```

Additional count batches extend the dataset’s count list while preserving the original
batch manifests and raw samples. Existing trials must be identical; a different rerun needs a new ID. Keep CPU,
GPU and rendering batches on the same machine, source build and settings.

## Completed desktop sweep

The i9-9900K / RTX 4070 SUPER dataset contains all 108 successful trials:
nine counts from 100 to 200k, four paths, three trials each. Global scheduling
was used throughout. The saved 50k records were retained unchanged, and all
executable hashes match across both collection batches. No sweep is running.

GPU physics overtakes CPU between the sampled 5k and 10k points. At 200k,
CPU/GPU throughput is 4.19/18.84 completed steps/s; CPU/GPU rendering is
3.84/18.61 FPS. See [the chart and full table](charts/README.md).
The chart uses logarithmic axes to show the full range. Bands retain all trial
ranges; measurements were taken under normal desktop conditions, not fixed clocks.

The dataset identifier retains `50k` for continuity with the original batch;
its count list and chart cover the full sweep. Each additional computer should
use its own unique identifier and the collection command above. Rerendering
requires only the checked-in files, Python and matplotlib, with no GPU required.
