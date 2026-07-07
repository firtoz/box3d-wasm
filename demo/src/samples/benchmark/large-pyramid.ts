import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createLargePyramidBodies, largePyramidCamera, largePyramidGroundSize } from "./large-pyramid-scene";

const half = largePyramidGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createLargePyramidBodies(),
  camera: largePyramidCamera,
  info: "90-base pyramid (4095 boxes), sleeping disabled",
};

export const largePyramidSample = createGenericSample(
  "benchmark/large-pyramid", "Benchmark / Large Pyramid", spec,
  () => new Worker(new URL("./large-pyramid.worker.ts", import.meta.url), { type: "module" }),
);
