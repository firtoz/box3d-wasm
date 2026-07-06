import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createSphereStackBodies, sphereStackCamera, sphereStackGroundSize } from "./stack-scene";

const half = sphereStackGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createSphereStackBodies(),
  camera: sphereStackCamera,
};

export const sphereStackSample = createGenericSample("sphere-stack", "Stacking / Sphere Stack", spec, () => new Worker(new URL("./stack.worker.ts", import.meta.url), { type: "module" }));
