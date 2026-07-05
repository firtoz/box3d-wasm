import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [
    { kind: "box", size: [1, 0.5, 0.5], position: [0, 1, 0], color: 0x8b5cf6 },
  ],
  camera: { position: [0, 15, 5], target: [0, 0, 0] },
  info: "transformed hull with offset and scale",
};

export const hullTransformSample = createGenericSample(
  "geometry/hull-transform", "Geometry / Hull Transform", spec,
  () => new Worker(new URL("./hull-transform.worker.ts", import.meta.url), { type: "module" }),
);
