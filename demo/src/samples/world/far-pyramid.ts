import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [800, 2, 800],
  bodies: [],
  camera: { position: [10040, -10, 60], target: [10000, 20, 0] },
  info: "40-base pyramid 10000km from origin",
};

export const farPyramidSample = createGenericSample(
  "world/far-pyramid", "World / Far Pyramid", spec,
  () => new Worker(new URL("./far-pyramid.worker.ts", import.meta.url), { type: "module" }),
);
