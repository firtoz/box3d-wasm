import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32 } from "../f32";

export function buildOverlapRecoveryDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const baseCount = 4;
  const extent = f32(0.5);
  const overlap = f32(0.25);
  const fraction = f32(1 - overlap);
  const half: Vec3 = [extent, extent, extent];

  world.setContactTuning(30, 10, 3);

  let y = extent;
  for (let i = 0; i < baseCount; i++) {
    let x = f32(fraction * extent * f32(i - baseCount));
    for (let j = i; j < baseCount; j++) {
      const body = world.createBody({ type: BodyType.Dynamic, position: [x, y, 0] });
      runtime.createHullShape(body, half, { density: 1 });
      handles.push(body);
      x = f32(x + f32(2 * fraction * extent));
    }
    y = f32(y + f32(2 * fraction * extent));
  }

  return handles;
}

export function overlapRecoveryGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const overlapRecoveryBodies: RenderBody[] = (() => {
  const bodies: RenderBody[] = [];
  const baseCount = 4;
  const extent = 0.5;
  const overlap = 0.25;
  const fraction = 1 - overlap;

  let y = extent;
  for (let i = 0; i < baseCount; i++) {
    let x = fraction * extent * (i - baseCount);
    for (let j = i; j < baseCount; j++) {
      bodies.push({
        kind: "box",
        size: [1, 1, 1],
        position: [x, y, 0],
        color: 0x60a5fa,
      });
      x += 2 * fraction * extent;
    }
    y += 2 * fraction * extent;
  }

  return bodies;
})();

export const overlapRecoveryCamera: RenderSpec["camera"] = { position: [45, 20, 15], target: [0, 0, 0] };

export const dumpSampleName = "Overlap Recovery";
export const dumpSampleId = "robustness/overlap-recovery";
export const dumpCppSampleName = "Overlap Recovery";
export const dumpGroundSize = overlapRecoveryGroundSize;
export const dumpBuildDynamicBodies = buildOverlapRecoveryDynamicBodies;
