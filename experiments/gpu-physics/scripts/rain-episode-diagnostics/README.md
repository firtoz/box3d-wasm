# Rain episode diagnostics

Run from `experiments/gpu-physics`. These scripts were used for the historical
600-step Rain investigation; they produce diagnostic excerpts, not full-state
qualification or replay inputs. Raw captures are local and are not in Git.

`select_health.py ordinary` (or `native`) validates the historical batch health
file under `artifacts/gpu-solver-qualification/rain-buffered-five`, and selects
the fixed historical twist and cone windows. For a new capture, pass `--source`,
`--out-dir` and `--windows selections.json`. The windows file maps labels to
`[first_health_frame, last_health_frame, cell_base_creation, target_creation]`,
with inclusive zero-based frames. An explicit source requires explicit windows
to prevent accidental reuse of historical identities. The selector validates
the entire report and requires all 42 cell bodies throughout each window.

`extract.py --help` accepts explicit trace,
health, output, window and Human identity arguments; it checks same-step slot
and generation identity. Traces may start at a later core frame; recorded frames
must be consecutive and cover the entire requested window. It accepts v19
through v22 traces and emits diagnostic excerpts without upgrading historical
state coverage. `analyze_compliance.py --help` evaluates a selected
spherical joint in the resulting excerpt. Adapt input paths to a new batch;
do not assume another device reproduces these event frames or identities.
The analysis includes swing effective mass, cached-impulse compliance estimates,
cone excess and the alignment of step-start/end swing axes. Cone excess uses
libm atan2, so it is an approximate diagnostic rather than a bitwise match to
Box3D's polynomial atan2. Cached impulse divided by substep duration is not the
complete per-substep torque balance; stationary compliance estimates alone do
not explain a moving joint's error.

See `docs/gpu-solver-qualification.md` from the repository root for the findings
and limitations. CPU reproduction and compliance estimates do not close the
physical acceptance gate.
