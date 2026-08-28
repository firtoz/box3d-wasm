import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createVoxelBodies, voxelCamera, voxelGroundSize } from "./voxel-scene";

const half = voxelGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: createVoxelBodies(),
  camera: voxelCamera,
  info: "voxel collision mesh + custom hull (obj load)",
};

export const voxelSample = createGenericSample(
  "mesh/voxel",
  "Mesh / Voxel",
  spec,
  () => new Worker(new URL("./voxel.worker.ts", import.meta.url), { type: "module" }),
);
