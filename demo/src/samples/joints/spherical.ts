import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { buildSphericalJointDynamicBodies, sphericalJointCamera, sphericalJointGroundSize, sphericalJointVisibleBodies } from "./spherical-scene";

export const sphericalJointSample: DemoSample = {
  id: "joints/spherical",
  name: "Joints / Spherical",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    addBox(world, scene, bodies, sphericalJointGroundSize(), [0, -1, 0], 0x222222, true);
    const handles = buildSphericalJointDynamicBodies(world, runtime);
    addVisibleJointBodies(scene, bodies, handles, sphericalJointVisibleBodies);
    return { world, bodies, profile: true, camera: sphericalJointCamera, controls: [], step(dt, subSteps) { world.step(dt ?? 1 / 60, subSteps ?? 4); syncBodies(world, bodies); }, dispose() { disposeBodies(scene, bodies); world.destroy(); } };
  },
};
