import * as THREE from "three";
import { B3_AXIS_X, B3_DEG_TO_RAD, BodyType, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, disposeBodies, syncBodies } from "./shared";

export const shapesInclinedPlaneSample: DemoSample = {
  id: "shapes-inclined-plane",
  name: "Shapes / Inclined Plane",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];

    addBox(world, scene, bodies, [50, 1, 50], [0, -1, 0], 0x111827, true);

    const angle = 40 * B3_DEG_TO_RAD;
    const planeBody = world.createBody({ type: BodyType.Static, position: [0, 7.5, -5], isAwake: true });
    runtime.setBodyTransform(planeBody, [0, 7.5, -5], runtime.makeQuatFromAxisAngle(B3_AXIS_X, angle));
    runtime.createHullShape(planeBody, [16, 0.5, 10], { friction: 1 });

    const planeGeo = new THREE.BoxGeometry(32, 1, 20);
    const planeMesh = new THREE.Mesh(planeGeo, new THREE.MeshStandardMaterial({ color: 0x94a3b8, roughness: 0.8 }));
    planeMesh.position.set(0, 7.5, -5);
    planeMesh.rotation.x = angle;
    planeMesh.castShadow = true;
    planeMesh.receiveShadow = true;
    scene.add(planeMesh);
    bodies.push({ handle: planeBody, mesh: planeMesh, type: BodyType.Static });

    for (let i = 0; i < 5; i++) {
      const p: [number, number, number] = [-10 + 5 * i, 15.75, -10.6];
      const bodyHandle = world.createBody({ type: BodyType.Dynamic, position: p, isAwake: true });
      runtime.createHullShape(bodyHandle, [1, 1, 1], { friction: (i + 1) * (i + 1) * 0.04 });
      const mesh = new THREE.Mesh(
        new THREE.BoxGeometry(2, 2, 2),
        new THREE.MeshStandardMaterial({ color: 0x60a5fa + i * 0x050505, roughness: 0.75 }),
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
