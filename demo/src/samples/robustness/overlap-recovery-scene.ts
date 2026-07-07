import type { BodyType, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildOverlapRecoveryDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const baseCount = 4;
  const extent = 0.5;
  const overlap = 0.25;
  const fraction = 1 - overlap;
  const half: Vec3 = [extent, extent, extent];

  world.setContactTuning(30, 10, 3);

  let y = extent;
  for (let i = 0; i < baseCount; i++) {
    let x = fraction * extent * (i - baseCount);
    for (let j = i; j < baseCount; j++) {
      const body = world.createBody({ type: 2 as BodyType, position: [x, y, 0] });
      runtime.createHullShape(body, half, {});
      handles.push(body);
      x += 2 * fraction * extent;
    }
    y += 2 * fraction * extent;
  }

  return handles;
}

export function overlapRecoveryGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const overlapRecoveryBodies: RenderBody[] = [];

export const overlapRecoveryCamera: RenderSpec["camera"] = { position: [45, 20, 15], target: [0, 0, 0] };

export const dumpSampleName = "Overlap Recovery";
export const dumpSampleId = "robustness/overlap-recovery";
export const dumpCppSampleName = "Overlap Recovery";
export const dumpGroundSize = overlapRecoveryGroundSize;
export const dumpBuildDynamicBodies = buildOverlapRecoveryDynamicBodies;
