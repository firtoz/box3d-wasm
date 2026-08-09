import {B3_AXIS_X, B3_DEG_TO_RAD, BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3, type BodyId} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32Mul, f32Sub } from "../f32";

const ANGLE = f32Mul(20, B3_DEG_TO_RAD);
const BOX_LOCAL_CENTER: Vec3 = [1, 0.5, 1];

export function buildSlideTwistOffCenterDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const handles: BodyId[] = [];
  const orientation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, ANGLE);

  const plane = world.createBody({ type: BodyType.Static, position: [0, 4, 0], rotation: orientation });
  runtime.createHullShape(plane, [10, 0.5, 10], { friction: 0.6 });
  handles.push(plane);

  const boxOffset = runtime.rotateVector(orientation, BOX_LOCAL_CENTER);
  const box = world.createBody({
    type: BodyType.Dynamic,
    position: [-boxOffset[0], f32Sub(5, boxOffset[1]), -boxOffset[2]],
    rotation: orientation,
  });
  // Upstream: b3MakeOffsetBoxHull(... boxLocalCenter) + b3CreateHullShape.
  runtime.createOffsetHullShape(box, [1, 0.5, 1], BOX_LOCAL_CENTER, { friction: 0.3 });

  const spun = runtime.rotateVector(orientation, [0, 1, 0]);
  world.setBodyAngularVelocity(box, [f32Mul(25, spun[0]), f32Mul(25, spun[1]), f32Mul(25, spun[2])]);
  handles.push(box);

  return handles;
}

export function slideTwistOffCenterGroundSize(): Vec3 {
  return [50, 1, 50];
}

export function createSlideTwistOffCenterBodies(runtime: Box3DRuntime): RenderBody[] {
  const orientation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, ANGLE);
  const boxOffset = runtime.rotateVector(orientation, BOX_LOCAL_CENTER);
  return [
    {
      kind: "box",
      size: [20, 1, 20],
      position: [0, 4, 0],
      rotation: orientation,
      color: 0x94a3b8,
      type: BodyType.Static,
    },
    {
      kind: "box",
      size: [2, 1, 2],
      position: [-boxOffset[0], f32Sub(5, boxOffset[1]), -boxOffset[2]],
      rotation: orientation,
      localPosition: BOX_LOCAL_CENTER,
      color: 0x60a5fa,
    },
  ];
}

export const slideTwistOffCenterCamera: RenderSpec["camera"] = cameraFromSetView(-30, 17, 30, [0, 5, 0]);

export const dumpSampleName = "Slide Twist Off Center Shape";
export const dumpSampleId = "issues/slide-twist-off-center";
export const dumpCppSampleName = "Slide Twist Off Center Shape";
export const dumpGroundSize = slideTwistOffCenterGroundSize;
export const dumpBuildDynamicBodies = buildSlideTwistOffCenterDynamicBodies;
