import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { fixedRotationBodies, fixedRotationCamera, fixedRotationGroundSize } from "./fixed-rotation-scene";

const half = fixedRotationGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: fixedRotationBodies,
  camera: fixedRotationCamera,
};

export const fixedRotationSample = createGenericSample(
  "bodies/fixed-rotation", "Bodies / Fixed Rotation", spec,
  () => new Worker(new URL("./fixed-rotation.worker.ts", import.meta.url), { type: "module" }),
);
