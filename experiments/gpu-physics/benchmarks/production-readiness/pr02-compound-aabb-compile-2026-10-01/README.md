# Compile-only repair of compound AABB candidate

One explicit diagnostic-map annotation repairs the demonstrated Rust E0282
without changing the candidate's bounds arithmetic or imported geometry. The
[finite protocol](protocol.json) permits one source repair and exactly four
ordinary/native library/test compile commands; all exit0. It permits zero GPU
processes and zero timing. No API, physics, repeatability or performance
acceptance is claimed from compilation.

The initial [build rejection](../pr02-compound-aabb-fix-2026-10-01/README.md) remains
preserved. Its production files were restored before this repair. The new driver
serially gates ordinary library/test then native library/test, stopping at the
first failed subprocess. All four pass with matching source hashes before/after.

| Compiled artifact | SHA-256 |
| --- | --- |
| Ordinary library | `096d9a9bfad7ba87677723ba37f3b6ee413f7ed266682b53927b7ebe3cc897a0` |
| Ordinary tests | `8f2b164cd88edd81efaffd0705e75db279d70aa678cf72a4139ad560205a4763` |
| Native library | `f5d3c3856536d53b8f8bfef51655cd062aed414adfeb68e6be80905be1d6d366` |
| Native tests | `00642a2e11e4ea879b0291bbf053e37f5f31aa656be88eb165c646c5e913eb30` |

[Raw index](raw-index.json) preserves source archives, exact commands/toolchain,
build logs, driver/engine receipts, native configuration hashes, baseline and
candidate production snapshots. Invocation checkout revision is context only;
compiled-input hashes and receipts identify the libraries/tests. C adapters are
built separately against these frozen libraries for
[first API validation](../pr02-compound-aabb-validation-2026-10-01/README.md).
None of the original rejected campaign's validation cases is resumed.
