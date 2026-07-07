import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createSlideTwistBodies, slideTwistCamera, slideTwistGroundSize } from "./slide-twist-scene";

const half = slideTwistGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createSlideTwistBodies(),
  camera: slideTwistCamera,
};

export const slideTwistSample = createGenericSample(
  "shapes/slide-twist", "Shapes / Slide Twist", spec,
  () => new Worker(new URL("./slide-twist.worker.ts", import.meta.url), { type: "module" }),
);
