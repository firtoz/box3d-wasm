import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createThinWallBodies, thinWallCamera, thinWallGroundSize } from "./thin-wall-scene";

const half = thinWallGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createThinWallBodies(),
  camera: thinWallCamera,
};

export const thinWallSample = createGenericSample(
  "continuous/thin-wall", "Continuous / Thin Wall", spec,
  () => new Worker(new URL("./thin-wall.worker.ts", import.meta.url), { type: "module" }),
);
