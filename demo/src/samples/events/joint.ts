import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { jointEventBodies, jointEventCamera, jointEventGroundSize } from "./joint-scene";

const half = jointEventGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: jointEventBodies,
  camera: jointEventCamera,
  info: "four breakable joints: distance, prismatic, revolute, weld",
};

export const jointEventSample = createGenericSample(
  "events/joint",
  "Events / Joint",
  spec,
  () => new Worker(new URL("./joint.worker.ts", import.meta.url), { type: "module" }),
);
