import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { bulletVsStackCamera, bulletVsStackGroundSize, createBulletVsStackBodies } from "./bullet-vs-stack-scene";

const half = bulletVsStackGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createBulletVsStackBodies(),
  camera: bulletVsStackCamera,
  info: "CCD bullet vs box stack. Press Launch (or L).",
  controls: [
    { type: "button", label: "Launch", message: { type: "launch" } },
  ],
  hotkeys: [
    { keys: ["l", "L"], message: { type: "launch" } },
  ],
};

export const bulletVsStackSample = createGenericSample(
  "continuous/bullet-vs-stack",
  "Continuous / Bullet vs Stack",
  spec,
  () => new Worker(new URL("./bullet-vs-stack.worker.ts", import.meta.url), { type: "module" }),
);
