import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { BodyType } from "box3d-wasm";

const spec: RenderSpec = {
  groundSize: [10, 1, 10],
  bodies: [
    { kind: "capsule", radius: 0.25, length: 1, position: [0, 8, 0], type: BodyType.Kinematic, color: 0x3b82f6 },
    { kind: "capsule", radius: 0.25, length: 1, position: [0, 5.5, 0], type: BodyType.Dynamic, color: 0x22c55e },
    { kind: "capsule", radius: 0.25, length: 1, position: [0, 3, 0], type: BodyType.Dynamic, color: 0xf97316 },
    { kind: "capsule", radius: 0.25, length: 1, position: [0, 0.5, 0], type: BodyType.Dynamic, color: 0xef4444 },
    { kind: "sphere", radius: 0.3, position: [-5, 4, 0], type: BodyType.Dynamic, color: 0xf59e0b },
  ],
  camera: { position: [8, 6, 12], target: [0, 4, 0] },
  info: "Chain with weld joints. Toggle enables/disables link[2] and the ball.",
  controls: [
    { type: "toggle", label: "Link[2]", message: { type: "enableLink2" }, value: true },
    { type: "toggle", label: "Ball", message: { type: "enableBall" }, value: true },
  ],
};

export const disableSample = createGenericSample(
  "bodies/disable", "Bodies / Disable", spec,
  () => new Worker(new URL("./disable.worker.ts", import.meta.url), { type: "module" }),
);
