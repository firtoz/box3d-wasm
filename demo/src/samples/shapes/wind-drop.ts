import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createWindDropBodies, windDropCamera, windDropGroundSize } from "./wind-drop-scene";

const half = windDropGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createWindDropBodies(),
  camera: windDropCamera,
};

export const windDropSample = createGenericSample(
  "shapes/wind-drop", "Shapes / Wind Drop", spec,
  () => new Worker(new URL("./wind-drop.worker.ts", import.meta.url), { type: "module" }),
);
