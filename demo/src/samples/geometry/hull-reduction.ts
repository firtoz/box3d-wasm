import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [
    { kind: "box", size: [0.5, 0.5, 0.5], position: [0, 1, 0], color: 0xf59e0b },
  ],
  camera: { position: [0, 15, 5], target: [0, 0, 0] },
  info: "hull reduced from 128 random sphere points",
};

export const hullReductionSample = createGenericSample(
  "geometry/hull-reduction", "Geometry / Hull Reduction", spec,
  () => new Worker(new URL("./hull-reduction.worker.ts", import.meta.url), { type: "module" }),
);
