import type { BodyType, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

function mulberry32(a: number): () => number {
  return () => {
    a |= 0; a = a + 0x6D2B79F5 | 0;
    const t = Math.imul(a ^ a >>> 15, 1 | a);
    const t2 = t + Math.imul(t ^ t >>> 7, 61 | t) ^ t;
    return ((t2 ^ t2 >>> 14) >>> 0) / 4294967296;
  };
}

export function buildHullReductionDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const rand = mulberry32(42);
  const count = 128;
  const points: number[] = [];
  for (let i = 0; i < count; i++) {
    const theta = 2 * Math.PI * rand();
    const phi = Math.acos(2 * rand() - 1);
    points.push(Math.sin(phi) * Math.cos(theta), Math.sin(phi) * Math.sin(theta), Math.cos(phi));
  }
  const hullHandle = runtime.createHullFromPoints(points);
  const body = world.createBody({ type: 2 as BodyType, position: [0, 1, 0] });
  runtime.createShapeFromHull(body, hullHandle, {});
  runtime.destroyHull(hullHandle);
  handles.push(body);
  return handles;
}

export function hullReductionGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const hullReductionBodies: RenderBody[] = [
  { kind: "box", size: [0.5, 0.5, 0.5], position: [0, 1, 0], color: 0xf59e0b },
];

export const hullReductionCamera: RenderSpec["camera"] = { position: [0, 15, 5], target: [0, 0, 0] };

export const dumpSampleName = "Hull Reduction";
export const dumpSampleId = "geometry/hull-reduction";
export const dumpNoPhysics = true;
export const dumpGroundSize = hullReductionGroundSize;
export const dumpBuildDynamicBodies = buildHullReductionDynamicBodies;
