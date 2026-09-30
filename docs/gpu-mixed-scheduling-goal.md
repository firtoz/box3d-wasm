# Bounded desktop mixed-topology scheduling goal

Read `/home/firtoz/work/2026/box3d-wasm/docs/gpu-mixed-scheduling-goal.md` at the
start of each continuation and after compaction. Pursue the full objective and
maintain this recovery record. Older scheduling work is complete; older Rain
work is unfinished and must not be resumed by this goal.

## Full objective and constraints

Improve mixed-topology GPU solver performance on i9-9900K / RTX 4070 SUPER in
`experiments/gpu-physics`, working directly on `feat/gpu`. Fetch and integrate
remote changes without overwriting local work. Keep Box3D and WASM unchanged.
Determine whether scheduling different island sizes separately improves measured
performance over the current automatic whole-world selection.

Freeze exactly three fixtures: existing 50,000 falling cubes; existing 4,096
independent-group bodies; new spatially separated 8,192-body pile plus 4,096
bodies in independent two-box groups. Preserve physics defaults and verify the
mixed fixture's actual island topology throughout the measured window.
Measure explicit global, component and current automatic schedules, record island
sizes and selected paths, and keep diagnostic GPU phase timings separate.
If supported by evidence, implement one targeted improvement, preferably small
islands on component and large islands on global. Preserve explicit overrides
and complete constraint coverage. No benchmark-name or machine-specific thresholds.

Native Vulkan on the actual NVIDIA adapter; four substeps, dt 1/60, sleep off;
90 warmup + 240 timed completed steps. Three fresh-process trials per
configuration, alternating order. Match every setting except scheduling.
Record source revisions, build receipts, executable hashes, adapter/driver, exact
settings and portable raw data. Invocation checkout metadata is not build proof.
Report median trial mean completed-step ms, corresponding steps/s and median
trial p95 latency. Preserve all results, including unfavorable trials.

## Frozen budget (before any measurement)

- 27 baseline runs: 3 fixtures x 3 schedules x 3 fresh trials.
- At most 6 diagnostic runs, separate from headline timing.
- At most 2 candidates, exactly 2 pilot runs each if evaluated.
- 18 confirmation runs for ONE candidate: 3 fixtures x baseline-auto/candidate
  x 3 fresh trials, alternating order.
- No selective extension. Builds, correctness checks and recordings are separate.

## Acceptance matrix

Evidence root: `experiments/gpu-physics/benchmarks/mixed-scheduling-2026-09-30/`.
Portable `raw.json.gz` contains receipts, patches, logs and all timing samples;
`README.md` explains results, failures and limits. Local binaries/videos remain
under `experiments/gpu-physics/artifacts/mixed-scheduling-2026-09-30/` and
`recordings/snapshots/`.

| Requirement | Status | Evidence |
| --- | --- | --- |
| Exactly three fixtures; physics defaults; intended topology | PASS | Fixed Rust/CPU constructors; diagnostic histograms at 13 checkpoints 91..330, largest pile 7,926–8,185, original 2,048 pairs disjoint, zero crossing; new mixed tests |
| 27 baseline controls, alternating fresh trials; build provenance | PASS | Protocol, per-process receipts, build logs/source patches/hashes; raw bundle validator |
| Selected paths and separate GPU diagnosis | PASS | Dispatch counts for every headline step; six separate diagnostic runs with GPU intervals and topology |
| >=10% mixed improvement; <=5% existing mean AND p95 regression; fastest controls | FAIL | Candidate2 mixed +6.337% mean; cubes +5.253% mean violates guardrail; slower than fastest explicit control on all three |
| Existing checks preserved; mixed comparisons/transitions/repeatability; unchanged tolerances | PASS | 68 relevant native test executions; final scheduling and check logs; mixed sampled errors zero, tolerance 1e-5 and exact repeats |
| Record affected scenes before commit; real Box3D CPU first | PASS | 300-frame CPU and restored-auto GPU mixed clips; shared wide recording view; recording receipts/hashes and visual review; old clips preserved |
| Portable data/charts/docs; existing datasets; Box3D/WASM unchanged | PASS | All 55 attempts retained; charts visually reviewed; delivery-validation.json records hash/settings/order/budget checks, prior dataset checks and unchanged protected paths |
| Restore rejected production changes if target fails | PASS | Byte-exact production restoration receipt; rebuilt executable matches measured baseline; final recording build differs only by optional MP4 camera |
| Commit and push completed fallback to feat/gpu | PASS | Delivery commit 0f99c8ac2644fc7b5990d5d8e058179096f2352c pushed to origin/feat/gpu; clean worktree and 0/0 synchronization verified |

The requested performance target remains UNMET. The user explicitly provided the
fallback: reject/restore production changes and deliver diagnosis and evidence.
Completing that bounded fallback must not be represented as meeting performance.
The bounded fallback is complete. No further performance work is active here.
Rain, complex joints, laptop validation, scaling sweeps and changing the default
policy are excluded. No new measurements or candidates are permitted.

## Current verified state (2026-10-01)

Workspace `/home/firtoz/work/2026/box3d-wasm`, branch `feat/gpu`, initial revision
`287cba5b88b7795965f1fb881293bac882738711`. Clean at start; fetched origin was
identical. Latest pre-delivery fetch is still identical (0/0). NVIDIA driver
610.57.04, adapter RTX 4070 SUPER. No active benchmark/build/recording jobs found
on the final process inspection. Do not trust a historical job handle as live.
Delivery `0f99c8ac2644fc7b5990d5d8e058179096f2352c` is committed and pushed;
portable validation passes from the committed dataset. This final recovery update
is a documentation-only follow-up. Resolve its current head with `git log -1`.

Budget fully consumed: **27 baseline + 6 diagnostic + 4 pilot + 18 confirmation =
55 fresh processes**. Every timing attempt completed and is retained; no discarded
trials or extensions. Builds, tests and clips did not run concurrently with timing.
Three configuration trials reversed order in trial2. Diagnostics have extra
readbacks/timestamps and full replay disabled and are excluded from headline means.

Confirmation median trial mean / median p95 (ms), auto → best rejected candidate:

| Fixture | Mean | p95 | Mean change / p95 change |
| --- | --- | --- | --- |
| Existing 50k cubes | 14.2494 → 14.9979 | 17.6918 → 18.3006 | +5.253% / +3.441% |
| Existing 4,096 independent bodies | 0.78087 → 0.80078 | 1.0222 → 1.0380 | +2.550% / +1.546% |
| Mixed 8,192 pile + 4,096 groups | 7.90903 → 8.41022 | 16.7104 → 17.1994 | +6.337% / +2.926% |

Initial explicit global/component controls and throughput are in the report.
Candidate2 is also slower than fastest explicit controls: cubes global +5.87%,
groups component +1.58%, mixed global +6.16%. Do not call beating slow mixed
component scheduling a win. CPU chart bars are preserved historical controls,
not new CPU timing; there is no mixed CPU performance claim.

Consequential decisions:

- Candidate1 used existing 8-body/32-contact small-island limits, local small
  solving plus global phases with repeated ownership checks. Pilots 9.5824/9.6286ms
  mixed: rejected. No scene/machine-specific threshold.
- Candidate2 built ownership once and stably filtered global color lists once.
  Pilots 8.4102/8.3724ms; fixed 18-run confirmation still fails. Diagnostics show
  only 0.1202ms global-solve saving versus 0.6317ms extra preparation/local/filter
  cost. This diagnoses these implementations, not every possible scheduler.
- First transition test wrongly assumed fixed 325 dispatches despite adaptive
  prefix (observed169). Replaced with component2/global>2 path assertions;
  physics tolerances unchanged. A private-field compilation error was repaired.
  Both failures are retained in logs.
- All rejected production shader/sim/world-policy changes are restored byte-exact
  to HEAD. No `split` override or default-policy change remains. Only new fixture,
  benchmark/recording instrumentation, mixed tests and evidence/docs are retained.
- Restored release SHA256 `25fa9372d9132643c8c885de4b779bc701bcf5dbe21d20ec39924c7fb8dcb203`
  equals measured baseline. Final recording-only build SHA256
  `68a3f46f00a2b338d09abc8d1f63a0ae6a6e96a54dbfcb5aeeedebcc2e3ecddb`
  has separate source/build receipt; all current engine inputs match that receipt.
- Initial clips cropped the groups. Kept them locally and recaptured the new scene
  with `GPU_RECORD_VIEW=-220,290,-300,95,8,145`; CPU first, both regions visible.
  MP4 camera changes affect rendering only. No solver changes remain to record
  other scenes. CPU submodule is pinned and clean.

Behavior evidence: five final scheduling tests pass (mixed global/component/auto
comparisons and transition/repeat included), plus 63 preserved relevant component,
dense schedule, long replay/reentry, island/capacity and precision executions.
Mixed candidate comparisons were also zero at 1,65,78,79,90,150,330 before rejection.
This does not claim whole-engine, Rain or laptop qualification.

Provenance distinguishes invocation metadata from compiled inputs. Build receipts
capture independent engine source hashes, base+patch, build command/log, native
configuration and frozen executable hash. First diagnostic test-only additions
made after compilation have an explicit reconstructed original input hash note.
AGENTS.md's release WASM path is absent in this checkout; no WASM build/edit was
performed. Protected Git paths and clean Box3D are the unchanged evidence.

## Recovery and next actions

1. Read this file and inspect Git/process state before continuing. Do not resume
   old Rain or rerun timing: this experiment's budget is exhausted.
2. No experiment work remains. Preserve the committed data, patches and failed
   results. Any alternate scheduler requires a separately authorized goal/budget.
3. At handoff report target unmet, regressions, restored policy, checks, desktop
   limits and the delivery commit. Do not resume timing from this scratchpad.

Commands from `experiments/gpu-physics`:

```sh
python3 scripts/plot-mixed-scheduling.py benchmarks/mixed-scheduling-2026-09-30 --validate-only
python3 scripts/plot-cube-machines.py --validate-only
python3 scripts/plot-bounded-scheduling.py benchmarks/scheduling-2026-09-30 --validate-only
```

Reproduction of rejected candidates requires their archived source patches at the
recorded base or preserved executable. The final production build intentionally
has only existing 0/1/auto scheduling. Do not invoke the collector to add runs here.
