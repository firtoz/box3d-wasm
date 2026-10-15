# PR09: complete native sample comparison and verified repairs

**PR09 verified:165 native samples reviewed/0 left;21 fixes and1 resolution without code repair.**

The inventory reconciles165active native C registrations,174browser catalog entries and20CPUoracle fixtures. Native comparisons use real Box3D CPU first, ordinary/native GPU and both combined routes. Thirteen browser entries have no native counterpart; browser runtime remains outside this native qualification. The CPUoracle is a nonvisual JSON/binary producer, so its20entries have an explicit nonvisual disposition and retain numerical qualification underPR03/PR04.

[Complete inventory](data/inventory.json), [all165original per-sample findings](native-review.md), [acceptance matrix](acceptance.json), [verification receipt excerpts](data/verification-summaries.json), [CPU-first Rain before/after clips](compare/index.html).

Repairs restore heightfield and combined ground drawing, visibility/sensors, sample primitives/text/material/state colors, queries, contact creation, far-world support, capacity and bounded progression/performance. Geometry/defaults/identities/picking are preserved under each report’s explicit checks. Existing failed hypotheses and full affected-scene recording protocols remain in the local audit archive; this selected bundle keeps representative original before/after media and compact evidence, without binaries/caches/fullhealth dumps. No new visual or solver experiment was run for publication.

| Group | Result | Report |
|---|---|---|
| VIS03 | Verified repair | [Before/after and checks](reports/VIS03.md) |
| VIS01 | Verified repair | [Before/after and checks](reports/VIS01.md) |
| VIS05 | Verified repair | [Before/after and checks](reports/VIS05.md) |
| VIS06 | Verified repair | [Before/after and checks](reports/VIS06.md) |
| VIS07 | Verified repair | [Before/after and checks](reports/VIS07.md) |
| VIS08 | Verified repair | [Before/after and checks](reports/VIS08.md) |
| VIS02 | Verified repair | [Before/after and checks](reports/VIS02.md) |
| QRY01 | Verified repair | [Before/after and checks](reports/QRY01.md) |
| DIAG01 | Verified repair | [Before/after and checks](reports/DIAG01.md) |
| BEH01 | Verified repair | [Before/after and checks](reports/BEH01.md) |
| BEH02 | Verified repair | [Before/after and checks](reports/BEH02.md) |
| BEH03 | Verified repair | [Before/after and checks](reports/BEH03.md) |
| CAP01 | Verified repair | [Before/after and checks](reports/CAP01.md) |
| RUNTIME01 | Resolved without code repair | [Before/after and checks](reports/RUNTIME01.md) |
| RUNTIME02 | Verified repair | [Before/after and checks](reports/RUNTIME02.md) |
| RUNTIME03 | Verified repair | [Before/after and checks](reports/RUNTIME03.md) |
| PERF01 | Verified repair | [Before/after and checks](reports/PERF01.md) |
| PERF02 | Verified repair | [Before/after and checks](reports/PERF02.md) |
| PERF03 | Verified repair | [Before/after and checks](reports/PERF03.md) |
| PERF04 | Verified repair | [Before/after and checks](reports/PERF04.md) |
| PERF05 | Verified repair | [Before/after and checks](reports/PERF05.md) |
| PERF06 | Verified repair | [Before/after and checks](reports/PERF06.md) |

## Open release gates

Closing PR09 routes these findings; it does not mark them passed or certify production readiness. User-selected option1 closes Rain’s verified timeout with the existing physical failures retained underPR04.

| Gate | Remaining work |
|---|---|
| PR04 | Rain original120:18anchor/27angular/249cone,0/0twist FAIL; older326physical failures/loadeddragging/CCD/energy/support, FarPyramid evolvedcollapseunconfirmed. |
| PR10 | Sample widgets/input/pause/restart/resize/manualdragging/picking and original camera/panel occlusion limits. |
| PR11 | Full independent raycast/query matrix, transformed geometry, other existing numeric watches; QRY01 repair is not whole-gate certification. |
| PR05_PR07 | Full lifecycle/complete-state/repeatability qualification beyond scoped preserved checks. |
| PR08_PR12 | Release native performance, otherdevices/coldstart claims. Browser/recording/player remain outside initial native qualification. |

[Far Pyramid setup triage](reports/FarPyramid.md) shows shared float32 placement creates200coincident cube-input pairs; evolved collapse remains unconfirmed, not another counted repair. Original physical tolerances remain unchanged. Exact pixel and chaotic CPU/GPU scene trajectory matching are deferred.

## Source and storage

The implementation/source and selected media hashes are in [manifest](manifest.json). Before/after images and10Rain clips are byte-identical to retained recordings; full recordings are available locally at their original archive paths. Source builds and individual repair checks were already completed with the exact retained implementation. This publication checks literal compile-time includes, Python syntax, staged changes and portable local links, preserving Box3D/WASM. Three exact recorded regression inputs are explicitly included: the compile-time QRY01 raycast cases and PERF02 mesh/cast data (14,278 casts). These are required by existing Rust tests in a fresh checkout; raw run directories remain excluded.

Next roadmap item isPR10UI/widgets; Rain physics staysPR04. No PR10 or old solver/scheduling campaign starts as part of publication.
