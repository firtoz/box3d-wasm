import { createShaderInstancedSample } from "../shader-instanced-host";
import { SNAPSHOT_BODY_COUNT_INDEX } from "../../physics-worker-protocol";
import { Box3DRng } from "../box3d-rng";
import {
  DESTRUCTION_BOX_COLOR,
  DESTRUCTION_BOX_SIZE,
  DESTRUCTION_GRID_COUNT,
  DESTRUCTION_MAX_BODY_COUNT,
  destructionCamera,
  destructionGroundSize,
  forEachDestructionBox,
} from "./destruction-scene";

const half = destructionGroundSize();

export const destructionSample = createShaderInstancedSample({
  id: "benchmark/destruction",
  name: "Benchmark / Destruction",
  createWorker: () => new Worker(new URL("./destruction.worker.ts", import.meta.url), { type: "module" }),
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "plane",
  groundPosition: [0, 0, 0],
  camera: destructionCamera!,
  defaultColor: DESTRUCTION_BOX_COLOR,
  resolveBodyCount: (state, readyCount) => Atomics.load(state, SNAPSHOT_BODY_COUNT_INDEX) || readyCount || 0,
  layers: [
    {
      capacity: DESTRUCTION_MAX_BODY_COUNT,
      geometry: { kind: "box", size: DESTRUCTION_BOX_SIZE },
      bind: { mode: "direct", bodyOffset: 0 },
      colors: "snapshot",
      fixedColor: DESTRUCTION_BOX_COLOR,
      forEachInstance: (callback) => {
        forEachDestructionBox(new Box3DRng(), (position) => callback(position, DESTRUCTION_BOX_COLOR));
      },
    },
  ],
  info: ({ workerCount, colorMode, bodyCount }) =>
    `${bodyCount}/${DESTRUCTION_MAX_BODY_COUNT} boxes (${DESTRUCTION_GRID_COUNT}³ grid, ~½ filled) | respawn @140 | shader | ${workerCount} workers | ${colorMode} colors (C)`,
});
