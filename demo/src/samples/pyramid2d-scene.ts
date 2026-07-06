import type { Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody } from "./generic-host";
import { addBox } from "./shared-worker";

export function buildPyramid2dDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  for (let row = 0; row < 12; row++) {
    for (let col = 0; col < 12 - row; col++) {
      addBox(world, runtime, handles, [-10 + 2 * col + row, 1.5 + 2.5 * row, 0], [1, 1, 1], [0, 0, 0, 1], { lock2d: true });
    }
  }
  return handles;
}

export function pyramid2dGroundSize(): Vec3 {
  return [40, 1, 40];
}

export function createPyramid2dBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let row = 0; row < 12; row++) for (let col = 0; col < 12 - row; col++) bodies.push({ kind: "box", size: [2, 2, 2], position: [-10 + 2 * col + row, 1.5 + 2.5 * row, 0], color: 0x60a5fa + (row % 10) * 0x010101 });
  return bodies;
}

export const dumpSampleName = "Pyramid2D";
export const dumpSampleId = "pyramid2d";
export const dumpCppSampleName = "Pyramid2D";
export const dumpGroundSize = pyramid2dGroundSize;
export const dumpBuildDynamicBodies = buildPyramid2dDynamicBodies;
