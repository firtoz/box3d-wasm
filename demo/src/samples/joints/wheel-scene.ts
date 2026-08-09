import {B3_AXIS_X, B3_AXIS_Y, B3_AXIS_Z, B3_PI, BodyType, quatFromAxisAngle, type BodyId, type Box3DRuntime, type PhysicsWorld, type Quat, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const CYLINDER_HEIGHT = 0.25;
const CYLINDER_RADIUS = 0.4;
const CYLINDER_SIDES = 12;
const BODY_POS: Vec3 = [0, 2, 0];

/** Matches `b3ComputeQuatBetweenUnitVectors(Y, Z)` for render (body rotation). */
export const WHEEL_BODY_ROTATION: Quat = quatFromAxisAngle(B3_AXIS_X, -0.5 * B3_PI);

export function buildWheelDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  // Empty static body for the joint (no shape) — included in dump handles.
  const groundId = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });

  const rotation = runtime.computeQuatBetweenUnitVectors(B3_AXIS_Y, B3_AXIS_Z);
  const body = world.createBody({
    type: BodyType.Dynamic,
    position: BODY_POS,
    rotation,
  });

  const hull = runtime.createCylinder(CYLINDER_HEIGHT, CYLINDER_RADIUS, 0, CYLINDER_SIDES);
  runtime.createShapeFromHull(body, hull);
  runtime.destroyHull(hull);

  const frameA = runtime.computeQuatBetweenUnitVectors(B3_AXIS_X, B3_AXIS_Y);
  const frameB = runtime.computeQuatBetweenUnitVectors(B3_AXIS_Z, B3_AXIS_Y);

  world.createWheelJoint(groundId, body, {
    localFrameA: { position: [0, 3, 0], rotation: frameA },
    localFrameB: { position: [0, 0, 0], rotation: frameB },
    collideConnected: true,
    enableSuspensionSpring: false,
    suspensionHertz: 2,
    suspensionDampingRatio: 0.7,
    enableSuspensionLimit: false,
    lowerSuspensionLimit: -1,
    upperSuspensionLimit: 1,
  });

  return [groundId, body];
}

export function wheelGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const wheelBodies: RenderBody[] = [
  // Index 0 = empty static joint ground (tiny placeholder; no upstream shape).
  { kind: "sphere", radius: 0.05, position: [0, -1, 0], color: 0x64748b, type: BodyType.Static },
  {
    kind: "compound",
    position: BODY_POS,
    rotation: WHEEL_BODY_ROTATION,
    parts: [
      {
        kind: "cylinder",
        radius: CYLINDER_RADIUS,
        height: CYLINDER_HEIGHT,
        segments: CYLINDER_SIDES,
        position: [0, 0.5 * CYLINDER_HEIGHT, 0],
        color: 0x38bdf8,
      },
    ],
  },
];

export const wheelCamera: RenderSpec["camera"] = cameraFromSetView(25, 20, 7, [0, 2, 0]);

export const dumpSampleName = "Wheel";
export const dumpSampleId = "joints/wheel";
export const dumpCppSampleName = "Wheel";
export const dumpGroundSize = wheelGroundSize;
export const dumpBuildDynamicBodies = buildWheelDynamicBodies;
