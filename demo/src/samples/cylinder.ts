import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import { cylinderBodies, cylinderGroundSize } from "./cylinder-scene";

const half = cylinderGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: cylinderBodies,
};

export const cylinderSample = createGenericSample("cylinder", "Stacking / Cylinder", spec, () => new Worker(new URL("./cylinder.worker.ts", import.meta.url), { type: "module" }));
