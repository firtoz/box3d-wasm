import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import { wedgeBodies, wedgeCamera, wedgeGroundSize } from "./wedge-scene";

const half = wedgeGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: wedgeBodies,
  camera: wedgeCamera,
};

export const wedgeSample = createGenericSample("wedge", "Stacking / Wedge", spec, () => new Worker(new URL("./wedge.worker.ts", import.meta.url), { type: "module" }));
