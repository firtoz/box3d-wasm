# box3d-wasm

Box3D compiled to WebAssembly, with TypeScript bindings and a browser demo.

[Open the live samples and demo](https://firtoz.github.io/box3d-wasm/).


https://github.com/user-attachments/assets/1c161c46-1dae-45a0-8d77-cb8df45819e8



[![Bun](https://img.shields.io/badge/Bun-1.2.0-black)](https://bun.sh/)
[![TypeScript](https://img.shields.io/badge/TypeScript-7.0.1--rc-blue)](https://www.typescriptlang.org/)
[![Turborepo](https://img.shields.io/badge/Turborepo-2.10.2-EF4444)](https://turbo.build/repo)
[![License: MIT](https://img.shields.io/badge/License-MIT-green)](./LICENSE)

## What You Get

- A `box3d-wasm` package with TypeScript entrypoints for the wasm bindings.
- A browser demo built with Vite and Three.js.
- A Turborepo workspace that keeps the package and demo in sync.

## Goals

- Keep the wasm-facing API small and stable.
- Expose primitive Box3D bindings first, with branded handle types for safety.
- Layer an opt-in object-oriented TS API on top.
- Keep the demo and package buildable from the same repo.

## Layout

- `box3d/`: git submodule checkout of the upstream engine source.
- `packages/box3d-wasm/`: package source, wasm build scripts, and generated artifacts.
- `demo/`: browser demo and showcase.
- `docs/`: notes and usage docs.
- `integration-test/`: smoke tests and harnesses.

## Quick Start

```bash
bun install
bun run dev
```

## Scripts

- `bun run dev` - run the full workspace in development mode (builds the `release` WASM binary by default).
- `BOX3D_WASM_VARIANT=profile bun run dev` - dev with the profiling WASM build (`wasm/profile/`).
- `BOX3D_WASM_VARIANT=growable bun run dev` - dev with the growable-heap WASM build (`wasm/growable/`).
- `bun run build` - build all WASM variants plus the demo.
- `bun run typecheck` - typecheck workspace packages and scripts.
- `bun run lint` - run workspace lint checks.
- `bun run clean` - clear build output.
- `bun run format` - format repo-owned files while skipping the `box3d/` submodule and generated output.

## Requirements

- Bun 1.2.0
- A C/C++ toolchain
- CMake
- Emscripten
- Git submodules initialized with `git submodule update --init --recursive`

## Notes

The upstream `box3d` engine is pinned as a submodule, and this repo layers wasm-specific build tooling and demo code on top.

Start with [`docs/TYPESCRIPT_API.md`](./docs/TYPESCRIPT_API.md) for TypeScript usage examples across both the primitive and object APIs, then use [`docs/WASM_API_SURFACE.md`](./docs/WASM_API_SURFACE.md) for the current binding checklist and API expansion TODOs.

## License

This repository is MIT licensed under `./LICENSE`.
The vendored `box3d/` submodule is licensed separately under Erin Catto's MIT license.

## Inspiration

- Inspired by `box2d-wasm` and `box2d3-wasm`.
- Inspired by `cf-multiworker-starter-kit` for the monorepo/docs shape.
- The focus is a clean developer-facing package, not a production runtime wrapper.
