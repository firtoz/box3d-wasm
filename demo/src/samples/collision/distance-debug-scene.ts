import { type Box3DRuntime, type PhysicsWorld, type Vec3, type WorldTransform } from "box3d-wasm";
import { cameraFromSetView } from "../shared";

const IDENTITY: WorldTransform = { position: [0, 0, 0], rotation: [0, 0, 0, 1] };
const TRANSFORM_B: WorldTransform = {
  position: [-1.64657831e-6, 1.00989532471, 0],
  rotation: [0, 0, 0.004947796, 0.999987781],
};

export type DistanceDump = {
  d: number;
  pA: Vec3;
  pB: Vec3;
  n: Vec3;
  i: number;
  s: number;
};

export function runDistanceDebug(runtime: Box3DRuntime): DistanceDump {
  const boxA = runtime.makeBoxHull([40, 1, 40]);
  const boxB = runtime.makeTransformedBoxHull([0.5, 10, 0.5], { position: [0, 10, 0] });
  const result = runtime.shapeDistance(
    { points: runtime.getHullPoints(boxA), radius: 0 },
    { points: runtime.getHullPoints(boxB), radius: 0 },
    IDENTITY,
    TRANSFORM_B,
    false,
  );
  runtime.destroyHull(boxA);
  runtime.destroyHull(boxB);
  return { d: result.distance, pA: result.pointA, pB: result.pointB, n: result.normal, i: result.iterations, s: result.simplexCount };
}

export function buildDistanceDebugDynamicBodies(): never[] {
  return [];
}

export function distanceDebugGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const distanceDebugCamera = cameraFromSetView(120, 30, 20, [0, 1.5, 0]);
export const distanceDebugTransformA = IDENTITY;
export const distanceDebugTransformB = TRANSFORM_B;

export const dumpSampleName = "Distance Debug";
export const dumpSampleId = "collision/distance-debug";
export const dumpCppSampleName = "Distance Debug";
export const dumpNoPhysics = true;
export const dumpOwnsStep = true;
export const dumpGroundSize = distanceDebugGroundSize;
export const dumpBuildDynamicBodies = buildDistanceDebugDynamicBodies;

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: never[];
  state: { distance: DistanceDump };
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: [], state: { distance: runDistanceDebug(runtime) } };
}

export function dumpStep(
  _world: PhysicsWorld,
  runtime: Box3DRuntime,
  _handles: readonly never[],
  _frame: number,
  _dt: number,
  state: { distance: DistanceDump },
): void {
  state.distance = runDistanceDebug(runtime);
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
