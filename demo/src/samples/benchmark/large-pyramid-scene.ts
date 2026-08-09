import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32, f32Add, f32Mul, f32Sub } from "../f32";
import { cameraFromSetView } from "../shared";

/** Match upstream Release `CreateLargePyramid` (`BENCHMARK_DEBUG=0`): baseCount 100. */
const BASE_COUNT = 100;
const H = f32(0.5);
const SHIFT = f32Mul(1, H);

export const LARGE_PYRAMID_BOX_COUNT = (BASE_COUNT * (BASE_COUNT + 1)) / 2;
export const LARGE_PYRAMID_BOX_COLOR = 0x60a5fa;

export function forEachLargePyramidBox(callback: (position: Vec3) => void): void {
  for (let i = 0; i < BASE_COUNT; i++) {
    const iF = f32(i);
    const y = f32Mul(f32Add(f32Mul(2, iF), 1), SHIFT);
    for (let j = i; j < BASE_COUNT; j++) {
      const jF = f32(j);
      const x = f32Sub(
        f32Add(f32Mul(f32Add(iF, 1), SHIFT), f32Mul(f32Mul(2, f32Sub(jF, iF)), SHIFT)),
        f32Mul(H, BASE_COUNT),
      );
      callback([x, y, 0]);
    }
  }
}

export function buildLargePyramidDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const a = H;
  const hullHandle = runtime.createHullFromPoints([
    -a, -a, -a,  a, -a, -a,  a, a, -a,  -a, a, -a,
    -a, -a,  a,  a, -a,  a,  a, a,  a,  -a, a,  a,
  ]);

  forEachLargePyramidBox((position) => {
    const body = world.createBody({ type: BodyType.Dynamic, position });
    runtime.createShapeFromHull(body, hullHandle, { density: 100 });
    handles.push(body);
  });

  runtime.destroyHull(hullHandle);
  return handles;
}

export function largePyramidGroundSize(): Vec3 { return [400, 1, 400]; }

export function createLargePyramidBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  forEachLargePyramidBox((position) => {
    bodies.push({ kind: "box", size: [1, 1, 1], position, color: LARGE_PYRAMID_BOX_COLOR });
  });
  return bodies;
}

export const largePyramidCamera: RenderSpec["camera"] = cameraFromSetView(40, -10, 110, [0, 40, 0]);

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
