import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { capsuleMeshBodies, capsuleMeshCamera, capsuleMeshGroundSize } from "./capsule-mesh-scene";

const half = capsuleMeshGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: capsuleMeshBodies,
  camera: capsuleMeshCamera,
};

export const capsuleMeshSample = createGenericSample(
  "issues/capsule-mesh",
  "Issues / Capsule Mesh",
  spec,
  () => new Worker(new URL("./capsule-mesh.worker.ts", import.meta.url), { type: "module" }),
);
