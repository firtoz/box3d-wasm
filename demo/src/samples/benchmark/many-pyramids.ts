import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createManyPyramidsBodies, manyPyramidsCamera, manyPyramidsGroundSize } from "./many-pyramids-scene";

const half = manyPyramidsGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createManyPyramidsBodies(),
  camera: manyPyramidsCamera,
};

export const manyPyramidsSample = createGenericSample(
  "benchmark/many-pyramids", "Benchmark / Many Pyramids", spec,
  () => new Worker(new URL("./many-pyramids.worker.ts", import.meta.url), { type: "module" }),
);
