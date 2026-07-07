import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { highMassRatio1Bodies, highMassRatio1Camera, highMassRatio1GroundSize } from "./high-mass-ratio-1-scene";

const half = highMassRatio1GroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: highMassRatio1Bodies,
  camera: highMassRatio1Camera,
  info: "3 high-mass-ratio pyramid stacks",
};

export const highMassRatio1Sample = createGenericSample(
  "robustness/high-mass-ratio-1", "Robustness / HighMassRatio1", spec,
  () => new Worker(new URL("./high-mass-ratio-1.worker.ts", import.meta.url), { type: "module" }),
);
