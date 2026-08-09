import {type Box3DRuntime, type MeshHandle, type PhysicsWorld, type Vec3, type BodyId} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import {
  createMeshDrop,
  createMeshDropRenderBodies,
  MESH_DROP_WAVE,
} from "../mesh-drop-shared";

const ORIGIN: Vec3 = [0, 0, 0];

export function buildMeshDropDeterminism(world: PhysicsWorld, runtime: Box3DRuntime) {
  return createMeshDrop(world, runtime, ORIGIN);
}

export function meshDropDeterminismGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createMeshDropDeterminismBodies(): RenderBody[] {
  return createMeshDropRenderBodies(ORIGIN);
}

export const meshDropDeterminismCamera: RenderSpec["camera"] = cameraFromSetView(0, 30, 20, [0, 0, 0]);

export const meshDropDeterminismWaveParams = { ...MESH_DROP_WAVE, position: ORIGIN as [number, number, number] };

export const dumpSampleName = "Determinism Mesh Drop";
export const dumpSampleId = "determinism/mesh-drop";
export const dumpCppSampleName = "Determinism Mesh Drop";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
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
