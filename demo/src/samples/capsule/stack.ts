import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createCapsuleStackBodies, capsuleStackCamera, capsuleStackGroundSize } from "./stack-scene";

const half = capsuleStackGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createCapsuleStackBodies(),
  camera: capsuleStackCamera,
  info: "20 capsules, 2D-stacked (Z-locked, rotation locked)",
};

export const capsuleStackSample = createGenericSample("capsule-stack", "Stacking / Capsule Stack", spec, () => new Worker(new URL("./stack.worker.ts", import.meta.url), { type: "module" }));
