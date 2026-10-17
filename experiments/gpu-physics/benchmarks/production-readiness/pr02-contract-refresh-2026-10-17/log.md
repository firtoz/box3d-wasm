# PR02 native API contract — recovery

Read this FIRST on continuation, then AGENTS.md and docs/goals/gpu-production-readiness.md. Use the goal-scratchpad skill.

## Objective and authorization

User requested a commit of verified PR11, then PR02 on2026-10-17. PR11 committed as17e3575a7; push not requested. PR02's named native API/truthful-control contract is now complete. The user subsequently authorized committing this PR02 refresh. This record is included in that commit; identify it with git log -1 -- this path. No later item is activated. Preserve original tolerances, local work, Box3D and WASM. New automation isTypeScript/Bun. No solver repair, old exhausted experiment, all-sample sweep or mandatory recording.

## Acceptance — 5 done / 0 left

| Gate | Status and evidence |
| --- | --- |
| G1 Current source/link/default inventory | PASS: inventory.json,415stateful/376candidate/39excluded+17defaultaliases perroute; no missing/duplicate/CPU-only stateful providers |
| G2 CPU/GPU defaults/masks/identities | PASS: seven configurations,34query/identity checks each; mask0/1/2/bit63/all,perworld mutation/deletion/replacement and stale IDs |
| G3 Named contracts and exclusions | PASS:32original Cfixtures+4error controls,plus16API/2default cases using native launcher policy |
| G4 Current source/evidence reconciliation | PASS: source-applicability.json; actual C fixtures execute current Rust; prior Rust/UI evidence stays historical and scoped |
| G5 Docs/integrity/cleanup | PASS: acceptance.json; source/library/outputs verified,temporary raw absent,no owned job,focused TypeScript check passes |

## Results and limits

[Report](README.md),[acceptance](acceptance.json),[inventory](inventory.json),[source reconciliation](source-applicability.json). PR11 repaired the actual default-filter alias; this task adds its independent regression and closes the API gate. All59fresh processes pass:36initial contracts,5unlaunched filter cases and18distinct native-policy cases. Current ordinary/native libraries exactly match PR11 source/library hashes. No production Rust/WGSL/C adapter, Box3D or WASM changes.376implemented symbols are candidates, not376independent physical passes. Only filter/shape/query filter defaults receive new independent default-behavior checks. Recording/player and CPU worker/static-tree operations remain unavailable; wheel angular separation retains the explicit upstream release-zero fallback. PR04Rain/dragging/CCD, broad lifecycle/concurrency, final builds/repeatability/performance remain open.

## Closed budgets and failures

- protocol.json:4incremental adapter builds (90s/jobs2),zero Rust/viewer builds/candidates/captures. Original batch closed after112.956s,36runtime passes,then one compile-only fixture error (missing header-required invokeContacts argument). No filter runtime failed. runs.json remains FAILED at that exact boundary.
- remaining-protocol.json: only5unlaunched filter cases,180s total/zero builds; PASS11.493s. Corrected argument false. No completed result repeated.
- First inventory parser required B3_API on upstream definitions and falsely omitted b3DefaultDebugDraw. Original findings/hash retained in inventory-parser-failure.json; duplicate rows removed after consumer finished. Corrected offline inventory PASS2.234s.
- Initial native-library runs used default runtime settings, not the supported launcher policy. Retain that distinction. native-policy-protocol.json freezes18previously untested policy cases,240s/zero builds; PASS53.371s. Explicit launcher settings recorded in every receipt.
- Two continuation log paths initially collided. Exact original compile/size bytes were recovered from displayed output/recorded values and verified against original SHA256 receipts. Successful logs moved to unique remaining-* paths. No result/digest changed; runner now prefixes all followup logs.
- Focused typecheck and final offline validation each bounded30s; no further engine launches. Original failure logs,protocols and all output hashes remain.

## Storage and process state

No live job or raw directory. Executables consumed one at a time (<=64MiB estimate) and deleted immediately: 1992568992cumulative bytes written/deleted, never accumulated together. Shared adapter growth80300bytes; measured pipeline growth43276bytes and NVIDIA cache growth3696324bytes during the task (other application activity not attributed). Shared caches/current builds preserved. Retained evidence size/free space are in acceptance.json; free space exceeds20GiB. Task raw path is locally excluded.

## Next action

Stop. PR02 is checked in the authoritative roadmap. PR11 commit is17e3575a7; PR02 regression/audit/docs are included in the commit containing this record, authorized after acceptance. Push and PR04 were not requested. Do not resume historical runs or expand into solver work implicitly.
