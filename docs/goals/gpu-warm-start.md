# GPU warm-start controls goal

Read `/home/firtoz/work/2026/box3d-wasm/docs/goals/gpu-warm-start.md` at the start of a continuation or after compaction. Maintain the full objective below.

Updated 2026-09-29. Implementation and verification are complete. This file is included in the delivery commit; resolve its hash with `git log -1 --format=%h -- docs/goals/gpu-warm-start.md`. No push is authorized.

## Objective and constraints

Implement working GPU warm-start controls end-to-end. Make b3World_EnableWarmStarting change actual GPU solver behavior and b3World_IsWarmStartingEnabled report the current per-world setting. Match upstream Box3D defaults and semantics for both contacts and joints, including switching off and back on during simulation. Check combined-viewer routing so the control reaches both engines.

Use small, deterministic contact and joint fixtures. Verify default state and setter/getter round trips, independent settings across worlds, disabling stops reuse of previous impulses, re-enabling follows upstream cache semantics, and default-enabled behavior passes relevant existing regressions.

Record before/after step timings on these fixtures without promising a speedup. Update relevant documentation and the API audit. Record affected-scene snapshots as required by AGENTS.md before committing. Keep scope limited to warm-start controls. Do not investigate Rain, redesign the solver, or implement speculative-contact controls. If blocked, document the exact blocker and smallest reproducer rather than expanding scope.

Done means the control changes real solver behavior, focused tests and relevant regressions pass, evidence and docs are updated, and the work is committed. Do not push.

## Acceptance matrix

| Requirement | Authoritative evidence | Result |
| --- | --- | --- |
| Per-world default, setter/getter, independence and recreation | `c_abi/api_settings_test.cpp`, ordinary/native GPU-only and combined binaries | Passed |
| Contacts: off/on, latest cache, all impulse channels and manifold children | Seeded GPU cache test plus 240-step sphere-ground CPU comparisons | Passed |
| Joints: off/on and cache semantics | All eight joint types seeded directly; 240-step spherical-joint CPU comparison | Passed |
| Substeps and sleeping behavior | One/four substep CPU cases, sleeping cache test, existing revolute substep regression | Passed |
| Combined viewer routing and visible control | Independent CPU/GPU state assertions, surviving-world test, UI generator regression | Passed |
| Default-enabled regressions | Five ordinary and four native cached Rust tests | Passed |
| Timing | Five alternating paired processes, separate prewarms, 60 warmup + 180 timed steps | Recorded; no demonstrated speedup |
| Documentation and API audit | Source/build audits in both paths, 415 required symbols, no additional missing symbols; docs/README updates | Passed |
| Affected-scene snapshots | 19 CPU + 19 GPU clips, each 180 frames/30 fps; checked metadata and midpoint sheets | Passed |
| Delivery | Commit adding this scratchpad and implementation; verify current Git status | Commit without push |

## Implementation

The setting is stored per world and defaults true. Public wrappers live in the core C shim, with explicit combined-viewer dual routing. The trailing SimParams padding word now carries disable_warm_starting without changing buffer size. Contact preparation clears active normal, tangent, twist and rolling caches after island wake, including recycled/child manifolds. A disabled-only joint preparation dispatch clears all supported joint impulse channels once per world step, preserving revolute axis caches. Substep warm-start waves remain enabled and sleeping caches remain intact. Ordinary and native cached paths share these semantics. Diagnostic traces include the host setting, GPU parameter and full replay key.

The viewer checkbox is restored. A newly exposed existing comparison lifetime bug used to clear all world maps when any world was destroyed; destruction now uses the existing per-world unmap function. The surviving-world control regression passes.

## Evidence and reproducibility

Paths below are relative to `experiments/gpu-physics`:

- `benchmarks/2026-09-29-warm-start-verification.json`: source hashes, API/audit results, test counts, clip hashes and log hashes.
- `benchmarks/2026-09-29-warm-start-timings.json`: all 12 process results (two prewarms plus five pairs), binary hashes and protocol.
- `scripts/check-warm-start.sh`: ordinary public C checks; set `GPU_PHYSICS_SAMPLES_NATIVE_CACHE=1` for the cached path.
- `cargo test --release --lib warm_start -- --test-threads=1`: four tests pass on both ordinary and pinned native builds. Ordinary `recycled_contact_separation_matches_fresh_at_rest` also passes. Six `scripts/test-native-portability.py` tests pass. `cargo check --lib --features replay-diagnostics` passes.
- `scripts/bench-warm-start.py`: identical ordinary core-shim link configuration in both arms. Archived baseline is 6546eb8. The initial mixed-linkage batch is preserved under `artifacts/warm-start/mixed-linkage-runs` and its receipt is retained; it was superseded to eliminate the link-configuration difference, not pooled or selectively sampled.
- `scripts/plot-warm-start.py` produces `artifacts/warm-start/charts/timings.png` and `.svg`; the chart was visually reviewed.
- `recordings/snapshots/000-box3d-cpu` and `recordings/snapshots/2026-09-29-warm-start`: generated with the required recording scripts, FRAMES=180, GPU METRIC_RUNS=1. `compare/index.html` pins CPU first and displays newest GPU next. All 38 videos have 180 frames at 30 fps (six seconds playback). Midpoint contact sheets are in `artifacts/warm-start/snapshots` and were reviewed.

| Fixture | Substeps | Before mean ms | After mean ms | Change |
| --- | ---: | ---: | ---: | ---: |
| Sphere-ground | 1 | 1.126112 | 1.104826 | -1.89% |
| Sphere-ground | 4 | 1.292773 | 1.303361 | +0.82% |
| Spherical joint | 1 | 1.251761 | 1.271344 | +1.56% |
| Spherical joint | 4 | 1.833267 | 1.827093 | -0.34% |

These are default-enabled completed-step-plus-position-read timings on RTX 4070 SUPER Vulkan. Mixed small changes and individual-run variation do not demonstrate a speedup. Snapshot metrics are separate, not the controlled performance experiment.

## Scope boundary and recovery

A distance-joint prototype differed on its first step, before any toggle/cache reuse: GPU y=-1.00004232, CPU y=-1.00028491. The exact fixture/log are preserved under `artifacts/warm-start/distance-reference-mismatch`, with hashes in the verification receipt. No distance solver fix or threshold relaxation was attempted. A spherical joint supplies the bounded independent CPU reference case; all eight supported joint cache representations, including distance, remain directly tested.

No Rain solver investigation, speculative-control work, solver redesign, Box3D submodule edit or WASM change occurred. The earlier broad solver scratchpad is historical context and is not this objective. The API audit remains incomplete for unrelated work: 36 stubs, 8 additional placeholders, 11 CPU-only wrappers. The two warm-start APIs are absent from those gaps.

All build/test/timing/snapshot jobs are terminal, with success for the final runs. Raw logs are copied into `artifacts/warm-start`. Final delivery check: verify receipt source hashes and `git diff --check`, commit all task files, confirm clean worktree and no push, then mark the active goal complete. No additional solver work remains within this goal.
