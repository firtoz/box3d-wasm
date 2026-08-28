import { B3_PI, BodyType, type BodyId, type Box3DRuntime, type HeightFieldHandle, type MeshHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView, getWasmBaseUrl } from "../shared";
import { parseObjText } from "../meshes/parse-obj";
import { f32Add, f32Mul } from "../f32";

const SRC = 0.0254;
const START: Vec3 = [7.5, 2, 9];
const BODY_RADIUS = f32Mul(16, SRC);
const TOTAL_HEIGHT = f32Mul(72, SRC);
const FEET_HEIGHT = f32Mul(TOTAL_HEIGHT, 0.5);
const MASS = 500;
const GRAVITY_SCALE = 15 / 10;

function createCharacter(world: PhysicsWorld, runtime: Box3DRuntime): BodyId {
  const body = world.createBody({
    type: BodyType.Dynamic,
    position: START,
    enableSleep: false,
    enableContactRecycling: false,
    gravityScale: GRAVITY_SCALE,
  });
  runtime.setBodyMotionLocks(body, { lockRotationX: true, lockRotationY: true, lockRotationZ: true });
  const halfExtX = f32Mul(BODY_RADIUS, 0.5);
  const halfExtY = f32Mul(FEET_HEIGHT, 0.5);
  const feetY = f32Add(-f32Mul(TOTAL_HEIGHT, 0.5), halfExtY);
  const feetVolume = f32Mul(f32Mul(f32Mul(8, halfExtX), halfExtY), halfExtX);
  runtime.createOffsetHullShape(body, [halfExtX, halfExtY, halfExtX], [0, feetY, 0], {
    friction: 0,
    restitution: 0,
    density: (MASS * 0.4) / feetVolume,
    customColor: 0x32cd32,
  });
  const capsuleRadius = f32Mul(BODY_RADIUS, 0.707);
  const capsuleBottom = f32Add(f32Add(-f32Mul(TOTAL_HEIGHT, 0.5), f32Mul(FEET_HEIGHT, 0.5)), capsuleRadius);
  const capsuleTop = f32SubH(f32Mul(TOTAL_HEIGHT, 0.5), capsuleRadius);
  if (capsuleTop > capsuleBottom) {
    const h = capsuleTop - capsuleBottom;
    const r = capsuleRadius;
    const capsuleVolume = Math.PI * r * r * (h + (4 * r) / 3);
    runtime.createCapsuleShape(body, [0, capsuleBottom, 0], [0, capsuleTop, 0], capsuleRadius, {
      friction: 0,
      restitution: 0,
      density: (MASS * 0.6) / capsuleVolume,
      customColor: 0x6495ed,
    });
  }
  return body;
}

function f32SubH(a: number, b: number): number {
  return Math.fround(Math.fround(a) - Math.fround(b));
}

export function buildRigidBodyDump(world: PhysicsWorld, runtime: Box3DRuntime): {
  handles: BodyId[];
  meshes: MeshHandle[];
  heightField: HeightFieldHandle | null;
} {
  const handles: BodyId[] = [];
  const meshes: MeshHandle[] = [];
  handles.push(createCharacter(world, runtime));
  handles.push(world.createBody());
  handles.push(world.createBody({ position: [-10, 0, 0] }));
  handles.push(world.createBody({ position: [-5, 0, -10] }));
  handles.push(world.createBody({ position: [10, 0, -10] }));
  handles.push(world.createBody({ position: [10, 0, 10] }));

  const wave = world.createBody({ position: [20, 0, 0] });
  const heightField = world.createWave(50, 50, [1, 1, 1], 0.02, 0.04, true);
  world.createHeightFieldShape(wave, heightField, {});
  handles.push(wave);

  const hullFriction = { friction: 0.6 };
  const rampQ = runtime.makeQuatFromAxisAngle([0, 0, 1], f32Mul(-20, B3_PI / 180));
  const ramp = world.createBody({ position: [6, 1, 4], rotation: rampQ });
  runtime.createHullShape(ramp, [3, 0.15, 1.5], { ...hullFriction, customColor: 0x6b8e23 });
  handles.push(ramp);
  const steepQ = runtime.makeQuatFromAxisAngle([0, 0, 1], f32Mul(-50, B3_PI / 180));
  const steep = world.createBody({ position: [6, 2, -4], rotation: steepQ });
  runtime.createHullShape(steep, [2.5, 0.15, 1.5], { ...hullFriction, customColor: 0xcd5c5c });
  handles.push(steep);
  for (let i = 0; i < 3; i++) {
    const platform = world.createBody({ position: [f32Add(-4, f32Mul(3.5, i)), 1.2, -5] });
    runtime.createHullShape(platform, [1.2, 0.15, 1.2], { ...hullFriction, customColor: 0x708090 });
    handles.push(platform);
  }
  for (let i = 0; i < 5; i++) {
    const lip = f32Add(0.05, f32Mul(0.08, i));
    const step = world.createBody({ position: [-8, lip, f32Add(-1, f32Mul(2, i))] });
    runtime.createHullShape(step, [1, lip, 0.6], { ...hullFriction, customColor: 0x6495ed });
    handles.push(step);
  }
  const wall = world.createBody({ position: [0, 1.5, 10] });
  runtime.createHullShape(wall, [4, 1.5, 0.2], { ...hullFriction, customColor: 0x2f4f4f });
  handles.push(wall);
  for (let i = 0; i < 3; i++) {
    const box = world.createBody({ type: BodyType.Dynamic, position: [f32Add(3, f32Mul(1.5, i)), 0.5, 0] });
    runtime.createHullShape(box, [0.4, 0.4, 0.4], { ...hullFriction, customColor: 0xffd700 });
    handles.push(box);
  }
  const sphere = world.createBody({ type: BodyType.Dynamic, position: [-3, 1, 0] });
  runtime.createSphereShape(sphere, [0, 0, 0], 0.5, { ...hullFriction, customColor: 0xffa500 });
  handles.push(sphere);
  return { handles, meshes, heightField };
}

async function attachObj(world: PhysicsWorld, body: BodyId, url: string, scale: Vec3, meshes: MeshHandle[]): Promise<void> {
  const response = await fetch(url);
  if (!response.ok) return;
  const { vertices, indices } = parseObjText(await response.text());
  const mesh = world.createMesh(vertices, indices, { useMedianSplit: true, identifyEdges: true, weldVertices: true });
  meshes.push(mesh);
  world.createMeshShape(body, mesh, { scale });
}

export async function buildRigidBodyLive(world: PhysicsWorld, runtime: Box3DRuntime): Promise<{
  handles: BodyId[];
  meshes: MeshHandle[];
  heightField: HeightFieldHandle | null;
}> {
  const built = buildRigidBodyDump(world, runtime);
  const base = `${getWasmBaseUrl()}meshes/`;
  await attachObj(world, built.handles[1]!, `${base}test_map01.obj`, [1, 1, 1], built.meshes);
  await attachObj(world, built.handles[2]!, `${base}stairs.obj`, [0.75, 0.75, -1.5], built.meshes);
  await attachObj(world, built.handles[3]!, `${base}building.obj`, [1, 1, 1], built.meshes);
  await attachObj(world, built.handles[4]!, `${base}voxel_mesh_01.obj`, [1, 1, 1], built.meshes);
  await attachObj(world, built.handles[5]!, `${base}voxel_mesh_02.obj`, [1, 1, 1], built.meshes);
  return built;
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildRigidBodyDump(world, runtime).handles };
}

export function rigidBodyGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createRigidBodyBodies(): RenderBody[] {
  const bodies: RenderBody[] = [
    {
      kind: "compound",
      position: START as [number, number, number],
      parts: [
        { kind: "box", size: [BODY_RADIUS, FEET_HEIGHT, BODY_RADIUS], position: [0, -TOTAL_HEIGHT * 0.25, 0], color: 0x32cd32 },
        { kind: "capsule", axis: "y", radius: BODY_RADIUS * 0.707, length: TOTAL_HEIGHT * 0.5, position: [0, FEET_HEIGHT * 0.25, 0], color: 0x6495ed },
      ],
    },
    { kind: "box", size: [0.1, 0.1, 0.1], position: [0, 0, 0], type: BodyType.Static, color: 0x334155 },
    { kind: "box", size: [0.1, 0.1, 0.1], position: [-10, 0, 0], type: BodyType.Static, color: 0x334155 },
    { kind: "box", size: [0.1, 0.1, 0.1], position: [-5, 0, -10], type: BodyType.Static, color: 0x334155 },
    { kind: "box", size: [0.1, 0.1, 0.1], position: [10, 0, -10], type: BodyType.Static, color: 0x334155 },
    { kind: "box", size: [0.1, 0.1, 0.1], position: [10, 0, 10], type: BodyType.Static, color: 0x334155 },
    { kind: "box", size: [50, 0.05, 50], position: [20, 0, 0], type: BodyType.Static, color: 0x1e293b },
    { kind: "box", size: [6, 0.3, 3], position: [6, 1, 4], type: BodyType.Static, color: 0x6b8e23 },
    { kind: "box", size: [5, 0.3, 3], position: [6, 2, -4], type: BodyType.Static, color: 0xcd5c5c },
  ];
  for (let i = 0; i < 3; i++) {
    bodies.push({ kind: "box", size: [2.4, 0.3, 2.4], position: [-4 + 3.5 * i, 1.2, -5], type: BodyType.Static, color: 0x708090 });
  }
  for (let i = 0; i < 5; i++) {
    const lip = 0.05 + 0.08 * i;
    bodies.push({ kind: "box", size: [2, 2 * lip, 1.2], position: [-8, lip, -1 + 2 * i], type: BodyType.Static, color: 0x6495ed });
  }
  bodies.push({ kind: "box", size: [8, 3, 0.4], position: [0, 1.5, 10], type: BodyType.Static, color: 0x2f4f4f });
  for (let i = 0; i < 3; i++) {
    bodies.push({ kind: "box", size: [0.8, 0.8, 0.8], position: [3 + 1.5 * i, 0.5, 0], color: 0xffd700 });
  }
  bodies.push({ kind: "sphere", radius: 0.5, position: [-3, 1, 0], color: 0xffa500 });
  return bodies;
}

export const rigidBodyCamera: RenderSpec["camera"] = cameraFromSetView(120, 30, 5, START);
export const rigidWalkSpeed = f32Mul(230, SRC);

export const dumpSampleName = "Rigid Body";
export const dumpSampleId = "character/rigid-body";
export const dumpCppSampleName = "Rigid Body";
export const dumpGroundSize = rigidBodyGroundSize;
