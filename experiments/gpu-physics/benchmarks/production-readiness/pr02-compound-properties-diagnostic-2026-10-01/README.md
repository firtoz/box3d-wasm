# Compound scalar/material contract diagnosis — desktop, 2026-10-01

Five first processes completed the frozen diagnostic. Real Box3D CPU matched all 928 observations. Each standalone GPU backend mismatched 864 of 1,016 observations; each combined backend mismatched 832. These are retained failures, not API acceptance. No timing was measured and no production source changed in this campaign.

| Configuration | Observations | Mismatches |
| --- | ---: | ---: |
| Box3D CPU control | 928 | 0 |
| Ordinary GPU | 1,016 | 864 |
| Native cached GPU | 1,016 | 864 |
| Ordinary combined | 1,016 | 832 |
| Native cached combined | 1,016 | 832 |

The immutable fixture covers sphere, capsule, hull and mesh compounds with two geometry-owned materials deliberately different from `shapeDef.baseMaterial`. It checks creation, valid parent density/userdata changes, a completed static-world step at 1/60 with four substeps, independent worlds, deletion and a second lifetime. Float comparisons retain absolute 1e-5; opaque tags/counts are exact. Invalid CPU handles are never queried. GPU stale-handle observations pass, but this C fixture does not assert that the same parent slot was reused, so it does not prove ABA protection.

Friction, restitution and userdata getters wrongly require conversion to spatial `HostShape`, which rejects public compound parents. The GPU parent also lacks the geometry-owned material table: its surface getter reads shape-definition metadata and indexed getters return defaults. Combined material count uses the CPU mirror and therefore passes, while the other getters still read incorrect GPU metadata. Density and stale observations pass. Both backends reproduce the same results.

The campaign consumes CPU control1/GPU diagnostic4/candidate0/timing0/retries0 in [protocol.json](protocol.json). All nine build commands and five process exits are retained in [raw/receipt.json](raw/receipt.json), together with exact binaries, linked-input hashes, adapter/driver, settings, source snapshots and stdout/stderr. Native Vulkan uses the actual NVIDIA RTX 4070 SUPER, driver610.57.04. Ordinary/native Rust libraries reuse exact [compile receipts](../pr02-compound-aabb-compile-2026-10-01/README.md); combined adapters reuse exact [ownership build proof](../pr02-compound-ownership-after-bounds-2026-10-01/README.md). Standalone adapters and all five fixture binaries are newly built. Checkout revision is context only, not binary identity. The fixture source and generated adapter inputs are archived, portable logs contain the numeric failures, and local executables are intentionally omitted.

Run `python3 validate.py` from this directory for an offline hash/source/result audit. It starts no engine process. The closed runner under `raw/` is a reproduction recipe, not permission to rerun this budget. Production/API acceptance and any repair validation are separate campaigns; the older failed experiments remain preserved.
