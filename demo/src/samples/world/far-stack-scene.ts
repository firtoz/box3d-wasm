import type { BodyType, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildFarStackDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const offset = 0;
  const half: Vec3 = [0.5, 0.5, 0.5];

  for (let i = 0; i < 6; i++) {
    const skew = 0.02 * (i & 1 ? 1 : -1);
    const body = world.createBody({
      type: 2 as BodyType,
      position: [offset + skew, 0.5 + 1 * i, 0],
    });
    runtime.createHullShape(body, half, {});
    handles.push(body);
  }

  return handles;
}

export function farStackGroundSize(): Vec3 {
  return [12, 1, 12];
}

export function createFarStackBodies(): RenderBody[] {
  return Array.from({ length: 6 }, (_, i) => ({
    kind: "box" as const, size: [1, 1, 1] as [number, number, number],
    position: [0, 0.5 + 1 * i, 0] as [number, number, number],
    color: 0x3b82f6,
  }));
}

export const farStackCamera: RenderSpec["camera"] = { position: [0, 8, 16], target: [0, 2, 0] };

export const dumpSampleName = "Far Stack";
export const dumpSampleId = "world/far-stack";
export const dumpCppSampleName = "Far Stack";
export const dumpGroundSize = farStackGroundSize;
export const dumpBuildDynamicBodies = buildFarStackDynamicBodies;
