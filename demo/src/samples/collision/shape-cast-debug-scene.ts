import { type Box3DRuntime, type PhysicsWorld, type Vec3, type WorldTransform } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const SCALE = 0.01;
const TRIANGLE: readonly [Vec3, Vec3, Vec3] = [
  [0, 0, 0],
  [0, -6400 * SCALE, 0],
  [6400 * SCALE, 0, 22.609375 * SCALE],
];
const CAPSULE = {
  center1: [43616.2109375 * SCALE, -100213 * SCALE, 132631.8125 * SCALE] as Vec3,
  center2: [342231.96875 * SCALE, 359711.6875 * SCALE, 132631.8125 * SCALE] as Vec3,
  radius: SCALE,
};
const TRANSFORM: WorldTransform = {
  position: [-115200 * SCALE, -19200 * SCALE, -202755 * SCALE],
  rotation: [0, 0, 0, 1],
};
const TRANSLATION: Vec3 = [0.008614914 * SCALE, 0, 72267.1171875 * SCALE];
const MAX_FRACTION = 0.970617533;

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

export const shapeCastDebugBodies: RenderBody[] = [];
export const shapeCastDebugCamera: RenderSpec["camera"] = cameraFromSetView(120, 30, 20, [0, 1.5, 0]);
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
