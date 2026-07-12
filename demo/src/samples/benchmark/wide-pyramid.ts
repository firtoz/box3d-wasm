import { createShaderInstancedSample } from "../shader-instanced-host";
import {
  forEachWidePyramidBox,
  WIDE_PYRAMID_BOX_COLOR,
  WIDE_PYRAMID_BOX_COUNT,
  WIDE_PYRAMID_BOX_SIZE,
  widePyramidCamera,
  widePyramidGroundSize,
} from "./wide-pyramid-scene";

const half = widePyramidGroundSize();

export const widePyramidSample = createShaderInstancedSample({
  id: "benchmark/wide-pyramid",
  name: "Benchmark / Wide Pyramid",
  createWorker: () => new Worker(new URL("./wide-pyramid.worker.ts", import.meta.url), { type: "module" }),
  instanceCount: WIDE_PYRAMID_BOX_COUNT,
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  camera: widePyramidCamera!,
  shape: { kind: "box", size: WIDE_PYRAMID_BOX_SIZE },
  defaultColor: WIDE_PYRAMID_BOX_COLOR,
  forEachInstance: (callback) => {
    forEachWidePyramidBox((position) => callback(position, WIDE_PYRAMID_BOX_COLOR));
  },
  info: ({ workerCount, colorMode }) =>
    `15-layer pyramid (${WIDE_PYRAMID_BOX_COUNT} boxes) | shader render | ${workerCount} workers | ${colorMode} colors (C)`,
});
