import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { meshScaleBodies, meshScaleCamera, meshScaleGroundSize } from "./mesh-scale-scene";

const half = meshScaleGroundSize();
const spec: RenderSpec = {
  groundKind: "none",
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: meshScaleBodies,
  camera: meshScaleCamera,
};

export const meshScaleSample = createGenericSample(
  "collision/mesh-scale",
  "Collision / Mesh Scale",
  spec,
  () => new Worker(new URL("./mesh-scale.worker.ts", import.meta.url), { type: "module" }),
);
