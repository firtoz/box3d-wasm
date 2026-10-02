# Distance-clamp precommit viewer relinks

Four artifact viewers link successfully against the verified clamp libraries:
ordinary/native GPU and ordinary/native combined CPU/GPU. The original 124/178
C/C++ source/object pairs, dependent archives and original viewers remain
unchanged. No Rust, C/C++ or CPU engine compilation or physics process occurred.
These are candidate recording executables, not PR06 clean release builds.

Frozen protocol SHA
`a6ccb0988a1f4af8f9f994808c6a9516debf0a64fef97d5cca96432531c699e3`
allows four relinks, zero retries/timing. Driver 53307 completed with exit 0.
The sole link changes replace the actual Rust archive with the corresponding
candidate library and place the executable in a fresh artifact directory; the
linker's dependency sidecar is suppressed to avoid modifying the old build.

Actual C/C++ consumer provenance is retained separately from Rust provider
provenance. All 614 relevant C/header/generator inputs and every reused object/
archive hash match their original producer. The recorded Rust provider receipt
identifies its actual 107 compiled inputs, library hashes and original-limit
comparison passes. The reused wrappers include the independent prefixed Box3D
CPU archive in combined viewers. Invocation HEAD alone proves none of these.

All 28 indexed raw files (9,514,243 bytes) preserve source archives, original
consumer and new link receipts, generated C inputs, command logs and provider
references. No ignored executable/object file is needed to verify the record:

```sh
python3 /home/firtoz/work/2026/box3d-wasm/experiments/gpu-physics/benchmarks/production-readiness/pr04-distance-viewers-2026-10-02/validate.py
```

Next is the separately frozen CPU-first three-scene recording/review. Prior
clamp/regression budgets remain closed, and broader PR04 failures/release gates
stay open. Box3D/WASM and old datasets are unchanged.

Follow-up: [all 15 CPU-first scene recordings and source retention](../pr04-distance-captures-2026-10-02/README.md) are complete; these cached relinks still do not qualify PR06 clean builds.
