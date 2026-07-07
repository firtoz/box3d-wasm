import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createFallingBoxesBodies, fallingBoxesCamera, fallingBoxesGroundSize } from "./falling-boxes-scene";

const half = fallingBoxesGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createFallingBoxesBodies(),
  camera: fallingBoxesCamera,
};

export const fallingBoxesSample = createGenericSample(
  "benchmark/falling-boxes", "Benchmark / Falling Boxes", spec,
  () => new Worker(new URL("./falling-boxes.worker.ts", import.meta.url), { type: "module" }),
);
