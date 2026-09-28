# Rain episode diagnostics

Run from `experiments/gpu-physics`. These scripts were used for the historical
600-step Rain investigation; they produce diagnostic excerpts, not full-state
qualification or replay inputs. Raw captures are local and are not in Git.

`select_health.py ordinary` (or `native`) validates the historical batch health
file under `artifacts/gpu-solver-qualification/rain-buffered-five`, and selects
the fixed twist and cone windows. `extract.py --help` accepts explicit trace,
health, output, window and Human identity arguments; it checks same-step slot
and generation identity. `analyze_compliance.py --help` evaluates a selected
spherical joint in the resulting excerpt. Adapt input paths to a new batch;
do not assume another device reproduces these event frames or identities.

See `docs/gpu-solver-qualification.md` from the repository root for the findings
and limitations. CPU reproduction and compliance estimates do not close the
physical acceptance gate.
