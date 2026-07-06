import { B3_AXIS_X, B3_PI, BodyType, quatFromAxisAngle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildGyroscopicTorqueDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const body = world.createBody({
    type: BodyType.Dynamic,
    position: [0, 2, 0],
    rotation: runtime.makeQuatFromAxisAngle(B3_AXIS_X, -0.5 * B3_PI),
    gravityScale: 0,
  });

  const cylinder = runtime.createCylinder(0.6, 0.15, 0, 32);

  runtime.createShapeFromHull(body, cylinder, {
    updateBodyMass: false,
  });

  runtime.createHullShape(body, [1, 0.05, 0.1], {
    updateBodyMass: false,
  });

  runtime.applyBodyMassFromShapes(body);

  runtime.setBodyAngularVelocity(body, [0.01, 0.01, 10]);

  runtime.destroyHull(cylinder);

  return [body];
}

export function gyroscopicTorqueGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const gyroscopicTorqueBodies: RenderBody[] = [
  {
    kind: "compound",
    position: [0, 2, 0],
    rotation: quatFromAxisAngle(B3_AXIS_X, -0.5 * B3_PI),
    parts: [
      { kind: "box", size: [2, 0.1, 0.2], color: 0x3b82f6 },
      { kind: "cylinder", radius: 0.15, height: 0.6, segments: 32, position: [0, 0.3, 0], color: 0x3b82f6 },
    ],
  },
];

export const gyroscopicTorqueCamera: RenderSpec["camera"] = { position: [0, 20, 4], target: [0, 2, 0] };

export const dumpSampleName = "Gyroscopic Torque";
export const dumpSampleId = "bodies/gyroscopic-torque";
export const dumpCppSampleName = "Gyroscopic Torque";
export const dumpGroundSize = gyroscopicTorqueGroundSize;
export const dumpBuildDynamicBodies = buildGyroscopicTorqueDynamicBodies;
