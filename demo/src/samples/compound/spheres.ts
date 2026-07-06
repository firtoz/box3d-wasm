import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { compoundSpheresCamera, compoundSpheresGroundSize, createCompoundSpheresBodies } from "./spheres-scene";

const half = compoundSpheresGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createCompoundSpheresBodies(),
  camera: compoundSpheresCamera,
};

export const compoundSpheresSample = createGenericSample("compound/spheres", "Compound / Spheres", spec, () => new Worker(new URL("./spheres.worker.ts", import.meta.url), { type: "module" }));
