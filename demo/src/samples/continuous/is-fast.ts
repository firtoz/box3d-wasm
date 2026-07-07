import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createIsFastBodies, isFastCamera, isFastGroundSize } from "./is-fast-scene";

const half = isFastGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createIsFastBodies(),
  camera: isFastCamera,
};

export const isFastSample = createGenericSample(
  "continuous/is-fast", "Continuous / Is Fast", spec,
  () => new Worker(new URL("./is-fast.worker.ts", import.meta.url), { type: "module" }),
);
