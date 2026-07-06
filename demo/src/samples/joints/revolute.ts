import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { buildRevoluteJointDynamicBodies, revoluteJointCamera, revoluteJointGroundSize, revoluteJointVisibleBodies } from "./revolute-scene";

export const revoluteJointSample: DemoSample = {
  id: "joints/revolute",
  name: "Joints / Revolute",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];
    const half = revoluteJointGroundSize();
    addBox(world, scene, bodies, half, [0, -1, 0], 0x222222, true);
    const handles = buildRevoluteJointDynamicBodies(world, runtime);
    addVisibleJointBodies(scene, bodies, handles, revoluteJointVisibleBodies);
    return {
      world,
      bodies,
      profile: true,
      camera: revoluteJointCamera,
      controls: [],
      step(dt, subSteps) { world.step(dt, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
