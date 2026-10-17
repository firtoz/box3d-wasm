# PR04 behavior and throughput recovery

Read this FIRST on continuation, compaction or restart, then `AGENTS.md`,
`docs/goals/gpu-production-readiness.md` and the goal-scratchpad skill.
Before solver edits read `docs/gpu-solver-goal.md` and relevant rejected
experiments in `docs/gpu-solver-qualification.md`. Verify actual process state.

## Objective and authorization

Complete PR04 on `feat/gpu` in `/home/firtoz/work/2026/box3d-wasm`:
physically plausible GPU behavior and useful completed physics steps/second on
ordinary and native NVIDIA Vulkan. Ragdolls may follow different trajectories
and settle differently from CPU. They must retain articulation, collisions and
support without unexplained explosions, severe stretching, persistent jitter or
energy gain. Cross-engine bit, pose and trajectory matching are excluded;
applicable physical, functional and identical-input calculation limits remain.
Preserve geometry, defaults, identities, picking, Box3D/WASM and user work.
Use TypeScript/Bun for new automation. Stop after PR04, not the full roadmap.

The active app goal already contains this objective. The latest user request
authorizes committing/pushing the current checkpoint and supplying reusable goal
text. It is not a request to certify PR04 or launch another campaign during
publication. The six-gate plan, pivot and stop contract are in the roadmap.

## Acceptance status

**0 done / 6 left.** No production solver repair has been made in this checkpoint.

| Gate | Evidence required | Status |
| --- | --- | --- |
| A | Audit failures/rejections; classify trajectory-only assertions; establish matched baselines and freeze numeric physical/performance limits before candidates | OPEN — timing observations exist; acceptance audit and targets unfinished |
| B | Rain/articulated behavior, identity and recycling over 600 steps on both backends | OPEN — no current GPU Rain600 physical pass |
| C | Independent dragging/CCD, landing/release/support and applicable 3,060-step checks | OPEN |
| D | Affected contact/mesh/joint/motor/limit/sleep/lifecycle regressions | OPEN |
| E | Frozen completed-step throughput and latency targets on both backends | OPEN |
| F | Final applicability, cleanup, docs and affected PR06 refreeze/reopening | OPEN — checkpoint cleanup verified; final qualification remains |

Runtime source `442689de1`, planning baseline `cebc1e08d`. PR05/06 release candidate
remains 20 files / 367,035,504 bytes under
`experiments/gpu-physics/artifacts/pr05-pr06-release-candidate-2026-10-17`.
Its next consumers are focused PR04 fixtures and release applicability review;
preserve it and shared build/driver caches. Changes in this checkpoint are docs,
diagnostic fixtures, bounded TypeScript automation and compact evidence only.

## Budgets and completed measurements

Per blocker: at most three 45-minute batches, at most two hypotheses and two
variants per hypothesis per batch. Freeze exact builds, runs, watchdogs, progress
criteria and storage/cleanup before launch. Pivot after two unproductive
candidates or a batch without causal, physical or performance gain. Do not renew
old exhausted budgets. Default new temporary data 1 GiB, retained evidence
64 MiB, free space at least20 GiB; owned process groups, no cores, cleanup on
failure and success. Credible approaches exhausted means report unsuccessful;
promising unfinished work at the ceiling means report partial, not success.

The separately frozen no-candidate viewer baseline phase is CLOSED/FAILED.
`baseline-protocol.json`: six cells, CPU first,600 completed steps, CPU90s/GPU180s,
1,100s phase, zero builds/candidates/retries/full-health scans/captures. Four cells
completed; ordinary Rain timed out. The distinct native-remaining phase launched
only its previously unlaunched native Rain cell, also timed out at180s. Do not
rerun these cells or present timeout as acceptance. Original receipts/logs remain.

Normal viewer measurements (single development runs, not repeated qualification):

| Scene | Real Box3D CPU | Ordinary GPU | Native GPU |
| --- | ---: | ---: | ---: |
| Falling Ragdolls,600 steps | 7,476.48 completed SPS | 98.52 completed SPS | 106.48 completed SPS |
| Rain,600 steps | 390.25 completed SPS | 180s timeout | 180s timeout |

Correction found during publication audit: these viewer JSON records report
**workerCount=8**, not one. The headless fixture explicitly uses one worker.
Do not compare their CPU rates as matched one-worker measurements. Full600-row
timings, p95/max and asleep tails are in the four `baseline-*.json` files. The
asleep ragdoll tail is not an active-body speed claim. Health scans were disabled;
default zero health fields establish no physical pass.

Rain/performance batch1 is CLOSED, with useful causal evidence but incomplete
controls. **1/3 batches used,2 left**; dragging/CCD0/3 used. No candidate selected.
`rain-batch1-headless-protocol.json` froze four C compiles plus one C++ compile
(15s each), three links(25s each), six fresh runs(CPU30s/GPU90s),700s phase;
zero engine builds, solver candidates, retries or captures. Original release
`CreateRain`/`StepRain`,600 steps, worker1, dt1/60, four substeps, recycleDistance
0.05, unchanged defaults and explicit GPU completion/mirror synchronization.
Two arms: viewer setters versus engine-only, CPU first on both backends.
Raw cap128MiB; total new data/cache growth1GiB; retained64MiB/free20GiB.

| Control | Completed | Outcome |
| --- | ---: | --- |
| CPU with viewer setters | 600 | Finite/valid endpoint,300 humans/4,200 dynamic bodies,90.43 physics SPS |
| CPU engine-only | 600 | Same endpoint digest `187d68e48d2ebcef`,90.44 physics SPS |
| Ordinary GPU with viewer setters | 407 | 90s timeout, no endpoint/pass; final20 mean394.75ms,2.53 completed SPS,p95406.69ms,max429.30ms |
| Ordinary engine-only; both native arms | 0 | Unlaunched when phase stopped; explicitly unreviewed |

H1: viewer/render overhead dominates Rain wall time. Headless ordinary stepping
still costs about400ms per loaded step with no renderer, so viewer cost alone
cannot explain this slowdown. The clock includes queue completion and CPU
mirror synchronization, not just GPU kernels.

H2: redundant unchanged setters dominate. Their direct final20 mean cost was
0.00131ms, far below stepping. An indirect effect on idle/cache state is still
unresolved because the GPU engine-only arm was not launched. Do not call that
hypothesis fully falsified or turn this observation into a production patch.
Endpoint digest/finite checks are not joint/support/energy acceptance.

## Cleanup and publication verification

All three phases are terminal; prior session49464 returned failure after the
ordinary headless watchdog. No live jobs or reusable raw objects/executables.
The driver removed19,934,931 bytes across the three receipts. `raw/` is ignored
and empty; consumed normal JSON/cwds, objects and executables are gone.

`checkpoint-audit.ts` is a read-only audit, bounded externally to25s(du10s), no
GPU processes/builds/captures. `checkpoint-audit.json` verifies all17 distinct
pinned inputs, frozen protocols and retained log hashes, all recorded PIDs and
groups absent, no raw data, and TypeScript transpilation. It records retained
bytes, free space and shared caches after these phases. Shared caches remain
preserved. Task evidence is about1MiB. Failure receipts remain FAILED;
audit integrity PASS is not physics acceptance.

## Next discriminating work

1. Finish A's physical/function versus deferred-trajectory classification and
   freeze justified numeric throughput/latency targets before candidates.
2. Use a newly frozen small profiling protocol to split the loaded headless
   step into encode/submit/completion/readback/host-CCD costs. Check current
   source first; preserve causal alternatives, including indirect setter
   invalidation. Do not restart the viewer campaign or extend its cap.
3. Choose the first demonstrated mechanism and smallest reproducer. Test
   plausible algorithmic alternatives if local fixes stall; full Rain confirms
   promising repairs rather than serving as every inner-loop test.

Historical rejections include angular refresh/extra waves trading cone errors
for anchor errors, coupled cone/point and hinge blocks worsening other limits,
and passive warm-cache line search or motor projection alone being insufficient.
Read the later coupled-reference investigations before proposing a supposedly
new coupled algorithm. Earlier ragdoll trajectory failures are separate from
its historical physical pass. Old loaded-drag CPU/GPU pose deltas are not
independent mouse-anchor residuals; retain actual physical/picking checks.
