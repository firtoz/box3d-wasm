import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createMeshTileBodies, meshTileCamera, meshTileGroundSize } from "./mesh-tile-scene";

const half = meshTileGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: createMeshTileBodies(),
  camera: meshTileCamera,
  info: "Static baked compound from four shared box mesh instances",
};

export const meshTileSample = createGenericSample(
  "compound/mesh-tile",
  "Compound / Mesh Tile",
  spec,
  () => new Worker(new URL("./mesh-tile.worker.ts", import.meta.url), { type: "module" }),
);
