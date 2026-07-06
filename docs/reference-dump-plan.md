# Reference Dump Plan

Goal: compare upstream C++ Box3D sample behavior against TypeScript/WASM ports by dumping body transforms at deterministic checkpoint frames and diffing both outputs.

## Current Status

- `tools/reference-dump/` builds a headless C++ executable that compiles upstream sample sources with no-op UI/render stubs.
- The headless `Sample` stub mirrors upstream physics lifecycle behavior: constructor defaults, random seed reset, world creation settings, contact recycle distance, step flags, pause/single-step timestep logic, step counters, and profile capture.
- Visual/UI work, cursor picking, camera work, mesh loading, and debug rendering remain no-op unless a future sample needs an explicit physics-relevant replacement.
- The tool uses Box3D internal headers in `dump-core.c` to enumerate bodies through `b3World::bodyIdPool`.
- JSON output includes checkpoint frames and per-body position, rotation, linear velocity, angular velocity, body type, and awake state.
- Low-level dumpers emit checkpoints every 50 frames through frame 300 by default, covering 5 seconds at 60Hz. CLI flags can override interval, max frame, start frame, or exact frames.
- The high-level `compare:sample` workflow uses sparse exact checkpoints by default: `0,50,100,200,300`.
- `--list-json` emits the registered sample list in a machine-readable format.
- The run stops early after a checkpoint when the awake solver set is empty, but not before frame 100.
- Verified with `Single Box`: checkpoints at `0,50,100`, two bodies, valid JSON.
- `scripts/reference-dump-smoke.sh` builds in `/tmp`, checks `--list-json`, and validates `Single Box` exact-frame JSON.
- `scripts/wasm-dump.ts` emits the same JSON shape for the WASM `Single Box` port.
- `scripts/compare-dumps.ts` compares dump files; current dump-enabled samples all match at the default `1e-5` tolerance. Many stable scenes still match at `1e-6`; more chaotic stacks such as `Cylinder Stack`, `Card House`, and `Card House Thick` currently need the looser threshold.
- `scripts/compare-sample.sh` builds/runs both dumpers for a requested sample and writes generated outputs under ignored `.reference-dumps/`.
- `scripts/wasm-dump.ts` uses the same `demo/src/samples/index.ts` sample list as the frontend for IDs and display names, then enables C++ comparison for samples whose `*-scene.ts` module exports `dumpSampleId`, `dumpCppSampleName`, `dumpGroundSize`, and `dumpBuildDynamicBodies`.
- No generated dump fixtures are committed. Clear `.reference-dumps/` whenever stale outputs are no longer useful.
- The previous `Single Box` drift was caused by the TypeScript wrapper overwriting native shape friction defaults after shape creation. Omitted material fields now preserve the native defaults.
- `Pyramid2D` exposed a motion-lock bridge bug: the WASM C bridge initialized `b3MotionLocks` in the wrong field order, causing `lockLinearZ` to become `angularZ`. The bridge and TypeScript argument order now match upstream `b3MotionLocks`.

## Build And Smoke Test

Build outside the repository tree:

```sh
cmake -S tools/reference-dump -B /tmp/reference-dump-build -DBOX3D_DOUBLE_PRECISION=OFF
cmake --build /tmp/reference-dump-build -j$(nproc)
```

Smoke test:

```sh
/tmp/reference-dump-build/reference-dump "Single Box" /tmp/single-box.json
node -e "const fs=require('fs'); const d=JSON.parse(fs.readFileSync('/tmp/single-box.json','utf8')); console.log(d.checkpoints.length)"
```

Or run the scripted smoke test:

```sh
bun run test:reference-dump
```

Compare a supported WASM sample against the C++ reference:

```sh
bun run compare:sample -- sample="Sphere Stack"
```

Generated files are written under `.reference-dumps/<sample>/` by default. Use `clean=1` to clear the local dump workspace before running.

## Remaining Work

### 1. Harden The C++ Reference Dumper

- Add CLI flags for checkpoint interval, max frames, start frame, and optional exact frame list. Done.
- Add a machine-readable sample listing mode, for example `--list-json`. Done.
- Decide how to handle samples that depend on mesh assets currently stubbed by `CreateMeshData` and `LoadTempMesh`.
- Add a small test script that builds the dumper in `/tmp` and validates `Single Box` JSON. Done.

### 2. Add WASM Dump Runner

- Create `scripts/wasm-dump.ts`. Done for the currently dump-enabled samples listed above.
- Load the project WASM runtime in Node/Bun. Done by importing the web Emscripten artifact with `wasmBinary` and a small environment shim.
- Build a selected ported sample scene using exported worker scene-builder functions. Done for the currently dump-enabled samples listed above.
- Step at the same timestep and substep count as upstream samples. Done for the currently dump-enabled samples listed above.
- Emit the same JSON shape as `reference-dump`. Done.
- Stop early when all dynamic bodies are asleep, matching the C++ runner behavior. Done.

### 3. Refactor Sample Workers

- Refactor worker files to export deterministic scene builder functions while preserving current demo behavior. Done for the currently dump-enabled samples listed above.
- Start with `Single Box` or the existing simplest generic-host sample. Done for the currently dump-enabled samples listed above.
- Keep body creation order stable because dumps compare bodies by enumeration order.
- Avoid fake render-only physics bodies; compound render specs should remain render-only.

### 4. Add Dump Comparator

- Create `scripts/compare-dumps.ts`. Done.
- Compare checkpoint count, frame numbers, body counts, body types, awake state, positions, rotations, linear velocities, and angular velocities. Done.
- Support tolerances, but default to a threshold that keeps the full paired sample set green. Done, default `--epsilon 1e-5`.
- On mismatch, rerun both dumpers over a narrowed frame range to find the first divergent frame.

### 5. Local Generated Dumps

- Keep generated dump files out of git. Done via ignored `.reference-dumps/`.
- Compare any requested sample with `bun run compare:sample -- sample="Sample Name"` as long as the frontend sample has a deterministic `*-scene.ts` module with the dump exports. The sample argument can be the frontend ID, frontend display name, or upstream C++ sample name.
- Keep generated C++ and WASM outputs side by side for diagnosis, then delete `.reference-dumps/` when no longer needed.

### 6. Add CI Coverage

- Add a script entry that builds `reference-dump` outside the repo tree.
- Run C++ and WASM dump generation for selected ported samples.
- Run `compare-dumps.ts` against freshly generated dumps.
- Use `xvfb-run` only if a future dependency needs a display; the current reference dumper is headless.

## Open Decisions

- Whether mesh-heavy upstream samples should use no-op mesh stubs, real asset loading, or be excluded until the corresponding WASM ports exist.
- Whether dump comparison should initially compare all bodies or only dynamic bodies.
- Whether bisection should be automatic in `compare-dumps.ts` or a separate command.

## Notes

- Keep C++ body enumeration inside C files, not C++, because Box3D internal C headers contain inline code that is not C++-clean.
- Keep generated dump and CMake output in ignored `.reference-dumps/` for ad-hoc comparisons. The smoke script may still use `/tmp` for disposable CI-style validation.
- If C bridge bindings are added later for dump support, update `docs/WASM_API_SURFACE.md`, `docs/TYPESCRIPT_API.md`, and `docs/OTHER_PROJECTS.md` as needed.
