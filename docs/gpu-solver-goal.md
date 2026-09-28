# GPU solver goal and recovery scratchpad

Updated: 2026-09-28. Status: active, incomplete.
Workspace: `/home/frtn/work/2026/box3d-wasm`.
Engine working directory: `experiments/gpu-physics` relative to that workspace.

## Read and maintain this file

Read this file at the start of each goal continuation and after compaction or restart. It contains the complete objective and current recovery context; no earlier conversation is required. Verify recorded facts against the worktree, artifacts and actual process state before relying on them. Update current evidence, decisions, jobs and next actions after meaningful progress and before handoff. Adapt implementation plans when evidence warrants it; do not silently narrow the objective, weaken acceptance limits or erase failures. User instructions take precedence.

Detailed evidence and historical investigations belong in [gpu-solver-qualification.md](gpu-solver-qualification.md). Generated evidence is under `experiments/gpu-physics/artifacts/gpu-solver-qualification/`; its `checkpoint.json` is a machine-readable evidence index. This file owns the current goal and recovery plan; the qualification document owns detailed results. Keep status summaries consistent.

## Portable checkpoint handoff (2026-09-28)

The user authorized committing and pushing this checkpoint. The solver goal remains incomplete. On the source machine, process inspection now confirms no qualification runners, samples or episode extractors remain. Old process handles below are historical. Ordinary Rain run1 has a `complete.json` receipt and compressed state; run2 was interrupted with only a partial raw trace. Native run1 remains a failed launcher attempt, not a completed repeat. Both ordinary episode extractions now have finalized output and manifests. No five-run Rain qualification is complete.

On another machine, resolve the repository root locally instead of using the source workspace path above. Read this file and the qualification report, inspect Git state and rebuild. `artifacts/`, `recordings/`, binaries, pipeline caches and session handles are local/ignored and do not transfer through Git. Historical results in the report are reported evidence, not independently reproduced results on the new machine. Missing raw evidence stays unavailable unless explicitly transferred or regenerated. Never count a missing artifact as a pass. The user-level goal-scratchpad skill also lives outside this repository; this file and the AGENTS.md pointer provide the necessary workflow without it.

Compact historical receipts and pre-commit recording provenance are tracked in `experiments/gpu-physics/benchmarks/2026-09-28-solver-checkpoint.json`. These do not substitute for missing raw captures or final qualification.

The Rain episode helper sources are now preserved in `experiments/gpu-physics/scripts/rain-episode-diagnostics/`. They select historical diagnostic windows; a new device may have different trajectories and event locations. Preserve same-device repeatability, not cross-machine agreement. First correct the runner's child-exit receipt handling around `xvfb-run` cleanup failures, with a bounded regression check, before launching another long batch. Do not rewrite failed receipts. Then finish the remaining physical criteria and persistent-state audit before freezing the final candidate and running the fixed matrix. Known restitution and contact-island symmetry screens still fail on both CPU and GPU; do not describe the full suite as green.

## Full objective

Make the GPU solver correct, stable, and repeatable using its own efficient constraint ordering. Exact CPU/GPU trajectories and resting poses are not required to match. Establish CPU-comparable physical capability within an explicitly documented qualification matrix, fix GPU defects, measure performance, and make efficient GPU ordering the normal mode only after qualification passes. Keep CPU-compatible ordering available for debugging and reference work.

### Determinism contract

Identical build, configuration, device, driver/backend, initial state, creation order, timestep/substeps and scripted inputs must produce identical semantic simulation results across fresh processes. Qualify ordinary and native cached GPU paths independently. No promise is made across devices, platforms, drivers, builds or configurations.

Compare relevant complete semantic state at every physics step, including poses, velocities, sleep state, contact geometry and impulses, joint configuration and accumulated impulses, lifecycle relationships, and persistent solver state that affects later results. Exclude padding, timestamps and incidental identifiers only with a documented audit. Preserve relational identities and any ordering that can affect future simulation. Pose checksums alone are insufficient.

### Physical correctness contract

Preserve upstream scene parameters, joint types, geometry and physical defaults. Validate joint constraints, collisions, friction, restitution, mass/inertia, sleeping/waking, motors/limits, CCD, dragging and body/shape/joint lifecycle operations. Detect missed collisions, persistent penetration, broken constraints, unexplained energy gain, invalid state and stale data after deletion/reuse. Preserve existing analytical, isolated, collision-free and numerical regression checks.

Use CPU as a capability and behavior reference, comparing constraint error, penetration, energy, settling, supported loads and scripted responses. Prefer analytical expectations when available. Define justified acceptance limits before evaluating a candidate fix. Allow different valid chaotic trajectories. Retain strict CPU comparisons where equivalent arithmetic and ordering make them meaningful. Unexplained instability cannot be called intentional divergence.

For each outstanding Rain failure, establish whether it is a GPU defect, behavior reproduced under equivalent CPU conditions, or an unresolved discrepancy. CPU reproduction alone is not a pass: explain whether behavior satisfies a physically justified criterion. Preserve failed original screens even when a better-founded criterion is proposed; do not broaden limits just because a candidate fails them.

### Qualification and practical execution

Start with Falling Ragdolls and Rain, including Rain joint errors and identity correspondence across recycling. Cover representative stacks, mechanisms, motors/limits, CCD, dragging, sleeping/waking and runtime creation/deletion/replacement.

Freeze the qualification matrix into named scenarios, commands, configurations, durations, physical limits and evidence requirements before another broad qualification campaign. The capability list below is the fixed scope; exact fixture selection and uncovered acceptance limits still need consolidation. Add tests only to cover an explicit missing requirement or reproduce a concrete defect, not speculative expansion.

Require at least five fresh runs per selected configuration. Make complete per-step comparison practical with compact lossless records or exact-bit hashes over audited semantic state, with detailed captures around discrepancies. Validate compact output against existing full captures and deliberate corruptions before relying on it. Hashing does not justify omitting state. Diagnose the earliest mismatch and fix its underlying race, ordering, initialization or stale-state cause.

Freeze candidate binaries and record hashes, configuration, device/backend and commands. Preserve completed runs across interruption; replace only incomplete runs. Do not mix different builds/configurations in one qualification batch. Existing partial traces are diagnostic evidence, not completed repeats. Avoid repeatedly launching multi-gigabyte full captures before addressing their runtime/storage cost.

Prioritize closure: close known failures, complete the fixed matrix on the candidate, measure performance, record comparisons, then decide the default. Add instrumentation only when it answers a specific unresolved requirement or discriminates a live hypothesis. Report closed gates and remaining failures; no unsupported completion percentages.

### Performance and delivery

Benchmark representative small and large scenes under controlled conditions, with both GPU paths and GPU-native versus CPU-compatible ordering. Exclude state/health/lifetime tracing and other diagnostic overhead from performance measurements. Record warmup, timed duration, repetitions, hardware/software configuration, completed-step timing and variability. Establish the performance acceptance decision before judging the candidate; no arbitrary speedup is promised without a baseline.

Publish the coverage matrix with explicit passes, failures and unsupported capabilities. Scope “equally capable” to that matrix. Document intentional trajectory differences separately from known defects. Record representative repeated GPU runs and CPU/GPU behavior comparisons. Do not hide failures behind averages, weaken checks or infer universal correctness from a few passing scenes.

Completion requires every required gate below to have current, directly applicable evidence, including performance, recordings and the default-mode decision. Unsupported capabilities must be explicit and consistent with the agreed scope; labeling a required capability unsupported does not complete it. Keep the goal active while required work remains.

## Acceptance gates and current evidence

All paths below are relative to the generated evidence directory unless stated otherwise. Historical passes must be checked for applicability to the final frozen build.

| Gate | Required evidence | Current status |
| --- | --- | --- |
| Fixed qualification contract | Named fixtures, limits, commands and durations for all capabilities | Fixed fixture selection and existing limits now recorded in qualification doc; acceptance gaps remain explicit (Rain/restitution, island wake, persistent state, performance decision) |
| Ragdoll physical behavior | 5 x 600 steps per path, unchanged stability limits | Historical passes: `ragdoll-current-five`; latest-build qualification open |
| Ragdoll full-state repeatability | Every relevant semantic field identical over those runs | Open: physical outputs match; contact slot-permutation diagnostics pass, final semantic/storage audit incomplete |
| Rain recycling and joint behavior | Full 600-step identity/health checks, resolve excessive residuals | Open: 8,400 created identities, 4,200 reused slots and 1,948,800 joint observations mapped; residual screens still fail |
| Rain complete-state repeatability | 5 fresh full 600-step runs per path on fixed candidate | Open: run1 reaches600both; ordinary run1 complete/run2 interrupted, native launcher cleanup failed; no five-run qualification |
| Collision/support/energy | Analytical support, mesh contact and penetration cases | 57 frozen-pose cases and 5 x 600-step stack checks passed historically; final-build/wider matrix open |
| Friction/restitution | Analytical motion, rebound, settling, penetration and energy | Friction 5 x 240 passed each path; restitution 49.31mm penetration exceeds original 30mm screen on CPU and GPU; unresolved acceptance gate |
| Mass/inertia, mechanisms, motors/limits | Analytical/reference fixtures plus lifecycle and scripted changes | Selected fixtures pass; matrix closure open |
| CCD | Sweeps, fast bodies and mutation with no missed barriers | Selected fixtures and 5 fresh mutation repeats pass; broader/final-build closure open |
| Sleep/wake | Settling, sleep-before-wake proof and island behavior | Explicit sleep-to-wake and joint-island73step five-repeat checks pass both paths; contact-island wake/conservation and73step repeat evidence pass; original symmetry screens fail on CPU/GPU and remain explicitly recorded |
| Dragging | Scripted drag/release, physical limits and per-step repeatability | 5 x 3,060-step runs per path pass historically; check final-build applicability |
| Lifecycle/query/events | Creation/deletion/reuse, stable semantics, no stale handles or duplicate ends | Several 5-run fixtures pass; full-scene/final-build closure open |
| Native replay | Actual cached-step reuse plus mutation/reentry and full-state repeats | 5 x 48-step fixtures pass with 42 replay hits each; check final-build applicability |
| Numerical regressions | Preserve analytical/isolated numerical checks | All 28 precision tests passed both paths after latest production fix |
| Persistent-state coverage | Audited future-relevant fields and negative controls | Open; timing-window scheduling omission corrected/tested both paths; remaining inventory/reset audit incomplete |
| Controlled performance | Small/large scene benchmarks, each path/order, no tracing | Plan exists; not executed |
| Visual delivery/default | Repeated GPU clips, CPU comparison, documented mode selection | GPU-native qualification recordings pending; GPU-native remains opt-in |

## Current implementation and findings

- Efficient ordering is `GPU_PHYSICS_LIVE_CONTACT_ORDER=0`. Default has NOT switched. Preserve CPU-compatible mode.
- Production fixes include collision flat-edge admission, contact handle/query ordering and duplicate end events, joint fallback execution, float32 recycling default, and most recently the joint collision-filter upload address during body growth.
- Latest filter bug: in-place scene updates uploaded joint filters using the old live body count, then changed the count used to calculate the shader address. `src/api/world.rs` now updates live parameters before scene/joint scratch uploads. Minimal two-to-three-body growth reproduces failure before and passes after on both paths.
- Latest native bounded Rain check passes the unchanged filter guard at frames80/81 and completes81 health frames. Those health results exactly match the earlier physical baseline: no Rain stability improvement demonstrated in this window. Evidence: `rain-filter-fixed/result.json`, `joint-filter-growth-*`, `filter-fix-{guard,lifecycle,precision}-{ordinary,native}.log`.
- Joint fallback fixes in `shaders/physics/solve.wgsl`: one global fallback writer; invalid-list small-wave path actually invokes fallback. Regressions pass both paths. Valid Rain component lists mean this does not explain the observed Rain episode.
- Rain matched isolated replays transplant all39 active joint caches (27 spherical,12 revolute;3 filter joints have no solver cache). CPU/GPU target cone errors closely agree; this narrows the investigation but does not clear full-scene failures. Evidence: `rain-{cpu,gpu}-all-joint-history/`.
- Initial Rain contact-history discrepancy is now reproduced on CPU as described below; other full-scene histories remain unmatched. At captured state122, target human bodies605–618 have6 active contact patches,3 with nonzero cached impulses. Terrain foot contacts and one self-contact are the main next comparison. See `rain-first-cone-full-state/precontact-cache-inventory.json` and `history.jsonl`.
- Extra CPU zero-time stepping is NOT a neutral way to seed contacts: immediate body getters match but subsequent trajectories change. Do not reuse that rejected control.
- CPU read-only `b3Solve` linker wrapper in `c_abi/reference_contact_boundary.c` observes after normal collision update. It passes the no-op control: all2,520 finite body-state rows across60steps byte-identical to baseline,60 hook calls,20 first-step contact patches. Evidence: `rain-cpu-solver-boundary/{result,manifest}.json`. The three nonzero-history target contacts now match CPU boundary1 to GPU collision geometry123: normals bit-identical, feature IDs equal, anchors/separations differ by at most2.98e-8m. Triangle mapping is now verified: GPU point IDs encode array index+1, so CPU406/241 map to GPUarray470/247 and encoded471/248; convex CPU-1 corresponds to GPU0. All512 torus triangles match bitwise under a bijection. An initial direct-index comparison falsely suggested adjacent triangles; that inference was corrected after inspecting the encoding. Evidence: `rain-cpu-solver-boundary/target-contact-comparison.json`. Native float32 friction conversion is prepared in `rain-contact-history-audit/friction-world.txt`; source audit confirms old tangent coefficients reconstruct world impulse, then new basis projection in CPU preparation/GPU collision. Same endpoints require no sign reversal; preserve rolling world vector/twist scalar and last-substep normal impulse, not total impulse. Evidence: `rain-contact-history-audit/audit.json`. Controlled CPU injection now implemented in the diagnostic boundary wrapper. It validates all3manifolds (ordered body/shape endpoints, exact normals, feature/triangle IDs and geometry within1e-6m) before changing only cached impulses. Three60step arms produce2,520finitebodyrows each; zero is byteexact baseline and baseline matches prior. Six malformed payloads reject with exit8. CPU restored onset cone error0.311083078 exactly matches fullGPU; first3steps exact and first5maxerror1.192e-7rad. Across18steps maxerror0.012300193rad as other histories/order remain unmatched. Evidence: `rain-cpu-contact-history/{result.json,full-gpu-comparison.json,manifest.json}`. This reproduces the early residual with equivalent joint/contact history; it is not physical acceptance or fullscene clearance. Next: use this evidence to classify the first episode, then target remaining persistent/severe residuals with the existing physical criteria rather than chasing later exact trajectories.
- Warm starting cannot currently be disabled on GPU; document this unsupported configuration, and do not use it as a matched CPU/GPU ablation.

## Latest capture-throughput correction

The v19 JSON writer used an unbuffered File. It now uses a 1MiB BufWriter and explicitly flushes each complete frame, propagating flush errors. No schema/state fields or capture guards changed. A three-repeat benchmark on a real20,026,862-byte Rain frame produced byte-identical raw/buffered files: median serialization24.63787s versus0.101028s (~244x for serialization only). This is not solver or end-to-end capture performance. Evidence: `trace-buffering/{bench.rs,timings.txt,result.json}`. The body-growth capture regression passes ordinary and native builds; native runtime was also checked with its configuration script sourced. Both sample executables are now rebuilt with buffering. Bounded end-to-end comparisons are complete; see results below.

Buffered end-to-end capture and interruption-safe batch recovery are now validated below. State coverage remains unchanged. Do not introduce hashing machinery merely to solve the already-identified tiny-write bottleneck.

## Capture and recovery validation completed

All jobs from this validation are terminal; no current live handle. `rain-buffered-capture/result.json` records ordinary10step wall time103.224s raw/12.185s buffered (8.47x) and native110.664s/13.243s (8.36x). Buffered81step captures finish in80.392s ordinary/75.956s native. Every81step physical health row matches the historical baseline on both paths. At10steps the only state difference is contact_allocation.occupied_order, with exactly equal membership on every frame (`{ordinary,native}/allocation-differences.json`). No other group differs. This is bounded diagnostic throughput/behavior evidence, not full determinism or solver performance.

`scripts/check-sample-state-repeats.py` provides locked, frozen-binary/configuration batches with preserved attempts, verified compressed traces and completed-process compression recovery. Five helper integrity tests pass (`scripts/test-sample-state-repeats.py`). An actual subprocess interruption/resumption test also passes: live duplicate rejected; completed run1 byte/mtime unchanged; interrupted run2 partial preserved; five runs complete after six total launches; second resume launches none; changed binary rejected. Evidence: `repeat-runner-recovery/{check.py,result.json,*log}`. It uses a synthetic fixture emitting one real captured frame, so its pass validates the runner, not physics. Full Rain repeats are still open.

Capture buffering and recovery are validated. The fixed fixture selection is consolidated; actual Rain repeats have now started as described below. Avoid further capture infrastructure without measured need.

## Live Rain qualification batches

`rain-buffered-five` run1 has reached600steps on both paths. Ordinary sample exit0 (2171.53s); batch session38633 is historical; run1 finalized with a completion receipt and run2 was subsequently interrupted. Native batch session25285 is terminal exit1: sample log reports600frames/0sokol errors, followed by `/usr/bin/xvfb-run: line185: kill ... No such process`. The launcher kills Xvfb under `set -e` before returning saved child status, so cleanup can mask that status. Do not claim the child's exact exit code from this evidence or edit exit.json. Native health is structurally validated through600sequential frames/statusok; its failed attempt remains diagnostic, not a qualified completed repeat. Native batch stopped before run2. Do not automatically restart the frozen failing runner. Next runner correction must separately record child and launcher exits, preserve this attempt, and validate cleanup-failure handling before replacement fresh runs. No ordinary process remains at handoff; preserve completed run1 and partial run2.

Both frozen sample hashes remain ordinaryc43c6e072de819aba88f0b4a379d786ec6c89b695ffaa732c05e47eed0d2111c and native6cb8d97496af49748ea88a4ac5220439b9b4d93cae06e2c439c990e58bbe3366. These are diagnostic captures, not performance measurements. Health reports are1.2GB each. `rain-later-full-state/select_health.py` reads the native writer's frame-per-line format, checks full600frame sequence/status/document end and hashes the entire source, then emits labelled42body-cell diagnostic selections. This avoids materializing the whole report while the ordinary runner finalizes. Both selectors32561/6652 are complete; ordinary health also validates600frames/statusok. Ordinary twist/cone state extraction completed with output and manifests; handles4646/92245 are historical.

`rain-residual-classification.json` consolidates evidence without changing limits. Early onset is reproduced with matched39joint+3contact caches; severecone1.26054549rad is reproduced by CPU/GPU with matched poses but fresh history and recovers to0; persistenttwist also reproduces in isolated CPU/GPU and remains when kept awake. The later fullscene cache/load explanation is still open. Once run1 reaches the relevant windows, extract persistenttwist health375–463 and severecone near516 using verified creation identities; candidate health identities2379/5004 must be reconciled with state identity mapping rather than assumed equal. Do not chase the already-explained earlytrajectory again.

## Verified health-to-state identity mapping

`scripts/health_identity.py::health_to_core_creations` maps Human creation IDs to core creation IDs using the same completed step, public body slot and generation. It validates live allocation membership and uniqueness, and rejects stale/deleted lifetimes. No fixed terrain offset is assumed. All15identity tests pass, including recycled-slot, stale-generation, deleted-slot and wrong-step controls. On both buffered81step captures, all68,880Human body observations per path map successfully and rotations/linear/angular velocities match float32 bits exactly. Evidence: `rain-health-core-identity/{check.py,result.json}`. Those bounded captures do not cover actual recycling; verify the later full-run episodes with their own health records once run1 completes. Current partial full-state captures alone cannot establish the Human creation mapping.

The later-window extractor is ready at `rain-later-full-state/extract.py` (run from engine cwd). It requires `--trace`, `--health`, `--out`, zero-based health `--first`/`--last`, and Human creation `--base`/`--target`. For persistent twist use first374,last463,base2353,target2379; for severe cone first514,last525,base4999,target5004. Use each current run1 attempt's completed health.json and state.jsonl.gz after completion. It rejects missing/stale mappings and requires exact q/v/w float32 bits for all42cell bodies. Output is explicitly a diagnostic subset, not a qualification trace. It retains connected external bodies, all cell joints/contact patches, cached state and step-start transforms. Controls pass: bounded health70–80 gives11frames with42bodies/42joints; historical contact-rich health122–130 gives9frames with42cell bodies plus1terrain body,42joints and71–72contact patches. Evidence: `rain-later-full-state/{bounded-control,contact-control}.manifest.json`. Current later-episode extraction still awaits run1 health completion; do not substitute historical health for the current run.

## Joint-island wake qualification

`trace_joint_island_wake_propagation` passes5fresh73step runs on both paths with exact raw v19state equality. Three1kg cubes connected by distance joints and a fourth disconnected cube naturally sleep for64steps. wake=false impulse leaves all asleep; wake=true1N·s centered impulse wakes all three connected bodies on the next step while the independent body remains asleep. Predefined limits pass: peak momentum error2.384e-7kg·m/s (<1e-4), energy0.166690504J (≤0.5001), length error0.000353443m (<0.01), no spin. All10runs agree on physical metrics. This closes the selected joint-island propagation/repeat case; contact-island coverage and complete persistent-state audit remain open. Evidence: `island-wake-five/{run.py,summary.json,ordinary/result.json,native/result.json}`. Builds require `--features replay-diagnostics`; an initial featureless selection ran0tests and is explicitly not evidence. Actual five-repeat runner requires exactly1passing test and exported73frame trace. Sessions25989/1259/78858/30510/90302 are all terminal. Test-only code changed; live Rain frozen sample binaries were not modified.

## Contact-island wake screen failure

Added `trace_contact_island_wake_propagation`, sharing the joint-island helper with touching cubes/no joints. Limits were recorded before execution. Ordinary run1 reaches frame66: both touching links established, all bodies slept, wake=false preserved sleep, wake=true woke all3connected bodies while disconnected body stayed asleep. It then fails the individual spin<0.001rad/s screen (peak0.0192691538). No five-repeat pass: `contact-island-wake-five/ordinary/run-1.log`, exit101. The runner correctly stops; no native run was launched. Both feature-enabled builds succeeded (sessions88291/15186 terminal).

New `c_abi/island_wake_reference.cpp` reproduces the same configuration on CPU and completes73steps, including wake/isolation assertions. CPU frame66 velocities/spins match GPU magnitudes with reflected transverse signs; peak spin0.0192691538 also fails the original screen. Evidence: `contact-island-reference/{manifest.json,result.json,cpu.txt,gpu-island-wake-true-539400.jsonl}`. This is shared multipoint-contact asymmetry, not evidence of a GPU-only wake failure. Physical acceptance is not yet established: retain the original spin screen failure and evaluate total angular momentum/energy through the full73GPU steps (the current test stops on first spin failure). If a better-founded criterion replaces individual ideal-symmetry screening, justify it from conservation/solver behavior and explicitly preserve this original failed screen; do not simply raise the spin threshold. The qualified joint-only fixture's limits are unchanged.

## Full contact-island diagnosis completed

The original spin and transverse-drift screens now run after complete73step capture/export, with unchanged limits; they still fail the Rust test (exit101). An intermediate full-window attempt stopped on transverse drift; preserved at `contact-island-full-five/ordinary/run-1.log`. The final diagnostic batch `contact-island-full-v2-five` requires the explicit symmetry-screen failure and complete capture for each run, then compares repeatability separately. All5fresh runs per path have exactly equal raw v19state. Wake, isolation, momentum, energy and penetration assertions pass through73steps; tests still report the original symmetry failure, not an overall physical pass.

`contact-island-full-v2-five/analysis.json` compares all73steps to CPU. All10GPU runs match CPU p/v/w exactly after Y reflection (polar p/v signs[+,-,+], axial omega signs[-,+,-]); maximum reflected error0. This is strong evidence of a symmetric numerical trajectory choice, not a GPU-specific wake defect. Identical metrics on both engines: peak linear momentum error5.97146e-8kg·m/s, angular momentum1.36406e-6kg·m²/s (<predeclared1e-4), energy0.167829651J (<0.5001), penetration8.08239e-5m. Original individual spin0.0192691538rad/s (>0.001) and transverse drift0.001521538m (>0.001) remain failed screens. Conservation and repeated-state evidence are established; reconcile the ideal-symmetry screen's role explicitly before marking the whole selected physical case accepted. Do not change the solver to force a CPU mirrored trajectory. All build/run sessions10062/35114/97486/82777/80973/55417 are terminal. No production solver changes this turn.

## Timing-window state audit correction

`GpuSim.metrics` is not wholly incidental instrumentation: steps/submitted select metric_step and suppress native full-step replay while the timing window is active. `diagnostic_policy_state` now exports optional `gpu_policy.timing_window={steps,submitted}`. No-window remains canonical field absence, preserving default v19 captures. Durations/query results remain excluded. Older active-window captures cannot be claimed covered; require independent no-window evidence for old captures. Source review found timing-window entry only through Rust API/headless/tests, with no C bridge/native sample call, so running frozen Rain captures are unaffected. `trace_captures_timing_window_replay_policy` passes ordinary and configured native: opening a window changes only this policy field, stepping increments submitted in full capture, finishing removes it. Evidence: `timing-window-state-audit.json`, `timing-policy-{ordinary,native}.log`. Inventory metrics entry refined; do not claim all209fields audited. Builds43824/26014 and native test are terminal. Only diagnostic capture code changed; no sample rebuild/restart needed.

## Force-clear and shape-identity audit

`step_force_slots` is represented by each live body's clear_force_next_step membership plus captured device loads. Source consumer drains old members with zero writes before any new load writes; order/duplicates therefore have no effect, and has_step_forces only checks emptiness. Uncaptured membership is rejected. `shape_identities` is the actual source of shape_storage_order, validated against live world shape generations; public shape slots are append-only. Added a generation-only corruption control to `trace_rejects_uncaptured_host_policy_and_force_membership`: stale GPU shape identity must reject capture without appending, then restore and confirm the existing force-membership corruption rejects. Both ordinary/configured native pass. Evidence: `state-identity-membership-audit.json`, `state-identity-membership-{ordinary,native}.log`. These two inventory entries are classified with consumer evidence; remaining explicit-consumer fields and device reset/persistence contracts remain open. Sessions77831/57578 and native test are terminal. Next persistent-state review candidate: contact_metrics boundary/current-step guarantees and output-versus-scheduling use; do not assume it is merely a metric by its name.

## Current full-scene Rain episode evidence

Native extraction completed: `rain-later-full-state/native-twist.jsonl` covers core375–464 (health374–463), corebody2479 verified as Human2379; `native-cone.jsonl` covers core515–526 (health514–525), core5104 verified as Human5004 including recycled slots. All42cell bodies on every selected frame match slot+generation and q/v/w float32 bits. Each window contains42joints and one external terrain body; contact patches retained. Extractor sessions92330/54117 are terminal. Evidence: manifests plus `native-health-validation.json` and `native-episode-summary.json`.

Persistent twist last-awake core426: measured lower violation0.05448156343rad, cached-impulse/stiffness estimate0.05448071309rad (difference8.5034e-7). First asleep core427 retains essentially the same error; finalcore464 error0.05448156626. `native-twist-compliance.json` uses `analyze_compliance.py`, an excerpt adapter of the existing reconstruction; it is not a full per-substep contact/joint torque balance. This supports loaded soft-constraint compliance, not a new sleep-created error or a physical acceptance pass. Severe cone still peaks1.26105142rad at health516, maxanchorerror0.0698773041m, recovering to0.00566038489rad at525. Oldscreens remain failed. Next: extract ordinary windows after selector completion, compare equivalent identity data, inspect target contact/load history for the severe transient and classify acceptance with physical evidence. Do not repeat early-onset work.

## Recovery after laptop restart

Verified 2026-09-28: no Rain fixture/runner processes remain. Prior session handles84200/38471 are historical, not live jobs.

`rain-filter-fixed-five/ordinary/run-1.jsonl` ends at complete frame156 (~2.72GB); native ends at158 (~2.76GB). No exit files, completed compressed traces or repeat results. Preserve these captures. Both crossed the old frame81 failure; neither qualifies a completed run. Older failed and interrupted batches also remain evidence; do not overwrite or count them as passes.

The latest ordinary/native sample binaries were rebuilt with the filter fix. Frozen copies and source/config manifests are in `rain-filter-fixed-five/{ordinary,native}/`. Verify their hashes before reuse. New capture-format code requires a new frozen candidate; do not mix its results with old binaries.

Build commands from the engine working directory:

```sh
cargo build --release --lib --features replay-diagnostics
cmake --build native-samples/build-gpu --target samples_gpu -j4
bash scripts/build-native-cache.sh build --release --lib --features replay-diagnostics
cmake --build native-samples/build-gpu-native-cache --target samples_gpu -j4
```

Native runtime requires sourcing `scripts/native-samples-cache-env.sh`; the build wrapper alone does not set runtime configuration. Sample binaries are under each build's `bin/samples_gpu`. Full qualification must leave `GPU_PHYSICS_STATE_TRACE_FIRST_STEP` unset; selected-frame captures are diagnostic only.

## Next actions

1. Close the explicitly listed acceptance gaps in the fixed fixture selection in gpu-solver-qualification.md. Existing fixture choices/limits are now consolidated; do not invent additional coverage beyond the stated scope.
2. Use the validated buffered capture and resumable batch runner for final frozen candidates. Preserve completed runs and failures; limit further infrastructure work to the demonstrated launcher child-exit receipt defect and any specific uncovered audit requirement.
3. Early Rain contact-history restoration is validated. Focus on later persistent/severe episodes from the preserved full captures and their physical/load criteria; use rain-residual-classification.json to avoid repeating settled onset diagnostics.
4. Run and close final fresh-repeat and capability gates on both configurations, investigate the earliest failures, and preserve original thresholds/results.
5. Run controlled performance measurements with no competing GPU work, then recordings and the documented default-mode decision.

## Authorization and repository constraints

User explicitly authorized commit and push of this checkpoint on 2026-09-28. Preserve unrelated dirty work. Do not modify the upstream `box3d/` submodule or grow the WASM library for this experiment. No subagent delegation currently authorized. Before committing solver/visual changes, repository instructions require affected scene snapshots and a real Box3D C comparison column; read AGENTS.md for the commands. Do not claim this scratchpad or a diagnostic hook completes the solver goal.

## Consequential decisions

- 2026-09-28: User requests a self-contained, file-backed goal maintained across compaction and restart, plus a reusable user-level skill. This file is the durable handoff.
- 2026-09-28: Retain physical correctness, same-configuration determinism and performance outcome. Tighten execution with a fixed matrix, practical complete-state comparison and interruption-safe batches. Stop reporting speculative percentage completion.
