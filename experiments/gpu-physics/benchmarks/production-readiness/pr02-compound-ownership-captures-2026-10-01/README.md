# Compound ownership: precommit CPU and combined recordings

All **12 captures complete 300 physics steps** with finite poses, no physics
capacity loss and no Sokol errors. All six completed-window comparison sheets
are reviewed. **Village has a retained visual failure:** nearby compound ground
and building geometry is absent in the GPU half of the combined view. This is
scene/health evidence, not visual parity, widget input, physical equivalence or
performance acceptance. PR09 remains open for that rendering gap.

The [ownership correctness campaign](../pr02-compound-ownership-after-bounds-2026-10-01/README.md)
passes14 original assertion processes on ordinary/native NVIDIA Vulkan. The
sole production change bypasses CPU mirroring for private primitive compound
children; public constructors still mirror. No renderer, Rust/WGSL, physics
setting, scheduling default, Box3D or WASM change is made by that candidate.

## Frozen recording budget

[Protocol](protocol.json) SHA-256
`06083a3cc76f56948cfae3d02b2e145c6dda29a911d6ed034b76c6f57d56874a`
freezes six real Box3D CPU processes first, then six native combined processes:
Simple, Spheres, Hulls, Tile Floor, Village and unchanged Mesh Tile control.
Five scenes are every affected upstream baked-compound constructor; Mesh Tile
checks the unchanged private mesh path. Each has a600-second watchdog; first
launch/source/health/capture failure stops, with zero retries/timing runs. All12
first processes complete. No trial or unfavorable observation was discarded.

Settings preserve upstream geometry/assets/materials/gravity, sleep, warm start,
CCD, four substeps and1/60 timestep. Warmup0 and300 completed steps are recording
settings, not a benchmark. Fresh isolated `{}` settings suppress only first-run
help/replay redirect; `--hide-ui` hides controls except the combined view toggle.
No Village drop is performed. Village retains52,500 compound children, including
2,500 building meshes. Startup/loading footage remains in the full clips.

| Scene | CPU / combined health | Visual review |
| --- | --- | --- |
| Simple | pass / pass | static compound visible in both views |
| Spheres | pass / pass | static spheres visible in both views |
| Hulls | pass / pass | static hulls visible in both views |
| Tile Floor | pass / pass | tile geometry visible in both views |
| Village | pass / pass | GPU nearby ground/buildings absent; retained failure |
| Mesh Tile | pass / pass | mesh control visible in both views |

Capsule presentation/trajectories can differ; no numerical physical acceptance
is inferred from these passive clips. [Source/scene review](implementation-review.md)
explains the render callers and scope. [Village sheet](review/compound-ownership-village.png)
shows the missing geometry; the exact full videos are linked in the comparison
grid and pinned by hashes in the receipt.

## Village renderer cause and recovery

The [source finding](raw/village-renderer-finding.json) and stderr record a
65,536-slot shared DebugShape pool. CPU and GPU each flatten a52,500-child
compound plus its parent, requiring at least105,002 slots before dynamic shapes.
`AllocDebugShape` returns-1 at capacity; compound registration silently skips
unregistered children. CPU draws first, so the GPU parent cannot register all
of its children. The renderer sources are unchanged from before this candidate.
The ownership delta leaves all private GPU creation/attachment/render callbacks
unchanged and removes extra CPU mirroring, reducing rather than adding CPU
shape allocations. This existing rendering capacity limit does not invalidate
the independently passing ownership checks. It is not excused as intentional
visual divergence: PR09 must repair and qualify it after the core gates, in the
user's requested order. No renderer candidate or extra GPU diagnostic ran here.

## Actual compiled provenance and portable evidence

The exact retained CPU binary is
`9d879afb242a73049e62c5a1ac85c2b701e3c7b070148605cb62b6e8339a28a0`.
Its [original build proof](../pr02-compound-aabb-captures-2026-10-01/raw/cpu-viewer/receipt.json)
and dependency source audit remain preserved. Every actual compiled source and
object matches. Only broad snapshot entries for the two recording wrappers and
`both_dual.c` are excluded: none is compiled or linked into that CPU viewer.
No new CPU build or changed binary is claimed.

The new native combined binary is
`b61d01e3fc71105173b3d22fc6d9830ecb0813122331edd4a31bfad2aa52f3b5`.
Its [build receipt](raw/native-viewer/receipt.json), logs and source archive pin
178 inventoried translation units, source/object hashes, actual link command,
unchanged before/after source/dependency snapshots and exact Rust library.
The shared cached NFD object is disclosed; the [link-input audit](raw/link-input-audit.json)
checks its bytes against the linked archive. This is not a clean final release
build or complete historical dependency-header proof; PR06 remains required.
Invocation checkout metadata is context, not binary provenance.

Physics uses the actual RTX4070SUPER NVIDIA Vulkan adapter, driver610.57.04.
Rendering is1280×720 Mesa software OpenGL on isolated Xvfb, encoded30FPS; NVIDIA
opaque pose import falls back to host pose mirrors because renderer/device UUIDs
differ. Recording/diagnostic clocks, encoded frame rate and on-screen FPS are
excluded from headline performance. Actual viewer interaction is unqualified.
The final fresh desktop-only chart campaign belongs to PR08/PR12.

The comparison grid retains the real CPU column first, then the latest combined
column (CPU left/GPU right inside each clip). Six scoped ownership rows preserve
older rows/clips/datasets. Append-only recording manifests store chronological
launch timestamps; timestamps are not build proof. Generator row additions occur
only after captures and change no compiled viewer or clip input.

Run the offline audit from any working directory:

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr02-compound-ownership-captures-2026-10-01/validate.py
```

It validates99 portable raw files, original build/source archives, all12 health
records, exact clip hashes and codec properties without launching physics.
[Raw index](raw-index.json) identifies every retained file. Review sheets sample
five fractions of each completed scene window; their times are approximate
presentation fractions, not synchronized physics-frame comparisons.
