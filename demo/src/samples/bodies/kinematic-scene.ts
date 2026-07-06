import { B3_AXIS_Z, BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const AMPLITUDE = 2;
const DELAY = 2;
const GROUND_HALF: Vec3 = [20, 1, 20];

export function buildKinematicDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const body = world.createBody({ type: BodyType.Kinematic, position: [2 * AMPLITUDE, AMPLITUDE + 1, 0] });
  runtime.createHullShape(body, [0.1, 1.0, 0.2]);
  return [body];
}

export function kinematicGroundSize(): Vec3 {
  return GROUND_HALF;
}

export function stepKinematic(_world: PhysicsWorld, runtime: Box3DRuntime, bodyId: number, time: number, dt: number): number {
  if (dt > 0 && time > DELAY) {
    const t = time - DELAY;
    const point: Vec3 = [
      2 * AMPLITUDE * Math.cos(t),
      AMPLITUDE * (Math.sin(2 * t) + 1) + 1,
      0,
    ];
    const rotation = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, 2 * t);
    runtime.setBodyTargetTransform(bodyId, point, rotation, dt, true);
  }
  return Math.fround(time + dt);
}

export const kinematicBodies: RenderBody[] = [
  { kind: "box", size: [0.2, 2, 0.4], position: [4, 3, 0], color: 0x22c55e },
];

export const kinematicCamera: RenderSpec["camera"] = { position: [0, 30, 10], target: [0, 1.5, 0] };

export const dumpSampleName = "Kinematic";
export const dumpSampleId = "bodies/kinematic";
export const dumpCppSampleName = "Kinematic";
export const dumpGroundSize = kinematicGroundSize;
export const dumpBuildDynamicBodies = buildKinematicDynamicBodies;

let _kt = 0;
export function dumpStep(world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], _frame: number, dt: number): void {
  _kt = stepKinematic(world, runtime, handles[1], _kt, dt);
}
