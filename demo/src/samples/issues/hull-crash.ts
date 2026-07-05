import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [
    { kind: "box", size: [0.5, 0.5, 0.5], position: [0, 1.5, 0], color: 0xef4444 },
  ],
  camera: { position: [0, 15, 5], target: [0, 0, 0] },
  info: "hull from 5-point regression case",
};

export const hullCrashSample = createGenericSample(
  "issues/hull-crash", "Issues / Hull Crash", spec,
  () => new Worker(new URL("./hull-crash.worker.ts", import.meta.url), { type: "module" }),
);
