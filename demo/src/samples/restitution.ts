import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import { createRestitutionBodies, restitutionCamera, restitutionGroundSize } from "./restitution-scene";

const half = restitutionGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createRestitutionBodies(),
  camera: restitutionCamera,
};

export const restitutionSample = createGenericSample("restitution", "Shapes / Restitution", spec, () => new Worker(new URL("./restitution.worker.ts", import.meta.url), { type: "module" }));
