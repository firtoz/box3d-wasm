import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { bodyCastBodies, bodyCastCamera, bodyCastGroundSize } from "./cast-scene";

const half = bodyCastGroundSize();
const spec: RenderSpec = {
  groundKind: "none",
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: bodyCastBodies,
  camera: bodyCastCamera,
};

export const bodyCastSample = createGenericSample(
  "bodies/cast",
  "Bodies / Cast",
  spec,
  () => new Worker(new URL("./cast.worker.ts", import.meta.url), { type: "module" }),
);
