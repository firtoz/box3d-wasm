import { B3_AXIS_X, BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const f32 = Math.fround;
const B3_DEG_TO_RAD_F32 = f32(0.01745329251);
const angle = f32(20 * B3_DEG_TO_RAD_F32);

export function buildSlideTwistDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const rotation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, angle);

  const plane = world.createBody({ type: BodyType.Static, position: [0, 4, 0], rotation });
  runtime.createHullShape(plane, [10, 0.5, 10], { friction: 0.6 });
  handles.push(plane);

  const rotated = runtime.rotateVector(rotation, [0, 1, 0]);
  const angularVelocity: Vec3 = [f32(25 * rotated[0]), f32(25 * rotated[1]), f32(25 * rotated[2])];

  const box = world.createBody({
    type: BodyType.Dynamic,
    position: [0, 5, 0],
    rotation,
    angularVelocity,
  });
  runtime.createHullShape(box, [1, 0.5, 1], { friction: 0.3 });
  handles.push(box);

  return handles;
}

export function slideTwistGroundSize(): Vec3 { return [50, 1, 50]; }

export function createSlideTwistBodies(): RenderBody[] {
  const sinHalf = Math.sin(angle / 2);
  const cosHalf = Math.cos(angle / 2);
  const q: [number, number, number, number] = [sinHalf, 0, 0, cosHalf];
  return [
    { kind: "box", size: [20, 1, 20], position: [0, 4, 0], rotation: q, color: 0x94a3b8, type: BodyType.Static },
    { kind: "box", size: [2, 1, 2], position: [0, 5, 0], rotation: q, color: 0x60a5fa },
  ];
}

export const slideTwistCamera: RenderSpec["camera"] = { position: [-30, 17, 30], target: [0, 5, 0] };

export const dumpSampleName = "Slide Twist";
export const dumpSampleId = "shapes/slide-twist";
export const dumpCppSampleName = "Slide Twist";
export const dumpGroundSize = slideTwistGroundSize;
export const dumpBuildDynamicBodies = buildSlideTwistDynamicBodies;
