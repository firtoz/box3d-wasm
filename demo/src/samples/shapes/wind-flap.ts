import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createWindFlapBodies, windFlapCamera, windFlapGroundSize } from "./wind-flap-scene";

const half = windFlapGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createWindFlapBodies(),
  camera: windFlapCamera,
};

export const windFlapSample = createGenericSample(
  "shapes/wind-flap", "Shapes / Wind Flap", spec,
  () => new Worker(new URL("./wind-flap.worker.ts", import.meta.url), { type: "module" }),
);
