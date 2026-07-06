import type { Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import { addBox } from "../shared-worker";

export function buildBoxStackDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  for (let i = 0; i < 40; i++) {
    addBox(world, runtime, handles, [0, 0.75 + 1.25 * i, 0], [0.5, 0.5, 0.5], [0, 0, 0, 1], { rollingResistance: 0.1 });
  }
  return handles;
}

export function boxStackGroundSize(): Vec3 {
  return [40, 1, 40];
}

export const dumpSampleName = "Box Stack";
export const dumpSampleId = "box-stack";
export const dumpCppSampleName = "Box Stack";
export const dumpGroundSize = boxStackGroundSize;
export const dumpBuildDynamicBodies = buildBoxStackDynamicBodies;
