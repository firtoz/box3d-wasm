import type { BodyType, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildHighMassRatio1DynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const extent = 1;
  const half: Vec3 = [extent, extent, extent];

  for (let j = 0; j < 3; j++) {
    let count = 10;
    const offset = -20 * extent + 2 * (count + 1) * extent * j;
    let y = extent;
    while (count > 0) {
      for (let i = 0; i < count; i++) {
        const coeff = i - 0.5 * count;
        const yy = count === 1 ? y + 2 : y;
        const density = count === 1 ? (j + 1) * 100 : 1;
        const body = world.createBody({ type: 2 as BodyType, position: [2 * coeff * extent + offset, yy, 0] });
        runtime.createHullShape(body, half, { density });
        handles.push(body);
      }
      count--;
      y += 2 * extent;
    }
  }

  return handles;
}

export function highMassRatio1GroundSize(): Vec3 {
  return [50, 1, 50];
}

export const highMassRatio1Bodies: RenderBody[] = Array.from({ length: 165 }, () => ({
  kind: "box" as const, size: [2, 2, 2] as [number, number, number],
  position: [0, 0, 0] as [number, number, number], color: 0x3b82f6,
}));

export const highMassRatio1Camera: RenderSpec["camera"] = { position: [30, 15, 70], target: [0, 0, 0] };

export const dumpSampleName = "HighMassRatio1";
export const dumpSampleId = "robustness/high-mass-ratio-1";
export const dumpCppSampleName = "HighMassRatio1";
export const dumpGroundSize = highMassRatio1GroundSize;
export const dumpBuildDynamicBodies = buildHighMassRatio1DynamicBodies;
