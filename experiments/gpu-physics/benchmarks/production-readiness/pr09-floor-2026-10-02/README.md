# PR09 inventory and missing-floor allocation milestone

Combined Village omitted nearby GPU ground and buildings because the shared
renderer retained only 65,536 debug shapes. CPU and GPU each flatten a 52,500-child
compound plus its parent, requiring at least 105,002 retained slots. The CPU
registered first; later GPU children received no slot and were skipped before
culling/draw submission. Single CPU, ordinary GPU and native GPU views retained
visible Village geometry. The other four reviewed floor representations did not
show this omission. Village was selected from actual comparisons, not assumed to
be the entire affected sample set.

The local renderer generator now uses existing stable-address chunk storage for
debug metadata, grows when its free list empties, and stores an explicit slot
index for destruction. Compound child maps, transforms, shape/body identities,
material inheritance and selection highlighting keep their existing semantics.
Reset/destroy/reuse and renderer release/reinitialization are covered by the
actual generated adapter regression. The common allocation path also covers
primitive/compound allocations that cross the same retained-metadata boundary;
this is not a hard-coded Village or doubled combined-view reservation.
Opaque per-frame upload limits remain separate. Box3D, WASM, Rust/WGSL, scene
geometry/assets, physics defaults, body creation order and picking APIs are
unchanged. Actual mouse/raycast/UI qualification remains PR10/PR11.

## Inventory and scope

The [full inventory](inventory.md) and [machine-readable record](inventory.json)
cover **165 active native CPU registrations**, their shared ordinary/native GPU
scene counterparts, **174 registered browser CPU entries**, and **20 real CPU
oracle/Rust demo fixtures**. They include geometry/manifold/tree/replay tools and
samples outside the short benchmark list. Disabled `#if0` registrations are
excluded, with their active browser ports recorded as gaps. Thirteen browser
entries have no active native counterpart; Replay has an explicit native
recording/player API exclusion. Factory variants are family mappings, not
claims of identical setup. Source hashes and registration lines are retained.

Only five native scenes received a **floor-visibility review** here. All remaining
native entries, every browser/oracle entry, broader behavior, interactions and
performance are explicitly **UNREVIEWED**. Even these five entries need the rest
of the full PR09 review. **PR09 stays unchecked.** The full production roadmap
remains paused; this user-authorized milestone stops after commit/push.

## Calculation-first evidence

The focused harness includes the actual generated adapter and executes its
allocation, compound-child registration, index lookup, identity/highlight and
teardown functions. It does not substitute a rewritten allocator formula.
With identical reservation 65,536 and 105,004 children:

| Implementation | Registered children | First missing child | Metadata slots |
| --- | ---: | ---: | ---: |
| Retained original | 65,535 | 65,535 | 65,536 |
| Repair | 105,004 | none | 131,072 |

The parent consumes one slot. The original's first differing branch is
`AllocDebugShape` finding an empty free list and returning -1; registration then
skips that child. The repair grows only the pointer directory/chunks and keeps
the parent's callback address valid throughout allocation. Both versions ran
six identical-input cases: large/default reservation, tiny reservations, chunk
boundaries, complete child maps, generation-bearing body/shape identity,
selection highlighting, reset followed by stale destroy, double-free/reuse and
release/reinitialization. Original omissions are retained as demonstrated
results; the repaired checks require every child. All 12 assertions/contract
cases passed. See [focused logs](raw/focused-before/0.log),
[repair logs](raw/focused-after/0.log) and [build receipt](raw/build-receipt.json).

A reusable current-adapter regression is available without a GPU process:

```sh
python3 experiments/gpu-physics/scripts/test-sokol-debug-pool.py \
  --build-dir experiments/gpu-physics/native-samples/build-cpu
```

It requires an already built CPU viewer's compile/link metadata. This convenience
runner uses the retained harness recipe; it was not separately executed after
the frozen two-build/twelve-case budget closed.

Five viewers compiled only their regenerated adapter and relinked a copied gfx
archive against unchanged original objects/archives: CPU, ordinary/native GPU,
and ordinary/native combined. No old object, executable or archive was modified.
Actual C consumer receipts, compiled-unit hashes, commands and unchanged linked
inputs are retained; the original Rust provider remains the previously qualified
distance-clamp archive. Invocation HEAD is not build provenance. These cached
consumer builds are not PR06 clean release qualification.

## Captures and review

[CPU-first comparison grid](compare.html),
[shared repository grid](../../../compare/index.html),
[early before](review-before-early.png), [late before](review-before-late.png),
[early after](review-after-early.png), [late after](review-after-late.png).
The shared grid adds five rows, keeps real Box3D CPU first, then latest repaired
GPU columns before retained older columns. All existing clips/datasets remain.
The portable clips are tracked under `recordings/snapshots/`; no ignored binary
or live GPU is required to view or validate them.

| Scene | Reviewed result |
| --- | --- |
| Compound/Village | Both combined viewers reproduce the old omission and show repaired nearby floor/buildings. GPU visible instances rise from 143 to 1,551, matching the CPU visible count; body/joint/draw-shape counts are unchanged. |
| Compound/Tile Floor | Tiled compound floor remains visible across all views. |
| Compound/Mesh Tile | Four compound mesh platforms remain visible across all views. |
| Stacking/Single Box | Static floor/body remain visible. CPU grid decoration is absent in the combined view before and after: separate PR09 rendering finding. Late body colors differ between CPU/GPU; sleep/behavior qualification remains PR04, no new physical failure is inferred from color alone. |
| Mesh/Grid | Triangle mesh ground and cylinder remain visible across all views. |

All 45 planned captures retain 60 completed/rendered physics steps with finite
bodies, no GPU capacity loss/failure, unchanged scene population and sleep
setting, zero warmup, dt1/60/four substeps and upstream Release defaults/assets.
No Village drop or scripted interaction was introduced. Two stages per view
were inspected (100 sampled images including repeated CPU references); they are
approximate callback checkpoints, not synchronized exact physics timestamps or
all-frame pixel/trajectory qualification. Full videos retain startup/shutdown.
Combined purple GPU coloring is intentional; projection differs from standalone.

Actual compute: RTX 4070 SUPER, NVIDIA Vulkan, driver 610.57.04, Manjaro Linux.
Graphics: Mesa llvmpipe/Xvfb, 1280×720, clips encoded at 30 FPS. These are diagnostic
visual/health captures. Neither encoding FPS nor incidental profile/health clocks
are rendering FPS, headline physics performance or desktop release qualification.

## Frozen budget and interruption

[Protocol](protocol.json) was written before any build/check/engine execution.
Consumption: five/five viewer builds, two/two focused builds, twelve/twelve focused
cases, 45/45 captures of 60 steps, zero Rust builds/retries/performance campaigns.
Build watchdog 180s, scene watchdog 300s. All prior experiments stay closed.

The after driver/session54585 terminated143 after ordinary combined Grid had
completed its 60 healthy steps and printed `samples: 60 frames, 0 sokol errors`.
Its original app/ffmpeg exit statuses are unavailable. The engine was not rerun.
Existing orphan ffmpeg was finalized with SIGINT; the full video including its
long black tail is retained. Review uses the 11.258-second window derived from
health/launch file times; comparison grids use an explicit media time fragment, while the original full
video and raw tail are neither trimmed nor overwritten.
[Original interrupted receipt](raw/after-driver-interrupted-receipt.json),
[recovery record](raw/after-original-terminal-receipt.json) and
[remaining-cell protocol](raw/recovery-protocol.json) preserve this limitation.
Only the five never-launched native combined cells continued, with the same
frozen builds/settings/budget. They completed normally. Thus 44 captures have
observed zero process exits; one has complete engine health/video with unavailable
exit observations, not a fabricated exit-zero pass. The combined evidence receipt
labels it `recovered-health-and-video`.

Preparatory path/source-provenance/hash errors occurred before engine launches
and consumed no build/check/capture retries. Their records are retained. The
recorder now accepts a finite `steps_per_capture`, preserving its existing
300-step default. Pure artifact packaging briefly observed a running receipt;
no engine was relaunched. Local preexisting roadmap edits were preserved and
archived before editing; initial fetch found no remote divergence.

## Offline verification and handoff

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr09-floor-2026-10-02/validate.py
```

This verifies indexed evidence/clip hashes, all catalog counts and named review
gaps, actual generated-source build receipts, original/repaired calculation
results, every health step, unchanged before/after population counts and both
combined Village render-count changes. It distinguishes the recovered transport
from observed normal exits. It does not require original executables, caches,
absolute host paths, a display or GPU.

Remaining PR09 work: review the 160 other active native scenes, dispose of native
availability gaps and review remaining behavior/interactions/performance of the
five sampled scenes. Investigate the separate Single Box combined grid decoration;
route widgets/query findings to PR10/PR11 and actual physical defects to PR04.
Do not resume these automatically: this milestone explicitly ends after its
verified commit/push. Historical solver failures and their budgets remain intact.
