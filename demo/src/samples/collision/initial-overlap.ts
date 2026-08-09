import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { initialOverlapBodies, initialOverlapCamera, initialOverlapGroundSize } from "./initial-overlap-scene";

const half = initialOverlapGroundSize();
const spec: RenderSpec = {
  groundKind: "none",
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: initialOverlapBodies,
  camera: initialOverlapCamera,
};

export const initialOverlapSample = createGenericSample(
  "collision/initial-overlap",
  "Collision / Initial Overlap",
  spec,
  () => new Worker(new URL("./initial-overlap.worker.ts", import.meta.url), { type: "module" }),
);
