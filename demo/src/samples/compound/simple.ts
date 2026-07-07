import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { compoundSimpleBodies, compoundSimpleCamera, compoundSimpleGroundSize } from "./simple-scene";

const half = compoundSimpleGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: compoundSimpleBodies,
  camera: compoundSimpleCamera,
  launchSpeed: 1,
  info: "compound with hull shape (C++ SimpleCompound)",
};

export const compoundSimpleSample = createGenericSample("compound/simple", "Compound / Simple", spec, () => new Worker(new URL("./simple.worker.ts", import.meta.url), { type: "module" }));
