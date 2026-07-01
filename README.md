# box3d-wasm

Box3D compiled to WebAssembly, with TypeScript bindings and a browser demo.

[![TypeScript](https://img.shields.io/badge/TypeScript-5.9-blue)](https://www.typescriptlang.org/)
[![Bun](https://img.shields.io/badge/Bun-1.x-black)](https://bun.sh/)
[![Turborepo](https://img.shields.io/badge/Turborepo-2.x-EF4444)](https://turbo.build/repo)

## What You Get

- A publishable npm package for Box3D wasm bindings.
- A browser demo showing Box3D + Three.js.
- A Turborepo workspace that keeps the package, demo, docs, and tests separate.

## Goals

- Keep the wasm-facing API small and stable.
- Expose primitive Box3D bindings first.
- Layer a nicer TS API on top.
- Keep the demo and package buildable from the same repo.

## Layout

- `box3d/`: git submodule checkout of the upstream engine source.
- `packages/box3d-wasm`: package source and generated bindings.
- `demo/`: browser demo and showcase.
- `docs/`: API notes and usage docs.
- `integration-test/`: smoke tests and harnesses.

## Quick Start

This repo is still being bootstrapped, but the intended flow is:

```bash
bun install
bun run dev
```

## Monorepo Scripts

- `bun run dev` - run the demo workspace
- `bun run build` - build the full workspace
- `bun run typecheck` - typecheck all packages
- `bun run lint` - lint all packages
- `bun run clean` - clear build output

## Baseline Tools

- `bun`
- `cmake`
- `emscripten`
- a C/C++ compiler toolchain
- `git submodule update --init --recursive`

The intended flow is to keep the upstream `box3d` repo pinned as a submodule and layer wasm-specific glue on top in this repo.

## Inspiration

- Inspired by `box2d-wasm` and `box2d3-wasm`.
- Inspired by `cf-multiworker-starter-kit` for the monorepo/docs shape.
- The focus is a clean developer-facing package, not a production runtime wrapper.
