import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { hullTransformBodies, hullTransformCamera, hullTransformGroundSize } from "./hull-transform-scene";

const half = hullTransformGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: hullTransformBodies,
  camera: hullTransformCamera,
  info: "transformed hull with offset and scale",
};

export const hullTransformSample = createGenericSample(
  "geometry/hull-transform", "Geometry / Hull Transform", spec,
  () => new Worker(new URL("./hull-transform.worker.ts", import.meta.url), { type: "module" }),
);
