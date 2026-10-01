# PR02 native API audit and explicit unavailable operations

PR02 remains **OPEN**. Partial API milestone `806e634c9b0f5fe58fdd8a9a0dac171e862890d4`
is committed and pushed on `feat/gpu`; it is not production qualification. The [roadmap](../../../../../docs/goals/gpu-production-readiness.md)
still requires real diagnostics, the complete supported API behavior review and
viewer controls before closing PR02. No physics, shader, Box3D or WASM change,
GPU initialization, timing or Rain campaign occurred here.

The fresh linked-source audit inventories all **415 stateful header symbols** on
both backend/linkage pairs. It finds zero additional missing symbols/duplicates,
but symbol presence is not behavioral acceptance. See
[ordinary inventory](api-inventory-ordinary.json) and
[native inventory](api-inventory-native.json). Classifications are explicit:
declared unavailable, required diagnostic gap, CPU routing review, or implemented
but unqualified. Unknown/missing build inputs cannot count as support.

## Exclusions now report errors

The initial release excludes native recording/player parity and CPU-specific
worker/static-tree APIs. All **39 named operations** in
[`native_unavailable.inc`](../../../c_abi/native_unavailable.inc) now set
`errno=ENOTSUP` and a sticky thread-local operation name. Creation returns NULL;
saving returns false without creating a file. Other existing failure return
values remain compatibility sentinels accompanied by explicit unavailability.
The combined adapter uses these same failures, rather than running a successful
CPU-only recording or worker operation while leaving GPU behavior unchanged.

Consumers include [`native_api_status.h`](../../../c_abi/native_api_status.h).
`gpu_b3_native_api_last_error()` returns the static operation name or NULL;
`gpu_b3_native_api_clear_error()` clears that thread's API diagnostic and errno.
It never clears sticky world/capacity failure status. Check errno immediately
after the call: unrelated library calls can change it. The operation name remains
until explicit clear or another unavailable call. The exclusion lookup describes
only the named exclusions; false for an unknown name is not proof of support.

## Frozen checks and every result

The [original protocol](unavailable-protocol.json) permits two baseline host
processes and eight candidate host processes; no GPU or timing runs.

| Configuration | Original candidate trial | Remaining trial(s) | Result scope |
| --- | --- | --- | --- |
| Ordinary GPU | Trial 1 aborts in harness after all immediate API assertions | Trial 2 passes | One complete pass, one retained harness failure |
| Ordinary combined | No process before generated-link failure | Trials 1/2 pass | Corrected shared error routing |
| Native GPU | None | Trials 1/2 pass | Host C API linked with frozen native library |
| Native combined | None | Trials 1/2 pass | Corrected shared error routing |

Both [baseline processes](raw/baseline/receipt.json) reproduce non-null recording
creation with errno zero. They use frozen pre-edit adapters, not rebuilt sources
labelled with an old checkout revision.

The [first failed trial](raw/unavailable-candidate/ordinary-gpu/trial-1.stderr)
passes all 39 immediate ENOTSUP/name assertions, then incorrectly requires errno
to survive filesystem/thread-library calls. Preserve its exact compiled fixture,
binary hash and failed result. The corrected harness keeps every API assertion
and only removes that unrelated errno assumption. The
[remaining protocol](unavailable-remaining-protocol.json) spends seven remaining
trials, never replacing the failed one.

The combined build then exposes a real generator bug: treating exclusions as
DUAL wrappers causes portable filtering to remove their shared C implementations.
The [link failure](raw/unavailable-remaining/ordinary-both/link.log) is retained;
no process runs there. Correct the two generators to exclude CPU passthrough
separately and retain shared C definitions. The
[routing protocol](unavailable-routing-protocol.json) freezes the six remaining
trials before execution. Total candidate consumption remains **eight: seven
complete passes and one failed harness trial**. No timing or GPU process credit.

The successful fixtures exercise all 39 public calls, null recording/no saved
file, explicit clear, sticky name, thread independence, and exclusion lookup.
Public stdout repeats exactly within each completed configuration. They do not
create a world or validate GPU physics. Both baseline and combined initial
fixture link failures are also retained, with the latter fixed by supplying the
viewer selection hooks required by the existing combined archive.

A separate [one-process closed-stderr protocol](closed-stderr-protocol.json)
addresses a concrete error-contract edge: failed stderr output can overwrite
ENOTSUP before save returns. Publish the unavailable error again after output.
The [whole-source C check](raw/closed-stderr/receipt.json) passes with fd2 closed.
Its strict-C17 build failure is retained; the successful build uses the same
gnu17 dialect as CMake. This run is never substituted for the exhausted campaign.
Earlier normal-stderr checks remain applicable: the additional publication is
idempotent, with the same name and ENOTSUP. Final whole-API qualification remains
required on final sources under PR02/PR06/PR07.

## Provenance and reproduction

[Raw index](raw-index.json) hashes all 81 preserved logs/receipts/fixture sources.
[Compiled-source index](compiled-source-index.json) maps six source sets to exact
generated C inputs and fixture bytes. Successful adapters record before/after
input hashes, CMake compile commands, link inputs and binary hashes. Frozen
ordinary/native Rust inputs and build identity are the unchanged
[PR01 receipts](../pr01-speculative-2026-10-01/source-index.json). Invocation HEAD
is recorded only as checkout context. Neither backend label implies device
execution for these host-only checks.

From the engine directory, use `scripts/check-native-unavailable.py` with the
recorded protocol and a new output directory after freezing applicable libraries
and receipts. The script refuses an existing result directory. Historical split
protocols document actual consumption; replaying all of them is not a new
qualification campaign. Freeze a new full API protocol before later release
checks. Portable compiled inputs support independent review without local build
trees; large binaries and dependency caches are not checked in.

## Remaining required work

The audit still reports one real stub (shape-bounds dump), three placeholders
(profile, maximum-capacity and memory diagnostics), incomplete counter fields,
and five CPU-only routes requiring review: counters, profile, maximum-capacity,
memory dump and shape type. The source pass finds GPU contactCount always zero
and combined counter/profile calls reporting the CPU world. Native contact
registry counts and scheduling-phase roots have different populations; define
and test their exact semantics before substituting one for another.

Finish real diagnostics and routing, verify forces/replacement/joint queries
and per-world lifetime behavior, and make viewer availability truthful. Preserve
all required physics and original tolerances. No checkbox closes here.
