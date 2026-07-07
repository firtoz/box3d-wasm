import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const BASE_COUNT = 90;
const h = 0.5;
const shift = h;

export function buildLargePyramidDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const half: Vec3 = [h, h, h];

  for (let i = 0; i < BASE_COUNT; i++) {
    const y = (2 * i + 1) * shift;
    for (let j = i; j < BASE_COUNT; j++) {
      const x = (i + 1) * shift + 2 * (j - i) * shift - h * BASE_COUNT;
      const body = world.createBody({ type: BodyType.Dynamic, position: [x, y, 0] });
      runtime.createHullShape(body, half, { density: 100 });
      handles.push(body);
    }
  }

  return handles;
}

export function largePyramidGroundSize(): Vec3 { return [400, 1, 400]; }

export function createLargePyramidBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < BASE_COUNT; i++) {
    const y = (2 * i + 1) * shift;
    for (let j = i; j < BASE_COUNT; j++) {
      const x = (i + 1) * shift + 2 * (j - i) * shift - h * BASE_COUNT;
      bodies.push({ kind: "box", size: [1, 1, 1], position: [x, y, 0], color: 0x60a5fa });
    }
  }
  return bodies;
}

export const largePyramidCamera: RenderSpec["camera"] = { position: [40, -10, 110], target: [0, 40, 0] };

export const dumpSampleName = "Large Pyramid";
export const dumpSampleId = "benchmark/large-pyramid";
export const dumpCppSampleName = "Large Pyramid";
export const dumpGroundSize = largePyramidGroundSize;
export const dumpBuildDynamicBodies = buildLargePyramidDynamicBodies;
export const dumpNoPhysics = false;

export function createLargePyramid(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  world.enableSleeping(false);
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, largePyramidGroundSize(), {});
  return { world, handles: [ground, ...buildLargePyramidDynamicBodies(world, runtime)] };
}

export const dumpCreate = createLargePyramid;
