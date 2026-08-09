import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createHollowBoxBodies, hollowBoxCamera, hollowBoxGroundSize } from "./hollow-box-scene";

const half = hollowBoxGroundSize();
const spec: RenderSpec = {
  groundKind: "none",
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createHollowBoxBodies(),
  camera: hollowBoxCamera,
};

export const hollowBoxSample = createGenericSample(
  "mesh/hollow-box",
  "Mesh / Hollow Box",
  spec,
  () => new Worker(new URL("./hollow-box.worker.ts", import.meta.url), { type: "module" }),
);
