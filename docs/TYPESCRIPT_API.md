# TypeScript API

This guide shows the public TypeScript wrapper around the Box3D WASM build. It is intentionally small and practical: create a runtime, create a world, add bodies and shapes, step the simulation, read transforms, and clean up.

For the exhaustive binding checklist, see [`WASM_API_SURFACE.md`](./WASM_API_SURFACE.md).

## Runtime Model

- `Box3DRuntime.load()` loads the WASM module and returns the runtime wrapper.
- `runtime.createWorld()` creates a `PhysicsWorld` wrapper for one Box3D world. Omitted `gravity` defaults to `[0, -10, 0]` (same as upstream `b3DefaultWorldDef`).
- Bodies, shapes, joints, hulls, compounds, and humans are represented by branded numeric handles.
- Vectors are tuple types: `Vec3` is `[x, y, z]`, and `Quat` is `[x, y, z, w]`.
- Body types use the exported `BodyType` enum, not raw Box3D numbers.
- Call `world.destroy()` when a world is no longer needed.

```ts
import { BodyType, Box3DRuntime } from "box3d-wasm";
```

## Getting Started: Cube Tower

This is the smallest useful browser scene: import the runtime, create a Three.js scene, build a floor and a small tower of dynamic cubes, step Box3D, and copy body transforms into meshes. In a Vite-style app, this can live in `src/main.ts` with an `<div id="app"></div>` in `index.html`.

```ts
import * as THREE from "three";
import { BodyType, Box3DRuntime, type Vec3 } from "box3d-wasm";

const app = document.querySelector<HTMLDivElement>("#app");
if (app === null) throw new Error("#app not found");

const scene = new THREE.Scene();
scene.background = new THREE.Color(0x0f172a);

const camera = new THREE.PerspectiveCamera(55, window.innerWidth / window.innerHeight, 0.1, 200);
camera.position.set(8, 7, 10);
camera.lookAt(0, 2, 0);

const renderer = new THREE.WebGLRenderer({ antialias: true });
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
renderer.shadowMap.enabled = true;
app.appendChild(renderer.domElement);

scene.add(new THREE.HemisphereLight(0xe0f2fe, 0x1e293b, 1.2));

const sun = new THREE.DirectionalLight(0xffffff, 2.5);
sun.position.set(8, 12, 6);
sun.castShadow = true;
scene.add(sun);

const runtime = await Box3DRuntime.load();
const world = runtime.createWorld({ gravity: [0, -10, 0] });

const bodies: Array<{ handle: number; mesh: THREE.Mesh }> = [];

function addBox(halfSize: Vec3, position: Vec3, dynamic: boolean, color: number): number {
  const handle = world.createBody({
    type: dynamic ? BodyType.Dynamic : BodyType.Static,
    position,
    isAwake: dynamic,
  });

  world.createHullShape(handle, halfSize, {
    density: dynamic ? 1000 : 0,
    friction: 0.7,
    restitution: 0.05,
  });

  const mesh = new THREE.Mesh(
    new THREE.BoxGeometry(halfSize[0] * 2, halfSize[1] * 2, halfSize[2] * 2),
    new THREE.MeshStandardMaterial({ color, roughness: 0.75 }),
  );

  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = dynamic;
  mesh.receiveShadow = true;
  scene.add(mesh);
  bodies.push({ handle, mesh });
  return handle;
}

addBox([6, 0.5, 6], [0, -0.5, 0], false, 0x1e293b);

const cubeHalfSize: Vec3 = [0.45, 0.45, 0.45];
for (let row = 0; row < 6; row++) {
  for (let col = 0; col < 6 - row; col++) {
    const x = (col - (5 - row) / 2) * 1.02;
    const y = 0.45 + row * 0.92;
    const z = row % 2 === 0 ? 0 : 0.08;
    addBox(cubeHalfSize, [x, y, z], true, 0x60a5fa + row * 0x080808);
  }
}

function animate(): void {
  requestAnimationFrame(animate);
  world.step(1 / 60, 4);

  for (const body of bodies) {
    const transform = world.getBodyTransform(body.handle);
    body.mesh.position.set(...transform.position);
    body.mesh.quaternion.set(...transform.rotation);
  }

  renderer.render(scene, camera);
}

animate();

window.addEventListener("resize", () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
});

window.addEventListener("beforeunload", () => {
  world.destroy();
});
```

The important API shape is `Box3DRuntime.load()` → `createWorld()` → create bodies/shapes → `world.step()` → `world.getBodyTransform()`.

## Primitive And Object Styles

The default API is the primitive one from `box3d-wasm`: bodies, shapes, joints, hulls, and humans are opaque branded handles. This keeps the wrapper close to the underlying WASM API, works well with batching APIs, and adds compile-time protection against mixing handle kinds.

```ts
const body = world.createBody({ type: BodyType.Dynamic, position: [0, 4, 0] });
const shape = world.createHullShape(body, [0.5, 0.5, 0.5], { density: 1000 });

world.setBodyLinearVelocity(body, [1, 0, 0]);
world.setShapeFriction(shape, 0.7);
```

For one-shape convenience bodies where you know you will want the shape immediately, use the paired helpers:

```ts
const box = world.createBoxWithShape({
  size: [0.5, 0.5, 0.5],
  position: [0, 2, 0],
  density: 1000,
});

world.setBodyAngularVelocity(box.bodyHandle, [0, 4, 0]);
world.setShapeFriction(box.shapeHandle, 0.8);
```

If you prefer a more object-oriented authoring style, use the opt-in `box3d-wasm/objects` entry point. It wraps the primitive API with `BodyRef`, `ShapeRef`, `JointRef`, and `HullRef` objects and adds `dispose()` lifecycle methods.

```ts
import { ObjectRuntime } from "box3d-wasm/objects";

const runtime = await ObjectRuntime.load();
const world = runtime.createWorld({ gravity: [0, -10, 0] });

const body = world.createBody({ type: BodyType.Dynamic, position: [0, 4, 0] });
const shape = body.createHullShape([0.5, 0.5, 0.5], { density: 1000 });

body.setLinearVelocity([1, 0, 0]);
shape.setFriction(0.7);
const local = body.getLocalPoint([0, 5, 0]);
const shapes = body.getShapes();

shape.dispose();
body.dispose();
world.dispose();
runtime.dispose();
```

Prefer the primitive API for heavy scenes, hot loops, workers, and batched transform reads. The object API is for ergonomics, not peak throughput.
Object methods accept object refs, not raw handles; if you need to cross from the primitive layer into the object layer, use `objectWorld.body(handle)` explicitly.

## Deterministic Math Helpers

Most applications can use normal JavaScript or Three.js math helpers. For C++/WASM dump parity or deterministic fixtures, shared helpers such as `B3_PI`, `B3_DEG_TO_RAD`, `B3_AXIS_X/Y/Z`, `quatFromAxisAngle`, `runtime.makeQuatFromAxisAngle`, `runtime.b3wSin`, `runtime.b3wCos`, `runtime.randomVec3`, `runtime.lerpVec3`, and `runtime.getLengthAndNormalize` make sample code clearer and keep common constants in one place. When upstream C++ samples use `cosf`/`sinf` from `<math.h>` (rather than Box3D's Bhāskara I approximation), use `runtime.b3wCosf`/`runtime.b3wSinf` instead.

```ts
import { B3_AXIS_X, B3_PI, quatFromAxisAngle } from "box3d-wasm";

const renderRotation = quatFromAxisAngle(B3_AXIS_X, -0.5 * B3_PI);
const physicsRotation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, -0.5 * B3_PI);
```

These helpers are mainly for exact numeric parity work. Ordinary gameplay and rendering code does not need them. Use pure `quatFromAxisAngle` for static render specs or general TS code; use runtime-backed `makeQuatFromAxisAngle` when matching Box3D physics setup. For highly sensitive dump comparisons, still verify the sample: matching upstream half-angle rounding with `b3wSin`/`b3wCos` can occasionally be more stable than replacing existing code with `makeQuatFromAxisAngle`. When upstream C++ code calls `cosf`/`sinf` directly (rather than Box3D's Bhāskara I approximation used by `b3wSin`/`b3wCos`), use `b3wCosf`/`b3wSinf` with `Math.fround` on intermediate values to match float32 arithmetic exactly.

JavaScript always evaluates `*`/`+` in float64, so `Math.fround(0.1 * i)` is **not** the same as C `0.1f * i`. Demo sample scenes that need C float32 evaluation order use helpers from `demo/src/samples/f32.ts`:

```ts
import { f32, f32Add, f32Mul } from "../f32";

// Match upstream: 0.5f + 1.1f * row
const y = f32Add(0.5, f32Mul(1.1, row));
```

`f32Mul`/`f32Add`/`f32Sub`/`f32Div` round operands to float32, then round the result — matching IEEE float32 ops for values whose exact product fits in a double mantissa.

## Headless Smoke Test

If you want to verify physics without a renderer, step a single body and log its transform.

```ts
import { BodyType, Box3DRuntime } from "box3d-wasm";

const runtime = await Box3DRuntime.load();
const world = runtime.createWorld({ gravity: [0, -10, 0] });
const body = world.createBody({ type: BodyType.Dynamic, position: [0, 4, 0] });

world.createHullShape(body, [0.5, 0.5, 0.5], { density: 1000 });

for (let i = 0; i < 120; i++) world.step(1 / 60, 4);

console.log(world.getBodyTransform(body));
console.log(world.getBodyLinearVelocity(body));
console.log(world.getBodyAngularVelocity(body));
world.destroy();
```

## Body Creation Styles

Use convenience helpers when the body has one simple shape.

```ts
const floor = world.createBox({
  static: true,
  size: [10, 0.5, 10],
  position: [0, -0.5, 0],
});

const ball = world.createSphere({
  radius: 0.5,
  position: [0, 3, 0],
  density: 1000,
  restitution: 0.4,
});

const pairedBall = world.createSphereWithShape({
  radius: 0.5,
  position: [2, 3, 0],
  density: 1000,
});

world.setShapeRestitution(pairedBall.shapeHandle, 0.4);
```

Use `createBody` plus explicit shape creation when you need more control or multiple shapes on one body.
Omitted shape material fields keep Box3D defaults, including friction `0.6`, restitution `0`, and rolling resistance `0`.

```ts
const body = world.createBody({
  type: BodyType.Dynamic,
  position: [0, 4, 0],
  angularVelocity: [0, 5, 0],
});

const shape = world.createHullShape(body, [0.4, 0.8, 0.2], {
  density: 1000,
  friction: 0.7,
  rollingResistance: 0.05,
});

const [primaryShape] = world.getBodyShapes(body);
world.setShapeFriction(primaryShape, 0.8);

const localPoint = world.getBodyLocalPoint(body, [0, 5, 0]);
const worldPoint = world.getBodyWorldPoint(body, [0, 0.5, 0]);
const center = world.getBodyWorldCenter(body);

const scratch: Vec3 = [0, 0, 0];
world.getBodyLocalPointTo(body, [0, 5, 0], scratch);
world.getBodyWorldPointVelocityXYZTo(body, 0, 0.5, 0, scratch);
```

Tuple-based helpers are the default API. For hot loops or repeated queries where you want to reduce short-lived allocations, use the `To(out)` or `XYZTo(out)` variants instead. Matching `XYZ(...)` variants are also available when scalar inputs are more convenient.

## Mesh Ground

Grid meshes are available for cases like terrain-style floors and upstream sample parity.

```ts
const world = runtime.createWorld({ gravity: [0, -10, 0] });

const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
const mesh = world.createGridMesh(20, 20, 1, 1, true);
world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });
```

Use `runtime.destroyMesh(mesh)` or `world.destroyMesh(mesh)` if you create standalone mesh handles outside normal world teardown.

## Body Types

`BodyType` mirrors Box3D's body categories while keeping application code readable.

```ts
const staticBody = world.createBody({ type: BodyType.Static });
const kinematicBody = world.createBody({ type: BodyType.Kinematic });
const dynamicBody = world.createBody({ type: BodyType.Dynamic });

world.setBodyTransform(staticBody, [0, 0, 0]);
world.setBodyLinearVelocity(kinematicBody, [1, 0, 0]);
world.applyLinearImpulseToCenter(dynamicBody, [0, 10, 0]);
```

## Render Loop Shape

The runtime does not own rendering. A typical Three.js integration stores body handles next to meshes, steps the world, then copies transforms into those meshes. This example assumes you already have a `scene`, `camera`, and `renderer`.

```ts
import * as THREE from "three";
import { BodyType, Box3DRuntime, type Vec3 } from "box3d-wasm";

const runtime = await Box3DRuntime.load();
const world = runtime.createWorld({ gravity: [0, -10, 0] });

const bodies: Array<{ handle: number; mesh: THREE.Mesh }> = [];

function addBox(size: Vec3, position: Vec3, dynamic = true): void {
  const handle = world.createBody({
    type: dynamic ? BodyType.Dynamic : BodyType.Static,
    position,
    isAwake: true,
  });

  world.createHullShape(handle, size, { density: dynamic ? 1000 : 0 });

  const mesh = new THREE.Mesh(
    new THREE.BoxGeometry(size[0] * 2, size[1] * 2, size[2] * 2),
    new THREE.MeshStandardMaterial({ color: dynamic ? 0x60a5fa : 0x222222 }),
  );

  mesh.position.set(position[0], position[1], position[2]);
  scene.add(mesh);
  bodies.push({ handle, mesh });
}

addBox([20, 0.5, 20], [0, -0.5, 0], false);
addBox([0.5, 0.5, 0.5], [0, 4, 0]);

function animate(): void {
  requestAnimationFrame(animate);
  world.step(1 / 60, 4);

  for (const body of bodies) {
    const transform = world.getBodyTransform(body.handle);
    body.mesh.position.set(...transform.position);
    body.mesh.quaternion.set(...transform.rotation);
  }

  renderer.render(scene, camera);
}

animate();
```

The demo uses the same idea, plus batched transform reads for heavy scenes. See `demo/src/samples/shared.ts`, `demo/src/samples/generic-host.ts`, and `demo/src/physics-worker-base.ts` for production-sized patterns.

## Queries

Use `rayCastClosest` for simple picking or visibility checks.

```ts
const hit = world.rayCastClosest([0, 10, 0], [0, -20, 0]);

if (hit !== null) {
  console.log(hit.bodyHandle, hit.point, hit.normal, hit.fraction);
}
```

## Simulation Tuning

Common world-level toggles are exposed on `PhysicsWorld`.

```ts
world.enableSleeping(true);
world.enableContinuous(true);
world.enableWarmStarting(true);
world.setContactTuning(60, 10, 1);
world.setWorkerCount(4);
```

## Cleanup

Destroy temporary resources that are not owned by the world.

```ts
const hull = runtime.createCylinder(1, 0.25, 0, 16);
const body = world.createBody({ type: BodyType.Dynamic, position: [0, 2, 0] });
const shape = world.createShapeFromHull(body, hull, { density: 1000 });
runtime.destroyHull(hull);

world.destroyShape(shape);
world.destroyBody(body);
world.destroy();
```

## Scale, Limits, And Custom Builds

The WASM bridge tracks every world/body/joint/hull/shape/mesh/compound/human you create through JavaScript handles. Those pools have **compile-time maximums** (`B3W_MAX_*` in `packages/box3d-wasm/cmake/CMakeLists.txt`). Defaults are sized for the ported upstream samples:

| Pool | Default max |
|------|-------------|
| bodies / joints / shapes | 65536 |
| hulls | 16384 |
| humans | 512 |
| meshes / compounds | 1024 |
| worlds | 16 |

When a pool is full, creation throws `SlotExhaustedError` instead of failing silently. Inspect usage at runtime:

```ts
const runtime = await Box3DRuntime.load();
console.log(runtime.limits);
console.log(runtime.getSlotUsage());
```

### Growable memory variant

The demo ships two release binaries:

- `wasm/box3d-web.wasm` — fixed 256MB heap (predictable for the demo)
- `wasm/growable/box3d-web.wasm` — 64MB initial heap with `-sALLOW_MEMORY_GROWTH=1`

Games that spawn large dynamic scenes should load the growable build:

```ts
const runtime = await Box3DRuntime.load({ variant: "growable" });
```

Slot limits are the same in both builds; growth only affects Box3D engine heap allocations.

### World capacity hints

Like upstream `b3Capacity`, you can pre-reserve engine memory when creating a world:

```ts
const world = runtime.createWorld({
  gravity: [0, -10, 0],
  capacity: {
    dynamicBodyCount: 12000,
    dynamicShapeCount: 12000,
    contactCount: 60000,
  },
});
```

Omit a field or pass `0` to leave that capacity at the Box3D default minimum reserve. This does **not** raise bridge slot limits; it only reduces realloc churn inside the engine.

### Rebuilding with higher slot limits

Configure limits at CMake time, then rebuild WASM:

```sh
cd packages/box3d-wasm
cmake -S cmake -B build-custom \
  -DBOX3D_SOURCE_DIR=../../box3d \
  -DB3W_MAX_HUMANS=2048 \
  -DBOX3D_WASM_ALLOW_MEMORY_GROWTH=ON \
  -DBOX3D_WASM_INITIAL_MEMORY=67108864 \
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=../../demo/public/wasm/custom
cmake --build build-custom
```

Load your custom output from your app’s static assets path, or replace `demo/public/wasm/` after `bun run build:wasm`.

For very large games, prefer fewer tracked entities (compounds/meshes instead of thousands of separate bodies), worker-side simulation with packed snapshots, and simpler proxies instead of full ragdolls for crowds.

## Current Limitations

The wrapper is sample-driven and does not yet mirror every upstream Box3D API. For example, several event buffers, query variants, mesh APIs, character mover APIs, and lower-level geometry utilities are still TODOs.

Use [`WASM_API_SURFACE.md`](./WASM_API_SURFACE.md) to check current coverage before porting a new sample or depending on an advanced Box3D feature.
