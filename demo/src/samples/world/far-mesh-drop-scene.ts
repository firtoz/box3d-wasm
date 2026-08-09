import { type Box3DRuntime, type MeshHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32Mul } from "../f32";
import {
  createMeshDrop,
  createMeshDropRenderBodies,
  MESH_DROP_WAVE,
} from "../mesh-drop-shared";

/** 1000 km * 1000 — matches FarMeshDrop `m_offsetKilometers`. */
const OFFSET = f32Mul(1000, 1000);
const ORIGIN: Vec3 = [OFFSET, 0, OFFSET];

export function buildFarMeshDrop(world: PhysicsWorld, runtime: Box3DRuntime) {
  return createMeshDrop(world, runtime, ORIGIN);
}

export function farMeshDropGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createFarMeshDropBodies(): RenderBody[] {
  return createMeshDropRenderBodies(ORIGIN);
}

export const farMeshDropCamera: RenderSpec["camera"] = cameraFromSetView(0, 30, 20, [OFFSET, 0, OFFSET]);

export const farMeshDropWaveParams = {
  ...MESH_DROP_WAVE,
  position: [OFFSET, 0, OFFSET] as [number, number, number],
};

export const dumpSampleName = "Far Mesh Drop";
export const dumpSampleId = "world/far-mesh-drop";
export const dumpCppSampleName = "Far Mesh Drop";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: { mesh: MeshHandle };
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, bodies, mesh } = createMeshDrop(world, runtime, ORIGIN);
  return {
    world,
    handles: [ground, ...bodies],
    state: { mesh },
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}
