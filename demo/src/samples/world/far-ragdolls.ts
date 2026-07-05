import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [40, 2, 40],
  bodies: [],
  camera: { position: [1180, 30, 20], target: [1000, 0, 0] },
  info: "20 ragdolls 1000km from origin",
};

export const farRagdollsSample = createGenericSample(
  "world/far-ragdolls", "World / Far Ragdolls", spec,
  () => new Worker(new URL("./far-ragdolls.worker.ts", import.meta.url), { type: "module" }),
);
