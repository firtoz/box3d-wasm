import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { motionLocksBodies, motionLocksCamera, motionLocksGroundSize } from "./motion-locks-scene";

const half = motionLocksGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: motionLocksBodies,
  camera: motionLocksCamera,
};

export const motionLocksSample = createGenericSample(
  "joints/motion-locks", "Joints / Motion Locks", spec,
  () => new Worker(new URL("./motion-locks.worker.ts", import.meta.url), { type: "module" }),
);
