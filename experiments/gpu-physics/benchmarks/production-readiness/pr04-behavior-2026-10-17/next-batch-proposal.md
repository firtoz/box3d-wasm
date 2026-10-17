# Optional additional Rain throughput batch

**Not authorized or launched.** The existing three Rain/performance batches are
spent. This proposal requires an explicit additional budget; it does not reset
those batches or activate a new goal. The full PR04 objective stays unchanged.

The previous clean-getter-copy lead is ruled out by the current source. Instead,
this batch would address the measured joint-filter fallback: Rain has 4,200
unique pairs, while the current lookup has 4,096 slots. Overflow sends every
query through the full joint scan. Prior timestamps put broadphase near 186 ms
per loaded step. The previous artifact only moved cheap geometric rejection
before that scan; it did not remove the capacity mismatch.

One hypothesis and one candidate: size the internal lookup from actual distinct
joint-pair demand, keeping a power-of-two capacity and the existing conservative
scan fallback. Update Rust/WGSL scratch offsets and upload metadata together.
Preserve pair identity, collide-connected semantics, physics parameters, scene
size, timestep, geometry, picking, Box3D and WASM. No solver equation changes.

Maximum wall time: 45 minutes, with a single bounded execution phase capped at
1,900 seconds inside that time. Exact maximum operations:

- Two engine-library builds and two test builds, 180 seconds each.
- Four original C fixture/shared-source compiles, 15 seconds each; three links,
  25 seconds each, using the existing frozen CPU/API inputs.
- One actual CPU 336-step Rain control, 30 seconds, before two GPU controls,
  180 seconds each, at the original worker/timestep/substep/recycling settings.
- Two focused lookup groups, 60 seconds each: original scan-oracle coverage plus
  demand beyond 4,096, reversed/duplicate pairs, collide-connected transitions,
  and high body slots. Compare accepted-pair decisions, not scene trajectories.
- Four original physical regression groups, 90 seconds each, ordinary/native
  and ordering0/1. Preserve the original assertions and numerical limits.
- At most 60 seconds for metadata/cache inventory, source/library restoration,
  cleanup and audit. No captures, sweeps, extra candidates or automatic retries.

Progress requires no lost/incorrect filtering and either a discriminating causal
result or at least 20% lower loaded completed-step time on both GPU backends
without a required physical regression. Report completed SPS, simulated SPS,
p95/max and timestamped phases; keep the 60-SPS target unchanged. Stop after
this batch, even if promising. Rejected source is restored; an artifact result
alone does not qualify a production repair, Rain600 or the whole roadmap.

Default new temporary allowance: 1 GiB including observed shared-cache growth;
raw cap128MiB, retained diagnostic evidence64MiB, free-space floor20GiB. Reuse
shared build caches; no copied cache/tree. Name/delete objects, test traces and
executables after consumption on both outcomes. Preserve the existing frozen
archives. Candidate archives may exist only as temporary inputs; any retained
binary exception must be explicitly budgeted before launch with its consumer.

Before execution, verify source pins, freeze the exact commands and source
restoration protocol, and reject duplicate labels. This document authorizes
nothing by itself.
