# PR02 native diagnostics — focused milestone (2026-10-01)

The focused diagnostic campaign passes on the RTX 4070 SUPER / NVIDIA 610.57.04
using actual Vulkan. This is a partial PR02 milestone: four viewer builds and 40 standard CPU-first recordings are complete. Actual counter/tab interactions, the remaining supported API audit and final production qualification remain open. The failed mouse-input harness is retained below.

The historical direct GPU APIs reported zero public contacts, profile fields and
capacity after a four-step fixture with one real contact. Both separately frozen
baseline processes reproduce that behavior. Candidate public counters synchronize
native contact records; profiles publish only measured GPU aggregates with a
step/mask; occupancy peaks and reserved buffers have separate contracts. Combined
public diagnostics report GPU data, while the explicit CPU helper retains CPU
measurements. No timing or speedup claim is made.

| Frozen campaign | Consumed / budget | Result |
| --- | --- | --- |
| Baseline diagnostic GPU processes | 2 / 2 | Both reproduce historical placeholders; no acceptance credit |
| Linked C diagnostics | 8 / 8 | Two fresh processes in each ordinary/native × GPU/combined cell pass |
| Rust diagnostics | 4 / 4 | Contract and unread-step/retirement/growth/capture checks pass on both backends |
| Preserved regressions | 16 / 16 | Eight exact selectors per backend pass |
| Storage/comparator suite | 1 / 1 | 16 tests pass, including peak corruption and older schemas |
| Candidates / timing | 1 / 1; 0 timing runs | No budget extension, rejected GPU trial or timing result |

The [protocol](protocol.json) freezes the hypothesis, scope, process counts,
selectors, settings and stop rule. Its `progress` field is the pre-launch state;
[current progress](progress.json) and per-campaign receipts hold actual results.
Runs are serial and separated from builds. No Rain work or exhausted scheduling
campaign was resumed. Unfavorable results are retained: pre-launch shell cwd
mistakes and a C fixture syntax rejection are described in progress; the rejected
fixture spelling is [retained](raw/fixture-syntax-rejected.cpp). These failures did
not launch GPU processes. A preliminary build receipt omitted Box3D C/header
inputs; it remains local and is superseded by complete before/after snapshots
before every candidate GPU trial.

## What the checks cover

C processes call the linked public API, not replacement mocks. A static/dynamic
contact plus independent dynamic and kinematic bodies runs four steps at dt 1/60
and four substeps, zero gravity, sleep disabled. Assertions cover exact public
body/shape/contact/joint counts, two completed dynamic islands, touching manifold
buckets, unsupported-field sentinels, partial profile availability, 64-bit primary
bytes, occupancy peaks, owned shape bounds, hull-to-sphere type replacement,
independent worlds, destruction/stale errors, and sticky thread-local API names.
Combined cases additionally query the actual mapped CPU profile/counters.
Profile assertions validate whichever timestamp mask is available; they do not
claim complete CPU profiler parity or separately time every GPU substage.

Rust contract checks additionally hold the world mutex to verify busy/reentrant
status without deadlock, preserve zero-step peak semantics, inject capacity loss,
and verify that querying or clearing diagnostics never heals a failed world.
The separate five-step peak fixture suppresses status observations, retires the
contacting body, grows to 35 dynamic bodies, then captures both host and device
peaks. It proves a peak of one survives unread steps, retirement and buffer
reallocation despite a current public contact count of zero.

The eight existing regression selectors cover buffer growth, proxy/order changes,
contact-metric freshness and lifetimes, completed loss refresh, component merge/
split equivalence, captured metrics, undrained-status rejection and speculative
zero-step/lifetime policy. Their existing assertions and tolerances are unchanged.
This is not final Rain, joint, lifecycle or complete-state repeat qualification.

Schema v24 adds host occupancy peaks, the harvested GPU peak and persistent
query-word-74 state. Storage controls require integer bounds/field presence and
exact comparisons, retaining v1–v23 readers. One selected step per backend is a
peak diagnostic capture, not a full five-step repeat trace or a fresh-repeat pass.

## Provenance and portable data

The [raw index](raw-index.json) records lossless portable file hashes and original
trace hashes. No result depends on ignored executables to read it. Build receipts
identify compiled input contents before/after, toolchain, commands, logs and
library/test hashes; checkout metadata is only contextual. Compressed source
archives contain the actual engine plus pinned Box3D C/header inputs. Candidate
adapter archives contain filtered/generated sources, original headers, full
compile commands and linked library/test hashes. Old baseline receipts identify
the preserved PR01 engine and PR02 adapter binaries separately.

- [Rust campaign receipt](raw/rust/receipt.json)
- [Regression campaign receipt](raw/regressions/receipt.json)
- [Ordinary engine build](raw/candidate-complete-inputs/ordinary-build.json)
- [Native engine build](raw/candidate-complete-inputs/native-build.json)
- [Ordinary source archive](raw/candidate-complete-inputs/ordinary-compiled-sources.tar.gz)
- [Native source archive](raw/candidate-complete-inputs/native-compiled-sources.tar.gz)
- [Ordinary API inventory](raw/audit-ordinary.json)
- [Native API inventory](raw/audit-native.json)
- [Storage controls](raw/storage-controls.log)

Both linked inventories cover 415 stateful symbols: 39 explicit exclusions,
zero remaining stubs/placeholders, zero missing/duplicate symbols and zero
CPU-only comparison passthroughs. All other APIs retain the inventory label
`implemented-behavior-unqualified`; symbol coverage is not broad API acceptance.
CMake viewer include/dependency additions after the C campaign do not alter its
C/Rust implementation. All four viewer receipts verify unchanged generated adapter sources, before/after
UI inputs, compiled translation units, build logs and executable hashes. The ordinary GPU Profile tab is observed, but Counters/Frame Time interactions remain open after the retained harness failure.

From the engine directory, preserved runners in `raw/` show exact commands and
configuration. Do not rerun them into this exhausted campaign. A final release
build and qualification campaign will have its own protocol under PR03–PR07.
The diagnostic API contract is documented in [gpu-physics.md](../../../../../docs/gpu-physics.md).

## Required recordings and actual viewer findings

The separate [recording protocol](raw/recording-protocol.json) completes 20 real
Box3D CPU then 20 GPU clips through the repository scripts. Every clip has 300
frames at 1280×720/30 fps. GPU metric runs are skipped; CPU oracle metrics generated
alongside dumps are incidental reference data, not performance acceptance.
[Checks and hashes](raw/recording-checks.json), exact recorder/oracle
[build receipts](raw/recording-build/receipt.json) and
[CPU build](raw/recording-build/oracle-receipt.json) identify separately compiled
inputs and executables. These are distinct from the test libraries.

The [portable comparison grid](recordings.html) pins actual Box3D CPU on the left.
Full clips and manifests are portable under `raw/recordings`. Four first/last
review sheets are retained. Setup/cameras agree and both mixed regions are visible;
pile/chain trajectories differ. Visual plausibility does not close analytical
physics gates. Older datasets are unchanged and the previous local CPU column is
archived, including its native ghost-collision reference. The new local GPU column
is `2026-10-01-truthful-native-diagnostics`; a
[label map](raw/recording-label-map.json) records the rename needed by the existing
grid's lexical chronology when timing metrics are absent.

The separate five-app [viewer interaction protocol](raw/viewer-interaction-protocol.json)
is **stopped after two apps**. Real CPU reference and ordinary GPU both open and
quit cleanly. The GPU Profile tab displays nonzero timestamp aggregates and step
IDs, including labelled unavailable CPU substages after scrolling. Immediate
synthetic mouse down/up fails to select Counters and Frame Time: those screenshots
still show Profile. This is a retained **interaction-harness failure**, not a
counter-tab pass. All actions, logs, screenshots and full clips are included;
the other three apps are unlaunched, no rerun is performed, and PR02 viewer control
qualification stays open. See the [result](raw/viewer-interaction-result.json)
and [CPU-first viewer gallery](viewer-interactions.html). A later distinct
uncovered-control campaign must verify stimulus delivery before assessing controls.

There is no speedup or no-regression claim. Host occupancy counting scans the body
and shape slots once per positive step; device contact peaks add diagnostic atomics.
Their cost remains unmeasured and must be assessed under PR08's frozen final-build
performance protocol. Broader sensor/compound/mesh diagnostic populations and all
remaining force/replacement/joint/control behavior remain open.
