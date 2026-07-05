import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [50, 2, 50],
  bodies: Array.from({ length: 165 }, () => ({
    kind: "box" as const, size: [1, 1, 1] as [number, number, number],
    position: [0, 0, 0] as [number, number, number], color: 0x3b82f6,
  })),
  camera: { position: [30, 15, 70], target: [0, 0, 0] },
  info: "3 high-mass-ratio pyramid stacks",
};

export const highMassRatio1Sample = createGenericSample(
  "robustness/high-mass-ratio-1", "Robustness / HighMassRatio1", spec,
  () => new Worker(new URL("./high-mass-ratio-1.worker.ts", import.meta.url), { type: "module" }),
);
