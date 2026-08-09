import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import {
  QUERY_SPAWN_COUNT,
  createQuerySpawnBodies,
  querySpawnCamera,
  querySpawnGroundSize,
} from "./query-spawn-scene";

const half = querySpawnGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: createQuerySpawnBodies(),
  camera: querySpawnCamera,
  info: `zero-g query spawn · up to ${QUERY_SPAWN_COUNT} bodies (ray/overlap/sphere-cast)`,
};

export const querySpawnSample = createGenericSample(
  "determinism/query-spawn",
  "Determinism / Query Spawn",
  spec,
  () => new Worker(new URL("./query-spawn.worker.ts", import.meta.url), { type: "module" }),
);
