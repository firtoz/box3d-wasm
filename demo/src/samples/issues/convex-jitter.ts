import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [
    { kind: "box", size: [0.5, 0.5, 0.5], position: [-0.6, 2, 0.27], color: 0x3b82f6 },
    { kind: "box", size: [0.5, 0.5, 0.5], position: [0.4, 2, 0.16], color: 0xf59e0b },
  ],
  camera: { position: [0, 15, 10], target: [0, 2, 0] },
  info: "custom hull shapes from point clouds",
};

export const convexJitterSample = createGenericSample(
  "issues/convex-jitter", "Issues / Convex Jitter", spec,
  () => new Worker(new URL("./convex-jitter.worker.ts", import.meta.url), { type: "module" }),
);
