import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [{ kind: "cylinder", radius: 0.25, height: 1, position: [0, 2, 0], color: 0x38bdf8 }],
};

export const cylinderSample = createGenericSample("cylinder", "Stacking / Cylinder", spec, () => new Worker(new URL("./cylinder.worker.ts", import.meta.url), { type: "module" }));
