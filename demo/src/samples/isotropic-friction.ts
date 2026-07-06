import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import { createIsotropicFrictionBodies, isotropicFrictionCamera, isotropicFrictionGroundSize } from "./isotropic-friction-scene";

const half = isotropicFrictionGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createIsotropicFrictionBodies(),
  camera: isotropicFrictionCamera,
};

export const isotropicFrictionSample = createGenericSample("isotropic-friction", "Shapes / Isotropic Friction", spec, () => new Worker(new URL("./isotropic-friction.worker.ts", import.meta.url), { type: "module" }));
