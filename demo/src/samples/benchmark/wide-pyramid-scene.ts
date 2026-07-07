import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const BOX_SIZE = 2.0;
const BOX_SEPARATION = 0.5;
const HALF_BOX_SIZE = 0.5 * BOX_SIZE;
const PYRAMID_HEIGHT = 15;
const h = HALF_BOX_SIZE - 0.025;

export function buildWidePyramidDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const half: Vec3 = [h, h, h];

  for (let i = 0; i < PYRAMID_HEIGHT; i++) {
    const jStart = Math.floor(i / 2);
    const jEnd = PYRAMID_HEIGHT - Math.floor((i + 1) / 2);
    for (let j = jStart; j < jEnd; j++) {
      for (let k = jStart; k < jEnd; k++) {
        const x = -PYRAMID_HEIGHT + BOX_SIZE * j + (i & 1 ? HALF_BOX_SIZE : 0);
        const y = 1 + (BOX_SIZE + BOX_SEPARATION) * i;
        const z = -PYRAMID_HEIGHT + BOX_SIZE * k + (i & 1 ? HALF_BOX_SIZE : 0);
        const body = world.createBody({ type: BodyType.Dynamic, position: [x, y, z] });
        runtime.createHullShape(body, half, {});
        handles.push(body);
      }
    }
  }

  return handles;
}

export function widePyramidGroundSize(): Vec3 { return [100, 1, 100]; }

export function createWidePyramidBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < PYRAMID_HEIGHT; i++) {
    const jStart = Math.floor(i / 2);
    const jEnd = PYRAMID_HEIGHT - Math.floor((i + 1) / 2);
    for (let j = jStart; j < jEnd; j++) {
      for (let k = jStart; k < jEnd; k++) {
        const x = -PYRAMID_HEIGHT + BOX_SIZE * j + (i & 1 ? HALF_BOX_SIZE : 0);
        const y = 1 + (BOX_SIZE + BOX_SEPARATION) * i;
        const z = -PYRAMID_HEIGHT + BOX_SIZE * k + (i & 1 ? HALF_BOX_SIZE : 0);
        bodies.push({ kind: "box", size: [2 * h, 2 * h, 2 * h], position: [x, y, z], color: 0x60a5fa });
      }
    }
  }
  return bodies;
}

export const widePyramidCamera: RenderSpec["camera"] = { position: [0, 5, 80], target: [0, 18, 0] };

export const dumpSampleName = "Wide Pyramid";
export const dumpSampleId = "benchmark/wide-pyramid";
export const dumpCppSampleName = "Wide Pyramid";
export const dumpGroundSize = widePyramidGroundSize;
export const dumpBuildDynamicBodies = buildWidePyramidDynamicBodies;
