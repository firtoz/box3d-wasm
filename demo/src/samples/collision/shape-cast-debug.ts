import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { capsuleMesh } from "../shared";
import {
  createDebugLine,
  disposeDebugObject,
  updateDebugLine,
} from "../debug-overlay";
import {
  shapeCastDebugCamera,
  shapeCastDebugCapsule,
  shapeCastDebugGroundSize,
  shapeCastDebugTransform,
  shapeCastDebugTranslation,
  shapeCastDebugTriangle,
} from "./shape-cast-debug-scene";

const half = shapeCastDebugGroundSize();

function addTriangle(scene: THREE.Scene, color: number): THREE.LineSegments {
  const [a, b, c] = shapeCastDebugTriangle;
  const geometry = new THREE.BufferGeometry();
  geometry.setAttribute("position", new THREE.BufferAttribute(new Float32Array([...a, ...b, ...b, ...c, ...c, ...a]), 3));
  const lines = new THREE.LineSegments(geometry, new THREE.LineBasicMaterial({ color, toneMapped: false }));
  scene.add(lines);
  return lines;
}

function addCapsule(scene: THREE.Scene, color: number, origin: readonly number[]): THREE.Mesh {
  const c1 = new THREE.Vector3(
    shapeCastDebugCapsule.center1[0],
    shapeCastDebugCapsule.center1[1],
    shapeCastDebugCapsule.center1[2],
  );
  const c2 = new THREE.Vector3(
    shapeCastDebugCapsule.center2[0],
    shapeCastDebugCapsule.center2[1],
    shapeCastDebugCapsule.center2[2],
  );
  const length = c1.distanceTo(c2);
  const mesh = capsuleMesh(shapeCastDebugCapsule.radius, length, color, 0.85, "y");
  const dir = c2.clone().sub(c1);
  if (dir.lengthSq() > 0) {
    mesh.quaternion.setFromUnitVectors(new THREE.Vector3(0, 1, 0), dir.normalize());
  }
  const mid = c1.clone().add(c2).multiplyScalar(0.5);
  mesh.position.set(origin[0] + mid.x, origin[1] + mid.y, origin[2] + mid.z);
  scene.add(mesh);
  return mesh;
}

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: [],
  camera: shapeCastDebugCamera,
  info: "b3ShapeCast of a capsule against a triangle (debug regression)",
  overlay: (scene) => {
    const grid = new THREE.GridHelper(10, 10, 0x4b5563, 0x4b5563);
    scene.add(grid);
    const axes = new THREE.AxesHelper(1);
    scene.add(axes);
    const triangle = addTriangle(scene, 0x22d3ee);
    const start = addCapsule(scene, 0x22c55e, shapeCastDebugTransform.position);
    const endPos = [
      shapeCastDebugTransform.position[0] + shapeCastDebugTranslation[0],
      shapeCastDebugTransform.position[1] + shapeCastDebugTranslation[1],
      shapeCastDebugTransform.position[2] + shapeCastDebugTranslation[2],
    ];
    const miss = addCapsule(scene, 0x9ca3af, endPos);
    const hit = addCapsule(scene, 0xef4444, shapeCastDebugTransform.position);
    hit.visible = false;
    const travel = createDebugLine(scene, 0xfacc15);
    updateDebugLine(travel, shapeCastDebugTransform.position, endPos);

    return {
      update({ workerState }) {
        const buffer = workerState?.extra?.cast;
        if (!(buffer instanceof SharedArrayBuffer)) return;
        const values = new Float32Array(buffer);
        if (values[0] === 1) {
          const fraction = values[1] ?? 0;
          hit.position.set(
            start.position.x + fraction * shapeCastDebugTranslation[0],
            start.position.y + fraction * shapeCastDebugTranslation[1],
            start.position.z + fraction * shapeCastDebugTranslation[2],
          );
          hit.visible = true;
        } else {
          hit.visible = false;
        }
      },
      dispose() {
        scene.remove(grid);
        grid.geometry.dispose();
        const gridMaterial = grid.material;
        if (Array.isArray(gridMaterial)) gridMaterial.forEach((material) => material.dispose());
        else gridMaterial.dispose();
        scene.remove(axes);
        axes.dispose();
        for (const object of [triangle, start, miss, hit]) {
          scene.remove(object);
          const mesh = object as THREE.Mesh;
          mesh.geometry.dispose();
          const material = mesh.material;
          if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
          else material.dispose();
        }
        disposeDebugObject(scene, travel);
      },
    };
  },
};

export const shapeCastDebugSample = createGenericSample(
  "collision/shape-cast-debug",
  "Collision / Shape Cast Debug",
  spec,
  () => new Worker(new URL("./shape-cast-debug.worker.ts", import.meta.url), { type: "module" }),
);
