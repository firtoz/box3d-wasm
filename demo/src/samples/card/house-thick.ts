import { createGenericSample } from "../generic-host";
import type { RenderBody, RenderSpec } from "../generic-host";

function qz(angle: number): [number, number, number, number] { return [0, 0, Math.sin(angle / 2), Math.cos(angle / 2)]; }

const spec: RenderSpec = (() => {
  const alpha = 25 * Math.PI / 180;
  const ox = 0.5 * 0.98 * Math.sin(alpha) + 0.045;
  const oy = 0.5 * 0.98 * Math.cos(alpha) + 0.035;
  const bodies: RenderBody[] = [];
  const addPair = (x: number, y: number, count: number) => {
    for (let j = 0; j < count; j++) {
      for (const s of [-1, 1]) bodies.push({ kind: "box", size: [0.08, 0.98, 0.38], position: [x + s * ox, y, 0], rotation: qz(s * alpha), color: 0xfde68a });
      x += 4 * ox;
    }
  };
  const addRow = (x: number, y: number, c: number) => {
    for (let i = 0; i < c; i++) bodies.push({ kind: "box", size: [0.08, 0.98, 0.38], position: [x + i * 4 * ox, y, 0], rotation: qz(Math.PI / 2), color: 0xfde68a });
  };
  addPair(-6 * ox, oy, 4); addRow(-4 * ox, 2 * oy + 0.04, 3); addPair(-4 * ox, 3 * oy + 0.08, 3); addRow(-2 * ox, 4 * oy + 0.12, 2); addPair(-2 * ox, 5 * oy + 0.16, 2); addRow(0, 6 * oy + 0.20, 1); addPair(0, 7 * oy + 0.24, 1);
  return { groundSize: [20, 2, 20], bodies, camera: { position: [0, 6.226, 9.063], target: [0, 2, 0] } };
})();

export const cardHouseThickSample = createGenericSample("card-house-thick", "Stacking / Card House Thick", spec, () => new Worker(new URL("./house-thick.worker.ts", import.meta.url), { type: "module" }));
