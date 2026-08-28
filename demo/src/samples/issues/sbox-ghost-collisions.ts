import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { sboxGhostBodies, sboxGhostCamera, sboxGhostGroundSize } from "./sbox-ghost-collisions-scene";

const half = sboxGhostGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: sboxGhostBodies,
  camera: sboxGhostCamera,
  info: "locked box walks a flat mesh floor (speculative contacts off)",
};

export const sboxGhostCollisionsSample = createGenericSample(
  "issues/sbox-ghost-collisions",
  "Issues / s&box Ghost Collisions",
  spec,
  () => new Worker(new URL("./sbox-ghost-collisions.worker.ts", import.meta.url), { type: "module" }),
);
