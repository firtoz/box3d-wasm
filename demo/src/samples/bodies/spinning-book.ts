import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [20, 2, 20],
  bodies: [
    { kind: "box", size: [0.7, 0.16, 1], position: [-2, 2, 0], color: 0x3b82f6 },
    { kind: "box", size: [0.7, 0.16, 1], position: [0, 2, 0], color: 0x22c55e },
    { kind: "box", size: [0.7, 0.16, 1], position: [2, 2, 0], color: 0xef4444 },
  ],
  camera: { position: [0, 6, 8.66], target: [0, 1, 0] },
};

export const spinningBookSample = createGenericSample(
  "bodies/spinning-book", "Bodies / Spinning Book", spec,
  () => new Worker(new URL("./spinning-book.worker.ts", import.meta.url), { type: "module" }),
);
