# Rain phase and partial residual diagnosis — 2026-10-02

The ordinary GPU Rain slowdown is concentrated inside the health/synchronization
phase. The complete 180-step diagnostic also retains failed original CPU-relative
joint residual screens, first at zero-based frame106 (completed step107).
**PR03 remains open and the engine is not production ready.** This is partial
baseline diagnosis; neither the earlier failed600-step runs nor the release
physics/repeatability/performance requirements are satisfied by it.

## Frozen budget and results

The [original protocol](raw/original/protocol.json) permits one C++ observer
compile, one link and exactly three ordinary GPU processes:30 observer-off,
30 observer-on and180 observer-on. Zero Rust/CPU builds, solver candidates,
retries or headline timing. Each process retains its original900-second watchdog.
The logger is an artifact-only copy of the existing hook; production sources,
geometry, defaults, API call order, timestep1/60, foursubsteps, sleep enabled,
eightworkers and efficient ordering0 remain unchanged. Rendering is diagnostic
Mesa/Xvfb at640×360; physics uses actual RTX4070SUPER NVIDIA Vulkan.

| Case | Engine result | Captured frames | Body / joint observations each | Meaning |
| --- | --- | ---: | ---: | --- |
| Observer-off control | exit0, zero Sokol errors | 30 | 18,480 | Retained without a rerun |
| Observer-on control | exit0, zero Sokol errors | 30 | 18,480 | Exact ordered captured semantic equality with control |
| Observer-on phase probe | exit0, zero Sokol errors | 180 | 272,160 | Complete partial record; original residual screens fail |

The original driver stopped after the successful control because `re.findall`
returned tuples and the harness compared them with lists. Its stopped receipt,
logs, source and error are retained. The [remaining-cell protocol](raw/remaining/protocol.json)
corrects classification, host-validates the existing control and consumes only
the two previously unlaunched cells. No rebuild or repeated process. The180-step
probe was launched only after exact equality of the30 captured control frames.
Equality covers public poses/velocities, identities, joints/limits/residuals and
named health fields with meaningful array order. It excludes clocks/presentation
metadata and does not establish complete future-state repeatability.

All three windows pass the unchanged complete-record, finite-state and spherical
capture-consistency checks. These checks validate capture/health availability;
they do not assert all constraints are physically qualified. The original
ordinary/native900-second600-step failures stay preserved in the earlier
[CPU/ordinary report](../pr03-rain-launch-timeout-2026-10-02/README.md) and
[native report](../pr03-drag-order-native-timeout-2026-10-02/README.md).

## Phase attribution and limits

The180-step probe emits2,340 progress records, including180 completed-frame rows.
Existing broad physics clocks sum338.608seconds; the nested health clocks
sum337.923seconds,99.798% of that interval. Health reaches5.498seconds in one
frame. Renderer submission clocks sum46.309seconds using software graphics.
[Exact phase summary](raw/phase-summary.json) retains every minimum/median/maximum
and sum. **These are diagnostic clocks, not headline step latency, rendering FPS
or a CPU/GPU performance comparison.** Health is already included in physics;
adding both would double-count it. The230.55/16.14second control-process elapsed
times include startup/cache effects and do not measure observer overhead.

[Source/clock audit](source-clock-audit.md) establishes that the generated sample
starts the physics clock before `b3World_Step`, then calls the health hook before
marking profile. The hook's first world health getter may perform GPU completion,
mirror readback, deferred host CCD and event processing. Subsequent body/joint
collection shares that phase. The picking query clocks are a later, independent
query and cannot isolate the health wait or CCD contribution. Progress identifies
a slow phase; it does not yet distinguish GPU solving from host CCD or collector
work. No production optimization follows from this result alone.

## Original residual screens, with the CPU prefix disclosed

A separately frozen [host-only protocol](raw/host-analysis/host-analysis-protocol.json)
permits one unchanged residual evaluation, zero engine processes/builds/candidates
or timing. It uses the exact first180 ordered frames from the previously completed
CPU600 record. The projected file changes only `frames_observed`, `timed` and
`measured` to180 and closes the JSON array; all frame semantics remain exact.
Its other whole-run header summaries still belong to CPU600 and are not timing
inputs. The portable verifier compares every projected frame with the original
lossless CPU record. This is not a new CPU180 trial.

All272,160 joint observations match by positive creation identity, typed/oriented
endpoints and captured constraint parameters. Both sides create2,520 bodies;
no body slot is recycled in this short window. Thus this cannot qualify Rain's
required600-step recycling. The unchanged +.05m/+.05rad CPU-relative screens fail:

| Screen | Failed joint observations |
| --- | ---: |
| Anchor | 48 |
| Angular | 117 |
| Cone excess | 919 |
| Lower twist excess | 2 |
| Upper twist excess | 1 |

The [full residual report](raw/host-analysis/residual-comparison.json) retains
first failures and peaks. At frame106 a revolute anchor residual is.290407062m
versus CPU.118746519m, and an angular residual is.164449975rad versus
CPU.111362636rad. At frame179 spherical endpoints creation33/34 show cone excess
1.05444145rad versus CPU.660036623rad and upper twist excess.160478294rad versus
CPU.00670570135rad. Chaotic CPU/GPU trajectories can differ; these screens locate
required discrepancies for diagnosis and do not themselves prove a root cause.
No limit changed, failed observation was discarded or physical acceptance inferred.
The host setup's initial wrong working-directory path is also retained; it
launched no comparison or engine process.

## Provenance and portable verification

Observer executable SHA256:
`bfaaecd3be07942d8858bd152fe7b8bab017568e09b76c345e693c9f07df0244`.
Observer object SHA256:
`deef95d8d46e305a9470ce3cab2f8013c3b98c3623e5953259e60c04d663dd1b`.
The exact compile/link commands reuse the parent ordinary viewer's124 compiled
units, objects/archives and production library. Only the hook object is replaced;
the link dependency-file output option is omitted to preserve parent artifacts.
Production ordinary library remains SHA`aba7a834443ba7174d92bcf39217929ce9af83adc28ea9c244943b5e72eb6ec6`.
Parent source archive contains723 inputs; the sole current local difference is
the already committed `cfg(test)` sleeper fixture, which is absent from production
libraries. Original source, observer source/header, commands, build logs, actual
link input hashes, evaluator bytes, settings and all process outputs are portable.
Invocation revisiona90705c is context, not compiled-source proof. Adapter/driver
610.57.04 are retained in actual engine logs and [desktop inventory](raw/desktop.txt).
Cached dependencies remain disclosed; this does not replace PR06 clean builds.

[Raw index](raw-index.json) covers54 files/68,998,785bytes, including four exact
lossless captures: both30-step GPU controls, GPU180 and the disclosed CPU600
prefix. Every compressed part is under48MiB. [Capture manifest](raw/captures.json)
records original byte counts/SHA256s and compressed-stream/part hashes.
[Producer references](raw/producer-references.json) pin existing committed source
archives/receipts and the original CPU600 lossless parts, without duplicating or
replacing older datasets. Executables/objects/pipeline caches remain local;
portable validation needs no ignored binary, GPU or live process.

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr03-rain-phase-diagnostic-2026-10-02/validate.py
```

Next: finish PR03's configuration/capture applicability record, then diagnose
required physical failures under PR04. Before another Rain run, freeze a distinct
phase-attribution hypothesis and finite budget using this evidence. Do not rerun
these closed cases or extend their watchdog. Floor visibility, sample UI/widgets,
raycast behavior/appearance and fresh desktop-only FPS/step-time charts remain
the later PR09–PR12 items in the [authoritative roadmap](../../../../../docs/goals/gpu-production-readiness.md).
