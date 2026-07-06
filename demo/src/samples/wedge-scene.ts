import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "./generic-host";

export const wedgePoints: Vec3[] = [
  [-1.0, 1.0, -0.1],
  [1.0, 1.0, -0.1],
  [-1.0, 1.0, 0.1],
  [1.0, 1.0, 0.1],
  [-0.5, 0.5, 0.0],
  [0.5, 0.5, 0.0],
];

export function buildWedgeDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const hull = runtime.createHullFromPoints(wedgePoints.flat());
  const body = world.createBody({ type: BodyType.Dynamic, position: [0, 1, 0] });
  runtime.createShapeFromHull(body, hull);
  runtime.destroyHull(hull);
  return [body];
}

export function wedgeGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const wedgeBodies: RenderBody[] = [
  { kind: "hull", points: wedgePoints, position: [0, 1, 0], color: 0x38bdf8, type: BodyType.Dynamic },
];

export const wedgeCamera: RenderSpec["camera"] = { position: [75, 10, 10], target: [0, 0, 0] };

export const dumpSampleName = "Wedge";
export const dumpSampleId = "wedge";
export const dumpCppSampleName = "Wedge";
export const dumpGroundSize = wedgeGroundSize;
export const dumpBuildDynamicBodies = buildWedgeDynamicBodies;
