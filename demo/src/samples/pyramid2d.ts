import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, disposeBodies, syncBodies } from "./shared";

const PYRAMID_SIZE = 12;

export const pyramid2dSample: DemoSample = {
  id: "pyramid2d",
  name: "Stacking / Pyramid2D",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];

    addBox(world, scene, bodies, [40, 1, 40], [0, -1, 0], 0x222222, true);

    const a = 1.0;
    for (let row = 0; row < PYRAMID_SIZE; row++) {
      for (let col = 0; col < PYRAMID_SIZE - row; col++) {
        const px = (-10 + 2 * col + row) * a;
        const py = (1.5 + 2.5 * row) * a;
        const p: [number, number, number] = [px, py, 0];
        const bodyHandle = world.createBody({ type: 2, position: p, isAwake: true });
        runtime.createHullShape(bodyHandle, [a, a, a]);
        runtime.setBodyMotionLocks(bodyHandle, { lockLinearZ: true, lockRotationX: true, lockRotationY: true });
        const mesh = new THREE.Mesh(
          new THREE.BoxGeometry(a * 2, a * 2, a * 2),
          new THREE.MeshStandardMaterial({ color: 0x60a5fa + (row % 10) * 0x010101, roughness: 0.75 }),
        );
        mesh.position.set(p[0], p[1], p[2]);
        mesh.castShadow = true;
        mesh.receiveShadow = true;
        scene.add(mesh);
        bodies.push({ handle: bodyHandle, mesh, type: 2 });
      }
    }

    return {
      world,
      bodies,
      controls: [],
      info: `${PYRAMID_SIZE} rows, 2D stacking (Z-locked)`,
      step(dt) {
        world.step(dt, 4);
        syncBodies(world, bodies);
      },
      dispose() {
        disposeBodies(scene, bodies);
        world.destroy();
      },
    };
  },
};
