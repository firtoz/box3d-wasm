import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createJointGridBodies, jointGridCamera, jointGridGroundSize } from "./joint-grid-scene";

const half = jointGridGroundSize();
const spec: RenderSpec = {
  groundKind: "plane",
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createJointGridBodies(),
  camera: jointGridCamera,
};

export const jointGridSample = createGenericSample(
  "benchmark/joint-grid", "Benchmark / Joint Grid", spec,
  () => new Worker(new URL("./joint-grid.worker.ts", import.meta.url), { type: "module" }),
);
