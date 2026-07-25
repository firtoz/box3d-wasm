import * as THREE from "three";
import { createShaderInstancedSample } from "../shader-instanced-host";
import { createWaveMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import {
  CHAINS_BODY_COUNT,
  CHAINS_COLOR,
  CHAINS_LINK_EXTENT,
  CHAINS_LINK_RADIUS,
  chainsCamera,
  chainsGroundSize,
  forEachChainsLink,
} from "./chains-scene";

const half = chainsGroundSize();
const CAPSULE_LENGTH = 2 * CHAINS_LINK_EXTENT;

export const chainsSample = createShaderInstancedSample({
  id: "benchmark/chains",
  name: "Benchmark / Chains",
  createWorker: () => new Worker(new URL("./chains.worker.ts", import.meta.url), { type: "module" }),
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  camera: chainsCamera!,
  defaultColor: CHAINS_COLOR,
  setupScene: (scene) => {
    const wave = createWaveMeshVisual(scene, {
      xCount: 80,
      zCount: 80,
      cellWidth: 1,
      amplitude: 0.5,
      rowFrequency: 0.05,
      columnFrequency: 0.01,
    });
    return {
      dispose() {
        disposeObject3D(scene, wave);
      },
    };
  },
  layers: [
    {
      id: "links",
      capacity: CHAINS_BODY_COUNT,
      geometry: {
        kind: "geometry",
        create: () => new THREE.CapsuleGeometry(CHAINS_LINK_RADIUS, CAPSULE_LENGTH, 4, 8),
      },
      bind: { mode: "direct" },
      colors: "fixed",
      fixedColor: CHAINS_COLOR,
      forEachInstance: (callback) => {
        forEachChainsLink((position, color) => callback(position, color));
      },
    },
  ],
  info: ({ workerCount, colorMode }) =>
    `${CHAINS_BODY_COUNT} capsule links (25×25×4) + wind | wave mesh | ${workerCount} workers | ${colorMode} colors (C)`,
});
