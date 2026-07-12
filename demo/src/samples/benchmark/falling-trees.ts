import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { createWaveMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import {
  createFallingTreesBodies,
  fallingTreesCamera,
  fallingTreesGroundSize,
  type TreeScaleCm,
  waveMeshParams,
  treeScaleFromCm,
} from "./falling-trees-scene";

const half = fallingTreesGroundSize();

export const fallingTreesSample: DemoSample = {
  id: "benchmark/falling-trees",
  name: "Benchmark / Falling Trees",
  create(runtime, scene, solverParams) {
    let waveVisual: THREE.Group | null = null;
    let overlayScene: THREE.Scene | null = null;

    function rebuildWave(cm: TreeScaleCm): void {
      if (overlayScene === null) return;
      if (waveVisual !== null) disposeObject3D(overlayScene, waveVisual);
      waveVisual = createWaveMeshVisual(overlayScene, waveMeshParams(treeScaleFromCm(cm)));
    }

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: createFallingTreesBodies(),
      camera: fallingTreesCamera,
      info: "50 compound trees on wave mesh — 100/50/25 cm ground (matches upstream radios)",
      overlay: (overlay) => {
        overlayScene = overlay;
        rebuildWave(100);
        return {
          update() {},
          dispose() {
            if (waveVisual !== null && overlayScene !== null) disposeObject3D(overlayScene, waveVisual);
            waveVisual = null;
            overlayScene = null;
          },
        };
      },
      controls: [
        {
          type: "button",
          label: "100cm",
          message: { type: "set-tree-scale", cm: 100 },
          onHostClick: () => rebuildWave(100),
        },
        {
          type: "button",
          label: "50cm",
          message: { type: "set-tree-scale", cm: 50 },
          onHostClick: () => rebuildWave(50),
        },
        {
          type: "button",
          label: "25cm",
          message: { type: "set-tree-scale", cm: 25 },
          onHostClick: () => rebuildWave(25),
        },
      ],
    };

    return createGenericSample(
      "benchmark/falling-trees",
      "Benchmark / Falling Trees",
      spec,
      () => new Worker(new URL("./falling-trees.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
