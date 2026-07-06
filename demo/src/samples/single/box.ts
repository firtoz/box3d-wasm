import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { singleBoxBodies, singleBoxCamera, singleBoxGroundSize } from "./box-scene";

const half = singleBoxGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: singleBoxBodies,
  camera: singleBoxCamera,
};

export const singleBoxSample = createGenericSample("single-box", "Stacking / Single Box", spec, () => new Worker(new URL("./box.worker.ts", import.meta.url), { type: "module" }));
