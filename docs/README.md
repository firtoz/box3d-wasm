# Docs

Project docs are split by audience and purpose. Prefer updating an existing doc over adding a small disconnected section elsewhere.

## Getting Started

- [`TYPESCRIPT_API.md`](./TYPESCRIPT_API.md) - user-facing TypeScript API guide covering the branded-handle primitive API, the opt-in object wrapper API, and browser/headless usage examples.
- [`../README.md`](../README.md) - repository overview, setup, scripts, requirements, and high-level links.

## Implementation Tracking

- [`WASM_API_SURFACE.md`](./WASM_API_SURFACE.md) - binding checklist for C bridge and TypeScript wrapper coverage.
- [`SAMPLES.md`](./SAMPLES.md) - upstream Box3D sample port status and missing API notes.
- [`reference-dump-plan.md`](./reference-dump-plan.md) - plan for C++/WASM sample transform dumps, local generated comparisons, and CI coverage.

## Project Context

- [`OTHER_PROJECTS.md`](./OTHER_PROJECTS.md) - comparison with other Box3D WASM projects, including API style, sample coverage, threading, and WASM size.
- [`washer-performance-plan.md`](./washer-performance-plan.md) - performance notes for high-body-count sample rendering.

## Where To Put New Docs

- Public API usage belongs in `TYPESCRIPT_API.md` unless it is only a checklist item.
- Binding availability and TODOs belong in `WASM_API_SURFACE.md`.
- Sample porting status belongs in `SAMPLES.md`.
- Repository setup and orientation belong in the root `README.md`.
- Comparative positioning belongs in `OTHER_PROJECTS.md` only when API shape, sample counts, WASM size, or project positioning changes.
