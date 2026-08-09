import {B3_AXIS_Z, B3_PI, BodyType, quatFromAxisAngle, type BodyId, type Box3DRuntime, type PhysicsWorld, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Div, f32Mul } from "../f32";

const NUM_SEGS = 7;
const RIM_R = f32(2);
const RIM_H = f32(2);
const COUNT = 8;
const SEPARATION = f32(6);
/** `15.0f * B3_PI / 180.0f` */
const TILT_RAD = f32Div(f32Mul(15, B3_PI), 180);
const SPIN_OMEGA: Vec3 = [0, f32(75), 0];

function topHullPoints(runtime: Box3DRuntime): number[] {
  const points: number[] = [];
  const dphi = f32Div(f32Mul(2, B3_PI), NUM_SEGS);
  for (let i = 0; i < NUM_SEGS; i++) {
    const angle = f32Mul(i, dphi);
    points.push(f32Mul(RIM_R, runtime.b3wCosf(angle)), RIM_H, f32Mul(RIM_R, runtime.b3wSinf(angle)));
  }
  points.push(0, 0, 0);
  return points;
}

export function buildGyroscopicPrecessionDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const points = topHullPoints(runtime);
  const hull = runtime.createHullFromPoints(points);
  const rotation = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, TILT_RAD);
  const angularVelocity = runtime.rotateVector(rotation, SPIN_OMEGA);
  const halfCount = (COUNT / 2) | 0;
  const handles: BodyId[] = [];

  for (let x = 0; x < COUNT; x++) {
    for (let z = 0; z < COUNT; z++) {
      const body = world.createBody({
        type: BodyType.Dynamic,
        position: [
          f32Mul(x - halfCount, SEPARATION),
          RIM_H,
          f32Mul(z - halfCount, SEPARATION),
        ],
        rotation,
        allowFastRotation: true,
      });
      runtime.createShapeFromHull(body, hull);
      runtime.setBodyAngularVelocity(body, angularVelocity);
      handles.push(body);
    }
  }

  runtime.destroyHull(hull);
  return handles;
}

export function gyroscopicPrecessionGroundSize(): Vec3 {
  return [40, 1, 40];
}

function topRenderPoints(): [number, number, number][] {
  // Visual-only; physics uses b3wCosf/b3wSinf. Math.cos is fine for rendering.
  const points: [number, number, number][] = [];
  const dphi = (2 * Math.PI) / NUM_SEGS;
  for (let i = 0; i < NUM_SEGS; i++) {
    points.push([RIM_R * Math.cos(i * dphi), RIM_H, RIM_R * Math.sin(i * dphi)]);
  }
  points.push([0, 0, 0]);
  return points;
}

export function createGyroscopicPrecessionBodies(): RenderBody[] {
  const points = topRenderPoints();
  const halfCount = (COUNT / 2) | 0;
  const rotation = quatFromAxisAngle(B3_AXIS_Z, TILT_RAD);
  const bodies: RenderBody[] = [];
  for (let x = 0; x < COUNT; x++) {
    for (let z = 0; z < COUNT; z++) {
      bodies.push({
        kind: "hull",
        points,
        position: [(x - halfCount) * SEPARATION, RIM_H, (z - halfCount) * SEPARATION],
        rotation,
        color: 0x3b82f6,
      });
    }
  }
  return bodies;
}

export const gyroscopicPrecessionCamera: RenderSpec["camera"] = cameraFromSetView(40, 30, 75, [0, 2, 0]);

export const dumpSampleName = "Gyroscopic Precession";
export const dumpSampleId = "bodies/gyroscopic-precession";
export const dumpCppSampleName = "Gyroscopic Precession";
export const dumpGroundSize = gyroscopicPrecessionGroundSize;
export const dumpBuildDynamicBodies = buildGyroscopicPrecessionDynamicBodies;
