import { B3_AXIS_X, B3_DEG_TO_RAD, BodyType, quatFromAxisAngle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const wallAngle = 90 * B3_DEG_TO_RAD;
const wallQuat = quatFromAxisAngle(B3_AXIS_X, wallAngle);
const bulletVel: Vec3 = [0, 0, -180];

export function buildThinWallDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const wallRotation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, wallAngle);

  const wall = world.createBody({ type: BodyType.Static, position: [0, 10, 0], rotation: wallRotation });
  runtime.createHullShape(wall, [10, 0.1, 10], {});
  handles.push(wall);

  const sphereBullet = world.createBody({
    type: BodyType.Dynamic, position: [-5, 10, 20], linearVelocity: bulletVel, angularVelocity: [20, 0, 0],
  });
  runtime.createSphereShape(sphereBullet, [0, 0, 0], 0.1, { rollingResistance: 0.1 });
  handles.push(sphereBullet);

  const capsuleBullet = world.createBody({
    type: BodyType.Dynamic, position: [0, 10, 20], linearVelocity: bulletVel, angularVelocity: [20, -5, 0],
  });
  runtime.createCapsuleShape(capsuleBullet, [-0.3, 0, 0], [0.3, 0, 0], 0.1, { rollingResistance: 0.1 });
  handles.push(capsuleBullet);

  const boxBullet = world.createBody({
    type: BodyType.Dynamic, position: [5, 10, 20], linearVelocity: bulletVel, angularVelocity: [20, 5, 0],
  });
  runtime.createHullShape(boxBullet, [0.4, 0.1, 0.1], { rollingResistance: 0.1 });
  handles.push(boxBullet);

  return handles;
}

export function thinWallGroundSize(): Vec3 { return [40, 1, 40]; }

export function createThinWallBodies(): RenderBody[] {
  return [
    { kind: "box", size: [20, 0.2, 20], position: [0, 10, 0], rotation: wallQuat, color: 0x94a3b8, type: BodyType.Static },
    { kind: "sphere", radius: 0.1, position: [-5, 10, 20], color: 0xf59e0b },
    { kind: "capsule", radius: 0.1, length: 0.6, position: [0, 10, 20], color: 0xf59e0b },
    { kind: "box", size: [0.8, 0.2, 0.2], position: [5, 10, 20], color: 0xf59e0b },
  ];
}

export const thinWallCamera: RenderSpec["camera"] = { position: [45, 30, 30], target: [0, 0, 0] };

export const dumpSampleName = "Thin Wall";
export const dumpSampleId = "continuous/thin-wall";
export const dumpCppSampleName = "Thin Wall";
export const dumpGroundSize = thinWallGroundSize;
export const dumpBuildDynamicBodies = buildThinWallDynamicBodies;
