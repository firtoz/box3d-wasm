import * as THREE from "three";
import { createShaderInstancedSample } from "../shader-instanced-host";
import { SNAPSHOT_BODY_COUNT_INDEX } from "../../physics-worker-protocol";
import {
  forEachLargeWorldTile,
  LARGE_WORLD_CELL,
  LARGE_WORLD_GRID,
  LARGE_WORLD_SPHERE_COLOR,
  LARGE_WORLD_SPHERE_RADIUS,
  LARGE_WORLD_SPHERES,
  largeWorldCamera,
  largeWorldTileCount,
} from "./large-world-scene";

const TILE_COLOR = 0x222222;
const TILE_FULL = LARGE_WORLD_CELL; // 10 — matches hull half [5, 0.25, 5]

export const largeWorldSample = createShaderInstancedSample({
  id: "benchmark/large-world",
  name: "Benchmark / Large World",
  createWorker: () => new Worker(new URL("./large-world.worker.ts", import.meta.url), { type: "module" }),
  // Floor tiles are drawn in setupScene to match upstream per-tile boxes (top at y=0.25).
  groundSize: [1, 1, 1],
  groundKind: "none",
  camera: largeWorldCamera!,
  defaultColor: LARGE_WORLD_SPHERE_COLOR,
  resolveBodyCount: (state, readyCount) => Atomics.load(state, SNAPSHOT_BODY_COUNT_INDEX) || readyCount || 0,
  layers: [
    {
      capacity: LARGE_WORLD_SPHERES,
      geometry: { kind: "sphere", radius: LARGE_WORLD_SPHERE_RADIUS },
      bind: { mode: "direct", bodyOffset: 0 },
      colors: "snapshot",
      fixedColor: LARGE_WORLD_SPHERE_COLOR,
      parkY: -1000,
      resolveInstanceCount: (ctx) => Math.min(LARGE_WORLD_SPHERES, ctx.bodyCount),
    },
  ],
  setupScene(scene) {
    const geom = new THREE.BoxGeometry(TILE_FULL, 0.5, TILE_FULL);
    const mat = new THREE.MeshStandardMaterial({ color: TILE_COLOR, roughness: 0.9, metalness: 0 });
    const mesh = new THREE.InstancedMesh(geom, mat, largeWorldTileCount());
    mesh.receiveShadow = true;
    mesh.instanceMatrix.setUsage(THREE.StaticDrawUsage);
    const m = new THREE.Matrix4();
    let i = 0;
    forEachLargeWorldTile((position) => {
      m.makeTranslation(position[0], position[1], position[2]);
      mesh.setMatrixAt(i++, m);
    });
    mesh.instanceMatrix.needsUpdate = true;
    scene.add(mesh);
    return {
      dispose() {
        scene.remove(mesh);
        geom.dispose();
        mat.dispose();
      },
    };
  },
  info: `debug ${LARGE_WORLD_GRID}×${LARGE_WORLD_GRID} tiles (release C++ is 1000×1000) + up to ${LARGE_WORLD_SPHERES} spheres`,
  getInfo: ({ workerCount, colorMode, bodyCount }) =>
    `debug ${LARGE_WORLD_GRID}×${LARGE_WORLD_GRID} tiles (release C++ is 1000×1000) + ${bodyCount}/${LARGE_WORLD_SPHERES} spheres | ${workerCount} workers | ${colorMode} colors (C)`,
});
