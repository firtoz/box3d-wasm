import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

function qx(angle: number): [number, number, number, number] { return [Math.sin(angle / 2), 0, 0, Math.cos(angle / 2)]; }

const spec: RenderSpec = {
  groundSize: [100, 2, 100],
  bodies: (() => {
    const bodies: import("../generic-host").RenderBody[] = [
      { kind: "box", size: [32, 1, 20], position: [0, 7.5, -5], rotation: qx(40 * Math.PI / 180), color: 0x94a3b8, type: 0 },
    ];
    for (let i = 0; i < 5; i++) bodies.push({ kind: "box", size: [2, 2, 2], position: [-10 + 5 * i, 15.75, -10.6], color: 0x60a5fa + i * 0x050505 });
    return bodies;
  })(),
};

export const shapesInclinedPlaneSample = createGenericSample("shapes-inclined-plane", "Shapes / Inclined Plane", spec, () => new Worker(new URL("./inclined-plane.worker.ts", import.meta.url), { type: "module" }));
