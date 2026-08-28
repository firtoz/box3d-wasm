import { B3_PI, BodyType, type BodyId, type Box3DRuntime, type HeightFieldHandle, type MeshHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView, getWasmBaseUrl } from "../shared";
import { parseObjText } from "../meshes/parse-obj";

const MOVER_START: Vec3 = [7.5, 0.75, 9];

export function buildMoverDumpBodies(world: PhysicsWorld, runtime: Box3DRuntime): {
  handles: BodyId[];
  meshes: MeshHandle[];
  heightField: HeightFieldHandle | null;
} {
  const meshes: MeshHandle[] = [];
  const handles: BodyId[] = [];

  const level = world.createBody();
  runtime.createTransformedHullShape(level, [1, 1, 1], { position: [4, 1, 14] });
  runtime.createTransformedHullShape(level, [1, 1, 1], { position: [4, 1, 13.95] });
  runtime.createTransformedHullShape(level, [1, 1, 1], {
    position: [5.8, 1, 13.7],
    rotation: runtime.makeQuatFromAxisAngle([0, 1, 0], 0.1 * B3_PI),
  });
  handles.push(level);

  const stairs = world.createBody({ position: [-10, 0, 0] });
  handles.push(stairs);

  const torus = world.createTorusMesh(10, 12, 2, 1);
  meshes.push(torus);
  const torusBody = world.createBody({
    position: [-10, 1, -8],
    rotation: runtime.makeQuatFromAxisAngle([0, 1, 0], 0.5 * B3_PI),
  });
  world.createMeshShape(torusBody, torus, { scale: [-0.75, 1.5, 0.5] });
  handles.push(torusBody);

  const waveBody = world.createBody({ position: [20, 0, 0] });
  const heightField = world.createWave(50, 50, [1, 1, 1], 0.02, 0.04, true);
  world.createHeightFieldShape(waveBody, heightField, {});
  handles.push(waveBody);

  const enemy = world.createBody({ position: [0, 1.4, 6] });
  runtime.createCapsuleShape(enemy, [0, -0.5, 0], [0, 0.5, 0], 0.3, { customColor: 0xc71585 });
  handles.push(enemy);

  const friend = world.createBody({ position: [0, 1.4, 5] });
  runtime.createCapsuleShape(friend, [0, -0.5, 0], [0, 0.5, 0], 0.3, {
    categoryBits: 2,
    maskBits: 0xffffffff,
    customColor: 0x32cd32,
  });
  handles.push(friend);

  const sphere = world.createBody({ type: BodyType.Dynamic, position: [7, 5, 0] });
  runtime.createSphereShape(sphere, [0, 0, 0], 0.5, {});
  handles.push(sphere);

  const ignore = world.createBody({ position: [7, 2, -3] });
  runtime.createHullShape(ignore, [0.5, 0.25, 0.5], { customColor: 0xfffaf0 });
  handles.push(ignore);

  const jointGround = world.createBody();
  const door = world.createBody({ type: BodyType.Dynamic, position: [-2, 1.6, 0], gravityScale: 2 });
  runtime.createHullShape(door, [0.75, 1.5, 0.1], { density: 1000 });
  const axis = runtime.computeQuatBetweenUnitVectors([0, 0, 1], [0, 1, 0]);
  const offset: Vec3 = [-0.75, 0, 0];
  world.createRevoluteJoint(jointGround, door, {
    localFrameA: { position: [-2.75, 1.6, 0], rotation: axis },
    localFrameB: { position: offset, rotation: axis },
    enableLimit: true,
    lowerAngle: (-90 * Math.PI) / 180,
    upperAngle: (90 * Math.PI) / 180,
    enableSpring: true,
    hertz: 1,
    dampingRatio: 0.5,
    enableMotor: false,
    maxMotorTorque: 100,
  });
  handles.push(jointGround, door);

  return { handles, meshes, heightField };
}

async function loadObjMesh(world: PhysicsWorld, url: string): Promise<MeshHandle | null> {
  const response = await fetch(url);
  if (!response.ok) return null;
  const { vertices, indices } = parseObjText(await response.text());
  const mesh = world.createMesh(vertices, indices, { useMedianSplit: true, identifyEdges: true, weldVertices: true });
  return mesh;
}

export async function buildMoverLive(world: PhysicsWorld, runtime: Box3DRuntime): Promise<{
  handles: BodyId[];
  meshes: MeshHandle[];
  heightField: HeightFieldHandle | null;
}> {
  const built = buildMoverDumpBodies(world, runtime);
  const levelMesh = await loadObjMesh(world, `${getWasmBaseUrl()}meshes/test_map01.obj`);
  if (levelMesh !== null) {
    built.meshes.push(levelMesh);
    world.createMeshShape(built.handles[0]!, levelMesh, { scale: [1, 1, 1] });
  }
  const stairsMesh = await loadObjMesh(world, `${getWasmBaseUrl()}meshes/stairs.obj`);
  if (stairsMesh !== null) {
    built.meshes.push(stairsMesh);
    world.createMeshShape(built.handles[1]!, stairsMesh, { scale: [0.75, 0.75, -1.5] });
  }
  return built;
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildMoverDumpBodies(world, runtime).handles };
}

export function moverGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const moverBodies: RenderBody[] = [
  {
    kind: "compound",
    position: [0, 0, 0],
    type: BodyType.Static,
    parts: [
      { kind: "box", size: [2, 2, 2], position: [4, 1, 14], color: 0x64748b },
      { kind: "box", size: [2, 2, 2], position: [4, 1, 13.95], color: 0x64748b },
      { kind: "box", size: [2, 2, 2], position: [5.8, 1, 13.7], color: 0x64748b },
    ],
  },
  { kind: "box", size: [0.1, 0.1, 0.1], position: [-10, 0, 0], type: BodyType.Static, color: 0x334155 },
  { kind: "torus", radius: 2, tube: 1, position: [-10, 1, -8], type: BodyType.Static, color: 0x94a3b8 },
  { kind: "box", size: [50, 0.05, 50], position: [20, 0, 0], type: BodyType.Static, color: 0x1e293b },
  { kind: "capsule", axis: "y", radius: 0.3, length: 1, position: [0, 1.4, 6], type: BodyType.Static, color: 0xc71585 },
  { kind: "capsule", axis: "y", radius: 0.3, length: 1, position: [0, 1.4, 5], type: BodyType.Static, color: 0x32cd32 },
  { kind: "sphere", radius: 0.5, position: [7, 5, 0], color: 0xf59e0b },
  { kind: "box", size: [1, 0.5, 1], position: [7, 2, -3], type: BodyType.Static, color: 0xfffaf0 },
  { kind: "box", size: [0.05, 0.05, 0.05], position: [0, 0, 0], type: BodyType.Static, color: 0x111827 },
  { kind: "box", size: [1.5, 3, 0.2], position: [-2, 1.6, 0], color: 0xcbd5e1 },
];

export const moverCamera: RenderSpec["camera"] = cameraFromSetView(120, 30, 5, MOVER_START);
export const moverStart = MOVER_START;

export const dumpSampleName = "Mover";
export const dumpSampleId = "character/mover";
export const dumpCppSampleName = "Mover";
export const dumpGroundSize = moverGroundSize;
