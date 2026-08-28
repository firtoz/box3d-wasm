import type { BodyId, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import { cameraFromSetView } from "../shared";

export const meshCreationCamera = cameraFromSetView(45, 30, 40, [0, 0, 0]);

export function meshCreationGroundSize(): Vec3 {
  return [10, 1, 10];
}

export function buildMeshCreationDynamicBodies(_world: PhysicsWorld, _runtime: Box3DRuntime): BodyId[] {
  return [];
}

export const dumpNoPhysics = true;
export const dumpSampleName = "Creation Benchmark";
export const dumpSampleId = "mesh/creation-benchmark";
export const dumpCppSampleName = "Creation Benchmark";
export const dumpGroundSize = meshCreationGroundSize;
export const dumpBuildDynamicBodies = buildMeshCreationDynamicBodies;
