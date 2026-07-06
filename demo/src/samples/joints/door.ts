import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { buildDoorDynamicBodies, doorCamera, doorGroundSize, doorVisibleBodies } from "./door-scene";

export const doorSample: DemoSample = {
  id: "joints/door",
  name: "Joints / Door",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];
    const ground = addBox(world, scene, bodies, doorGroundSize(), [0, -1, 0], 0x222222, true);
    const handles = buildDoorDynamicBodies(world, runtime, ground.handle);
    addVisibleJointBodies(scene, bodies, handles, doorVisibleBodies);
    const doorHandle = handles[0]!;
    let magnitude = 50000;
    return {
      world,
      bodies,
      profile: true,
      camera: doorCamera,
      controls: [
        { key: "impulse", label: "Impulse", type: "button", onClick: () => world.applyLinearImpulse(doorHandle, [0, 0, -magnitude], world.getBodyWorldPoint(doorHandle, [0.75, 0, 0])) },
        { key: "magnitude", label: "Magnitude", type: "range", min: 1000, max: 100000, step: 1000, value: magnitude, onChange: (v) => { if (typeof v === "number") magnitude = v; } },
      ],
      step(dt, subSteps) { world.step(dt ?? 1 / 60, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
