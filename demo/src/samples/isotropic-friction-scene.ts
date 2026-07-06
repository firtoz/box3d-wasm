import { B3_AXIS_Y, B3_PI, BodyType, quatFromAxisAngle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "./generic-host";

const boxCount = 32;
const f = Math.fround;

export function buildIsotropicFrictionDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];

  for (let index = 0; index < boxCount; ++index) {
    const alpha = f(f(B3_PI / 16.0) * index);
    const cs = runtime.b3wCos(alpha);
    const sn = runtime.b3wSin(alpha);
    const body = world.createBody({
      type: BodyType.Dynamic,
      position: [f(15.0 * cs), 1.0, f(15.0 * sn)],
      rotation: runtime.makeQuatFromAxisAngle(B3_AXIS_Y, -alpha),
      linearVelocity: [f(25.0 * cs), 0.0, f(25.0 * sn)],
    });
    runtime.createHullShape(body, [1.0, 1.0, 1.0], { friction: 0.6 });
    handles.push(body);
  }

  return handles;
}

export function isotropicFrictionGroundSize(): Vec3 {
  return [100, 1, 100];
}

export function createIsotropicFrictionBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let index = 0; index < boxCount; ++index) {
    const alpha = B3_PI / 16.0 * index;
    bodies.push({
      kind: "box",
      size: [2, 2, 2],
      position: [15.0 * Math.cos(alpha), 1.0, 15.0 * Math.sin(alpha)],
      rotation: quatFromAxisAngle(B3_AXIS_Y, -alpha),
      color: 0x38bdf8,
      type: BodyType.Dynamic,
    });
  }
  return bodies;
}

export const isotropicFrictionCamera: RenderSpec["camera"] = { position: [45, 30, 150], target: [0, 0, 0] };

export const dumpSampleName = "Isotropic Friction";
export const dumpSampleId = "isotropic-friction";
export const dumpCppSampleName = "Isotropic Friction";
export const dumpGroundSize = isotropicFrictionGroundSize;
export const dumpBuildDynamicBodies = buildIsotropicFrictionDynamicBodies;
