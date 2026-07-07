import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { boxHullBodies, boxHullCamera, boxHullGroundSize } from "./box-hull-scene";

const half = boxHullGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: boxHullBodies,
  camera: boxHullCamera,
};

export const boxHullSample = createGenericSample(
  "geometry/box-hull", "Geometry / Box Hull", spec,
  () => new Worker(new URL("./box-hull.worker.ts", import.meta.url), { type: "module" }),
);
