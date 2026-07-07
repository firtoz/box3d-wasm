import type { Box3DRuntime, HumanHandle, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildFarRagdollsDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const offset = 1000000;
  const count = 20;

  for (let i = 0; i < count; i++) {
    const x = 0.15 * (i - 0.5 * count) + offset;
    const y = 2 + 0.25 * i;
    const z = 0.15 * (0.5 * count - i);
    const human = world.createHuman(
      [x, y, z],
      { frictionTorque: 10, hertz: 0.5, dampingRatio: 0.7, groupIndex: i },
    );
    const boneCount = runtime.getHumanBoneCount();
    for (let j = 0; j < boneCount; j++) {
      const boneBody = runtime.getHumanBoneBody(human, j);
      if (boneBody !== 0) handles.push(boneBody);
    }
  }

  return handles;
}

export function farRagdollsGroundSize(): Vec3 {
  return [10, 0.5, 10];
}

export const farRagdollsBodies: RenderBody[] = [];

export const farRagdollsCamera: RenderSpec["camera"] = { position: [1180, 30, 20], target: [1000, 0, 0] };

export const dumpSampleName = "Far Ragdolls";
export const dumpSampleId = "world/far-ragdolls";
export const dumpCppSampleName = "Far Ragdolls";
export const dumpGroundSize = farRagdollsGroundSize;
export const dumpGroundPosition: Vec3 = [1000000, -1, 0];
export const dumpBuildDynamicBodies = buildFarRagdollsDynamicBodies;
