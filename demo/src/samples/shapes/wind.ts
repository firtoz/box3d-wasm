import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createWindBodies, windCamera, windGroundSize } from "./wind-scene";

const half = windGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createWindBodies(),
  camera: windCamera,
};

export const windSample = createGenericSample(
  "shapes/wind", "Shapes / Wind", spec,
  () => new Worker(new URL("./wind.worker.ts", import.meta.url), { type: "module" }),
);
