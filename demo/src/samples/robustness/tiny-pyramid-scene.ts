import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const f32 = Math.fround;
const EXTENT_F32 = f32(0.025);
const BASE_COUNT = 30;
const BASE_COUNT_EXTENT_F32 = f32(BASE_COUNT * EXTENT_F32);

export function buildTinyPyramidDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const half: Vec3 = [EXTENT_F32, EXTENT_F32, EXTENT_F32];

  for (let i = 0; i < BASE_COUNT; i++) {
    // Matches C++ float32: (2.0f * i + 1.0f) * m_extent
    const y = f32(f32(2 * i + 1) * EXTENT_F32);
    // Precompute t1 per row: (i + 1.0f) * m_extent
    const t1 = f32(f32(i + 1) * EXTENT_F32);

    for (let j = i; j < BASE_COUNT; j++) {
      // Matches C++ float32: (i + 1.0f) * extent + 2.0f * (j - i) * extent - baseCount * extent
      // Each operation independently rounded to float32 via f32()
      const t2 = f32(2 * f32(j - i) * EXTENT_F32);
      const x = f32(f32(t1 + t2) - BASE_COUNT_EXTENT_F32);
      const body = world.createBody({ type: BodyType.Dynamic, position: [x, y, 0] });
      runtime.createHullShape(body, half, {});
      handles.push(body);
    }
  }

  return handles;
}

export function tinyPyramidGroundSize(): Vec3 {
  return [20, 1, 20];
}

// 465 tiny boxes at 2.5cm half-extent
export const tinyPyramidBodies: RenderBody[] = (() => {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < BASE_COUNT; i++) {
    const y = (2 * i + 1) * 0.025;
    for (let j = i; j < BASE_COUNT; j++) {
      const x = (i + 1) * 0.025 + 2 * (j - i) * 0.025 - BASE_COUNT * 0.025;
      bodies.push({
        kind: "box" as const,
        size: [0.05, 0.05, 0.05] as [number, number, number],
        position: [x, y, 0] as [number, number, number],
        color: 0x88ccff,
      });
    }
  }
  return bodies;
})();

export const tinyPyramidCamera: RenderSpec["camera"] = { position: [-30, 20, 10], target: [0, 0.5, 0] };

export const dumpSampleName = "Tiny Pyramid";
export const dumpSampleId = "robustness/tiny-pyramid";
export const dumpCppSampleName = "Tiny Pyramid";
export const dumpGroundSize = tinyPyramidGroundSize;
export const dumpBuildDynamicBodies = buildTinyPyramidDynamicBodies;
