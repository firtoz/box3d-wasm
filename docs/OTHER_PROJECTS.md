# Box3D WASM Projects

This document compares several independent efforts to bring Erin Catto's Box3D physics engine to the web. The projects take different approaches: comprehensive JavaScript-facing API bindings, ergonomic embind wrappers, TypeScript-first sample ports, and full native testbed ports.

Upstream project:

- **Box3D**: https://github.com/erincatto/box3d
- **Author**: Erin Catto
- **Description**: 3D physics engine for games, written in portable C17 with samples, multithreading, SIMD, recording/replay, contacts/events, queries, character mover, meshes, height fields, and joints
- **Initial release**: https://github.com/erincatto/box3d/releases/tag/v0.1.0

## Contents

- [Isaac Mason - box3d.js](#isaac-mason)
- [Luis Montes - box3d-wasm](#monteslu)
- [Firtina Ozbalikchi - @firtoz/box3d-wasm](#firtoz)
- [Erik Sombroek - box3d-wasm](#eriksom)
- [Overall Comparison](#overall-comparison)
- [API Coverage](#api-coverage)

<a id="isaac-mason"></a>
## Isaac Mason - box3d.js (`box3d.js` npm package)

- **X post**: https://x.com/isaac_mason_/status/2072530527501103187
- **Live demo**: https://isaac-mason.github.io/box3d.js/ (28+ example pages)
- **Repo**: https://github.com/isaac-mason/box3d.js
- **npm**: `box3d.js`
- **JS Physics Benchmarks**: https://isaac-mason.github.io/js-physics-benchmarks/
- **Benchmark repo**: https://github.com/isaac-mason/js-physics-benchmarks

### Background

Isaac Mason ([@isaac_mason_](https://x.com/isaac_mason_/)) is a web developer and physics engine enthusiast. His `box3d.js` package is the most API-complete JavaScript-facing Box3D binding in this comparison. The measured release WASM size is **~309KB gzipped (806KB raw)**; the ~320KB number from the original announcement is approximate.

Isaac also maintains [JS Physics Benchmarks](https://isaac-mason.github.io/js-physics-benchmarks/), comparing JavaScript/WASM physics engines including Box3D, Rapier, Jolt, and others.

### Approach

Isaac compiles the Box3D C library with Emscripten and embind, but the binding design is much closer to the C API than a typical fluent JS wrapper. Function names and parameter conventions mirror the upstream C API: `b3CreateWorld`, `b3World_Step`, `b3CreateBody`, etc.

Key details:

- **Toolchain**: Emscripten, CMake, `emcmake`, embind, and a Node build script
- **Binding style**: 1:1 Box3D C API mirror, so upstream docs and C samples translate directly
- **Build flavours**: `box3d.js`, `box3d.js/inline`, `box3d.js/mt`, and `box3d.js/mt-inline`
- **Events API**: Reusable WASM-backed events buffer with typed array readers, avoiding per-event WASM/JS crossings and GC churn
- **Contacts API**: Reusable contacts buffer for current contact manifold inspection
- **Queries**: Raycast closest, raycast all, shapecast, AABB overlap, shape overlap, mover casts, and mover collision
- **Advanced geometry**: Meshes, mesh generators, compounds, heightfields, hull utilities, GJK, manifolds, time of impact, collision helpers
- **Callbacks**: Custom filter and pre-solve callbacks
- **Tooling APIs**: Debug draw, dynamic tree, recording/replay, math utilities
- **Examples**: 28+ focused examples covering shapes, stacking, robustness, CCD, joints, ragdolls, character movement, compounds, triangle meshes, mesh generators, casts, GJK, manifolds, dynamic tree, events, contacts, multithreading, and replay

### Developer Quotes

From Box2D Discord, July 2, 2026:

> *"I think for a good wasm version completely auto-generating bindings will be not so ideal. There are quirks around embind and webidl binder that require decisions."*

> *"Also, in the bindings I created there, I have an events API that internally lets JS just read out from a typed array. No GC, no frequent wasm->JS calls or JS->wasm calls."*

> *"Let me know if I can be a help in 'official-izing' anything re wasm/javascript!"*

<a id="monteslu"></a>
## Luis Montes - box3d-wasm (`box3d-wasm` npm package)

- **X post**: https://x.com/monteslu/status/2072730028488716744
- **Live demo**: https://box3d.netlify.app/ (Three.js + Box3D)
- **Repo**: https://github.com/monteslu/box3d-wasm
- **npm**: `box3d-wasm`

### Background

Luis Montes ([@monteslu](https://x.com/monteslu/)) is a web/games developer with prior experience publishing WASM physics packages to npm. His `box3d-wasm` package is a standalone JavaScript/TypeScript library, with a separate Three.js demo app showing ragdolls, dominoes, and a drivable buggy.

### Approach

Monteslu compiles the **Box3D C library only** into WASM, then wraps it with a hand-written C++ embind layer (`csrc/glue.cpp`, roughly 1800 lines). The public API is object-oriented and JavaScript-friendly: `World`, `Body`, `Shape`, and typed joint classes.

Key details:

- **Toolchain**: Emscripten, `emcmake`, CMake, and embind
- **Build process**: Fetches a pinned Box3D source SHA, builds `libbox3d.a`, then links `glue.cpp` plus Box3D into an ES module
- **Build flavours**: `standard` single-threaded and `deluxe` threaded, both SIMD-enabled
- **Threading**: Threaded build uses `-pthread` and a `PTHREAD_POOL_SIZE` based on `navigator.hardwareConcurrency`
- **API style**: Fluent object API, e.g. `world.createBody({ type: 'dynamic' }).createBox({ halfExtents: { x: 1, y: 1, z: 1 } })`
- **Type shape**: Vectors and quaternions are plain JS objects compatible with Three.js patterns
- **Runtime targets**: `web`, `worker`, and `node`
- **Filesystem**: Disabled via `-sFILESYSTEM=0` for smaller output
- **Memory**: Uses `-sALLOW_MEMORY_GROWTH=1`
- **Package**: Published to npm as `box3d-wasm`

Monteslu's API exposes a broad practical subset of Box3D: bodies, shapes, all 9 joint types, raycast closest, explosion, contact events, sensor events, and body move events. It does not attempt to expose every lower-level Box3D facility such as GJK, mesh generation, heightfields, dynamic tree, recording/replay, or all query variants.

<a id="firtoz"></a>
## Firtina Ozbalikchi - [@firtoz/box3d-wasm](https://github.com/firtoz/box3d-wasm)

- **Repo**: https://github.com/firtoz/box3d-wasm
- **GitHub**: https://github.com/firtoz
- **Website**: https://firtoz.com
- **X**: https://x.com/firtoz
- **Package status**: Not published to npm yet; internal package is currently private

### Background

Firtina Ozbalikchi ([@firtoz](https://x.com/firtoz/)) is a product engineer and creative technologist working across AI, WebGL/XR, games, Cloudflare-native tooling, and TypeScript systems. His GitHub profile highlights previous Three.js and Unity/game tooling work, including `react-three-renderer`, `Unity3D-TextureAtlasSlicer`, Cloudflare Worker starter kits, and TypeScript developer tooling.

The [`@firtoz/box3d-wasm`](https://github.com/firtoz/box3d-wasm) project is a TypeScript-first Box3D WASM workspace with a browser demo, custom Three.js visualization, and per-sample ports of Box3D's C++ samples.

### Approach

[`@firtoz/box3d-wasm`](https://github.com/firtoz/box3d-wasm) compiles the **Box3D C library only** into WASM and exposes a manual TypeScript wrapper over a custom C bridge. The current API surface is still comparatively small, but the project is expanding it incrementally as more samples and features are ported.

Key details:

- **Toolchain**: Emscripten, CMake, Bun, TypeScript, Vite, and Turborepo
- **Binding method**: Manual C bridge functions with `b3w*` prefixes, wrapped by TypeScript classes (`Box3DRuntime`, `PhysicsWorld`)
- **API style**: Mid-level TypeScript API using named enums, branded handles, typed option objects, and tuple vectors, plus an opt-in object wrapper layer for `BodyRef`/`ShapeRef` ergonomics
- **Renderer**: Included Three.js browser demo
- **Samples**: 43 C++ sample scenes currently ported to TypeScript, with a tracking document for the remaining ~136 upstream samples
- **Build flavours**: Release and profile builds
- **Threading model**: Emscripten pthreads are enabled in the WASM build (`USE_PTHREADS=1`), with Box3D worker-count controls exposed; the demo also runs simulation work through browser workers around that runtime
- **WASM size**: **~231KB gzipped (542KB raw)** for the release binary, built with `-O3`, pthreads, and WASM SIMD enabled
- **Distinct feature**: Includes a custom human/ragdoll helper API (`createHuman`, bone access, human velocity/joint tuning) that is not exposed by the other JS library bindings in this comparison

This project is currently less complete as a general-purpose Box3D API wrapper than Isaac's or Monteslu's packages, but the API is actively growing. Its present smaller surface keeps the WASM output compact while the demo retains direct TypeScript ownership over sample-specific behavior.

<a id="eriksom"></a>
## Erik Sombroek - box3d-wasm

- **X post**: https://x.com/ErikSombroek/status/2072606706853372295
- **Live demo**: https://box3d-7rh.pages.dev/
- **Fork**: https://github.com/ErikSom/box3d
- **PR**: https://github.com/ErikSom/box3d/pull/1

### Background

Erik Sombroek ([@ErikSombroek](https://x.com/ErikSombroek/)) previously worked with Alex Birch (Birch-san) on bringing Box2D v3 to the web via [box2d3-wasm](https://github.com/Birch-san/box2d3-wasm), building on top of Birch-san's earlier [box2d-wasm](https://github.com/Birch-san/box2d-wasm). For Box3D, Erik focused on getting the native Box3D sample application running in the browser with minimal upstream changes.

### Approach

Erik compiled the **entire C++ samples app** into a single WASM binary: Box3D library, sokol renderer, imgui UI, assets, and all ~136 sample scenes. It is effectively the native app running on the web.

Key details:

- **Toolchain**: Emscripten with `-pthread`, mapping Box3D's native pthread usage to web workers
- **Graphics**: sokol with `SOKOL_WGPU`, using WebGPU through Dawn's `emdawnwebgpu` port
- **Shader compilation**: WGSL generated at build time with `sokol-shdc`
- **Threading**: Fixed pthread pool of 16 workers (`B3_WEB_MAX_WORKERS=8` per world x 2 worlds)
- **Memory**: Fixed 512MB heap (`-sINITIAL_MEMORY=536870912`) because WebGPU rejects growable shared memory
- **Data**: Mesh/asset files preloaded with Emscripten `--preload-file`
- **Deployment**: Cloudflare Pages with COOP/COEP headers for `SharedArrayBuffer`
- **Patch size**: Around 260 lines, mostly fenced behind `if(EMSCRIPTEN)` / `#ifdef __EMSCRIPTEN__`

This gets all native samples, renderer features, imgui controls, assets, and multithreading working in the browser. It is excellent as a showcase and repro-sharing tool, but it does not expose individual Box3D APIs as a reusable JavaScript library.

### Upstream Discussion

In [box3d/discussions/36](https://github.com/erincatto/box3d/discussions/36) (July 2, 2026), ErikSom described the port:

> *"I took a shot at getting the testbed running in the browser. It's a straightforward Emscripten port. The existing renderer runs unchanged on WebGPU (including the GTAO compute shaders), and the solver runs multithreaded on web workers."*

He framed the PR primarily as a **showcase**: a browser-hosted version of the native testbed that could stay close to upstream and be shared easily.

<a id="overall-comparison"></a>
## Overall Comparison

| Aspect | Isaac Mason (`box3d.js`) | Monteslu (`box3d-wasm`) | [`@firtoz/box3d-wasm`](https://github.com/firtoz/box3d-wasm) | ErikSom |
|--------|---------------------------|--------------------------|------------------------|---------|
| Nature | WASM library + 28+ examples | WASM library + separate demo | TS-first workspace + bundled demo | Full native app port |
| C++ compilation | Box3D library only | Box3D library only | Box3D library only | Entire app: Box3D + sokol + imgui + samples |
| Binding method | Embind 1:1 C API mirror + JS facade | Embind class wrapper | Manual C bridge + TypeScript wrapper | No reusable JS binding layer |
| JS API style | Low-level C API mirror | Fluent object API | Mid-level TypeScript branded handles plus opt-in object refs | N/A; runs native app |
| Samples/examples | 28+ API-focused examples | Custom Three.js demo scenes | 43/136 C++ samples ported | All ~136 native samples |
| Renderer | Per-example custom rendering | Three.js in separate demo repo | Three.js included in repo | Native sokol/WebGPU |
| UI | Example-specific UI | Demo-specific UI | HTML/CSS/Three.js demo UI | imgui from native testbed |
| npm package | `box3d.js` | `box3d-wasm` | Not published yet | No |
| Node.js support | Yes | Yes | Browser-focused currently | No reusable library API |
| Multithreading | Single-threaded + threaded builds | Single-threaded + threaded builds | Emscripten pthread-enabled WASM + browser workers in demo/runtime | Emscripten pthread pool |
| Events | Zero-GC typed-array buffers for events and contacts | JS arrays from embind (`getContactEvents`, `getSensorEvents`, `getBodyEvents`) | Event toggles exist; event buffers/callbacks not exposed | Native app internals |
| Queries | Raycast, shapecast, overlap, mover queries | Raycast closest only | Raycast closest only | Native app internals |
| Advanced geometry | Meshes, heightfields, compounds, generators, GJK | Not exposed | Compounds plus basic mesh construction/mesh shape binding; no heightfield yet | Native app internals |
| WASM size | ~309KB gzipped (806KB raw) | ~211KB gzipped (521KB raw) standard; ~218KB gzipped (533KB raw) deluxe | ~231KB gzipped (542KB raw) | Large app binary, fixed 512MB heap |
| Best fit | Comprehensive low-level Box3D API for JS | Ergonomic JS physics library | Growing TS-first wrapper + sample-porting playground | Browser-hosted native testbed/showcase |

<a id="api-coverage"></a>
## API Coverage

This table focuses on APIs callable directly from JavaScript. ErikSom's project uses the Box3D API internally in native C++ code, but does not expose those functions as a reusable JS API.

| API Area | Isaac (`box3d.js`) | Monteslu | [`@firtoz/box3d-wasm`](https://github.com/firtoz/box3d-wasm) | ErikSom |
|----------|---------------------|----------|------------------------|---------|
| **World lifecycle** (create, destroy, step, gravity, counters, profile) | ✓ | ✓ | ✓ | N/A |
| **World settings** (sleep, CCD, warm start, contact tuning, workers, thresholds) | ✓ | ◐ | ◐ | N/A |
| **Body lifecycle** (create, destroy, type get/set) | ✓ | ✓ | ✓ | N/A |
| **Body transforms** (position, rotation, transform get/set, target transform) | ✓ | ✓ | ◐ | N/A |
| **Body velocity** (linear, angular get/set) | ✓ | ✓ | ◐ | N/A |
| **Body forces** (force at point, force to center, torque) | ✓ | ✓ | ✗ | N/A |
| **Body impulses** (linear impulse at point, to center, angular impulse) | ✓ | ✓ | ◐ | N/A |
| **Body mass/inertia** (mass, mass data, inertia, centers of mass) | ✓ | ◐ | ◐ | N/A |
| **Body state/flags** (awake, sleep, enabled, bullet, locks, damping, gravity scale) | ✓ | ✓ | ◐ | N/A |
| **Body queries** (local/world point/vector, point velocity, AABB, shapes, joints) | ✓ | ◐ | ◐ | N/A |
| **Basic shape creation** (sphere, capsule, box/hull, transformed hull) | ✓ | ✓ | ✓ | N/A |
| **Advanced shape creation** (mesh, compound, heightfield) | ✓ | ✗ | ◐ | N/A |
| **Hull construction** (points, cylinder, cone, rock, clone, vertices) | ✓ | ◐ | ◐ | N/A |
| **Mesh construction/generators** | ✓ | ✗ | ◐ | N/A |
| **Compound construction** | ✓ | ✗ | ✓ | N/A |
| **Heightfield construction** | ✓ | ✗ | ✗ | N/A |
| **Joint types available** | 9/9 | 9/9 | 6/9 | N/A |
| **Joint runtime controls** | ✓ | ✓ | ◐ | N/A |
| **World queries** (raycast closest/all, shapecast, overlap AABB/shape) | 5/5 | 1/5 | 1/5 | N/A |
| **Body queries** (raycast, shapecast, overlap) | ✓ | ◐ | ✗ | N/A |
| **Mover queries** (cast mover, collide mover) | ✓ | ✗ | ✗ | N/A |
| **Contact events** (begin, end, hit) | ✓ | ✓ | ✗ | N/A |
| **Sensor events** (begin, end) | ✓ | ✓ | ✗ | N/A |
| **Body move events** | ✓ | ✓ | ✗ | N/A |
| **Joint break events** | ✓ | ✗ | ✗ | N/A |
| **Pre-solve callback** | ✓ | ✗ | ✗ | N/A |
| **Custom filter callback** | ✓ | ✗ | ✗ | N/A |
| **Explosion** | ✓ | ✓ | ✗ | N/A |
| **Debug draw** | ✓ | ✗ | ✗ | N/A |
| **Dynamic tree** | ✓ | ✗ | ✗ | N/A |
| **Recording/replay** | ✓ | ✗ | ✗ | N/A |
| **GJK/collision geometry** | ✓ | ✗ | ✗ | N/A |
| **Mass/AABB without body** | ✓ | ✗ | ✗ | N/A |
| **Character/mover helpers** (solve planes, clip vector) | ✓ | ✗ | ✗ | N/A |
| **Math utilities** | ✓ | ✗ | ✗ | N/A |
| **Human/ragdoll helper** | ✗ | ✗ | ✓ | N/A |

### API Surface Summary

| Metric | Isaac (`box3d.js`) | Monteslu | [`@firtoz/box3d-wasm`](https://github.com/firtoz/box3d-wasm) | ErikSom |
|--------|---------------------|----------|------------------------|---------|
| Approx. JS-callable functions/methods | ~280 (250+ embind + ~30 facade helpers) | ~140 | ~95 | 0 reusable JS API |
| Joint types exposed | 9/9 | 9/9 | 6/9 | 0 |
| World query types | 5/5 | 1/5 | 1/5 | 0 |
| Event groups exposed | 4/4 | 3/4 | 0/4 | 0 |
| Body force/impulse groups | 4/4 | 4/4 | 2/4 | 0 |
| Mass/inertia accessor groups | 8/8 | ~3/8 | ~2/8 | 0 |
| Advanced shape families (mesh/compound/heightfield) | 3/3 | 0/3 | 1/3 | 0 |
| WASM gzipped | ~309KB | ~211KB standard; ~218KB deluxe | ~231KB | N/A |

Isaac's larger WASM size is largely explained by the much larger exposed API surface. Each embind binding, value object, callback adapter, and facade helper adds marshalling code to the generated module. Monteslu's package is also embind-based, but it wraps a smaller practical set. [`@firtoz/box3d-wasm`](https://github.com/firtoz/box3d-wasm) sits between Monteslu's single-threaded standard build and threaded deluxe build in gzipped size. These rows should not be read as a direct performance comparison; the binaries differ in binding layer, exported runtime surface, threading/memory choices, and exact build inputs.
