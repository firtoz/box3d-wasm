import { createShaderInstancedSample } from "../shader-instanced-host";
import {
  forEachManyPyramidsBox,
  MANY_PYRAMIDS_BOX_COLOR,
  MANY_PYRAMIDS_BOX_COUNT,
  manyPyramidsCamera,
  manyPyramidsGroundSize,
} from "./many-pyramids-scene";

const half = manyPyramidsGroundSize();

export const manyPyramidsSample = createShaderInstancedSample({
  id: "benchmark/many-pyramids",
  name: "Benchmark / Many Pyramids",
  createWorker: () => new Worker(new URL("./many-pyramids.worker.ts", import.meta.url), { type: "module" }),
  instanceCount: MANY_PYRAMIDS_BOX_COUNT,
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  camera: manyPyramidsCamera!,
  shape: { kind: "box", size: 1 },
  defaultColor: MANY_PYRAMIDS_BOX_COLOR,
  forEachInstance: (callback) => {
    forEachManyPyramidsBox((position) => callback(position, MANY_PYRAMIDS_BOX_COLOR));
  },
  info: ({ workerCount, colorMode }) =>
    `${MANY_PYRAMIDS_BOX_COUNT} boxes (14×14 pyramids) | shader render | sleep off | ${workerCount} workers | ${colorMode} colors (C)`,
});
