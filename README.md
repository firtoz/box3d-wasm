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
- A browser demo built with Vite and Three.js (responsive layout with touch camera / body-drag / shoot controls on phones and tablets).
- A Turborepo workspace that keeps the package and demo in sync.

## Goals

- Keep the wasm-facing API small and stable.
- Expose primitive Box3D bindings first, with branded handle types for safety.
- Layer an opt-in object-oriented TS API on top.
- Keep the demo and package buildable from the same repo.

## Layout

- `box3d/`: git submodule checkout of the upstream engine source (keep clean).
- `patches/box3d/`: optional local patches applied onto a build copy during WASM compile (see `patches/box3d/README.md`).
- `packages/box3d-wasm/`: package source, wasm build scripts, and generated artifacts.
- `demo/`: browser demo and showcase.
- `docs/`: notes and usage docs.
- `experiments/gpu-physics/`: experimental Rust/WGSL native physics engine (separate from the Box3D WASM package). See [`experiments/gpu-physics/README.md`](./experiments/gpu-physics/README.md) and the [CPU/GPU falling-cube charts, renderer previews and measured limits](./experiments/gpu-physics/README.md#falling-cube-scaling-benchmark).
- `integration-test/`: smoke tests and harnesses.

## Quick Start

Install the [requirements](#requirements) for the path you want to run, then
initialize the checkout:

```bash
git submodule update --init --recursive
bun install
```

For the Box3D WASM browser demo:

```bash
bun run setup:emsdk
bun run dev
```

For the native CPU/GPU comparison viewer on this branch:

`bun install` only installs JavaScript packages. Install Rust separately before
running the GPU samples. On **Arch/Manjaro**, if Rust is not already installed:

```bash
sudo pacman -S --needed rustup
rustup default stable
cargo --version
```

If you get `cargo: command not found`, complete the Rust setup above first.
After installing the remaining [native requirements](#native-cpugpu-samples-featgpu),
build and run the viewer; Cargo downloads the Rust crate dependencies automatically:

```bash
bun run samples:both --build-only
bun run samples:both
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
- `bun run samples:cpu` / `bun run samples:gpu` / `bun run samples:both` - native CPU, experimental GPU, or side-by-side sample viewer. See [`experiments/gpu-physics/README.md`](./experiments/gpu-physics/README.md).

## Requirements

### Box3D WASM browser demo

- Bun 1.2.0
- A C/C++ toolchain
- CMake
- Python 3 and Bash for Emscripten setup
- Emscripten (**prefer 6.0.2**). System packages like Manjaro/Arch `6.0.3` currently ICE compiling Box3D `shape.c` with `-msimd128`. For this repo, install a local pin with `bun run setup:emsdk` (downloads into gitignored `.emsdk/`; WASM builds prefer it automatically). Override with `BOX3D_EMSDK_DIR` / `BOX3D_EMSDK_VERSION`, or set `BOX3D_DISABLE_SIMD=1` as a last-resort workaround.
- Git submodules initialized with `git submodule update --init --recursive`

### Native CPU/GPU samples (`feat/gpu`)

Testing so far is limited to Arch Linux/Manjaro environments. The development
machines are:

| Machine | CPU | GPU | Validation |
|---|---|---|---|
| Current Manjaro desktop | Intel Core i9-9900K | NVIDIA GeForce RTX 4070 SUPER | Selected contact-island and restitution fixtures pass on both GPU paths; see the [qualification report](./docs/gpu-solver-qualification.md). Full solver qualification and controlled performance measurements remain incomplete. |
| Earlier test laptop | AMD Ryzen 9 8945HS | NVIDIA GeForce RTX 4070 Laptop GPU and AMD Radeon 780M integrated graphics | Recorded NVIDIA benchmarks and native runtime checks on both GPUs; see the [test coverage](./experiments/gpu-physics/README.md#tested-platforms-and-hardware). |

Other Linux distributions, macOS and Windows have not been verified. Build paths
for those platforms do not imply tested support, and the laptop benchmark results
should not be treated as measurements of the desktop GPU.

- Git and initialized submodules, Bun, Python 3.10+, CMake 3.24+, and a native C17/C++20 toolchain.
- A current stable Rust toolchain (`rustc` and `cargo`) for `samples:gpu`, `samples:both`, and the direct Rust viewer. The CPU-only sample viewer does not build Rust.
- Linux: `pkg-config`, OpenGL, X11, Xi, Xcursor and GTK 3 development packages; GNU binutils (`nm`, `objcopy`) or LLVM equivalents for GPU/combined linking. The default cached Vulkan build also uses Bash, `patch` and `flock`.
- A working graphics driver and desktop display. Linux GPU physics needs a Vulkan loader and the Vulkan driver for your GPU; the native sample viewer renders with OpenGL/X11 (XWayland on Wayland desktops).
- macOS: Xcode command-line tools and LLVM archive tools. Windows: Visual Studio C++ Build Tools, Windows SDK, LLVM archive tools and a matching Rust MSVC toolchain. These paths still await CI and hardware verification.
- Optional: FFmpeg for MP4 recordings; Wayland and xkbcommon libraries for the direct Rust viewer's Wayland path.

See the [native dependency installation commands](./experiments/gpu-physics/README.md#dependencies)
and [platform/backend guide](./experiments/gpu-physics/compiler/native-backend/README.md).
Native samples do not require Emscripten. Bun installs JavaScript dependencies,
Cargo downloads Rust crates, and CMake fetches ImGui, ImPlot and Native File Dialog
on the first native build, so initial setup needs network access.

## Notes

The upstream `box3d` engine is pinned as a submodule. Keep that tree clean; WASM-only engine tweaks (for example profile levels) live under `patches/box3d/` and are applied to `packages/box3d-wasm/.box3d-patched/` at build time. A patch that fails to apply fails the build — refresh patches when bumping the submodule.

Start with [`docs/TYPESCRIPT_API.md`](./docs/TYPESCRIPT_API.md) for TypeScript usage examples across both the primitive and object APIs, then use [`docs/WASM_API_SURFACE.md`](./docs/WASM_API_SURFACE.md) for the current binding checklist and API expansion TODOs.

## License

This repository is MIT licensed under `./LICENSE`.
The vendored `box3d/` submodule is licensed separately under Erin Catto's MIT license.

## Inspiration

- Inspired by `box2d-wasm` and `box2d3-wasm`.
- Inspired by `cf-multiworker-starter-kit` for the monorepo/docs shape.
- The focus is a clean developer-facing package, not a production runtime wrapper.
