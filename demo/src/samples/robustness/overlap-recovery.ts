import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { overlapRecoveryBodies, overlapRecoveryCamera, overlapRecoveryGroundSize } from "./overlap-recovery-scene";

const half = overlapRecoveryGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: overlapRecoveryBodies,
  camera: overlapRecoveryCamera,
  info: "bodies starting with 25% overlap",
};

export const overlapRecoverySample = createGenericSample(
  "robustness/overlap-recovery", "Robustness / Overlap Recovery", spec,
  () => new Worker(new URL("./overlap-recovery.worker.ts", import.meta.url), { type: "module" }),
);
