import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createRagdollPileBodies, ragdollPileCamera, ragdollPileGroundSize } from "./pile-scene";

const half = ragdollPileGroundSize();
const spec: RenderSpec = {
  groundKind: "plane",
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createRagdollPileBodies(),
  camera: ragdollPileCamera,
};

export const ragdollPileSample = createGenericSample(
  "ragdoll/pile", "Ragdoll / Pile", spec,
  () => new Worker(new URL("./pile.worker.ts", import.meta.url), { type: "module" }),
);
