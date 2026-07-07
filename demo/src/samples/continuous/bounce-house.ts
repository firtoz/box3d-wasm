import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { bounceHouseCamera, bounceHouseGroundSize, createBounceHouseBodies } from "./bounce-house-scene";

const half = bounceHouseGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createBounceHouseBodies(),
  camera: bounceHouseCamera,
};

export const bounceHouseSample = createGenericSample(
  "continuous/bounce-house", "Continuous / Bounce House", spec,
  () => new Worker(new URL("./bounce-house.worker.ts", import.meta.url), { type: "module" }),
);
