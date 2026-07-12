import { createShaderInstancedSample } from "../shader-instanced-host";
import {
  FALLING_BOXES_BOX_COLOR,
  FALLING_BOXES_BOX_COUNT,
  FALLING_BOXES_BOX_SIZE,
  fallingBoxesCamera,
  fallingBoxesGroundSize,
  forEachFallingBox,
} from "./falling-boxes-scene";

const half = fallingBoxesGroundSize();

export const fallingBoxesSample = createShaderInstancedSample({
  id: "benchmark/falling-boxes",
  name: "Benchmark / Falling Boxes",
  createWorker: () => new Worker(new URL("./falling-boxes.worker.ts", import.meta.url), { type: "module" }),
  instanceCount: FALLING_BOXES_BOX_COUNT,
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  camera: fallingBoxesCamera!,
  shape: { kind: "box", size: FALLING_BOXES_BOX_SIZE },
  defaultColor: FALLING_BOXES_BOX_COLOR,
  forEachInstance: (callback) => {
    forEachFallingBox((position) => callback(position, FALLING_BOXES_BOX_COLOR));
  },
  info: ({ workerCount, colorMode }) =>
    `${FALLING_BOXES_BOX_COUNT} boxes (50×8×8) | shader render | ${workerCount} workers | ${colorMode} colors (C)`,
});
