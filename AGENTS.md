# Notes for code-generation agents

## Project overview

This is a WASM port of Erin Catto's [Box3D](https://github.com/erincatto/box3d) physics engine. It compiles the Box3D C library via Emscripten and exposes a manual C bridge (`b3w*` functions) wrapped by TypeScript (`Box3DRuntime`, `PhysicsWorld`). A browser demo (Three.js) hosts ported C++ sample scenes.

Key docs:
- `docs/README.md` — documentation index and placement guide
- `docs/TYPESCRIPT_API.md` — public TypeScript API usage guide and examples
- `docs/SAMPLES.md` — port status of ~136 upstream C++ samples; **Easy next ports** queue + status tables
- `docs/OTHER_PROJECTS.md` — comparison with other Box3D WASM projects (update WASM size here)
- `docs/WASM_API_SURFACE.md` — API binding checklist (~70 TS methods, adding as we go)
- `README.md` — project readme (may need updates for new features, build steps, etc.)
- `patches/box3d/README.md` — local patches applied on top of the clean `box3d/` submodule for WASM builds

## Box3D patches (keep the submodule clean)

Do **not** leave lasting dirty edits in `box3d/`. Engine changes that we need before upstream has them live as patches under `patches/box3d/*.patch`.

- WASM build runs `packages/box3d-wasm/scripts/prepare-box3d.ts`, which exports a clean `git archive` of the submodule into `packages/box3d-wasm/.box3d-patched/` and applies those patches.
- **Apply failure fails the build** on purpose. If you bump the `box3d` submodule and a patch no longer applies, refresh or drop the patch, update `patches/box3d/BASE_SHA`, and rebuild.
- Prefer contributing durable engine APIs upstream when practical; keep patches small and documented in `patches/box3d/README.md`.
- Regenerating: edit under `box3d/`, `git diff > ../patches/box3d/<name>.patch`, write `BASE_SHA`, then `git checkout -- .` so the submodule is clean again.

## Documentation structure

Keep docs cohesive. Do not add tiny standalone sections that only make sense in the context of the immediate change.

- `README.md` is for repository overview, setup, scripts, requirements, and links to deeper docs.
- `docs/README.md` is the docs index and should route readers to the right document.
- `docs/TYPESCRIPT_API.md` is for public TypeScript usage guidance. If adding API examples, include enough context to be useful: imports, runtime/world setup, body/shape creation, stepping, transform reads, and cleanup when relevant.
- `docs/WASM_API_SURFACE.md` is a binding checklist. Keep it factual and terse; do not turn it into a tutorial.
- `docs/SAMPLES.md` is for upstream sample port status, missing API notes, and sample-specific implementation notes.
- `docs/OTHER_PROJECTS.md` is comparative context. Update it when API style, sample counts, WASM size, threading story, or project positioning materially changes.
- Design/performance notes belong in focused docs such as `docs/washer-performance-plan.md`, not in README unless they affect setup or user-facing behavior.

Before adding a new section, check whether the content belongs in an existing doc. If it is public API guidance, prefer extending `docs/TYPESCRIPT_API.md` and linking from `docs/README.md` or `README.md` instead of creating an isolated island.

## Keep docs in sync

Every change should update all relevant docs to keep them accurate. As a rule: if you touch code, check which docs reference that code and update them. Specifically:

- **New sample added** → update `docs/SAMPLES.md` status table, optionally update `README.md` sample list
- **New WASM binding added** → update `docs/WASM_API_SURFACE.md` (mark item `[x]`), update `docs/TYPESCRIPT_API.md` if the binding changes public usage patterns, update `docs/OTHER_PROJECTS.md` API coverage table if it changes the API surface counts
- **Public API ergonomics changed** → update `docs/TYPESCRIPT_API.md` with a coherent example, then update `README.md` or `docs/README.md` only as navigation if needed
- **WASM binary size changes** → rebuild, check gzipped size, update `docs/OTHER_PROJECTS.md` (per-project WASM size row + 4-way table), update the current-size line below
- **README.md** references features, build instructions, or sample counts → keep in sync

When in doubt, grep for references to the file or API you changed.

## What's next (session loop)

The user often runs short agent sessions: ask what's next → do one unit of work → commit/push → new chat → repeat. Treat that as the default product workflow.

### When the user asks "what's next"

1. Check `git status` for unfinished local work. If the tree is dirty, prefer finishing/committing that before starting a new sample (say so clearly).
2. Read `docs/SAMPLES.md` → **Easy next ports** (top of that section is the queue).
3. Recommend **one** primary next item (plus at most 1–2 alternates). Prefer:
 - `[ ]` + `🔧` (APIs already wrapped) over `🚧` / `🧩`
 - Small, dump-ready samples over mesh/mover/events megasamples
 - Infra fixes only when they block ports or you just found a real bug
   Before recommending, skim the upstream C++ sample class (do not trust a `🔧` note alone). Confirm it creates bodies in `m_worldId` and is dumpable. Manifold collide demos, geometry editors, and similar no-body tools are not generic-host ports — see the Manifold section note in `docs/SAMPLES.md`.
4. Answer briefly: what to do, why it's next, and what "done" means (sample + `SAMPLES.md` + dump compare when applicable).

Do **not** invent a parallel todo list outside `docs/SAMPLES.md`. The Easy next ports list and status tables are the backlog.

### When the user says "ok let's do it" (or similar)

1. Do **one** queue item end-to-end (or the agreed alternate)—not a grab-bag of future ports.
2. Follow Goal / scene / dump-match guidance below.
3. Update `docs/SAMPLES.md` in the same change: mark the sample `[x]`, refresh notes, and rewrite **Easy next ports** so the finished item is gone and the new top entries are still the best next picks.
4. When the user asks to commit (and push), do that; do not commit unless asked. After push, a fresh agent should be able to answer "what's next" from `docs/SAMPLES.md` alone.

### Hand-off checklist (before ending a port session)

- [ ] Sample registered and behaves like upstream (geometry + defaults)
- [ ] `bun run compare:sample -- sample="<id-or-name>"` green when the sample is dump-enabled
- [ ] `docs/SAMPLES.md` status + Easy next ports updated
- [ ] Related binding/docs size updates if you touched WASM (`WASM_API_SURFACE.md`, `OTHER_PROJECTS.md`)

## Goal

Port all samples, then add extra samples for any API not covered by upstream samples. Approach:

1. Pick the next sample from `docs/SAMPLES.md` → **Easy next ports** (or an explicit user choice)
2. If it needs WASM bindings that don't exist yet, add them:
   - Add C bridge function in `packages/box3d-wasm/cmake/box3d_web_*.c`
   - Put new C bridge functions in the domain-specific bridge file (`box3d_web_body.c`, `box3d_web_shape.c`, `box3d_web_joint.c`, `box3d_web_math.c`, etc.). If no good domain file exists, add one and wire it into `CMakeLists.txt`; do not pile unrelated helpers into the nearest existing file.
   - Add TypeScript `cwrap` signature and wrapper in `packages/box3d-wasm/src/index.ts`
   - Rebuild WASM, check gzipped size, update `docs/OTHER_PROJECTS.md`
   - Update `docs/WASM_API_SURFACE.md` and `docs/SAMPLES.md`
3. Implement sample: worker + host file, register in `demo/src/samples/index.ts`
4. Update `docs/SAMPLES.md` status **and** the Easy next ports queue

## Sample rendering guidance

Ported upstream samples should match the C++ sample behavior and visible geometry exactly unless the user explicitly approves a variation. The known exception is Dominoes, where the current variation is intentional.

Be careful with Box3D geometry conventions when building Three.js render specs:

- `b3MakeBoxHull(hx, hy, hz)` uses half-extents. Three.js `BoxGeometry(x, y, z)` uses full dimensions, so render boxes as `[2 * hx, 2 * hy, 2 * hz]`.
- `b3CreateCylinder(height, radius, yOffset, sides)` creates a Y-axis hull from local `y = yOffset` to `y = yOffset + height`. Three.js `CylinderGeometry(radius, radius, height)` is Y-axis but centered at local origin, so render it with local position `[0, yOffset + 0.5 * height, 0]` when it is part of a compound body.
- The shared capsule render helper defaults to X-axis. If a physics capsule is built along Y or Z, set the render spec capsule `axis` field to match instead of sneaking in an unrelated body rotation.
- For simple render bodies, remember that some physics shapes are not centered on the body origin. If the physics shape endpoints or transformed hull center are offset in local space, set the render body's `localPosition` (and `localRotation` if needed) to match the actual local geometry instead of shifting the body transform.
- Preserve upstream body transforms. If the C++ sample sets `bodyDef.rotation`, put the same quaternion in both the worker body definition and the host render spec.

Match upstream defaults, not just visible geometry:

- Do not override `b3DefaultShapeDef`, `b3DefaultBodyDef`, `b3DefaultWorldDef`, or joint-def defaults unless the upstream sample actually changes that field.
- Be especially careful with density, gravity, gravity scale, joint base constraint tuning, damping, restitution, friction, rolling resistance, sleep/awake flags, and explosion parameters. A single wrong default can make a sample look "basically right" at rest but behave very differently under impulses, explosions, motors, or dump comparison.
- If a TS port needs an explicit value, verify it against the upstream sample code first rather than guessing from the visual result.

For one physics body with multiple visible shapes, use the generic host compound render spec (`kind: "compound", parts: [...]`) instead of adding fake render bodies. Fake bodies break worker snapshot/body-index mapping and picking.

For benchmark-heavy samples (many bodies), prefer the shared multi-layer **shader-instanced host** (`demo/src/samples/shader-instanced-host.ts`, ~7 floats per instance) over the **dominoes approach** (`InstancedMesh` with `setMatrixAt`, ~16 floats per instance). Box/sphere benches and Washer (shader path) / Rain already use it; see those samples for patterns.

## Scene file pattern

Every sample should have a single `*-scene.ts` file as the single source of truth for both the physics worker and the render host. It exports:

- `build*DynamicBodies(world, runtime)` — creates physics bodies on the worker
- `*GroundSize()` — returns ground **half-extents** (`Vec3`)
- `*Bodies` (static array) or `create*Bodies()` (procedural) — render body descriptors
- `*Camera` (optional) — camera config `{ position, target }`
- Dump metadata: `dumpSampleName`, `dumpSampleId`, `dumpCppSampleName`, `dumpGroundSize = *GroundSize`, `dumpBuildDynamicBodies = build*DynamicBodies`

The worker imports the builder and ground size from the scene file:
```ts
import { buildXxxDynamicBodies, xxxGroundSize } from "./xxx-scene";
```

The render spec imports render bodies, camera, and ground size, doubling the half-extents for Three.js `BoxGeometry`:
```ts
const half = xxxGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: xxxBodies,
  camera: xxxCamera,
};
```

This ensures positions, sizes, and camera never drift between the physics and render sides.

Simple stacking/shape samples can use the generic host (`demo/src/samples/generic-host.ts`).

## Dump-match readiness (new samples)

When porting a sample, make the scene dump-ready in the same PR when practical. Goal: `bun run compare:sample -- sample="<id-or-name>"` is green at the default `1e-5` tolerance on checkpoints `0,50,100,200,300`.

### Required scene exports

In `*-scene.ts`, export:

- `dumpSampleName`, `dumpSampleId`, `dumpCppSampleName`
- `dumpGroundSize` / `dumpBuildDynamicBodies` **or** a full `dumpCreate` when ground/body setup is nonstandard
- Optional: `dumpStep` / `dumpPostStep` when upstream does work around `Sample::Step`
- Optional: `dumpInteractionSchedule` + `dumpRunInteraction` for scripted impulses/motors/explosions (mirror the same schedule in `tools/reference-dump/reference-dump.cpp`)

Keep body creation order identical to upstream; dumps compare by enumeration order.

### Match upstream physics defaults

- World gravity is `b3DefaultWorldDef`: `[0, -10, 0]` (not Earth `-9.81`). Demo workers and `createWorld()` default to `-10`.
- Do not invent density, friction, restitution, damping, sleep, gravity scale, or joint tuning — copy the C++ sample fields exactly (including omitted fields that leave native defaults).
- Preserve pre/post-step order: if C++ applies wind/forces after `Step`, export `dumpPostStep` and call the same helper from the live worker after `world.step`.
- If the sample uses `m_stepCount` for spawn or scripted actions, drive the live worker from a per-physics-step counter (`PhysicsWorkerBase.totalSteps` increments inside `stepPhysics`). Never key cadence off snapshot publish throttling.

### Float32-safe setup math

JavaScript `*`/`+` are float64. Prefer helpers from `demo/src/samples/f32.ts` (`f32`, `f32Mul`, `f32Add`, …) for any loop/index arithmetic that upstream writes with `float`/`0.1f`/`1.1f * i`. See `docs/TYPESCRIPT_API.md` (float32 helpers) and past fixes in Bullet vs Stack / Wind Drop / ragdoll piles.

Use runtime `makeQuatFromAxisAngle` / `b3wSin`/`b3wCos` (or `b3wSinf`/`b3wCosf` when upstream calls `sinf`/`cosf` directly) for orientations that must match Box3D.

### Verify before calling it done

```sh
bun run compare:sample -- sample="<sample id or name>"
# denser diagnosis if needed:
# bun run compare:sample -- sample="..." frames=0,1,2,...,50 epsilon=1e-5
```

Some multi-contact piles only match for an early window then drift (documented in `docs/SAMPLES.md`); that is acceptable only after setup/order/float32 bugs are ruled out. Use `--disable-sleep-term` via `compare-sample.sh` when sleep early-exit hides a later checkpoint (e.g. far ragdolls).

Details: `docs/reference-dump-plan.md`, `tools/reference-dump/README.md`.

## WASM binary size

The release WASM binary is at `demo/public/wasm/box3d-web.wasm`. When making changes that affect the compiled WASM output (adding new C API bindings, changing compile flags, etc.), rebuild and check the gzipped size:

```sh
gzip -c demo/public/wasm/box3d-web.wasm | wc -c
```

Then update the `WASM size` row in `docs/OTHER_PROJECTS.md` (both the per-project comparison table and the 4-way table).

Current size: ~239KB gzipped (559KB raw).
