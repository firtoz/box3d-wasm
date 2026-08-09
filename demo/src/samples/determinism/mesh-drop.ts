import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { createWaveMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import {
  createMeshDropDeterminismBodies,
  meshDropDeterminismCamera,
  meshDropDeterminismGroundSize,
  meshDropDeterminismWaveParams,
} from "./mesh-drop-scene";

const half = meshDropDeterminismGroundSize();

export const meshDropDeterminismSample: DemoSample = {
  id: "determinism/mesh-drop",
  name: "Determinism / Mesh Drop",
  create(runtime, scene, solverParams) {
    let waveVisual: THREE.Group | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: createMeshDropDeterminismBodies(),
      camera: meshDropDeterminismCamera,
      info: "20×20 boxes on wave mesh (CreateMeshDrop)",
      overlay: (overlayScene) => {
        waveVisual = createWaveMeshVisual(overlayScene, meshDropDeterminismWaveParams);
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
      "determinism/mesh-drop",
      "Determinism / Mesh Drop",
      spec,
      () => new Worker(new URL("./mesh-drop.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
