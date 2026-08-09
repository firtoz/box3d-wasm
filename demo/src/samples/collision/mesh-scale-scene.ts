import {
  type BodyHandle,
  type Box3DRuntime,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

export const MESH_SCALE_CENTER: Vec3 = [0, 0, 0];
export const MESH_SCALE_EXTENT: Vec3 = [0.5, 0.5, 0.5];
export const MESH_SCALE_SHAPE: Vec3 = [1, 1, 1];

export interface MeshScaleScene {
  body: BodyHandle;
  mesh: MeshHandle;
}

export function buildMeshScaleScene(world: PhysicsWorld, _runtime: Box3DRuntime): MeshScaleScene {
  const mesh = world.createBoxMesh(MESH_SCALE_CENTER, MESH_SCALE_EXTENT, true);
  const body = world.createBody({ position: [0, 0, 0] });
  world.createMeshShape(body, mesh, { scale: MESH_SCALE_SHAPE });
  return { body, mesh };
}

export function buildMeshScaleDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  return [buildMeshScaleScene(world, runtime).body];
}

export function meshScaleGroundSize(): Vec3 {
  return [5, 0.5, 5];
}

export const meshScaleBodies: RenderBody[] = [
  {
    kind: "box",
    size: [1, 1, 1],
    position: [0, 0, 0],
    color: 0x60a5fa,
  },
];

export const meshScaleCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 20, [0, 0, 0]);

export const dumpSampleName = "Mesh Scale";
export const dumpSampleId = "collision/mesh-scale";
export const dumpCppSampleName = "Mesh Scale";
export const dumpGroundSize = meshScaleGroundSize;
export const dumpBuildDynamicBodies = buildMeshScaleDynamicBodies;

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { body, mesh } = buildMeshScaleScene(world, runtime);
  return {
    world,
    handles: [body],
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}
