# PR02 actual viewer controls — retained focus-harness failure

The first current-source CPU app starts normally and produces 85 default-state
observer records. It stops **before any input was sent** because the focus helper
invokes `xwininfo`, which is not installed. This is a harness infrastructure
failure, not a demonstrated engine/control failure. No control gets acceptance
credit. The four ordinary/native GPU/combined apps remain **unlaunched**, following
the first-failure stop rule. One app process, zero candidates, zero timing and
zero retries consumed; all unfavorable evidence is retained.

The [terminal receipt](raw/terminal.json) verifies driver/app cleanup against the
actual process list and hashes all final files. The initial settings file is
still `{}`; exit/shutdown or settings persistence did not pass. The full short
[capture](raw/cpu/clip.mp4), [observer](raw/cpu/observer.jsonl), logs, queued unsent
actions and failed [receipt](raw/cpu/receipt.json) are portable. Screenshot actions
were never reached. Startup alone does not qualify lifecycle behavior.

The immutable [app protocol](raw/protocol-before-runs.json) requires five serial
actual UI apps, real CPU first, with at most40 actions/20 screenshots and600s per
app. No benchmark mode or frame-limit bypass;60Hz/four substeps and default sleep,
warm starting and continuous collision. Read-only instrumentation may synchronize
reads and alter cadence, so this is diagnostic, never headline performance.
Exact viewer/build proof is in the
[current-source build report](../pr02-controls-current-builds-2026-10-01/README.md).

Before any app launched, a source audit corrected an invented persistence
expectation: upstream `SampleContext::Save` stores rendering options and sample
selection, not solver toggles. The [original draft](raw/protocol-initial-before-source-audit.json)
is preserved byte-for-byte, with its hash and the zero-app amendment recorded in
the launched protocol. No physics tolerance or previously observed result changed.
The existing [held-input host proof](../pr02-viewer-controls-2026-10-01/README.md)
was source-audited for reuse without rerunning its closed budget.

Next: replace the external focus helper with direct `libX11` window enumeration
and name lookup under a **distinct finite protocol**, reusing these exact five
binaries after checking their hashes. Preserve this failed campaign and all older
closed campaigns; do not rerun their drivers. PR02 remains open. Later floors,
sample widgets, raycast and desktop-only charts remain PR09–PR12.

Run `python3 validate.py` for offline integrity/state validation. It checks the
failure and initial records; it deliberately cannot certify any UI interaction.
