import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const FAR_PYRAMID_OFFSET = 10_000_000;
const FAR_PYRAMID_BASE_COUNT = 40;
const FAR_PYRAMID_HALF = 0.5;

export function buildFarPyramidDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const offset = FAR_PYRAMID_OFFSET;
  const baseCount = FAR_PYRAMID_BASE_COUNT;
  const h = FAR_PYRAMID_HALF;
  const shift = h;
  const half: Vec3 = [h, h, h];

  for (let i = 0; i < baseCount; i++) {
    const y = (2 * i + 1) * shift;
    for (let j = i; j < baseCount; j++) {
      const x = (i + 1) * shift + 2 * (j - i) * shift - h * baseCount + offset;
      const body = world.createBody({ type: BodyType.Dynamic, position: [x, y, 0] });
      runtime.createHullShape(body, half, { density: 100 });
      handles.push(body);
    }
  }

  return handles;
}

export function farPyramidGroundSize(): Vec3 {
  return [400, 1, 400];
}

export const farPyramidGroundPosition: Vec3 = [FAR_PYRAMID_OFFSET, -1, 0];

export function buildFarPyramidGround(world: PhysicsWorld, runtime: Box3DRuntime): number {
  const ground = world.createBody({ type: BodyType.Static, position: farPyramidGroundPosition });
  runtime.createHullShape(ground, farPyramidGroundSize(), {});
  return ground;
}

export const farPyramidBodies: RenderBody[] = (() => {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < FAR_PYRAMID_BASE_COUNT; i++) {
    const y = (2 * i + 1) * FAR_PYRAMID_HALF;
    for (let j = i; j < FAR_PYRAMID_BASE_COUNT; j++) {
      const x = (i + 1) * FAR_PYRAMID_HALF + 2 * (j - i) * FAR_PYRAMID_HALF - FAR_PYRAMID_HALF * FAR_PYRAMID_BASE_COUNT + FAR_PYRAMID_OFFSET;
      bodies.push({
        kind: "box",
        size: [1, 1, 1],
        position: [x, y, 0],
        color: 0x60a5fa,
      });
    }
  }
  return bodies;
})();

export const farPyramidCamera: RenderSpec["camera"] = { position: [10_000_040, 10, 60], target: [10_000_000, 20, 0] };

export const dumpSampleName = "Far Pyramid";
export const dumpSampleId = "world/far-pyramid";
export const dumpCppSampleName = "Far Pyramid";
export const dumpGroundSize = farPyramidGroundSize;
export const dumpGroundPosition: Vec3 = farPyramidGroundPosition;
export const dumpBuildDynamicBodies = buildFarPyramidDynamicBodies;
