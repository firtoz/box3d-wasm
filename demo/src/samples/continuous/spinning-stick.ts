import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createSpinningStickBodies, spinningStickCamera, spinningStickGroundSize } from "./spinning-stick-scene";

const half = spinningStickGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createSpinningStickBodies(),
  camera: spinningStickCamera,
};

export const spinningStickSample = createGenericSample(
  "continuous/spinning-stick", "Continuous / Spinning Stick", spec,
  () => new Worker(new URL("./spinning-stick.worker.ts", import.meta.url), { type: "module" }),
);
