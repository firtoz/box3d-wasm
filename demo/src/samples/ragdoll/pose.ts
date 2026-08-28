import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createPoseBodies, poseCamera, poseGroundSize } from "./pose-scene";

const half = poseGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: createPoseBodies(),
  camera: poseCamera,
  info: "upstream Pose sample is #if 0; human + grid + cylinder only",
};

export const ragdollPoseSample = createGenericSample(
  "ragdoll/pose",
  "Ragdoll / Pose",
  spec,
  () => new Worker(new URL("./pose.worker.ts", import.meta.url), { type: "module" }),
);
