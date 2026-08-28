import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { extraForcesBodies, extraForcesCamera, extraForcesGroundSize } from "./forces-scene";

const half = extraForcesGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: extraForcesBodies,
  camera: extraForcesCamera,
  info: "Green: ApplyForceToCenter each step. Blue: linear impulse. Orange: angular impulse. Nudge: ApplyForce off-center.",
  controls: [
    { type: "button", label: "Impulse", message: { type: "impulse" } },
    { type: "button", label: "Spin", message: { type: "spin" } },
    { type: "button", label: "Nudge", message: { type: "nudge" } },
  ],
};

export const extraForcesSample = createGenericSample(
  "extra/forces",
  "Extra / Forces",
  spec,
  () => new Worker(new URL("./forces.worker.ts", import.meta.url), { type: "module" }),
);
