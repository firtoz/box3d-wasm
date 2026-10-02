# Current desktop fixture baseline — PR03 remains open

The unchanged distance-joint prototype still fails on both current GPU backends.
The original 600-step ragdoll physical limits, four isolated spherical-joint
checks, collision-free ragdolls and 57 frozen-mesh pose checks pass. The older
strict ragdoll trajectory screen fails on both backends. These are first baseline
observations, not final-build qualification or five-process repeat evidence.

This work changes no production source, numerical tolerance, physics default,
ordering or scheduling default, Box3D source, or WASM output. The
[production roadmap](../../../../../docs/goals/gpu-production-readiness.md)
continues at PR03. Rain, loaded dragging, the rest of the fixed matrix and full
capture-state criteria still require a current executable baseline/contract.
Floor visibility, sample widgets and raycast behavior remain the later PR09–PR11
items. Fresh desktop-only CPU/GPU step-time, throughput, p95 and separately
measured rendering-FPS charts remain PR12; no laptop data is included here.

## Results

| Check | Ordinary GPU | Native cached GPU | Original acceptance |
| --- | --- | --- | --- |
| Distance prototype | **FAIL**, distance step0/lane1 | **FAIL**, same first discrepancy | All13 pose/velocity lanes within1e-5 of independent CPU |
| Four signed twist/tilted cases,120steps each | PASS, zero component error | PASS, zero component error | All14 printed state/separation fields within1e-5 |
|112body collision-free ragdolls,60steps | PASS, max position error8.999999998593466e-7m | PASS, same | Position distance≤5e-5m |
|112body ragdoll drop,600steps | PASS physical limits | PASS physical limits | Original peak/tail separation and speed limits below |
| Frozen grid45poses | PASS, maximum matched error1.0244999999991632e-8 | PASS, same | Contact count, normal and separation multisets within1e-5; allposes contact/upnormal |
| Frozen torus12poses | PASS, maximum matched error1.3870000000089366e-7 | PASS, same | Same manifold comparison within1e-5 |
| Original full ragdoll reference/trajectory screen | **FAIL**, first60step position error0.06358576716417953m | **FAIL**, same | Original0.006m screen unchanged |

The distance fixture reports GPUY=-1.00004232 versus CPUY=-1.00028491,
a difference of0.00024259m, at the first distance step with one substep. Each
child aborts with signal6. The earlier sphere-ground cases return to the loop
before this assertion; stdout remains empty because its buffered output was
not flushed on abort. No240step distance completion or later four-substep
case is claimed. The preserved prototype includes its original warm-start
transitions, deterministic loads, sleep/CCD settings and1e-5 comparisons.
Spherical-joint passes do not resolve this distance discrepancy.

The unchanged full ragdoll validator passes preceding setup/isolated/contact
assertions, then stops at its first60step trajectory screen. Its later strict
trajectory/visual checks were not reached. Its original traceback is retained.
The independent physical evaluator measures all67312 rows, including frames
480–600, and passes the original physical limits. A physical pass does not
turn the failed strict reference screen into a pass. No cause is asserted here;
PR04 must resolve or explicitly analyze the discrepancy under the published
physical contract without changing the original screen retrospectively.

| Ragdoll physical metric | Independent CPU | Each GPU backend | Original limit |
| --- | --- | --- | --- |
| Peak joint separation |0.213310137m |0.116392836m |<0.25m |
| Tail joint separation |0.00139051978m |0.00298925536m |<0.005m |
| Tail linear / angular speed |0 /0 |0 /0 |<0.05m/s /<0.1rad/s |
| Peak angular speed |101.96400514017317rad/s |74.41479353952438rad/s | GPU≤1.2×CPU |

Ragdolls preserve the upstream fixture's sleep default; this explains zero tail
speeds. The separate scheduling/performance contract disables sleep when it
requires it. No performance measurement occurs in this report.

## Frozen budgets and retained build failure

| Campaign | Frozen budget | Actual consumption / terminal result |
| --- | --- | --- |
| Original fixture build |3sharedC commands+10links /15translation units |3C commands succeed, first CPU link fails;9links unlaunched |
| CPU dependency repair |10links /12translation units; reuse3successfulC objects | All10links succeed; no sharedC recompilation |
| Distance baseline |2first processes, one per backend | Both abort at original physical assertion; driver completes planned cells |
| Ragdoll/mesh baseline |24first processes:8CPU+8ordinary+8native | All24child processes complete; unchanged evaluators retain passes/failures above |

Every protocol freezes zero candidates, retries and headline timing; both build
protocols freeze zero engine processes. There are26actual engine processes
across the two baseline protocols, including8independent CPU-only processes
and2dual processes with their independent mapped CPU worlds. Expected baseline
numeric failures are retained while the predeclared second/backend cases run;
input mismatch, missing actual NVIDIA Vulkan or watchdog stops the campaign.
None of these results count toward the exhausted scheduling campaign or final
five-run release qualification.

The first CPU link mistakenly used the GPU viewer's geometry-only Box3D archive.
It therefore lacked native public world/body/joint and allocation symbols. The
failed command and complete linker log remain byte-exact in
[the original stopped receipt](raw/first-build/receipt.json). No engine ran.
The distinct dependency repair uses the full independent CPU archive, retains
all original sources/assertions, reuses the three successful shared objects,
and links the nine previously unlaunched fixtures. This is a harness dependency
correction, not a production candidate or replacement physics trial.

## Compiled source and adapter proof

Retained source milestone: `abc0a54a3ce3b285c9984f2c7c6b5baefff00d1b`.
Invocation revision `5a40f05` is context only, not binary provenance. Runtime
adapter banners identify NVIDIA GeForce RTX4070SUPER with Vulkan; hardware
receipt records driver610.57.04 on this i9-9900K desktop. GPU physics uses the
actual NVIDIA adapter, without a display viewer or software rendering.

| Reused artifact | SHA256 |
| --- | --- |
| Ordinary Rust library |`aba7a834443ba7174d92bcf39217929ce9af83adc28ea9c244943b5e72eb6ec6` |
| Native Rust library |`c8a16154cb576c49e7bd7fd8e030319601192b466d9ddb19305bd043aa6a7267` |
| Full independent CPU Box3D archive |`b35f71d07515e333fa19f3e6297bbfc2040984a49f366c94afc5f974acef2c0c` |

[New fixture link receipts](raw/dependency-repair/receipt.json) record every
compiler command, fixture source hash, resulting binary hash, actual linked
archive/library hash and reused object hash. Fixtures compile with assertions
active. CPU provenance uses119actual CPU producer translation units/source and
object hashes; the older parent Rust build does not identify current GPU code.
Combined GPU fixtures use the independently prefixed CPU archive, not CPU
getters synthesized from GPU state.

[Producer receipts](raw/producers/engine-build.json),
[viewer parent](raw/producers/viewer-parent.json) and per-cell producer receipts
are included. All107Rust and723viewer inputs remain unchanged after the runs.
Their exact source bytes are already preserved in the
[existing producer source archive](../pr02-world-lifetime-viewer-builds-2026-10-02/raw/compiled-source-inputs.tar.gz).
The validator verifies that archive against both producer receipts. The new
fixture source archive preserves the exact current fixture/headers/sharedC and
original distance prototype. This source applicability is not PR06 clean-build
qualification: dependencies/cached object caveats remain recorded.

The execution protocols include exact commands, environment/settings, binary
identities, case durations, watchdogs and adapter selection. BothGPU cells use
`GPU_PHYSICS_LIVE_CONTACT_ORDER=0`; the native cell additionally sources the
pinned native-cache policy. CPU results are reused for both GPU comparisons,
without launching duplicate oracle processes. Ragdolls use1/60s andfour
substeps; frozen-pose mesh probes preserve their original tiny-step override.
State output here is physical/pose evidence, not full future-state capture.
Recorded elapsed process/fixture clocks are incidental and excluded from
performance acceptance, FPS or step-time charts.

## Portable evidence and checking

121indexed raw files preserve all frozen protocols/drivers, complete source
archive, producer/build/result receipts, every stdout/stderr, actual frozen
pose inputs, unchanged evaluators and their failures. No compiled binaries,
objects or libraries are checked in. Older datasets remain unchanged.

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr03-baseline-fixtures-2026-10-02/validate.py
```

This offline check needs neither local ignored executables nor a GPU. It verifies
raw hashes, producer/source hashes, original failure and repaired link budgets,
all26process receipts, adapter proof and exact fixture provenance; recomputes
physical/isolated/manifold checks and reproduces the archived strict trajectory
failure using only saved data. Do not rerun closed engine drivers to validate
this report. Remaining PR03 contract/current baseline work is explicitly open.
