import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import { createPyramid2dBodies, pyramid2dGroundSize } from "./pyramid2d-scene";

const half = pyramid2dGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createPyramid2dBodies(),
  info: "12 rows, 2D stacking (Z-locked)",
};

export const pyramid2dSample = createGenericSample("pyramid2d", "Stacking / Pyramid2D", spec, () => new Worker(new URL("./pyramid2d.worker.ts", import.meta.url), { type: "module" }));
