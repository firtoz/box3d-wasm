import { type Box3DRuntime, type HullHandle, type PhysicsWorld, type Vec3, type WorldTransform } from "box3d-wasm";
import { cameraFromSetView } from "../shared";
import type { DistanceDump } from "./distance-debug-scene";

const IDENTITY: WorldTransform = { position: [0, 0, 0], rotation: [0, 0, 0, 1] };
const TRANSFORM_B: WorldTransform = { position: [0, 1, 0], rotation: [0, 0, 0, 1] };
const POINT: Vec3 = [0, 0, 0];
const SEGMENT: readonly [Vec3, Vec3] = [[-0.5, 0, 0], [0.5, 0, 0]];
const TRIANGLE: readonly [Vec3, Vec3, Vec3] = [[-1.5, 0, 0], [1.5, 0, 0], [0, 0, 2]];

export const ShapeDistanceType = { Point: 0, Segment: 1, Triangle: 2, Box: 3 } as const;
export type ShapeDistanceTypeId = (typeof ShapeDistanceType)[keyof typeof ShapeDistanceType];

function proxyPoints(runtime: Box3DRuntime, type: ShapeDistanceTypeId, boxHull: HullHandle | null): number[] {
  if (type === ShapeDistanceType.Point) return [...POINT];
  if (type === ShapeDistanceType.Segment) return [...SEGMENT[0], ...SEGMENT[1]];
  if (type === ShapeDistanceType.Triangle) return [...TRIANGLE[0], ...TRIANGLE[1], ...TRIANGLE[2]];
  return runtime.getHullPoints(boxHull!);
}

export function runShapeDistance(
  runtime: Box3DRuntime,
  typeA: ShapeDistanceTypeId = ShapeDistanceType.Triangle,
  typeB: ShapeDistanceTypeId = ShapeDistanceType.Box,
  radiusA = 0,
  radiusB = 0,
): DistanceDump {
  const box = runtime.makeBoxHull([0.125, 0.25, 0.5]);
  const result = runtime.shapeDistance(
    { points: proxyPoints(runtime, typeA, box), radius: radiusA },
    { points: proxyPoints(runtime, typeB, box), radius: radiusB },
    IDENTITY,
    TRANSFORM_B,
    radiusA > 0 || radiusB > 0,
  );
  runtime.destroyHull(box);
  return { d: result.distance, pA: result.pointA, pB: result.pointB, n: result.normal, i: result.iterations, s: result.simplexCount };
}

export function buildShapeDistanceDynamicBodies(): never[] {
  return [];
}

export function shapeDistanceGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const shapeDistanceCamera = cameraFromSetView(-45, 10, 5, [0, 0, 0]);
export const shapeDistanceTriangle = TRIANGLE;
export const shapeDistanceSegment = SEGMENT;
export const shapeDistanceTransformB = TRANSFORM_B;

export const dumpSampleName = "Shape Distance";
export const dumpSampleId = "collision/shape-distance";
export const dumpCppSampleName = "Shape Distance";
export const dumpNoPhysics = true;
export const dumpOwnsStep = true;
export const dumpGroundSize = shapeDistanceGroundSize;
export const dumpBuildDynamicBodies = buildShapeDistanceDynamicBodies;

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: never[];
  state: { distance: DistanceDump };
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: [], state: { distance: runShapeDistance(runtime) } };
}

export function dumpStep(
  _world: PhysicsWorld,
  runtime: Box3DRuntime,
  _handles: readonly never[],
  _frame: number,
  _dt: number,
  state: { distance: DistanceDump },
): void {
  state.distance = runShapeDistance(runtime);
}

export function dumpCheckpointExtras(
  _world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly never[],
  _frame: number,
  state: { distance: DistanceDump },
): Record<string, unknown> {
  return { distance: state.distance };
}
