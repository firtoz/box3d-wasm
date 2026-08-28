import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createRagdollMeshBodies, ragdollMeshCamera, ragdollMeshGroundSize } from "./mesh-scene";

const half = ragdollMeshGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createRagdollMeshBodies(),
  camera: ragdollMeshCamera,
  info: "ragdoll on mesh — joint friction / hertz / damping",
  controls: [
    { type: "range", label: "Joint Friction", message: { type: "set-friction" }, min: 0, max: 20, step: 1, value: 5 },
    { type: "range", label: "Hertz", message: { type: "set-hertz" }, min: 0, max: 20, step: 0.1, value: 2 },
    { type: "range", label: "Damping", message: { type: "set-damping" }, min: 0, max: 4, step: 0.1, value: 0.7 },
    { type: "button", label: "Respawn", message: { type: "respawn" } },
  ],
};

export const ragdollMeshSample = createGenericSample(
  "ragdoll/mesh",
  "Ragdoll / Mesh",
  spec,
  () => new Worker(new URL("./mesh.worker.ts", import.meta.url), { type: "module" }),
);
