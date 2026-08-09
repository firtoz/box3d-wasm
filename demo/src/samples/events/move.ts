import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { moveEventBodies, moveEventCamera, moveEventGroundSize } from "./move-scene";

const half = moveEventGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: moveEventBodies,
  camera: moveEventCamera,
  info: "tall box with hit events · initial pivot velocity",
};

export const moveEventSample = createGenericSample(
  "events/move",
  "Events / Move",
  spec,
  () => new Worker(new URL("./move.worker.ts", import.meta.url), { type: "module" }),
);
