import {type BodyId, type Box3DRuntime, type PhysicsWorld, type PlaneResult, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

export const MOVER_OVERLAP_CAPACITY = 32;
export const MOVER_OVERLAP_HEADER_FLOATS = 8; // origin xyz, count, zeroNormals, solved xyz
export const MOVER_OVERLAP_STRIDE_FLOATS = 8; // n xyz, offset, p xyz, valid

export const moverOverlapCapsule = {
  center1: [0, -0.5, 0] as Vec3,
  center2: [0, 0.5, 0] as Vec3,
  radius: 0.35,
};

export const moverOverlapDefaultOrigin: Vec3 = [0, 3.5, 0];

export function buildMoverOverlapDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const sphere = world.createBody({ position: [-3, 1, 0] });
  runtime.createSphereShape(sphere, [0, 0, 0], 0.6);

  const capsule = world.createBody({ position: [0, 1, 0] });
  runtime.createCapsuleShape(capsule, [0, 0, -0.7], [0, 0, 0.7], 0.4);

  const box = world.createBody({ position: [3, 1, 0] });
  runtime.createHullShape(box, [0.6, 0.6, 0.6]);

  return [sphere, capsule, box];
}

export function collectMoverOverlap(world: PhysicsWorld, origin: Vec3): PlaneResult[] {
  return world.collideMover(origin, moverOverlapCapsule, MOVER_OVERLAP_CAPACITY);
}

export function solveMoverOverlap(world: PhysicsWorld, origin: Vec3, planes: readonly PlaneResult[]): Vec3 {
  const solved = world.solvePlanes(
    [0, 0, 0],
    planes.map((result) => ({ plane: result.plane, clipVelocity: true })),
  );
  return [origin[0] + solved.delta[0], origin[1] + solved.delta[1], origin[2] + solved.delta[2]];
}

function isNormalized(normal: Vec3): boolean {
  const len = Math.hypot(normal[0], normal[1], normal[2]);
  return Math.abs(len - 1) < 1e-3;
}

export function writeMoverOverlapBuffer(origin: Vec3, planes: readonly PlaneResult[], solved: Vec3, out: Float32Array): void {
  let zeroNormals = 0;
  out[0] = origin[0];
  out[1] = origin[1];
  out[2] = origin[2];
  out[3] = planes.length;
  out[5] = solved[0];
  out[6] = solved[1];
  out[7] = solved[2];
  for (let i = 0; i < planes.length; i++) {
    const plane = planes[i]!;
    const valid = isNormalized(plane.plane.normal);
    if (!valid) zeroNormals += 1;
    const base = MOVER_OVERLAP_HEADER_FLOATS + i * MOVER_OVERLAP_STRIDE_FLOATS;
    out[base] = plane.plane.normal[0];
    out[base + 1] = plane.plane.normal[1];
    out[base + 2] = plane.plane.normal[2];
    out[base + 3] = plane.plane.offset;
    out[base + 4] = plane.point[0];
    out[base + 5] = plane.point[1];
    out[base + 6] = plane.point[2];
    out[base + 7] = valid ? 1 : 0;
  }
  out[4] = zeroNormals;
}

export function createMoverOverlap(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildMoverOverlapDynamicBodies(world, runtime) };
}

export function dumpCheckpointExtrasMoverOverlap(
  world: PhysicsWorld,
): Record<string, unknown> {
  const planes = collectMoverOverlap(world, moverOverlapDefaultOrigin).map((result) => ({
    n: result.plane.normal,
    o: result.plane.offset,
    p: result.point,
  }));
  return { planes };
}

export function moverOverlapGroundSize(): Vec3 {
  return [6, 0.5, 6];
}

export const moverOverlapBodies: RenderBody[] = [
  { kind: "sphere", radius: 0.6, position: [-3, 1, 0], color: 0x3b82f6 },
  { kind: "capsule", radius: 0.4, length: 1.4, axis: "z", position: [0, 1, 0], color: 0x22c55e },
  { kind: "box", size: [1.2, 1.2, 1.2], position: [3, 1, 0], color: 0xef4444 },
];

export const moverOverlapCamera: RenderSpec["camera"] = cameraFromSetView(120, 25, 10, [0, 1, 0]);

export const dumpSampleName = "MoverOverlap";
export const dumpSampleId = "character/mover-overlap";
export const dumpCppSampleName = "MoverOverlap";
export const dumpCreate = createMoverOverlap;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointExtrasMoverOverlap;
