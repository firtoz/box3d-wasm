import { BodyType, type BodyHandle, type Box3DRuntime, type MeshHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView, getWasmBaseUrl } from "../shared";
import { getBuildingMeshData } from "../meshes/building-mesh";
import { parseObjText } from "../meshes/parse-obj";

const GROUND_HALF: Vec3 = [50, 0.1, 50];
const CAPSULE_CENTER1: Vec3 = [0, -0.5, 0];
const CAPSULE_CENTER2: Vec3 = [0, 0.5, 0];
const CAPSULE_RADIUS = 0.3;
const CAPSULE_POS: Vec3 = [0, 4, 10];
const BUILDING_POS: Vec3 = [0, 0.1, 0];

export function createBuildingMeshShape(world: PhysicsWorld, vertices: number[], indices: number[]): { body: BodyHandle; mesh: MeshHandle } {
  const mesh = world.createMesh(vertices, indices, { useMedianSplit: true, identifyEdges: true });
  const body = world.createBody({ position: BUILDING_POS });
  world.createMeshShape(body, mesh, { scale: [1, 1, 1] });
  return { body, mesh };
}

export function buildCapsuleMeshScene(world: PhysicsWorld, runtime: Box3DRuntime, vertices: number[], indices: number[]): {
  handles: BodyHandle[];
  meshes: MeshHandle[];
} {
  const meshes: MeshHandle[] = [];
  const ground = world.createBody({ position: [0, 0, 0] });
  runtime.createHullShape(ground, GROUND_HALF, {});

  const building = createBuildingMeshShape(world, vertices, indices);
  meshes.push(building.mesh);

  const capsule = world.createBody({
    type: BodyType.Dynamic,
    position: CAPSULE_POS,
    motionLocks: { angularX: true, angularY: true, angularZ: true },
    enableSleep: false,
    enableContactRecycling: false,
  });
  runtime.createCapsuleShape(capsule, CAPSULE_CENTER1, CAPSULE_CENTER2, CAPSULE_RADIUS, { friction: 0.3 });

  return { handles: [ground, building.body, capsule], meshes };
}

export function buildCapsuleMeshDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const { vertices, indices } = getBuildingMeshData();
  return buildCapsuleMeshScene(world, runtime, vertices, indices).handles;
}

export async function buildCapsuleMeshDynamicBodiesAsync(world: PhysicsWorld, runtime: Box3DRuntime): Promise<{
  handles: BodyHandle[];
  meshes: MeshHandle[];
}> {
  const response = await fetch(`${getWasmBaseUrl()}meshes/building.obj`);
  const text = await response.text();
  const { vertices, indices } = parseObjText(text);
  return buildCapsuleMeshScene(world, runtime, vertices, indices);
}

export function capsuleMeshGroundSize(): Vec3 {
  return GROUND_HALF;
}

export const capsuleMeshBodies: RenderBody[] = [
  {
    kind: "box",
    size: [100, 0.2, 100],
    position: [0, 0, 0],
    type: BodyType.Static,
    color: 0x64748b,
  },
  {
    kind: "box",
    size: [20, 20, 20],
    position: BUILDING_POS,
    type: BodyType.Static,
    color: 0x94a3b8,
  },
  {
    kind: "capsule",
    axis: "y",
    radius: CAPSULE_RADIUS,
    length: 1,
    position: CAPSULE_POS,
    color: 0xd946ef,
  },
];

export const capsuleMeshCamera: RenderSpec["camera"] = cameraFromSetView(20, 10, 30, [0, 2, 0]);

export const dumpSampleName = "Capsule Mesh";
export const dumpSampleId = "issues/capsule-mesh";
export const dumpCppSampleName = "Capsule Mesh";
export const dumpGroundSize = capsuleMeshGroundSize;

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ position: [0, 0, 0] });
  runtime.createHullShape(ground, GROUND_HALF, {});

  const capsule = world.createBody({
    type: BodyType.Dynamic,
    position: CAPSULE_POS,
    motionLocks: { angularX: true, angularY: true, angularZ: true },
    enableSleep: false,
    enableContactRecycling: false,
  });
  runtime.createCapsuleShape(capsule, CAPSULE_CENTER1, CAPSULE_CENTER2, CAPSULE_RADIUS, { friction: 0.3 });

  return { world, handles: [ground, capsule] };
}
