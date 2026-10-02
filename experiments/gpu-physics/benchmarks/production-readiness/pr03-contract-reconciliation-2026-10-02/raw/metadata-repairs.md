# Metadata preparation repairs

No engine, build, diagnostic or timing process ran in this reconciliation.
The initial draft and its two preparation scripts are retained. Final metadata
also normalizes each original result's `exit` or `wrapper_exit` field without
altering that original result. The original single remaining-host receipt is
catalogued as one process. Mesh grid and torus inputs have separate recipes.

Offline validator development corrected three source/receipt assumptions:
the native-only test module uses `cfg(all(test, not(target_arch = "wasm32")))`,
the ordering guard belongs in `src/sim.rs`, and launcher results use
`wrapper_exit`. Older raw indexes contain SHA strings rather than byte/SHA
objects. These are validator/schema corrections, not new runs, waived physical
assertions, or altered source evidence. The final validator verifies all formats
explicitly. Do not rerun the draft/finalize scripts over the final manifest.
