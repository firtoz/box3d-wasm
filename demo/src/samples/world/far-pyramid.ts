import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { farPyramidBodies, farPyramidCamera, farPyramidGroundSize } from "./far-pyramid-scene";

const half = farPyramidGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: farPyramidBodies,
  camera: farPyramidCamera,
  info: "40-base pyramid 10000km from origin",
};

export const farPyramidSample = createGenericSample(
  "world/far-pyramid", "World / Far Pyramid", spec,
  () => new Worker(new URL("./far-pyramid.worker.ts", import.meta.url), { type: "module" }),
);
