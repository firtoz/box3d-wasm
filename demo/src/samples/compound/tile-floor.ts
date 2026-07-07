import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createTileFloorBodies, tileFloorCamera, tileFloorGroundSize } from "./tile-floor-scene";

const half = tileFloorGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createTileFloorBodies(),
  camera: tileFloorCamera,
  info: "Compound tile floor (2500 hulls) + falling sphere",
};

export const tileFloorSample = createGenericSample(
  "compound/tile-floor", "Compound / Tile Floor", spec,
  () => new Worker(new URL("./tile-floor.worker.ts", import.meta.url), { type: "module" }),
);
