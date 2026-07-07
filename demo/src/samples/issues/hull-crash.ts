import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { hullCrashBodies, hullCrashCamera, hullCrashGroundSize } from "./hull-crash-scene";

const half = hullCrashGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: hullCrashBodies,
  camera: hullCrashCamera,
  info: "hull from 5-point regression case",
};

export const hullCrashSample = createGenericSample(
  "issues/hull-crash", "Issues / Hull Crash", spec,
  () => new Worker(new URL("./hull-crash.worker.ts", import.meta.url), { type: "module" }),
);
