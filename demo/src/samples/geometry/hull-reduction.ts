import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { hullReductionBodies, hullReductionCamera, hullReductionGroundSize } from "./hull-reduction-scene";

const half = hullReductionGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: hullReductionBodies,
  camera: hullReductionCamera,
  info: "hull reduced from 128 random sphere points",
};

export const hullReductionSample = createGenericSample(
  "geometry/hull-reduction", "Geometry / Hull Reduction", spec,
  () => new Worker(new URL("./hull-reduction.worker.ts", import.meta.url), { type: "module" }),
);
