import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildHullTransformDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  let body = world.createBody({ type: BodyType.Dynamic, position: [-1.5, 2, 0] });
  runtime.createHullShape(body, [0.5, 0.5, 0.5], {});
  handles.push(body);
  body = world.createBody({ type: BodyType.Dynamic, position: [1.5, 2, 0] });
  runtime.createTransformedHullShape(body, [0.5, 0.25, 0.125], { position: [0.5, 0.3, 0], rotation: [0, 0, 0.382683, 0.92388] }, [1.5, 1, 0.5], {});
  handles.push(body);
  return handles;
}

export function hullTransformGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const hullTransformBodies: RenderBody[] = [
  { kind: "box", size: [1, 0.5, 0.5], position: [0, 1, 0], color: 0x8b5cf6 },
];

export const hullTransformCamera: RenderSpec["camera"] = { position: [0, 15, 5], target: [0, 0, 0] };

export const dumpSampleName = "Hull Transform";
export const dumpSampleId = "geometry/hull-transform";
export const dumpNoPhysics = true;
export const dumpGroundSize = hullTransformGroundSize;
export const dumpBuildDynamicBodies = buildHullTransformDynamicBodies;
