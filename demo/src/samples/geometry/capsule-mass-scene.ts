import { BodyId, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildCapsuleMassDynamicBodies(_world: PhysicsWorld, _runtime: Box3DRuntime): BodyId[] {
  return [];
}

export function capsuleMassGroundSize(): Vec3 {
  return [5, 0.5, 5];
}

export const capsuleMassBodies: RenderBody[] = [];

export const capsuleMassCamera: RenderSpec["camera"] = { position: [0, 15, 5], target: [0, 0, 0] };

export const dumpSampleName = "Capsule Mass";
export const dumpSampleId = "geometry/capsule-mass";
export const dumpCppSampleName = "Capsule Mass";
export const dumpNoPhysics = true;
export const dumpGroundSize = capsuleMassGroundSize;
export const dumpBuildDynamicBodies = buildCapsuleMassDynamicBodies;
