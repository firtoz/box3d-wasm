import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { offsetKinematicBodies, offsetKinematicCamera, offsetKinematicGroundSize } from "./offset-kinematic-scene";

const half = offsetKinematicGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: offsetKinematicBodies,
  camera: offsetKinematicCamera,
};

export const offsetKinematicSample = createGenericSample(
  "bodies/offset-kinematic", "Bodies / Offset Kinematic", spec,
  () => new Worker(new URL("./offset-kinematic.worker.ts", import.meta.url), { type: "module" }),
);
