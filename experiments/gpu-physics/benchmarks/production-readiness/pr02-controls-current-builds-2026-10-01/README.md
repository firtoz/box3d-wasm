# PR02 current-source diagnostic viewers

Both current-source Rust libraries and all five serial viewer configurations
build successfully: real Box3D CPU, ordinary/native standalone GPU and
ordinary/native combined CPU/GPU. The [receipt](raw/receipt.json) freezes exact
commands, features, toolchain, before/after input hashes, library identities and
per-viewer executable hashes. Each viewer receipt contains actual compiled
units/object hashes, generated sources, linked archive hashes and link commands.
This is build proof, **not API, UI, physical or release acceptance**.

| Input | SHA-256 |
|---|---|
| Ordinary library | `3ea91456484f8fddffca889cabc580a287f4fb48ee17df32dd77ced946d40497` |
| Native library | `c718a10ddbc5f5758fdaba79caa895dd451ce4fd9d8eab23ff589eaa61ee852b` |
| CPU viewer | `6c65fabab6fbd048a321061c8f9b2bb061afdcfa450d57b1bbc5ff96d5e6d14e` |
| Ordinary GPU viewer | `dec18cb9126eea41a07d8e819bca5688061d2a43acfdd555b39341295d6fbecd` |
| Native GPU viewer | `b5f0f1103f4bb883d9bd9b3881024318632dfe516684a1ae75ebc7f6202f8fcd` |
| Ordinary combined viewer | `fdc596ce03ff0c808bfab194bd34cdeca741814b55a64c0356d5bf4eb49a13ca` |
| Native combined viewer | `f93c1ccf46766dffd811ad951f425f199a04b13797e0d3775a422a8d1a9faeec` |

Only generated `main_bench.cpp` copies gain one observer include and one
post-Step call. [Observer source](raw/observer.inc) reads actual world flags,
body events/counters, completed steps and combined mapped CPU flags, then writes
JSONL. It never writes context/physics state or changes input handling/defaults.
Original and instrumented main sources are preserved in each cell. Reads may
wait/synchronize, so resulting app cadence is diagnostic, never performance.
Production sources, Box3D, WASM, WGSL and scheduling/default policy remain unchanged.
The source snapshot now includes the corrected cfg(test) module directly,
removing the earlier library-source exception for these control builds.

The finite [build-only protocol](raw/protocol-before-builds.json) budgets two
library builds and five configure/build pairs; physics/timing/candidates/retries
zero. An initial driver-file creation used a wrong relative path before any
build launched; the [exact setup error](raw/setup-failure.json) is retained.
Correcting the path consumed no build or app trial. No command retry occurred.
Five explicit generation commands prepare the observer copies between configure
and build; all17 resulting build/generation commands exit0.

The source archive snapshot was over-broad and included unrelated historical
`native-samples/build-*` files. Their original hashes remain in the receipt, but
[archive applicability](raw/archive-applicability.json) lists the omitted
**uncompiled** files; the portable source archive contains actual original and
dependency sources, with generated sources archived per cell. No old or new
executable/object/library is checked in. The validator checks all actual compiled
source identities against these archives:119/124/124/178/178 units. Each inventory
also discloses a reused shared FetchContent NFD object. This is not the clean
complete release-build proof required by PR06; toolchain/system/dependency
qualification remains there.

`python3 validate.py` verifies45 portable raw files, source identity and the
exact observer-only patch. The separate
[app campaign](../pr02-controls-current-apps-2026-10-01/README.md) stops on a
retained focus-helper infrastructure failure before its first input. Its zero
control passes are not inferred from these successful builds.
