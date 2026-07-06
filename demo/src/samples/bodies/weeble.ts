import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { weebleBodies, weebleCamera, weebleGroundSize } from "./weeble-scene";

const half = weebleGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: weebleBodies,
  camera: weebleCamera,
  info: "Weeble wobbles but doesn't fall down.",
  controls: [
    { type: "button", label: "Teleport", message: { type: "teleport" } },
    { type: "button", label: "Explode", message: { type: "explode" } },
  ],
};

export const weebleSample = createGenericSample(
  "bodies/weeble", "Bodies / Weeble", spec,
  () => new Worker(new URL("./weeble.worker.ts", import.meta.url), { type: "module" }),
);
