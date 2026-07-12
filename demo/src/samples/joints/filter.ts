import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { buildFilterJointDynamicBodies, filterJointCamera, filterJointGroundSize, filterJointVisibleBodies } from "./filter-scene";

export const filterJointSample: DemoSample = {
  id: "joints/filter",
  name: "Joints / Filter",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    const half = filterJointGroundSize();
    addBox(world, scene, bodies, half, [0, -1, 0], 0x222222, true);
    const handles = buildFilterJointDynamicBodies(world, runtime);
    addVisibleJointBodies(scene, bodies, handles, filterJointVisibleBodies);
    return {
      world,
      bodies,
      profile: true,
      camera: filterJointCamera,
      controls: [],
      step(dt, subSteps) { world.step(dt, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
