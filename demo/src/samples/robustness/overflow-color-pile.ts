import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [50, 2, 50],
  bodies: [],
  camera: { position: [30, 35, 15], target: [0, 0, 0] },
  info: "constraint graph color overflow test",
};

export const overflowColorPileSample = createGenericSample(
  "robustness/overflow-color-pile", "Robustness / Overflow Color Pile", spec,
  () => new Worker(new URL("./overflow-color-pile.worker.ts", import.meta.url), { type: "module" }),
);
