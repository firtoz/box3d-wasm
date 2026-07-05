import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { BodyType } from "box3d-wasm";

const spec: RenderSpec = {
  groundSize: [10, 1, 10],
  bodies: [
    { kind: "box", size: [0.5, 2, 0.5], position: [-2, 3, 0], type: BodyType.Dynamic, color: 0x3b82f6 },
    { kind: "box", size: [0.5, 2, 0.5], position: [3, 3, 0], type: BodyType.Dynamic, color: 0xf97316 },
    { kind: "box", size: [0.5, 4, 0.5], position: [-4, 5, 0], type: BodyType.Dynamic, color: 0x22c55e },
    { kind: "box", size: [0.75, 0.75, 0.75], position: [-3, 8, 0], type: BodyType.Dynamic, color: 0xef4444 },
    { kind: "box", size: [0.75, 0.75, 0.75], position: [2, 8, 0], type: BodyType.Dynamic, color: 0xa855f7 },
    { kind: "capsule", radius: 0.25, length: 1, position: [8, 0.2, 0], type: BodyType.Dynamic, color: 0xf59e0b },
    { kind: "sphere", radius: 0.25, position: [-8, 12, 0], type: BodyType.Dynamic, color: 0x888888 },
  ],
  camera: { position: [0, 30, 30], target: [0, 1.5, 0] },
  info: "Switch body types (Static/Kinematic/Dynamic) and toggle enable/disable.",
  controls: [
    { type: "button", label: "Static", message: { type: "setBodyType", bodyType: BodyType.Static } },
    { type: "button", label: "Kinematic", message: { type: "setBodyType", bodyType: BodyType.Kinematic } },
    { type: "button", label: "Dynamic", message: { type: "setBodyType", bodyType: BodyType.Dynamic } },
    { type: "toggle", label: "Enabled", message: { type: "setEnabled" }, value: true },
  ],
};

export const bodyTypeSample = createGenericSample(
  "bodies/body-type", "Bodies / Body Type", spec,
  () => new Worker(new URL("./body-type.worker.ts", import.meta.url), { type: "module" }),
);
