# Compound AABB repair: source applicability review

The imported box is `b3ComputeCompoundAABB(compound, b3Transform_identity)` from
real source compound data, before transformed hull baking loses its tree-box
semantics. Both standalone and combined adapters retain it in the GPU public
parent. No borrowed compound pointer is retained. The setter validates finite,
ordered bounds and the live public parent generation/kind. Existing parent
center/half fields carry the data and are already present in semantic captures.

Queries group private children by public owner. They transform the imported
local enclosing box once and add the unchanged speculative distance. A Rust
parent created without native compound input derives its local box from its
children. Disabled direct queries stay available; invalid handles return default
zero bounds. Body queries union public shape groups, retaining body origin for
an empty body. Noncompound queries retain their previous calculation.

Native diagnostics synchronize and copy owned records under the world lock;
C visitors still run after the lock is released. Compound records and public
queries share their bounds calculation. `samples_api.c` routes native world bounds
to the diagnostics/scene inventory; native Frame Camera/Home consumes world/body
bounds in sample.cpp. These call paths make the affected Compound captures
relevant, although passive capture does not qualify Frame shortcut interactions.
Those required control interactions remain open under PR02.

`ensure_sim` continues replacing public GPU proxy center/half with
collider-derived topology boxes. Imported query metadata does not alter solver
or broadphase geometry. No WGSL, solver schedule, material/default, timestep,
Box3D submodule or WASM changes are included. Diagnostic grouping introduces
additional host work; its cost is unmeasured and must be assessed under PR08.

Existing duplicate CPU child creation in the combined primitive compound adapter
remains unchanged. The twelve API validation processes explicitly do not qualify
those counts. Compound scalar getters still need independent qualification.
No all-scene correctness, complete repeatability or production-ready claim is
made by the AABB fixture or passive clips.

The precommit recording protocol pins the exact new GPU viewer and a fresh real
Box3D CPU viewer. It checks all actual compiled translation-unit source/object
hashes and recorded inputs. Only the two recording shell entry points changed
after the viewer builds, and are explicitly excluded because viewers do not
compile them. The new recorder, wrappers, native-cache runtime configuration
and upstream data assets have separate frozen hashes. Third-party translation
units are supplemented with exact source bytes matching recorded build hashes;
their clean pinned header trees are a post-build audit. PR06 must produce its
own complete final-build dependency proof rather than treating this supplement
as a before/after header receipt.
