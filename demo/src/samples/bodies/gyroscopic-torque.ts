import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [10, 1, 10],
  bodies: [
    { kind: "box", size: [1, 0.05, 0.1], position: [0, 2, 0], color: 0x3b82f6 },
  ],
  camera: { position: [5, 10, 15], target: [0, 2, 0] },
  info: "Dzhanibekov effect: cylinder + box, gravityScale=0, angular velocity",
};

export const gyroscopicTorqueSample = createGenericSample(
  "bodies/gyroscopic-torque", "Bodies / Gyroscopic Torque", spec,
  () => new Worker(new URL("./gyroscopic-torque.worker.ts", import.meta.url), { type: "module" }),
);
