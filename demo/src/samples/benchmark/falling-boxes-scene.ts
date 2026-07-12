import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const N = 50;
const a = 0.5;

export const FALLING_BOXES_BOX_COUNT = N * 8 * 8;
export const FALLING_BOXES_BOX_COLOR = 0x60a5fa;
export const FALLING_BOXES_BOX_SIZE = 2 * a;

export function forEachFallingBox(callback: (position: Vec3) => void): void {
  for (let i = 0; i < N; i++) {
    for (let j = 0; j < 8; j++) {
      for (let k = 0; k < 8; k++) {
        const x = -16 * a + 4 * a * j;
        const y = 4 * a * i + 5 * a;
        const z = -16 * a + 4 * a * k;
        callback([x, y, z]);
      }
    }
  }
}

export function buildFallingBoxesDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const half: Vec3 = [a, a, a];

  forEachFallingBox(([x, y, z]) => {
    const body = world.createBody({ type: BodyType.Dynamic, position: [x, y, z] });
    runtime.createHullShape(body, half, {});
    handles.push(body);
  });

  return handles;
}

export function fallingBoxesGroundSize(): Vec3 { return [100, 1, 100]; }

export function createFallingBoxesBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  forEachFallingBox((position) => {
    bodies.push({ kind: "box", size: [FALLING_BOXES_BOX_SIZE, FALLING_BOXES_BOX_SIZE, FALLING_BOXES_BOX_SIZE], position, color: FALLING_BOXES_BOX_COLOR });
  });
  return bodies;
}

export const fallingBoxesCamera: RenderSpec["camera"] = { position: [45, 10, 80], target: [0, 20, 0] };

export const dumpSampleName = "Falling Boxes";
export const dumpSampleId = "benchmark/falling-boxes";
export const dumpCppSampleName = "Falling Boxes";
export const dumpGroundSize = fallingBoxesGroundSize;
export const dumpBuildDynamicBodies = buildFallingBoxesDynamicBodies;
