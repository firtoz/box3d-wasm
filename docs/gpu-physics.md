# GPU physics experiment

The [experiment](../experiments/gpu-physics/README.md) implements rigid-body physics
in Rust/WGSL and exposes part of Box3D's native C API. It does not replace the
Box3D WASM package or modify the upstream engine. Native samples can run either
engine or show two independent worlds side by side.

## Support status

| Area | Status |
|---|---|
| Linux NVIDIA GeForce RTX 4070 Laptop GPU | Native CPU/GPU/combined builds and normal/strict retained-history drag checks pass |
| Linux AMD Radeon 780M | Builds, picking and normal/strict retained-history drag checks pass; convex-sweep regression remains |
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

## Work priorities

1. **Completed: ground-drag agreement.** Both tested GPUs pass the independent
   ten-drag sequence at unchanged normal and strict thresholds. Solver/contact
   regressions pass; the separate AMD sweep and secondary-queue failures below
   remain. These results do not establish general GPU parity.
2. **Next: non-recording native APIs.** Shape replacement is implemented; next are joint
   reaction/separation queries, world controls and diagnostics. Review CPU-only
   comparison wrappers. Each completed API needs a sample or focused fixture
   exercising independent CPU and GPU behavior, including mutation and lifetime
   cases where relevant.
3. **Queued: GPU engine in WASM/WebGPU.** Repair the experimental browser build
   and add browser runtime tests. Start with Firefox on the development machine:
   WebGPU is reported available there, but unavailable in the local Chromium,
   Brave and Helium installations. Verify adapter/device creation when testing.

Recording/replay, application performance work, and Windows/macOS/other-GPU
portability are deferred. Runtime testing so far is Linux-only on the RTX 4070
Laptop GPU and Radeon 780M. Other platforms/devices still need implementation
where incomplete and build/runtime verification. GPU-only Sokol remains slower
than CPU-only in the measured workloads; optimization is deferred until after
correctness and non-recording APIs.

## Missing features and known failures

The portable-build audit on 2026-09-20 identifies **40 stub definitions and 10
additional placeholders**; **36 concern recording/replay**. It also lists 17
CPU-only comparison wrappers for semantic review, with no additional missing
linked symbols in the selected build. Full native compatibility must not be inferred from scene checks.

| Gap | User-visible consequence / remaining work |
|---|---|
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

### Native shape replacement

`b3Shape_SetSphere`, `SetCapsule` and `SetHull` replace GPU geometry in place.
They retain shape IDs, material/filter/event settings and user data. Shape mass
data changes immediately; body mass, center of mass, inertia and stored CCD
extents change only when `b3Body_ApplyMassFromShapes` is called. Retiring touching
contacts wakes their owners and connected sleeping islands; an isolated sleeping body stays asleep. An identical hull is a no-op,
including input from `b3Shape_GetHull`. The bridge owns complete hull copies,
including the face data and padding used by upstream's content comparison.

Replacement retires affected GPU contact roots and public contact handles,
updates query/CCD geometry and invalidates cached scene uploads. Unrelated
contact histories remain intact. Public shape/body AABB queries include the
native 0.02 m speculative margin. Both comparison engines execute the mutations
independently, and both renderers rebuild replaced geometry.

Run **GPU API / Shape Replacement** in any native viewer. Its shared input
schedule cycles through an offset sphere, capsule, box and cylinder every 180
steps, with explicit mass recomputation on alternate replacements. The focused
fixture checks metadata, queries, mass policy, motion, sleeping contacts, hull
aliasing, debug geometry destruction/recreation and geometry ownership:

```sh
# From experiments/gpu-physics; defaults to the native-cache backend on Linux.
GPU_PHYSICS_ADAPTER=amd ./scripts/check-shape-replacement.sh artifacts/shape-replacement/cached
GPU_PHYSICS_ADAPTER=nvidia ./scripts/check-shape-replacement.sh artifacts/shape-replacement/cached
GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0 GPU_PHYSICS_ADAPTER=amd \
  ./scripts/check-shape-replacement.sh artifacts/shape-replacement/ordinary
GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0 GPU_PHYSICS_ADAPTER=nvidia \
  ./scripts/check-shape-replacement.sh artifacts/shape-replacement/ordinary
```

On 2026-09-20 the fixture passes on AMD Radeon 780M and NVIDIA RTX 4070 Laptop,
both with native-cache and with cache opt-out. It retains the existing `1e-5`
motion tolerance and checks that unrelated sleeping contacts survive replacement.
CPU and combined AMD viewer health scans complete 720 measured steps without
NaNs or exploded bodies. The free-running contact scene can diverge in transient
capsule tipping time; this is not a long-running pose-parity gate. A diagnostic
reproduces the initial 180-step sphere motion exactly through the existing
destroy/create path in each engine, establishing divergence before the capsule
phase. Local logs, scene captures and source/binary hashes are
in `experiments/gpu-physics/artifacts/shape-replacement/RESULTS.md`. These are
Linux Vulkan correctness checks; other platforms and performance remain unqualified.
The comparison grid includes a `shape-replacement` row made from the recorded
CPU/GPU panes. This native sample is not hosted by `record-snapshot.sh` or the
Rust oracle replay renderer; its CPU pane runs real Box3D C.

### Ground-drag qualification

The default native-cache path passes all ten drags on both Linux Vulkan adapters,
including release/re-grab with retained histories. Each world performs its own
picking and joint creation; CPU state is never copied into the GPU simulation.
Both GPUs produced these maxima on 2026-09-20:

| Measurement | Observed | Unchanged limit |
|---|---:|---:|
| Held position difference | 0.002881 m | 0.005 m |
| Held quaternion chord | 0.005064 | 0.01 |
| Held velocity difference | 0.047432 m/s | 0.1 m/s |
| Settled velocity difference | 0 m/s | 0.02 m/s |
| All-frame position difference | 0.006028 m | 0.025 m |
| All-frame quaternion chord | 0.005973 | 0.025 |
| Strict all-frame velocity difference | 0.230405 m/s | 0.5 m/s |

Reproduce from `experiments/gpu-physics` (Linux cached backend):

```sh
GPU_PHYSICS_ADAPTER=amd ./scripts/check-both-pointer.sh artifacts/drag-check
source scripts/native-samples-cache-env.sh
GPU_PHYSICS_ADAPTER=amd artifacts/drag-check/both-drag --ground-strict
GPU_PHYSICS_ADAPTER=nvidia artifacts/drag-check/both-drag --ground-strict
```

No experimental precision flag is required. The first command also checks picking
and isolated dragging. Local qualification logs are under
`artifacts/drag-agreement/default-*`. Diagnostic CPU-state replay remains a
first-divergence tool, not evidence of independent simulation agreement.

The broader AMD library suite exposes a separate convex-sweep failure: case 20 of
`gpu_convex_sweeps_match_cpu_conservative_advancement` reports a GPU hit at
fraction 0.115237534 where the CPU reports no hit. It reproduces in the earlier
2026-09-19 test binary; the sweep shader and test are unchanged by the drag fixes.

Gear Lift's current native gate measures geometric support, penetration duration,
floor crossings and joint behavior over 1,200 steps. Its earlier chaotic rock
trajectory mismatch is not a bitwise-parity guarantee. Village has bounded
600-step sphere-drop and scene-transition coverage, not exhaustive interaction
coverage. Neither scene establishes full native API compatibility.

## Architecture and state ownership

Native Vulkan physics preserves explicit scalar arithmetic order with SPIR-V
`NoContraction` decorations, including explicit FMA residuals used for corrected
division and square roots. Other backends retain the WGSL path and still need
equivalent numerical validation. Compiled modules are cached by shader source,
entry point and exact override values; no device-specific solver constants are used.

The drag investigation found several rounding differences that accumulated across
contacts: fused vector operations, approximate reciprocals, square-root double
rounding, and different matrix and impulse evaluation order. Box clipping now
keeps the native local-coordinate calculation order and preserves original contact
separations for recycling instead of reconstructing them from solver offsets.
Mass queries retain the original mass instead of inverting its rounded reciprocal,
so independently created pointer joints receive matching inputs. These changes
preserve independent simulations and the existing comparison thresholds.

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

The 2026-09-20 native-cache release library suite reports **247/247 passed on
NVIDIA** and **241/247 passed on AMD**. One AMD failure is the pre-existing
convex-sweep case above; five secondary-queue tests fail because RADV exposes
fewer than the two required graphics/compute queues in family zero. These are
failures, not passes or silently skipped checks. Both GPUs pass the solver/contact,
mass-query, command-replay, pose-staging and event regressions. The six native
tooling tests also pass. Normal and strict ten-drag checks pass again on both
adapters with the unchanged maxima above. Current replacement/regression evidence
is in `experiments/gpu-physics/artifacts/shape-replacement/`; earlier drag evidence
remains in `experiments/gpu-physics/artifacts/drag-agreement/`.

Earlier merge-review checks covered TypeScript, lint (with warnings), the demo
build, a two-restart Junkyard smoke, cached/ordinary native builds and artifact
audits. The 12-case native scene matrix included 1,200-step Gear Lift, 600-step
Village and a Village → Bounce House switch, using NVIDIA physics and AMD graphics.
Those results predate the drag fixes; they are not current scene-matrix or
cross-platform qualification. Logs are in
`experiments/gpu-physics/artifacts/merge-review/`. WASM was not rebuilt.

Before merging:

- Verify clean Linux CPU/GPU/combined builds with and without caching.
  Windows/macOS CI and hardware verification are deferred; retain Linux-only
  runtime claims until that work is complete. The CI matrix remains available
  for future platform validation.
- Run the complete correctness gate and cached-backend lifecycle/CCD/event
  tests on the final code. The earlier 58-case correctness result predates the
  portability changes, and the 12-case native matrix predates the drag fixes.
  Refresh both before merging and retain failures/skips with source identity.
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
