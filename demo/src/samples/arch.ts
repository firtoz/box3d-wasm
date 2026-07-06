import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import { archCamera, archGroundSize, createArchBodies } from "./arch-scene";

const half = archGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createArchBodies(),
  camera: archCamera,
};

export const archSample = createGenericSample("arch", "Stacking / Arch", spec, () => new Worker(new URL("./arch.worker.ts", import.meta.url), { type: "module" }));
