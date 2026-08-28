import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { sboxMoverBodies, sboxMoverCamera, sboxMoverGroundSize } from "./sbox-mover-scene";

const half = sboxMoverGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: sboxMoverBodies,
  camera: sboxMoverCamera,
  info: "heightfield + platform mesh + locked box",
};

export const sboxMoverSample = createGenericSample(
  "issues/sbox-mover",
  "Issues / s&box mover",
  spec,
  () => new Worker(new URL("./sbox-mover.worker.ts", import.meta.url), { type: "module" }),
);
