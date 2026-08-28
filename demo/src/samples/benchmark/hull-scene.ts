import { type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { cameraFromSetView } from "../shared";

export function buildBenchmarkHullDynamicBodies(): never[] {
  return [];
}

export function benchmarkHullGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const benchmarkHullCamera = cameraFromSetView(0, 15, 5, [0, 0, 0]);

export const dumpSampleName = "Hull";
export const dumpSampleId = "benchmark/hull";
export const dumpCppSampleName = "Benchmark/Hull";
export const dumpNoPhysics = true;
export const dumpGroundSize = benchmarkHullGroundSize;
export const dumpBuildDynamicBodies = buildBenchmarkHullDynamicBodies;

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: never[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: [] };
}
