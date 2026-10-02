# Current Rust qualification baseline — PR03 remains open

195 of199 selected checks pass on the current ordinary/native GPU executables.
Two selectors fail on each backend. Their complete original logs remain in this
report. No production or test source, tolerance or default was changed.

| Baseline checks | Ordinary | Native |
| --- | --- | --- |
| Twelve separate exported trace fixtures |12pass,1408frames |12pass,1408frames |
| Native full-replay/reentry | Not applicable | PASS,48frames/42actual replay hits |
| Numerical precision |28pass |28pass |
| Remaining CCD-selected suite |28pass |28pass |
| Schedule/capacity suite |22pass /1fail |22pass /1fail |
| Remaining analytical/lifecycle suite |7pass /1fail |7pass /1fail |
| Total |97pass /2fail |98pass /2fail |

The CCD mutation fixture's separate five-frame process plus the28-test suite
cover all29current CCD-selected tests on each backend. The historical27-test
count is not reused. All five mixed/automatic scheduling tests and four original
complete-component schedule comparisons pass unchanged, including mixed
12288body topology,4096independent-group bodies,15000cube impact/repeats,
transitions and explicit overrides. The original30000cube insertion-capacity
control passes. Existing island and capacity-loss checks retain their assertions.
These are correctness fixtures, not a scaling or scheduling timing campaign.

The25exported traces preserve2864 complete schema24 frames: support600,
friction240, restitution240, prismatic120, mass mutation18, joint/contact wake
73each, callbacks12, contact retirement3, child-joint history16, child-query8,
CCD mutation5 perbackend, plus native replay48. Separate process/file identities
prevent fixtures from overwriting each other's captures. Original physical,
reference, energy, impulse and identity assertions pass in these selected cases.
Every compressed trace round-trips its exact raw bytes and hash. This is one
baseline run per selected cell, not five-process repeat qualification or proof
that schema24 captures every future-relevant registry/ABI allocation field.
The creation-identity fixture internally captures4frames and removes its file;
it has no export hook, so no external trace is claimed for that selector.

## Failures and source applicability

- `contact_order_survives_body_capacity_growth_and_invalidates_proxy_changes`
  fails its initial `assert!(before.0)` on both backends. The frozen baseline
  uses efficient ordering0. Source explicitly disables live-order storage and
  seeding in this mode; its snapshot returns `params.order_enabled!=0`.
  This fixture's storage-preservation assertion therefore requires the separate
  CPU-compatible ordering configuration. Preserve both failed0observations;
  the separately frozen required1configuration does not replace them.
- `high_resistance_sleeper_wakes_on_velocity` fails “impulse must wake a
  sleeping capsule” on both backends. Its source constructs a generation1body
  handle after previous serial tests create/destroy worlds. The current world
  registry raises child generations beyond previously issued values; an
  invalid handle can satisfy the fixture's initial `!awake` assertion and
  cannot wake a live body. This source-supported fixture identity hypothesis
  still needs a generation-safe reproducer/fixture check before classifying
  actual sleep/wake behavior. No sleep assertion was removed or relaxed.

[Source review](source-review.json) records the relevant implementations/locations
and distinguishes inference from executed behavior. Exact source bytes are in
[the current source archive](raw/compiled-source-inputs.tar.gz). The failures
remain open in the production roadmap alongside the preserved distance and
strict ragdoll trajectory failures; adjacent passing wake fixtures do not close
this sleeper check.

## Protocol, provenance and limitations

The [protocol](raw/protocol.json), SHA
`a30762d9a3803bcfca730218b421607a7faf408ba967ef4417b1c9020746a2ca`,
freezes33processes:25separate trace fixtures and8serial suites;99ordinary and
100native exact selected checks. Budget: zero builds, candidates, retries and
headline timing. Each case runs once. Expected baseline assertion failures are
retained while remaining declared cells run; input mismatch, timeout, missing
actual NVIDIA Vulkan or invalid requested capture stops the campaign. Driver
8018 exits0 after completing the planned matrix; four child suite exits101
remain failures. No ignored test counts as a pass.

Retained source milestone `abc0a54a3ce3b285c9984f2c7c6b5baefff00d1b` and actual
producer receipts identify compiled code; invocation revision1329d52 is context.
Ordinary test executable SHA
`597b893bcbbb401866721e1b1b45cb03c58263ae8bee76cbe8af00cdfaef23b9`;
native SHA`34ede43142323516f585d4d236755b8ef46dc6d9e61b6ba51354f379c85aa4a3`.
All107Rust and723viewer producer source inputs verify unchanged during the
campaign. The exact names are frozen from already verified host lists without
new test-list/device processes. The standalone engine source archive and existing
[viewer source archive](../pr02-world-lifetime-viewer-builds-2026-10-02/raw/compiled-source-inputs.tar.gz)
provide portable bytes for receipt validation. Cached dependencies/build-source
applicability do not close PR06 clean-build acceptance.

Every GPU-containing process logs the actual NVIDIA GeForce RTX4070SUPER Vulkan
adapter, desktop driver610.57.04. Pure host checks inside mixed suites do not
individually require or acquire an adapter. The native cells additionally use the
pinned native-cache policy. Exact geometry, defaults, fixture-specific settings,
commands and existing limits remain in the archived source and protocol; source
assertions remain authoritative. Native replay explicitly proves42actual hits,
including after substep/transform reentry. No headline performance, FPS or
full-release readiness follows. Internal fixture diagnostic/process clocks are
excluded from performance/chart acceptance. Scheduling/default policies and
Box3D/WASM stay unchanged.

100 indexed raw files retain every protocol/driver, host list, source archive,
producer receipt, process stdout/stderr and25lossless traces. Older datasets
remain unchanged; compiled binaries, objects, libraries and driver pipeline-cache
blobs are not checked in. Original pipeline caches remain in the local artifact
directory; they are not test results or portable source provenance.

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr03-rust-baseline-2026-10-02/validate.py
```

This offline check verifies raw/source hashes, exact selectors/counts, all33exit
receipts, actual adapter proof, retained failures, trace schema/frame/hash counts
and replay-hit assertions, without local ignored binaries or GPU execution.
Do not rerun the closed engine driver. Remaining PR03 work includes the current
Rain/drag baseline, mode/fixture applicability and full future-state contract.
