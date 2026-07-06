import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { BodyType } from "box3d-wasm";
import { bodyTypeBodies, bodyTypeCamera, bodyTypeGroundSize } from "./body-type-scene";

const half = bodyTypeGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: bodyTypeBodies,
  camera: bodyTypeCamera,
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
