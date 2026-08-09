import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { createWaveMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import { f32 } from "../f32";
import {
  longRayCastBodies,
  longRayCastCamera,
  longRayCastGroundSize,
  longRayCastHeightFieldVisual,
  longRayCastWaveMeshVisual,
} from "./long-ray-cast-scene";

function createWaveHeightFieldVisual(scene: THREE.Scene): THREE.Group {
  const { rowCount, columnCount, scale, rowFrequency, columnFrequency, position } = longRayCastHeightFieldVisual;
  const positions = new Float32Array(rowCount * columnCount * 3);
  let cursor = 0;
  for (let row = 0; row < rowCount; row++) {
    const rowHeight = f32(Math.sin(f32(2 * Math.PI * rowFrequency * row)));
    for (let column = 0; column < columnCount; column++) {
      const columnHeight = f32(Math.sin(f32(2 * Math.PI * columnFrequency * column)));
      positions[cursor++] = f32(column * scale[0]);
      positions[cursor++] = f32(scale[1] * rowHeight * columnHeight);
      positions[cursor++] = f32(row * scale[2]);
    }
  }

  const indices: number[] = [];
  for (let row = 0; row < rowCount - 1; row++) {
    for (let column = 0; column < columnCount - 1; column++) {
      const i1 = row * columnCount + column;
      const i2 = i1 + 1;
      const i3 = i2 + columnCount;
      const i4 = i3 - 1;
      indices.push(i1, i2, i3, i3, i4, i1);
    }
  }

  const geom = new THREE.BufferGeometry();
  geom.setAttribute("position", new THREE.BufferAttribute(positions, 3));
  geom.setIndex(indices);
  geom.computeVertexNormals();

  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({
      color: 0x2a2a2a,
      roughness: 0.9,
      side: THREE.DoubleSide,
      flatShading: true,
    }),
  );
  fill.receiveShadow = true;

  const edges = new THREE.LineSegments(
    new THREE.WireframeGeometry(geom),
    new THREE.LineBasicMaterial({ color: 0x64748b }),
  );

  const root = new THREE.Group();
  root.add(fill);
  root.add(edges);
  root.position.set(position[0], position[1], position[2]);
  scene.add(root);
  return root;
}

const half = longRayCastGroundSize();

export const longRayCastSample: DemoSample = {
  id: "collision/long-ray-cast",
  name: "Collision / Long Ray Cast",
  create(runtime, scene, solverParams) {
    let waveMesh: THREE.Group | null = null;
    let heightFieldVisual: THREE.Group | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: longRayCastBodies,
      camera: longRayCastCamera,
      info: "5 static targets (sphere/capsule/rock/wave mesh/heightfield) · long-ray accuracy demo",
      overlay: (overlayScene) => {
        waveMesh = createWaveMeshVisual(overlayScene, {
          ...longRayCastWaveMeshVisual,
          position: [...longRayCastWaveMeshVisual.position],
        });
        heightFieldVisual = createWaveHeightFieldVisual(overlayScene);
        const groundGrid = new THREE.GridHelper(40, 40, 0x4b5563, 0x4b5563);
        overlayScene.add(groundGrid);
        return {
          update() {},
          dispose() {
            if (waveMesh !== null) disposeObject3D(overlayScene, waveMesh);
            waveMesh = null;
            if (heightFieldVisual !== null) disposeObject3D(overlayScene, heightFieldVisual);
            heightFieldVisual = null;
            overlayScene.remove(groundGrid);
            groundGrid.geometry.dispose();
            const mat = groundGrid.material;
            if (Array.isArray(mat)) mat.forEach((m) => m.dispose());
            else mat.dispose();
          },
        };
      },
    };

    const instance = createGenericSample(
      "collision/long-ray-cast",
      "Collision / Long Ray Cast",
      spec,
      () => new Worker(new URL("./long-ray-cast.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
    // Placeholder slots for wave mesh / heightfield — real geometry is in the overlay.
    for (const index of [3, 4]) {
      const mesh = instance.bodies[index]?.mesh;
      if (mesh !== undefined) mesh.visible = false;
    }
    return instance;
  },
};
