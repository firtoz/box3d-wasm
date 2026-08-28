import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createVillageBodies, villageCamera, villageGroundSize } from "./village-scene";

const half = villageGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: createVillageBodies(),
  camera: villageCamera,
  info: "8×8 baked compound (debug village; no building meshes)",
};

export const villageSample = createGenericSample(
  "compound/village",
  "Compound / Village",
  spec,
  () => new Worker(new URL("./village.worker.ts", import.meta.url), { type: "module" }),
);
