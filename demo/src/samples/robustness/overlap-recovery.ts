import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [],
  camera: { position: [45, 20, 15], target: [0, 0, 0] },
  info: "bodies starting with 25% overlap",
};

export const overlapRecoverySample = createGenericSample(
  "robustness/overlap-recovery", "Robustness / Overlap Recovery", spec,
  () => new Worker(new URL("./overlap-recovery.worker.ts", import.meta.url), { type: "module" }),
);
