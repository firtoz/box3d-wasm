import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { createGridMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import { crashBodies, crashCamera, crashGroundSize } from "./crash-scene";

const half = crashGroundSize();

export const crashSample: DemoSample = {
  id: "issues/crash",
  name: "Issues / Crash",
  create(runtime, scene, solverParams) {
    let gridVisual: THREE.Group | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: crashBodies,
      camera: crashCamera,
      controls: [{ type: "button", label: "Add Joint", message: { type: "add-joint" } }],
      info: "mesh floor + two boxes | Add Joint welds them",
      overlay: (overlayScene) => {
        // Upstream: createGridMesh(20, 20, 2) at y=-1, scale 1.
        gridVisual = createGridMeshVisual(overlayScene, {
          cellCount: 20,
          cellWidth: 2,
          position: [0, -1, 0],
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
      "issues/crash",
      "Issues / Crash",
      spec,
      () => new Worker(new URL("./crash.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
