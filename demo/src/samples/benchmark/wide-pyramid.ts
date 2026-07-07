import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createWidePyramidBodies, widePyramidCamera, widePyramidGroundSize } from "./wide-pyramid-scene";

const half = widePyramidGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createWidePyramidBodies(),
  camera: widePyramidCamera,
};

export const widePyramidSample = createGenericSample(
  "benchmark/wide-pyramid", "Benchmark / Wide Pyramid", spec,
  () => new Worker(new URL("./wide-pyramid.worker.ts", import.meta.url), { type: "module" }),
);
