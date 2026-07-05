import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [
    { kind: "box", size: [1, 0.5, 0.25], position: [0, 2, 0], color: 0x3b82f6 },
    { kind: "box", size: [1, 0.5, 0.25], position: [2, 2, 0], color: 0x22c55e },
  ],
  camera: { position: [0, 15, 5], target: [0, 0, 0] },
};

export const boxHullSample = createGenericSample(
  "geometry/box-hull", "Geometry / Box Hull", spec,
  () => new Worker(new URL("./box-hull.worker.ts", import.meta.url), { type: "module" }),
);
