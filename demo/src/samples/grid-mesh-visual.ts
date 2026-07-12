import * as THREE from "three";

/** Visual for `b3CreateGridMesh` + mesh shape (fill + triangle edges). */
export function createGridMeshVisual(
  scene: THREE.Scene,
  options: {
    cellCount: number;
    cellWidth: number;
    scale?: [number, number, number];
    position?: [number, number, number];
    fillColor?: number;
    edgeColor?: number;
  },
): THREE.Group {
  const size = options.cellCount * options.cellWidth;
  const geom = new THREE.PlaneGeometry(size, size, options.cellCount, options.cellCount);
  geom.rotateX(-Math.PI / 2);

  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({
      color: options.fillColor ?? 0x2a2a2a,
      roughness: 0.9,
      metalness: 0,
      side: THREE.DoubleSide,
      flatShading: true,
    }),
  );
  fill.receiveShadow = true;

  const edges = new THREE.LineSegments(
    new THREE.WireframeGeometry(geom),
    new THREE.LineBasicMaterial({ color: options.edgeColor ?? 0x6b7280 }),
  );

  const root = new THREE.Group();
  root.add(fill);
  root.add(edges);
  const pos = options.position ?? [0, 0, 0];
  root.position.set(pos[0], pos[1], pos[2]);
  const scale = options.scale ?? [1, 1, 1];
  root.scale.set(scale[0], scale[1], scale[2]);
  scene.add(root);
  return root;
}

/** Visual for `b3CreateWaveMesh` (same vertex/triangle layout and `sinf` heights). */
export function createWaveMeshVisual(
  scene: THREE.Scene,
  options: {
    xCount: number;
    zCount: number;
    cellWidth: number;
    amplitude: number;
    rowFrequency: number;
    columnFrequency: number;
    position?: [number, number, number];
    fillColor?: number;
    edgeColor?: number;
  },
): THREE.Group {
  const { xCount, zCount, cellWidth, amplitude, rowFrequency, columnFrequency } = options;
  const xWidth = cellWidth * xCount;
  const zWidth = cellWidth * zCount;
  const omegaZ = 2 * Math.PI * rowFrequency * cellWidth;
  const omegaX = 2 * Math.PI * columnFrequency * cellWidth;

  const vertexCount = (xCount + 1) * (zCount + 1);
  const positions = new Float32Array(vertexCount * 3);
  let index = 0;
  let x = -0.5 * xWidth;
  for (let ix = 0; ix <= xCount; ix++) {
    const rowHeight = Math.fround(Math.sin(Math.fround(omegaX * ix)));
    let z = -0.5 * zWidth;
    for (let iz = 0; iz <= zCount; iz++) {
      const columnHeight = Math.fround(Math.sin(Math.fround(omegaZ * iz)));
      const y = Math.fround(amplitude * rowHeight * columnHeight);
      positions[index * 3] = x;
      positions[index * 3 + 1] = y;
      positions[index * 3 + 2] = z;
      z += cellWidth;
      index += 1;
    }
    x += cellWidth;
  }

  const indices: number[] = [];
  for (let ix = 0; ix < xCount; ix++) {
    for (let iz = 0; iz < zCount; iz++) {
      const index1 = iz + (zCount + 1) * ix;
      const index2 = index1 + 1;
      const index3 = index2 + (zCount + 1);
      const index4 = index3 - 1;
      indices.push(index1, index2, index3, index3, index4, index1);
    }
  }

  const geom = new THREE.BufferGeometry();
  geom.setAttribute("position", new THREE.BufferAttribute(positions, 3));
  geom.setIndex(indices);
  geom.computeVertexNormals();

  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({
      color: options.fillColor ?? 0x2a2a2a,
      roughness: 0.9,
      metalness: 0,
      side: THREE.DoubleSide,
      flatShading: true,
    }),
  );
  fill.receiveShadow = true;

  const edges = new THREE.LineSegments(
    new THREE.WireframeGeometry(geom),
    new THREE.LineBasicMaterial({ color: options.edgeColor ?? 0x6b7280 }),
  );

  const root = new THREE.Group();
  root.add(fill);
  root.add(edges);
  const p = options.position ?? [0, 0, 0];
  root.position.set(p[0], p[1], p[2]);
  scene.add(root);
  return root;
}

export function disposeObject3D(scene: THREE.Scene, root: THREE.Object3D): void {
  scene.remove(root);
  root.traverse((obj) => {
    const mesh = obj as THREE.Mesh | THREE.LineSegments;
    if (mesh.geometry) mesh.geometry.dispose();
    const material = mesh.material;
    if (material === undefined) return;
    if (Array.isArray(material)) material.forEach((m) => m.dispose());
    else material.dispose();
  });
}
