import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createJengaStackBodies, jengaStackCamera, jengaStackGroundSize } from "./stack-scene";

const half = jengaStackGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createJengaStackBodies(),
  camera: jengaStackCamera,
};

export const jengaStackSample = createGenericSample("jenga-stack", "Stacking / Jenga Stack", spec, () => new Worker(new URL("./stack.worker.ts", import.meta.url), { type: "module" }));
