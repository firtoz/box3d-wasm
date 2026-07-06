import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { disposeBodies, syncBodies } from "../shared";
import { addVisibleJointCapsule, addVisibleJointSphere } from "./shared";
import { ballAndChainCamera, createBallAndChain } from "./ball-and-chain-scene";

export const ballAndChainSample: DemoSample = {
  id: "joints/ball-and-chain",
  name: "Joints / Ball and Chain",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const { world, handles } = createBallAndChain(runtime);
    const bodies: DemoBody[] = [];
    for (let i = 1; i <= 32; i++) addVisibleJointCapsule(scene, bodies, handles[i]!, 0.125, 1, [(1 + 2 * (i - 1)) * 0.5, 0, 0], 0x38bdf8);
    addVisibleJointSphere(scene, bodies, handles[33]!, 2, [(1 + 2 * 32) * 0.5 + 2 - 0.5, 0, 0], 0xf97316);
    return { world, bodies, profile: true, camera: ballAndChainCamera, controls: [], step(dt, subSteps) { world.step(dt ?? 1 / 60, subSteps ?? 4); syncBodies(world, bodies); }, dispose() { disposeBodies(scene, bodies); world.destroy(); } };
  },
};
