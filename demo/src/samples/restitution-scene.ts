import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "./generic-host";

const count = 40;

export function buildRestitutionDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const dr = 1.0 / (count - 1);
  let x = -(count - 1);
  let restitution = 0;

  for (let i = 0; i < count; i++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [x, 40, 0] });
    runtime.createSphereShape(body, [0, 0, 0], 0.5, { restitution });
    handles.push(body);
    restitution += dr;
    x += 2.0;
  }

  return handles;
}

export function restitutionGroundSize(): Vec3 {
  return [50, 1, 50];
}

export function createRestitutionBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  let x = -(count - 1);
  for (let i = 0; i < count; i++) {
    bodies.push({ kind: "sphere", radius: 0.5, position: [x, 40, 0], color: 0x38bdf8, type: BodyType.Dynamic });
    x += 2.0;
  }
  return bodies;
}

export const restitutionCamera: RenderSpec["camera"] = { position: [0, 25, 85], target: [0, 20, 0] };

export const dumpSampleName = "Restitution";
export const dumpSampleId = "restitution";
export const dumpCppSampleName = "Restitution";
export const dumpGroundSize = restitutionGroundSize;
export const dumpBuildDynamicBodies = buildRestitutionDynamicBodies;
