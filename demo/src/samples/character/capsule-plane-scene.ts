import {type BodyId, type Box3DRuntime, type PhysicsWorld, type PlaneResult, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

export const CAPSULE_PLANE_CAPACITY = 3;
export const CAPSULE_PLANE_HEADER_FLOATS = 4; // origin xyz + count
export const CAPSULE_PLANE_STRIDE_FLOATS = 7; // n xyz, offset, p xyz

export const capsulePlaneCapsule = {
  center1: [0, -0.5, 0] as Vec3,
  center2: [0, 0.5, 0] as Vec3,
  radius: 0.25,
};

export const capsulePlaneDefaultOrigin: Vec3 = [0, 1, 0.4];

export function buildCapsulePlaneDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const body = world.createBody({ position: [0, 1, 1] });
  runtime.createHullShape(body, [0.5, 0.5, 0.5]);
  return [body];
}

export function collectCapsulePlane(world: PhysicsWorld, origin: Vec3): PlaneResult[] {
  return world.collideMover(origin, capsulePlaneCapsule, CAPSULE_PLANE_CAPACITY);
}

export function solveCapsulePlane(world: PhysicsWorld, origin: Vec3): Vec3 {
  const planes = collectCapsulePlane(world, origin).map((result) => ({
    plane: result.plane,
    clipVelocity: true,
  }));
  const solved = world.solvePlanes([0, 0, 0], planes);
  return [origin[0] + solved.delta[0], origin[1] + solved.delta[1], origin[2] + solved.delta[2]];
}

export function writeCapsulePlaneBuffer(origin: Vec3, planes: readonly PlaneResult[], out: Float32Array): void {
  out[0] = origin[0];
  out[1] = origin[1];
  out[2] = origin[2];
  out[3] = planes.length;
  for (let i = 0; i < planes.length; i++) {
    const base = CAPSULE_PLANE_HEADER_FLOATS + i * CAPSULE_PLANE_STRIDE_FLOATS;
    const plane = planes[i]!;
    out[base] = plane.plane.normal[0];
    out[base + 1] = plane.plane.normal[1];
    out[base + 2] = plane.plane.normal[2];
    out[base + 3] = plane.plane.offset;
    out[base + 4] = plane.point[0];
    out[base + 5] = plane.point[1];
    out[base + 6] = plane.point[2];
  }
}

export function createCapsulePlane(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildCapsulePlaneDynamicBodies(world, runtime) };
}

export function dumpCheckpointExtrasCapsulePlane(
  world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly BodyId[],
): Record<string, unknown> {
  const planes = collectCapsulePlane(world, capsulePlaneDefaultOrigin).map((result) => ({
    n: result.plane.normal,
    o: result.plane.offset,
    p: result.point,
  }));
  return { planes };
}

export function capsulePlaneGroundSize(): Vec3 {
  return [5, 0.5, 5];
}

export const capsulePlaneBodies: RenderBody[] = [
  { kind: "box", size: [1, 1, 1], position: [0, 1, 1], color: 0x3b82f6 },
];

export const capsulePlaneCamera: RenderSpec["camera"] = cameraFromSetView(120, 30, 20, [0, 1.5, 0]);

export const dumpSampleName = "CapsulePlane";
export const dumpSampleId = "character/capsule-plane";
export const dumpCppSampleName = "CapsulePlane";
export const dumpCreate = createCapsulePlane;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointExtrasCapsulePlane;
