# Repaired world-lifetime viewer builds

Four first diagnostic viewer builds pass on the exact local lifetime repair. Both standalone viewers contain124 compiled units each; both combined viewers178. All12 configure/generate/build commands succeed. The finite protocol permits zero Rust rebuilds, app processes, timing runs or retries.

The ordinary and native libraries are reused byte-for-byte from the [passing API subset](../pr02-world-lifetime-repair-2026-10-01/README.md), with their original compiler artifact/depfile/source receipts. All107 Rust inputs match that evidence; new C guards and combined wrappers are compiled into these fresh viewers. HEAD786e2f8 is invocation context only: the production repair remains local and uncommitted.

| Configuration | Executable SHA256 | Compiled units |
| --- | --- | --- |
| ordinary-gpu | `5f62cad9c9a0b1cfee92cc569cefa7728f8e14d03807d4943a438b4e65229bea` | 124 |
| native-gpu | `f71d11955ac90c3cadc06b1939f8d65dc0c80339da8dd6cefd153c3d4a40e666` | 124 |
| ordinary-both | `32cca25f8853b9ff225be895382eafb2db95d38a909d572b7ce483553f02de94` | 178 |
| native-both | `c497cd8f5f9b93bb83bbdbf87541c57a80fb93d86e23773ccd7a36063ba4d109` | 178 |

[Protocol](raw/protocol-before-builds.json), [build receipt](raw/receipt.json), [raw index](raw-index.json) and per-viewer receipts retain exact commands, toolchain, before/after source hashes, generated C/C++ inputs, compile commands/object hashes, linked archive hashes and final executable hashes. The source archive excludes historical build directories. The generated observer changes only a main-file include and a post-Step read; flags/events/counters/completion reads are diagnostic and may synchronize. No production source is changed by instrumentation.

Shared cached FetchContent NFD is explicitly inventoried; this is not PR06 clean-release evidence. The previously accepted real CPU viewer and controls are reused by exact hash/applicability, with no CPU rebuild or process repeat. No UI, physical, performance, capacity, concurrency or full repeatability gate follows from compilation alone. Failed prior builds/apps remain in their original reports. Box3D/WASM, WGSL, physics defaults and scheduling policy are unchanged.

Run the portable verifier from any checkout:

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr02-world-lifetime-viewer-builds-2026-10-02/validate.py
```

It checks34 raw files, all604 actual compiled sources, generated observer patch and link-library correspondence. Ignored local executables/objects/archives are excluded from Git; their hashes and commands remain portable here.
