import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { convexJitterBodies, convexJitterCamera, convexJitterGroundSize } from "./convex-jitter-scene";

const half = convexJitterGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: convexJitterBodies,
  camera: convexJitterCamera,
  info: "custom hull shapes from point clouds",
};

export const convexJitterSample = createGenericSample(
  "issues/convex-jitter", "Issues / Convex Jitter", spec,
  () => new Worker(new URL("./convex-jitter.worker.ts", import.meta.url), { type: "module" }),
);
