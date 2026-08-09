import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { createWaveMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import {
  createFarMeshDropBodies,
  farMeshDropCamera,
  farMeshDropGroundSize,
  farMeshDropWaveParams,
} from "./far-mesh-drop-scene";

const half = farMeshDropGroundSize();

export const farMeshDropSample: DemoSample = {
  id: "world/far-mesh-drop",
  name: "World / Far Mesh Drop",
  create(runtime, scene, solverParams) {
    let waveVisual: THREE.Group | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: createFarMeshDropBodies(),
      camera: farMeshDropCamera,
      info: "CreateMeshDrop at 1000 km from origin",
      overlay: (overlayScene) => {
        waveVisual = createWaveMeshVisual(overlayScene, farMeshDropWaveParams);
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
      "world/far-mesh-drop",
      "World / Far Mesh Drop",
      spec,
      () => new Worker(new URL("./far-mesh-drop.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
