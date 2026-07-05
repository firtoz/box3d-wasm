# Notes for code-generation agents

## Project overview

This is a WASM port of Erin Catto's [Box3D](https://github.com/erincatto/box3d) physics engine. It compiles the Box3D C library via Emscripten and exposes a manual C bridge (`b3w*` functions) wrapped by TypeScript (`Box3DRuntime`, `PhysicsWorld`). A browser demo (Three.js) hosts ported C++ sample scenes.

Key docs:
- `docs/SAMPLES.md` — port status of ~136 upstream C++ samples (13 done)
- `docs/OTHER_PROJECTS.md` — comparison with other Box3D WASM projects (update WASM size here)
- `docs/WASM_API_SURFACE.md` — API binding checklist (~70 TS methods, adding as we go)
- `README.md` — project readme (may need updates for new features, build steps, etc.)

## Keep docs in sync

Every change should update all relevant docs to keep them accurate. As a rule: if you touch code, check which docs reference that code and update them. Specifically:

- **New sample added** → update `docs/SAMPLES.md` status table, optionally update `README.md` sample list
- **New WASM binding added** → update `docs/WASM_API_SURFACE.md` (mark item `[x]`), update `docs/OTHER_PROJECTS.md` API coverage table if it changes the API surface counts
- **WASM binary size changes** → rebuild, check gzipped size, update `docs/OTHER_PROJECTS.md` (per-project WASM size row + 4-way table), update the current-size line below
- **README.md** references features, build instructions, or sample counts → keep in sync

When in doubt, grep for references to the file or API you changed.

## Goal

Port all samples, then add extra samples for any API not covered by upstream samples. Approach:

1. Pick a sample from `docs/SAMPLES.md` (start with "Easy next ports")
2. If it needs WASM bindings that don't exist yet, add them:
   - Add C bridge function in `packages/box3d-wasm/cmake/box3d_web_*.c`
   - Add TypeScript `cwrap` signature and wrapper in `packages/box3d-wasm/src/index.ts`
   - Rebuild WASM, check gzipped size, update `docs/OTHER_PROJECTS.md`
   - Update `docs/WASM_API_SURFACE.md` and `docs/SAMPLES.md`
3. Implement sample: worker + host file, register in `demo/src/samples/index.ts`
4. Update `docs/SAMPLES.md` status

## Sample rendering guidance

For benchmark-heavy samples (many bodies), prefer the **washer approach** (custom `ShaderMaterial` with per-instance quaternion attribute, ~7 floats per instance) over the **dominoes approach** (InstancedMesh with `setMatrixAt`, ~16 floats per instance). See `demo/src/samples/washer.ts` for the pattern.

Simple stacking/shape samples can use the generic host (`demo/src/samples/generic-host.ts`).

## WASM binary size

The release WASM binary is at `demo/public/wasm/box3d-web.wasm`. When making changes that affect the compiled WASM output (adding new C API bindings, changing compile flags, etc.), rebuild and check the gzipped size:

```sh
gzip -c demo/public/wasm/box3d-web.wasm | wc -c
```

Then update the `WASM size` row in `docs/OTHER_PROJECTS.md` (both the per-project comparison table and the 4-way table).

Current size: ~207KB gzipped (487KB raw).
