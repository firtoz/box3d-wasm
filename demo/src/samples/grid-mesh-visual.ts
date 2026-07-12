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
