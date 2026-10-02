# Native GPU CCD: direct first-impact boundary

The native GPU backend's first loaded-drag position jump is now directly observed
inside its convex CCD shader. At original frame 227 (zero based), selected GPU
body slot 1, CCD fraction **0.623385488986969** moves the endpoint **16.992787 mm**.
The independent CPU records motion below its fast-body threshold and applies no
TOI correction. The native diagnostic matches all original printed baseline
records through frame 239 exactly.

This resolves the native observation gap left in the
[earlier ordinary/CPU boundary report](../pr04-drag-ccd-boundary-2026-10-02/README.md).
That report and its missing-coverage audit remain unchanged historical evidence.
The original full 3,060-step dragging test remains failed; no physical tolerance,
default, scheduling policy or production source changed. PR04/release acceptance
remains open. This is diagnosis, with no retained solver candidate or timing claim.

## Actual captured boundary

| Frame 227 / completed step 228 quantity | Native observation / derivation |
| --- | ---: |
| Actual convex GPU CCD presence | true |
| Live joint count / solver mode | 1 / 0 |
| CPU motion / fast threshold | 45.120038 / 250 mm |
| CPU fast classification | false |
| GPU motion / cutoff | 45.119993 / 20.000000 mm |
| GPU branch | 4: correction applied |
| GPU fraction IEEE-754 word | `3f1f9631` |
| GPU fraction | 0.623385488986969 |
| GPU pre-correction endpoint difference from CPU | 1.056374 mm |
| GPU correction magnitude | 16.992787 mm |
| GPU post-correction position difference from CPU | 17.164658 mm |
| Linear-velocity difference | 0.000001876 m/s |
| Float64 `start + fraction * (end - start)` residual | 0.000163 mm |

The exact GPU endpoint records are:

```text
start [-5.889269828796387, 0.6248248815536499, 0.41560450196266174]
pre   [-5.897047996520996, 0.5803803801536560, 0.41560474038124084]
post  [-5.894118785858154, 0.5971187949180603, 0.41560465097427370]
```

The shader captures 32 exact u32 words per GPU body slot in a dedicated buffer:
body/flags/branch/fraction/motion/cutoff, two unused words, then vec4 start/pre/post
position and quaternion. Branches distinguish flag skip (1), motion below cutoff
(2), completed sweep without correction (3), and correction (4). Original arithmetic,
classification, sweep, interpolation and physics-buffer writes are retained.

There are 39 GPU records, three identical observations for each of 13 selected
steps 221–233. Duplicate waits are retained, checked and collapsed only in derived
analysis. All selected recorded post positions/quaternions correspond **bit for
bit at float32 precision** to their GPU trace values. All 13 CPU classification
positions/quaternions equal the corresponding CPU trace records; all classifications
are false and the CPU TOI hook emits zero selected records. Source identity and the
recorded branch explain the latter; it is not an inference about a GPU fraction.
[analysis.json](analysis.json) retains the full window, hex words and derivations.

## Frozen budget and provenance

[Protocol](raw/protocol.json) SHA256:
`267a0e661267832ef81807e63e3dd5431721f626fd2b094d7b58c30b105cf1ee`.
Driver **92458 terminated with exit 0**. The one-time budget is fully consumed
and closed:

- One native Rust library build, including its four normal hull C units.
- One original-prefix C++ fixture compilation/link.
- One fresh native diagnostic process, 240 original completed steps, dt 1/60,
  four substeps and original sleep/defaults/settings. Build/link/process watchdogs
  900/120/300 seconds; no timeout occurred.
- Three copied observer source files, zero CPU solver builds, production
  candidates, retries or headline timing runs.

The observer lives in a separate copied workspace. It adds a GPU output binding
and reads the resulting buffer only after normal completion. It retains the
original native backend, ordering 0, cache requests and `bounded-static-sort` AB
selection; no phase-capture policy is substituted. Only trace/cache output paths
and the observer-output flag differ in invocation configuration. The unchanged
artifact CPU observer and 240-step C++ prefix fixture from the previous campaign
are reused. Early successful exit completes the diagnostic prefix, not later
full dragging assertions. All commands, settings and receipts are preserved.

All **107 actual observer Rust inputs** are archived and checked before/after
build. Differences from the frozen production provider are the three observer
sources and the same isolated native manifest/lock already used by that provider.
All **453 pinned native backend source/configuration inputs** match the prior
actual provider receipt and are archived. Successful Cargo artifact records
show the same features/profile as the prior native provider and a fresh engine
compilation. Invocation revision `379fe7f` is context only.

The C compiler wrapper delegates to `/usr/bin/cc` and adds dependency emission
only. It preserves every compiler invocation, actual flags, four successful
compiled source/object receipts and dependency files. Their **107 actual compiler
dependencies**, including system headers, are hashed and archived. The independent
CPU solver archive and combined wrappers are reused with exact hashes and their
earlier complete source/object/archive receipts. The offline verifier validates
that earlier portable chain, rather than treating a checkout label as provenance.

| Produced output | SHA256 |
| --- | --- |
| Artifact native observer library | `652bc603cc8ded7bc854c4ad7cfc2a2f2a790dadcc54402b166e455ac5f3fdec` |
| Diagnostic executable | `897d144395bcbb46cc7e843c6c9a42649187273937dc35a45167b07c25087e6e` |

The actual process reports **NVIDIA GeForce RTX 4070 SUPER / Vulkan**. A separate
read-only post-capture query records driver **610.57.04** and full Rust compiler
identity. Receipt elapsed clocks are incidental; no latency/FPS/speed claim is
made. The original frozen library `native-candidate.a` with SHA
`53819158e2321f1f270fa3d885664ce6519770f6448163d4894de1041b19bc3e`
is unchanged. The shared `target/native-cache-build/release/libgpu_physics.a`
now contains the **diagnostic observer**; do not use it as a production provider.
Use retained named libraries/receipts or a new budgeted production build. See
[current source applicability](source-applicability.json).

## Neutrality, limits and offline reproduction

All **14,020 original B/F/M/P lines** through frame 239 match the archived native
baseline exactly: 1,200 paired body headers, 2,400 body-state records and 240
completed original steps, including printed contact features, anchors,
separations and impulses. This establishes neutrality of captured
nine-significant-digit records. It does not establish all future-relevant cache
equality or equality beyond the prefix. The original full failed baseline is
retained. The dedicated GPU words additionally expose the selected internal CCD
boundary directly; no fraction is inferred from environment requests or post poses.

```sh
python3 -B experiments/gpu-physics/benchmarks/production-readiness/pr04-native-ccd-boundary-2026-10-02/validate.py
```

This needs no GPU, original absolute artifact paths or build. It verifies all
**32 portable raw files / 6,430,264 bytes**, the fixed build/link/process budget,
source archives/patches, Cargo/provider identities, actual C command/dependency
receipts, earlier independent CPU/C consumer provenance, full prefix equality,
exact internal post-lane correspondence and reproducible analysis.
The preceding report must remain alongside this report in the repository.
[Raw index](raw-index.json) and [reference map](reference-map.json) retain identities.
No executable, object/static-library binary, pipeline cache or changed production
source is checked in. The receipts validate the recorded compilation chain;
independently recreating the compiler's output would require a new build.

The next repair must address the demonstrated CCD activation difference while
preserving all CCD/support/restitution and original full dragging screens. A
threshold-only rollback is already rejected: the historical paired control
fails settling at frame 2460. Its existing energy analysis shows slow edge
toppling; it does not establish that observed release-energy change is unique
to GPU paired solving. Do not repeat that unchanged control or waive its limits.
Review CPU substep motion/extent semantics and the historical paired-settling
boundary before freezing a distinct, evidence-supported candidate budget.

The [static flag/acceptance audit](handoff-audit.json) finds that `apply_deltas`
already computes CPU-style motion before setting `FAST`/`CCD_NO_HIT` with the
shell-capped cutoff. The directly recorded native flags are `98312`, including
both flags. They affect next-step padding, mesh refresh and CCD handoff; the old
host-only rollback did not update them. This is a concern to resolve, not proof
of the cause of its late toppling. The existing low-speed landing regression
also requires first height0.505±0.001 and next height≥0.495; those limits remain
required. Resolve that landing/loaded-joint interaction with a faithful independent
reproducer before choosing a candidate. No consistent-cutoff change is assumed
to pass, and no candidate is selected in this report.
