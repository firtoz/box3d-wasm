import { B3_AXIS_X, B3_DEG_TO_RAD, BodyType, quatFromAxisAngle } from "box3d-wasm";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [100, 2, 100],
  bodies: (() => {
    const bodies: import("../generic-host").RenderBody[] = [
      { kind: "box", size: [32, 1, 20], position: [0, 7.5, -5], rotation: quatFromAxisAngle(B3_AXIS_X, 40 * B3_DEG_TO_RAD), color: 0x94a3b8, type: BodyType.Static },
    ];
    for (let i = 0; i < 5; i++) bodies.push({ kind: "box", size: [2, 2, 2], position: [-10 + 5 * i, 15.75, -10.6], color: 0x60a5fa + i * 0x050505 });
    return bodies;
  })(),
};

export const shapesInclinedPlaneSample = createGenericSample("shapes-inclined-plane", "Shapes / Inclined Plane", spec, () => new Worker(new URL("./inclined-plane.worker.ts", import.meta.url), { type: "module" }));
