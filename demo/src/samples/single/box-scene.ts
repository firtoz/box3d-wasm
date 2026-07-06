import type { BodyType, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";

export function buildSingleBoxDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const body = world.createBody({ type: 2 as BodyType, position: [0, 0.5, 0], rotation: [0, 0, 0, 1], isAwake: true, angularVelocity: [0, 10, 0] });
  runtime.createHullShape(body, [0.5, 0.5, 0.5]);
  return [body];
}

export function singleBoxGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const dumpSampleName = "Single Box";
export const dumpSampleId = "single-box";
export const dumpCppSampleName = "Single Box";
export const dumpGroundSize = singleBoxGroundSize;
export const dumpBuildDynamicBodies = buildSingleBoxDynamicBodies;
