# TypeScript API

This guide shows the public TypeScript wrapper around the Box3D WASM build. It is intentionally small and practical: create a runtime, create a world, add bodies and shapes, step the simulation, read transforms, and clean up.

For the exhaustive binding checklist, see [`WASM_API_SURFACE.md`](./WASM_API_SURFACE.md).

## Runtime Model

- `Box3DRuntime.load()` loads the WASM module and returns the runtime wrapper.
- `runtime.createWorld()` creates a `PhysicsWorld` wrapper for one Box3D world.
- Bodies, shapes, joints, hulls, compounds, and humans are represented by numeric handles.
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
const world = runtime.createWorld({ gravity: [0, -9.81, 0] });

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

## Headless Smoke Test

If you want to verify physics without a renderer, step a single body and log its transform.

```ts
import { BodyType, Box3DRuntime } from "box3d-wasm";

const runtime = await Box3DRuntime.load();
const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
const body = world.createBody({ type: BodyType.Dynamic, position: [0, 4, 0] });

world.createHullShape(body, [0.5, 0.5, 0.5], { density: 1000 });

for (let i = 0; i < 120; i++) world.step(1 / 60, 4);

console.log(world.getBodyTransform(body));
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
```

Use `createBody` plus explicit shape creation when you need more control or multiple shapes on one body.

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
```

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
const world = runtime.createWorld({ gravity: [0, -9.81, 0] });

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
world.createShapeFromHull(body, hull, { density: 1000 });
runtime.destroyHull(hull);

world.destroyBody(body);
world.destroy();
```

## Current Limitations

The wrapper is sample-driven and does not yet mirror every upstream Box3D API. For example, several event buffers, query variants, mesh APIs, character mover APIs, and lower-level geometry utilities are still TODOs.

Use [`WASM_API_SURFACE.md`](./WASM_API_SURFACE.md) to check current coverage before porting a new sample or depending on an advanced Box3D feature.
