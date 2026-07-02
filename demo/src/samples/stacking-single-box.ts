import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, disposeBodies, syncBodies } from "./shared";

export const singleBoxSample: DemoSample = {
  id: "single-box",
  name: "Stacking / Single Box",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];

    addBox(world, scene, bodies, [20, 1, 20], [0, -1, 0], 0x222222, true);

    const cube = addBox(world, scene, bodies, [0.5, 0.5, 0.5], [0, 0.5, 0], 0xf59e0b);
    runtime.setBodyAngularVelocity(cube.handle, [0, 10, 0]);
    return {
      world,
      bodies,
      camera: { position: [0, 4.226, 9.063], target: [0, 0, 0] },
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
