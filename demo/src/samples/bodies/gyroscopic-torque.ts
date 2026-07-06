import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { gyroscopicTorqueBodies, gyroscopicTorqueCamera, gyroscopicTorqueGroundSize } from "./gyroscopic-torque-scene";

const half = gyroscopicTorqueGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: gyroscopicTorqueBodies,
  camera: gyroscopicTorqueCamera,
};

export const gyroscopicTorqueSample = createGenericSample(
  "bodies/gyroscopic-torque", "Bodies / Gyroscopic Torque", spec,
  () => new Worker(new URL("./gyroscopic-torque.worker.ts", import.meta.url), { type: "module" }),
);
