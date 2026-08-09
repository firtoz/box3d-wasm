import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { disposeObject3D } from "../grid-mesh-visual";
import {
  createHumpMeshBodies,
  humpMeshCamera,
  humpMeshCellWidth,
  humpMeshGroundSize,
  humpMeshVertices,
} from "./hump-mesh-scene";

const half = humpMeshGroundSize();

function createHumpVisual(scene: THREE.Scene): THREE.Group {
  const { vertices, indices } = humpMeshVertices(humpMeshCellWidth);
  const positions = new Float32Array(vertices);
  const geom = new THREE.BufferGeometry();
  geom.setAttribute("position", new THREE.BufferAttribute(positions, 3));
  geom.setIndex(indices);
  geom.computeVertexNormals();

  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({
      color: 0x64748b,
      roughness: 0.9,
      side: THREE.DoubleSide,
      flatShading: true,
    }),
  );
  fill.receiveShadow = true;
  const edges = new THREE.LineSegments(
    new THREE.WireframeGeometry(geom),
    new THREE.LineBasicMaterial({ color: 0x94a3b8 }),
  );
  const root = new THREE.Group();
  root.add(fill);
  root.add(edges);
  scene.add(root);
  return root;
}

export const humpMeshSample: DemoSample = {
  id: "continuous/hump-mesh",
  name: "Continuous / Hump Mesh",
  create(runtime, scene, solverParams) {
    let humpVisual: THREE.Group | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      bodies: createHumpMeshBodies(),
      camera: humpMeshCamera,
      info: "thin box onto median-split hump mesh",
      overlay: (overlayScene) => {
        humpVisual = createHumpVisual(overlayScene);
        return {
          update() {},
          dispose() {
            if (humpVisual !== null) disposeObject3D(overlayScene, humpVisual);
            humpVisual = null;
          },
        };
      },
    };

    return createGenericSample(
      "continuous/hump-mesh",
      "Continuous / Hump Mesh",
      spec,
      () => new Worker(new URL("./hump-mesh.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
