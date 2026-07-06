import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { disableBodies, disableCamera, disableGroundSize } from "./disable-scene";

const half = disableGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: disableBodies,
  camera: disableCamera,
  info: "Chain with weld joints. Toggle enables/disables link[2] and the ball.",
  controls: [
    { type: "toggle", label: "Link[2]", message: { type: "enableLink2" }, value: true },
    { type: "toggle", label: "Ball", message: { type: "enableBall" }, value: true },
  ],
};

export const disableSample = createGenericSample(
  "bodies/disable", "Bodies / Disable", spec,
  () => new Worker(new URL("./disable.worker.ts", import.meta.url), { type: "module" }),
);
