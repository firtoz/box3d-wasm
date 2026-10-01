# Repaired lifetime: actual viewer control qualification

All four repaired diagnostic viewers pass the original limited PR02 control contract: ordinary/native GPU and ordinary/native combined. **The seven production repair files remain local and uncommitted pending CPU-first scene recordings. PR01/PR02 and final readiness remain open.** No timing or default-policy decision occurs here.

| Configuration | Actions | Screenshots | Observer records | GPU restart | CPU restart |
| --- | --- | --- | --- | --- | --- |
| Ordinary GPU | 29 | 11 | 1991 | [1,1] → [1,2] | — |
| Native GPU | 29 | 11 | 1623 | [1,1] → [1,2] | — |
| Ordinary combined | 29 | 11 | 1226 | [1,1] → [1,2] | [1,0] → [1,1] |
| Native combined | 29 | 11 | 1569 | [1,1] → [1,2] | [1,0] → [1,1] |

The apps preserve Single Box geometry/defaults,60Hz and four substeps. Actual P/period input proves paused stability and exactly one completed step before and after restart. Sleep/Warm Starting/Continuous checkboxes turn off together, remain off at restart, then restore on. Context/public GPU flags agree throughout; combined mapped CPU flags also agree. The post-restart completed-step window contains a real moved-body event with position. M toggles metrics; agent visual inspection confirms actual Profile/Counters/Frame Time content and exclusions. Ctrl+Q closes all four apps/encoders cleanly. Upstream-supported rendering settings save correctly; no unsupported physics-settings persistence claim.

Profile/Frame Time show typed GPU timestamp phases and their step/physics identities; Counters shows public bodies/shapes/contacts/joints2/2/1/0 and completed-step island1. These are synchronized diagnostics, not completed-step benchmarks. Standalone viewers visibly state automatic GPU scheduling; combined views label CPU Workers8 and show real CPU left/GPU right. Recording is explicitly unavailable with GPU physics. Narrow Info text remains clipped and belongs to later PR10 layout qualification.

## Fixed provenance and retained failures

[Build proof](../pr02-world-lifetime-viewer-builds-2026-10-02/README.md) freezes each exact executable, linked Rust library/C archive,604 compiled source entries (124 per standalone and178 per combined), generated observer and command. The libraries come from the [passing API repair subset](../pr02-world-lifetime-repair-2026-10-01/README.md), with the [direct mapped-CPU cleanup case](../pr02-world-lifetime-mapped-cleanup-2026-10-01/README.md). Checkout metadata alone does not identify these binaries. Each app's stderr confirms NVIDIA GeForce RTX4070SUPER on Vulkan; [adapter receipt](raw/adapter.txt) records driver610.57.04/UUID/PCI. Graphics uses isolated1280×720 Xvfb/Mesa, so captures cannot establish desktop rendering FPS.

The [first coordinate campaign](../pr02-world-lifetime-viewer-apps-2026-10-02/README.md) stops after15 actions/five screenshots: new contact rows move Sleep from587 to627 and the earlier coordinate misses. The [adaptive campaign](../pr02-world-lifetime-adaptive-apps-2026-10-02/README.md) restores all flags after restart but stops at19 completed actions on an empty action20 during non-atomic publication. Both failed apps, raw receipts, pending action, screenshots and complete diagnostic captures remain preserved; neither becomes a complete pass.

The next distinct finite [protocol](raw/protocol-before-runs.json) permits one atomic-publication host process and four full-control apps, no CPU repetitions, production candidates, timing or retries. Same-directory temp+`os.replace` publishes actions and receipts. One host process passes501 writes/1747 concurrent reads with zero partial JSON. Each control batch requires a reviewed current screenshot/hash, including after lifetime changes. The original held-input/focus behavior and exact event-window evaluator/assertions remain unchanged; the portable verifier checks this source correspondence. Prior budgets stay closed. A later terminal-publication-only missing-import error is retained separately; it starts no app/test/source change and requires no rerun.

## Portable evidence and limits

[Raw index](raw-index.json) covers243 portable files: immutable protocol, driver/coordinator/helper/evaluator, host result, all116 action/state receipts,6409 observer records,44 original screenshots, labeled review crops, agent visual reviews, full four MP4s and terminal summary. Crops are derived review aids; original images remain authoritative. The real CPU controls from the earlier successful process are reused by exact proof/applicability, without another CPU run. Every app's completed-known field is true and submitted/completed counts match; combined CPU roots also change on restart.

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-atomic-apps-2026-10-02/validate.py
```

The verifier uses portable data and the unchanged original evaluator, without launching viewers. Local binaries/object archives are excluded from Git; exact hashes/source/build commands are in the linked reports. No Box3D/WASM, shader, solver arithmetic, scene-default or scheduling-policy changes are introduced by these harnesses.

This qualifies named Single Box controls only. It does not qualify sample-wide UI/widgets/layout, all physical scenes, actual device loss, concurrency/capacity boundaries, full-state repetition, raycast differences, floor inventory, rendering FPS or performance. Required precommit scene recordings/review are next; later roadmap gates remain open, including the requested floor/UI/raycast sequence and fresh desktop-only charts. Laptop validation/data remain deferred.
