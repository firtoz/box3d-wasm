import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { createGridMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import {
  persistentContactBodies,
  persistentContactCamera,
  persistentContactGroundSize,
} from "./persistent-contact-scene";

const half = persistentContactGroundSize();

export const persistentContactSample: DemoSample = {
  id: "events/persistent-contact",
  name: "Events / Persistent Contact",
  create(runtime, scene, solverParams) {
    let gridVisual: THREE.Group | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: persistentContactBodies,
      camera: persistentContactCamera,
      info: "dense sphere on grid mesh · contact events",
      overlay: (overlayScene) => {
        gridVisual = createGridMeshVisual(overlayScene, {
          cellCount: 20,
          cellWidth: 2,
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
      "events/persistent-contact",
      "Events / Persistent Contact",
      spec,
      () => new Worker(new URL("./persistent-contact.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
