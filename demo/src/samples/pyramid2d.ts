import { createGenericSample } from "./generic-host";
import type { RenderBody, RenderSpec } from "./generic-host";

const spec: RenderSpec = {
  groundSize: [80, 2, 80],
  bodies: (() => {
    const bodies: RenderBody[] = [];
    for (let row = 0; row < 12; row++) for (let col = 0; col < 12 - row; col++) bodies.push({ kind: "box", size: [2, 2, 2], position: [-10 + 2 * col + row, 1.5 + 2.5 * row, 0], color: 0x60a5fa + (row % 10) * 0x010101 });
    return bodies;
  })(),
  info: "12 rows, 2D stacking (Z-locked)",
};

export const pyramid2dSample = createGenericSample("pyramid2d", "Stacking / Pyramid2D", spec, () => new Worker(new URL("./pyramid2d.worker.ts", import.meta.url), { type: "module" }));
