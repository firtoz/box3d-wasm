import { BodyType } from "box3d-wasm";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [
    { kind: "capsule", radius: 0.3, length: 1, position: [0, 0.5, 0], color: 0x888888, type: BodyType.Static },
    { kind: "capsule", radius: 0.2, length: 1, position: [0.3, 0.5, 0], color: 0x3b82f6 },
  ],
  camera: { position: [0, 15, 10], target: [0, 0, 0] },
};

export const fixedRotationSample = createGenericSample(
  "bodies/fixed-rotation", "Bodies / Fixed Rotation", spec,
  () => new Worker(new URL("./fixed-rotation.worker.ts", import.meta.url), { type: "module" }),
);
