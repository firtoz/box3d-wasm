import * as THREE from "three";
import { ConvexGeometry } from "three/examples/jsm/geometries/ConvexGeometry.js";
import { createShaderInstancedSample } from "../shader-instanced-host";
import {
  CONVEX_PILE_BODY_COUNT,
  CONVEX_PILE_COLOR,
  convexPileCamera,
  convexPileGroundSize,
  convexPilePoints,
  forEachConvexPileBody,
} from "./convex-pile-scene";

const half = convexPileGroundSize();

function createConvexPileGeometry(): THREE.BufferGeometry {
  const flat = convexPilePoints();
  const vectors: THREE.Vector3[] = [];
  for (let i = 0; i < flat.length; i += 3) {
    vectors.push(new THREE.Vector3(flat[i], flat[i + 1], flat[i + 2]));
  }
  return new ConvexGeometry(vectors);
}

export const convexPileSample = createShaderInstancedSample({
  id: "benchmark/convex-pile",
  name: "Benchmark / Convex Pile",
  createWorker: () => new Worker(new URL("./convex-pile.worker.ts", import.meta.url), { type: "module" }),
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  camera: convexPileCamera!,
  defaultColor: CONVEX_PILE_COLOR,
  layers: [
    {
      capacity: CONVEX_PILE_BODY_COUNT,
      geometry: { kind: "geometry", create: createConvexPileGeometry },
      bind: { mode: "direct", bodyOffset: 0 },
      colors: "snapshot",
      fixedColor: CONVEX_PILE_COLOR,
      forEachInstance: (callback) => {
        forEachConvexPileBody((position) => callback(position, CONVEX_PILE_COLOR));
      },
    },
  ],
  info: ({ workerCount, colorMode }) =>
    `${CONVEX_PILE_BODY_COUNT} convexes (8×8×80, seed 42) | ${workerCount} workers | ${colorMode} colors (C)`,
});
