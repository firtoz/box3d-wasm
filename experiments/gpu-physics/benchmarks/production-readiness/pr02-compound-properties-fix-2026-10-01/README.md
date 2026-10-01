# Rejected compound property build — 2026-10-01

The first production candidate was rejected during build. The ordinary library compiled; the new Rust test failed E0061 because it called the internal `b3_create_world` without a `GpuDevice`. The serial driver stopped immediately and restored all five production files byteexact to the accepted baseline. Native builds, all C builds and every validation process remained unlaunched. No API, physical or performance acceptance is claimed.

The frozen [protocol](raw/protocol-before-change.json), [candidate inputs](raw/candidate-inputs.json), [driver receipt](raw/rust-driver-receipt.json), both ordinary compiler logs and exact baseline/candidate source are retained. The source archive records the compiled candidate inputs; invocation checkout revision does not prove source identity. No rejected result is replaced. A [distinct compile-only correction](../pr02-compound-properties-compile-2026-10-01/README.md) supplies the missing test GPU argument; it changes neither property assertions nor production behavior.

Run `python3 validate.py` offline. Do not rerun the closed runner. No GPU validation or timing process ran in this campaign.
