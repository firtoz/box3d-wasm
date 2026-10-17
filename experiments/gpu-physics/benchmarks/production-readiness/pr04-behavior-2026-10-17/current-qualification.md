# Retained-policy qualification

This qualifies the contact-normal policy published as `8d18dc8b6`; it does not
close PR04. Gate D is closed with the linked source applicability; C and D
and F are now closed, with three gates left. The production solver source is unchanged during these checks.

The original C Falling Ragdolls fixture passes all 600 steps on ordinary/native
GPU with both contact-order modes, after a real Box3D CPU reference. Each run
contains 67,312 finite, ordered body records and verifies the 112 typed joints,
normalized endpoints and float32 setup fields. GPU peak anchor separation is
0.116392836 m (limit 0.25); final-window separation is 0.00298925536 m (limit
0.005), with zero final-window linear/angular speed. The CPU peaks are
0.213310137/0.00139051978 m. Peak angular speed remains within the original
1.2× CPU bound. Initial-state, first-step and frame-2 spine-contact calculation
checks also pass. Full chaotic trajectory/final-pose matching is deferred.

The same finite phase passes the four original isolated spherical fixtures,
60-step no-contact ragdolls and all 45 grid/12 torus frozen mesh poses, for
40 CPU/GPU cells total. Mesh input bytes match the original PR03 frozen copies;
contact/point counts and normal/separation multisets retain their 1e-5 bounds.
See [summary](mesh-ragdoll-qualification-summary.json), the frozen protocol,
receipt, and individual records under `qualification-results/`. This phase took
82.457 s including compilation/linking/observation, not a physics SPS benchmark.

The original combined distance fixture passes all four 240-step cases on each
backend/ordering combination (16 cases): all 13 state lanes at 1e-5 and the
original warm-start toggles. The original native joint-separation fixture passes
2,686 checks. Ordinary reaction/separation pass 2,856/2,686 checks; the earlier
native reaction timeout remains archived. After per-kind diagnosis, the
original unfiltered 36-world native recipe passes both ordering modes at its
separately frozen 120-second confirmation cap (89.441/88.863 seconds). The
original assertions and process-lifetime sequence are preserved.

The remaining original Rust selectors are reconciled in
`rust-qualification-matrix.json`: all400requiredcells areaccountedfor, with
286current-buildpasses and114explicitmode-0source-equivalence reuses;four
configurationexclusions followthe originalcontract.
The first 120-second aggregate group completed 18 tests and was interrupted
during creation identity. Later groups consumedonly previously unattempted
selectors; onefinalstandalone30s check passedidentityin1.299s. Theoriginalfailed
aggregate remainsfailed; itwasnot anindividualfixturetimeout. Historical
mode-0 CCD/numerical evidence is identified as source-equivalence reuse rather
than fresh current-build execution.

All temporary objects/executables and consumed CPU-reference data are removed.
The mesh/ragdoll phase deleted 68,840,992 bytes of raw outputs plus 461,743 bytes
of compact CPU inputs, with zero shared-cache growth. A Bun empty-directory
cleanup error was reconciled explicitly after all reference files had already
been deleted. Frozen development archives retain their prior storage exception
and named qualification/release consumers; shared caches are preserved.

Rain's full 600-step physical gate and throughput/hitch failures remain open.
Release applicability is closed by the subsequent gate F refreeze below. Frozen geometry probes are not a full Rain pass.
Recordings are optional; this introduces no new full-catalog visual claim.
[Related-regression acceptance](related-qualification.json) reconciles all
52 non-Rain C cells and the 50 applicable trace-fixture cells, with evidence reuse
explicit. Rain stays in gate B; the measured native runtime is not a speed pass.
The inventory retains 165 rows, with Falling Ragdolls explicitly qualified only
at its physical-fixture scope and the other 164 individually unreviewed for the
current policy. Gate count stays **3 closed, 3 open**.

Gate F subsequently completed the [current-source release refreeze](refreeze-report.md).
The old source-applicability opening is reconciled by20frozen candidate artifacts;
Rain/fullbaseline andperformance gatesA/B/E remainopen.
