import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { viewerBodies, viewerCamera, viewerGroundSize } from "./viewer-scene";

const half = viewerGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: viewerBodies,
  camera: viewerCamera,
  info: "voxel mesh viewer (z-up, scale 0.01)",
  controls: [
    { type: "range", label: "mesh", message: { type: "set-mesh" }, min: 0, max: 3, step: 1, value: 0 },
    { type: "toggle", label: "median split", message: { type: "set-median" }, value: true },
    { type: "toggle", label: "edges", message: { type: "set-edges" }, value: true },
    { type: "toggle", label: "weld", message: { type: "set-weld" }, value: true },
  ],
};

export const meshViewerSample = createGenericSample(
  "mesh/viewer",
  "Mesh / Viewer",
  spec,
  () => new Worker(new URL("./viewer.worker.ts", import.meta.url), { type: "module" }),
);
