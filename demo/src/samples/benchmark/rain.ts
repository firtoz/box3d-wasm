import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { rainCamera, rainGroundSize } from "./rain-scene";

const half = rainGroundSize();
const spec: RenderSpec = {
  groundKind: "plane",
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: [],
  camera: rainCamera,
  info: "ragdoll rain spawns incrementally in the worker",
};

export const rainSample = createGenericSample(
  "benchmark/rain", "Benchmark / Rain", spec,
  () => new Worker(new URL("./rain.worker.ts", import.meta.url), { type: "module" }),
);
