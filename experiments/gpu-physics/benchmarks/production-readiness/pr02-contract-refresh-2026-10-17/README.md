# PR02 — native API contract refresh

PR02's named functional API contract is verified on the PR11 source milestone
`17e3575a7`. This closes the filter-default reopening and refreshes current API
coverage after PR09–PR11. It does not qualify the remaining physics or release
gates. Box3D, WASM, solver defaults and production sources are unchanged here.

The repair itself is already in PR11: standalone `b3DefaultFilter.categoryBits`
now returns `UINT64_MAX`, matching actual Box3D CPU. The old stateful inventory
excluded this no-ID default alias. The new inventory includes all 17 public
`b3Default*` aliases separately, preventing that omission from being mistaken for
coverage. Only filter, shape-filter and query-filter defaults receive new
independent behavior checks in this refresh.

## Results

| Contract | Result and evidence |
| --- | --- |
| Current C/Rust symbols and combined routing | [Inventory](inventory.json): 415 stateful operations per route, 376 implemented candidates and 39 explicit exclusions; 17 additional default aliases. Each operation has exactly one linked provider and an indexed current source definition. No missing provider, missing GPU call or stateful CPU-only passthrough. |
| Reopened default-filter defect | Actual CPU first, then ordinary/native libraries in standalone/combined modes, plus the supported native launcher policy: all seven processes pass the same 34 query/identity checks. Masks 0, 1, 2, bit 63 and all bits, filter mutation, independent worlds, deletion/replacement and stale IDs are covered. |
| Named API controls | All 32 unchanged C contracts pass on current adapter/Rust libraries. World/body settings, warm-start cache behavior, contact/profile/capacity diagnostics, sensor/compound/mesh populations, mass, forces, shape replacement, nine joint types, compound ownership/AABBs and lifetimes are exercised. |
| Supported native launcher policy | All 16 named C contracts and two filter fixtures pass with the explicit settings from `scripts/native-samples-cache-env.sh`, plus original live contact ordering. No engine/default change. |
| Excluded-operation errors | Four host-only linked fixtures pass all 39 operations: `ENOTSUP`, sticky thread-local operation name, null recording creation and failed saving without a file. No GPU initialization or recording support is inferred. |
| Source/evidence applicability | [Source reconciliation](source-applicability.json) maps the changed C wrapper definitions and fingerprints current Rust API sources. Fresh ABI checks complement the scoped PR10 viewer and PR11 query evidence. |

There are **59 successful fresh processes**: the initial 36 completed contracts,
five previously unlaunched filter cases, and 18 checks of the distinct native
launcher configuration. No completed configuration was rerun to replace a result.
The 32 original API assertions and their tolerances are unchanged. For example,
forces retain 3,744 independent CPU/GPU scalar comparisons per configuration;
joint reactions retain 2,856 and separation retains 2,686. Incidental fixture
clocks are not performance claims.

The actual CPU and all GPU filter fixtures print the same defaults:
category/mask `18446744073709551615`, group `0`. CPU and GPU results use live
identity assertions rather than assuming equal numeric IDs across engines.

## Budgets, failures and cleanup

[Original protocol](protocol.json), [original receipt](runs.json),
[remaining-case protocol](remaining-protocol.json), [remaining receipt](remaining-runs.json),
[native-policy protocol](native-policy-protocol.json) and
[native-policy receipt](native-policy-runs.json) preserve the complete execution
boundary. All four incremental adapter builds passed. No Rust/viewer build,
solver candidate, sample capture or retry of a completed configuration occurred.

Two harness errors are retained. My new filter fixture initially omitted the
header-required `invokeContacts` argument, stopping at compile time after the
first 36 passes. It was corrected to `false`; only its five unlaunched cases
continued. The first inventory parser required `B3_API` on upstream definitions
and falsely omitted `b3DefaultDebugDraw`; [that original result](inventory-parser-failure.json)
is preserved. Correcting the parser closed both source-index findings without
changing product code.

The first native-library checks used default runtime settings. They are valid
default-runtime evidence, and remain labelled that way. A separate finite
protocol checked the previously untested supported native launcher policy;
its explicit environment is recorded on every operation. This avoids treating
the native archive's name as evidence that caching options were active.

Each temporary executable was deleted immediately after its consumer; the raw
directory is absent. [Final acceptance](acceptance.json) records exact library,
source, output and cleanup identities, free space and owned-process checks.
Shared current build and driver/pipeline caches remain available. Measured cache
growth is reported without attributing unrelated application activity to this task.

## Scope and recovery

The supported inventory is a source/link contract with named behavior tests,
not 376 independent physical passes. Wheel angular separation retains the
documented upstream release-zero fallback. Profile fields that are not measured
remain unavailable/NaN with explicit availability bits; they are not placeholder
successful measurements. Recording/player and CPU worker/static-tree controls
remain explicitly unavailable.

The original 20 Rust selectors and earlier native UI evidence remain historical,
scoped evidence. They were not rerun or promoted to new build passes. Current
public C fixtures execute the current Rust library. PR10 actual controls and PR11
queries provide complementary evidence; no all-sample review was repeated.

PR04 Rain/dragging/CCD and other physical failures, broad lifecycle/concurrency,
complete-state repeatability, clean release builds and performance qualification
remain open. No exact pixel/chaotic trajectory match, timing improvement, other
hardware or operating-system qualification is claimed. Tested here on Manjaro
Linux, RTX 4070 SUPER, NVIDIA 610.57.04/Vulkan.

Read [log.md](log.md) first on recovery, then repository `AGENTS.md` and the
production-readiness roadmap. PR11 was committed by user request; this PR02
refresh is committed separately by the subsequent user request. The acceptance
receipt records its earlier uncommitted verification state. Push and another
milestone were not requested.
