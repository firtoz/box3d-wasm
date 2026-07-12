import { createShaderInstancedSample } from "../shader-instanced-host";
import { RAGDOLL_RENDER_BONE_COUNT, SNAPSHOT_BODY_COUNT_INDEX } from "../../physics-worker-protocol";
import { RAGDOLL_RENDER_BONES, ragdollCapsuleGeometry } from "../../ragdoll-render";
import { rainCamera, rainGroundSize, rainMaxHumanCount, rainTileCount } from "./rain-scene";

const TILE_COUNT = rainTileCount();
const MAX_HUMANS = rainMaxHumanCount();
const half = rainGroundSize();

function humanCountFromBodies(bodyCount: number): number {
  return Math.max(0, Math.min(MAX_HUMANS, Math.floor((bodyCount - TILE_COUNT) / RAGDOLL_RENDER_BONE_COUNT)));
}

export const rainSample = createShaderInstancedSample({
  id: "benchmark/rain",
  name: "Benchmark / Rain",
  createWorker: () => new Worker(new URL("./rain.worker.ts", import.meta.url), { type: "module" }),
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "plane",
  groundPosition: [0, 0, 0],
  camera: rainCamera!,
  projectile: { radius: 0.25, color: 0xffa500, metalness: 0.75 },
  resolveBodyCount: (state, readyCount) => Atomics.load(state, SNAPSHOT_BODY_COUNT_INDEX) || readyCount || TILE_COUNT,
  layers: RAGDOLL_RENDER_BONES.map((bone, boneIndex) => ({
    id: `bone-${boneIndex}`,
    capacity: MAX_HUMANS,
    geometry: {
      kind: "geometry" as const,
      create: () => ragdollCapsuleGeometry(bone.a, bone.b, bone.radius),
    },
    bind: {
      mode: "gather" as const,
      bodyIndex: (human, _ctx) => TILE_COUNT + human * RAGDOLL_RENDER_BONE_COUNT + boneIndex,
    },
    colors: "fixed" as const,
    fixedColor: bone.color,
    parkY: -1000,
    resolveInstanceCount: (ctx) => humanCountFromBodies(ctx.bodyCount),
  })),
  getInfo: ({ workerCount, bodyCount }) => {
    const humans = humanCountFromBodies(bodyCount);
    return `Rain | ${humans}/${MAX_HUMANS} humans | ${workerCount} workers`;
  },
  info: "ragdoll rain with instanced capsule shaders",
});
