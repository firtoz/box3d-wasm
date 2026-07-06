import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildSphereStackDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  for (let i = 0, y = 0.75; i < 30; i++, y += 1.5) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [0, y, 0], isAwake: true, angularVelocity: [0, 0, 0] });
    runtime.createSphereShape(body, [0, 0, 0], 0.5, { rollingResistance: 0.1 });
    handles.push(body);
  }
  return handles;
}

export function sphereStackGroundSize(): Vec3 {
  return [15, 1, 15];
}

export function createSphereStackBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0, y = 0.75; i < 30; i++, y += 1.5) bodies.push({ kind: "sphere", radius: 0.5, position: [0, y, 0], color: 0x38bdf8 });
  return bodies;
}

export const sphereStackCamera: RenderSpec["camera"] = { position: [0, 22.94, 48.30], target: [0, 10, 0] };

export const dumpSampleName = "Sphere Stack";
export const dumpSampleId = "sphere-stack";
export const dumpCppSampleName = "Sphere Stack";
export const dumpGroundSize = sphereStackGroundSize;
export const dumpBuildDynamicBodies = buildSphereStackDynamicBodies;
