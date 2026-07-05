import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { BodyType } from "box3d-wasm";

const spec: RenderSpec = {
  groundSize: [10, 1, 10],
  bodies: [
    { kind: "capsule", radius: 0.25, length: 1, position: [0, 10, 0], type: BodyType.Dynamic, color: 0x3b82f6 },
  ],
  camera: { position: [5, 12, 10], target: [0, 5, 0] },
  info: "Center of mass shifted upward — weeble rights itself",
  controls: [
    { type: "button", label: "Teleport", message: { type: "teleport" } },
    { type: "button", label: "Explode", message: { type: "explode" } },
  ],
};

export const weebleSample = createGenericSample(
  "bodies/weeble", "Bodies / Weeble", spec,
  () => new Worker(new URL("./weeble.worker.ts", import.meta.url), { type: "module" }),
);
