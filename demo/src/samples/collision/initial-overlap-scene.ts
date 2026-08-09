import {
  B3_AXIS_Z,
  B3_DEG_TO_RAD,
  type BodyHandle,
  type Box3DRuntime,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const MESH_SCALE: Vec3 = [4, 4, 4];
const MESH_INDICES = [0, 1, 2, 2, 3, 0];
const MESH_VERTICES: number[] = [
  -0.5, 0.5, 0.5,
  -0.5, 0.5, -0.5,
  -0.5, -0.5, -0.5,
  -0.5, -0.5, 0.5,
];

export interface InitialOverlapScene {
  body: BodyHandle;
  mesh: MeshHandle;
}

export function buildInitialOverlapScene(world: PhysicsWorld, runtime: Box3DRuntime): InitialOverlapScene {
  const rotation = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, 10 * B3_DEG_TO_RAD);
  const body = world.createBody({
    position: [0, 0, 0],
    rotation,
  });

  const mesh = world.createMesh(MESH_VERTICES, MESH_INDICES, { useMedianSplit: false, identifyEdges: false });
  world.createMeshShape(body, mesh, { scale: MESH_SCALE });

  return { body, mesh };
}

export function buildInitialOverlapDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  return [buildInitialOverlapScene(world, runtime).body];
}

export function initialOverlapGroundSize(): Vec3 {
  return [5, 0.5, 5];
}

export const initialOverlapBodies: RenderBody[] = [
  {
    kind: "box",
    size: [0.2, 4, 4],
    position: [-2, 0, 0],
    rotation: [0, 0, Math.sin(5 * B3_DEG_TO_RAD), Math.cos(5 * B3_DEG_TO_RAD)],
    color: 0x60a5fa,
  },
];

export const initialOverlapCamera: RenderSpec["camera"] = cameraFromSetView(-140, 10, 10, [0, 0, 0]);

export const dumpSampleName = "Initial Overlap";
export const dumpSampleId = "collision/initial-overlap";
export const dumpCppSampleName = "Initial Overlap";
export const dumpGroundSize = initialOverlapGroundSize;
export const dumpBuildDynamicBodies = buildInitialOverlapDynamicBodies;

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { body, mesh } = buildInitialOverlapScene(world, runtime);
  return {
    world,
    handles: [body],
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}
