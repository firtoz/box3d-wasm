import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { buildPrismaticJointDynamicBodies, prismaticJointCamera, prismaticJointGroundSize, prismaticJointVisibleBodies } from "./prismatic-scene";

export const prismaticJointSample: DemoSample = {
  id: "joints/prismatic",
  name: "Joints / Prismatic",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    const half = prismaticJointGroundSize();
    addBox(world, scene, bodies, half, [0, -1, 0], 0x222222, true);
    const handles = buildPrismaticJointDynamicBodies(world, runtime);
    addVisibleJointBodies(scene, bodies, handles, prismaticJointVisibleBodies);
    return {
      world,
      bodies,
      profile: true,
      camera: prismaticJointCamera,
      controls: [],
      step(dt, subSteps) { world.step(dt, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
