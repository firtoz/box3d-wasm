import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const QUARTER_TURN_Z: [number, number, number, number] = [0, 0, 0.7071067811865476, 0.7071067811865476];

export function buildFixedRotationDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];

  let body = world.createBody({ type: BodyType.Static, position: [0, 1, 0], rotation: QUARTER_TURN_Z });
  runtime.createCapsuleShape(body, [-0.5, 0, 0], [0.5, 0, 0], 0.3, {});
  handles.push(body);

  body = world.createBody({
    type: BodyType.Dynamic, position: [0.3, 1, 0],
    gravityScale: 0, enableSleep: false, isAwake: true, rotation: QUARTER_TURN_Z,
  });
  runtime.setBodyMotionLocks(body, { lockRotationX: true, lockRotationY: true, lockRotationZ: true });
  runtime.createCapsuleShape(body, [-0.5, 0, 0], [0.5, 0, 0], 0.2, {});
  handles.push(body);

  return handles;
}

const GROUND_HALF: Vec3 = [10, 1, 10];

export function fixedRotationGroundSize(): Vec3 {
  return GROUND_HALF;
}

export const fixedRotationBodies: RenderBody[] = [
  { kind: "capsule", radius: 0.3, length: 1, position: [0, 1, 0], rotation: QUARTER_TURN_Z, color: 0x888888, type: BodyType.Static },
  { kind: "capsule", radius: 0.2, length: 1, position: [0.3, 1, 0], rotation: QUARTER_TURN_Z, color: 0x3b82f6 },
];

export const fixedRotationCamera: RenderSpec["camera"] = { position: [0, 15, 10], target: [0, 0, 0] };

export const dumpSampleName = "Fixed Rotation";
export const dumpSampleId = "bodies/fixed-rotation";
export const dumpCppSampleName = "Fixed Rotation";
export const dumpGroundSize = fixedRotationGroundSize;
export const dumpBuildDynamicBodies = buildFixedRotationDynamicBodies;
