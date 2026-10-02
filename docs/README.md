# Docs

Project docs are split by audience and purpose. Prefer updating an existing doc over adding a small disconnected section elsewhere.

## Getting Started

- [`TYPESCRIPT_API.md`](./TYPESCRIPT_API.md) - user-facing TypeScript API guide covering the branded-handle primitive API, the opt-in object wrapper API, and browser/headless usage examples.
- [`../README.md`](../README.md) - repository overview, setup, scripts, requirements, and high-level links.
- [`../patches/box3d/README.md`](../patches/box3d/README.md) - local Box3D patches applied during WASM builds (keep `box3d/` clean).

## Implementation Tracking

- [`WASM_API_SURFACE.md`](./WASM_API_SURFACE.md) - binding checklist for C bridge and TypeScript wrapper coverage.
- [`SAMPLES.md`](./SAMPLES.md) - upstream Box3D sample port status, **Easy next ports** queue, and missing API notes.
- [`SAMPLE_CONTROLS.md`](./SAMPLE_CONTROLS.md) - per-sample C++ UI / keyboard / touch-control parity checklist.
- [`reference-dump-plan.md`](./reference-dump-plan.md) - plan for C++/WASM sample transform dumps, local generated comparisons, and CI coverage.
- [`box3d-submodule-bump.md`](./box3d-submodule-bump.md) - when to advance the `box3d` submodule, and the post-bump dump/API/sample coverage checklist.

## Project Context

- [`OTHER_PROJECTS.md`](./OTHER_PROJECTS.md) - comparison with other Box3D WASM projects, including API style, sample coverage, threading, and WASM size.
- [`washer-performance-plan.md`](./washer-performance-plan.md) - performance notes for high-body-count sample rendering.
- [`gpu-physics.md`](./gpu-physics.md) - experimental GPU engine: support status, missing native APIs, architecture and merge requirements.
- [`goals/gpu-production-readiness.md`](./goals/gpu-production-readiness.md) - authoritative `feat/gpu` readiness queue, evidence-backed checkboxes and compaction/restart recovery; first release scope is Linux/NVIDIA native. Its linked [PR09 sample inventory](../experiments/gpu-physics/benchmarks/production-readiness/pr09-floor-2026-10-02/inventory.md) includes native, browser and CPU oracle catalogs with explicit review gaps.
- [`goals/gpu-warm-start.md`](./goals/gpu-warm-start.md) - bounded GPU warm-start controls goal and verification evidence.
- [`gpu-mixed-scheduling-goal.md`](./gpu-mixed-scheduling-goal.md) - bounded desktop mixed-topology scheduling diagnosis, fixed experiment budget and recovery; performance target unmet, rejected changes restored.
- [`gpu-solver-goal.md`](./gpu-solver-goal.md) - existing full solver criteria, historical recovery and rejected experiments; current production next actions live in the readiness roadmap.
- [`gpu-solver-qualification.md`](./gpu-solver-qualification.md) - GPU-order correctness, stability and same-device repeatability requirements, coverage matrix and outstanding evidence.

## Where To Put New Docs

- Public API usage belongs in `TYPESCRIPT_API.md` unless it is only a checklist item.
- Binding availability and TODOs belong in `WASM_API_SURFACE.md`.
- Sample porting status belongs in `SAMPLES.md`.
- Repository setup and orientation belong in the root `README.md`.
- Comparative positioning belongs in `OTHER_PROJECTS.md` only when API shape, sample counts, WASM size, or project positioning changes.
