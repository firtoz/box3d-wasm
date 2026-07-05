import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [],
  camera: { position: [-30, 20, 10], target: [0, 0.5, 0] },
  info: "tiny 2.5cm box pyramid",
};

export const tinyPyramidSample = createGenericSample(
  "robustness/tiny-pyramid", "Robustness / Tiny Pyramid", spec,
  () => new Worker(new URL("./tiny-pyramid.worker.ts", import.meta.url), { type: "module" }),
);
