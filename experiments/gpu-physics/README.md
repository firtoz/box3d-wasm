# gpu-physics

Research reports, captures, benchmark JSON, and recordings are local ignored outputs under `experiments/gpu-physics/artifacts/` and `recordings/`. Historical evidence links require that local archive; fresh clones retain source, build manifests, test fixtures, and reproduction scripts. Regenerate the comparison index with `bun scripts/refresh-compare.ts .` from the experiment directory after recording clips. Copy the local archive to external storage before removing a checkout if it is needed for publication.

A second physics engine: Rust + WGSL compute, with a Box3D-shaped C API. It lives on `feat/gpu` and does **not** patch `box3d/` or the WASM package.

Native sample loading: `bun run samples:gpu` and `bun run samples:both` show a preparation checklist while a worker uploads GPU buffers and compiles the scene collision pipeline. The panel reports completed stages, the current shader and elapsed time; it does not estimate driver compile percentage. Both physics worlds remain unstepped during loading. Pipeline caches persist under `${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines`; set `GPU_PHYSICS_PIPELINE_CACHE=0` to disable caching. Scene construction and graphics initialization still run on the main thread before this panel; closing during an active driver compile waits for safe worker teardown. Loading frames do not consume physics or benchmark frame counts; separate `gpu-loading` log lines record their duration and count.

Body Type contact correction: mixed boxes and generic hulls now use the existing face-clipping manifold, with implicit box topology, instead of the one-point fallback. `scripts/check-body-dynamics-reference.sh` checks the upstream Body Type setup for 300 steps and its first-impact four-point contacts. The correctness gate also requires this script. It gates Gyroscopic Torque and a custom-inertia variant for 600 steps: position/quaternion chord ≤ 1e-5 and angular-velocity difference ≤ 1e-4 against Box3D C. Box inertia is computed directly at unit density and then scaled, avoiding an inverse-inertia round trip. Native Vulkan rotational integration uses CPU operation order, refined sqrt/reciprocal operations, and SPIR-V NoContraction restricted to `gyro_*` functions. Contact/joint arithmetic is unchanged. Other backends retain the WGSL path; cross-device bitwise determinism is not claimed. The CPU integration method and its numerical energy loss are retained; no damping or CPU pose substitution is added. GPU-only native `SetMassData` now reaches the implementation instead of its old stub. Local evidence: `artifacts/gyro-substep/`.

Current qualified milestone (2026-09-13): the final implementation passes **55/0/0 correctness cases, 222 library tests, and 12 native cases**, including long-run Gear Lift and ordinary Village. Five matched AC trials per engine and scene meet the aggregate whole-frame target: mixed-stacks GPU/CPU p50 **0.489/0.661 ms**, Dominoes **1.885/3.687 ms**, with lower GPU p95 on both. `cpu_win_validated=true` is scoped to NVIDIA physics + AMD display-local rendering, equal one-step display delay, and these two fixtures. See [final evidence and limitations](artifacts/gear-clipped-support-final/README.md). The investigation entries below are historical; their earlier failures and qualification flags are preserved.

Earlier reliability finding: the captured Gear Lift hull–capsule pair exposed incorrect fresh contact geometry: CPU +1.75 mm separation versus GPU −2.70 mm with a 24° normal error. The shader now computes segment/hull closest witnesses and clips face-aligned capsule contacts in hull-local coordinates. Seven geometric controls pass; the captured separation now differs by 0.24 micrometres. In 1,202-step runs, gear-chain-only hinge excess falls from 54.7° to 0.044°, and original Gear Lift from roughly 35° to 0.187°, without NaNs/capacity loss. Core intersections retain the prior penetration path. Full Gear Lift still fails the existing debris trajectory envelope (step 126, body 152), so full native compatibility is not established. Evidence and limits: [hull–capsule correction](artifacts/hull-capsule-witness-fix/README.md). The contact-list lifetime repair remains; its targeted tests and the subsequent ordinary milestone gate pass. The native viewer is also rebuilt: its seven geometry controls and exact mixed-stacks checkpoint pass with zero Vulkan validation errors. Five GPU-before/after frame pairs keep mixed p50 essentially unchanged and improve Dominoes p50, but worsen Dominoes p95 (5.37 → 10.99 ms). The initial short headless window was insufficient; a matched 600-step follow-up still shows nearly unchanged device p95 (2.787 → 2.807 ms), with worse host encode/completed-step tails. Nsight finds fence/present waits and outside-Vulkan time, but does not establish one root cause (`artifacts/hull-capsule-tail-trace/`). The full ordinary correctness result is recorded below; no CPU performance qualification is claimed; `cpu_win_validated` stays false. A [fresh ordinary 1,202-step Gear Lift run](artifacts/gear-current-long/README.md) confirms finite, bounded mechanism motion and comparable CPU/GPU speed bounds. The step-126 debris envelope still fails, but body 152 leaves the stairwell depth at step85 and later settles on the main floor; this is not demonstrated tunnelling or an explosion. The original envelope remains unchanged.

The [latest ordinary milestone gate](artifacts/milestone-village-repaired/README.md) passes **53/0/0**, including **220/220 library tests**, the **12/12 native matrix**, and three 3,600-step runs. The malformed-component fixture and Village gate are repaired; the old 51/2 artifact remains historical evidence. Village now exercises a 600-step sphere drop on its full geometry and matches CPU within 1e-5. Native scene checks remain bounded: Gear Lift's longer debris divergence is still open. [Large-component chain validation](artifacts/large-component-chain-validation/README.md) remains a provisional performance optimization with mixed tail evidence. `cpu_win_validated` stays false.


The [component impulse-store optimization](artifacts/component-impulse-stores/README.md) is provisionally retained: three focused native component/reference tests pass, and a Vulkan-validated 600-step Dominoes dump is byte-identical to the frozen baseline. Five alternating GPU pairs improve mixed-stacks frame p50 0.8160 → 0.7999 ms and Dominoes 2.1306 → 1.9961 ms. Mixed p95 improves; Dominoes median trial p95 is essentially flat, with one worse tail run. The 53/0/0 ordinary milestone predates this change. No new CPU qualification is claimed. A subsequent [velocity-only body-store screen](artifacts/component-velocity-stores/README.md) passed focused physics tests but worsened frame tails and Dominoes median time; it was reverted. The [root-only color-offset screen](artifacts/component-root-offsets/README.md) also passed focused checks but showed negligible mixed-stacks median benefit and slower Dominoes median; it was reverted. A [current 8/16/32/64-thread sweep](artifacts/impulse-store-workgroups/README.md) still supports keeping16: 8 has similar median with worse tails, while32/64 slow the median. A [component-specific body-loader test](artifacts/component-body-load/README.md) passed after correcting an inertia-offset bug but regressed frame performance, so it was reverted.

[Fresh five-pair CPU comparisons](artifacts/impulse-store-matched-cpu/README.md) put Dominoes GPU/CPU p50 at1.9166/2.8389ms and p95 at8.0621/11.1437ms: aggregate thresholds pass, with one slightly worse GPU tail pair. Mixed stacks remains GPU-slower (0.8077/0.7803ms p50;2.7237/2.4576ms p95), requiring about22.7% lower GPU median to reach the0.62424ms target. CPU worker counts8/16 reuse the recorded calibration. Overall cpu_win_validated remainsfalse.

A [current CPU sampling capture](artifacts/encoding-cpu-profile/README.md) found 21 Vulkan command-buffer begin/end pairs per frame; incomplete stacks and tracing overhead limit attribution. An [isolated native reset replay](artifacts/native-reset-replay/README.md) preserves the 600-step state with zero Vulkan errors but trades a small mixed median gain for worse tails and slower Dominoes median. It was reverted. The submission split is 14 physics / 7 rendering command-buffer pairs per frame. A [five-pair matrix replay retest](artifacts/current-pair-replay/README.md) shows negligible median gain and worse tails, so that option stays disabled.

Box3D C is the visual/timing reference. Same-adapter GPU dumps should match. Lock-step poses vs Box3D at `1e-5` are not a ship gate.

Architecture notes: [`docs/gpu-physics.md`](../../docs/gpu-physics.md). Native command-reuse feasibility is isolated in `examples/command_reuse.rs`; its synthetic results and ownership constraints are documented in `artifacts/command-reuse/README.md`. It does not change engine submission or establish a physics speedup. The original radix-cache prototype is archived in `artifacts/native-command-cache/`; its debug validation claim was corrected after finding wgpu filters a shader-layout error. Offline reproduction confirms the compiler defect. Five matched cache-off/on native-frame pairs per scene did not justify enabling the one-pass cache: mixed stacks regressed and Dominoes p95 worsened. The complete-broadphase follow-up (`artifacts/native-broadphase-cache/`) improves mixed-stacks p50 about 8% and p95 about 15% in five longer-window GPU/GPU pairs; the compiler repair now passes isolated GPU tests (`artifacts/spirv-layout-repair/`), and the in-process Rust port passes focused offline/GPU checks (`compiler/spirv-layout`, `artifacts/spirv-layout-rust/`); focused cache lifecycle checks now pass; repaired-compiler mixed-stacks frame pairs improve p50 8.6% but worsen p95 1.3%, so the cache remains experimental; no CPU win is claimed. The contact-allocation/narrowphase follow-up (`artifacts/native-contact-cache/`) preserves tested physics and halves that device stage, but adds only a noisy 3.6% mixed-stacks whole-frame p50 improvement over broadphase caching. Graph replay (`artifacts/native-graph-cache/`) also reduces scoped encoding/device costs but fails to improve both frame percentiles on either named scene. Fresh five-trial matched CPU comparisons of the combined candidate (`artifacts/native-matched-current/`) meet the Dominoes frame threshold against the selected eight-worker CPU, but mixed stacks still needs roughly 25% lower GPU frame p50. Overall CPU-win validation remains false. No cache is enabled in ordinary builds. A bounded GPU pair-bitset follow-up (`artifacts/pair-bitset/`) halves mixed-stacks broadphase device cost and improves five-pair whole-frame p50 8.4% (1.1685 to 1.0702 ms), with p95 essentially unchanged. Exact scene and mutation/capacity/mesh checks pass after correcting stale-contact retirement. It is now available through the opt-in native build. Replaying its six dispatches (`artifacts/pair-bitset-cache/`) improves five-pair frame p50 another 5.0% against its own control, but worsens p95 1.8% and does not improve short-profile encoding time. Keep it optional. Native tail replay (`artifacts/native-tail-cache/`) reduces mixed encoding 0.462 to 0.190 ms and passes seven focused regressions plus exact scene checks after fixing query-buffer initialization. Fresh five-trial frames are mixed GPU/CPU 0.920/0.867 ms p50 (fail) and Dominoes 2.586/5.574 ms (median criterion passes); overall CPU-win validation stays false. Demand-driven CPU pose staging (`artifacts/demand-pose-staging/`) removes the unused per-step pose copy from the direct viewer when `GPU_PHYSICS_DEMAND_POSES=1`. This isolated option is retained for testing; automatic snapshots remain the default and C/Sokol behavior is unchanged. Eight focused checks and a native Vulkan smoke pass on the cached prototype. Five frame pairs per scene show mixed p50 0.922 to 0.893 ms (p95 slightly worse) and essentially unchanged Dominoes p50 2.546 to 2.535 ms. The ordinary release build and all four new regression tests pass; no new CPU win is claimed. A combined physics/draw submission screen (`artifacts/combined-frame-submit/`) passes current-step consumer and native Vulkan checks but improves mixed frame p50 only 2.9% in three pairs, with essentially unchanged aggregate p95 and worse tails in two pairs. The extra runtime path was not retained. Draw-stage attribution (`artifacts/draw-stage-profile/`) finds about 0.25 ms in GPU surface acquisition, with much smaller preparation/recording/submit costs. Early acquisition (`artifacts/early-surface-acquire/`) improves p95 14.3% but regresses p50 4.3% in three mixed pairs; it was not retained. A current-frame GPU-only render snapshot (`artifacts/gpu-render-snapshot/`) passes current-state/pixel/shadow checks and native Vulkan smoke, but regresses mixed p50 3.1% across three pairs despite improving p95 8.1%. It was rejected. A follow-up reusing native buffer declarations (`artifacts/native-access-reuse/`) preserves exact scene results and passes Vulkan checks, but changes device time less than 1% and does not improve encoding/completed-step time; it was also rejected. The subgroup component-prefix scan also passed 43 exact integer/capacity cases with no Vulkan errors, but saved only 0.352 microseconds at 602 bodies; it was archived without a runtime change (`artifacts/subgroup-component-scan/`). A separate eight-body invocation-private cache preserves the three tested scene dumps but worsens mixed solve/device cost in its initial screen; it was also archived (`artifacts/small-component-private-cache/`). Its two-scalar-state follow-up also matches those scenes but worsens mixed solve p50 about 18%; it remains archived (`artifacts/two-body-scalar-cache/`). An exact-isotropic inertia shortcut saves only about 4 microseconds of solve time in its initial screen and is also archived (`artifacts/isotropic-inertia/`); it does not establish a latency win. Driver diagnostics now show 168 registers per thread for both component kernels (`artifacts/solver-executable-stats/`). Removing three redundant finite-loop counters does not reduce that count; this compiler prototype is archived without a timing claim (`artifacts/bounded-manifold-loops/`). SPIR-V aggregate scalarization also leaves register counts unchanged and is archived without a performance claim (`artifacts/solver-scalarization/`). Splitting small-component phases increases solver dispatches from 2 to 22 and regresses mixed solve p50 0.223 to 0.327 ms; that candidate is archived too (`artifacts/small-component-phases/`). A single-slot component parameter upload reduces host encoding in two paired profiles and passes transition checks, but does not improve the three-pair mixed frame screen; it remains archived (`artifacts/component-param-upload/`).

Hull–triangle contact clipping now sends every clipped vertex through bounded four-point reduction; truncating the polygon first could discard its only penetrating support witness. The saved Gear Lift step-800 pose is covered by `./scripts/check-hull-mesh-rest-reference.sh`. The captured replay matches the CPU first-step position; the expanded contact regression, 26 manifold tests and 1,200-step native Gear Lift run pass (`artifacts/gear-final-regression/`). Peak penetration is 3.53 cm with a one-frame maximum deep streak, below the matched CPU bounds. The fresh full gate after this fix passes 55/0/0, including all 12 native matrix cases, with 222/222 library tests (`artifacts/gear-clipped-support-final/`). Final-build performance qualification remains pending, and `cpu_win_validated` is false.

Village uses the ordinary shared-geometry compound import path (up to 65,535 children plus the public parent); `GPU_PHYSICS_AB=large-compound-import` is no longer required. World shape capacity and checked allocation limits still apply. Run `./scripts/village-probe.sh <fresh-output-dir>` for the matched native sphere-drop check and `python3 scripts/scene-health-probe.py <fresh-output-dir>` for the Village→Bounce House transition. These correctness probes do not certify performance or all Village interactions. The earlier 240-step drop check missed a later mover-query mismatch: an invented deep hull plane kept the character applying impulses to the sphere. Hull mover queries now follow Box3D: no plane when the capsule axis intersects the hull, but a plane for positive separation below linear slop. A fresh 600-step native comparison matches position/rotation/velocities within `1e-5`; both spheres finish moving at step 305 and remain at y=2.19756746 for the final 120 steps. The ordinary packed geometry heap is 12,257,696 bytes. Evidence: `artifacts/village-hull-mover/result.json`. The earlier 8-frame Village → 20-step Bounce House transition passed (`artifacts/v19-scene-switch-ordinary/`); it was not repeated for this query-only change. Append-only shape creation now extends cached body extents instead of rescanning all previous children. The same Village startup/short-transition run fell from 104.9 s to 10.8 s in a single battery-mode comparison (`artifacts/v19-village-startup/result.json`), with unchanged CPU trajectory errors. Frame costs remain high and `cpu_win_validated` is false.

The v20 host CCD change builds ordered shape/target lists once per harvest instead of scanning the whole scene for each fast body. Three alternating AC-powered Dominoes runs reduced median frame p50 from 16.31 to 14.63 ms (`artifacts/v20-ccd-index/result.json`). This compares GPU builds; the separate CPU baseline remains faster at 9.03 ms. Seven focused Rust CCD tests and 31 C API CCD cases pass, including mesh/height-field impacts, rotating shapes, sensors and callback vetoes; GPU and combined viewers are rebuilt. A cold terrain-shader compile exceeded the first test's 90-second timeout; debugger inspection identified pipeline compilation, and subsequent completed runs passed. The full gate was not rerun for this slice. Completion waits inside picking remain a priority, and `cpu_win_validated` stays false.

The v22 regular color solver keeps bodies in lane-local variables instead of shared-memory staging; its solve p50 improved by 2–4% in three paired GPU trials. All **181 release library tests** pass. Full mirror reads also publish completed GPU clocks, fixing a HUD cache that could remain at step zero after picking consumed the pose snapshot.

**Native timing correction:** Sokol interprets descriptor `swap_interval=0` as default interval 1. Earlier artifacts hard-coded zero without verifying it; their “unpaced” label only proved that the application limiter was off. The benchmark now explicitly sets the current GLX drawable to interval zero and queries it; other platforms report unknown (-1). `sokol-timeline.sh` rejects unpaced certification without verified zero. Five corrected native Dominoes trials (warmup 30, timed 120, no sleep, four workers) give median frame p50 **6.17 ms CPU vs 9.98 ms GPU**, with GPU pick around **6.94 ms**. See `artifacts/v22-unpaced/result.json` and `artifacts/v22-solver-local/result.json`. Neither full native compatibility nor the CPU-win goal is achieved.

The v24 default schedule keeps wide dynamic colors parallel, processes the remaining dynamic colors in one workgroup, then dispatches static colors 20–22 in parallel. It preserves overflow-first and ascending-color order within every existing TGS phase. A previous-step GPU hint selects the split; an underestimated color is fully processed in batches, never dropped. Scene uploads reset the hint. The existing status copy grows from 48 to 56 bytes, without another dispatch, copy command or wait. `GPU_PHYSICS_COLOR_PREFIX=23` restores the full reference schedule; `auto` selects the default. Fused TGS remains off.

Validation: **182/182 library tests** pass, including wide colors, the existing small-world shortcut, overflow, joints and growth. Native Dominoes at step 121 has identical positions, rotations and velocities for all 5,430 dynamic bodies versus the full schedule. In five matched trials under variable desktop load, frame p50 medians were **8.85 ms CPU / 13.53 ms reference GPU / 12.80 ms adaptive GPU**; GPU physics-submit p50 fell from **3.07 to 1.76 ms**. This is progress, not a CPU win. Evidence: `artifacts/v24-final/result.json`. The remaining blocking-pick cost requires accounting for GPU graphics/presentation outside the physics timestamps; do not infer its cause solely from CPU render timings.

The native viewers now include `GPU Bench / Mixed Stacks 600`, compiled from the same experiment-owned C++ fixture in CPU, GPU and combined modes. It matches `create_mixed_stacks(world, 600)`: two overlapping supports and 600 unit boxes in two layers. Run with `--sample-name "Mixed Stacks 600"`; use the ordinary Sokol timeline flags. In v30, all 600 dynamic bodies remained stacked across 300 health-scanned steps; maximum CPU/GPU position difference was 0.0000573 m. Five matched no-sleep trials remain CPU-faster; see `artifacts/v30-native-mixed-stacks/result.json`. Health scans are separate from timed performance runs.

Native performance qualification uses `scripts/run-native-bench.sh` to select X11/PRIME and Immediate presentation consistently; the interactive launcher keeps normal display selection. Always verify the logged start/end pixel dimensions match across compared runs. An unchanged-build repeat exposed large frame variability and a Wayland/X11 mismatch in recent screens: [display-path evidence](artifacts/display-path-baseline/README.md).

The current ordinary Village support check passes: full geometry, 600-step CPU/GPU interaction agreement within 1e-5, and a healthy Village → Bounce House switch, with no GPU_PHYSICS overrides or allocation failure. This is support evidence, not frame-performance certification. See [Village results](artifacts/village-current-support/README.md).

A [broadphase cost screen](artifacts/broadphase-reuse-cost/README.md) localized mixed-stacks matrix construction/clear to about .05–.06 ms. [Early bounds rejection](artifacts/matrix-early-overlap/README.md) reduced physics device time about .024 ms and passed focused mutation/capacity/mesh validation, but five whole-frame pairs did not improve median or tail performance; the shader candidate was reverted. The test-only pipeline helper was repaired after the startup-cache refactor.

A [bounded presentation-worker screen](artifacts/present-overlap/README.md) overlapped next-frame encoding with present calls, but increased presentation-return latency on both scenes and worsened tails. It was reverted; producer enqueue timing was not treated as completion latency.

Small components now validate contact chains once before their substeps, retaining checks on all other solver paths. The [focused validation and paired frame screen](artifacts/component-chain-validation/README.md) favors mixed stacks modestly; Dominoes is near unchanged with a slightly higher median p95. This is retained provisionally, not CPU-win qualification.

A [large-component workgroup sweep](artifacts/large-component-workgroups/README.md) covered64/128/256/512/1024 threads with exact Dominoes checkpoints.256 slightly reduced solve time but worsened viewer p95;512/1024 were slower. The64-thread path remains, and the experiment is reverted. A [render bind-group reuse screen](artifacts/render-binding-reuse/README.md) also remains reverted: mixed-stacks timings varied heavily, and Dominoes had essentially unchanged median frame time with worse tails.

Profiler setup and capture workflow: [Nsight notes](artifacts/nsight-systems/README.md). Individual-workload Systems captures and a pixel-verified unnamed Graphics replay are available. Named Graphics replays remain blank; hardware counters are now accessible after the user’s session capability grant. See [Graphics verification](artifacts/nsight-graphics-no-export/README.md).

The native launcher persists compiled pipelines under `${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines`. Use `GPU_PHYSICS_PIPELINE_CACHE=0` for cold-start controls or `GPU_PHYSICS_PIPELINE_CACHE_DIR` to override the directory. A full warm headless launch measured 1.73 seconds versus a 41.48-second cold run; subsequent mixed-stacks/Dominoes runs took 2.08–4.23 seconds with matching 120-step positions. This does not establish native frame speed. See [startup results](artifacts/pipeline-startup-cache/README.md).

The subsequent three-pair cached-startup frame screen remains below the goal: mixed stacks GPU/CPU median frame p50 0.9695/0.7364 ms; Dominoes 2.4483/2.5199 ms, with GPU p95 worse on both. CPU uses the prior eight-worker setting; see [current frame evidence](artifacts/cached-frame-screen/README.md).

## NVIDIA (this machine)

Hybrid Intel + RTX 4070. The app refuses non-NVIDIA adapters:

```bash
cd experiments/gpu-physics
export __NV_PRIME_RENDER_OFFLOAD=1
export __GLX_VENDOR_LIBRARY_NAME=nvidia
export VK_DRIVER_FILES=/usr/share/vulkan/icd.d/nvidia_icd.json
```

## Run

**Rust window** (Vulkan only):

```bash
cargo run --release -- --scene box-stack
```

- Esc quits. `[` / `]` cycle scenes. `R` restarts.
- `--no-sleep` keeps physics active in the Rust window and `--native-timeline`, including the real CPU reference. Timeline JSON records `sleep`. Rebuild `oracle/build-viewer` when using the CPU reference; its bridge now exposes the sleeping toggle. Earlier Rust native runs ignored this CLI option (headless and Sokol were separate paths).
- `--mp4` writes a 30 fps clip (needs `ffmpeg`). `--frames N` is world steps (default 300).

Scenes: `single-box`, `box-stack`, `sphere-stack`, `capsule-stack`, `revolute`, `weld`, `anchored-mechanisms`, `joint-chain`, `stack`, `pyramid`, `bounce`, `mixed`, `spinner`, `ramp`, `spheres`, `high-resistance`, `mixed-stacks`, `dominoes`.

`--bodies` is scene-specific:

| Scene | `--bodies` means |
|---|---|
| `spheres` | dynamic sphere count |
| `mixed-stacks` | dynamic box count (plus two hidden grounds) |
| `anchored-mechanisms` | independent pendulum count |
| `joint-chain` | connected revolute link count |
| `dominoes` | ring count (default 30) |

Passing `--bodies 256` on mixed-stacks creates 256 dynamics, not the 600-body demo default.

**Upstream sample viewer** (sokol GL window + Vulkan physics). Do not give wgpu a GL context here — Sokol already owns GL.

```bash
bun run samples:cpu    # Box3D C
bun run samples:gpu    # this crate behind box3d.h
bun run samples:both   # CPU left / GPU right, same step
```

`samples:both` starts in split view: CPU on the left, GPU on the right. The bottom-left checkbox restores the overlapping view. Both panes use one camera and one sample controller, so camera gestures, scene selection, keyboard controls, settings, pause (`p`) and single-step stay synchronized. Mesh resources are shared; poses and per-view instance streams are independent. Each pane renders the same submitted physics step; Sokol still uses CPU pose snapshots, with one graphics commit/present for the two panes.

In split view, click selects independently and **Ctrl + left-drag** grabs independently. Pane-relative cursor coordinates produce the same world ray for both engines; each engine keeps its own hit body, local anchor, hit depth, mass and motor joint. Moving over the divider does not wrap the drag. A colored plus cursor marks the corresponding location in the other pane, with a ring while pressed; it is hidden outside the window and over UI unless a gesture is captured. A miss on one engine leaves only that side ungrabbed. Releasing over the UI, losing focus, changing mode or destroying the world releases the comparison drag. Door's Ctrl-click impulse likewise uses each engine's own hit point. Other sample-specific controls retain the shared sample controller's existing behavior; this is not two isolated copies of the sample's event/query logic. The inspector remains GPU-led. Motor joints reapply their cached linear and angular impulses during the warm-start wave, then solve during the ordinary waves. `scripts/check-both-pointer.sh` checks 600 held steps against CPU motion with independent hit depths (position and quaternion components within 5e-4), as well as release, misses, pause and viewport mapping.

Run `./scripts/check-both-pointer.sh` for unequal-depth/one-sided-hit fixtures, pause/release/destruction, impulse lever arms, viewport rays, and ten sequential five-cube drags (center and four offset face points). Joint topology uses the sparse slot span, including deleted holes: using the live count silently omitted the second drag joint. The isolated fixture resets both worlds to the same prescribed setup above the floor between grabs, without transferring CPU poses or hit results to GPU. It checks every step with limits of 0.0005 m position, 0.0005 sign-independent quaternion chord and 0.005 m/s velocity; observed maxima were 0.0000243 m, 0.0000203 and 0.000165 m/s.

The same script also runs `artifacts/split-view/both-drag --ground`, retaining floor impacts and previous outcomes across all ten drags. SAT face-cache hits now retain their original separation baseline, matching Box3D; updating it every frame incorrectly preserved tilted contact normals under gradual rotation. Joint phases run before contact waves so contacts respond to the motor target. Together these reduced the observed worst position difference from 1.33 m to 0.0162 m. The ground-motion gate checks every-frame position/quaternion chord within 0.025, held position within 0.005 m, held quaternion chord within 0.01, held velocity within 0.1 m/s, and post-release velocity agreement within 0.02 m/s. Both engines must also settle below 0.1 m/s linear and 0.1 rad/s angular speed after each release.

`artifacts/split-view/both-drag --ground-strict` additionally retains the original 0.5 m/s instantaneous velocity check. That diagnostic still fails (latest peak 0.983 m/s). In the earlier 1.09 m/s baseline, at frame 2404 one cube impacts a frame earlier on CPU, with only 0.0062 m position difference; the GPU impact follows at frame 2405. This limitation is reported separately, not hidden by matching CPU poses or hit points. Set `BOTH_DRAG_TRACE=/absolute/path/trace.txt` and optionally `BOTH_DRAG_TRACE_BODY=0..4` to capture both engines’ per-step poses, velocities and contact manifolds for diagnosis. Full native compatibility and identical impact timing are not claimed. The trace also includes angular velocity for reproducing input states. `scripts/check-impact-replay.sh STATE [OUT] [STEPS]` replays a unit cube above the same floor from 13 floats (position, quaternion xyzw, linear velocity, angular velocity); it verifies identical initial states and reports motion differences. This is a cold-start diagnostic without joint/contact caches, not a replacement for the retained-history gate. Four contact-free captures from frames 842 and 2341, using each engine as the source, produced peak velocity differences from 0.00020 to 0.03091 m/s over 120 replay steps. The complete drag sequence still fails its strict check.

`scripts/check-drag-phases.sh [OUT]` builds a diagnostic copy of the CPU solver outside the submodule and runs the unchanged strict sequence with `GPU_PHYSICS_AB=phase-capture`. It exits nonzero while the strict gate fails. `DRAG_PHASE_RANGE=660:900` selects the recorded frame range. GPU and CPU records use body slots and submitted frame indices; phase 1 is prepared constraints, phases `2 + 5*substep` through `6 + 5*substep` are integrated velocity, warm-start, biased solve, position integration, and relaxation, and phase 22 is restitution completion. Records contain x/z velocity, angular-speed magnitude, and engine-specific flags (flags are not directly comparable). These intermediate snapshots currently cover the ordinary four-substep solver path. Normal stepping performs no diagnostic readbacks. The instrumented baseline reproduced the same end-to-end errors as the ordinary build; arithmetic prototypes that regressed held motion were reverted.

For a cached-state control, run `scripts/check-drag-state-replay.sh FRAME [OUT]` after building the native archives with `scripts/check-both-pointer.sh`. It builds the opt-in `replay-diagnostics` feature in a separate target directory, reproduces the original history up to FRAME, transfers native body states, motor targets/impulses and cached contact data, then stops after one step. Uploads are read back and verified; native SAT enum values are explicitly translated. This is scoped to the five unit cubes/floor/cursor fixture with matching geometry, tuning and independent floor contacts, not a general world snapshot format. Sleep timers, arbitrary constraint schedules and mesh caches are not restored. Normal viewer builds have no state-transfer export.

Earlier cached-state controls, measuring cube 3 only, at frames 663, 838 and 2404 have endpoint velocity differences of 1.71e-6, 4.20e-6 and 2.39e-5 m/s respectively; the selected impact occurs on the same step. Evidence is under `artifacts/drag-full-state/`. These controls isolate accumulated history from individual step behavior; they do not change the original strict gate or establish bitwise equivalence. The ten-drag strict failure remains unresolved.

The cached-state replay now compares all five bodies and fails above 1e-5 m position, 1e-5 quaternion chord, or 1e-4 m/s velocity for its single seeded step. `./scripts/check-drag-state-replay.sh 1735` currently fails: the old cube-3-only check missed a 0.0474 m/s difference in cube 1. At the second substep, CPU separation is zero while GPU rounds to +5.96e-8 m, selecting speculative bias instead of contact softness. A roundoff-band prototype reduced that step to 9.21e-6 m/s but introduced a 0.0279 m/s disagreement at step 234, so it was removed. No solver change from that experiment remains. Future changes must pass both seeded cases and the original retained-history strict gate; snapping small positive gaps to zero does not consistently reproduce CPU behavior. Evidence remains ignored under `artifacts/drag-step-sweep/` and `artifacts/drag-full-state/1735/`. Follow-up controls changed CPU angular-X velocity by one float increment (1.49e-8 rad/s) and vertical velocity by one increment (1.86e-9 m/s) immediately before step 1735: both endpoint differences rounded away. Changing the CPU's prepared separation for contact point 2 by one increment (5.96e-8 m), however, reproduced a 0.0474413 m/s endpoint difference. These are CPU-only perturbations with unchanged preceding history, not a tolerance waiver; see `artifacts/drag-sensitive-ulp/`.

Prepared-contact traces locate differences before the impulse jump: step 1735 regenerates its manifold; step 234 retains cached anchors. A combined local-coordinate manifold reduction and collision NoContraction experiment passes both selected replays (5.80e-6 and 4.15e-7 m/s), and 2,919 eligible seeded steps complete without a one-step outlier above 0.001 m/s. The other 141 steps were not seedable and are not passes. This diagnostic repeatedly imports CPU state and does **not** establish independent simulation parity. The actual unchanged ten-drag strict run regresses to 0.0600 m position and 2.20 m/s velocity, including held-motion failures; the candidate was removed from production sources. Its sources and measurements remain under `artifacts/drag-local-manifold-precise/`, `artifacts/drag-prepare-inputs/`, and `artifacts/drag-step-sweep/local-precise.*`.

Host body-space conversion and the C sample helper now use Box3D's cross-product quaternion rotation, matching the GPU integrator. This preserves vectors along the rotation axis when a stored unit quaternion's norm rounds slightly away from one. Pointer and isolated/ground-motion drag gates pass; the original strict velocity gate remains unresolved (latest peak 0.983 m/s). No experimental motor or clipping change is enabled. Detailed results are in ignored `artifacts/drag-anchor-input/NOTES.md`.


HUD labels:

| Label | Meaning |
|---|---|
| **GPU device** | full GPU-clock step (timestamps 0→sleep) |
| **GPU collide** | broadphase + narrowphase + graph |
| **GPU prepare** | island construction between collide and solve |
| **GPU encode** | queue submit |
| **GPU fetch** | one consumed pose snapshot for Sokol `DrawShape` (not a full contact mirror, not the GL import) |
| **CPU import / pose prep / draw list** | non-overlapping CPU stages (nanosecond subtract, then float ms) |

Do not sum collide+solve+integrate and call that the whole physics cost. Do not add those CPU times to GPU device. Setters after a wait must still win over that snapshot.

Linux NVIDIA may export poses over `VK_KHR_external_memory_fd` → `GL_EXT_memory_object_fd`. Sokol DrawShape does not consume that buffer yet. The Rust window renders directly from GPU body state.

`GPU_PHYSICS_GPU_CCD=1` enables topology-cached convex GPU CCD and resident stepping for eligible clean worlds. Steady frames submit physics and render without downloading body state. Meshes, sensors, bullets, and callbacks retain CPU CCD; jointed or contact-event worlds retain the compatibility mirror boundary. Dirty mutations take the upload path before residency resumes. Synchronous getters still wait and return finalized state. The default remains CPU CCD. Set `GPU_PHYSICS_RESIDENT=0` to retain the mirror boundary for an otherwise identical GPU-CCD comparison. Shape event/CCD eligibility and body bullet/component eligibility are cached from the uploaded scene metadata. Scene or body edits refresh that cache before upload clears the dirty bits; dirty reads inspect the current metadata. Live forces, joints, callbacks, contact events and pending CCD remain checked at their existing boundaries. A pending-force flag prototype passed analytic force/upload tests, but its five paired frame trials did not establish a consistent improvement (Dominoes p95 worsened in three pairs), so it was removed; the regression remains (`artifacts/pending-force-state/`). Focused mutation and residency regressions are recorded in `artifacts/scene-capability-cache/`.

The GPU CCD comparison covers 167 convex sweeps. It exposed a CPU GJK degeneracy: negative tetrahedron barycentric weights must reject a false interior result. The library suite passed 191/191 before the final topology-invalidation follow-up; all four focused integration tests passed afterwards. Batched Dominoes and mixed-stacks checks cover positions, orientations and velocities through step 120. A 240-step regression verifies zero body-mirror bytes and zero pose maps during resident stepping/render metadata calls, followed by correct synchronous reads, sleep events, teleport/force, deletion/reuse and growth.

Five matched same-binary trials on AC (`artifacts/resident-immediate-frames/result.json`, 30 warmup / 120 timed) reduced median frame p50 from 4.88 to 2.88 ms for mixed stacks and 6.45 to 3.03 ms for Dominoes. These compare two GPU paths, not the CPU engine. p95 was no worse in only 4/5 and 3/5 pairs respectively. `GPU_PHYSICS_PRESENT_MODE=immediate` requests and verifies wgpu Immediate support; compositor/display pacing is not proven absent. Real CPU Box3D through the same renderer remains necessary for a fair goal comparison. The Linux `oracle/` build now provides a private `libbox3d_viewer_cpu.so` bridge using the existing oracle scene definitions and the renderer's 96-byte COM-state layout. It exports only `viewer_cpu_*`, keeping native Box3D symbols isolated from the GPU C ABI. Configure with `cmake -S oracle -B oracle/build-viewer -DCMAKE_BUILD_TYPE=Release`, then run `python3 scripts/check-viewer-cpu.py`. This compares every body's pose and velocity with standalone Box3D over 121 frames for both named scenes. The Linux Rust viewer now selects this real CPU engine with `GPU_PHYSICS_CPU_REFERENCE=/absolute/path/to/oracle/build-viewer/libbox3d_viewer_cpu.so`; `GPU_PHYSICS_CPU_WORKERS` defaults to 4. Only Dominoes and mixed stacks are supported. CPU initial transforms and velocities are checked against the GPU fixture before drawing; CPU collection/upload is included in frame timing. Both modes share geometry, lighting, camera and presentation. `physics_submit` includes synchronous CPU stepping plus collection/upload in CPU mode, and host encode/submit in GPU mode; these scopes are not device-time comparisons. Five matched trials (`artifacts/shared-renderer-cpu-gpu/result.json`) give CPU/GPU frame p50 1.64/3.01 ms for mixed stacks and 2.13/3.36 ms for Dominoes. The GPU path remains slower. GPU host physics submission takes roughly 2.3–2.5 ms and is the next measured target. Presentation tails remain variable; no unpaced-display or end-to-end latency certification. `cpu_win_validated` remains false.

A body-count-bounded direct solver-dispatch experiment was removed after
five repeated pairs failed to establish a reliable frame benefit. Host submission
improved, but p50 improved in only 3/5 pairs for each scene, and p95 was no worse
in only 2/5 mixed-stacks and 3/5 Dominoes pairs. Indirect scheduling remains;
wgpu validation stays enabled. Historical evidence is in
`artifacts/direct-color-repeat/result.json`; the experimental environment switch
is no longer supported.

`GPU_PHYSICS_COMPONENT_TGS=1` is a whole-component solver prototype, disabled
by default. Small islands run per lane; islands exceeding eight bodies or 32
contacts use a cooperative workgroup, with overflow serial and remaining colors
parallel in reference order. Jointed, callback, kinematic and other moving-
immovable worlds retain global phases. Membership checks use persistent pair
keys and read-only shape metadata rather than concurrently updated hot contacts.
The relaxed phase stays uniform-derived to preserve the strict NVIDIA comparison. A workgroup-uniform mask skips only colors with no contacts in that component; active colors keep every barrier and their reference order.
The `1e-5` position/orientation/velocity comparison passes both named full scenes
after 120 steps and asserts both prototype dispatches ran. This is separate
from the disabled old per-body fused shortcut.
The small-component entry now defaults to workgroups of 16 lanes; the large
entry remains 64. `GPU_PHYSICS_SMALL_COMPONENT_WG=8|16|32|64` changes only this
small entry and its matching host dispatch divisor. Across four ordered stage
trials, mixed solve p50 improves 0.333→0.207 ms and completed-step 1.218→1.077 ms
(64→16). Five longer native GPU/GPU pairs improve p50 in every pair: median
mixed 1.951→1.778 ms; Dominoes 3.137→3.070 ms. Tails remain unresolved: mixed
median p95 12.906→12.914 ms, worse in four individual pairs. These are not CPU
comparisons. Empty/partial groups and all four sizes pass state comparisons;
the component solver itself stays opt-in. The final default release library
suite passes 206/206. Evidence: `artifacts/small-component-workgroups/`.
Sleeping components now return before local-list traversal and TGS substeps,
after island-wide wake propagation. The root sleep flag therefore represents
all writable members; waking/reconnecting a component still runs ordinary phases.
The merge/split regression covers sleep, wake and return to sleep at unchanged
`1e-5`. An exact-window Dominoes diagnostic reduces solve p50 0.780→0.704 ms.
Five viewer pairs give frame p50 3.040→2.965 ms; mixed-stacks tails remain variable.
This is a GPU-path improvement, not CPU-win validation. Evidence:
`artifacts/sleeping-components/`.

The prototype now builds compact body/contact/color lists on the GPU, using
parallel count/scatter and a cooperative prefix scan. Only nonempty large roots
are dispatched. Small components restore canonical order; large components
preserve color barriers and canonical serial overflow. List-capacity failure
invalidates the submission. Extra list storage allocates only on first enable.

Three paired development trials show global/prototype frame p50 **2.59/2.04 ms**
for mixed stacks and **3.48/3.06 ms** for Dominoes. Mixed stacks improved in
three of three pairs; Dominoes in two of three. Median trial p95 was
3.98/2.80 ms and 4.09/3.54 ms respectively, with visible desktop timing noise.
These are GPU-schedule comparisons, not CPU wins. The prototype stays off by
default. One 64-lane workgroup still limits parallelism inside a large connected
component; canonical overflow still scans the global list.
Evidence: `artifacts/parallel-component-prefix/result.json`; the earlier
compact-list/serial-prefix checkpoint is in `artifacts/compact-components/`.

A subsequent hybrid experiment returned large components to the global color
waves while retaining compact small components. It passed both scene comparisons,
merge/split ownership checks and four resident/CCD tests, but was slower than
compact components: mixed p50 2.52 vs 2.01 ms, Dominoes 3.94 vs 3.07 ms
(three interleaved development trials). The hybrid runtime code was removed;
measurements and its patch are archived in `artifacts/hybrid-components/`.
The retained compact path now also has a merge/split regression that checks
actual GPU large-component counts and states against the reference solver.

Dynamic greedy coloring still runs in one invocation, preserving pair-key order.
A temporary Dominoes graph probe measured 0.469 ms dynamic assignment out of
0.567 ms total graph time. The retained selector now loads both occupancy masks
once and bit-scans the lowest available dynamic color, retaining overflow and
all color/constraint order. A GPU fixture verifies exact colors including gaps,
all 20 dynamic colors and overflow; both full named-scene solver comparisons pass.
The full release library suite passes 200/200.
Normal device profiles show graph p50 0.559→0.511 ms; five frame pairs are
essentially flat (3.142→3.178 ms). This is a modest device improvement, not a frame
or CPU win. Temporary timestamp instrumentation was removed. Evidence:
`artifacts/graph-stage-probe/` and `artifacts/dynamic-color-bits/`.

`GPU_PHYSICS_GRAPH_SHARED=1` optionally caches coloring occupancy and counters
in workgroup memory. Cooperative lanes load/write masks and counters; one lane
retains the canonical greedy walk. It uses exactly 32 KiB (8,168 body-slot masks
plus 24 counters). Larger slot spans or devices with less storage retain the
original entry, without truncation. Native limits request at most the adapter's
supported 32 KiB; the extra shader/pipeline is created lazily only when eligible.
For spans up to 7,975 slots and at least 128 dynamic pairs, the shared path
uses spare occupancy storage to prefetch 64 contact endpoints cooperatively.
The greedy decisions remain serial in canonical order. Their unique contact metadata and color-list writes are published cooperatively after a workgroup barrier; the final storage barrier precedes graph finalization. This adds
no storage allocation or dispatch; larger spans and shorter lists keep the
original walk. Exact threshold/color tests, 600-step Dominoes checkpoints and
paired device/frame evidence are in `artifacts/graph-endpoint-prefetch/`. The parallel-publication follow-up passes exact colors, local indices, list entries/counts and masks. It saves 13% of graph and 3% of device time; five Dominoes frame pairs improve p50 about 3% but worsen aggregate p95 about 11%, so it remains provisional (`artifacts/graph-parallel-publish/`).


Graph classification now feeds three paired stable-compaction passes for static
and dynamic lists, replacing six separate dispatches without changing list order
or allocating buffers. Exact list tests cover maximum counts and stale scratch.
The final default release library suite passes 204/204.
Near-matched stage samples save about 0.008 ms; this is a work reduction, not a
validated frame/CPU win. `artifacts/paired-graph-compaction/` preserves the A/B
prototype; its temporary environment switch is absent from the final code.

With shared coloring enabled, `GPU_PHYSICS_GRAPH_MEMO=1` optionally reuses the
canonical dynamic schedule only when all ordered contact identities/endpoints,
initial body occupancy masks and color counts exactly match. It retains current
contact math and wake/CCD processing. Capacity above 8,160 bodies, meshes, joints
or insufficient device limits keep ordinary coloring. Private cache storage is
allocated only when requested and starts invalid on growth. The cache remains
off by default: stable mixed stacks benefits (five longer active GPU/GPU trials,
frame p50 1.937→1.836 ms), while changing Dominoes graphs can add overhead.
Four of five mixed p95 comparisons improve; this is not a CPU-win claim.
The full release library suite passes 203/203 with both flags requested, including
the exact-schedule and mutation/growth regressions.
See `artifacts/graph-memo/` and `../../docs/gpu-physics.md` for exact coverage
and both the short and longer measurements.

Exact coloring/mask checks cover the cache boundary, overflow and fallback;
both named-scene comparisons pass at 1e-5. The full release library suite passes
201/201 with the option enabled. A diagnostic reduces Dominoes graph p50
0.509→0.382 ms, but completed-step time is flat. Five short frame pairs give
Dominoes p50 2.999/3.116 ms and mixed stacks 2.868/1.889 ms (global/cached), with
p95 4.178/4.039 ms and 13.075/12.556 ms. Idle stepping was disabled in both.
These are GPU-path comparisons with variable presentation tails, not CPU wins;
the cache stays off by default. Evidence: `artifacts/shared-graph-cache/`.

A cooperative contact-batch variant of the cache was evaluated and reverted.
Dominoes graph p50 improved 0.380→0.214 ms, but five paired viewer trials had
worse Dominoes p95 and mixed-stacks p50; this did not establish a frame benefit.
The existing scalar cache remains unchanged. Candidate patch and all measurements:
`artifacts/batched-graph-cache/`.

A per-step HUD timestamp opt-out experiment was removed after five paired frame
trials failed to improve latency. With the cache enabled and idle disabled,
timestamps on/off yielded Dominoes p50 2.987/3.085 ms and mixed stacks
1.955/2.062 ms. Completion, explicit capture and injected-failure checks passed,
but this did not identify a useful performance change. Normal timestamp behavior
remains; no `GPU_PHYSICS_HUD_TIMESTAMPS` runtime option is retained. Evidence:
`artifacts/hud-timestamp-overhead/`.

Opt-in `GPU_PHYSICS_AB=bounded-static-sort` removes 32 static-sort dispatches
when a conservative topology bound permits at most two static edges per body.
One diagnostic mixed-stacks run reduced graph/device p50 from 0.61/1.46 ms to
0.28/0.94 ms. Five short frame pairs did not improve; three longer windows were
essentially flat (2.034/2.026 ms). This is a device-stage improvement, not a frame
or CPU win. Evidence: `artifacts/degree-two-static/result.json`.
A temporary renderer probe measured image acquisition around 1.1 ms in both
sort modes, versus about 0.03 ms preparation and 0.09 ms queue submission.
Acquiring before physics was tested and removed: frame p50 increased from
2.01 to 3.12 ms across three paired longer windows. Acquisition stays in its
original position and inside measured frame time; no deeper queue was introduced.
Evidence: `artifacts/frame-wait-probe/` and `artifacts/acquire-first/`.

A 256-lane large-component trial was removed: solve p50 changed only
0.65→0.63 ms with no repeated frame advantage. Retained 64-lane empty-color
skipping reduces Dominoes solve/device p50 from 0.65/1.93 to 0.55/1.84 ms
in a diagnostic run. Five paired frame trials improved median p50 3.32→3.17 ms
(four of five pairs), but p95 did not consistently improve. The component solver
remains opt-in; this is not a CPU win. Artifacts: `wide-components/` and
`empty-component-colors/` under `artifacts/`.

Fresh shared-renderer CPU comparison (five alternating trials, CPU4 workers,
warmup30/timed120) still misses the target: mixed stacks GPU/CPU frame p50
2.01/1.73 ms; Dominoes 3.14/1.22 ms, with GPU p95 worse in both.
The CPU bridge again exactly matches 121 standalone oracle frames for both
scenes. Mixed stacks is already asleep after this warmup, but the GPU still
runs its pipeline; active/settled windows need separate interpretation.
See `artifacts/combined-cpu-gpu/result.json`. No CPU win is validated.

Eligible resident convex worlds now use the sleeping-world shortcut by default;
`GPU_PHYSICS_IDLE=0` disables it for comparison. The GPU counts bodies requiring advancement after CCD;
the existing asynchronous status copy carries that count. Only a zero count
matching the latest submitted step, topology/state revisions and mutation epoch
permits skipping physics dispatches. Steps still submit normally and retain
completion, status, timestamps and pose exports. Awake/kinematic bodies,
unsupported worlds or stale proofs keep ordinary stepping. Mutations invalidate
the proof, including forces uploaded by a query before the next step.

Four regressions cover consecutive idle steps, submission identity, wake-up,
teleports, force/impulse after queries, deletion and capacity growth. Wake-up
matches ordinary stepping at unchanged 1e-5. This exposed and fixed ordinary
contact solving that modified warm-start impulses while both endpoints slept;
sleeping contacts now retain that history, consistent with Box3D sleeping sets.
The original release library validation passed 199/199. Settled overhead and
active-scene performance must be evaluated separately.

The default-enabled path passes 201 release library tests. Five longer matched
CPU/GPU trials give mixed-stacks aggregate frame p50 GPU1.150/CPU2.189 ms and
Dominoes GPU2.953/CPU1.231 ms; only one paired trial per scene meets both target
thresholds, so `cpu_win_validated` remains false. Evidence: `artifacts/idle-matched-current/`.


A settled mixed-stacks diagnostic (30 warmup / 60 measured, explicit completion)
reduced completed-step p50 from 1.446 to 0.068 ms and device p50 from 1.004 to
0.007 ms, with zero solver dispatches. Five alternating viewer trials show much
less consistent benefit: off/on frame p50 1.905/1.843 ms and p95 11.779/11.987 ms.
Active Dominoes p50 was 3.032/3.169 ms. These initial results did not justify enabling it
by default or claiming a CPU win. The single status staging slot can deliver a
proof too old for the exact-latest-step requirement; do not relax that requirement
or insert a synchronous wait just to enter the shortcut. Native timeline JSON
records `zero_solver_frames` to distinguish actual entry from requested mode.
Evidence: `artifacts/resident-idle/`; profile and frame results use the same binary,
with a separately recorded binary for the later entry-count diagnostic.

The opt-in viewer now also polls ready status once after presentation, without
waiting or copying body state; that cost stays inside total frame time. Five
paired before/after trials with idle enabled reduce mixed frame p50 2.253→1.310 ms
and host physics p50 0.468→0.074 ms. Zero-solver frames increase from a median
58 to 97 of 120. Frame p95 is worse (11.186→12.183 ms), so this remains an
experimental improvement, not default promotion or CPU validation. Some frames
still lack a current proof. Evidence: `artifacts/resident-idle-late-poll/`.

A three-slot status ring was tested and removed. Median mixed-stacks idle frames
increased 87→104 of 120, but frame p50 worsened 1.193→1.884 ms across five paired
trials (p95 9.036→7.415 ms). Dominoes p50 was essentially flat at 3.049/3.016 ms.
The single-slot late-poll path remains. This rejects more status buffering as the
next optimization; repeated status copies during already-proven idle steps are
a better candidate to investigate. Tests/patches and all trials are archived in
`artifacts/resident-status-ring/`; the ring is not present in runtime code.
Omitting status copies/maps during proven-idle steps was also tested and removed:
five mixed-stacks pairs reduced host submit p50 0.077→0.046 ms, but frame p50
1.183→1.243 ms and p95 5.093→7.367 ms did not improve. Four focused wake/status
identity tests passed; the prior single-slot implementation remains unchanged.
Evidence: `artifacts/resident-idle-no-status/`. Further idle bookkeeping is deferred;
active Dominoes graph/solver work is the next performance target.

The Rust window uses Box3D's state colours (tan awake, slate sleeping,
orange fast, turquoise awake bullets, blue kinematics, grey statics, wheat
sensors), selected from live GPU flags without pose readback. Custom colours
still override them. CPU-only TOI/speed-cap diagnostic flags and debug material
presets are not yet represented by this renderer; lighting remains simplified.


**C ABI regression** (link `libgpu_physics.a`, not the `.so`):

```bash
./c_abi/build_and_test.sh
```

Hulls are cooked with Box3D C (`b3CreateCylinder`, `b3CreateHull`, …). Rust uploads the blob; WGSL steps the world.

`WorldDef.capacity` is a peak slot pool (default floor 256 bodies). Spawn/despawn within that peak does not rebuild contacts. Overflow doubles and copies.

## Compare grid

```bash
./scripts/record-box3d-oracle.sh              # CPU Box3D column
./scripts/record-snapshot.sh YYYY-MM-DD-label
bun run compare                               # http://127.0.0.1:8766/
```

Rows are samples. CPU stays on the left; newer GPU snapshots sit next to it. Clips are in `recordings/snapshots/` (Git LFS). `SKIP_MP4=1` refreshes timings only. Oracle video replay accepts both the historical 144-byte body record and the current 160-byte record; the appended island/sleep fields do not change the rendered pose prefix.

## Checks

Native API coverage remains incomplete. See the [completion scope](../../docs/gpu-physics.md). From this directory, `python scripts/audit-native-api.py --require-complete` reports remaining gaps (and deliberately fails until they are closed); `./scripts/check-api-settings.sh` tests public C settings in GPU-only and dual builds. The Rust library test `api_completion_tests` checks that per-body sleep settings actually affect GPU integration.


```bash
./scripts/correctness-gate.sh
./scripts/native-scene-gate.sh
./scripts/balanced-bench.sh
cargo run --release -- --self-test --scene box-stack
cargo test --manifest-path experiments/gpu-physics/Cargo.toml --lib -- --test-threads=1
```

`--self-test` dumps the default checkpoints plus the requested final frame. Same adapter should `memcmp`. `--no-sleep` keeps bodies awake.

The gate writes `correctness-latest.json` (`source_sha256`, split file/bin/adapter hashes in `fingerprint_parts`, `box3d_rev`, pass/fail/skip). Skipped required oracle compare is **incomplete**, not pass. Long-run 3600-step jobs inspect periodic and final physics quality (NaN, floor loss, speed/joint bounds), not just process exit.

`native-scene-gate.sh` writes a **fresh** `artifacts/native-scene-<stamp>/` plus `native-scene-latest.json`; reusing a stamp is refused. Each listed Sokol sample is pass, fail, incomplete, or unsupported against a matched CPU trace. Every frame requires finite body poses/velocities, joint anchor/angular measurements, matching identities/settings and physics-step sequence. Failed CPU runs cannot supply passing references. GPU health positions use body origins, matching CPU getters (v14 incorrectly used GPU COM). Anchor-constrained joints allow CPU deflection plus 0.05m and angular constraints allow CPU error plus 0.05rad; other extrema remain broad screening checks, not comprehensive compatibility proof. Ghost Collisions additionally checks its known flat-floor support and launch limit. The generated Mesh Drop benchmark uses `GPU_SOKOL_SEED` (default 52977 in the gate) and requires matching recorded seeds; interactive clock seeding remains the default outside the gate. Compound/Village is checked with its ordinary 52,502-shape scene, a sphere-drop support probe, and a same-process switch. `--health-scan` is for that gate only; omit it from performance timelines. `cpu_win_validated=false`. Do not treat `cargo test` counts as native compatibility.

Gear Lift requires 1,200 measured steps after two warmup steps. Its 120 rocks use complete hull/prism SAT against the actual stair, back wall and ground, comparing >0.02m penetration frequency and duration against the matched CPU window; peak depth may exceed CPU by at most one 0.005m linear slop. Complete floor crossings and deep final penetration fail. Mechanism identities, ordinary joint error/velocity/launch checks and driven-gear phase remain required. This replaces only the rocks' chaotic per-body vertical trajectory comparison. NumPy is required by `scripts/gear_support.py`; missing dependencies or changed source hashes in `gear_support_fixture.py` fail closed. Regenerate the geometry capture from the native sample after sample/default changes. `scripts/test_gear_support.py` checks rejection of each support failure independently. Historical failing trajectory results remain in `artifacts/milestone-hull-witness/`; reclassification evidence belongs in a new artifact directory.


Latest v15 continuation (`artifacts/v15-prismatic-validation.json`): 137 release library tests and five real CPU/GPU prismatic oracle fixtures pass. The fresh native matrix remains eight scene passes, Gear Lift and Mesh Drop fail, Village unsupported and its switch incomplete. Gear Lift's peak constrained angular error improves from 0.1021863 to 0.0669222 rad, still outside the screening budget. The last complete aggregate gate was the earlier checkpoint (25 pass / 1 fail / 0 skip); this continuation ran the library, prismatic oracle and native matrix separately. See [the detailed results](../../docs/gpu-physics.md). Native compatibility and a CPU performance win remain unvalidated.

For a first-divergence capture, run the native GPU sample with `--health-scan` and `GPU_PHYSICS_TRACE_BODY=<one-based body id>`. CCD start/end poses and clamping fractions go to stderr. Add `GPU_PHYSICS_TRACE_CONTACTS=1` for that body's contact manifolds, including normal, COM-relative anchors, cached state and impulses. Use an absolute `--bench-json` path and save stderr beside it. These diagnostics deliberately synchronize and are not performance measurements. For Mesh Drop, set the same `GPU_SOKOL_SEED` for both engines.


### Bench scripts

| Script | What it measures |
|---|---|
| `--bench` | completed-step p50/p95 (submit + device wait, no pose mirror), `step_plus_host_mirror`, GPU-clock stages, encode (`encoder.finish` included), awake/live-contact counts, sleep-window labels, five isolated CPU oracle trials |
| `./scripts/balanced-bench.sh` | matched 5×CPU / 5×GPU windows at warmup 0, 20, 200, plus early and late sleep windows |
| `./scripts/sokol-timeline.sh` | `samples:cpu\|gpu\|both` with `--bench-json` (unpaced/paced, `--completed-step`, `--pause-script`) |
| `./scripts/native-timeline.sh` | Rust window only |
| `./scripts/scale-sweep.sh` | sparse / dense / jointed scale |
| `./scripts/capture-v9-baseline.sh` | freeze source+binary hashes, then gate + balanced benches |

Use `--warmup` for late windows (e.g. `--warmup 200 --frames 100`). Rebuild before writing JSON.

`bench-dominoes-sleep-late` is the late sleep-enabled window; warmup 200 is not proof of rest.
Completed-step benches honor exactly `--warmup` and `--frames` on both engines.
Sleep-enabled runs no longer add an automatic GPU-only settling interval.
`python3 scripts/check-bench-window.py` checks both sleep modes against the CPU oracle.
Historical sleep-enabled headless reports may contain `settle_wait_steps > 0`:
their GPU/CPU windows differ, so do not use them as matched CPU comparisons.
Native timeline runs were not affected. `bench-dominoes.historical-fused.json` is an old fused-TGS run and is not current evidence. Fused islands stay disabled.

Timestamp HUD uses a two-slot map ring and does not stall the step to finish a map. Sample IDs are submitted physics steps, not harvested-sample counts.

### Results

Balanced `--no-sleep` on RTX 4070 Laptop/Vulkan (v9 spot freeze, 5 GPU + 5 CPU trials; wall is the CPU latency authority; not a v10 CPU-win claim):

| Workload | bodies | window | GPU completed-step p50 | GPU device / encode | CPU wall p50 (5 trials) |
|---|---:|---|---:|---|---|
| Revolute | 3 | warmup 0, 20 timed | 1.32 ms | encode 0.51 ms | ~0.004 ms |
| High Resistance | 11 | warmup 0, 20 timed | **1.28 ms** | encode 0.47 ms | ~0.009 ms |
| Dominoes | 5431 | warmup 20, 20 timed | 5.46 ms | encode 2.31 ms | 4.78–6.04 ms |
| Mixed stacks | 602 | warmup 0, 20 timed | 4.62 ms | encode 1.98 ms | 0.68–0.72 ms |

That is **not** a ≥20% completed-step plus Sokol GPU-only unpaced CPU win (`cpu_win_validated=false`). Tiny scenes remain CPU-faster.

Native application cadence is `./scripts/sokol-timeline.sh` (physics / draw / render / UI / commit). v9’s empty `Render()` column is not evidence. DrawShape still uses the CPU pose snapshot. HUD GPU times carry a submitted **physics step id**.

### Solver notes

- High Resistance uses the one-workgroup color wave only when the host pair bound is ≤64 (13 solver dispatches vs 325 `general-solver` / large-world fallback).
- Static radix is skipped by default when `D_b * N ≤ 1`. Opt-in `GPU_PHYSICS_AB=bounded-static-sort` extends this to degree two, using parallel minimum-pair ranking. No meshes; higher/unproven degrees retain the general sort. GPU degree verifies the host bound.
- Large worlds keep the 24-color multi-workgroup encode plus the GPU-gated fused prefix. Applying the 64-lane color loop *instead of* the 24-color path fails mixed-stack support.
- Revolute keeps those 13 contact waves and separate joint dispatches.
- Joint filtering is a pair hash with scan fallback. Joint stores skip immovable endpoints.
- Compact parallel joint solving is the default (256 independent pendulums: completed-step p50 8.7 ms vs serial 12.6 ms). `GPU_PHYSICS_AB=serial-joints` is the containment path. A single long chain stays serial inside its component.
- Closest-hit (`CastRayClosest`) is GPU-only: primitives, triangle meshes, and height fields, with a dedicated query buffer and a single deterministic writer on equal fractions (lowest CPU shape index, then triangle). Callback `CastRay`, overlaps, and shape casts stay on the host. Sokol JSON records `query_wait_ms` / `query_dispatch_ms` (exclusive of map) / `query_map_ms` / `query_encode_ms` / `query_copied_bytes`. Do not add dispatch and map as independent GPU time. `cpu_win_validated=false`.
- Island-local contact waves and per-island dynamic coloring are **not** the default.
- Body slots and pair/shape ids remain 16-bit (65536). Capacity drops are sticky and fail-closed: later submissions are refused.

The Rust `BodyDef.enable_contact_recycling` and native `b3BodyDef.enableContactRecycling` settings are honored per endpoint. Disabling either endpoint forces fresh contacts; the default is enabled. Runtime `b3Body_EnableContactRecycling` and `b3Body_IsContactRecyclingEnabled` also work in GPU and dual mode. Metadata reads and unchanged setters avoid world downloads. The follow-up passes 136 library tests plus the public C query cohort; after building `samples:both`, `./scripts/check-recycling-both.sh` verifies flags in both real worlds. The subsequent prismatic continuation passes 137 library tests and five CPU oracle fixtures; its fresh native matrix remains 8 pass / 2 fail / 1 unsupported / 1 incomplete (`artifacts/v15-prismatic-validation.json`).

Prismatic joints use the CPU reference's two-axis point-to-line constraint, moving-line Jacobians and impulse warm start. `./scripts/check-prismatic-reference.sh` compares five 120-step native fixtures against real Box3D; it is required by the correctness gate. The targeted Gear Lift rerun reduces peak angular error to 0.0669222 rad, but still fails the strict native screen. These are correctness checks, not new performance measurements.

Optional: `GPU_PHYSICS_AB=phase-capture,no-sleep` writes `phase-probe.b3pr`. Other flags: `bounded-static-sort`, `no-recycle`, `no-sat-cache`, `no-rolling`, `rebuild-graph`, `general-solver`, `general-static-sort`, `joint-filter-scan`, `serial-joints`, `parallel-joints`.

## Layout

| Path | What |
|---|---|
| `src/api/` | Rust `b3_*` |
| `src/c_abi.rs`, `c_abi/shim.c` | C ABI for the sample app |
| `shaders/physics/` | Compute (concatenated in `src/sim.rs`) |
| `shaders/render.wgsl` | Instanced draw |
| `c_abi/cohort_sample.cpp` | Headless ABI scenes |
| `native-samples/` | Erin’s viewer linked against this crate |

## Ship bar

- `scripts/correctness-gate.sh` green (invariants, Dominoes repeatability, long-run no crash)
- Compare-grid clips vs the CPU column (record CPU oracle first)
- Stacks and joints that stay up; pause/single-step matches continuous stepping
- No CPU-win claim from fused-path or unmatched HUD vs oracle numbers

Not a gate: bit-identical poses across GPUs, or Box3D trajectory match to `1e-5`.

### v15 contact and terrain validation (2026-09-10)

The v15 checkpoint passed **141/141** release library tests. Prismatic constraints use CPU-reference moving-line Jacobians and impulse warm starts; the five-fixture CPU comparison remains required. Fast mesh contacts bypass recycling using a GPU-computed motion flag, and cached bounds include every child shape (16 extra bytes per body, no new dispatch/binding/readback). Slow/non-mesh recycling and runtime continuous-off are covered. Event endpoint/hit reconstruction now uses stable body slots, with a regression for preceding deletions. Hull–mesh backface rejection includes the CPU's 5 mm tolerance; spheres/capsules keep their strict rule.

The latest full native matrix is **8 pass / 2 fail / 1 unsupported / 1 incomplete** (`artifacts/native-scene-20260910-v15-fast-mesh`). After the event/backside fixes, targeted matched runs in `artifacts/v15-slot-backside-native/result.json` keep Wave Pile passing. Mesh Drop now keeps body 233 supported and settled, but body **842** fails at step **101**. Gear Lift still fails at step 30, joint 78 (angular error 0.0560034 rad). These targeted checks do not replace a fresh full matrix. Village remains unsupported, its switch incomplete, and `cpu_win_validated=false`. `samples:both` has been rebuilt.

`./scripts/check-mesh-impact-reference.sh` is a required correctness-gate case. Both engines start native body 233 from identical frame-55 state on the actual wave mesh. The latest result (`artifacts/v15-mesh-impact-geometric/result.json`) passes terrain support and settling but **fails distinct manifold normals**: at step 58 Box3D retains two non-coplanar patches while GPU retains one. Support is measured against the cooked triangles beneath the body; CPU trajectory deltas are separate diagnostics. The overall oracle remains fail. The validator's synthetic self-check is not physics evidence, and the aggregate correctness gate has not been rerun for this follow-up.

Mesh face retention and bounded point reduction fixed earlier support failures but cannot represent several independent normals/friction states. Next work reproduces body 842's first divergence at steps 57–58, checks missing triangle face/edge/fallback semantics, and then adds proper manifold storage and scheduling. The earlier body-233 fall was resolved by backside tolerance, so missing multiple manifolds must not be assumed to cause every remaining tunnel. Public contact-data stubs and compound/event identity mapping also remain outstanding. No native compatibility or new performance-win claim is made.


### v16 checkpoint: manifold ownership infrastructure (2026-09-10)

The checkpoint build passed **141/141 release library tests** (`artifacts/v16-checkpoint-lib.log`). Body-842 shared-state diagnostics reproduce the native Mesh Drop failure; splitting the same terrain into separate triangle shapes makes it settle. This is diagnostic evidence for preserving multiple contact normals, not a production mesh replacement. The mesh-impact validator explicitly rejects split-mesh diagnostic runs.

Contacts carry explicit manifold links, and every solver entry point visits the chain under one body-pair owner; graph coloring excludes child slots. The bounded GPU allocator reserves primary pairs first, retains old children until history transfer completes, and reports exhaustion rather than dropping constraints. Per-point triangle IDs have separate persistent storage (16 bytes per slot; hot solver storage unchanged). Warm starts require both triangle and feature identity, and clipping/reduction preserve those identities.

Production mesh narrowphase now generates per-triangle candidates, clusters compatible collision and triangle normals, matches old patches once each, and publishes complete chains. Recycling checks and updates every member; sensor overlap checks every patch before retaining one event witness. Contact events aggregate multiple patches under one pair identity. Empty candidates are explicitly rejected before allocation; a regression exercises this between two live patches. The current release library suite passes **153/153** (`artifacts/v16-mesh-multinormal-lib.log`). The full correctness gate is **28 pass / 1 fail / 0 skipped** (`artifacts/v16-multi-manifold-correctness.json`); its only failing case is the native matrix. Both mesh oracles, all CPU comparisons, query cohorts and all three 3600-step runs pass. `correctness-latest.json` reflects this failing aggregate, and `artifacts/v16-multi-manifold-validation.json` records the remaining scope. Two earlier tests assumed one mesh manifold; executed Box3D fixtures in `c_abi/mesh_patch_normals_reference.cpp` demonstrate two, and the GPU tests now verify the corresponding normals without depending on slot order.

The required shared-state body-233 mesh-impact oracle now **passes** distinct normals, terrain support and settling (`artifacts/v16-mesh-guard-oracle/result.json`). At frame 58 the GPU retains both CPU terrain normals; final COM-y differs by about 2.9 mm and tail speed is below 0.00007 m/s. The separate body-842 reproduction also settles through frame 100 (`artifacts/v16-mesh-guard-842.txt`). Its evolved frame-58 normals differ from CPU after the first impact; a replay from identical CPU frame-57 state produces all three CPU normals (`artifacts/v16-842-frozen57/`). The gate now includes that frozen-state fixture so contact geometry is compared before trajectory divergence. Neither reproduction replaces the native Mesh Drop scene. The fresh native matrix (`artifacts/v16-native-result.json`) is **9 pass / 1 fail / 1 unsupported / 1 incomplete**. Mesh Drop completes all 180 measured steps and passes the CPU screening envelope. Gear Lift fails at step 90 (body 175 more than 2 m below CPU); Village remains unsupported. These are screening checks, not full contact/constraint certification. The CPU/GPU/both viewers have been rebuilt. CPU tentative-edge topology filtering, general triangle narrowphase parity, mixed-material cluster policy, compound/event identity, Gear Lift and Village capacity remain work to validate or complete. No native compatibility or performance win is claimed; `cpu_win_validated=false`.

The pre-commit comparison grid has a new `2026-09-10-v16-checkpoint` GPU column (120 frames per scene, one diagnostic metrics run), alongside the existing real Box3D CPU column. These recordings are visual regression evidence, not matched performance trials.


### Contact identity and callback follow-up

GPU collider indices now map to public shapes consistently across events and callbacks: compound parents consume no GPU slot, hidden children report their parent, and body slot holes are preserved. Callback decisions are shared per public pair. Custom-filter-only steps now update completion/pose/event bookkeeping. Pre-solve vetoes validate and clear every owned manifold, invalidate cached geometric misses, and can be reversed while stationary. Host destruction/refilter end events are published without requiring a GPU contact download after the last event-enabled shape disappears. Begin events retain actual GPU start flags, so a first late read does not manufacture a begin.

Current validation is **157/157 release library tests** and **10/10 affected C-ABI cohorts**, recorded in `artifacts/v17-contact-identity-validation.json`; `samples:both` is rebuilt. The full/native gate has not been rerun for this host/API slice; the v16 28/1 aggregate and 9/1 native matrix remain historical evidence. Public contact-data/validity APIs are still stubs. Completing alive-pair IDs, non-touching validity, skipped-harvest event history and manifold exports is the next API work. Gear Lift, Village and measured GPU performance remain unresolved. No new commit or CPU-win claim.


Point-history validation now passes **158/158 release library tests**, both shared-state mesh-impact oracles (233 and 842), and the `samples:both` rebuild. Four existing lifecycle bits preserve actual point matching through publication and recycling, including zero-impulse persisted points; buffer sizes are unchanged. A real CPU fixture confirms the zero-impulse semantics. Evidence: `artifacts/v17-point-provenance-validation.json`.

The skipped-read C-ABI reproduction confirms a remaining event bug: after reading the first begin, separating on step 2 without harvesting, and reading step 3, GPU reports one stale end while CPU reports none (`artifacts/v17-event-history/result.json`). This is a known failure, not a passing compatibility claim. Contact getters remain unfinished and the full native/performance gates have not been rerun.


Checkpoint `0e014bc` includes the mesh/identity/persistence work and 300-frame CPU/GPU comparison recordings. The event-history follow-up preserves previous-step touching keys in the existing GPU broadphase-clear pass, before retirement/reuse. It uses four bytes per contact slot in scratch storage (256 KiB reserved at maximum pair capacity), no additional dispatch or storage binding, and no per-step CPU history download. Explicit contact-event harvest copies the live capacity's history alongside the existing contact snapshot. Immutable public-shape maps preserve the previous dense-index interpretation across compound-child deletion; ordinary steps share these maps without rebuilding them.

`scripts/check-contact-event-history.sh` is now a required correctness-gate case. The executed CPU/GPU cases cover skipped old ends, latest-step ends, separation/re-touch, and an end whose begin was never read. All four pass in `artifacts/contact-event-history-gate/result.json`. The original stale-end failure is retained in `artifacts/v17-event-history/result.json`. Contact-ID lifetime, the public contact-data API, and skipped-harvest sensor/continuous-event history remain separate work; these changes do not establish full native compatibility. The follow-up is uncommitted.


Final event-history follow-up validation: **158/158 release library tests**, **4/4 matched CPU/GPU event-history cases**, **10/10 affected C-ABI cohorts**, and a successful `samples:both` rebuild. Both mesh oracles passed with the new scratch layout before the final host shape-map correction. `artifacts/v17-event-history-validation.json` records the exact scope. No fresh full native gate or performance-win claim; all follow-up changes remain uncommitted.


### Native manifold data and remaining contact API

The shared decoder in `src/api/contact_data.rs` now supplies native-layout manifold data to trace diagnostics: cached/current separation, original COM anchors, final/total impulses, pre-solve velocity, full feature and triangle IDs, persisted bits, and world-space friction/rolling impulses. This avoids confusing the packed solver separation with the native cached separation. Native C++ and Rust layout assertions agree. `scripts/check-contact-manifold-reference.sh` executes the layout/decoding and CPU/GPU persistence checks. The library suite is **160/160**, both mesh oracle comparisons pass, and `samples:both` builds. B3TR format is unchanged.

The six contact getters now use a cached public-pair registry and native manifold data. Explicit reads refresh once per changed world state; repeated getters reuse the snapshot. Public shape ordering follows Box3D, and contact handles do not alias across world-slot reuse. The world contact-recycling distance and standalone body-destruction bridge are implemented as well.

Checkpoint `9259d97` validation: **161/161 release library tests**. The expanded native lifetime gate passes CPU box/sphere/compound and GPU box/sphere, but **GPU compound crashes during factory-built hull import before stepping**. The shim box factory supplies only an AABB, leaving an invalid cooked hull for compound cloning. This is a required failing case, not native compatibility. Evidence: `artifacts/contact-api-reference-gate/result.json`. The earlier stub failure in `artifacts/v17-manifold-data/result.json` is historical.

Compound public-pair lifetime across child gaps and unread steps remains unresolved: physical child contacts are not sufficient proof of public compound-pair lifetime. Skipped-read sensor history, Gear Lift, Village capacity, and measured GPU performance also remain open. No fresh full native gate or performance-win claim applies to this checkpoint.


The follow-up removes the incomplete box/cube/offset-box shim factories and uses the actual upstream geometry factories already compiled into the experiment. This fixes the factory-built compound import crash without changing upstream sources or adding CPU physics stepping. The lifetime fixture now checks cooked hull size, topology, volume, points, and translated cloning before simulation.

Current `artifacts/v17-hull-factory/contact-api/result.json`: **7 pass / 1 fail** (four cases per engine). Both factory cases pass; GPU compound construction and initial contact succeed, then validity fails at the child-gap check (exit 20). This is the public-pair lifetime limitation described above, not the former import crash. `samples:both` rebuild succeeds. The follow-up is uncommitted; full native compatibility and a GPU performance win remain unclaimed.

Factory-repair validation: **161/161 library tests**, latest-step event and manifold gates pass, and both mesh-impact comparisons (233/842) pass. Rebuilt `samples:both` completes High Resistance (2 warmup + 8 measured frames, zero Sokol errors); this short run is a smoke check, not a performance result. `artifacts/v17-hull-factory/result.json` keeps overall status **fail** because the compound lifetime assertion still fails.


### Native compound contact contract correction

Further CPU fixtures and `box3d/src/contact.c::b3CreateContact` / `broad_phase.c::b3PairQueryCallback` disprove the earlier one-contact-per-public-parent-pair assumption. Native compound contacts include **child index** in their key. Overlapping children create separate contact IDs (each may itself have multiple mesh manifolds). A child contact survives a gap while parent fat bounds overlap; touching another child creates a different ID while the old zero-manifold contact can remain valid. Initial outer-AABB overlap without a child candidate creates no contact. The host registry/event union and corresponding GPU-only regression expectations are therefore incomplete compatibility work.

A trial that added one parent ghost pair was removed after these CPU failures (`artifacts/v17-public-proxy/rejected.json`); its green library run is not native validation. The required native fixture now covers separate overlapping-child contacts, initial gaps, unread child switching, and ray identity. The ray path now returns the public parent ID with a stored, stable native child ordinal. Both native import paths pass the original ordinal explicitly; sibling deletion does not renumber it, and newly attached children do not reuse retired ordinals. These findings supersede the prior plan's recommendation for a single public parent-pair registry. No overall pass is claimed.

Validation for the compound-contract correction: **162/162 library tests**; required native API gate **12 pass / 4 fail** (CPU 8/8, GPU 4/8). GPU factory, box, sphere, and compound-ray cases pass. Gap retention and per-child identity cases remain mandatory failures. `samples:both` builds and completes Compound/Simple (2 warmup + 8 measured smoke frames, zero Sokol errors). Source/binary fingerprints and exact scope: `artifacts/v17-compound-contract/result.json`. No changes from this follow-up are committed or pushed.


The child-contact follow-up implements structured `(public shape pair, child ordinal)` keys for contact data and begin/end/hit history. Submitted ordinal maps are shared between steps and rebuilt only with topology, preserving previous-step interpretation through child deletion. Custom-filter and pre-solve decisions distinguish child contacts too; mesh manifold chains remain grouped within their owning child contact.

GPU children now reference bounds-only parent metadata. Initial candidate insertion still visits physical children; retained child roots use parent bounds for disjoint tests and their original geometry for narrowphase. Thus an old child can remain valid with zero manifolds in a gap, while touching a different child creates another ID. Parent records never enter candidate hashing or ray geometry. This adds one existing-format shape record per compound, without another solver contact, dispatch, storage binding, or per-step scene readback.

Required native contact gate: **20/20** CPU/GPU cases, including overlapping children, initial gap, unread switching and retirement/recreation, public ray identity, callback counts, and selective pre-solve veto. Library suite **162/162**; event-history/manifold checks and mesh-impact comparisons 233/842 pass. Evidence: `artifacts/v17-child-contacts/`. This is not full native compatibility: CPU fat-proxy containment/hysteresis is not yet reproduced, topology edits still use the adjacent-observation identity fallback, and Gear Lift, Village, sensor history and performance certification remain outstanding. The earlier rejected single-parent-contact design was not reinstated.


The fat-bound follow-up stores persistent world AABBs in an eight-word-per-shape scratch tail, updated by existing broadphase-clear lanes. Candidate hash ranges and overlap tests use that cache. Standalone/public proxies preserve their fat box until new tight bounds escape; compound child discovery uses raw child bounds. The parameter block remains 256 bytes and no dispatch, binding, or readback was added. Allocation reporting includes the tail.

The executed CPU/GPU per-step movements near x=4.10/4.14 now retain the same contact. A new batched-teleport fixture remains a **required failure**: two SetTransform calls before one step update CPU proxy history twice, but GPU currently sees only the final pose. Native gate **23 pass / 1 fail**; library **162/162**, event/manifold checks, both mesh-impact comparisons, and rebuilt samples:both Compound/Simple smoke pass. Evidence: `artifacts/v17-fat-proxy/result.json`. Scene rebuilds also reset the cache, so unrelated edits and topology-generation continuity remain unfinished. No full native compatibility or performance win is claimed.


### Ordered transform history checkpoint

The GPU now replays each body's ordered host SetTransform history in the existing broadphase pass, with one packed upload per submission. Zero-duration steps retain pending history; callback broadphase passes consume each batch once. Material-only scene uploads preserve unchanged fat bounds using geometry keys instead of invalidating the whole cache. No ordinary-frame dispatch or readback is added.

Current validation: 162/162 release library tests and 31/32 required CPU/GPU contact cases. Batched transforms, callback stepping, dt=0, and friction-only edits pass. The new compound-fat-grow case fails on GPU at retirement (exit 25): after adding 300 unrelated bodies, a far teleport leaves the old contact valid. This checkpoint deliberately retains that required failure. Evidence: `artifacts/v17-transform-history/contact-api/result.json`. Growth must preserve/rebuild contact scheduling and fat-bound history correctly; packed topology identity, full native compatibility, and performance certification remain unfinished. `cpu_win_validated` remains false.


### Capacity-growth history repair

After checkpoint `cc7537a`, GpuSim growth now preserves the occupied-contact count/list needed to revisit retained roots, plus matching fat-bound records and transform-history epochs. Fixed-layout contact metadata and coalesced matching bounds are copied GPU-to-GPU in the existing reallocation submission; body-dependent graph scratch is rebuilt. Normal steps gain no dispatch or readback.

Fresh validation: **34/34 native CPU/GPU contact cases**, **162/162 release library tests**, event-history and manifold gates pass. The growth cases verify retention, far retirement, recreation, and batched teleports after reallocation. Rebuilt `samples:both` completes Compound/Simple with two warmup/eight measured smoke frames and zero Sokol errors. This concurrent-validation smoke is not timing evidence. Fingerprints and scope: `artifacts/v17-growth-history-final/result.json`.

This follow-up is uncommitted. Dense physical contact keys across shape compaction/reuse, sensor history, Gear Lift, Village, full native validation, and performance certification remain open. Preserving bounds by source slot and geometry does not establish replacement-shape generation identity. `cpu_win_validated` stays false.


### Required topology identity regressions

The expanded native contact gate is **40 pass / 2 fail** (`artifacts/v17-topology-persistence/result.json`). Both failures are GPU-only; all 21 CPU cases pass. Deleting an unrelated earlier shape compacts GPU collider indices. After three unread steps, the surviving contact changes ID (exit 73). Reading immediately masks that change through the host adjacency fallback, but a separate fixture proves all four native manifold points persist while the GPU reports a new point (exit 80). Immediate latest-step event counts and same-slot replacement identity pass these fixtures. These results do not prove warm-start impulse parity: the fixture deliberately uses zero gravity to isolate identity and point history.

Do not fix this by relaxing ID/persistence assertions or retaining public IDs indefinitely. Preserve physical contact roots, manifold chains, and point history across generation-aware shape-index remapping. Rebuild the GPU contact hash consistently with remapped packed keys, preserve occupied-root traversal, and maintain old submitted keys for previous-step event decoding. The current growth repair does not solve in-place shape compaction. No runtime correction for these two failures is claimed yet; the required gate now exposes them.


### GPU topology remapping implemented

The required shape-hole ID and point-persistence failures are repaired. Geometry uploads track actual shape slot/generation identities. On compaction or replacement, three GPU passes remap every surviving physical contact key, retire deleted endpoints, and rebuild the root hash. Physical contact slots, manifold links, and point history survive. Occupied-root traversal remains intact; matching fat bounds move through coalesced GPU copies rather than a CPU download. Append-only additions skip the contact-remap passes. Normal stepping gains no dispatch or readback; the existing 256-byte parameter block carries the remap controls.

The remap captures old touching keys before changing them, preserving previous-step event decoding through the next submitted physics step. Zero-duration steps do not advance submitted shape maps. Contact getters after a query-triggered rebuild use the actual GPU slot identities while retaining the old submitted event maps; this fixed an additional executed native failure where getters returned zero contacts before the next step.

Fresh evidence in `artifacts/v17-topology-remap-verified/`: **54/54 native CPU/GPU contact cases**, **162/162 release library tests**, event-history/manifold gates and mesh-impact references 233/842 pass. The native cases cover unread compaction, point persistence, compound contact ownership, growth plus compaction, dt=0, query-triggered uploads/getters, and fresh history for a replacement shape. The identity fixture uses zero gravity, so it does not certify nonzero warm-start impulse parity or whole-scene stability.

The host adjacency fallback remains until broader shape-edit/manifold ownership cases prove it unnecessary. Multiple query-triggered topology rebuilds before one step, changed geometry without a generation change, mutable compound-child lifetime, sensor/continuous event history, Gear Lift, Village, and matched performance certification still need work. No full native compatibility or CPU-win claim; these follow-up changes are uncommitted.


### Native settings and mesh-seam follow-up

Repeated query-triggered topology edits (including slot reuse, growth and dt=0 before the next step) now have an executed passing CPU/GPU regression. The full native matrix on the remapping build remains **9 pass / 1 fail / 1 unsupported / 1 incomplete**, at `artifacts/native-scene-20260910-v17-remap-summary.json`. Gear Lift still exceeds the per-body downward-drift screening threshold; Village is refused and its switch case incomplete. That matrix predates the following setting/seam repair.

Native GPU and both-mode shape import ignored `ShapeDef.enableSpeculativeContact`. Both now forward it into the Rust shape state. The WGSL hull/mesh gate checks both endpoints, matching native's hull-only experimental setting; sphere/capsule speculation is not disabled by it. An executed positive-gap fixture previously retained GPU hull contacts when CPU disabled them, and now passes for either disabled endpoint.

The sphere control exposed a separate missing-contact bug exactly above a shared flat triangle edge. The rounded-shape sample path discarded both triangles after its projected closest point was classified as an edge. It now preserves face support for a projection lying on that boundary, while rejecting outside projections. The native regression requires exactly one sphere contact point, preventing duplicate seam constraints.

Fresh scope: **64/64 CPU/GPU contact cases**, **162/162 release library tests**, mesh-impact references 233/842 pass, GPU/both viewers rebuild. Ghost Collisions explicitly disables speculative contact in upstream; its corrected GPU run completes all **300 measured steps** with zero Sokol errors and passes the matched CPU screening envelope. Evidence and source/binary fingerprints: `artifacts/v17-speculative-fixed/result.json`. No full-native or performance-win claim, and no new commit.

Next work remains native terrain/constraint validation (including Gear Lift's divergent debris path), world-level speculative control (currently a native stub), compound mesh material forwarding, sensor/continuous events, and Village's shared geometry/capacity architecture. The release Village setup contains 40,000 hulls, 10,000 alternating spheres/capsules and 2,500 mesh instances when the building asset loads. Do not merely raise the 4096-child guard while importing a transformed copy of every building mesh; preserve bounded allocation and introduce shared immutable geometry with per-instance transforms and suitable spatial queries.


### Compound mesh material import repaired

The native GPU and both-mode compound import paths now upload each mesh child's complete material table through `child.materialIndices`, which maps triangle-local indices to the compound's shared material entries. Previously only the first/base material survived: an executed high-friction compound-mesh patch kept a sliding box at 1 m/s, while CPU and standalone GPU meshes stopped it. After repair, the box stops on the high-friction patch; a frictionless control retains 1 m/s. An earlier hull child deliberately permutes the compound material indices, covering both sides of the mapping rather than only an identity table. Material upload occurs at import and adds no simulation dispatch or readback.

Validation: **72/72 native CPU/GPU contact cases**, **162/162 release library tests**. Rebuilt `samples:both` completes Compound/Mesh Tile with two warmup and twenty measured smoke frames, zero Sokol errors. Evidence: `artifacts/v17-compound-material-verified/result.json`. This is material/import verification, not full scene stability or performance certification. Changes remain uncommitted; native compatibility and CPU-win flags remain false.

World-level speculative-control audit correction: in the pinned upstream revision, `b3World_EnableSpeculative` stores `world->enableSpeculative` (`physics_world.c`), and `world_snapshot.c` serializes it, but no collision/solver path reads that world field. Shape-level flags do affect hull/mesh collision. Do not implement an invented GPU-wide collision cutoff to make the native world setter appear functional. World snapshot/recording state remains separate unfinished API work. Gear Lift, Village, sensor/continuous event history, general geometry edits and measured performance still remain open.


### Gear Lift support diagnostics

The recorded Gear Lift failure is now accompanied by a reproducible geometry diagnostic, `scripts/diagnose-gear-support.py`, using vertices emitted by the real native `b3CreateRock` (`c_abi/gear_rock_geometry.cpp`). It validates matching trace settings/steps and derives the 32-point stair profile from upstream source. In the remap-build traces, CPU/GPU show 8/9 body-frame vertex intrusions over 2 cm, peaking around 5.54/5.56 cm; neither has debris centers deeper than 2 cm. Body 175 leaves the positive-z open side before the height screen fails. Source/input hashes and limitations are in `artifacts/v18-gear-support/report.json`.

These samples do not prove whole-hull or swept separation. Native acceptance was not relaxed, and the 9/1/1/1 matrix remains a failure. The next investigation requires identical-state impact replays and complete geometry/support checks before changing any trajectory-based criterion. No runtime physics change or performance claim applies to this diagnostic follow-up.


### Identical-state Gear Lift impact replay

`c_abi/gear_impact_replay.cpp`, `scripts/prepare-gear-impact-replay.py` and `scripts/run-gear-impact-replay.py` reproduce the native terrain/rock geometry with recorded pre-impact states. Velocities are restored after shape creation establishes COM; the runner rejects any initial-state mismatch, missing body/frame, nonfinite value or failed process. Four inputs (CPU frame 26 / GPU frame 45, single rock / all 120 debris) each completed 12 steps on both engines. Initial pose/velocity errors are zero at the recorded precision. Evidence: `artifacts/v18-gear-impact-contacts/runs.json` and `impact-comparison.json`, including contact witnesses, separation, impulses and source/binary hashes. The earlier `v18-gear-impact` batch is rejected as identical-state evidence because shape creation changed its velocities.

The CPU-frame-26 single rock develops 0.449 rad/s angular speed after impact on CPU versus 2.865 rad/s on GPU. CPU retains two penetrating points and GPU one; both retain four points overall. The same starting rock therefore diverges without the mechanism or other debris. This localizes a contact-response discrepancy, but does not yet distinguish candidate generation, clustering/reduction, rest offset, and solver contributions. GPU's streaming deepest-first five-to-four reduction differs from native's full-cluster farthest-pair/area selection with separation hysteresis. Do not blindly transplant a serial whole-world reduction or claim this algorithm difference alone is the cause.

These are cold-start diagnostics: mechanism bodies/joints and old contact caches are omitted. No runtime physics changed in this replay follow-up, no native tolerance was relaxed, and no performance claim was made. Gear Lift, Village and full compatibility remain open.


### Gear Lift mesh witness and reduction repairs (v18)

The opt-in `GPU_PHYSICS_AB=mesh-candidates` trace isolates the missing witness: the GPU generates the native second penetrating point within 2.23 micrometres, then loses it during reduction. It allocates a bounded 2048-record tail in the existing atomic buffer only at world creation; no extra binding or ordinary-step readback is added. Overflow records sticky capacity loss and is rejected by the replay runner. `gpu_b3_world_dump_mesh_candidates` is a diagnostic export, not a viewer path. Evidence: `artifacts/v18-mesh-candidates/comparison.json`.

Hull vertex samples now use hull-surface witnesses consistently with face clipping, avoiding midpoint duplicates and incorrect friction lever arms. Bounded five-to-four reduction uses farthest-pair/area selection with native separation hysteresis, retaining the existing deepest-witness safety invariant for nonplanar inputs. This remains streaming per pair, not native's full-cluster algorithm or a serial scene pass. Native mesh rest offset (one linear slop) is applied after clustering. The slow-recycling fixture now starts its tiny sphere at the same positive solver gap above that offset; all original assertions remain.

Targeted validation: 162 existing release library tests plus the new executed GPU witness regression pass; 72 native contact API cases and Mesh Drop references 233/842 pass. Four recorded states, each with CPU/GPU 12-step replay, retain identical initial pose/velocity and mass/local inertia at recorded precision. Source/binary evidence is in `artifacts/v18-gear-impact-final/` and `artifacts/v18-mesh-final-evidence/result.json`.

The isolated impact's GPU angular speed falls from 2.865 to 2.043 rad/s; CPU is 0.449. Both penetrating witnesses and their separations now match closely, but solver response remains wrong enough to investigate. Disabling continuous collision handling changes neither engine's result in this case. Returning selected points in native order alone also does not resolve it. These are partial corrections, not a native compatibility pass. The complete native matrix, viewer and performance measurements have not been refreshed for this build.

The native convenience getters `b3Body_GetLocalRotationalInertia` and `b3Body_GetWorldInverseRotationalInertia` still resolve to zero stubs in this experiment's replay link; do not infer zero solver inertia from them. `b3Body_GetMassData` is implemented and its mass/local tensor matches CPU in all replay inputs. Implement the convenience getters with current-state/lock semantics and native regressions as part of remaining API parity. Next solver diagnosis should compare prepared inertia, softness, normal mass, friction centres and velocities after each substep against native, keeping the same corrected manifold. Gear Lift, Village, full functionality and CPU-win certification remain open; these changes are uncommitted.


### Native inertia getters implemented

`b3Body_GetLocalRotationalInertia` and `b3Body_GetWorldInverseRotationalInertia` now have real Rust/C ABI implementations in both the GPU and combined viewer bridges. The world tensor is reconstructed from the complete symmetric local inverse tensor and synchronized current rotation. Partial angular locks leave the tensor intact, as native does; static, kinematic and fully rotation-locked bodies return zero. The mass-data getter also stops exposing a nonzero inertia for these bodies. Local mass/inertia reads use host-owned metadata and no longer trigger a GPU mirror download; current world-inertia reads still use the existing synchronized mirror path.

The required `scripts/check-inertia-reference.sh` compares 24 native/GPU cases covering body types, partial/full locks and unlocking, teleport, post-step rotation, and zero mass with zero inertia. All pass; maximum absolute tensor-entry difference is 1.5e-5 on entries around 200 (relative float tolerance). `correctness-gate.sh` now requires this check. The release library suite passes 163/163 and the combined viewer rebuilds. Artifacts: `artifacts/v18-inertia-final/`. These checks establish the specified getter behavior, not equality of actual solver preparation or resolution of Gear Lift's excess spin. Positive inertia with zero mass, broader native scenes and performance certification remain separate work.

The rebuilt combined viewer also completes Gear Lift with 2 warmup and 12 measured frames under the normal NVIDIA launcher settings: pose import succeeds, zero GL/Sokol errors. A direct launch without those settings selected AMD OpenGL alongside NVIDIA Vulkan and failed external-memory import; that run is retained separately as cross-adapter diagnostic evidence, not a renderer pass. This short smoke does not exercise Gear Lift's later impact failure.


### Solver input correction and fresh native screening (September 10)

The isolated Gear Lift impact mismatch is resolved in the executed fixture. WGSL indexed body-extra data using the live body count even though the host packs it after the allocated BodyCold capacity. Reading the actual allocation offset restores off-diagonal inverse inertia and motion bounds. Mesh rolling resistance now uses the native full hull inner radius. The same cold-start impact produces 0.448573 rad/s on GPU versus 0.448577 on CPU; maximum state error is 5.13e-6. The required `check-gear-impact-reference.sh` prevents this regression. This supersedes the earlier unresolved 2.043 versus 0.449 comparison above.

Release library tests pass 164/164, contact API controls 72/72, and mesh references 233/842 pass. The fresh frozen-binary native matrix (`artifacts/native-scene-20260910-v18-solver-frozen-summary.json`) reports 10 pass, 0 fail, 1 unsupported and 1 incomplete. Those passes are unchanged CPU screening envelopes, not complete physical certification. Village remains unsupported and its scene-switch gate incomplete. One earlier Gear Lift step-10 capacity failure has not reproduced in three repeats or the frozen matrix; its cause remains open. Sticky reporting now retains individual failure causes. The earlier solver-fixed matrix is explicitly invalidated because binaries changed during its run.

`cpu_win_validated` remains false. Remaining work includes shared Village geometry, complete support/joint/API checks, zero-mass positive-inertia ownership, the generic convex hull rolling radius, and matched repeated performance measurements. GPU solver tracing is opt-in; normal stepping adds no diagnostic readback.


The post-checkpoint continuation (`d2d6540`) corrects generic convex-hull rolling resistance to use one quarter of the live child's stored inner radius, matching native `contact.c`. Bounds remain appropriate for the experiment's analytic boxes; sphere/capsule radius and full mesh hull radius retain their separate rules. The GPU regression uses two compound children with equal bounds and unequal inner radii. This change adds no buffers, dispatches or host readbacks. Checkpoint recordings/native results precede this continuation; they are not validation of the new shader.

Post-checkpoint rolling-radius validation: 165/165 release library tests pass, including the executed child-radius regression. Evidence: `artifacts/v18-convex-rolling/`. Native/performance results for this continuation remain pending.


### Dynamic zero-mass bodies

Zero mass no longer changes a dynamic body into a static solver endpoint. Host topology proofs, joint ownership, GPU islands, integration, force/impulse APIs and continuous-motion selection now use body type/disabled state. Gravity remains suppressed when inverse mass is zero, matching native. `SetMassData` preserves independently specified rotational inertia instead of clearing its GPU diagonal tensor. This permits assigned motion and rotational response with zero translational mass, including motorized joints; zero inertia still permits explicitly assigned velocity.

The required native `check-zero-mass-reference.sh` executes 124 state rows across angular impulse/torque, off-center linear impulse, zero-inertia assigned motion and a revolute motor. Maximum CPU/GPU state error is 1.6089999999996385e-06. Evidence: `artifacts/v18-zero-mass-final/`; the preceding motor failure is retained under `v18-zero-mass-verified/`. No tolerance was widened. This is a bounded native regression, not full compatibility certification.

The earlier full checkpoint gate reports 32 pass / 2 fail: incomplete Village/native compatibility and a source fingerprint change during the run. It is not a clean aggregate pass. Subsequent source changes require a new frozen run. Village shared geometry/capacity, unexplained intermittent Gear capacity failure, wider API/physics parity and GPU performance remain outstanding; cpu_win_validated stays false.


### Shared mesh snapshots and density controls

Host mesh snapshots now share immutable vertex, triangle, triangle-ID and BVH arrays with their owning shape. Repeated query snapshots no longer copy those four arrays. Replacing geometry publishes new arrays; retained snapshots survive replacement and world destruction. The isolated lifetime regression passes. This is storage ownership groundwork: GPU packing still repeats instance geometry, and Village remains unsupported.

`b3Shape_SetDensity` now updates unit-density mass/inertia caches and optionally recomputes body mass/COM; zero-to-positive density and deferred updates are supported. GPU and combined-viewer C bridges are wired. Related corrections keep zero-mass and static/kinematic centers at the native origin, clear mass/inertia when all shape densities become zero, and make velocity setters respect static/disabled bodies, angular locks and sleep wake semantics. The required native `check-density-reference.sh` passes 30 rows covering sphere, capsule, offset hull, static/kinematic types, deferred/zero/restored density and a subsequent step. Mass/tensor comparisons use relative tolerances; pose/velocity use absolute tolerances. Evidence: `artifacts/v18-density-final/`. The combined viewer rebuilds; a combined-mode behavior run is not yet claimed.

The preceding frozen zero-mass full gate is 34 pass / 1 fail / 0 skip with a stable fingerprint (`artifacts/v18-zero-final-correctness.json`). Its failed native matrix includes Gear Lift's divergent escape trajectory and unsupported Village; later density/storage edits are not covered by that aggregate. The linked-stub audit and executed API controls also expose substantial remaining functionality gaps (`artifacts/v18-linked-stub-audit/`, `artifacts/v18-api-controls-before/`). No full native compatibility or CPU-win claim.

Combined storage/density/velocity continuation validation: 166/166 release library tests pass; the isolated shared-storage suite also passed 166/166. The API control probe now observes GPU mass increasing from ~1000 to ~2000 after SetDensity, matching CPU. Remaining API controls still fail and remain in scope. No new commit.


### Identity accessors and body-slot lifetime

The GPU and combined C bridges implement body/shape/joint world lookup, joint type (translated to native enumeration), and joint endpoints. These and validity/shape-owner reads use host metadata without harvesting pending GPU state. Body slots retain a generation counter across deletion; reuse increments it. Internal shape-to-body paths reconstruct the live generation. Destroying a body now removes its attached joints and rejects stale body IDs, preventing old constraints or handles from acting on a replacement body.

The native identity fixture covers all nine joint types, endpoint generations, world IDs and attached-joint destruction. It is required by `check-joint-metadata-reference.sh`. A GPU regression repeats accessors/validity reads during a pending submit and requires the mirror to stay stale, then checks deletion/reuse and stale-handle containment. These changes do not complete joint setters, per-body sleep APIs, other remaining stubs, Village instancing or performance certification.

Identity/lifetime continuation: 167/167 release library tests and 20 exact native control rows pass; combined viewer rebuild succeeds. Evidence: `artifacts/v18-joint-metadata-final-controls/`. No additional commit or broad compatibility/performance claim.

Pre-commit comparison refreshed on September 10: real Box3D CPU oracle plus `2026-09-10-v18-identity-density` GPU clips (300 frames, all 13 recording scenes). Snapshot metrics use one run for this recording checkpoint and are not a repeated CPU-win benchmark. Native Gear Lift/Village failures remain open.

World maximum linear speed now honors `b3WorldDef.maximumLinearSpeed` and runtime `b3World_SetMaximumLinearSpeed`/`GetMaximumLinearSpeed` in GPU and combined C bridges. Integration reads a per-world uniform instead of hardcoding 400. Configuration access does not harvest the pending body mirror. The 256-byte parameter block retains its size, with an explicit offset assertion for the new field. Required `check-speed-limit-reference.sh` passes 12 native cases (dynamic, kinematic, zero-mass, motion-locked; creation/decrease/increase), comparing actual integrated positions and velocities with maximum absolute difference 1.9e-6. The combined viewer rebuild passes; combined-mode runtime behavior is not claimed. This is a bounded API correction; Gear Lift, Village, remaining stubs, joint collision lifecycle and matched GPU performance still require work. `cpu_win_validated` stays false.

Speed-limit validation completed: release library tests 167/167, pending-state metadata regression pass, and native speed controls 12/12. Evidence: `artifacts/speed-limit-reference-gate/`. No fresh full native-scene matrix or matched performance benchmark was run for this change.

Joint collision controls now implement native `SetCollideConnected`/`GetCollideConnected` in GPU and combined bridges. Filter joints follow their collision flag rather than unconditionally blocking. Disabling collision dispatches a GPU mutation over unique contact roots, retires each matching manifold chain and pair-hash entry, invalidates public contact IDs, and queues end events for the next step. Re-enabling rebuilds filtering inputs; disable/re-enable before a step cannot resurrect an old ID. Metadata reads and no-op setters do not harvest GPU state; actual mutations currently harvest pending state/events first. The mutation is queue-ordered and does not advance a physics step or wake endpoints explicitly.

The required native `check-joint-collision-reference.sh` passes eight observations covering two joints sharing a pair, filter enable, no-op retention, immediate contact capacity/data removal, old-ID invalidation and deferred end/fresh begin events. A GPU manifold regression verifies child retirement, preservation of unrelated roots and fail-closed malformed ownership. Combined viewer rebuild passes. This does not complete joint creation/destruction lifecycle parity, other API stubs, Village, Gear Lift or CPU-win validation.

Joint collision validation completed: 168/168 release library tests, eight exact native lifecycle observations, and combined viewer rebuild pass. Evidence: `artifacts/joint-collision-reference-gate/`. No new full native scene matrix or performance certification is claimed. The previous checkpoint remains `f6aa597`; speed-limit and joint-collision continuation changes are uncommitted.

The GPU now honors the world restitution threshold at creation and through native runtime setters/getters. All restitution solver paths use the per-world uniform instead of a hardcoded 1.0; negative setter values clamp to zero like native. Configuration access does not harvest pending GPU body state. The existing 256-byte parameter block retains its size and asserts the new field's byte offset. Required `check-restitution-threshold-reference.sh` compares actual impact velocity/position below, exactly at, and above the threshold, including runtime changes after GPU initialization. This implements a missing control; it does not establish full scene compatibility or a CPU performance win.

Restitution validation: 24/24 native impact cases pass (maximum absolute state difference 2.36e-10), release library tests 168/168, combined viewer rebuild pass. Evidence: `artifacts/restitution-threshold-reference-gate/`. A fresh full correctness run for accumulated world/joint controls is running with frozen implementation sources; its intended output is `artifacts/v18-world-controls-correctness.json`. Do not treat that aggregate as passed until the process completes and its result is inspected. Preserve all remaining native functionality, Village/shared geometry, Gear support, scheduling and performance requirements.

Frozen-gate investigation found translated/mirrored compound GPU ray failures. An isolated candidate passes 32 native ray observations and 8 support cases; it is not applied to main yet. Evidence/patch/base hashes: `artifacts/v18-compound-mirror-probe/`. The fresh native Gear Lift run also reproduces contact capacity loss at step 10 (`native-scene-20260910-080652`), so its previous intermittent failure remains unresolved.

The translated/mirrored compound mesh candidate has been promoted to main. GPU mesh rays now use body-origin-relative vertices/BVH bounds consistently; baked negative-scale mesh instances retain winding and inverse edge flags. The required release `check-compound-mesh-reference.sh` passes 32 ray observations and 16 support cases (horizontal/oblique, all eight sign combinations, parent/child rotations, public compound IDs, freed source mesh lifetime). Maximum ray difference is 1.49e-7; maximum support difference is 3.58e-7 m. Evidence: `artifacts/compound-mesh-reference-gate/`; promotion is recorded separately from the earlier isolated stage reports.

The preceding frozen full gate completed 39 pass / 1 fail / 0 skip with a stable fingerprint (`artifacts/v18-world-controls-correctness.json`). Native compatibility remained false: Gear Lift stopped at step 10 with one contact loss, and Village remains unsupported. New contact-drop reason bits distinguish 14 allocation/ownership/graph/traversal sites; their map is `artifacts/v18-contact-drop-reasons/reason-map.json`. The existing asynchronous status transfer grows from 24 to 28 bytes, with no extra map, submission, buffer, or scene mirror. Explicit live-stat reads also include this word. Fail-closed behavior is unchanged.

Four subsequent instrumented Gear Lift runs all reached 122 submitted steps and passed the unchanged CPU screening envelope (`artifacts/v18-contact-drop-reasons/gear-runs.json`). This does not resolve the intermittent step-10 loss or certify constraints/support generally. Retain its failed artifact and use the new cause bits on recurrence. Village geometry sharing, remaining API semantics and measured GPU performance remain required; cpu_win_validated remains false. Current continuation is uncommitted.

Post-promotion release validation: 168/168 library tests and combined viewer rebuild pass. Both async sticky-cause reporting and malformed-chain cause reporting are exercised. Evidence: `artifacts/v18-contact-drop-reasons/validation.json`. No process from this validation remains running.


Checkpoint query follow-up: the expanded translated compound-mesh regression now exercises overlap, callback raycast, and mover queries. Removing a duplicate mesh-center offset from host BVH traversal restores all three hits, but the regression still fails public compound identity (GPU returns an internal child ID). `artifacts/v18-host-mesh-before/` preserves the missed-hit baseline and `artifacts/v18-host-mesh-fixed/` preserves this partial fix. These are failing diagnostic artifacts, not a passed gate. Public identity/child metadata is the next correction. The earlier 168-test result predates this host-query change. Full native compatibility and CPU-win validation remain false.


After checkpoint `8b9d0ee`, host query snapshots retain internal collider IDs for geometry/cache updates and carry public compound IDs plus child ordinals separately. Overlap callbacks now run once per public shape; compound AABB queries use union bounds; callback ray/shape casts select the closest child before invoking the user; mover results batch child planes under the public shape (native limit 64). Query groups are cached with topology. CastMover traverses mesh triangles instead of casting against a convex envelope of the mesh. The GPU-only closest-hit path is unchanged.

The required compound query native control matches callback counts, two-plane batching and public identity, plus exact opposite-direction ray fraction 0.25, child ordinals 0/1 and material IDs 100/101. The expanded compound mesh control passes 32 closest-ray observations, 32 host query/refit cases and 16 physical support cases (all mirrored scale signs and both orientations). Evidence: `artifacts/compound-query-reference-gate/` and `artifacts/compound-mesh-reference-gate/`. A separate translated overlap diagnostic exposes a native compound-tree coordinate discrepancy; it is retained in `artifacts/v18-host-mesh-refit/`. The parity refit control uses equivalent origin-relative query coordinates and keeps full hit/ID checks. These bounded controls do not establish full native compatibility, Village support or a CPU performance win.


Post-checkpoint query validation is complete: 168/168 release library tests, combined viewer build, required compound callback/identity control, and expanded compound mesh control pass. The mesh control also executes 16 mirrored/oblique mover casts against native (maximum fraction difference 2.980000000096794e-07). Evidence: `artifacts/v18-query-continuation/validation.json`. No full native matrix or matched performance rerun is claimed. This continuation remains uncommitted after checkpoint `8b9d0ee`; Village, Gear contact-loss diagnosis and wider API/physics parity remain open.


Static-only shape type rules now match native: bodies owning baked compounds or height fields reject dynamic/kinematic conversion before solver ownership changes. Native GPU/combined bridges now honor BodyDef.isEnabled and expose generation-aware Body/Shape_IsValid rather than validity stubs. Required `check-static-shape-type-reference.sh` compares 12 compound/height/sphere cases across enabled/disabled bodies plus stale/null-ID checks following four legal sphere transitions. Evidence: `artifacts/static-shape-type-reference-gate/`; baseline mismatches are retained under `artifacts/v18-static-shape-types/`. Native rejected type changes leak the world's locked flag and refuse teardown; this native defect is documented, not reproduced in the GPU implementation. Full native compatibility and CPU-win validation remain unproven.


Static-shape continuation validation: 168/168 Rust library tests pass; the final native control passes 12 type cases, four legal-transition ID-lifetime checks and four initial awake/enabled combinations. The final C initialization guard only calls SetEnabled when disabling, preserving initially sleeping bodies; the native control and combined-viewer rebuild pass after that guard. Evidence: `artifacts/v18-static-shape-types/validation.json`. These changes remain uncommitted. No new native-matrix or CPU-win claim.


The shape-less timeout was localized with GDB to NVIDIA create_compute_pipeline inside GpuSim::new. An opt-in GPU_PHYSICS_TRACE_PIPELINES trace recorded collide_pairs compilation at 40374.855 ms. It eventually returned GPU kinematic velocities 2/3 versus native 1.91879284/2.80806375; this was pipeline startup plus a real damping mismatch, not a demonstrated GPU queue hang.

Creation damping is now wired through both C bridges. Kinematic velocities receive damping on the GPU, with inverse mass still suppressing gravity; static creation velocities are zeroed like native. WGSL quat_rotate now uses native's cross-product form, avoiding the homogeneous formula's small axis-speed scaling when quaternion length rounds away from unity. Required body-damping native controls pass 36 pose/velocity/rotation observations across creation/runtime settings, all three body types and with/without shapes (maximum state difference 1.91e-6 at unchanged 1e-5 tolerance).

Collision pipelines are lazy and cached per simulation with a mesh-free specialization and a general mesh variant selected from live triangle count. Later mesh introduction selects/compiles the general pipeline even if geometry capacity is unchanged. A geometry switch test passes primitive→mesh→primitive support and compiles two variants; the extended test also removes all supports and verifies free fall/contact retirement. With fewer than two collider slots, no collision pair exists, so only collision dispatch is skipped; allocation, retirement and GPU integration continue. Unexpected nonempty mesh use of the mesh-free variant sets contact-loss reason bit 14. This is not a CPU fallback or a CPU-win claim.

The frozen damping/query checkpoint gate has completed: **43 pass / 1 fail / 0 skip**, stable source fingerprint, with **169/169** release library tests. The native matrix is **10 pass / 0 fail / 1 unsupported / 1 incomplete**: all ten supported scenes pass the existing all-frame CPU screening envelopes, but Village still exceeds the child cap and the successful same-process switch cannot certify an unsupported Village. These envelopes are not full constraint/support proofs; the earlier intermittent Gear Lift contact loss remains unresolved despite this passing run. Evidence: `artifacts/v18-damping-pipelines-correctness.json` and `artifacts/native-scene-20260910-v18-damping-summary.json`. Full native compatibility and `cpu_win_validated` remain false.

A separate executed direct-shape ray probe finds native compound hits (including disabled bodies) while the GPU bridge returns misses: `artifacts/v18-direct-compound-ray-before/`. Next, resolve public compound handles to their children for direct ray queries, preserve child/triangle/material indices, and keep direct disabled-shape queries distinct from world-query filtering. Validate against native before expanding this API path. Village still requires shared immutable geometry with per-instance transforms, bounded packing and lifetime checks, followed by the unchanged full scene and switch.

Pre-commit comparison recordings completed for all 13 scenes, 300 frames each, with the real Box3D CPU column first and GPU label `2026-09-10-v18-query-damping`. All 26 clips have 300 reported video frames. Recording metrics use one run and are not performance certification. Evidence: `artifacts/v18-query-damping-recordings.json`.

After checkpoint `0075c2e`, direct `b3Shape_RayCast` resolves public compound handles in one synchronized lookup, returns the nearest child, and remaps mesh-local material indices into the compound material table. The C and combined bridges retain each child's four-entry material map. Native direct sphere rays return initial-overlap hits at fraction zero, including zero-length interior rays; the direct API now preserves that behavior separately from GPU world closest-hit picking. Direct shape/body snapshots include disabled bodies, while world-query snapshots continue to exclude them. Public compound density reads now use generation-checked metadata without a pose/contact download.

Validation: **170/170** release library tests, combined viewer build, **99** direct native ray cases plus **9** world-filter/density checks, and the existing compound world-query and translated/mirrored mesh controls pass. The direct control covers initial overlap, zero length, surface entry/exit, finite segment misses, parent teleport/rotation, disabled bodies, and child/triangle/material indices (maximum hit-state error 9e-7). The new lifetime regression rejects wrong-generation and destroyed/replaced parent handles. Evidence: `artifacts/v18-direct-ray-validation.json`. Follow-up changes are uncommitted; no new full native matrix or matched performance run is claimed.

Next: audit the combined importer's negative-scale mesh bake against the GPU-only shim before expanding shared geometry. Source inspection shows `both_dual.c` still passes original triangle winding/flags after baking negative scale, while `shim.c` reverses winding and uses inverse flags. This is a source finding, not yet an executed combined-mode failure. Prefer one shared checked importer so these paths cannot diverge, and add an actual combined-mode mirrored-mesh control. Direct compound lookup currently scans host shape slots once; cache generation-safe child membership or use the forthcoming instance index as part of Village scaling. Other public compound AABB/metadata operations, per-triangle user materials in host world callbacks, mesh/mover coverage and full API/physics/performance requirements remain open.

The combined viewer's mirrored-mesh importer now uses the same checked bake as the GPU-only bridge (`c_abi/compound_mesh_bake.h`). Before the repair, real combined-world controls lost support in all four negative-determinant orientations (about -19.76 m); after it, both paths pass all 32 mirrored/oblique ray observations, 16 support cases, host-query/refit cases and 16 mover casts. The test links the real combined and sample APIs and checks CPU/GPU object mappings. It does not substitute render-hook stubs or test only the GPU shim.

Allocation/attachment failure is explicit: either a missing compound parent, a missing child, a rejected child attachment or a failed mesh bake returns a null shape and invalidates the GPU world. Five injected failures plus a successful import/teardown control pass for each backend. The shared bake passes eight orientation and three allocation-failure checks under AddressSanitizer/leak detection. Required gate scripts cover these controls. Geometry is still baked per instance; this change does not claim Village scalability.

Fresh validation after the interrupted build: **170/170** release library tests, **99** direct native ray comparisons and **9** world-filter/density checks, both GPU and combined mesh references, failure controls, and the combined viewer build pass. Evidence: `artifacts/v18-shared-import-validation.json`. These continuation edits are uncommitted. The latest full native matrix remains the checkpoint's 10 supported-scene screening passes plus unsupported Village/incomplete switch; no full-matrix or matched CPU-win rerun is claimed here. Next is owned shared raw geometry with per-instance transforms and deduplicated GPU packing, followed by actual capacity/lifetime/Village/switch tests before raising limits.

Raw mesh-instance core is now implemented in Rust/WGSL. `b3_create_mesh_instance` shares immutable vertex/triangle/triangle-ID/BVH Arcs and supplies independent affine transforms and materials. GPU scene packing deduplicates those geometry ranges by owned storage identity. ShapeGpu is 176 bytes (44 words), with asserted affine-field offsets; legacy baked meshes retain a separate flag-zero interpretation. Host mesh triangles/proxies/BVH queries and GPU mesh vertices/rays/BVH traversal apply the affine transform consistently. Reflections select reversed winding and inverse edge flags without copying shared triangles. Geometry replacement leaves clones and retained snapshots on their original data.

The executed raw-instance regression covers eight nonuniform/reflected instances, public compound child IDs/materials, host/GPU rays and 120-step physical support. Shared GPU **live counts** stay at 3 vertices/1 triangle/1 BVH node; replacing the source introduces a second range (6/2/2), and destroying it returns to 3/1/1 while clones survive. This is a deduplicated live-range proof, not a measured allocated-memory or frame-rate win. Release library tests **171/171** pass, and both legacy GPU-only and real combined mirrored-mesh native controls pass with the new layout. Evidence: `artifacts/v18-instance-core-validation.json`.

Next, connect the common C importer to the raw-instance constructor with an import-local bounded geometry cache, retaining real CPU/combined controls. Check transformed bounds for finite results before exposing the constructor through C; finite inputs can still overflow during affine arithmetic. Avoid material or identity aliasing, temporary source-shape leakage and quadratic pointer lookup. The current native importer still bakes geometry, and Village's cap remains in place. Full native matrix and matched performance have not been rerun for this core layer; compatibility and CPU-win claims remain false. All continuation changes remain uncommitted after `0075c2e`.

Pre-commit comparison recordings for the shared-mesh core: real Box3D CPU first, GPU `2026-09-10-v18-shared-mesh`, 13 scenes each and 300 frames per clip verified with ffprobe. Recording metrics use one trial and do not certify performance. See `artifacts/v18-shared-mesh-recordings.json`. Native compound importer integration remains the next step.

The native GPU-only and combined compound importers now create raw mesh instances through an import-local, bounded hash cache. Each distinct native mesh pointer is copied into one temporary GPU source shape; children retain its immutable geometry, and source shapes are destroyed on success and every subsequent import-failure exit. Native pointers are never retained after the import. Per-child transforms, materials and public compound ordinals remain independent. The old bake helper remains a tested reference utility, not a production fallback.

The instance constructor rejects quaternion-norm overflow and non-finite transformed corners before attaching a shape, and computes bounds without unnecessarily overflowing sums. The cache has a required sanitizer control for repeated sources, allocation/source/clone failures and idempotent cleanup. This does not raise the Village cap or establish a native-frame performance win. Follow-up: measure actual shared geometry/capacity totals on the unchanged Village, resolve broadphase limits and validate Village→Bounce House before lifting support restrictions.

Native raw-instance integration validation: **171/171** release library tests; GPU-only and real combined native references with two shared mesh children; direct-ray parity; explicit import-failure controls; cache AddressSanitizer/leak checks all pass. Evidence: `artifacts/v18-native-instance-validation.json`. Full native matrix, actual Village allocation and matched performance were not rerun. Integration changes remain uncommitted after checkpoint `856c07a`.

Village diagnostics after native raw-instance integration: the unchanged 52,500-child scene imports under the explicit `GPU_PHYSICS_AB=large-compound-import` diagnostic override. Default support remains capped at 4096. Executed allocation telemetry shows 52,501 packed shapes, 2,678 mesh vertices, 4,370 mesh triangles and 1,571 mesh nodes, with a 12,257,472-byte packed scene heap. This is actual shared geometry packing, not the previous derived estimate. The initial idle probe completed three frames without gpu_fail but had no dynamic bodies and therefore does not validate physical support. Its process peak RSS was 1,581,428 KiB including the native viewer and shader compilation, not just scene storage.

Host query work now uses existing BVH candidates for shape overlap, shape cast and mover-plane queries. Snapshots retain collider order and public compound grouping and leave the world lock before callbacks. A 512-child regression checks full-scan hit agreement, candidate count and post-teleport refit. Unchanged body poses no longer force a query BVH refit. Release library tests 172/172 and native compound query / GPU+combined mesh controls pass. An opt-in `GPU_SOKOL_VILLAGE_DROP=1` fixture adds an identical radius-0.5 dynamic sphere at (0,30,0) to the unchanged Village in either native backend; ordinary viewer runs are unchanged.

Next correctness/performance work must retain the full objective:
- Finish the 240-step native CPU/GPU Village drop comparison and same-process Village→Bounce House; record mismatches rather than treating the idle scene as supported.
- Cache public compound bounds and prune mover casts while preserving the native filter callback even when the public bounds overlap but no child does. Do not simply use child candidates and silently drop that callback.
- Profile startup: C hull/mesh/height mirror lookup currently scans fixed 65,536-slot arrays. Replace with a generation/world-safe bounded lookup only after lifetime and multiworld controls; do not assume this explains all shader startup time.
- Callback ray reentrancy is repaired using owned BVH candidate snapshots. Keep native controls for callback state reads, nested closest/callback rays and clipping. Arbitrary mutation/destruction during native callbacks is not claimed as a supported contract.
- Validate actual dynamic broadphase insertion/pair/contact usage before raising the compound cap. A static-only Village cannot prove those capacities fit, and the serial large-static scan is not an effective final GPU broadphase.
- Full native scene matrix and repeated matched performance remain outstanding; compatibility and cpu_win_validated remain false.

Bounce House CCD correction: fresh and switched GPU traces were identical, ruling out scene-switch lifetime as the cause of the lost tangential step. Conservative advancement and root refinement now target the actual TOI separation instead of the outer tolerance shell; initial-contact tolerance and conservative iteration-exhaustion handling remain. The failing wall regression returned fraction 0.7018745 before the fix versus native target 0.7025. The rotating Mesh Drop test formerly demanded fraction <0.01; an executed native `b3TimeOfImpact` probe returned 0.0439814366, so that assertion now checks the native-backed interval and positive target separation instead.

Validation: 173/173 release library tests; native Bounce House 120-step maximum position difference 0.00063229 m and no lost tangential step; 180-step native Mesh Drop passes the existing CPU screening envelope (CPU minimum y -0.4174, GPU -0.4111). Evidence: `artifacts/v18-ccd-target-validation.json` and `artifacts/v18-ccd-target-oracle/`. Bounce House raw harness status is still incomplete because the generic 120 m/s health limit rejects the native 169.7 m/s initial velocity in both backends. Do not mistake the separate trajectory comparison for a repaired general harness.

Next: calibrate scene-aware health validation using the executed native reference; replace the misleading unpopulated GPU contact counter; address large-static GPU scheduling and remaining Village query/startup costs. Also inspect non-closing fixed-orientation sweeps near the tolerance shell so conservative iteration exhaustion cannot unnecessarily truncate tangential motion. Full functionality, native compatibility and CPU performance certification remain incomplete. Changes remain uncommitted after 856c07a.

Callback ray continuation after the laptop restart: the corrected C probe timed out (exit 124) at its first body read before the fix. The earlier interrupted probe used a missing GPU C helper and is not deadlock evidence. Callback rays now capture only owned BVH candidates, preserve collider/public-child order, and release the world mutex before exact traversal and external callbacks. Immutable mesh geometry remains shared. Query profiles start fresh, exclude callback duration from exact-test time, and publish through generation-checked world lookup; the last completed query owns the profile after nested queries. Blocking GPU closest-hit picking is unchanged.

`scripts/check-query-reentrancy.sh` runs bounded native CPU and GPU controls for callback body reads, nested closest-hit and callback rays, and continue/terminate/ignore/clip responses. It is a required correctness-gate entry. The owned-snapshot regression checks candidate pruning, public compound grouping, refit and survival after index destruction. Arbitrary mutation/destruction inside native callbacks and a GPU performance win are not claimed.

Callback-ray validation: **174/174** release library tests pass; bounded native CPU/GPU reentrancy and native compound-query reference controls pass. Evidence: `artifacts/v18-query-reentrancy-validation.json`. Full correctness gate/native scene matrix and performance were not rerun for this slice. Remaining priorities are scene-aware Bounce House/switch health validation, truthful contact telemetry, near-shell non-closing CCD sweeps, and large-static GPU/query scheduling. Changes remain uncommitted after `856c07a`.

Native scene-health validation now records the actual scene on every frame, including after switches. Bounce House alone permits up to 180 m/s because its native initial velocity is (120,0,120); validation additionally requires one dynamic sphere in the native enclosure at y=4 and speed within 0.05 m/s of sqrt(2)*120. Other scenes retain the 120 m/s screen. The GPU hook derives scene health from finite/pose/speed measurements instead of inheriting the Rust scanner's generic 120 m/s explosion label. The engine scanner itself is unchanged. The same-process gate now requires complete phase identities, 8 Village steps and a reset to Bounce House steps 1–20; merely counting a switch is insufficient.

Fresh CPU/GPU correctness probes pass: Bounce House 120 steps, maximum position difference 0.00063229 m; same-process Village→Bounce House, maximum post-switch position difference 0.00004196 m and velocity difference 0.000412 m/s. Eleven negative controls reject missing frames, wrong identities/step reset, incomplete status, capacity loss, an empty refused Village, non-finite state, energy loss, enclosure escape, tangent trajectory error, and applying Bounce House's speed exception to Events/Hit. The first recording attempt had malformed scene-field JSON and is retained as invalid evidence under `artifacts/v18-scene-health`; only `artifacts/v18-scene-health-verified` supports these results. Aggregate/source fingerprints: `artifacts/v18-scene-health-validation.json`.

Reproduce after building both viewers with `python3 scripts/scene-health-probe.py <fresh-output-dir>`. The script now uses ordinary imports, hashes binaries before/after each run, checks matched CPU trajectories and runs negative controls. The historical 4096-child cap has been raised; the 8 static Village frames do not certify settled dynamic support or large-scene performance. Full native matrix and CPU-win certification remain outstanding. Next: replace unpopulated contact telemetry, prove non-closing near-shell CCD sweeps cannot freeze tangential movement, and improve large-static GPU scheduling/query costs. No commit or push.

Near-shell CCD continuation: a fixed-orientation sphere sweep parallel to a wall at core separation 0.496999741 incorrectly exhausted 512 conservative-advancement iterations and returned a false hit at fraction 0.51194763. Executed native Box3D reports separated at fraction 1; a nearby closing control reports a real hit at fraction 0.00334964064. The engine now verifies support extrema on a single separating axis at both endpoints. For exactly fixed orientations (including opposite quaternion signs), support points translate linearly, so separation beyond the TOI target plus a scale-aware roundoff margin proves the entire sweep is separated. Approximate GJK distance alone is not trusted as that proof. Any real rotation, including one small enough for acos(dot) to round to zero, retains ordinary conservative TOI. Unresolved iteration exhaustion remains conservative.

Validation: **175/175** release library tests; regression controls for reversed body order, common translation, quaternion signs, genuine closing motion and tiny rotations; fresh native Bounce House 120-step comparison passes (max position difference 0.00063229 m); native Mesh Drop 180-step CPU screening envelope passes (CPU min y -0.4174, GPU -0.4111). Evidence: `artifacts/v18-ccd-parallel-validation.json`, `artifacts/v18-ccd-parallel-oracle`, and `artifacts/v18-ccd-parallel-native`. This is a host CCD correctness/work-reduction repair, not a measured native-frame speedup or a migration of CCD to the GPU. Full native compatibility and cpu_win_validated remain false.

Next performance/telemetry action: `b3_world_counts` currently uses `with_world`, which calls `ensure_cpu_mirror_world` despite body/shape/joint counts already being host topology. Remove that unnecessary synchronization with a copy-count/no-harvest regression, count only live joints, and provide contact telemetry with explicit submitted/completed step identity. Do not replace the current fake-zero contact count with a stale value presented as current. Then use truthful capacity/workload evidence to continue large-static GPU scheduling and Village support. No commit or push.

World topology counters now use a no-sync world lookup: body/shape/joint reads do not download the body mirror, harvest CCD/events, or mark a physics submission complete. Public compound parents count once and hidden collider children do not inflate shapeCount; JOINT_NONE holes do not inflate jointCount. Both regressions failed before the fix. Validation: **177/177** release library tests, including 1000 repeated reads preserving the exact pending state, and native CPU public-count comparisons through the same GPU FFI used by samples_api.c. The fixture explicitly links the GPU joint-destroy FFI because that C wrapper resides in samples_api.c rather than the core shim archive. The bounded `scripts/check-world-counters.sh` is a required gate entry. Evidence: `artifacts/v18-world-counters-validation.json`. No native-frame speedup is claimed without new measurements.

Contact telemetry audit: `LiveStepStats.live_contacts` currently reads SCR_NCONTACTS, which compact_unique_bases assigns the bounded unique candidate-pair count. It is neither active patch count nor native contactCount. Native Box3D contactCount is the live contact-ID pool size, including non-touching allocations; compounds introduce additional public-versus-child identity requirements. Do not expose candidate pairs as contact counts. Next implement distinctly named GPU metrics (candidate pairs, occupied roots, active patches, unique public contacts), return submitted/completed sample identity and topology revision, and add fresh no-contact/compound/multi-patch/retirement controls. Reuse the existing compact async status readback where possible, with explicit unknown/stale semantics. A synchronous public native counter must not pretend an older async sample is current. The current samples_api.c contactCount zero remains unimplemented, not validated evidence. Native functionality, Village support, large-static GPU scheduling and CPU-win certification remain unfinished. No commit or push.

GPU contact metrics core now reuses the occupied-contact collection pass to report distinct unique candidate pairs, allocated roots, allocated root/child manifold slots and touching roots (including sensors). Per-workgroup sums limit the added global metric atomics to two per workgroup. The existing asynchronous sticky-status readback grows from 28 to 44 copied bytes; no extra compute dispatch, body download or contact-buffer download is added. Regular and callback-finish submissions share status capture/mapping. A snapshot includes its full u64 submitted physics step and capacity-loss flag; a busy status slot may return an older completed snapshot, never relabeled as the newest step. The measurement phase is contact scheduling before solve/integration and host CCD, not a claim about post-CCD geometry.

Misleading names corrected: LiveStepStats.live_contacts is now narrowphase_pairs, and the benchmark workload JSON active_contacts key is now narrowphase_pairs. Historical artifacts retain their original schema and must not be reinterpreted as actual touching-contact measurements. Native Box3D public contactCount is not implemented by these diagnostics. Native compound child identity and mutation handling must be preserved before using these diagnostics for the native counter.

Validation: **178/178** release library tests and release build pass. An expanded targeted GPU test also passes with the only occupied root in the tail of the third workgroup. Controls distinguish 3 (then 130) unique candidates from 1 root, 2 manifold slots and 1 touching root; compare allocation totals with actual contact storage; verify initial unknown, retirement to zero, no pose maps, a busy-slot stale snapshot, and step IDs above 32-bit range. Evidence: `artifacts/v18-contact-metrics-validation.json`. No native matrix or performance run was repeated for this slice; no CPU-win claim.

Next: expose metrics through a world-generation/topology-aware API and viewer fields with explicit unknown/stale status, replace the viewer's unpopulated zero contact display, and implement native allocated-public-contact semantics separately. Validate ordinary and callback paths with actual compound/multi-patch scenes, mutation/retirement and same-process world reuse before making support/capacity claims. Large-static GPU scheduling, Village support and full native parity remain unfinished. No commit or push.

World-aware contact metrics are now exposed through `b3_world_contact_metrics` and the nonblocking `gpu_b3_world_contact_metrics` FFI. Each captured status slot preserves the topology/state revisions from submission alongside its full step ID. The result includes snapshot/current revisions and steps, known/current/capacity-loss flags, and distinct GPU lifecycle counts. World-generation lookup rejects destroyed IDs. Teleports, geometry changes, retirement and new submissions cannot relabel an older snapshot as current. Host CCD corrections now advance query_state so both query-index freshness and scheduling-snapshot invalidation reflect corrected poses. Diagnostics may explicitly wait for the compact status; the viewer only polls and does not harvest CCD/events or download bodies.

The native GPU/combined sidebar shows pending or older-state contact metrics and separate roots/manifold slots/touching roots/candidate pairs. Per-frame benchmark JSON includes `gpu_contact_metrics` with phase `contact_scheduling`; `contact_count_known` is false in GPU/combined mode because native allocated-public-contact semantics are still unimplemented. CPU mode retains its real native contact count. C/Rust layout assertions cover the shared 80-byte result struct. Do not interpret current scheduling metrics as post-CCD collision geometry.

Validation: **180/180** release library tests; ordinary and custom-filter callback stepping, no-harvest state, teleport/topology changes, shape retirement, world destruction/reuse, and an actual fast-impact CCD correction invalidating its prior scheduling snapshot. Release CPU/GPU/both viewers rebuild successfully. Fresh 32-frame High Resistance probes pass in all three modes: GPU/both each report 31 known snapshots and peak 10 roots, with initial unknown kept explicit. Reproduce after building viewers with `python3 scripts/probe-contact-metrics.py <fresh-output-dir>`. Evidence: `artifacts/v18-world-metrics-validation.json` and `artifacts/v18-world-metrics-native`. These are plumbing/correctness probes, not latency certification or a full native matrix rerun. No commit or push; future visual/solver commits still require the pre-commit comparison recordings.

Next: implement native public allocated-contact counts with correct compound-pair identity and independent CPU controls, rather than aliasing GPU roots or touching patches. Then use current, valid capacity metrics for large-static GPU broadphase/query work and removal of Village's default support restriction. Full Box3D functionality, native scene coverage and CPU-win validation remain unfinished.

Native contact allocation semantics are now backed by an executed C reference, correcting the earlier suggestion to deduplicate all public shape pairs. Native `b3CreateContact` assigns separate contact IDs to compound children (`childIndex` participates in the contact key); mesh manifold patches belong to their root contact. Sensors do not enter the native contact pool. Non-touching allocations still count. Teleporting preserves allocations until the next step, whereas destroying a shape removes them immediately. Preserve compound child identity; do not collapse distinct child contacts into one public-pair count.

The GPU scheduling metrics now include `non_sensor_roots`, excluding sensors, bounds-only public proxy shapes and unsupported mesh/mesh pairs while retaining non-touching allocations. This uses the existing occupied-root pass and per-workgroup reduction; the shared status copy is now 48 bytes, with no additional compute dispatch. The C result remains 80 bytes by using the prior trailing padding; a new offset assertion covers non_sensor_roots at byte 76. The viewer and benchmark JSON label this as a scheduling-phase diagnostic, not the native public world counter.

Validation: **180/180** release library tests; rebuilt CPU/GPU/both viewers and fresh 32-frame High Resistance probes pass, with GPU/both peak 10 non-sensor roots. `scripts/check-contact-count-semantics.sh` is a required gate: 48 CPU/GPU registry lifecycle observations and 24 current post-step GPU count observations match, including actual allocated non-touching contacts verified through GetContactData. Evidence: `artifacts/v18-native-contact-semantics-validation.json`. Runtime CPU fallback was not added. Full native compatibility and CPU-win certification remain false.

Next exact counter implementation must handle immediate mutation without downloading the entire world. Add an on-demand parallel GPU reduction over allocated root slots, excluding child manifold slots, sensors, unsupported pairs and retired logical shape identities. Supply generation-checked shape liveness/retirement metadata from host topology (small uploads only on mutation); keep each compound child distinct. Use a dedicated scalar result and cache it by submitted physics step plus contact-allocation revision. Teleports must not incorrectly retire allocations; shape/body destruction, filter changes and joint collision vetoes must invalidate/update the result immediately. Extend the native reference with those mutations and slot reuse before wiring b3World_GetCounters.contactCount or setting contact_count_known=true. Do not route ordinary HUD reads through the full CPU pose/contact mirror. Then proceed to large-static GPU broadphase/query scheduling and Village support. No commit or push.

The measured Vulkan command-cache path is now available as an opt-in source build: run `./scripts/build-native-cache.sh`, then `./scripts/run-native-cache.sh --scene mixed-stacks` from `experiments/gpu-physics`. Backend preparation is fingerprint-checked and uses a separate generated workspace/lockfile; the ordinary Cargo build remains available. See [`compiler/native-backend/README.md`](compiler/native-backend/README.md). This integrates existing measured work; it does not establish a new CPU win.

For this Linux hybrid laptop, `./scripts/run-split-adapter.sh --scene mixed-stacks` enables NVIDIA physics with AMD rendering on the laptop display (build with `./scripts/build-native-cache.sh` first). Dominoes and mixed-stacks use two staged snapshots and render the previous physics step while the next step runs. This adds one physics step of display latency; CPU comparison uses the same delay. Other scenes select the existing NVIDIA renderer, and scene changes recreate the rendering resources. The launcher selects X11 and both NVIDIA/RADV ICDs; override their paths with `GPU_PHYSICS_SPLIT_ICDS` if needed. It requires both adapters for the optimized scenes and is not a general cross-platform default.

The final source and executable now pass correctness, lifecycle validation, and five fresh matched CPU/GPU trials per scene on AC. Mixed-stacks GPU/CPU cadence p50 is 0.489/0.661 ms and p95 1.983/3.699 ms; Dominoes p50 is 1.885/3.687 ms and p95 3.562/9.284 ms. These are medians of five trial percentiles, with no discarded trials; one mixed-stacks pair was GPU-slower and remains included. Both engines use equal rendering delay, 2000 warmup / 20000 measured steps, no sleeping, and final-drain work charged to cadence. The scoped `cpu_win_validated=true` audit, 185 verified source hashes, full 55/0/0 gate, and final-binary lifecycle results are in [the qualification record](artifacts/gear-clipped-support-final/README.md). This does not establish a single-adapter or universal scene performance win.


A packed small-root follow-up is archived in `artifacts/packed-small-roots`: GPU activation was confirmed (300 mixed-stacks components), but solve p50 remained 0.2232 ms in its paired screen. The candidate was reverted; no frame or CPU-win claim follows.

A two-body/four-contact small-partition screen also left the combined solve-stage p50 unchanged at 0.2232 ms and was reverted (`artifacts/small-component-partition`). That timer includes component-list construction, not only constraint math; a subsequent count/scatter grid-stride screen regressed the stage slightly and was reverted (`artifacts/component-grid-stride`). A focused timestamp split (`artifacts/component-stage-split`) attributes roughly 0.19 ms to solver kernels and 0.03 ms to list construction, with only 0.001 ms total-device change in that diagnostic screen. Solver execution remains the larger target.

A follow-up sweep of small-component workgroups 1/2/4 was slower than 16 (`artifacts/subwarp-component-groups`); the experimental sizes were reverted. The native launcher now honors explicit supported workgroup-size overrides, while retaining 16 as its default.

The [current unused GL export retest](artifacts/current-unused-pose-export/README.md) failed: three paired trials with equal solver work worsened mixed-stacks median and tail. Source and binary were restored; no default change or CPU win.

The [scene render-bundle screen](artifacts/scene-render-bundles/README.md) passed its GPU pixel/shadow check but failed to improve mixed-stacks whole-frame timing in three alternating pairs. Candidate source and executable were reverted; repeated draw-command setup is not a demonstrated route to the target.

The [prepared twist-mass experiment](artifacts/prepared-twist-mass/README.md) preserved all three tested600-step checkpoint dumps and passed component regressions, but its three-pair frame screen worsened mixed stacks. Shader and executable were restored; no performance gain retained.

The [current acquisition-fence retest](artifacts/current-semaphore-acquire/README.md) passed its bounded Vulkan smoke but failed the five-pair frame screen. Removing the host acquisition wait shifts cost toward presentation without improving mixed stacks; ordinary backend and executable remain unchanged.

The [fixed-index normal-point screen](artifacts/normal-point-unroll/README.md) preserved the tested states and passed Vulkan validation, but did not improve mixed-stacks frame medians. Its loop expansion was reverted; the corrected prior solver remains active.

The [current long Bridge comparison](artifacts/bridge-current-long/README.md) passes the unchanged CPU-envelope screen for1202 steps, with150 bodies and302 joints recorded every measured frame. No explosions/nonfinite states; worst anchor-error excess over CPU is2.9mm. This extends stability evidence, not performance qualification.

The [long Ghost Collisions check](artifacts/ghost-current-long/README.md) passes1202 steps without late sinking, nonfinite state or ghost launches under the sample's0.5m/s threshold. Final height matches within a micrometre; sideways trajectories differ, so this is bounded stability evidence rather than exact CPU parity.

The [packed normal-impulse layout](artifacts/packed-normal-impulses/README.md) passed27 focused tests and exact600-step checkpoints, but a noisy three-pair frame screen did not establish improvement and had worse aggregate tails. The layout and executable were restored; no CPU-win claim.

### Gear Lift first-contact correction (after checkpoint 60a1e73)

The remaining long-run discrepancy is being traced from the first step. A CPU one/eight-worker control matches body positions exactly through step 152. A two-rock replay isolates an extra GPU hull-face selection bias that changes the normal for deeply overlapping rocks; using the upstream hull-hull selection rule corrects that normal. Projected contact points are also deduplicated in the same coordinate representation, preventing repeated constraints on the same point. Evidence and current validation limits are in `artifacts/gear-first-contact/README.md`. Older Gear Lift artifact prose incorrectly calls its seed 52977: that environment override only affects Mesh Drop, while Gear Lift uses upstream seed 12345. No tolerance or scene default is changed; long-run and final gate qualification remain open.

The combined contact correction passes 24 manifold regressions and the isolated normal/unique-point check. The 1202-step ordinary Gear Lift run still fails the unchanged CPU envelope at step 139 body 123; it is not a scene-level pass. Angular constraint error improves, but peak speed and anchor-error excess increase. The remaining rock manifold uses three unique GPU points versus four CPU points, giving a concrete next reduction/coverage investigation. See `artifacts/gear-first-contact/gear-result.json`; full-gate and CPU-win claims remain false.

### Hull face reduction coverage

Hull manifolds now retain all vertices when the clipped polygon has at most four points, and use Box3D's depth/distance/area selection for larger polygons. The captured Gear Lift pair improves from three unique GPU contacts to four, matching the CPU normal and separation set. A rotated octagonal pair exercises reduction of larger polygons. Full Gear Lift still fails the unchanged trajectory screen at step149/body115; no scene-level pass is claimed. See `artifacts/gear-manifold-reduction/README.md` for evidence and remaining limits.

Validation for this reduction slice: all 25 manifold regression tests pass, including the new polygon-coverage test. No commit or full-gate/CPU-win claim.

### Hull-hull edge witnesses

A same-pose rock pair exposed a false contact: face axes overlapped, but a separating edge axis had a 3.63 cm projected gap. Hull-hull collision now checks Gauss edge pairs and constructs actual segment witnesses when the edge contact wins. The old face-only path is removed for hulls with topology. Three CPU/GPU C ABI regressions (separated pair, edge contact, nearby face contact) pass and are required by the correctness gate. Full scene results and limits are recorded in `artifacts/hull-edge-separation/README.md`; final full-gate and performance qualification remain open.

The hull-edge slice passes 25 manifold regressions plus the three required CPU/GPU edge fixtures and the octagonal face replay. Gear Lift still fails the unchanged trajectory screen (step107/body94). Peak speed and joint anchor-error excess decrease, but no overall scene pass is claimed. A vertex/volume check places the flagged rock above the lower stair tread at that frame; it does not certify every edge/face or swept collision. The next diagnosis must distinguish descent along the stairs from missed support, without hiding genuine contact defects.

### Gear Lift CPU sensitivity control

A fresh CPU-only control shows that moving each rock's initial x by one float32 step is enough to fail the unchanged 2m trajectory envelope at step115/body144. Both CPU runs pass finite/runaway checks. The generated diagnostic source and ordinary CPU executable are restored; production sample/validator settings are unchanged. This establishes that trajectory divergence alone cannot diagnose this pile's GPU support failures. `artifacts/gear-cpu-ulp-control/README.md` records the matched runs and their limits. Direct full hull/stair intersection and ground-support checks remain necessary before resolving Gear Lift; the contact fixes and their regression gates remain in place.

### Persistent Gear Lift mesh penetration

A complete convex-hull/prism SAT analysis of all recorded rocks finds persistent GPU stair intersections, distinct from the sensitive trajectory screen. A one-rock replay at the final body140 pose reports CPU penetration of 6.13cm, while GPU reports only 0.10mm with an incorrect upward normal. The generic hull/triangle path lacks the CPU's full face/edge witness selection. Evidence is in `artifacts/gear-solid-support/` and `artifacts/gear-mesh-rest-witness/`; this is the next concrete collision fix. No production tuning or gate changes were made during diagnosis.

The face/edge witness implementation now reproduces the saved resting-rock CPU manifold. A fresh 1,202-step Gear Lift run reduces independently measured >2cm hull/solid intersections from 6,140 to 10, with no >5cm or final-frame >2cm intersections. The trajectory envelope still fails and remains unchanged. The permanent hull/mesh CPU regression is required by the gate; full scene acceptance and performance qualification remain open. See `artifacts/hull-triangle-witness/README.md` for exact evidence and the remaining fallback limitations.

Fresh matched 1,202-step Ghost Collisions and Mesh Drop runs pass the unchanged native scene screens with the hull/triangle witness change (`artifacts/hull-triangle-scenes/`). Gear Lift joint errors also remain within the existing limits over all 1,200 measured frames. Final full-gate and performance qualification remain outstanding.

Generic hull/triangle incident-face selection now follows the native support-edge rule. A CPU-generated face-6-versus-face-7 regression and all 26 manifold tests pass; the saved mesh-rest comparison also passes. The fresh Gear Lift run retains the penetration improvement (eight >2cm poses, none >5cm or deep at the final frame), while the unchanged trajectory screen still fails. Evidence: `artifacts/hull-incident-face/`.

The full hull-contact milestone gate completed with **54 pass / 1 fail / 0 skip** and 222 passing library tests. The only failure is the native Gear Lift trajectory screen; all other native cases, ordinary Village support/switching, 3,600-step runs and final CPU oracles pass. Sources stayed unchanged during the gate. Complete evidence: `artifacts/milestone-hull-witness/`. Production acceptance is not yet changed and `cpu_win_validated` remains false.

The prepared world-inertia candidate was rejected: it reduced the measured solver stage slightly but worsened total device and completed-step latency in both paired mixed-stacks screens, and exceeded the existing High Resistance position threshold. Original sources and the fresh baseline native executable are restored. Evidence: `artifacts/prepared-world-inertia/README.md`. No CPU-win claim.

The fresh hull-contact/Gear Lift support milestone passes **55/0/0**, including 222 library tests, all 12 native matrix cases, ordinary Village and switching, three 3600-step runs, and final CPU comparisons. Gear Lift now uses a 1200-step independent hull/solid support check with joint, phase and motion bounds; the earlier CPU-sensitive per-rock trajectory failure is superseded, not silently tolerated. Source fingerprint stayed stable. Evidence and limits: `artifacts/milestone-gear-support/`. Latest matched frame trials still fail the mixed-stacks median target; `cpu_win_validated` remains false.
