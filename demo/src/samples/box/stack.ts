import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { RenderBody } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [80, 2, 80],
  bodies: (() => {
    const bodies: RenderBody[] = [];
    for (let i = 0; i < 40; i++) bodies.push({ kind: "box", size: [1, 1, 1], position: [0, 0.75 + 1.25 * i, 0], color: 0x60a5fa + (i % 10) * 0x010101 });
    return bodies;
  })(),
};

export const boxStackSample = createGenericSample("box-stack", "Stacking / Box Stack", spec, () => new Worker(new URL("./stack.worker.ts", import.meta.url), { type: "module" }));
