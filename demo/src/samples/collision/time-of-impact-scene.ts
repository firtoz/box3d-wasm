import { type Box3DRuntime, type HullHandle, type PhysicsWorld, type Sweep, type TimeOfImpactResult, type Vec3, ToiState } from "box3d-wasm";
import { cameraFromSetView } from "../shared";

export const ToiShape = { Box: 0, Capsule: 1, Triangle: 2 } as const;
export type ToiShapeId = (typeof ToiShape)[keyof typeof ToiShape];

const TRIANGLE: readonly [Vec3, Vec3, Vec3] = [[-4, 0, -4], [-4, 0, -8], [-8, 0, -8]];
const CAPSULE = { center1: [0, -0.2, 0] as Vec3, center2: [0, 0.2, 0] as Vec3, radius: 0.02 };
const SWEEP_A: Sweep = { c1: [0, 0, 0], c2: [0, 0, 0], q1: [0, 0, 0, 1], q2: [0, 0, 0, 1] };
const SWEEP_B: Sweep = {
  c1: [-4.0651207, 0.101333618, -7.87591267],
  c2: [-4.15895557, 0.0356027633, -7.69682646],
  q1: [-0.860495985, -0.272824734, 0.0724888667, 0.424097389],
  q2: [-0.604184389, -0.424355596, 0.0457959622, 0.672894001],
};

function proxyPoints(runtime: Box3DRuntime, type: ToiShapeId, boxHull: HullHandle | null): { points: number[]; radius: number } {
  if (type === ToiShape.Capsule) return { points: [...CAPSULE.center1, ...CAPSULE.center2], radius: CAPSULE.radius };
  if (type === ToiShape.Triangle) return { points: [...TRIANGLE[0], ...TRIANGLE[1], ...TRIANGLE[2]], radius: 0 };
  return { points: runtime.getHullPoints(boxHull!), radius: 0 };
}

export function runTimeOfImpact(
  runtime: Box3DRuntime,
  typeA: ToiShapeId = ToiShape.Triangle,
  typeB: ToiShapeId = ToiShape.Capsule,
): TimeOfImpactResult {
  const box = runtime.makeBoxHull([0.02, 0.2, 0.04]);
  const a = proxyPoints(runtime, typeA, box);
  const b = proxyPoints(runtime, typeB, box);
  const result = runtime.timeOfImpact(a, b, SWEEP_A, SWEEP_B, 1);
  runtime.destroyHull(box);
  return result;
}

export function toiDump(result: TimeOfImpactResult): Record<string, unknown> {
  return {
    state: result.state,
    f: result.fraction,
    d: result.distance,
    p: result.point,
    n: result.normal,
    i: [result.distanceIterations, result.pushBackIterations, result.rootIterations],
  };
}

export function buildTimeOfImpactDynamicBodies(): never[] {
  return [];
}

export function timeOfImpactGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const timeOfImpactCamera = cameraFromSetView(-90, 0, 10, [0, 0, 0]);
export const timeOfImpactTriangle = TRIANGLE;
export const timeOfImpactCapsule = CAPSULE;
export const timeOfImpactSweepB = SWEEP_B;
export { ToiState };

export const dumpSampleName = "Time of Impact";
export const dumpSampleId = "collision/time-of-impact";
export const dumpCppSampleName = "Time of Impact";
export const dumpNoPhysics = true;
export const dumpOwnsStep = true;
export const dumpGroundSize = timeOfImpactGroundSize;
export const dumpBuildDynamicBodies = buildTimeOfImpactDynamicBodies;

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: never[];
  state: { toi: ReturnType<typeof toiDump> };
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: [], state: { toi: toiDump(runTimeOfImpact(runtime)) } };
}

export function dumpStep(
  _world: PhysicsWorld,
  runtime: Box3DRuntime,
  _handles: readonly never[],
  _frame: number,
  _dt: number,
  state: { toi: ReturnType<typeof toiDump> },
): void {
  state.toi = toiDump(runTimeOfImpact(runtime));
}

export function dumpCheckpointExtras(
  _world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly never[],
  _frame: number,
  state: { toi: ReturnType<typeof toiDump> },
): Record<string, unknown> {
  return { toi: state.toi };
}
