import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createHighResistanceBodies, highResistanceCamera, highResistanceGroundSize } from "./high-resistance-scene";

const half = highResistanceGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createHighResistanceBodies(),
  camera: highResistanceCamera,
};

export const highResistanceSample = createGenericSample(
  "shapes/high-resistance", "Shapes / High Resistance", spec,
  () => new Worker(new URL("./high-resistance.worker.ts", import.meta.url), { type: "module" }),
);
