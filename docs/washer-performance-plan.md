# Washer Performance Plan

This tracks the current performance work for the WASM washer demo. Checkboxes should be updated as implementation lands.

## Measurement Hygiene

- [x] Add benchmark URL flags for disabling expensive overlays (`charts`, `stats`, `shadows`).
- [x] Disable or throttle physics charts during benchmark captures.
- [x] Add timing around washer snapshot consumption and render-state updates.
- [x] Add timing around `renderer.render` for whole-frame GPU/Three visibility.
- [x] Add `/bench` page to run trace-friendly washer variants sequentially.
- [x] Emit per-variant bench summaries with worker profile and frame/render timing.
- [ ] Capture clean before/after traces with the same URL flags and worker count.

## Renderer Data Path

- [x] Replace `InstancedMesh.setMatrixAt` per cube with position/quaternion instanced attributes.
- [x] Use a custom Three shader for washer cubes that applies quaternion transforms in the vertex shader.
- [x] Upload `7` floats per cube instead of `16` matrix floats per cube.
- [ ] Keep drum, ground, camera, lights, controls, and projectiles compatible with existing Three.js scene flow.
- [x] Add bench variants to compare shader path against current `InstancedMesh` path.
- [x] Route the washer shader path through the shared multi-layer `shader-instanced-host` (drum via `setupScene`; matrix A/B still sample-local).

## Worker Publish Path

- [ ] Keep washer simulation in the dedicated worker.
- [ ] Introduce double/triple-buffered snapshot slots so main reads only complete frames.
- [ ] Publish render-ready packed data rather than general body state when running washer benchmark mode.
- [ ] Avoid per-frame debug color extraction unless explicitly enabled.
- [ ] Measure worker publish/copy time before and after packed snapshots.

## WASM/Engine Exports

- [ ] Add a direct box/cube shape creation path for washer cubes instead of generic hull-from-points setup.
- [ ] Add a packed transform export for render state.
- [ ] Benchmark continuous collision disabled for washer cubes.
- [ ] Benchmark solver/substep/contact settings specifically for washer.
- [ ] Profile Box3D WASM pthread task granularity and semaphore waiting.

## Threading

- [ ] Add washer-specific worker-count benchmark for `1,2,3,4,5,6,8` workers.
- [x] Add browser runner that sequences worker-count variants in one trace capture.
- [ ] Compare physics step time, publish time, main render/update time, and full frame time per worker count.
- [ ] Tune default worker count so physics helpers do not starve main/render/GPU threads.
- [ ] Keep pool size and physics worker count independently tunable.

## To Consider Later

- [ ] Prototype a WebGPU washer renderer once packed transform data exists.
- [ ] Consider GPU-side culling or transform expansion if WebGL shader path remains GPU-bound.
- [ ] Explore engine-side dirty active-body lists for sleeping bodies.
- [ ] Consider visual/physics simplifications for the drum if benchmark goals allow it.
- [ ] Consider a full GPU physics experiment only as a separate long-term project.

## Completed

- [x] Read `bench-profile.json.gz` and `bench-release.json.gz` traces.
- [x] Identified worker physics, main-thread transform/render work, GPU work, and chart drawing as key measured costs.
- [x] Confirmed current worker/main bridge already uses `SharedArrayBuffer`, but still copies WASM heap data into SAB snapshots.
- [x] Confirmed current washer render loop rebuilds up to 8,000 Three.js instance matrices on the main thread.
- [x] Implemented `?bench=1`, `?charts=0`, `?chartHz=N`, `?stats=0`, `?shadows=0`, and `?timings=1` demo flags.
