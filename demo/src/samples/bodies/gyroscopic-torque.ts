import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [40, 2, 40],
  bodies: [
    {
      kind: "compound",
      position: [0, 2, 0],
      rotation: [-0.7071067811865475, 0, 0, 0.7071067811865476],
      parts: [
        { kind: "box", size: [2, 0.1, 0.2], color: 0x3b82f6 },
        { kind: "cylinder", radius: 0.15, height: 0.6, segments: 32, position: [0, 0.3, 0], color: 0x3b82f6 },
      ],
    },
  ],
  camera: { position: [0, 20, 4], target: [0, 2, 0] },
  info: "Dzhanibekov effect: cylinder + box, gravityScale=0, angular velocity",
};

export const gyroscopicTorqueSample = createGenericSample(
  "bodies/gyroscopic-torque", "Bodies / Gyroscopic Torque", spec,
  () => new Worker(new URL("./gyroscopic-torque.worker.ts", import.meta.url), { type: "module" }),
);
