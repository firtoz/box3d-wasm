import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { ragdollRenderBodies } from "../ragdoll/ragdoll-scene-shared";
import { f32 } from "../f32";
import { fallingRagdollsCamera, fallingRagdollsGroundSize } from "./falling-ragdolls-scene";

const GRID_COUNT = 2;
const GRID_SIZE = f32(15);

function createFallingRagdollsBodies() {
  const bodies = [];
  const span = f32(GRID_COUNT * GRID_SIZE);
  const groupDistance = f32(span / GRID_COUNT);
  for (let i = 0; i < GRID_COUNT; i++) {
    for (let j = 0; j < GRID_COUNT; j++) {
      let x = f32(f32(-0.5) * span + f32(groupDistance * f32(j + 0.5)));
      const z = f32(f32(-0.5) * span + f32(groupDistance * f32(i + 0.5)));
      for (let k = 0; k < 2; k++) {
        bodies.push(...ragdollRenderBodies([x, 15, z]));
        x = f32(x + 0.75);
      }
    }
  }
  return bodies;
}

const half = fallingRagdollsGroundSize();
const spec: RenderSpec = {
  groundKind: "plane",
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createFallingRagdollsBodies(),
  camera: fallingRagdollsCamera,
};

export const fallingRagdollsSample = createGenericSample(
  "determinism/falling-ragdolls", "Determinism / Falling Ragdolls", spec,
  () => new Worker(new URL("./falling-ragdolls.worker.ts", import.meta.url), { type: "module" }),
);
