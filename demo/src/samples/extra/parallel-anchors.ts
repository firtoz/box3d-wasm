import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { extraParallelAnchorsBodies, extraParallelAnchorsCamera, extraParallelAnchorsGroundSize } from "./parallel-anchors-scene";

const half = extraParallelAnchorsGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: extraParallelAnchorsBodies(),
  camera: extraParallelAnchorsCamera,
  info: "createHumanParallelAnchors + getHumanAnchorBody (bones are rendered; anchors are extra kinematic helpers)",
  getInfo: (workerState) => {
    const buffer = workerState?.extra?.parallelAnchorsHud;
    if (!(buffer instanceof SharedArrayBuffer)) return spec.info;
    const v = new Int32Array(buffer);
    return `bone bodies = ${v[0] ?? 0}\nnon-zero parallel anchors = ${v[1] ?? 0}`;
  },
};

export const extraParallelAnchorsSample = createGenericSample(
  "extra/parallel-anchors",
  "Extra / Parallel Anchors",
  spec,
  () => new Worker(new URL("./parallel-anchors.worker.ts", import.meta.url), { type: "module" }),
);
