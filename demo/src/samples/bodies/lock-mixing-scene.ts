import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildLockMixingDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const halfWidths: Vec3 = [1, 1, 1];

  let body = world.createBody({ type: BodyType.Dynamic, position: [0, 2, 0] });
  runtime.createHullShape(body, halfWidths);
  handles.push(body);

  body = world.createBody({ type: BodyType.Dynamic, position: [2, 2, 0] });
  runtime.setBodyMotionLocks(body, { lockRotationX: true, lockRotationZ: true });
  runtime.createHullShape(body, halfWidths);
  handles.push(body);

  body = world.createBody({ type: BodyType.Dynamic, position: [-2, 2, 0] });
  runtime.setBodyMotionLocks(body, { lockX: true, lockY: true, lockLinearZ: true });
  runtime.createHullShape(body, halfWidths);
  handles.push(body);

  body = world.createBody({ type: BodyType.Dynamic, position: [0, 1, 2] });
  runtime.setBodyMotionLocks(body, { lockX: true, lockY: true, lockLinearZ: true, lockRotationX: true, lockRotationY: true, lockRotationZ: true });
  runtime.createHullShape(body, halfWidths);
  handles.push(body);

  body = world.createBody({ type: BodyType.Static, position: [0, 1, -3] });
  runtime.createHullShape(body, halfWidths);
  handles.push(body);

  return handles;
}

export function lockMixingGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const lockMixingBodies: RenderBody[] = [
  { kind: "box", size: [2, 2, 2], position: [0, 2, 0], color: 0x22c55e },
  { kind: "box", size: [2, 2, 2], position: [2, 2, 0], color: 0x3b82f6 },
  { kind: "box", size: [2, 2, 2], position: [-2, 2, 0], color: 0xf59e0b },
  { kind: "box", size: [2, 2, 2], position: [0, 1, 2], color: 0xef4444 },
  { kind: "box", size: [2, 2, 2], position: [0, 1, -3], color: 0x888888, type: BodyType.Static },
];

export const lockMixingCamera: RenderSpec["camera"] = { position: [45, 30, 40], target: [0, 0, 0] };

export const dumpSampleName = "Lock Mixing";
export const dumpSampleId = "bodies/lock-mixing";
export const dumpCppSampleName = "Lock Mixing";
export const dumpGroundSize = lockMixingGroundSize;
export const dumpBuildDynamicBodies = buildLockMixingDynamicBodies;
