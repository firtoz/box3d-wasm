import { createShaderInstancedSample } from "../shader-instanced-host";
import {
  forEachLargePyramidBox,
  LARGE_PYRAMID_BOX_COLOR,
  LARGE_PYRAMID_BOX_COUNT,
  largePyramidCamera,
  largePyramidGroundSize,
} from "./large-pyramid-scene";

const half = largePyramidGroundSize();

export const largePyramidSample = createShaderInstancedSample({
  id: "benchmark/large-pyramid",
  name: "Benchmark / Large Pyramid",
  createWorker: () => new Worker(new URL("./large-pyramid.worker.ts", import.meta.url), { type: "module" }),
  instanceCount: LARGE_PYRAMID_BOX_COUNT,
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  camera: largePyramidCamera!,
  shape: { kind: "box", size: 1 },
  defaultColor: LARGE_PYRAMID_BOX_COLOR,
  forEachInstance: (callback) => {
    forEachLargePyramidBox((position) => callback(position, LARGE_PYRAMID_BOX_COLOR));
  },
  // CreateLargePyramid only disables sleeping; continuous stays the world default (on).
  info: ({ workerCount, colorMode }) =>
    `100-base pyramid (${LARGE_PYRAMID_BOX_COUNT} boxes) | shader render | ${workerCount} workers | ${colorMode} colors (C) | Sleep toggle for awake/sleep`,
});
