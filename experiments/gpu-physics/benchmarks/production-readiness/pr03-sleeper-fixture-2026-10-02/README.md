# Generation-safe HighResistance sleep/wake fixture

The HighResistance sleep/wake check now uses the actual public body handle.
It passes on ordinary/native NVIDIA Vulkan with both efficient ordering0 and
CPU-compatible ordering1, preserving the original400settling steps and one wake
step. Only the test fixture changed; production solver,scene geometry,defaults,
Box3D and WASM remain unchanged.

The [original Rust baseline](../pr03-rust-baseline-2026-10-02/README.md) retained
“impulse must wake a sleeping capsule” failures on both backends. Its fixture
fabricated generation1 after earlier serial tests destroyed/recreated worlds.
The lifetime repair deliberately advances child generations on world reuse;
invalid handles report false from `IsAwake`,so the old fixture could incorrectly
claim a live capsule was asleep and then try waking through a rejected handle.
Those failed observations remain unchanged; they are not replaced with passing
logs or erased from the199check baseline.

The corrected fixture first creates/retires an identical HighResistance world
without stepping it,then recreates the world slot. It finds capsule7 through
public dynamic-body enumeration and asserts a live handle with an advanced
generation and an invalid retired handle. It keeps original scene/default setup,
sleep enabled,foursubsteps,dt1/60,and400settling steps. After verifying the actual
live body remains valid/asleep,it checks a stale velocity setter leaves its
velocity and sleep unchanged,then applies [0,4,0] through the live handle and
requires awake after one original step. No physics assertion or tolerance was
removed,relaxed or shortened. Unrelated tests remain byte-identical.

| Backend | Ordering0 | Ordering1 |
| --- | --- | --- |
| Ordinary wgpu/Vulkan | PASS,oneexacttest | PASS,oneexacttest |
| Native cached/Vulkan | PASS,oneexacttest | PASS,oneexacttest |

Each fresh process records retired capsule generation1 and valid live capsule
generation2. Every original settling/wake assertion and added validity/stale
setter check passes. This resolves the fixture's identity error and qualifies
this focused current sleep/wake case; it does not certify allsleep/lifecycle,
fullstate repeats,Rain,loadeddragging or production readiness.

## Frozen execution and provenance

The [fixture protocol](raw/fixture-builds/protocol.json), SHA
`8d488983557c8513cf6c654cc52b2b4e2b5e52cc681319c893658a5c3715ee64`,
freezes twotestbuilds/fourfresh backend×ordering processes,zero productionlibrary
builds,solvercandidates,retries and headline timing. Original Rain/drag campaign
is terminal before editing source. Only `src/gpu_invariants.rs` changes among
107frozen Rust inputs; `src/lib.rs` gates the module with `cfg(test)`.
The [candidate source archive](raw/fixture-builds/candidate-source-inputs.tar.gz)
retains all107exact source bytes. Original fixture and build logs/actualCargo
compiler-artifact receipts are included. Ordinary/native test executable SHA:

- Ordinary:`7fbeb199acf04e5d6ac26d0e30ae966187f98ac2f738dac393f5f6fb37c7d3e8`.
- Native:`c3e08c0bb86f40f92d35114c862adefd823b7423595adf1896809861777117b4`.

Both builds succeed. The original artifact copy omitted execute permission;
driver99621 stops withPermissionError before any test launches. That complete
setup failure and empty first-launch logs remain retained. The separate
[permission repair](raw/permission-repair/protocol.json), SHA
`d6e3db8d316828c7be10896eff21ca32f615ae720b7ed87ac05617710b1e629d`,
freezes two permission repairs and only the four stillunlaunched cells,zero builds/
retries/candidates. It restores the producer's execute mode without changing
binary bytes or source/settings. Driver79134 completes allfour checks,exit0.
No repeated test/build or additionalcandidate consumes either budget.

Production archives remain exact retainedsource`abc0a54` with their recorded
binary hashes; no API,numerical shader or viewer source changes. The source
applicability distinction is explicit: archived originalbaseline tests use the
old fixture,while current correctedfixture uses these new testexecutables. Its
module is excluded from production libraries,so prior functional PR01/PR02
receipts remain applicable to their unchanged production code. This is not a
solver/visual change; affected live scene behavior/recordings are unchanged.
PR06 still needs clean release builds; these test builds reuse cacheddependencies.
Auxiliary native wrapper/backendfingerprint/config/lock files are archived with
explicit postbuild capture disclosure and successful lockedbuild logs,not a
claim of clean private-dependency qualification.

Allfour processes log actualNVIDIA GeForce RTX4070SUPER Vulkan,desktopdriver
610.57.04,with pinned nativepolicy where applicable. The exact settings/features/
toolchain/commands are in protocols and producer receipts. Incidental test clocks
are excluded from performance charts. No scheduling/default policy changes.

Fortythreeindexed rawfiles retain allsource/protocol/driver/build/test receipts
and original setup failure. Executables,objects,libraries,pipeline-cache blobs
and bytecode caches are excluded. Earlier datasets and failed physics screens
stay preserved.

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr03-sleeper-fixture-2026-10-02/validate.py
```

Offline validation checks actualsource/compiler-artifact hashes,the single test
source change/productioncfg exclusion,permissionrepair byte identity,allfour
processes/adapterproof/validity evidence and unchanged400+1 assertions. Full
release-build/repeatability,physical and performance gates remain open in the
production roadmap. Do not rerun the closed drivers.
