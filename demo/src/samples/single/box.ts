import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [40, 2, 40],
  bodies: [{ kind: "box", size: [1, 1, 1], position: [0, 0.5, 0], color: 0xf59e0b }],
  camera: { position: [0, 4.226, 9.063], target: [0, 0, 0] },
};

export const singleBoxSample = createGenericSample("single-box", "Stacking / Single Box", spec, () => new Worker(new URL("./box.worker.ts", import.meta.url), { type: "module" }));
