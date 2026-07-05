import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [24, 1, 24],
  bodies: [
    { kind: "box", size: [2, 2, 2], position: [-2, 4, 0], color: 0x38bdf8 },
    { kind: "box", size: [2, 2, 2], position: [2, 4, 0], color: 0xf97316 },
  ],
};

export const compoundMaterialDedupSample = createGenericSample("compound-material-dedup", "Compound Material Dedup", spec, () => new Worker(new URL("./material-dedup.worker.ts", import.meta.url), { type: "module" }));
