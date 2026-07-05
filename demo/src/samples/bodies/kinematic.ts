import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [
    { kind: "box", size: [0.1, 1, 0.2], position: [4, 3, 0], color: 0x22c55e },
  ],
  info: "drives target transform after 2s delay",
  camera: { position: [0, 30, 10], target: [0, 1.5, 0] },
};

export const kinematicSample = createGenericSample(
  "bodies/kinematic", "Bodies / Kinematic", spec,
  () => new Worker(new URL("./kinematic.worker.ts", import.meta.url), { type: "module" }),
);
