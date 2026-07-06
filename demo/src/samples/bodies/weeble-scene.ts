import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Mat3, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

function steinerAdjustedInertia(inertia: Mat3, mass: number, offset: Vec3): Mat3 {
  const r2 = offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2];
  const out = [...inertia] as unknown as Mat3;
  for (let i = 0; i < 3; i++) {
    for (let j = 0; j < 3; j++) {
      out[i * 3 + j] += mass * (r2 * (i === j ? 1 : 0) - offset[i] * offset[j]);
    }
  }
  return out;
}

export function buildWeebleDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const body = world.createBody({ type: BodyType.Dynamic, position: [0, 3, 0] });

  runtime.createCapsuleShape(body, [0, -1, 0], [0, 1, 0], 1, {
    density: 1,
    rollingResistance: 0.1,
  });

  const mass = runtime.getBodyMassData(body).mass;
  const inertia = runtime.getBodyLocalRotationalInertia(body);
  const offset: Vec3 = [0, -1.5, 0];
  const adjusted = steinerAdjustedInertia(inertia, mass, offset);
  runtime.setBodyMassData(body, mass, offset, adjusted);

  return [body];
}

export function weebleGroundSize(): Vec3 {
  return [30, 1, 30];
}

export const weebleBodies: RenderBody[] = [
  { kind: "capsule", radius: 1, length: 2, position: [0, 3, 0], color: 0x3b82f6 },
];

export const weebleCamera: RenderSpec["camera"] = { position: [45, 25, 25], target: [0, 0, 0] };

export const dumpSampleName = "Weeble";
export const dumpSampleId = "bodies/weeble";
export const dumpCppSampleName = "Weeble";
export const dumpGroundSize = weebleGroundSize;
export const dumpBuildDynamicBodies = buildWeebleDynamicBodies;
