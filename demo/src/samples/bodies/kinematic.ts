import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { kinematicBodies, kinematicCamera, kinematicGroundSize } from "./kinematic-scene";

const half = kinematicGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: kinematicBodies,
  info: "drives target transform after 2s delay",
  camera: kinematicCamera,
};

export const kinematicSample = createGenericSample(
  "bodies/kinematic", "Bodies / Kinematic", spec,
  () => new Worker(new URL("./kinematic.worker.ts", import.meta.url), { type: "module" }),
);
