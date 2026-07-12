import * as THREE from "three";
import { ConvexGeometry } from "three/examples/jsm/geometries/ConvexGeometry.js";
import { createShaderInstancedSample } from "../shader-instanced-host";
import {
  forEachJunkyardRock,
  JUNKYARD_PUSHER_COLOR,
  JUNKYARD_PUSHER_CYLINDER_RADIUS,
  JUNKYARD_PUSHER_HEIGHT,
  JUNKYARD_PUSHER_RADIUS,
  JUNKYARD_ROCK_COLOR,
  JUNKYARD_ROCK_COUNT,
  junkyardCamera,
  junkyardGroundSize,
  junkyardRockPoints,
} from "./junkyard-scene";

const half = junkyardGroundSize();

function createRockGeometry(): THREE.BufferGeometry {
  const points = junkyardRockPoints();
  const vectors = points.map(([x, y, z]) => new THREE.Vector3(x, y, z));
  return new ConvexGeometry(vectors);
}

export const junkyardSample = createShaderInstancedSample({
  id: "benchmark/junkyard",
  name: "Benchmark / Junkyard",
  createWorker: () => new Worker(new URL("./junkyard.worker.ts", import.meta.url), { type: "module" }),
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  camera: junkyardCamera!,
  defaultColor: JUNKYARD_ROCK_COLOR,
  layers: [
    {
      capacity: JUNKYARD_ROCK_COUNT,
      geometry: { kind: "geometry", create: createRockGeometry },
      bind: { mode: "direct", bodyOffset: 0 },
      colors: "snapshot",
      fixedColor: JUNKYARD_ROCK_COLOR,
      forEachInstance: (callback) => {
        forEachJunkyardRock((position) => callback(position, JUNKYARD_ROCK_COLOR));
      },
    },
  ],
  setupScene(scene) {
    const floor = new THREE.Mesh(
      new THREE.BoxGeometry(240, 2, 240),
      new THREE.MeshStandardMaterial({ color: 0x222222 }),
    );
    floor.position.set(0, -1, 0);
    floor.receiveShadow = true;
    scene.add(floor);

    const wallMat = new THREE.MeshStandardMaterial({ color: 0x374151 });
    const walls: { size: [number, number, number]; pos: [number, number, number] }[] = [
      { size: [2, 16, 100], pos: [-50, 7, 0] },
      { size: [2, 16, 100], pos: [50, 7, 0] },
      { size: [100, 16, 2], pos: [0, 7, -50] },
      { size: [100, 16, 2], pos: [0, 7, 50] },
    ];
    const wallMeshes = walls.map(({ size, pos }) => {
      const mesh = new THREE.Mesh(new THREE.BoxGeometry(size[0], size[1], size[2]), wallMat);
      mesh.position.set(pos[0], pos[1], pos[2]);
      mesh.receiveShadow = true;
      scene.add(mesh);
      return mesh;
    });

    const pusherGeom = new THREE.CylinderGeometry(
      JUNKYARD_PUSHER_CYLINDER_RADIUS,
      JUNKYARD_PUSHER_CYLINDER_RADIUS,
      JUNKYARD_PUSHER_HEIGHT,
      16,
    );
    const pusher = new THREE.Mesh(
      pusherGeom,
      new THREE.MeshStandardMaterial({ color: JUNKYARD_PUSHER_COLOR }),
    );
    pusher.position.set(JUNKYARD_PUSHER_RADIUS, JUNKYARD_PUSHER_HEIGHT * 0.5, 0);
    pusher.castShadow = true;
    pusher.receiveShadow = true;
    scene.add(pusher);

    const pusherBodyIndex = JUNKYARD_ROCK_COUNT;
    return {
      sync(ctx) {
        const i = pusherBodyIndex;
        if (i >= ctx.bodyCount) return;
        pusher.position.set(ctx.positions[i * 3]!, ctx.positions[i * 3 + 1]!, ctx.positions[i * 3 + 2]!);
        pusher.quaternion.set(
          ctx.rotations[i * 4]!,
          ctx.rotations[i * 4 + 1]!,
          ctx.rotations[i * 4 + 2]!,
          ctx.rotations[i * 4 + 3]!,
        );
        // CylinderGeometry is centered; physics hull starts at y=0 on the body.
        pusher.position.y += JUNKYARD_PUSHER_HEIGHT * 0.5;
      },
      dispose() {
        scene.remove(floor);
        floor.geometry.dispose();
        (floor.material as THREE.Material).dispose();
        for (const wall of wallMeshes) {
          scene.remove(wall);
          wall.geometry.dispose();
        }
        wallMat.dispose();
        scene.remove(pusher);
        pusher.geometry.dispose();
        (pusher.material as THREE.Material).dispose();
      },
    };
  },
  info: ({ workerCount, colorMode }) =>
    `${JUNKYARD_ROCK_COUNT} rocks + kinematic pusher (matches upstream Junkyard) | ${workerCount} workers | ${colorMode} colors (C)`,
});
