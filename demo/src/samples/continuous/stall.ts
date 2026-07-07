import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { stallBodies, stallCamera, stallGroundSize } from "./stall-scene";

const half = stallGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: stallBodies,
  camera: stallCamera,
};

export const stallSample = createGenericSample(
  "continuous/stall", "Continuous / Stall", spec,
  () => new Worker(new URL("./stall.worker.ts", import.meta.url), { type: "module" }),
);
