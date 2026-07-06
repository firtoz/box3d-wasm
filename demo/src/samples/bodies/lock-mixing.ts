import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { lockMixingBodies, lockMixingCamera, lockMixingGroundSize } from "./lock-mixing-scene";

const half = lockMixingGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: lockMixingBodies,
  camera: lockMixingCamera,
};

export const lockMixingSample = createGenericSample(
  "bodies/lock-mixing", "Bodies / Lock Mixing", spec,
  () => new Worker(new URL("./lock-mixing.worker.ts", import.meta.url), { type: "module" }),
);
