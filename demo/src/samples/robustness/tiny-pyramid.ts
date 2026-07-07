import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { tinyPyramidBodies, tinyPyramidCamera, tinyPyramidGroundSize } from "./tiny-pyramid-scene";

const half = tinyPyramidGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: tinyPyramidBodies,
  camera: tinyPyramidCamera,
  info: "tiny 2.5cm box pyramid",
};

export const tinyPyramidSample = createGenericSample(
  "robustness/tiny-pyramid", "Robustness / Tiny Pyramid", spec,
  () => new Worker(new URL("./tiny-pyramid.worker.ts", import.meta.url), { type: "module" }),
);
