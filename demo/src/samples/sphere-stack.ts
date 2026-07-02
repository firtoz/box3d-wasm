import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, disposeBodies, syncBodies } from "./shared";

export const sphereStackSample: DemoSample = {
  id: "sphere-stack",
  name: "Stacking / Sphere Stack",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];

    addBox(world, scene, bodies, [15, 1, 15], [0, -1, 0], 0x222222, true);

    const r = 0.5;
    let y = 1.5 * r;
    for (let i = 0; i < 30; i++) {
      const p: [number, number, number] = [0, y, 0];
      const bodyHandle = world.createBody({ type: 2, position: p, awake: true });
      runtime.createSphereShape(bodyHandle, { radius: r, rollingResistance: 0.1 });
      const mesh = new THREE.Mesh(
        new THREE.SphereGeometry(r, 24, 16),
        new THREE.MeshStandardMaterial({ color: 0x38bdf8, roughness: 0.6 }),
      );
      mesh.position.set(p[0], p[1], p[2]);
      mesh.castShadow = true;
      mesh.receiveShadow = true;
      scene.add(mesh);
      bodies.push({ handle: bodyHandle, mesh, type: 2 });
      y += 3.0 * r;
    }

    return {
      world,
      bodies,
      controls: [],
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
