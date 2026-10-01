# Combined compound ownership: rejected candidate

The first candidate removes duplicate CPU sphere children at creation, but does
**not** qualify: the rotated-compound bounds comparison fails at the unchanged
absolute `1e-5` tolerance. Production `c_abi/both_dual.c` is restored byte-for-byte
to the baseline. PR02 remains open. This campaign contains no timing data or
performance claim, and does not qualify the later physical release gates.

The immutable [protocol](protocol.json) froze two baseline processes, one
production candidate, four candidate ownership processes and eight existing C
regression processes. Both baseline processes complete as defect reproductions;
all ten candidate fixtures build, then the first ordinary combined process exits
`-6`. The stop rule leaves three ownership and eight regression processes
unlaunched. Nothing is repeated or replaced. The candidate cannot be accepted
from its first matching count.

| Configuration/result | Observed evidence |
| --- | --- |
| Ordinary/native baseline | Each primitive compound adds two extra CPU shapes; first creation is GPU1/CPU3. After parent and individual shapes are deleted, CPU orphan counts are2,4,6 across three cycles. Mesh compounds have no extra CPU shapes. |
| Ordinary candidate, first sphere creation | GPU1/CPU1; bounds lane0 then fails: GPU `-1.57911253`, mapped CPU `-1.6875484`. |
| Native candidate and remaining repetitions/regressions | Unlaunched under the stop rule; builds alone are not acceptance. |

The fixture covers sphere/capsule/hull/mesh compound definitions, default world
and material settings, a static rotated body, three create/delete cycles,
public individual constructors, mapped CPU handles, exact public counts and
independent bounds. Candidate execution stops before the other child kinds,
public individual constructors and deletion/recreation. Their intended coverage
must not be described as a candidate pass.

## Source diagnosis and next requirement

Baseline compound sphere/capsule/hull children call the public dual constructors,
which mirror each child into CPU Box3D. The constructor then creates a whole CPU
compound as well. Parent deletion destroys only the mapped whole compound. The
raw baseline counts independently demonstrate the resulting orphan colliders.
The rejected candidate factors private constructors with an explicit CPU-mirror
argument; compound children pass false, public constructors true. It changes no
Rust/WGSL or mesh creation code and uses no global/thread-local mode guard.

The failed bounds comparison exposes a separate semantic gap. Pinned Box3D
`b3ComputeCompoundAABB` reads the compound tree's local enclosing AABB and calls
`b3AABB_Transform`; `b3UpdateShapeAABBs` adds speculative padding, and
`b3Shape_GetAABB` returns that stored parent box. GPU native diagnostics instead
compute each child's world AABB, pad it, then union the children. Transforming a
local enclosing box and unioning transformed child geometry are different for a
rotated compound. This explains the mismatch by source inspection; the failed
raw run prints only lane0, not a full six-lane geometric proof.

The public GPU `b3_shape_get_aabb` has another source-demonstrated gap: it obtains
`query_shape(parent)` and computes the zero-sized placeholder parent box, rather
than resolving `query_shape_parts(parent)`. The combined public C wrapper calls
that same GPU API. Native merged diagnostics therefore cannot serve as proof
that public compound AABB semantics match Box3D.

Next, qualify public/extended compound bounds against independent CPU geometry
in a separately frozen diagnostic protocol, including rotated and unrotated
spheres/capsules/hulls/meshes. Keep the original `1e-5` physical tolerance. Resolve
the required public bounds contract before reconsidering ownership changes.
Do not rerun or extend this rejected ownership campaign under a new label.

## Portable provenance and retained results

[Raw index](raw-index.json) hashes every retained file. [Offline validation](validate.py)
checks receipts, frozen protocols, source snapshots, actual linked-input/binary
identities and all completed logs without launching physics. The files include
all two baseline and ten candidate build logs, all three executed processes,
the exact baseline/rejected source, scripts, and source archives.

`revision_context` identifies the invocation checkout only. Baseline provenance
comes from the prior [compiled adapter and engine receipts](../pr02-diagnostics-2026-10-01/README.md),
whose hashes and linked inputs are pinned in this protocol. Candidate archives
replace only `both_dual.c.o`, compiled with the original CMake flags
`-O3 -DNDEBUG -std=gnu17 -w`. Archive build receipts record source hashes before
and after, source archive hash, command/toolchain/log, changed object and archive
hashes, and unchanged `both_map.c.o` / `both_passthrough.c.o` member hashes. The
fixture receipts record every linked archive, compiler command and executable
hash. Both backends use their unchanged byte-verified frozen Rust libraries.
Assertions in C++ fixtures remain enabled.

All executed processes select actual NVIDIA RTX4070SUPER Vulkan with
CPU-compatible ordering; native settings are captured exactly in the receipt.
Environment/driver details are in [environment.json](environment.json).
Diagnostics, builds and this correctness campaign are separate from headline
measurement. Box3D, WASM, solver defaults and prior datasets remain unchanged.
Because the production candidate was restored, no new affected-scene recording
is used to justify retaining or committing it.

Historical commands are retained in receipts and scripts; they are evidence of
what ran, not permission to rerun the closed budget. Local executables/archives
remain under `artifacts/production-readiness/pr02-compound-fix/`; the reviewable
raw evidence and compiled-source snapshots here do not depend on those ignored
binaries. Reproduction of a future requirement must use its own justified frozen
protocol and current compiled inputs.
