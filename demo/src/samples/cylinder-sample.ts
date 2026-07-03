import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, disposeBodies, syncBodies } from "./shared";

export const cylinderSample: DemoSample = {
  id: "cylinder",
  name: "Stacking / Cylinder",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];

    addBox(world, scene, bodies, [10, 1, 10], [0, -1, 0], 0x222222, true);

    const hullHandle = runtime.createCylinder(1, 0.25, 0, 12);
    const bodyHandle = world.createBody({ type: 2, position: [0, 2, 0], isAwake: true });
    runtime.createShapeFromHull(bodyHandle, hullHandle, { density: 1000, rollingResistance: 0.05 });
    runtime.destroyHull(hullHandle);

    const cylGeo = new THREE.CylinderGeometry(0.25, 0.25, 1, 12);
    const cylMesh = new THREE.Mesh(cylGeo, new THREE.MeshStandardMaterial({ color: 0x38bdf8, roughness: 0.6 }));
    cylMesh.position.set(0, 2, 0);
    cylMesh.castShadow = true;
    cylMesh.receiveShadow = true;
    scene.add(cylMesh);
    bodies.push({ handle: bodyHandle, mesh: cylMesh, type: 2 });

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
