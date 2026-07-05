import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [24, 2, 24],
  bodies: Array.from({ length: 6 }, (_, i) => ({
    kind: "box" as const, size: [1, 1, 1] as [number, number, number],
    position: [10000, 0.5 + 1 * i, 0] as [number, number, number],
    color: 0x3b82f6,
  })),
  camera: { position: [10000, 8, 16], target: [10000, 2, 0] },
  info: "box stack 10000km from origin",
};

export const farStackSample = createGenericSample(
  "world/far-stack", "World / Far Stack", spec,
  () => new Worker(new URL("./far-stack.worker.ts", import.meta.url), { type: "module" }),
);
