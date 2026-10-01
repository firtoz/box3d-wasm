# PR02 world lifetime diagnosis — stale handles revive on both GPU backends

The real Box3D CPU control passes12 observations. Each ordinary/native standalone
and combined GPU diagnostic produces the same **six mismatches in13 checks**.
These are retained failures, never qualification passes. All five first processes
complete normally using the exact previously frozen current-source libraries/C
archives. No production candidate, engine edit, timing run or retry occurs.

| Case | CPU control | Every GPU cell |
|---|---|---|
| Recreate destroyed world | World ID changes; old remains invalid | Old/fresh ID both `[1,1]`; stale ID valid again |
| Stale getter | Invalid CPU getter deliberately skipped | Reads replacement userdata22 |
| Stale warm-start setter | Invalid CPU setter deliberately skipped | Disables replacement's default warm starting |
| Stale destroy | Invalid CPU destroy deliberately skipped | Destroys replacement world |
| Forged-generation destroy | IsValid false; invalid CPU destroy skipped | IsValid false, but destroy still removes live world |
| Independent second world | Valid, userdata33 preserved | Valid, userdata33 preserved |

The [new public C fixture](raw/world_lifetime_diagnostic.cpp) calls the actual
linked APIs. Safe CPU IsValid checks establish distinct world lifetime semantics;
invalid CPU reads/mutators are never called. GPU diagnostics deliberately probe
the demonstrated stale-ID failure. Every observation, ID and mismatch is in the
five portable stdout streams. Unchanged default world definitions are used;
userdata markers11/22/33 distinguish identities. No physics steps are needed for
this registry safety check, so it cannot establish physical correctness.

Source in the [current-source build archive](../pr02-controls-current-builds-2026-10-01/README.md)
shows `b3_create_world` initializes/reuses generation1; `b3_destroy_world` checks
only the slot index. `b3_world_is_valid` and ordinary setters check generation,
but reuse revives the identical old ID. A forged generation therefore fails
IsValid while destruction still succeeds. The [actual viewer restart](../pr02-controls-event-window-apps-2026-10-01/README.md)
first exposed ID reuse, and this independent fixture establishes its effects on
both backends and combined routing.

[Protocol](raw/protocol-before-runs.json):5 fixture compiles, CPU control1/GPU
diagnostics4,600s watchdog, candidate/timing/retries0. All programs build before
any process. CPU/infrastructure/provenance failure stops; GPU mismatches remain
planned diagnostic data. The [receipt](raw/receipt.json) records exact commands,
features/environments, NVIDIA adapter/driver, binary hashes, linked archives and
applicable compiled unit/object/source proof (50/54/54/107/107 C units). Rust
libraries and generated/dependency source proof are reused from the build report;
invocation checkout metadata is not binary provenance. Viewer observer, UI and NFD objects are not linked into these C fixtures;
recording APIs are not exercised.

PR01's per-world control/lifetime acceptance is **reopened**; its previously
passed physical checks and delivered toggle implementation remain preserved.
PR02 and PR05 also remain open. Next repair must retain world slot generations,
validate generation before destruction, and audit child-to-world routes currently
hardcoded to generation1. Audit stale body/shape/joint/contact identities across
world recreation and generation exhaustion rather than declaring root IDs a
complete lifecycle solution. Preserve physics defaults/order/tolerances and
existing checks; refreeze affected build evidence and record relevant scenes
before any applicable production commit. Keep this baseline/failures unchanged.

`python3 validate.py` verifies portable raw integrity, all exact checks/IDs,
original binaries/linked-source applicability and the six failures in each cell.
One offline publication-verifier syntax error is retained in raw evidence; only
conditional token whitespace was corrected, with no fixture/process rerun.
