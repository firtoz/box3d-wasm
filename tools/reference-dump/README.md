# reference-dump

Headless C++ reference dumper for upstream Box3D samples. It compiles the upstream sample source files with no-op UI/rendering stubs and writes checkpointed body transforms as JSON.

Build into the ignored local dump workspace to avoid generated files in `tools/`:

```sh
bun run prepare:clangd   # optional: IDE/clangd compile databases (reference-dump + WASM bridge)
cmake -S tools/reference-dump -B .reference-dumps/reference-build -DBOX3D_DOUBLE_PRECISION=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build .reference-dumps/reference-build -j$(nproc)
```

Run:

```sh
.reference-dumps/reference-build/reference-dump "Single Box" .reference-dumps/single-box/cpp.json
```

Useful options:

```sh
.reference-dumps/reference-build/reference-dump --list-json
.reference-dumps/reference-build/reference-dump --checkpoint-interval 25 --max-frames 200 "Single Box" .reference-dumps/single-box/cpp.json
.reference-dumps/reference-build/reference-dump --frames 0,1,2 "Single Box" .reference-dumps/single-box/cpp.json
.reference-dumps/reference-build/reference-dump --disable-sleep-term --frames 0,50,100,200,300 "Pile" out.json
```

`compare-dumps.ts` supports `--summary` for first-divergence / worst-abs / worst-ulp reporting. `wasm-dump.ts` accepts the same `--disable-sleep-term` flag as the C++ dumper; `compare:sample` wires both sides for samples that need a full window.

Sample names match upstream `RegisterSample` names by default. When multiple categories share the same name (e.g. Mesh/Box vs Ragdoll/Box), pass `Category/Name`:

```sh
.reference-dumps/reference-build/reference-dump "Ragdoll/Box" .reference-dumps/ragdoll-box/cpp.json
```

Compare a WASM port against the C++ reference from the repository root:

```sh
bun run compare:sample -- sample="single-box"
bun run compare:sample -- sample="Stacking / Sphere Stack" frames=0,50,100
bun run compare:sample -- sample="Sphere Stack"
```

The compare script resolves sample arguments through the frontend `demo/src/samples/index.ts` list. You can pass the frontend ID, frontend display name, or upstream C++ sample name for dump-enabled samples.

Extra local samples (not in upstream `box3d/samples/` anymore) can live beside this README and be listed in `CMakeLists.txt`. Currently: `sample_card_house_thick.cpp` for the removed Stacking / Card House Thick scene.

When enabling dump support for a new port, follow the checklist in `AGENTS.md` (Dump-match readiness) and `docs/reference-dump-plan.md` (New sample dump checklist): gravity `-10`, float32-safe setup math, shared `dumpStep`/`dumpPostStep` helpers, and dual-maintained interaction schedules.

Smoke test from the repository root:

```sh
bun run test:reference-dump
```

The JSON shape is:

```json
{"checkpoints":[{"frame":0,"bodies":[{"p":[0,0,0],"q":[0,0,0,1],"v":[0,0,0],"w":[0,0,0],"t":2,"a":true}]}]}
```

Checkpoints are emitted every 50 frames through frame 300 by default, covering 5 seconds at 60Hz. The dumper stops early after a checkpoint when the awake solver set is empty, but not before frame 100. The high-level `compare:sample` script uses sparse exact checkpoints by default: `0,50,100,200,300`.
