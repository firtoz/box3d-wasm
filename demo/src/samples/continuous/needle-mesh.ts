import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { disposeObject3D } from "../grid-mesh-visual";
import {
  createNeedleMeshBodies,
  needleMeshCamera,
  needleMeshGroundSize,
  needleOverlaySpecs,
} from "./needle-mesh-scene";

const half = needleMeshGroundSize();

function createNeedleVisual(scene: THREE.Scene): THREE.Group {
  const root = new THREE.Group();
  const mat = new THREE.MeshStandardMaterial({
    color: 0x64748b,
    roughness: 0.85,
    side: THREE.DoubleSide,
    flatShading: true,
  });
  for (const spec of needleOverlaySpecs) {
    const geom = new THREE.ConeGeometry(spec.radius, spec.height, 8, 1, true);
    // ConeGeometry is Y-up, centered; tip at +height/2. Needle tip is at y=height, base at y=0.
    const mesh = new THREE.Mesh(geom, mat);
    mesh.position.set(spec.center[0], spec.center[1] + spec.height / 2, spec.center[2]);
    root.add(mesh);
  }
  scene.add(root);
  return root;
}

export const needleMeshSample: DemoSample = {
  id: "continuous/needle-mesh",
  name: "Continuous / Needle Mesh",
  create(runtime, scene, solverParams) {
    let needleVisual: THREE.Group | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: createNeedleMeshBodies(),
      camera: needleMeshCamera,
      info: "thin box onto 4 needle meshes",
      overlay: (overlayScene) => {
        needleVisual = createNeedleVisual(overlayScene);
        return {
          update() {},
          dispose() {
            if (needleVisual !== null) disposeObject3D(overlayScene, needleVisual);
            needleVisual = null;
          },
        };
      },
    };

    return createGenericSample(
      "continuous/needle-mesh",
      "Continuous / Needle Mesh",
      spec,
      () => new Worker(new URL("./needle-mesh.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
