import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createRagdollInclineBodies, ragdollInclineCamera, ragdollInclineGroundSize } from "./incline-scene";

const half = ragdollInclineGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createRagdollInclineBodies(),
  camera: ragdollInclineCamera,
};

export const ragdollInclineSample = createGenericSample(
  "ragdoll/incline", "Ragdoll / Incline", spec,
  () => new Worker(new URL("./incline.worker.ts", import.meta.url), { type: "module" }),
);
