import { B3_DEG_TO_RAD, BodyType, quatFromAxisAngle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const count = 10;
const tiltAngle = 30 * B3_DEG_TO_RAD;
const tiltQuat = quatFromAxisAngle([0, 0, 1], tiltAngle);

export function buildHighResistanceDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const rotation = runtime.makeQuatFromAxisAngle([0, 0, 1], tiltAngle);

  for (let i = 0; i < count; i++) {
    const body = world.createBody({
      type: BodyType.Dynamic,
      position: [-22 + 5 * i, 1.5, 0],
      rotation,
    });
    runtime.createCapsuleShape(body, [0, -1, 0], [0, 1, 0], 0.5, { rollingResistance: 0.2 * i });
    handles.push(body);
  }

  return handles;
}

export function highResistanceGroundSize(): Vec3 {
  return [50, 1, 50];
}

export function createHighResistanceBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < count; i++) {
    bodies.push({
      kind: "capsule",
      radius: 0.5,
      length: 2,
      position: [-22 + 5 * i, 1.5, 0],
      rotation: tiltQuat,
      color: 0x60a5fa + i * 0x0a0a0a,
    });
  }
  return bodies;
}

export const highResistanceCamera: RenderSpec["camera"] = { position: [0, 5, 40], target: [0, 7.5, 0] };

export const dumpSampleName = "High Resistance";
export const dumpSampleId = "shapes/high-resistance";
export const dumpCppSampleName = "High Resistance";
export const dumpGroundSize = highResistanceGroundSize;
export const dumpBuildDynamicBodies = buildHighResistanceDynamicBodies;
