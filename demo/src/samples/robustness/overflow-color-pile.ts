import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { overflowColorPileBodies, overflowColorPileCamera, overflowColorPileGroundSize } from "./overflow-color-pile-scene";

const half = overflowColorPileGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: overflowColorPileBodies,
  camera: overflowColorPileCamera,
  info: "constraint graph color overflow test",
};

export const overflowColorPileSample = createGenericSample(
  "robustness/overflow-color-pile", "Robustness / Overflow Color Pile", spec,
  () => new Worker(new URL("./overflow-color-pile.worker.ts", import.meta.url), { type: "module" }),
);
