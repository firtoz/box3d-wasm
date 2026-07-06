import { B3_AXIS_Y, quatFromAxisAngle } from "box3d-wasm";
import { createGenericSample } from "../generic-host";
import type { RenderBody, RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [60, 2, 60],
  bodies: (() => {
    const bodies: RenderBody[] = [];
    for (let i = 0; i < 24; i++) {
      const even = (i & 1) === 0;
      const a = even ? 0.5 * Math.PI : 0;
      const x = even ? 1.75 : 0;
      const z = even ? 0 : 1.75;
      bodies.push({ kind: "box", size: [5, 0.5, 0.5], position: [x, 0.5 * i + 0.25, z], rotation: quatFromAxisAngle(B3_AXIS_Y, a), color: 0xf59e0b });
      bodies.push({ kind: "box", size: [5, 0.5, 0.5], position: [-x, 0.5 * i + 0.25, -z], rotation: quatFromAxisAngle(B3_AXIS_Y, a), color: 0xf59e0b });
    }
    return bodies;
  })(),
};

export const jengaStackSample = createGenericSample("jenga-stack", "Stacking / Jenga Stack", spec, () => new Worker(new URL("./stack.worker.ts", import.meta.url), { type: "module" }));
