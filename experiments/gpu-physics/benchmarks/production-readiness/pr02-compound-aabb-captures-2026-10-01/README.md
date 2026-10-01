# Compound AABB precommit scene review

All twelve frozen captures complete: six real Box3D CPU processes, then six native
GPU processes on NVIDIA RTX 4070 SUPER Vulkan (driver610.57.04). Every process
completes300 submitted/completed/rendered steps, with finite scanned state,
no reported capacity loss and zero Sokol errors. Five affected Compound scenes
and the Mesh Tile control are retained. This supplies the precommit scene evidence
for the [qualified AABB repair](../pr02-compound-aabb-validation-2026-10-01/README.md).
PR02 remains open for combined ownership, scalar getters and actual viewer controls.

| Scene | CPU-first visual review |
| --- | --- |
| [Simple](review/compound-simple.png) | Same small platform and camera framing; distant upstream default camera retained |
| [Spheres](review/compound-spheres.png) | Same distribution of visible compound spheres |
| [Hulls](review/compound-hulls.png) | Same distribution/orientation of visible hull children |
| [Tile Floor](review/compound-tile-floor.png) | Tiled ground geometry appears in both views |
| [Village](review/compound-village.png) | Buildings and terrain visible; all52,500 upstream children retained, including2,500 building-mesh instances |
| [Mesh Tile](review/compound-mesh-tile.png) | Same layered mesh tiles in the unaffected control |

The [gallery screenshot](review/gallery.png) confirms Box3D CPU first and this
GPU column next. Actual browser Play/Pause row interactions load both Village
clips without a media error; this checks the comparison page, not native widgets.
Each contact sheet has five views per backend from the completed scene window;
raw clips retain all startup/loading footage. Still-selection fractions are
presentation-time approximations, not equal-step pose comparisons. Village's
moving blue capsule is visible at differing positions across samples; these
images do not qualify trajectory equivalence or controller behavior.

## Fixed scope and reproduction

The [immutable protocol](protocol.json) has CPU6/GPU6, timing0/retries0, fixed
scene order and a600s/scene watchdog. Every budgeted case completes; none is
repeated, replaced or extended. API validation/build-rejection budgets remain
closed. Preparatory path and Python syntax errors occurred before freezing or
launching any capture; they produced no trial and changed no compiled input.

The existing entry points delegate a supplied native protocol to the new recorder:

```sh
NATIVE_CAPTURE_PROTOCOL="$PWD/artifacts/production-readiness/pr02-compound-aabb-captures/protocol.json" ./scripts/record-box3d-oracle.sh
NATIVE_CAPTURE_PROTOCOL="$PWD/artifacts/production-readiness/pr02-compound-aabb-captures/protocol.json" ./scripts/record-snapshot.sh 2026-10-01-compound-aabb
```

These are the executed commands, not permission to rerun this closed campaign.
A future reproduction needs a separately frozen applicable protocol, fresh
artifact directory and append-only output filenames. Older columns/datasets
are preserved. CPU clips append to `000-box3d-cpu`; GPU clips are in
`2026-10-01-compound-aabb`. Per-campaign recording manifests preserve their own
binary identity without relabelling older clips.

Capture defaults remain60Hz/four substeps, sleep/warm-start/CCD enabled, upstream
geometry/material/gravity and automatic CPU worker count. Fresh isolated `{}`
settings suppress first-run help/replay redirection. Relative data assets are
available through an audited upstream-data path, including Village's building
mesh. Only UI is hidden. Xvfb/Mesa software OpenGL captures the visible viewer;
GPU physics independently uses the actual NVIDIA Vulkan adapter. This setup,
health scans and startup footage cannot supply headline physics/FPS measurements.
No performance improvement or no-regression claim is made here. The user-requested
fresh desktop-only FPS/step-ms charts remain a final roadmap deliverable.

## Provenance and limitations

| Viewer | SHA-256 |
| --- | --- |
| Fresh real CPU executable | `9d879afb242a73049e62c5a1ac85c2b701e3c7b070148605cb62b6e8339a28a0` |
| Native GPU executable | `164b03b5f55cfc8f02b7d49f1d7d1155ff7557057047cb39377e251588f98a61` |

[Raw index](raw-index.json) covers103 portable raw files: immutable protocol,
launch settings/environment, all output/health/codec receipts, asset/source
archives and CPU build proof. GPU viewer/library provenance remains in the
linked AABB validation/compile reports. Actual built translation-unit sources,
objects and linked archives have hashes; engine source is separately archived.
Invocation HEAD is context only. Nine third-party compiled units per viewer
are supplemented with exact matching source bytes and clean pinned dependency
trees. Their headers are explicitly a post-build audit, not a claimed before/after
header receipt; PR06 must freeze complete final-build dependencies independently.

[Source applicability review](implementation-review.md) identifies query/camera
callers and confirms collider-derived GPU proxy bounds remain independent from
imported public query boxes. No solver/shader, physics-default, Box3D or WASM
changes are included. Diagnostic host cost remains unmeasured under PR08.
Comparison-generator chronology was repaired after capture: persisted recording
manifest timestamps put latest GPU next to CPU, with legacy date/version fallbacks.
The frozen archive retains the exact generator used at capture time; the later
UI-only generator change does not alter any compiled viewer or clip.

Run `python3 validate.py` for offline hashes, sources, all twelve health/step
records and clip format checks. It does not launch physics. Scope is passive
scene/capture health plus the separately tested numeric AABB contract, not final
physical qualification, full-state repeatability, controls or production readiness.
