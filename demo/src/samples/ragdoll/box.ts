import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createRagdollBoxBodies, ragdollBoxCamera, ragdollBoxGroundSize } from "./box-scene";

const half = ragdollBoxGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createRagdollBoxBodies(),
  camera: ragdollBoxCamera,
  info: "ragdoll on box — joint friction / hertz / damping",
  controls: [
    { type: "range", label: "Joint Friction", message: { type: "set-friction" }, min: 0, max: 20, step: 1, value: 5 },
    { type: "range", label: "Hertz", message: { type: "set-hertz" }, min: 0, max: 20, step: 0.1, value: 1 },
    { type: "range", label: "Damping", message: { type: "set-damping" }, min: 0, max: 4, step: 0.1, value: 0.7 },
    { type: "button", label: "Respawn", message: { type: "respawn" } },
  ],
};

export const ragdollBoxSample = createGenericSample(
  "ragdoll/box", "Ragdoll / Box", spec,
  () => new Worker(new URL("./box.worker.ts", import.meta.url), { type: "module" }),
);
