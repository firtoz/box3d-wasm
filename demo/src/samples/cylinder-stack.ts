import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import { cylinderStackBodies, cylinderStackGroundSize } from "./cylinder-stack-scene";

const half = cylinderStackGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: cylinderStackBodies,
};

export const cylinderStackSample = createGenericSample("cylinder-stack", "Stacking / Cylinder Stack", spec, () => new Worker(new URL("./cylinder-stack.worker.ts", import.meta.url), { type: "module" }));
