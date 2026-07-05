import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [
    { kind: "box", size: [1, 1, 1], position: [0, 2, 0], color: 0x22c55e },
    { kind: "box", size: [1, 1, 1], position: [2, 2, 0], color: 0x3b82f6 },
    { kind: "box", size: [1, 1, 1], position: [-2, 2, 0], color: 0xf59e0b },
    { kind: "box", size: [1, 1, 1], position: [0, 1, 2], color: 0xef4444 },
    { kind: "box", size: [1, 1, 1], position: [0, 1, -3], color: 0x888888 },
  ],
  camera: { position: [45, 30, 40], target: [0, 0, 0] },
};

export const lockMixingSample = createGenericSample(
  "bodies/lock-mixing", "Bodies / Lock Mixing", spec,
  () => new Worker(new URL("./lock-mixing.worker.ts", import.meta.url), { type: "module" }),
);
