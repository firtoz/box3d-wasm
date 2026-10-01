# Compound AABB candidate rejected at compilation

The initial public/body/native bounds candidate is rejected before any API or
GPU validation. Both ordinary and native library compiles exit101 with Rust
E0282 at `parts[destination].push(host)`: the grouping map value type is ambiguous.
All twelve planned validation processes and both test compiles are unlaunched.
Seven production files were restored byte-for-byte to their recorded baseline.
No physics or timing result exists for this candidate.

The [protocol](protocol.json) froze one candidate and a finite validation budget.
An initial pre-build protocol and source refinement are retained: source audit
showed transformed hull/mesh boxes cannot generally be recovered from their
baked vertices, so the unevaluated candidate preserves the imported native
compound local source box in existing parent center/half fields. That refinement
happened before any compiler or trial execution. It did not apply the earlier
rejected CPU ownership routing change.

The launcher incorrectly proceeded to native after ordinary compilation failed.
This is a sequencing deviation from the stop rule. Both failures/logs are kept;
no unfavorable result is removed or replaced. A guard added too late is preserved
separately. The initial launcher is explicitly reconstructed from the original
freeze template and path replacement, not misrepresented as a saved execution
snapshot. Actual compiler commands and logs are authoritative in the receipts.
Later orchestration gates each subprocess synchronously.

[Raw index](raw-index.json) retains the protocols, stop record, both source
snapshots/build receipts/logs and baseline/rejected production archives. The
[separate compile-only repair](../pr02-compound-aabb-compile-2026-10-01/README.md)
adds an explicit `HashMap<ShapeId,usize>` annotation. It is a compiler requirement,
not replacement API acceptance under this closed budget. The first GPU/API
validation is separately frozen against actual successful compiled inputs; this
failed campaign remains failed and its unlaunched cases remain unlaunched.

Run the [later offline validator](../pr02-compound-aabb-validation-2026-10-01/validate.py)
to verify all three datasets without launching physics. Preparing a build or
report does not qualify PR02 or production readiness. Box3D/WASM remain unchanged.
