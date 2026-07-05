import * as THREE from "three";
import { BodyType, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addSphere, disposeBodies, syncBodies } from "./shared";

export const compoundSimpleSample: DemoSample = {
  id: "compound-simple",
  name: "Compound / Simple",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];

    const a = 4;
    const compound = runtime.createCompoundFromHulls([
      {
        halfWidths: [a, 0.125 * a, a],
        transform: {
          position: [1, -0.125 * a, 0],
          rotation: [0, 0, 0, 1],
        },
        friction: 0.5,
      },
    ]);

    const groundBody = world.createBody({
      type: BodyType.Static,
      position: [2, -1, 0],
    });
    world.setBodyTransform(groundBody, [2, -1, 0], [0, Math.sin(Math.PI / 8), 0, Math.cos(Math.PI / 8)]);

    world.createCompoundShape(groundBody, compound);

    const groundMesh = new THREE.Mesh(
      new THREE.BoxGeometry(a * 2, 0.125 * a * 2, a * 2),
      new THREE.MeshStandardMaterial({ color: 0x223047, roughness: 0.75 }),
    );
    const qy = Math.sin(Math.PI / 8);
    const qw = Math.cos(Math.PI / 8);
    const localPx = 1;
    const localPy = -0.125 * a;
    const localPz = 0;
    const worldPx = 2 + (qw * qw - qy * qy) * localPx + 2 * qw * -qy * localPz;
    const worldPy = -1 + localPy;
    const worldPz = 0 - 2 * qw * qy * localPx + (qw * qw - qy * qy) * localPz;
    groundMesh.position.set(worldPx, worldPy, worldPz);
    groundMesh.quaternion.set(0, qy, 0, qw);
    groundMesh.receiveShadow = true;
    scene.add(groundMesh);

    const axes = new THREE.AxesHelper(1);
    axes.position.set(0, 0.01, 0);
    scene.add(axes);

    addSphere(world, scene, bodies, 0.25, [0, 2, 0], 0xf59e0b);

    return {
      world,
      bodies,
      launchSpeed: 1.0,
      controls: [],
      info: `compound tree height = ${world.getCompoundTreeHeight(compound)}`,
      step(dt, subSteps) {
        world.step(dt, subSteps ?? 4);
        syncBodies(world, bodies);
      },
      dispose() {
        world.destroyCompound(compound);
        scene.remove(groundMesh);
        groundMesh.geometry.dispose();
        (groundMesh.material as THREE.Material).dispose();
        scene.remove(axes);
        disposeBodies(scene, bodies);
        world.destroy();
      },
    };
  },
};
