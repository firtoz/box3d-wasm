import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { createWaveMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import {
  createContinuousMeshDropBodies,
  continuousMeshDropCamera,
  continuousMeshDropGroundSize,
  continuousMeshDropWaveParams,
} from "./mesh-drop-scene";

const half = continuousMeshDropGroundSize();

export const continuousMeshDropSample: DemoSample = {
  id: "continuous/mesh-drop",
  name: "Continuous / Mesh Drop",
  create(runtime, scene, solverParams) {
    let waveVisual: THREE.Group | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: createContinuousMeshDropBodies(),
      camera: continuousMeshDropCamera,
      info: "32×32 boxes on wave mesh + walls (seed 1910133196)",
      overlay: (overlayScene) => {
        waveVisual = createWaveMeshVisual(overlayScene, continuousMeshDropWaveParams);
        return {
          update() {},
          dispose() {
            if (waveVisual !== null) disposeObject3D(overlayScene, waveVisual);
            waveVisual = null;
          },
        };
      },
    };

    return createGenericSample(
      "continuous/mesh-drop",
      "Continuous / Mesh Drop",
      spec,
      () => new Worker(new URL("./mesh-drop.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
