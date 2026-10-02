# Loaded dragging: first-impact CCD boundary diagnosis

The ordinary GPU backend's first held-position failure is directly attributable
to an extra CCD correction at frame 227 (zero based), original trace body 0.
The independent CPU classifies the motion below its CCD threshold. The ordinary
GPU clips an otherwise closely matching endpoint with TOI fraction 0.6233841,
moving it by 16.993 mm. Velocity still agrees within 0.000001876 m/s; next-frame
contact response diverges in the original full failed trace.

The native backend completed the same prefix and captured CPU classification,
but emitted **no host GPU TOI records**. Its internal GPU fraction is unobserved.
Matching post states and an environment setting do not supply that observation.
See [coverage audit](coverage-audit.json). This closes ordinary attribution, not
native boundary coverage or the original 3,060-step dragging acceptance. PR04
remains open. No production code, physics default, tolerance or policy changed.

## Captured result

All values below come from the printed records, with derived arithmetic in host
float64. The original executable's physical assertions remain authoritative.

| Frame 227/body 0 quantity | Ordinary captured/derived value |
| --- | ---: |
| CPU maximum motion | 45.120038 mm |
| CPU fast-body threshold | 250 mm |
| CPU fast classification | false |
| CPU TOI observations, selected frames 220–232 | 0 |
| GPU start-to-end translation | 45.119961 mm |
| GPU pre-CCD endpoint difference from CPU | 1.056378 mm |
| GPU TOI fraction | 0.6233841 |
| GPU correction magnitude | 16.992826 mm |
| GPU post-CCD position difference from CPU | 17.164716 mm |
| Linear-velocity difference | 0.000001876 m/s |
| `start + fraction * (end - start)` residual from GPU post position | 0.000120 mm |

The observed ordinary translation alone exceeds the current 20 mm speculative
shell activation threshold; its nonnegative rotation term cannot undo that
classification. CPU uses a different maximum-motion calculation and half the
minimum extent. All 13 captured CPU classifications are false on both backends;
their observed positions/quaternions equal their corresponding CPU trace records.
The CPU TOI hook would print before applying a correction if this selected body
entered continuous solving. Its absence is consistent with the directly recorded
classification and source branch, not an inferred GPU result.

Ordinary stderr contains 132 host GPU TOI records through this 240-step prefix.
Native stderr contains zero. [analysis.json](analysis.json) preserves the entire
selected window, raw start/end/fractions where available and reconstruction.
GPU observer step 228 corresponds to zero-based frame 227. CPU internal body 1
and GPU observer body 2 correspond to original trace body 0.

## Frozen budget and execution

[Protocol](raw/protocol.json) SHA256:
`71465056aa2da1b96f060bea28f7cd1d6b1af5738b4278521a9647968748dbe3`.
Driver 11553 terminated with exit 0; [receipt](raw/receipt.json) is terminal.
All allocations are consumed once; the budget is closed:

- One CPU C translation-unit compilation, two C++ fixture compilations/links.
- Three CPU archive operations: copy, replace one member, prefix symbols.
- Two fresh ordinary/native diagnostic processes, ordering 0, 240 completed
  original steps each, original dt 1/60 and four substeps. Watchdog 300 seconds
  per process; neither timed out.
- Zero Rust builds, production candidates, retries or headline timing runs.

The fixture is an artifact copy of the original loaded-ground drag test. It
adds a CPU frame selector and terminates after the original 240-step prefix,
before the later full-test acceptance assertions. Exit 0 means completed prefix,
not full dragging success. [Fixture diff](fixture-prefix.patch) and original/full
failed baseline traces are preserved. No ground, grab target, motor, forces,
ordering, sleep or simulation default was modified. Exact overrides and commands
are in the protocol/receipt. The native configuration retains its original GPU
CCD and cache/replay requests; actual internal execution/fraction requires
further observation.

Both runs used actual **NVIDIA GeForce RTX 4070 SUPER Vulkan**. Per-process device
banners are retained; a separate post-capture query reports NVIDIA driver
**610.57.04**. No rendering FPS, completed-step speed or shader-time claim is made.
Receipt elapsed clocks are incidental diagnostic execution times.

## Observer neutrality and provenance

All original `B/F/M/P` printed lines through frame 239 match the archived baseline
prefix exactly, separately for each backend: 14,020 comparison lines, 2,400 body
state records and 240 completed steps per process. Total: 28,040 comparison lines,
4,800 body state records and 480 steps. Those records include pose/velocity and
contact feature/anchor/separation/impulse observations at printed precision.
This establishes neutrality for the captured nine-significant-digit records;
it does not establish equality of all future-relevant caches or later steps.

The CPU observer is an artifact-only copy of untouched Box3D `solver.c`; the
[read-only diff](CPU-read-only-observer.patch) adds two `fprintf` hooks and a
selection helper without replacing arithmetic. Original solver source SHA256:
`25e0f58f9d7d803be88ce0d2d3a424b902d41ff3bd3e01310a1ca7820f76d926`.
The actual compile dependency file and 91 hashed source/system-header dependencies
are archived. A copied 50-member CPU archive changes exactly `solver.c.o`;
every other recorded member hash is unchanged. The replacement object's hash,
prefixed archive hash, symbol map and operation commands/logs remain in the
receipt. The original archive and Box3D source were never changed.

The C consumer chain retains 614 source inputs plus generated source bytes,
178 original source/object receipts and reused archive identities. Existing
Rust libraries were reused, with their actual successful producer receipt and
107 compiled source inputs archived. The only current Rust source exception is
the already verified `cfg(test)` distance invariant; production sources match
the retained clamp provider. No distance joint exists in this motor-drag fixture.

| Input/output | SHA256 |
| --- | --- |
| Ordinary reused Rust library | `5bb8a5d7988eea83e5f6bdbfc0831872f59e06389d69c4a1b9919e7bf7f8e3f1` |
| Native reused Rust library | `53819158e2321f1f270fa3d885664ce6519770f6448163d4894de1041b19bc3e` |
| Ordinary diagnostic executable | `ecca39627d0cdd43a8f5a6a733220356501a31432f688cd4b2fa96d72de2a5db` |
| Native diagnostic executable | `17710b7b7261d639310043750116f276da20880ae3d5923523165c57ab788800` |

Invocation revision `55a22c7` is context only. It is not binary provenance.
All frozen source/object/archive inputs were checked before/after operations
by the preserved driver. Portable evidence verifies the recorded build chain;
it deliberately excludes executable/object/archive binaries and pipeline caches.
It cannot independently reconstruct a compiler's output without recompiling.

## Offline verification and next decision

Run from any checkout, without a GPU or the original artifact paths:

```sh
python3 -B experiments/gpu-physics/benchmarks/production-readiness/pr04-drag-ccd-boundary-2026-10-02/validate.py
```

The validator verifies all **36 raw files / 11,754,241 bytes**, compiler
dependencies, retained C/Rust source archives, receipt commands/hashes, recorded
single CPU archive-member replacement, exact baseline-prefix equality, source
diffs and reproducible analysis/coverage. `analyze.py` only reads portable bytes.
[raw-index.json](raw-index.json) and [reference-map.json](reference-map.json)
identify every retained raw/reference file. Both full failed baselines remain
preserved; no result was overwritten or promoted to a physical pass.

Before choosing a candidate, audit the native GPU CCD dispatch/classification
and its observation points. A distinct finite protocol is required for any new
build/process. Do not repeat this closed campaign to fill missing native coverage.
Do not disable CCD or simply restore the old cutoff: the historical paired
cutoff control fails settling at frame 2460. Preserve empty-manifold recycling,
CCD/restitution/support checks and every original full dragging tolerance.

The [frozen-source audit](source-audit.json) now verifies eight original compiled
Rust/WGSL files offline. A configured convex GPU pass bypasses the host observer;
its inline classifier uses the same shell cap, and its TOI fraction exists only
as a local shader variable. Existing separate CCD captures retain geometry,
configuration and start state, but omit the pre-correction endpoint and fraction.
The standalone motion classifier is not called by this `ConvexCcd` correction.
These source facts explain the observer gap; they do not prove runtime eligibility
or a native fraction. An artifact-only dedicated GPU output buffer is the next
discriminator proposed in that audit. No new observer/build/run is launched here.
