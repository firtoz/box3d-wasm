import type { BodyId, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import { cameraFromSetView } from "../shared";

export const treeBenchmarkCamera = cameraFromSetView(45, 45, 250, [0, 0, 0]);

export function treeBenchmarkGroundSize(): Vec3 {
  return [10, 1, 10];
}

export function buildTreeBenchmarkDynamicBodies(_world: PhysicsWorld, _runtime: Box3DRuntime): BodyId[] {
  return [];
}

export const dumpNoPhysics = true;
export const dumpSampleName = "Benchmark";
export const dumpSampleId = "tree/benchmark";
export const dumpCppSampleName = "Tree/Benchmark";
export const dumpGroundSize = treeBenchmarkGroundSize;
export const dumpBuildDynamicBodies = buildTreeBenchmarkDynamicBodies;
