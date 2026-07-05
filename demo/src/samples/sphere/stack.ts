import { createGenericSample } from "../generic-host";
import type { RenderBody, RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [30, 2, 30],
  bodies: (() => {
    const bodies: RenderBody[] = [];
    for (let i = 0, y = 0.75; i < 30; i++, y += 1.5) bodies.push({ kind: "sphere", radius: 0.5, position: [0, y, 0], color: 0x38bdf8 });
    return bodies;
  })(),
  camera: { position: [0, 22.94, 48.30], target: [0, 10, 0] },
};

export const sphereStackSample = createGenericSample("sphere-stack", "Stacking / Sphere Stack", spec, () => new Worker(new URL("./stack.worker.ts", import.meta.url), { type: "module" }));
