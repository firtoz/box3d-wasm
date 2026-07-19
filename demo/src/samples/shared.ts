import * as THREE from "three";
import type { DemoBody } from "./types";
import { BodyType, type BodyBatchBuffers, type BodyHandle, type PhysicsWorld, type RuntimeLoadOptions, type Vec3 } from "box3d-wasm";

const MAX_WEB_WORKERS = 16;
export type DemoWasmVariant = NonNullable<RuntimeLoadOptions["variant"]>;

export function getWorkerCounts(): { defaultWorkerCount: number, maxWorkerCount: number, poolSize: number } {
  const available = Math.min(MAX_WEB_WORKERS, Math.max(1, navigator.hardwareConcurrency || 4));
  const poolParam = typeof globalThis.location !== "undefined" ? new URL(globalThis.location.href).searchParams.get("pool") : null;
  const poolFromUrl = poolParam !== null ? Math.max(1, parseInt(poolParam, 10) || 1) : 0;
  return {
    defaultWorkerCount: Math.max(1, Math.floor(available * 0.75)),
    maxWorkerCount: available,
    poolSize: poolFromUrl || available,
  };
}

export function getWasmVariantOptions(): readonly DemoWasmVariant[] {
  return __BOX3D_DEMO_WASM_VARIANTS__;
}

export function cameraFromSetView(
  yawDegrees: number,
  pitchDegrees: number,
  radius: number,
  pivot: [number, number, number],
): { position: [number, number, number]; target: [number, number, number] } {
  const yaw = yawDegrees * (Math.PI / 180);
  const pitch = pitchDegrees * (Math.PI / 180);
  const cp = Math.cos(pitch);
  const forwardX = Math.sin(yaw) * cp;
  const forwardY = Math.sin(pitch);
  const forwardZ = Math.cos(yaw) * cp;
  return {
    position: [
      pivot[0] + radius * forwardX,
      pivot[1] + radius * forwardY,
      pivot[2] + radius * forwardZ,
    ],
    target: pivot,
  };
}

export function getWasmVariant(): DemoWasmVariant {
  const options = getWasmVariantOptions();
  const fallback = options.includes(__BOX3D_DEMO_WASM_VARIANT__)
    ? __BOX3D_DEMO_WASM_VARIANT__
    : (options[0] ?? "release");
  const fromUrl = new URL(globalThis.location.href).searchParams.get("wasm");
  if (fromUrl === "profile" || fromUrl === "growable" || fromUrl === "release") {
    if (options.includes(fromUrl)) return fromUrl;
    console.warn(
      `[box3d-wasm] wasm variant "${fromUrl}" is not built (${options.join(", ")}). Using ${fallback}.`,
    );
  }
  return fallback;
}

/** Vite asset base (`/` locally, `/box3d-wasm/` on GitHub Pages). Workers need this to find `wasm/`. */
export function getWasmBaseUrl(): string {
  return import.meta.env.BASE_URL;
}

function yToX(): THREE.Quaternion {
  return new THREE.Quaternion().setFromUnitVectors(new THREE.Vector3(0, 1, 0), new THREE.Vector3(1, 0, 0));
}

function yToZ(): THREE.Quaternion {
  return new THREE.Quaternion().setFromUnitVectors(new THREE.Vector3(0, 1, 0), new THREE.Vector3(0, 0, 1));
}

export function addBox(
  world: PhysicsWorld,
  scene: THREE.Scene,
  bodies: DemoBody[],
  size: Vec3,
  position: Vec3,
  color: number,
  isStatic = false,
): DemoBody {
  const created = world.createBoxWithShape({ size, position, static: isStatic, density: isStatic ? 0 : 1000 });
  const handle = created.bodyHandle;
  const mesh = new THREE.Mesh(
    new THREE.BoxGeometry(size[0] * 2, size[1] * 2, size[2] * 2),
    new THREE.MeshStandardMaterial({ color, roughness: 0.75 }),
  );
  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = !isStatic;
  mesh.receiveShadow = true;
  scene.add(mesh);
  const body = { handle, mesh, shapeIds: [created.shapeHandle], type: isStatic ? BodyType.Static : BodyType.Dynamic };
  bodies.push(body);
  return body;
}

export function addSphere(
  world: PhysicsWorld,
  scene: THREE.Scene,
  bodies: DemoBody[],
  radius: number,
  position: Vec3,
  color: number,
): DemoBody {
  const created = world.createSphereWithShape({ radius, position, density: 1000 });
  const handle = created.bodyHandle;
  const mesh = new THREE.Mesh(
    new THREE.SphereGeometry(radius, 24, 16),
    new THREE.MeshStandardMaterial({ color, roughness: 0.6 }),
  );
  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = true;
  mesh.receiveShadow = true;
  scene.add(mesh);
  const body = { handle, mesh, shapeIds: [created.shapeHandle], type: BodyType.Dynamic };
  bodies.push(body);
  return body;
}

export function addHull(
  world: PhysicsWorld,
  scene: THREE.Scene,
  bodies: DemoBody[],
  size: Vec3,
  position: Vec3,
  color: number,
  friction = 0.5,
  rollingResistance = 0,
  createHullShape?: (world: PhysicsWorld, size: Vec3, position: Vec3, friction: number, rollingResistance: number) => BodyHandle,
): DemoBody {
  const created = createHullShape === undefined ? world.createBoxWithShape({ size, position, static: false, density: 1000 }) : null;
  const handle = created === null ? createHullShape!(world, size, position, friction, rollingResistance) : created.bodyHandle;
  const mesh = new THREE.Mesh(
    new THREE.BoxGeometry(size[0] * 2, size[1] * 2, size[2] * 2),
    new THREE.MeshStandardMaterial({ color, roughness: 0.75 }),
  );
  mesh.position.set(position[0], position[1], position[2]);
  mesh.castShadow = true;
  mesh.receiveShadow = true;
  scene.add(mesh);
  const body = { handle, mesh, shapeIds: created === null ? world.getBodyShapes(handle) : [created.shapeHandle], type: BodyType.Dynamic };
  bodies.push(body);
  return body;
}

export function capsuleMesh(radius: number, length: number, color: number, roughness = 0.75, axis: "x" | "y" | "z" = "x"): THREE.Mesh {
  const geom = new (THREE as any).CapsuleGeometry(radius, length, 6, 12) as THREE.BufferGeometry;
  // Three.js capsules are Y-axis by default. Match the requested render axis
  // explicitly so scene specs can declare capsule orientation directly.
  if (axis === "x") geom.applyQuaternion(yToX());
  else if (axis === "z") geom.applyQuaternion(yToZ());
  const mat = new THREE.MeshStandardMaterial({ color, roughness });
  const mesh = new THREE.Mesh(geom, mat);
  mesh.castShadow = true;
  mesh.receiveShadow = true;
  return mesh;
}

export type BodySyncBatch = {
  buffers: BodyBatchBuffers;
  positions: Float32Array;
  rotations: Float32Array;
  awake: Uint8Array;
  colors: Uint32Array;
  awakeCache: Uint8Array;
  colorCache: Uint32Array;
  count: number;
  firstHandle: number;
  lastHandle: number;
};

const syncBatchCache = new WeakMap<DemoBody[], BodySyncBatch>();

export function createBodySyncBatch(world: PhysicsWorld, bodies: DemoBody[]): BodySyncBatch {
  const count = bodies.length;
  const buffers = world.allocBodyBatchBuffers(count);
  const memory = world.getMemoryView();
  const handles = new Int32Array(memory.heap32.buffer, buffers.bodyHandlesPtr, count);
  for (let i = 0; i < count; i++) handles[i] = bodies[i].handle;
  return {
    buffers,
    positions: new Float32Array(memory.heapF32.buffer, buffers.positionsPtr, count * 3),
    rotations: new Float32Array(memory.heapF32.buffer, buffers.rotationsPtr, count * 4),
    awake: new Uint8Array(memory.heapU8.buffer, buffers.awakePtr, count),
    colors: new Uint32Array(memory.heap32.buffer, buffers.colorsPtr, count),
    awakeCache: new Uint8Array(count),
    colorCache: new Uint32Array(count),
    count,
    firstHandle: count > 0 ? bodies[0].handle : 0,
    lastHandle: count > 0 ? bodies[count - 1].handle : 0,
  };
}

function getCachedBodySyncBatch(world: PhysicsWorld, bodies: DemoBody[]): BodySyncBatch {
  const cached = syncBatchCache.get(bodies);
  const count = bodies.length;
  if (cached !== undefined && cached.count === count && cached.firstHandle === (count > 0 ? bodies[0].handle : 0) && cached.lastHandle === (count > 0 ? bodies[count - 1].handle : 0)) {
    return cached;
  }
  if (cached !== undefined) {
    world.freeBodyBatchBuffers(cached.buffers);
  }
  const next = createBodySyncBatch(world, bodies);
  syncBatchCache.set(bodies, next);
  return next;
}

export function syncBodiesBatch(world: PhysicsWorld, bodies: DemoBody[], batch: BodySyncBatch): void {
  const count = bodies.length;
  // Keep WASM handle scratch in sync with `bodies` (each list has its own batch buffers).
  const memory = world.getMemoryView();
  const handles = new Int32Array(memory.heap32.buffer, batch.buffers.bodyHandlesPtr, count);
  for (let i = 0; i < count; i++) handles[i] = bodies[i]!.handle;

  world.writeBodyTransforms(count, batch.buffers.bodyHandlesPtr, batch.buffers.positionsPtr, batch.buffers.rotationsPtr, batch.buffers.awakePtr, batch.buffers.colorsPtr);
  const positions = batch.positions;
  const rotations = batch.rotations;
  const awake = batch.awake;
  const awakeCache = batch.awakeCache;
  const colors = batch.colors;
  const colorCache = batch.colorCache;
  for (let i = 0; i < count; i++) {
    const body = bodies[i];
    body.mesh.position.set(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
    body.mesh.quaternion.set(rotations[i * 4], rotations[i * 4 + 1], rotations[i * 4 + 2], rotations[i * 4 + 3]);
    if (!body.preserveColor) {
      const nextAwake = awake[i] !== 0;
      const prevAwake = awakeCache[i] !== 0;
      const colorHex = colors[i] & 0xffffff;
      const prevColorHex = colorCache[i] & 0xffffff;
      awakeCache[i] = nextAwake ? 1 : 0;
      colorCache[i] = colorHex;
      if (prevAwake !== nextAwake || prevColorHex !== colorHex || body.type === BodyType.Static) {
        const mat = body.mesh.material as THREE.MeshStandardMaterial;
        mat.color.setHex(colorHex);
      }
    }
  }
}

export function syncBodies(world: PhysicsWorld, bodies: DemoBody[]): void {
  if (bodies.length >= 12) {
    syncBodiesBatch(world, bodies, getCachedBodySyncBatch(world, bodies));
    return;
  }
  for (const body of bodies) {
    const transform = world.getBodyTransform(body.handle);
    body.mesh.position.set(transform.position[0], transform.position[1], transform.position[2]);
    body.mesh.quaternion.set(
      transform.rotation[0],
      transform.rotation[1],
      transform.rotation[2],
      transform.rotation[3],
    );
    if (!body.preserveColor) {
      const colorHex = world.getBodyDebugColor(body.handle) & 0xffffff;
      const mat = body.mesh.material as THREE.MeshStandardMaterial;
      mat.color.setHex(colorHex);
      if (body.extraMeshes !== undefined) {
        for (const em of body.extraMeshes) {
          const emMat = em.material as THREE.MeshStandardMaterial;
          emMat.color.setHex(colorHex);
        }
      }
    }
  }
}

export function makeBatchBuffers(world: PhysicsWorld, bodies: DemoBody[]): BodyBatchBuffers {
  return world.allocBodyBatchBuffers(bodies.length);
}

export function disposeBodies(scene: THREE.Scene, bodies: DemoBody[]): void {
  for (const { mesh, extraMeshes } of bodies) {
    scene.remove(mesh);
    mesh.geometry.dispose();
    const material = mesh.material;
    if (Array.isArray(material)) {
      material.forEach((entry) => entry.dispose());
    } else {
      material.dispose();
    }
    if (extraMeshes !== undefined) {
      for (const em of extraMeshes) {
        scene.remove(em);
        em.geometry.dispose();
        const emMat = em.material;
        if (Array.isArray(emMat)) {
          emMat.forEach((entry) => entry.dispose());
        } else {
          emMat.dispose();
        }
      }
    }
  }
}
