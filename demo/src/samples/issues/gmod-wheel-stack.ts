import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import {
  createGmodWheelStackBodies,
  gmodWheelStackCamera,
  gmodWheelStackGroundSize,
} from "./gmod-wheel-stack-scene";

const half = gmodWheelStackGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createGmodWheelStackBodies(),
  camera: gmodWheelStackCamera,
  info: "30 GMod metal_wheel1 hulls stacked — contact tuning 240/10/3",
};

export const gmodWheelStackSample = createGenericSample(
  "issues/gmod-wheel-stack",
  "Issues / GMod Wheel Stack",
  spec,
  () => new Worker(new URL("./gmod-wheel-stack.worker.ts", import.meta.url), { type: "module" }),
);
