import * as THREE from "three";

export function createDebugLine(scene: THREE.Scene, color: number): THREE.Line {
  const geometry = new THREE.BufferGeometry();
  geometry.setAttribute("position", new THREE.BufferAttribute(new Float32Array(6), 3));
  const material = new THREE.LineBasicMaterial({ color, toneMapped: false });
  const line = new THREE.Line(geometry, material);
  scene.add(line);
  return line;
}

export function updateDebugLine(line: THREE.Line, start: readonly number[], end: readonly number[]): void {
  const positions = line.geometry.getAttribute("position") as THREE.BufferAttribute;
  positions.setXYZ(0, start[0], start[1], start[2]);
  positions.setXYZ(1, end[0], end[1], end[2]);
  positions.needsUpdate = true;
  line.geometry.computeBoundingSphere();
}

export function createWireSphere(scene: THREE.Scene, radius: number, color: number): THREE.Mesh {
  const mesh = new THREE.Mesh(
    new THREE.SphereGeometry(radius, 24, 16),
    new THREE.MeshBasicMaterial({ color, wireframe: true, toneMapped: false }),
  );
  scene.add(mesh);
  return mesh;
}

export function disposeDebugObject(scene: THREE.Scene, object: THREE.Object3D): void {
  scene.remove(object);
  const geometry = (object as THREE.Mesh).geometry;
  if (geometry !== undefined) geometry.dispose();
  const material = (object as THREE.Mesh).material;
  if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
  else if (material !== undefined) material.dispose();
}
