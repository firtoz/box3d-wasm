# Bounded GPU scheduling goal and recovery

Updated: 2026-09-30. Bounded goal complete. Implementation/evidence commit `91485a9`
was pushed to `origin/feat/gpu`; working tree and remote were verified synchronized.
Read `/home/firtoz/work/2026/box3d-wasm/docs/gpu-scheduling-goal.md` on continuation
and after compaction. Maintain it. This bounded goal takes precedence over older
Rain next actions; the older full solver goal remains unfinished.

## Full objective

Improve GPU solver scheduling performance on feat/gpu, using bounded benchmarks and preserving physics behavior.

Repository: /home/firtoz/work/2026/box3d-wasm
Work directly on feat/gpu. Fetch and integrate remote changes without overwriting local work. Commit and push the completed work.

Context:
- GPU engine: experiments/gpu-physics. Do not modify the Box3D submodule or WASM engine.
- Desktop: i9-9900K / RTX 4070 SUPER. Second machine: Ryzen 9 8945HS / RTX 4070 Laptop.
- Both machines’ CPU/GPU scaling measurements through 200k cubes are checked in under experiments/gpu-physics/benchmarks/machines/, with raw data and chart-generation scripts.
- At 50k cubes on the desktop, current global scheduling achieved 69.26 steps/s, component scheduling 7.82, and CPU 19.87.
- The previously suspected large regression was a benchmark configuration mismatch: the collector omitted --gpu-solver global. Historical global achieved 57.71 steps/s, so no global regression was established. Do not repeat that bisection without new evidence.
- The existing falling_cube_solver_schedules_match_through_impact test found zero position, rotation, and velocity differences across global, component, and batched-graph schedules for 15k cubes through step 330. This is fixture-specific evidence, not whole-engine qualification.
- Prior small-scene work already investigated global replay and adaptive color grouping. Read its findings before duplicating experiments.

Read AGENTS.md, docs/gpu-solver-goal.md, experiments/gpu-physics/README.md, and benchmarks/cube-regression/README.md. Use the goal-scratchpad skill to maintain a current recovery record, distinguishing this bounded goal from older unfinished Rain work.

Objective:
Determine why component scheduling is much slower for large connected cube scenes, establish where it helps independent small groups, and implement a measured scheduling improvement. Prefer one targeted bottleneck fix or an evidence-based scheduling choice over broad solver rewrites.

Plan:
1. Establish reproducible global/component baselines on exactly two fixtures: 50k falling cubes and the existing mixed-stacks fixture of independent two-box groups. Choose and freeze a practical mixed-stacks size before evaluating candidates.
2. Profile enough to attribute the gap to concrete work: dispatches, GPU phases, host preparation, synchronization, or component processing. Keep diagnostic runs separate from headline timing.
3. Implement a targeted improvement supported by that evidence. If choosing schedules adaptively, use measured topology/workload properties rather than machine-specific thresholds or benchmark names. Preserve explicit schedule overrides.
4. Validate behavior and measure the candidate against the frozen baseline. Do not pursue Rain, complex joint scenes, or a full 100–200k sweep during iteration.

Measurement requirements:
- Follow the checked-in benchmark protocol: native Vulkan, actual NVIDIA adapter, four substeps, sleep disabled, 90 warmup and 240 timed steps, and matching solver settings except the intentional scheduling change.
- Use at least three fresh-process trials per configuration, alternating baseline/candidate order. Report completed-step throughput and p95 latency.
- Record source revisions, binary hashes, adapter/driver, exact settings, and portable raw data. Do not treat invocation checkout metadata as proof of binary provenance.
- Set a small experiment budget before starting. Reject unhelpful candidates promptly; do not extend runs selectively to obtain a favorable result.

Acceptance:
- Explain the dominant measured cause of the component/global gap and identify the observed useful range of component scheduling—or report that neither tested fixture demonstrates an advantage.
- Deliver a production improvement with at least 10% lower median completed-step time on one target fixture versus the current applicable path, with no greater than 5% median or p95 regression on the other. Compare any automatic selection against explicit global as well, so merely improving a much slower component path is not presented as beating global.
- Preserve the existing schedule-equivalence, repeatability, island, and capacity checks. Add focused coverage only for new scheduling behavior; verify the independent-group fixture too. Do not relax physics tolerances to make the optimization pass.
- Record affected scene snapshots before committing solver changes, following AGENTS.md.
- Check in portable before/after evidence and readable charts with distinct CPU/GPU colors, preserving existing machine datasets. Keep changed protocols separate from the existing scaling comparison.
- Validate the improvement on this desktop. Provide a copyable second-machine command/prompt and compatible data format; do not claim laptop validation unless it was actually run.
- Update relevant documentation and the scratchpad, commit and push to feat/gpu, and report the measured gain, behavior checks, remaining limitations, and commit.

If the bounded experiments do not produce a qualifying improvement, restore rejected production changes and deliver the diagnosis and evidence. Explicitly report that the performance objective remains unmet rather than claiming completion or broadening into unrelated work.

## Frozen plan and budget

Workspace `/home/firtoz/work/2026/box3d-wasm`, branch `feat/gpu`, baseline HEAD and
fetched remote `b3454f8`. The tree was clean, no remote changes required integration.
Desktop: i9-9900K / RTX 4070 SUPER / NVIDIA 610.57.04 / native cached Vulkan.
User authorized commit and push. No delegation, Box3D or WASM edits.

Exactly two fixtures: 50,000 falling cubes and 4,096 mixed-stack dynamic bodies
(2,048 independent two-box groups). Four substeps, dt 1/60, no sleep, 90 warmup plus
240 timed steps. Global/automatic prefix 20; component kernel does not consume
that global grouping option. Exact settings remain in raw manifests.

Budget fixed before measurement: 12 GPU baseline +6 optional CPU controls,
4 diagnostics, at most 2 candidates/two pilots per candidate, then 12 alternating
confirmation runs for one candidate. Actual: all baseline/CPU/diagnostic runs,
ONEcandidate/two pilots, all 12 confirmation runs. No count sweep, Rain, joint
investigation, selective extra performance rounds or discarded unfavorable trials.
Builds/tests/recordings were separate from timing. Static format verification
copied existing data; it did not rerun measurements.

## Acceptance matrix

| Gate | Required evidence | Status and path |
| --- | --- | --- |
| Frozen baseline | 3 fresh alternating global/component trials on both fixtures, hash/source/settings | PASS — baseline 18 runs and baseline-build.json; portable raw bundle |
| Cause/useful range | GPU phases, host/wait, dispatches, topology, diagnostics separate | PASS — large component 116.257 ms/99.3% of component interval; one 49,697-body island; useful point 2,048 two-body islands; no crossover/range extrapolation |
| Performance | >=10% mean reduction on one; <=5% mean/p 95 regression on other; compare explicit global too | PASS — against component cubes−89.08%, mixedmean−1.74%/p 95+1.72%; against global mixedmean−57.18%/p 95−56.22%, cubesmean−.04%/p 95+1.05% |
| Behavior | Existing equivalence/repeatability/island/capacity +independent/new-policy fixtures; unchanged tolerance | PASS —36 selected test executions; sampled dense and independent state differences 0; explicit override/topology-hint reentry test |
| Visuals | Affected scenes before commit, real CPU first | PASS —19 GPU+19 CPU fresh 300 frame clips; all streams and final-frame pairs reviewed; recordings.json |
| Portable delivery | Raw/receipts/charts with CPU/GPU colors, second-machine commands | PASS — benchmarks/scheduling-2026-09-30; hash validation and diagnostic-free compatible recipe pass. Laptop validation pending, no claim made |
| Docs/commit/push | Relevant docs, recovery, clean delivery feat/gpu | PASS — implementation/evidence `91485a9` pushed; this follow-up records final recovery |

## Implementation and evidence

`GPU_PHYSICS_COMPONENT_TGS=auto` reuses the existing hysteretic dynamic-contact
density hint carried by asynchronous status. Dense graphs select global; sparse
ones select component. Existing world exclusions remain. No shader, arithmetic,
constraint order, physical-default or readback changes. Full replay already keys
the selected path; the new requested automatic policy is in host-state captures.
Native launchers now preserve explicit 0/1/auto overrides. Defaults stay component;
automatic is opt-in. This is bounded scheduling qualification, not whole-engine
qualification or a change to the separate GPU-native ordering default.

At 50k, median trial means component/global/auto are 127.137/13.884/13.879 ms;
p 95 s 138.342/17.237/17.417 ms. At 4,096 mixed bodies they are.819/1.879/.805 ms;
p 95 s.941/2.187/.958 ms. All headline configurations have 3 fresh-process trials.
Component/CPU rows use initial controls; global/automatic rows use the fixed
alternating confirmation. All earlier global controls and pilots remain separate.

Detailed report, raw bundle, JSON and PNG/SVG charts:
`experiments/gpu-physics/benchmarks/scheduling-2026-09-30/README.md`.
Second-machine command/prompt: `OTHER_MACHINE_PROMPT.md` in the same directory.
Existing machines datasets unchanged and their validator still passes.

Generated detailed evidence: `experiments/gpu-physics/artifacts/scheduling-2026-09-30/`.
Baseline executable SHA256 `0b0cc4b690385af129d7f8cdd5d4902c4786ef0efa7361c2bf3d4c29ea66c773`.
Candidate SHA256 `08b6ef9cc022291cafc907dfe64f07d742247ff7f575d083a55035ed040f7a27`.
The final release rebuild matches the measured candidate exactly. A clean CPU
oracle rebuild matches the CPU control hash; pinned clean Box3D revision and
oracle source hashes are recorded in final-build-verification.json. Receipt
patch/source hashes establish provenance independently of runtime checkout fields.

The diagnostic compiler patch was reverted before candidate edits. Diagnostic
phase summaries use only first 240 rows (91–330), excluding separate mirror pass.
Initial missing test import was fixed; failed compile log retained. A synthetic
format check first used unsupported symlink folders, then passed on ordinary
folders; this is not a benchmark failure/retry. CPU recording directory contains
4 extra historical clips: preserve them; only 19 newly requested CPU clips count.
Known late pile/joint CPU-GPU arrangement differences remain outside this bounded
behavior proof. Density is a heuristic, not an island-size proof. Hardware
underutilization is inferred from kernel timings/single-workgroup design; no
hardware occupancy counters were collected.

## Recovery and next actions

All validation is terminal. The exact dense automatic-repeat test passes in the
separate cfg(test) module (98374). A late assertion edit inside world.rs changed
executable identity; it was restored to the measured source hash before delivery.
Final release58423 now matches the measured08b6... executable exactly, and every
measured non-documentation code/build-input hash matches the final tree. No
performance trial was repeated. See validation-placement.json for the discarded
unmeasured binary identity. Documentation hash changes are recorded separately.
No build/test/benchmark/recording jobs remain.

Terminal sessions: baseline 30913,
diagnosticbuild 21997/diagnostics 21296, candidatebuild 23935, checks 94953,
policytests 59267, pilots 90656, confirmation 59677, recordings 35063,
finalbuild 82877; formatting/data-only validation may have just completed as 1247.
Verify actual process state rather than trusting handles. No additional GPU runs
are required. Release/CPU hashes and recordings are already verified.

All static hash/portable checks pass, including both existing machine datasets
and the diagnostic-free compatible format. Implementation/evidence `91485a9` was
pushed successfully. This documentation follow-up closes recovery state; no further
work is required for this bounded goal. Confirm the current delivery with
`git status --short --branch` and `git rev-list --left-right --count HEAD...origin/feat/gpu`.

Stop this bounded goal. Laptop validation is a provided follow-up, not claimed
complete or a requirement of this desktop goal. Do not resume the older Rain goal
implicitly.

Older full solver recovery: [gpu-solver-goal.md](gpu-solver-goal.md).
