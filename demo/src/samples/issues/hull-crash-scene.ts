import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const raw = [
  [100.0, -142.292389, 130.826111],
  [99.5354385, -71.3011093, 130.826111],
  [99.5930862, -80.1112213, -100.0],
  [100.0, -142.292389, -100.0],
  [99.5930862, -80.1112213, 130.826111],
];

function makePoints(): number[] {
  const s = 0.01;
  const points: number[] = [];
  for (const p of raw) {
    points.push(p[0] * s, p[1] * s, p[2] * s);
  }
  return points;
}

export function buildHullCrashDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const hullHandle = runtime.createHullFromPoints(makePoints());
  const body = world.createBody({ type: BodyType.Dynamic, position: [0, 1.5, 0] });
  runtime.createShapeFromHull(body, hullHandle, {});
  runtime.destroyHull(hullHandle);
  handles.push(body);
  return handles;
}

export function hullCrashGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const hullCrashBodies: RenderBody[] = [
  { kind: "box", size: [0.5, 0.5, 0.5], position: [0, 1.5, 0], color: 0xef4444 },
];

export const hullCrashCamera: RenderSpec["camera"] = { position: [0, 15, 5], target: [0, 0, 0] };

export const dumpSampleName = "Hull Crash";
export const dumpSampleId = "issues/hull-crash";
export const dumpNoPhysics = true;
export const dumpGroundSize = hullCrashGroundSize;
export const dumpBuildDynamicBodies = buildHullCrashDynamicBodies;
