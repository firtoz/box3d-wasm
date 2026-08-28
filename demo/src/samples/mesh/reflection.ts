import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createReflectionBodies, reflectionCamera, reflectionGroundSize } from "./reflection-scene";

const half = reflectionGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: createReflectionBodies(),
  camera: reflectionCamera,
  info: "negative-scale building mesh + 20 humans (Neg/Pos X Y Z)",
  controls: [
    { type: "button", label: "Neg X", message: { type: "set-scale", x: -1 } },
    { type: "button", label: "Pos X", message: { type: "set-scale", x: 1 } },
    { type: "button", label: "Neg Y", message: { type: "set-scale", y: -1 } },
    { type: "button", label: "Pos Y", message: { type: "set-scale", y: 1 } },
    { type: "button", label: "Neg Z", message: { type: "set-scale", z: -1 } },
    { type: "button", label: "Pos Z", message: { type: "set-scale", z: 1 } },
  ],
};

export const reflectionSample = createGenericSample(
  "mesh/reflection",
  "Mesh / Reflection",
  spec,
  () => new Worker(new URL("./reflection.worker.ts", import.meta.url), { type: "module" }),
);
