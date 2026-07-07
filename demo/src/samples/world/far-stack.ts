import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createFarStackBodies, farStackCamera, farStackGroundSize } from "./far-stack-scene";

const half = farStackGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createFarStackBodies(),
  camera: farStackCamera,
  info: "box stack 10000km from origin",
};

export const farStackSample = createGenericSample(
  "world/far-stack", "World / Far Stack", spec,
  () => new Worker(new URL("./far-stack.worker.ts", import.meta.url), { type: "module" }),
);
