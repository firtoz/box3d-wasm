import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const N = 50;
const a = 0.5;

export function buildFallingBoxesDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const half: Vec3 = [a, a, a];

  for (let i = 0; i < N; i++) {
    for (let j = 0; j < 8; j++) {
      for (let k = 0; k < 8; k++) {
        const x = -16 * a + 4 * a * j;
        const y = 4 * a * i + 5 * a;
        const z = -16 * a + 4 * a * k;
        const body = world.createBody({ type: BodyType.Dynamic, position: [x, y, z] });
        runtime.createHullShape(body, half, {});
        handles.push(body);
      }
    }
  }

  return handles;
}

export function fallingBoxesGroundSize(): Vec3 { return [100, 1, 100]; }

export function createFallingBoxesBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < N; i++) {
    for (let j = 0; j < 8; j++) {
      for (let k = 0; k < 8; k++) {
        const x = -16 * a + 4 * a * j;
        const y = 4 * a * i + 5 * a;
        const z = -16 * a + 4 * a * k;
        bodies.push({ kind: "box", size: [2 * a, 2 * a, 2 * a], position: [x, y, z], color: 0x60a5fa });
      }
    }
  }
  return bodies;
}

export const fallingBoxesCamera: RenderSpec["camera"] = { position: [45, 10, 80], target: [0, 20, 0] };

export const dumpSampleName = "Falling Boxes";
export const dumpSampleId = "benchmark/falling-boxes";
export const dumpCppSampleName = "Falling Boxes";
export const dumpGroundSize = fallingBoxesGroundSize;
export const dumpBuildDynamicBodies = buildFallingBoxesDynamicBodies;
