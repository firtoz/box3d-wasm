# Source and clock applicability

These observations use the exact source bytes in the linked parent723-input
archive, ordinary viewer generated-input archive and the frozen hook copies.
Current production source remainsabc0a54 with the separately recorded test-only
sleeper correction. This audit identifies consumers, not measured internal costs.

- `scripts/inject-sokol-bench.py` marks physics immediately before
  `b3World_Step`. It calls `gpu_sokol_bench_note_world` before marking profile.
  In `sokol_bench_hooks.cpp`, `add_mark` charges elapsed time to the previous
  phase. Therefore the physics interval includes the health hook. Health starts
  its own nested clock before the first world health scan and stops after storing
  body/joint records. The observer only prints these existing fields/progress.
- `src/api/world.rs::b3_world_health_scan` uses `with_world`. That accessor calls
  `ensure_cpu_mirror_world`; stale mirror, pending events or pending CCD trigger
  `sync_world_mirror_parts`. That path waits for GPU completion, reads mirrors,
  runs deferred `run_continuous_collision`, applies corrections/snapshots and
  collects events. These costs can fall inside health. Low later picking
  `query_wait_ms` is not evidence that this earlier synchronization was cheap.
- `src/c_abi.rs::gpu_b3_world_visit_dynamic_bodies` collects handles and releases
  the world lock before invoking callbacks. The observer collector then reads
  public body values; joint health is collected in bulk followed by spherical
  metadata queries. The callback path is not a nested world-lock deadlock.
  `ensure_cpu_mirror_world` is conditional; it does not unconditionally clone a
  full mirror for every ordinary getter. Body creation tracking uses `std::map`.
  None of these source observations measures collector overhead.
- Deferred CCD constructs fast-body/shape/target lists. For each candidate target,
  `body_pair_allowed` scans the joint list, and `host_shape` is constructed before
  later geometric rejection. Rain has100 static terrain bodies/200 mesh shapes
  plus increasing articulated populations. Those host paths are plausible costs
  to instrument, alongside GPU completion and readback. No attribution percentage
  to CCD, joint scans, mesh clones or GPU kernels is proved by current clocks.
- Frame/progress labels are observer state at the reporting point: health-begin
  may retain the previous completed/submitted IDs until later drawing/accounting.
  Completed-frame rows must match frame/submitted/completed; they do for all180.
  Intermediate labels must not be interpreted as an extra physics step or loss.
- `box3d/shared/benchmarks.c::StepRain` drives spawning/recycling from step count.
  Neither observation timing nor software rendering changes that schedule.
  The first180 steps have no recycled body slots. Longer lifetime qualification
  remains required and the failed600-step runs cannot be converted into passes.

The source review also finds that the diagnostic spherical/revolute cache setters
in `src/api/world/state_trace.rs` still construct a generation1 world handle after
checking joint metadata. The present fresh first-world phase probe does not
exercise recreation through those setters. Their generation applicability and
WorldRegistry/C metadata future-state capture remain explicit PR05/PR07 audit
work; schema24 and this pose/identity subset do not prove those obligations.

All observer build/run budgets are now consumed. New instrumentation must have a
new discriminating producer/consumer hypothesis and a finite recorded budget;
no unchanged repeated trial or favorable-only extension is authorized.
