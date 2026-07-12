import * as THREE from "three";
import { createShaderInstancedSample } from "../shader-instanced-host";
import {
  EXPLOSION_CYLINDER_COLOR,
  EXPLOSION_CYLINDER_COUNT,
  EXPLOSION_CYLINDER_HEIGHT,
  EXPLOSION_CYLINDER_RADIUS,
  EXPLOSION_IMPULSE,
  EXPLOSION_IMPULSE_MAX,
  EXPLOSION_IMPULSE_MIN,
  explosionCamera,
  explosionGroundSize,
  forEachExplosionCylinder,
} from "./explosion-scene";

const half = explosionGroundSize();

function createCylinderGeometry(): THREE.BufferGeometry {
  // Physics cylinder spans local y=[0, height]; Three cylinder is centered at origin.
  const geom = new THREE.CylinderGeometry(EXPLOSION_CYLINDER_RADIUS, EXPLOSION_CYLINDER_RADIUS, EXPLOSION_CYLINDER_HEIGHT, 15);
  geom.translate(0, EXPLOSION_CYLINDER_HEIGHT * 0.5, 0);
  return geom;
}

export const explosionSample = createShaderInstancedSample({
  id: "benchmark/explosion",
  name: "Benchmark / Explosion",
  createWorker: () => new Worker(new URL("./explosion.worker.ts", import.meta.url), { type: "module" }),
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "plane",
  groundPosition: [0, 0, 0],
  camera: explosionCamera!,
  defaultColor: EXPLOSION_CYLINDER_COLOR,
  layers: [
    {
      capacity: EXPLOSION_CYLINDER_COUNT,
      geometry: { kind: "geometry", create: createCylinderGeometry },
      bind: { mode: "direct", bodyOffset: 0 },
      colors: "snapshot",
      fixedColor: EXPLOSION_CYLINDER_COLOR,
      forEachInstance: (callback) => {
        forEachExplosionCylinder((position) => callback(position, EXPLOSION_CYLINDER_COLOR));
      },
    },
  ],
  setupScene(scene) {
    const wallMat = new THREE.MeshStandardMaterial({ color: 0x94a3b8, metalness: 0.1, roughness: 0.8 });
    const walls: THREE.Mesh[] = [
      new THREE.Mesh(new THREE.BoxGeometry(40, 2, 0.2), wallMat),
      new THREE.Mesh(new THREE.BoxGeometry(40, 2, 0.2), wallMat),
      new THREE.Mesh(new THREE.BoxGeometry(0.2, 2, 40), wallMat),
      new THREE.Mesh(new THREE.BoxGeometry(0.2, 2, 40), wallMat),
    ];
    walls[0].position.set(0, 1, -20);
    walls[1].position.set(0, 1, 20);
    walls[2].position.set(-20, 1, 0);
    walls[3].position.set(20, 1, 0);
    for (const wall of walls) {
      wall.receiveShadow = true;
      scene.add(wall);
    }
    return {
      dispose() {
        for (const wall of walls) {
          scene.remove(wall);
          wall.geometry.dispose();
        }
        wallMat.dispose();
      },
    };
  },
  controls: [
    { type: "range", label: "Magnitude", message: { type: "set-explosion-magnitude" }, min: EXPLOSION_IMPULSE_MIN, max: EXPLOSION_IMPULSE_MAX, step: 10, value: EXPLOSION_IMPULSE },
    { type: "button", label: "Explode", message: { type: "explode" } },
  ],
  onKey(key, { worker }) {
    if (key === "e" || key === "E") {
      worker.postMessage({ type: "explode" });
    }
  },
  info: ({ workerCount, colorMode }) =>
    `${EXPLOSION_CYLINDER_COUNT} cylinders | Explode button or E | shader render | ${workerCount} workers | ${colorMode} colors (C)`,
});
