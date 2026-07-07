import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { hullBodies, hullCamera, hullGroundSize } from "./hull-scene";

const half = hullGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: hullBodies,
  camera: hullCamera,
  info: "hull created from 48 cloud points",
};

export const hullSample = createGenericSample(
  "geometry/hull", "Geometry / Hull", spec,
  () => new Worker(new URL("./hull.worker.ts", import.meta.url), { type: "module" }),
);
