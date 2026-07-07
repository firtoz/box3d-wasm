import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { conveyorBeltCamera, conveyorBeltGroundSize, createConveyorBeltBodies } from "./conveyor-belt-scene";

const half = conveyorBeltGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createConveyorBeltBodies(),
  camera: conveyorBeltCamera,
};

export const conveyorBeltSample = createGenericSample(
  "shapes/conveyor-belt", "Shapes / Conveyor Belt", spec,
  () => new Worker(new URL("./conveyor-belt.worker.ts", import.meta.url), { type: "module" }),
);
