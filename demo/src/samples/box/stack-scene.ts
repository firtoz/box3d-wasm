import type { Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody } from "../generic-host";
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

export function createBoxStackBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < 40; i++) bodies.push({ kind: "box", size: [1, 1, 1], position: [0, 0.75 + 1.25 * i, 0], color: 0x60a5fa + (i % 10) * 0x010101 });
  return bodies;
}

export const dumpSampleName = "Box Stack";
export const dumpSampleId = "box-stack";
export const dumpCppSampleName = "Box Stack";
export const dumpGroundSize = boxStackGroundSize;
export const dumpBuildDynamicBodies = buildBoxStackDynamicBodies;
