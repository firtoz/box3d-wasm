import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { disposeObject3D } from "../grid-mesh-visual";
import { f32 } from "../f32";
import {
  heightFieldBodies,
  heightFieldCamera,
  heightFieldGroundSize,
  heightFieldVisual,
} from "./height-field-scene";

function createWaveHeightFieldVisual(scene: THREE.Scene): THREE.Group {
  const { rowCount, columnCount, scale, rowFrequency, columnFrequency, position } = heightFieldVisual;
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

const half = heightFieldGroundSize();

export const heightFieldSample: DemoSample = {
  id: "mesh/height-field",
  name: "Mesh / Height Field",
  create(runtime, scene, solverParams) {
    let waveVisual: THREE.Group | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: heightFieldBodies,
      camera: heightFieldCamera,
      info: "400×400 wave heightfield (NDEBUG defaults; dynamics disabled upstream)",
      overlay: (overlayScene) => {
        waveVisual = createWaveHeightFieldVisual(overlayScene);
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
      "mesh/height-field",
      "Mesh / Height Field",
      spec,
      () => new Worker(new URL("./height-field.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
