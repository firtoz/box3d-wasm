import { createGenericSample } from "../generic-host";
import type { RenderBody, RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [40, 2, 40],
  bodies: (() => {
    const bodies: RenderBody[] = [];
    for (let i = 0, y = 0.75; i < 20; i++, y += 1) bodies.push({ kind: "capsule", radius: 0.5, length: 2, position: [0, y, 0], color: 0x38bdf8 });
    return bodies;
  })(),
  camera: { position: [0, 22.94, 48.30], target: [0, 10, 0] },
  info: "20 capsules, 2D-stacked (Z-locked, rotation locked)",
};

export const capsuleStackSample = createGenericSample("capsule-stack", "Stacking / Capsule Stack", spec, () => new Worker(new URL("./stack.worker.ts", import.meta.url), { type: "module" }));
