import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import {
  restitutionOvershootBodies,
  restitutionOvershootCamera,
  restitutionOvershootGroundSize,
} from "./restitution-overshoot-scene";

const half = restitutionOvershootGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: restitutionOvershootBodies,
  camera: restitutionOvershootCamera,
  info: "restitution=1 drop — bounce should not exceed drop height",
};

export const restitutionOvershootSample = createGenericSample(
  "issues/restitution-overshoot",
  "Issues / Restitution Overshoot",
  spec,
  () => new Worker(new URL("./restitution-overshoot.worker.ts", import.meta.url), { type: "module" }),
);
