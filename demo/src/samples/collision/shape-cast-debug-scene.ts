import { type Box3DRuntime, type PhysicsWorld, type Vec3, type WorldTransform } from "box3d-wasm";
import { f32, f32Mul, f32Sub } from "../f32";
import { cameraFromSetView } from "../shared";

const SCALE = f32(0.01);
function s(x: number, y: number, z: number): Vec3 {
  return [f32Mul(x, SCALE), f32Mul(y, SCALE), f32Mul(z, SCALE)];
}

const ORIGIN = s(0, 0, 0);
const TRIANGLE: readonly [Vec3, Vec3, Vec3] = [
  [f32Sub(s(0, 0, 0)[0], ORIGIN[0]), f32Sub(s(0, 0, 0)[1], ORIGIN[1]), f32Sub(s(0, 0, 0)[2], ORIGIN[2])],
  [f32Sub(s(0, -6400, 0)[0], ORIGIN[0]), f32Sub(s(0, -6400, 0)[1], ORIGIN[1]), f32Sub(s(0, -6400, 0)[2], ORIGIN[2])],
  [f32Sub(s(6400, 0, 22.609375)[0], ORIGIN[0]), f32Sub(s(6400, 0, 22.609375)[1], ORIGIN[1]), f32Sub(s(6400, 0, 22.609375)[2], ORIGIN[2])],
];
const CAPSULE = {
  center1: s(43616.2109375, -100213, 132631.8125),
  center2: s(342231.96875, 359711.6875, 132631.8125),
  radius: SCALE,
};
const TRANSFORM: WorldTransform = {
  position: [
    f32Sub(s(-115200, -19200, -202755)[0], ORIGIN[0]),
    f32Sub(s(-115200, -19200, -202755)[1], ORIGIN[1]),
    f32Sub(s(-115200, -19200, -202755)[2], ORIGIN[2]),
  ],
  rotation: [0, 0, 0, 1],
};
const TRANSLATION: Vec3 = s(0.008614914, 0, 72267.1171875);
const MAX_FRACTION = f32(0.970617533);

export type ShapeCastDebugDump = {
  h: number;
  f: number;
  p: Vec3;
  n: Vec3;
};

export function runShapeCastDebug(runtime: Box3DRuntime): ShapeCastDebugDump {
  const result = runtime.shapeCast(
    { points: TRIANGLE.flat(), radius: 0 },
    { points: [...CAPSULE.center1, ...CAPSULE.center2], radius: CAPSULE.radius },
    TRANSFORM,
    TRANSLATION,
    { maxFraction: MAX_FRACTION, canEncroach: false },
  );
  return {
    h: result.hit ? 1 : 0,
    f: result.fraction,
    p: result.point,
    n: result.normal,
  };
}

export function buildShapeCastDebugDynamicBodies(): never[] {
  return [];
}

export function shapeCastDebugGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const shapeCastDebugBodies: never[] = [];
export const shapeCastDebugCamera = cameraFromSetView(120, 30, 20, [0, 1.5, 0]);
export const shapeCastDebugTriangle = TRIANGLE;
export const shapeCastDebugCapsule = CAPSULE;
export const shapeCastDebugTransform = TRANSFORM;
export const shapeCastDebugTranslation = TRANSLATION;

export const dumpSampleName = "Shape Cast Debug";
export const dumpSampleId = "collision/shape-cast-debug";
export const dumpCppSampleName = "Shape Cast Debug";
export const dumpNoPhysics = true;
export const dumpOwnsStep = true;
export const dumpGroundSize = shapeCastDebugGroundSize;
export const dumpBuildDynamicBodies = buildShapeCastDebugDynamicBodies;

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: never[];
  state: { cast: ShapeCastDebugDump };
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: [], state: { cast: runShapeCastDebug(runtime) } };
}

export function dumpStep(
  _world: PhysicsWorld,
  runtime: Box3DRuntime,
  _handles: readonly never[],
  _frame: number,
  _dt: number,
  state: { cast: ShapeCastDebugDump },
): void {
  state.cast = runShapeCastDebug(runtime);
}

export function dumpCheckpointExtras(
  _world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly never[],
  _frame: number,
  state: { cast: ShapeCastDebugDump },
): Record<string, unknown> {
  return { cast: state.cast };
}
