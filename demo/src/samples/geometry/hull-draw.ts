import * as THREE from "three";
import { ConvexGeometry } from "three/examples/jsm/geometries/ConvexGeometry.js";
import type { Box3DRuntime, HullHandle } from "box3d-wasm";

export function hullMeshFromHandle(
  runtime: Box3DRuntime,
  hull: HullHandle,
  color: number,
  options: { wireframe?: boolean; opacity?: number } = {},
): THREE.Group {
  const flat = runtime.getHullPoints(hull);
  const points: THREE.Vector3[] = [];
  for (let i = 0; i + 2 < flat.length; i += 3) {
    points.push(new THREE.Vector3(flat[i], flat[i + 1], flat[i + 2]));
  }
  const geom = new ConvexGeometry(points);
  const group = new THREE.Group();
  const opacity = options.opacity ?? 0.18;
  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshBasicMaterial({
      color,
      transparent: true,
      opacity,
      side: THREE.DoubleSide,
      depthWrite: false,
    }),
  );
  fill.castShadow = false;
  fill.receiveShadow = false;
  group.add(fill);
  if (options.wireframe !== false) {
    group.add(new THREE.LineSegments(new THREE.EdgesGeometry(geom), new THREE.LineBasicMaterial({ color })));
  }
  return group;
}

export function disposeHullMesh(scene: THREE.Scene, object: THREE.Object3D | null): void {
  if (object === null) return;
  scene.remove(object);
  object.traverse((child) => {
    if (child instanceof THREE.Mesh || child instanceof THREE.LineSegments) {
      child.geometry.dispose();
      const material = child.material;
      if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
      else material.dispose();
    }
  });
}
