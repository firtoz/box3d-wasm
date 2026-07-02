import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, disposeBodies, syncBodies } from "./shared";

const RING_COUNT = 8;

export const dominoesSample: DemoSample = {
  id: "dominoes",
  name: "Stacking / Dominoes",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];

    addBox(world, scene, bodies, [40, 1, 40], [0, -1, 0], 0x222222, true);

    for (let ring = 0; ring < RING_COUNT; ring++) {
      const radius = 7.0 + 1.1 * ring;
      for (let deg = 0; deg < 360; deg += 2) {
        const rad = deg * Math.PI / 180;
        const cs = Math.cos(rad);
        const sn = Math.sin(rad);
        const nx = cs;
        const nz = sn;
        const px = radius * cs - (deg / 630) * nx;
        const pz = radius * sn - (deg / 630) * nz;
        const p: [number, number, number] = [px, 0.8, pz];
        const bodyHandle = world.createBody({ type: 2, position: p, awake: true });
        runtime.createHullShape(bodyHandle, [0.2, 0.8, 0.05]);
        const mesh = new THREE.Mesh(
          new THREE.BoxGeometry(0.4, 1.6, 0.1),
          new THREE.MeshStandardMaterial({ color: 0xfbbf24, roughness: 0.75 }),
        );
        mesh.position.set(p[0], p[1], p[2]);
        mesh.quaternion.set(0, -Math.sin(rad / 2), 0, Math.cos(rad / 2));
        mesh.castShadow = true;
        mesh.receiveShadow = true;
        scene.add(mesh);
        bodies.push({ handle: bodyHandle, mesh, type: 2 });
        if (ring === 0 && Math.abs(deg) < 0.1) {
          world.setBodyLinearVelocity(bodyHandle, [0, 0, 25]);
        }
      }
    }

    return {
      world,
      bodies,
      controls: [],
      info: `${RING_COUNT} rings of dominoes`,
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
