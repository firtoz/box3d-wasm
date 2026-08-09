import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { createGridMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import { createHitEventBodies, hitEventCamera, hitEventGroundSize } from "./hit-scene";

const half = hitEventGroundSize();

export const hitEventSample: DemoSample = {
  id: "events/hit",
  name: "Events / Hit",
  create(runtime, scene, solverParams) {
    let gridVisual: THREE.Group | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: createHitEventBodies(),
      camera: hitEventCamera,
      info: "welded capsule chain on grid mesh · hit events",
      overlay: (overlayScene) => {
        gridVisual = createGridMeshVisual(overlayScene, {
          cellCount: 20,
          cellWidth: 8,
          position: [0, 0, 0],
        });
        return {
          update() {},
          dispose() {
            if (gridVisual !== null) disposeObject3D(overlayScene, gridVisual);
            gridVisual = null;
          },
        };
      },
    };

    return createGenericSample(
      "events/hit",
      "Events / Hit",
      spec,
      () => new Worker(new URL("./hit.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
