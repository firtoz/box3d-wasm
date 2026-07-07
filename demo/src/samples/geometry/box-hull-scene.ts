import type { BodyType, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildBoxHullDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  let body = world.createBody({ type: 2 as BodyType, position: [0, 2, 0] });
  runtime.createHullShape(body, [0.5, 0.25, 0.125], {});
  handles.push(body);
  body = world.createBody({ type: 2 as BodyType, position: [2, 4, 0] });
  runtime.createTransformedHullShape(body, [0.5, 0.25, 0.125], { position: [0, 0.5, 0], rotation: [0, 0, 0.382683, 0.92388] }, [1, 1, 1], {});
  handles.push(body);
  return handles;
}

export function boxHullGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const boxHullBodies: RenderBody[] = [
  { kind: "box", size: [1, 0.5, 0.25], position: [0, 2, 0], color: 0x3b82f6 },
  { kind: "box", size: [1, 0.5, 0.25], position: [2, 2, 0], color: 0x22c55e },
];

export const boxHullCamera: RenderSpec["camera"] = { position: [0, 15, 5], target: [0, 0, 0] };

export const dumpSampleName = "Box Hull";
export const dumpSampleId = "geometry/box-hull";
export const dumpNoPhysics = true;
export const dumpGroundSize = boxHullGroundSize;
export const dumpBuildDynamicBodies = buildBoxHullDynamicBodies;
