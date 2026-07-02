import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, capsuleMesh, disposeBodies, syncBodies } from "./shared";

export const capsuleStackSample: DemoSample = {
  id: "capsule-stack",
  name: "Stacking / Capsule Stack",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];

    addBox(world, scene, bodies, [20, 1, 20], [0, -1, 0], 0x222222, true);

    const r = 0.5;
    const segLen = 2;
    const color = 0x38bdf8;
    let y = 1.5 * r;

    for (let i = 0; i < 20; i++) {
      const p: [number, number, number] = [0, y, 0];
      const bodyHandle = world.createBody({ type: 2, position: p });
      world.createCapsuleShape(bodyHandle, [-1, 0, 0], [1, 0, 0], r);
      runtime.setBodyMotionLocks(bodyHandle, { lockLinearZ: true, lockRotationX: true, lockRotationY: true, lockRotationZ: true });

      const mesh = capsuleMesh(r, segLen, color);
      mesh.position.set(0, y, 0);
      scene.add(mesh);

      bodies.push({ handle: bodyHandle, mesh, type: 2 });
      y += 2.0 * r;
    }

    return {
      world,
      bodies,
      controls: [],
      info: "20 capsules, 2D-stacked (Z-locked, rotation locked)",
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
