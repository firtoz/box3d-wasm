import * as THREE from "three";
import { ConvexGeometry } from "three/examples/jsm/geometries/ConvexGeometry.js";
import { createShaderInstancedSample } from "../shader-instanced-host";
import {
  CANDY_CUPS_COLOR,
  CANDY_CUPS_COUNT,
  candyCupPoints,
  candyCupsCamera,
  candyCupsGroundSize,
  forEachCandyCup,
} from "./candy-cups-scene";

const half = candyCupsGroundSize();

function createCandyCupGeometry(): THREE.BufferGeometry {
  const flat = candyCupPoints();
  const vectors: THREE.Vector3[] = [];
  for (let i = 0; i < flat.length; i += 3) {
    vectors.push(new THREE.Vector3(flat[i], flat[i + 1], flat[i + 2]));
  }
  return new ConvexGeometry(vectors);
}

export const candyCupsSample = createShaderInstancedSample({
  id: "benchmark/candy-cups",
  name: "Benchmark / Candy Cups",
  createWorker: () => new Worker(new URL("./candy-cups.worker.ts", import.meta.url), { type: "module" }),
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  camera: candyCupsCamera!,
  defaultColor: CANDY_CUPS_COLOR,
  layers: [
    {
      capacity: CANDY_CUPS_COUNT,
      geometry: { kind: "geometry", create: createCandyCupGeometry },
      bind: { mode: "direct", bodyOffset: 0 },
      colors: "snapshot",
      fixedColor: CANDY_CUPS_COLOR,
      forEachInstance: (callback) => {
        forEachCandyCup((position) => callback(position, CANDY_CUPS_COLOR));
      },
    },
  ],
  info: ({ workerCount, colorMode }) =>
    `${CANDY_CUPS_COUNT} candy cups (16×16×16) | shader render | ${workerCount} workers | ${colorMode} colors (C)`,
});
