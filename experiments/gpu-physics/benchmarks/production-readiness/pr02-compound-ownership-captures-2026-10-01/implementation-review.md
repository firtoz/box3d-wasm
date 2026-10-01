# Combined compound ownership: source applicability and scene review

The five affected scene constructors in `box3d/samples/sample_compound.cpp`
create baked static compounds: Simple (one hull), Spheres, Hulls, Tile Floor
(hulls), and Village (hulls, capsules, spheres and building meshes). Mesh Tile
is an unchanged mesh-only control. These are every upstream caller of
`b3CreateBakedCompoundShape` in the native sample suite; no other sample scene
constructor calls it. Geometry, assets, materials, camera and defaults remain
upstream. Village retains its complete building mesh asset, 52,500 compound
children and no diagnostic drop.

The ownership candidate only changes private combined-adapter primitive
creation. Public sphere/capsule/hull/transformed-hull constructors still request
CPU mirroring. A single CPU compound is created and mapped to the public GPU
parent after GPU children attach. Mesh cache behavior and imported source-local
bounds are unchanged. The correctness campaign directly checks both mapped
CPU and public GPU populations/lifetimes; passive recordings do not replace it.

The frozen recording driver now accepts an explicit `health_mode` in a viewer
receipt reference. The combined viewer reports `both`; the recorder does not
mislabel it as a standalone GPU binary. The new clips show synchronized CPU on
the left and GPU on the right inside each combined viewport. The comparison
column still follows the independent real Box3D CPU reference column. Every clip
preserves 300 completed steps, enabled sleep/warm-start/CCD, four substeps,
1/60 timestep and source scene defaults. Empty isolated settings suppress only
first-run help/replay redirect; `--hide-ui` hides widgets. These captures cannot
qualify actual widget input, scalar getters, physical trajectory agreement or
performance. Mesa software OpenGL draws on Xvfb, while physics uses NVIDIA Vulkan.

The CPU viewer is exactly the prior independently built binary; every actual
compiled source/object still matches. Three broad snapshot inputs changed but
are not compiled/linked into CPU: the two recording wrappers and `both_dual.c`.
The native combined viewer is newly built and pins the candidate, actual source/
object/link command and before/after production/dependency inputs. CMake may
reuse its shared NFD cache; this is disclosed, not a clean final-release build.
The CPU dependency headers remain the previous post-build audit. PR06 still
requires an independent clean final release build with complete dependency proof.

All12 clips/300-step health checks completed; all six five-fraction comparison
sheets are reviewed. Simple, Spheres, Hulls, Tile Floor and Mesh Tile show the
expected static geometry in both views. Their blue capsule presentation/trajectory
can differ; no exact physical equivalence is inferred. Village is a retained
visual failure: nearby compound ground/building geometry is visible in the CPU
view and absent in the GPU half. The renderer reports a65536-slot shared debug
pool; two52500-child compounds need at least105002 slots before dynamic shapes.
AllocDebugShape returns-1 at capacity, and compound flattening silently skips
those children. The capacity source is byte-identical to pre-candidate code;
private GPU render callbacks are unchanged by the ownership candidate, which
only reduces duplicate CPU mirroring. This is an existing combined rendering
capacity gap, now recorded under PR09, not a new ownership regression or a
visual-parity pass. Physics-side loss/NaN/health checks still pass all300steps.
See village-renderer-finding.json and the portable sheet/clip.

Startup/loading footage is retained; review sheets select five fractions of each
completed scene window. The postcapture comparison generator adds six scoped
rows without changing clips, frozen viewer inputs or older datasets. Real CPU
stays first; the new native combined column follows it. Neither source review
nor passive clips qualify native UI interactions. Rendering uses software Mesa;
opaque NVIDIA pose import is skipped due to UUID mismatch and the existing host
pose-mirror fallback is used. No rendering FPS/performance claim is made.
