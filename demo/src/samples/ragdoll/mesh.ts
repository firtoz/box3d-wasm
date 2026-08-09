import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createRagdollMeshBodies, ragdollMeshCamera, ragdollMeshGroundSize } from "./mesh-scene";

const half = ragdollMeshGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createRagdollMeshBodies(),
  camera: ragdollMeshCamera,
};

export const ragdollMeshSample = createGenericSample(
  "ragdoll/mesh",
  "Ragdoll / Mesh",
  spec,
  () => new Worker(new URL("./mesh.worker.ts", import.meta.url), { type: "module" }),
);
