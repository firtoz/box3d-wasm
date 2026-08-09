import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { capsuleMesh } from "../shared";
import { capsuleMassBodies, capsuleMassCamera, capsuleMassGroundSize } from "./capsule-mass-scene";

const half = capsuleMassGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: capsuleMassBodies,
  camera: capsuleMassCamera,
  info: "Geometry-only capsule mass visualization (no physics bodies)",
  overlay: (scene) => {
    const capsule = capsuleMesh(1, 2, 0x22d3ee, 0.5, "x");
    const capsuleMaterial = capsule.material as THREE.Material;
    capsuleMaterial.transparent = true;
    capsuleMaterial.opacity = 0.65;
    scene.add(capsule);

    const box = new THREE.Mesh(
      new THREE.BoxGeometry(4, 2, 2),
      new THREE.MeshStandardMaterial({ color: 0x8b5cf6, wireframe: true }),
    );
    scene.add(box);

    const axes = new THREE.AxesHelper(1);
    scene.add(axes);

    return {
      update() {},
      dispose() {
        scene.remove(capsule);
        capsule.geometry.dispose();
        capsuleMaterial.dispose();
        scene.remove(box);
        box.geometry.dispose();
        box.material.dispose();
        scene.remove(axes);
        axes.dispose();
      },
    };
  },
};

export const capsuleMassSample = createGenericSample(
  "geometry/capsule-mass",
  "Geometry / Capsule Mass",
  spec,
  () => new Worker(new URL("./capsule-mass.worker.ts", import.meta.url), { type: "module" }),
);
