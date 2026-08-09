import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { classRingCamera, classRingGroundSize, createClassRingBodies } from "./class-ring-scene";

const half = classRingGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createClassRingBodies(),
  camera: classRingCamera,
  info: "24-capsule ring + dense gem · 960 Hz (16× @ 8 substeps)",
};

export const classRingSample = createGenericSample(
  "bodies/class-ring",
  "Bodies / Class Ring",
  spec,
  () => new Worker(new URL("./class-ring.worker.ts", import.meta.url), { type: "module" }),
);
