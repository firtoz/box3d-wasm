import type { BodyType, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildFarPyramidDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const offset = 10000000;
  const baseCount = 40;
  const h = 0.5;
  const shift = h;
  const half: Vec3 = [h, h, h];

  for (let i = 0; i < baseCount; i++) {
    const y = (2 * i + 1) * shift;
    for (let j = i; j < baseCount; j++) {
      const x = (i + 1) * shift + 2 * (j - i) * shift - h * baseCount + offset;
      const body = world.createBody({ type: 2 as BodyType, position: [x, y, 0] });
      runtime.createHullShape(body, half, { density: 100 });
      handles.push(body);
    }
  }

  return handles;
}

export function farPyramidGroundSize(): Vec3 {
  return [400, 1, 400];
}

export const farPyramidBodies: RenderBody[] = [];

export const farPyramidCamera: RenderSpec["camera"] = { position: [10040, -10, 60], target: [10000, 20, 0] };

export const dumpSampleName = "Far Pyramid";
export const dumpSampleId = "world/far-pyramid";
export const dumpCppSampleName = "Far Pyramid";
export const dumpGroundSize = farPyramidGroundSize;
export const dumpGroundPosition: Vec3 = [10000000, -1, 0];
export const dumpBuildDynamicBodies = buildFarPyramidDynamicBodies;
