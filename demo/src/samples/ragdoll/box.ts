import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createRagdollBoxBodies, ragdollBoxCamera, ragdollBoxGroundSize } from "./box-scene";

const half = ragdollBoxGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createRagdollBoxBodies(),
  camera: ragdollBoxCamera,
};

export const ragdollBoxSample = createGenericSample(
  "ragdoll/box", "Ragdoll / Box", spec,
  () => new Worker(new URL("./box.worker.ts", import.meta.url), { type: "module" }),
);
