import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { compoundHullsCamera, compoundHullsGroundSize, createCompoundHullsBodies } from "./hulls-scene";

const half = compoundHullsGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createCompoundHullsBodies(),
  camera: compoundHullsCamera,
};

export const compoundHullsSample = createGenericSample("compound/hulls", "Compound / Hulls", spec, () => new Worker(new URL("./hulls.worker.ts", import.meta.url), { type: "module" }));
