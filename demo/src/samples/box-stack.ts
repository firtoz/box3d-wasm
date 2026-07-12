import * as THREE from "three";
import { BodyType, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, disposeBodies, syncBodies } from "./shared";

export const boxStackSample: DemoSample = {
  id: "box-stack",
  name: "Stacking / Box Stack",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];

    addBox(world, scene, bodies, [40, 1, 40], [0, -1, 0], 0x222222, true);

    const a = 0.5;
    for (let i = 0; i < 40; i++) {
      const p: [number, number, number] = [0, 1.5 * a + 2.5 * a * i, 0];
      const bodyHandle = world.createBody({ type: BodyType.Dynamic, position: p, isAwake: true });
      runtime.createHullShape(bodyHandle, [a, a, a], { rollingResistance: 0.1 });
      const mesh = new THREE.Mesh(
        new THREE.BoxGeometry(2 * a, 2 * a, 2 * a),
        new THREE.MeshStandardMaterial({ color: 0x60a5fa + (i % 10) * 0x010101, roughness: 0.75 }),
      );
      mesh.position.set(p[0], p[1], p[2]);
      mesh.castShadow = true;
      mesh.receiveShadow = true;
      scene.add(mesh);
      bodies.push({ handle: bodyHandle, mesh, type: BodyType.Dynamic });
    }

    return {
      world,
      bodies,
      controls: [],
      step(dt, subSteps) {
        world.step(dt, subSteps ?? 4);
        syncBodies(world, bodies);
      },
      dispose() {
        disposeBodies(scene, bodies);
        world.destroy();
      },
    };
  },
};
