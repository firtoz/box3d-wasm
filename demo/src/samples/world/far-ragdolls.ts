import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { farRagdollsBodies, farRagdollsCamera, farRagdollsGroundPosition, farRagdollsGroundSize } from "./far-ragdolls-scene";

const half = farRagdollsGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundPosition: farRagdollsGroundPosition,
  groundKind: "plane",
  bodies: farRagdollsBodies,
  camera: farRagdollsCamera,
  info: "20 ragdolls 1000km from origin",
};

export const farRagdollsSample = createGenericSample(
  "world/far-ragdolls", "World / Far Ragdolls", spec,
  () => new Worker(new URL("./far-ragdolls.worker.ts", import.meta.url), { type: "module" }),
);
