# GPU physics experiment

The [experiment](../experiments/gpu-physics/README.md) implements rigid-body physics
in Rust/WGSL and exposes part of Box3D's native C API. It does not replace the
Box3D WASM package or modify the upstream engine. Native samples can run either
engine or show two independent worlds side by side.

## Support status

| Area | Status |
|---|---|
| Linux NVIDIA | Native CPU/GPU/combined builds and targeted runtime checks pass; strict drag discrepancy remains |
| Linux AMD | Builds, picking, isolated dragging, CCD and scene-switch checks pass; ground dragging exceeds normal comparison tolerances |
| macOS / Windows | Build paths and CI matrix added; CI results and physical-GPU runtime validation are still required |
| Intel / Apple hardware | Adapter selection permits capable devices; hardware correctness and performance are unverified |
| Experimental browser target | Does not currently compile; native transport, ABI layout and world-storage assumptions need work |
| Box3D WASM package | Separate implementation; unchanged by the experiment |
| Native API parity | Incomplete; linkable symbols include placeholders and CPU-only comparison passthroughs |
| Performance | Cached Sokol improves over ordinary Sokol on two measured workloads; GPU-only Sokol remains slower than CPU-only |

Platform setup, adapter overrides and cache policy are in the
[backend guide](../experiments/gpu-physics/compiler/native-backend/README.md).
Automatic selection ranks capable devices; it does not benchmark every adapter
or tune the solver for each device.

## Missing features and known failures

The portable-build audit on 2026-09-19 identifies **43 stub definitions and 10
additional placeholders**; **36 concern recording/replay**. It also lists 21
CPU-only comparison wrappers for semantic review, with no additional missing
linked symbols in the selected build. Full native compatibility must not be inferred from scene checks.

| Gap | User-visible consequence / remaining work |
|---|---|
| Shape replacement (`b3Shape_SetSphere`, `SetCapsule`, `SetHull`) | Calls are stubs. Implement geometry replacement with stable IDs, mass updates, contact invalidation and matching render geometry. |
| Joint reaction/separation queries | Force, torque and linear/angular separation getters are stubs. Implement and compare against native fixtures. |
| World controls | Warm-start and speculative-contact toggles do not control GPU behavior. Worker-count APIs are placeholders rather than GPU scheduling controls. |
| World diagnostics | Profile/max-capacity APIs return placeholders; memory/bounds dump and static-tree rebuild helpers are incomplete. Public `contactCount` is not implemented by the GPU world counter. |
| Recording/replay | Native recording creation, storage, file I/O, playback, seeking and query-history APIs are placeholders. Diagnostic state replay is not an implementation of these APIs. |
| Combined viewer coverage | Some generated wrappers call only CPU APIs. Audit each remaining wrapper before claiming that controls or diagnostics affect/report both worlds. |
| Browser target | Fix native-only transport/pose dependencies, pointer-size ABI assertions and global world storage before advertising WebGPU support. |

Run `python3 experiments/gpu-physics/scripts/audit-native-api.py --require-complete`
for the function inventory. The audit selects portable builds using the launcher
cache policy; explicit `--gpu-build-dir` and `--both-build-dir` override it.
`--require-built` rejects absent archives or generated wrappers. Missing artifacts
produce unknown coverage rather than a successful empty inventory. CI checks
that builds supply these inputs, without requiring the unfinished APIs to pass.

Known numerical failures retain their existing thresholds:

- NVIDIA's strict ten-drag check reaches **0.982809 m/s** instantaneous velocity
  difference against **0.5 m/s**. Normal held-motion and settling checks pass.
- AMD ground dragging fails held position (**0.007997 m**, limit **0.005**),
  quaternion chord (**0.011881**, limit **0.01**) and held velocity
  (**0.420311 m/s**, limit **0.1**). Cache-on and cache-off results agree;
  disabling caching does not fix this discrepancy.
- The diagnostic seeded replay at step 1735 exceeds its single-step velocity
  tolerance across all five bodies. Copying CPU state in this diagnostic does
  not demonstrate independent simulation parity. Run
  `scripts/check-drag-state-replay.sh 1735` from the experiment directory.

Gear Lift's current native gate measures geometric support, penetration duration,
floor crossings and joint behavior over 1,200 steps. Its earlier chaotic rock
trajectory mismatch is not a bitwise-parity guarantee. Village has bounded
600-step sphere-drop and scene-transition coverage, not exhaustive interaction
coverage. Neither scene establishes full native API compatibility.

## Architecture and state ownership

| Layer | Responsibility |
|---|---|
| Upstream C helpers | Hull cooking, geometry utilities and the independent CPU reference |
| Rust | World/ID lifetimes, uploads, callbacks, host queries, events and CCD fallback |
| WGSL | Broadphase, contact generation, graph scheduling, constraint solving, integration and closest-hit queries |
| Native sample bridge | Box3D C interface, independent comparison worlds and current-state drawing |

Body/shape/joint slots retain generation-checked identities. Capacity growth must
preserve live contacts and constraint history; allocation or contact-capacity
loss must be reported, not silently interpreted as a successful simulation.

Stepping submits GPU work. Synchronous public getters and queries finalize the
required step, including pending CCD, before returning current state. Events are
harvested once; unread events expire at the next step. Callbacks use owned query
snapshots so nested queries do not hold the world lock. Mutations invalidate
cached state and upload affected data.

The resident GPU path and convex GPU CCD are eligibility-based optimizations.
Unsupported geometry, callbacks or other excluded world features retain the
ordinary path. Sleep dispatch elision requires a valid zero-active proof across
an unchanged eligible submission chain. Command replay must retain resources
through completion and invalidate on relevant parameter/resource changes.

The combined viewer never copies CPU poses or hits into GPU simulation. Both
panes draw the same completed step. Its shared sample controller and GPU-led
inspector remain limitations when interpreting sample-specific behavior.
Linux GL pose imports require matching Vulkan/OpenGL device UUIDs; otherwise
the viewer uses current CPU pose mirrors. Other platforms use mirrors directly.
The separate Linux split-adapter experiment displays N-1 while simulating N;
its CPU control uses the same delay. Those results do not describe Sokol.

GPU contact metrics describe the contact-scheduling phase and carry step and
state revisions. Unknown or stale snapshots must remain labelled. Candidate
pairs, allocated roots, manifold slots and touching roots are different counts;
none should be presented as the unimplemented native public contact counter.

## Validation and merge requirements

Merging as an **experimental native engine** does not require completing every
Box3D API or proving a speedup on every GPU. It does require accurate limitations,
reproducible builds and a current regression result. A production-compatible
replacement would additionally require closing the API and physics gaps above.

The ordinary release library suite passed **232/232 tests** at `f2f4e69b`; this
cleanup does not change Rust or shader physics. Cleanup checks pass six tooling
tests, full TypeScript checking, lint (with warnings), a two-restart Junkyard
smoke, cached and ordinary native builds, and artifact audits for both builds.
Cached combined and ordinary GPU launches preserve matching step identities.
The earlier demo build also passes; WASM was not rebuilt. Local review logs are
in `experiments/gpu-physics/artifacts/merge-review/`. The updated default-cache
native scene gate passes **12/12 cases**, including 1,200-step Gear Lift,
600-step Village and the Village → Bounce House switch. This is Linux NVIDIA
physics with AMD graphics, not cross-vendor or performance qualification.

Before merging:

- Run Linux/macOS/Windows CPU, GPU and combined build jobs from clean checkouts.
  The CI matrix includes ordinary builds on all three platforms and a Linux
  cached-backend build with library-test compilation. Physical GPU tests remain
  separate; these jobs have not yet produced results for this change.
- Run the complete correctness gate and cached-backend lifecycle/CCD/event
  tests on the final code. The earlier 58-case correctness result predates the
  portability changes; the native matrix has now been refreshed. Repeat affected
  suites after further code changes and retain failures/skips with source identity.
  The native scene gate now freezes binaries from the portable launcher and
  records/applies its runtime configuration; cache opt-out uses the same gate.
- Exercise the final launch path: default cache, cache opt-out, AMD selection,
  hybrid graphics fallback, repeated scene changes, dragging, pause/single-step,
  sleep/wake and shutdown during loading. Preserve independent step identities.
- Verify the original WASM/demo workflow still builds and typechecks. The
  2026-09-19 review passed the demo build, workspace typechecking and lint
  (with warnings); it did not rebuild WASM. The subsequent cleanup repaired
  Junkyard checks to use native object counts and all remaining bridge pools.
  Full TypeScript checking and a two-restart Junkyard smoke now pass.
- Review public C placeholders and generated CPU-only wrappers. Unsupported
  controls must not be presented as functioning GPU features. GPU warm-start
  and recording controls are now unavailable; the combined worker slider is
  explicitly CPU-only. Sample-specific controls still need a compatibility audit.
- Publish a compact test summary with the PR. Raw captures are ignored local
  files; archive any evidence needed by reviewers outside the checkout.

The large initial engine addition also needs a code review focused on GPU buffer
bounds, ID reuse, C/Rust layouts, callback reentrancy, command-cache lifetimes and
failure cleanup. Build success alone cannot establish these properties.

Benchmark reports must distinguish device time, completed physics time and whole
application cadence. Compare equal scenes, CPU worker counts, framebuffer sizes,
pacing, sleep settings and display delay; retain all interleaved trials. The
[current measurements](../experiments/gpu-physics/compiler/native-backend/README.md#measurements)
include their scope and do not establish a general CPU advantage.
