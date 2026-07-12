import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { stallBodies, stallCamera, stallGroundSize } from "./stall-scene";

const half = stallGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: stallBodies,
  camera: stallCamera,
  info: "CCD stall logging (threshold 1 ms). Dense torus + rock bullet. Press Launch (or L).",
  controls: [
    { type: "button", label: "Launch", message: { type: "launch" } },
  ],
  hotkeys: [
    { keys: ["l", "L"], message: { type: "launch" } },
  ],
};

export const stallSample = createGenericSample(
  "continuous/stall",
  "Continuous / Stall",
  spec,
  () => new Worker(new URL("./stall.worker.ts", import.meta.url), { type: "module" }),
);
