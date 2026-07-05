import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [
    { kind: "box", size: [0.5, 0.5, 0.5], position: [0, 1, 0], color: 0x3b82f6 },
  ],
  camera: { position: [0, 15, 5], target: [0, 0, 0] },
  info: "hull created from 48 cloud points",
};

export const hullSample = createGenericSample(
  "geometry/hull", "Geometry / Hull", spec,
  () => new Worker(new URL("./hull.worker.ts", import.meta.url), { type: "module" }),
);
