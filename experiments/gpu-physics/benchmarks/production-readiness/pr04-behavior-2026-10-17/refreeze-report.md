# Current policy release inputs

Gate F is complete for the retained policy source published as `8d18dc8b6`.
[Acceptance and all 20 artifact identities](refreeze-acceptance.json) record the
new candidate under `experiments/gpu-physics/artifacts/pr04-release-candidate-2026-10-17`.
This closes release-input reconciliation, not Rain or production readiness.

Two initially empty Rust targets compile offline/locked with the original
release features and two compiler jobs. Four initially empty native builds
compile GPU/combined viewers for ordinary/native backends. Dependency source
caches are pinned and reused; their compiled objects are fresh inside each owned
build directory. The unchanged CPU executable/archive are carried from the
original clean PR06 build, with source applicability checked. Rust dep-info,
actual feature/profile records, native compile commands, header/source hashes
and generated sources identify the compiled inputs. No experiment feature is
present and no production source changes during this qualification.

The original five external native consumers run with real Box3D CPU first.
Create/step/read/ray/counts/destruction checks pass on all four GPU routes at
absolute1e-5, maximum error0. All four original callback controls pass. Nine
viewer checks pass18measured frames,10single steps,0idle advances and1scene switch
each; NVIDIA Vulkan and actual enabled/disabled pipeline-cache behavior are
verified. Those lifecycle timings are not an SPS qualification campaign.

The workspace TypeScript/demo and WASM source paths are unchanged. Their
original explicit-SDK/typecheck/build/runtime evidence remains source-applicable;
there is no new WASM build or modification. Old candidate artifacts and failed
receipts remain unchanged. Later relevant engine changes invalidate affected
release evidence again; PR07/PR08 final campaigns remain separate.

Owned build trees, consumer executables/copied inputs and viewer working
directories are deleted after their consumers. The new20-file candidate totals
367,034,636bytes, within the frozen512MiB exception; its named consumers are
current-source qualification and PR07/refreeze. The temporary allowance was6GiB,
with64MiB retained diagnostics and20GiB free-space floor. Exact cleanup/cache
measurements and terminal process verification are in the final checkpoint.

PR04 is **3 gates closed, 3 open**: C,D,F closed; A,B,E remain incomplete.
Rain/performance3/3 and dragging3/3 investigation budgets remain spent. The
optional additional Rain batch still requires a human budget decision.
