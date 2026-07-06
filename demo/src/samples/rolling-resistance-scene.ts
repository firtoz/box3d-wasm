import { B3_AXIS_X, B3_DEG_TO_RAD, BodyType, quatFromAxisAngle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "./generic-host";

const count = 5;
const planeAngle = 10 * B3_DEG_TO_RAD;

export function buildRollingResistanceDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const planeRotation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, planeAngle);

  const planeBody = world.createBody({ position: [0, 2, -20], rotation: planeRotation });
  runtime.createHullShape(planeBody, [32, 0.5, 15]);
  handles.push(planeBody);

  for (let index = 0; index < count; index++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [-25 + 5 * index, 8, -24], rotation: planeRotation });
    runtime.createSphereShape(body, [0, 0, 0], 1.0, { rollingResistance: 0.05 * index });
    handles.push(body);
  }

  for (let index = 0; index < count; index++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [2 + 5 * index, 8, -24], rotation: planeRotation });
    runtime.createCapsuleShape(body, [-1, 0, 0], [1, 0, 0], 0.5, { rollingResistance: 0.05 * index });
    handles.push(body);
  }

  return handles;
}

export function rollingResistanceGroundSize(): Vec3 {
  return [50, 1, 50];
}

export function createRollingResistanceBodies(): RenderBody[] {
  const bodies: RenderBody[] = [
    { kind: "box", size: [64, 1, 30], position: [0, 2, -20], rotation: quatFromAxisAngle(B3_AXIS_X, planeAngle), color: 0x94a3b8, type: BodyType.Static },
  ];
  for (let index = 0; index < count; index++) bodies.push({ kind: "sphere", radius: 1.0, position: [-25 + 5 * index, 8, -24], color: 0x38bdf8, type: BodyType.Dynamic });
  for (let index = 0; index < count; index++) bodies.push({ kind: "capsule", radius: 0.5, length: 2.0, position: [2 + 5 * index, 8, -24], color: 0xf97316, type: BodyType.Dynamic });
  return bodies;
}

export const rollingResistanceCamera: RenderSpec["camera"] = { position: [-140, 17, 60], target: [0, 7.5, 0] };

export const dumpSampleName = "Rolling Resistance";
export const dumpSampleId = "rolling-resistance";
export const dumpCppSampleName = "Rolling Resistance";
export const dumpGroundSize = rollingResistanceGroundSize;
export const dumpBuildDynamicBodies = buildRollingResistanceDynamicBodies;
