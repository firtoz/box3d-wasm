import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const BASE_COUNT = 10;
const EXTENT = 0.5;
const ROW_COUNT = 14;
const COLUMN_COUNT = 14;
const f32 = Math.fround;

function createSmallPyramid(world: PhysicsWorld, runtime: Box3DRuntime, centerX: number, baseZ: number, handles: number[]): void {
  const half: Vec3 = [EXTENT, EXTENT, EXTENT];
  for (let i = 0; i < BASE_COUNT; i++) {
    const y = f32(f32(2 * i + 1) * EXTENT);
    for (let j = i; j < BASE_COUNT; j++) {
      const x = f32(f32(f32(i + 1) * EXTENT) + f32(2 * f32(j - i) * EXTENT) + f32(centerX - 0.5));
      const z = f32(baseZ);
      const body = world.createBody({ type: BodyType.Dynamic, position: [x, y, z], enableSleep: false });
      runtime.createHullShape(body, half, { density: 100 });
      handles.push(body);
    }
  }
}

export function buildManyPyramidsDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const groundExtent = f32(EXTENT * COLUMN_COUNT * (BASE_COUNT + 1));
  const baseWidth = f32(2 * EXTENT * BASE_COUNT);
  let baseZ = f32(-groundExtent + 2 * EXTENT);
  const deltaZ = f32(2 * (groundExtent - 2 * EXTENT) / (ROW_COUNT - 1));

  for (let row = 0; row < ROW_COUNT; row++) {
    for (let col = 0; col < COLUMN_COUNT; col++) {
      const centerX = f32(-groundExtent + col * (baseWidth + 2 * EXTENT) + 2 * EXTENT);
      createSmallPyramid(world, runtime, centerX, baseZ, handles);
    }
    baseZ = f32(baseZ + deltaZ);
  }

  return handles;
}

export function manyPyramidsGroundSize(): Vec3 {
  const groundExtent = EXTENT * COLUMN_COUNT * (BASE_COUNT + 1);
  return [groundExtent, 1, groundExtent];
}

export function createManyPyramidsBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  const groundExtent = EXTENT * COLUMN_COUNT * (BASE_COUNT + 1);
  const baseWidth = 2 * EXTENT * BASE_COUNT;
  let baseZ = -groundExtent + 2 * EXTENT;
  const deltaZ = (2 * (groundExtent - 2 * EXTENT)) / (ROW_COUNT - 1);

  for (let row = 0; row < ROW_COUNT; row++) {
    for (let col = 0; col < COLUMN_COUNT; col++) {
      const centerX = -groundExtent + col * (baseWidth + 2 * EXTENT) + 2 * EXTENT;
      for (let i = 0; i < BASE_COUNT; i++) {
        const y = (2 * i + 1) * EXTENT;
        for (let j = i; j < BASE_COUNT; j++) {
          const x = (i + 1) * EXTENT + 2 * (j - i) * EXTENT + centerX - 0.5;
          bodies.push({ kind: "box", size: [1, 1, 1], position: [x, y, baseZ], color: 0x60a5fa });
        }
      }
    }
    baseZ += deltaZ;
  }
  return bodies;
}

export const manyPyramidsCamera: RenderSpec["camera"] = { position: [-10, 10, 120], target: [0, 5, 0] };

export const dumpSampleName = "Many Pyramids";
export const dumpSampleId = "benchmark/many-pyramids";
export const dumpCppSampleName = "Many Pyramids";
export const dumpGroundSize = manyPyramidsGroundSize;
export const dumpBuildDynamicBodies = buildManyPyramidsDynamicBodies;

export function createManyPyramids(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  world.enableSleeping(false);
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, manyPyramidsGroundSize(), {});
  return { world, handles: [ground, ...buildManyPyramidsDynamicBodies(world, runtime)] };
}

export const dumpCreate = createManyPyramids;
