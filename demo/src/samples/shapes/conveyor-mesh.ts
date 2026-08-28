import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { conveyorMeshCamera, conveyorMeshGroundSize, createConveyorMeshBodies } from "./conveyor-mesh-scene";

const half = conveyorMeshGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createConveyorMeshBodies(),
  camera: conveyorMeshCamera,
  info: "conveyor.obj per-triangle tangent velocities + 20 cylinders",
};

export const conveyorMeshSample = createGenericSample(
  "shapes/conveyor-mesh",
  "Shapes / Conveyor Mesh",
  spec,
  () => new Worker(new URL("./conveyor-mesh.worker.ts", import.meta.url), { type: "module" }),
);
