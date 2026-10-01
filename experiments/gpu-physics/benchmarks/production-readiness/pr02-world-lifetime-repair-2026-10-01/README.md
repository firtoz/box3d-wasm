# World lifetime repair: API subset evidence

The candidate rejects stale and forged root-world handles on both Vulkan
backends and keeps valid body, shape and joint operations working after world
recreation. Stale C destruction also preserves replacement geometry in standalone
and combined configurations. **The production source changes are still local,
awaiting actual viewer qualification and CPU-first precommit recordings. PR01 and
PR02 remain open. This is neither final release qualification nor timing data.**

The source base is `88d8d532e02a2559b665bb0f73aaabd88b7553d7`. Candidate bytes,
compiler artifact/depfile audits, library/test/fixture hashes, C compile commands,
archive member replacements and adapter/driver evidence are portable here.
Checkout revision alone is not binary provenance. Original diagnosis remains in
[the preceding report](../pr02-world-lifetime-diagnostic-2026-10-01/README.md).

## Retained behavior and scope

Root epochs survive world destruction, and destruction validates the full ID.
A replacement's child epoch exceeds every body generation issued by its
predecessor, including recycled or already destroyed bodies. Shapes and joints
append within a world using that child epoch. Body recycling uses checked
increments; exhausted body/world slots retire instead of wrapping a public
identity. Root indices retain their existing ABI convention.

Child routing captures the live root epoch without recursively locking the
world mutex. Locked accessors revalidate that captured root; fixed-epoch shape
and joint routes also reject wrong child generations. All nine joint constructors
validate both endpoint ownership and lifetime. Existing process-wide contact
handle allocation is retained. New C destruction guards execute before geometry
or visual cleanup; matching combined CPU mappings can still be cleaned after a
GPU world is lost. Compound errors now derive the actual root through
`b3Body_GetWorld`.

No solver arithmetic, WGSL, timestep, scene defaults, scheduling policy, Box3D
source or WASM changes. The selected checks do not qualify every concurrent
operation, every C capacity boundary, full semantic repeatability or all supported
physical scenes. PR05 must reconcile the raw Rust world-index range with native
metadata/viewer caps. PR07 must account for the new child epoch in future-relevant
allocation state. The later [direct mapped-CPU case](../pr02-world-lifetime-mapped-cleanup-2026-10-01/README.md)
passes42 observations after deliberately retiring only the GPU object. This is
adapter cleanup evidence; it does not qualify actual device loss.

## Outcomes, including failures

| Campaign | Frozen budget and outcome |
| --- | --- |
| Initial candidate build | Four Rust builds, two lifetime processes, sixteen regressions and four C root builds/runs planned. Ordinary library/test compile both pass, then receipt script collides with the `ordinary-tests` log directory. Stops and restores production. Native builds and all validation cases unlaunched. |
| Artifact receipt repair | Zero ordinary compiler repeats; recover byte-identical library/test artifacts. Two previously unlaunched native builds pass on identical Rust/test inputs. An earlier setup path-prefix error occurs before any build/source mutation and is retained. |
| First Rust validation | Two new lifetime and sixteen existing regression processes, fresh processes/one test thread. All18 processes/72 tests pass on actual NVIDIA Vulkan, driver610.57.04. Existing tolerances/assertions unchanged. |
| First C cleanup build | Two standalone changed shim units/archives pass. Third API generation reveals the combined shim differs by two blank lines after overridden functions are removed. Strict source applicability stop; no fixture linked or process run. Production restored. |
| Exact combined source build | Reuse standalone artifacts/ordinary combined generation; one first native combined generation, four first combined unit compiles/archive replacements and all nine fixture links pass. First CPU control stops with10 observations/one mismatch: the fixture incorrectly requires native stale-child rejection across world recreation. All eight GPU cases unlaunched; production restored. |
| CPU contract correction / first GPU execution | Validate existing CPU observations offline against inspected native source; no CPU rerun. GPU-preprocessed fixture bytes are identical before/after the CPU-only correction. All eight previously unlaunched GPU processes pass120 observations, retaining stronger GPU stale-child rejection. Zero compiler/Rust/CPU repeats. |

The independent CPU record shows the old world ID remains invalid, but
`b3Shape_IsValid` can report the old shape as valid after recreating its world.
Native `physics_world.c` checks child slot/generation without the root epoch;
`shape.c` increments the newly initialized shape generation. This is recorded
as a native reference limitation; Box3D stays unchanged. No invalid CPU mutator
or destructor is invoked. The original CPU stdout, false assertion and exit1
remain failed. The later offline contract recognizes the observed native alias;
it neither converts that raw run into a pass nor relaxes a GPU assertion or a
physics tolerance.

The Rust subset passes three new lifetime tests per backend plus unchanged
joint metadata1, compound properties1, queries18, geometry replacement1,
contact determinism7, contact patch order1, speculative controls2 and native
diagnostics2 tests per backend. Lifetime coverage includes recycled generations
above the root epoch, stale root mutation/destruction, stale child mutation and
destruction, all nine valid post-recreation joint types, foreign/stale endpoint
rejection, one ordinary completed four-substep `dt=1/60` step, and allocator
exhaustion boundaries. This does not imply broad physical acceptance.

Each of ordinary GPU, native GPU, ordinary combined and native combined passes
17 cleanup observations plus all13 original root-world diagnostic observations.
Cleanup covers four owned hull mirrors across separate two-hull compounds;
stale/forged world and parent destruction preserves them, valid parent
destruction releases only its owned mirrors, and final cleanup releases all.
Root diagnosis mismatches previously6 per configuration are now0. No diagnostic
GPU timings or headline performance runs were made.

## Provenance and reproduction

[Raw index](raw-index.json) hashes every portable raw file. Failed campaigns,
immutable protocols, drivers, exact candidate/baseline snapshots and generated
C sources are retained. Executables, archives and object files are not committed.
Receipts store full hashes and exact commands/environments for those local
artifacts. The ordinary test executable is recovered from Cargo's successful
compiler artifact, with byte equality to the already copied executable. Both
root Rust artifacts are freshly compiled and their depfiles include the new
lifetime module, matching the archived inputs.

Cargo dependencies and the four unchanged native C hull-cooker units are cached;
the post-build artifact/depfile audit is explicitly **not** a clean PR06 build or
a before-build dependency/header freeze. C adapters reuse audited unchanged
objects/archives from the
[current-source build report](../pr02-controls-current-builds-2026-10-01/README.md).
Changed units are compiled with recorded original flags into new archives;
old source, objects, archives and reports remain intact. The final delivery needs
its own reproducible PR06 build/refreeze and applicability review.

Validate this portable report without a GPU or local binaries:

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-repair-2026-10-01/validate.py
```

Do not rerun archived campaign drivers. Local frozen artifacts are under
`artifacts/production-readiness/pr02-world-lifetime-*`; exact paths and hashes
are in the receipts. Next: fresh viewer build receipts, remaining actual GPU controls, and applicable CPU-first recordings
before committing the production repair. Floor visibility, sample-wide widgets,
raycast comparisons and fresh desktop-only CPU/GPU charts remain PR09–PR12,
after the core gates. Laptop results remain deferred.
