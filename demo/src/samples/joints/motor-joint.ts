import * as THREE from "three";
import { B3_AXIS_Z, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { buildMotorJointDynamicBodies, motorJointBodyIndex, motorJointCamera, motorJointGroundSize, motorJointSpringBodyIndex, motorJointTargetIndex, motorJointVisibleBodies } from "./motor-joint-scene";

export const motorJointSample: DemoSample = {
  id: "joints/motor-joint",
  name: "Joints / Motor Joint",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];
    const half = motorJointGroundSize();
    addBox(world, scene, bodies, half, [0, -1, 0], 0x222222, true);
    const handles = buildMotorJointDynamicBodies(world, runtime);
    addVisibleJointBodies(scene, bodies, handles, motorJointVisibleBodies);

    let speed = 0;
    let time = 0;
    const targetHandle = handles[motorJointTargetIndex];
    const bodyHandle = handles[motorJointBodyIndex];
    if (targetHandle === undefined || bodyHandle === undefined || handles[motorJointSpringBodyIndex] === undefined) {
      throw new Error("Motor joint scene handles missing expected bodies");
    }

    return {
      world,
      bodies,
      profile: true,
      camera: motorJointCamera,
      controls: [
        { key: "speed", label: "Speed", type: "range", min: -5, max: 5, step: 1, value: speed, onChange: (v) => { speed = typeof v === "number" ? v : speed; } },
        { key: "impulse", label: "Apply Impulse", type: "button", onClick: () => world.applyLinearImpulseToCenter(bodyHandle, [100000, 0, 0]) },
      ],
      step(dt, subSteps) {
        const timeStep = dt ?? 1 / 60;
        if (timeStep > 0) {
          time += speed * timeStep;
          const position: [number, number, number] = [6 * Math.sin(2 * time), 10 + 4 * Math.sin(time), 0];
          const rotation = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, 2 * time);
          runtime.setBodyTargetTransform(targetHandle, position, rotation, timeStep, true);
        }
        world.step(timeStep, subSteps ?? 4);
        syncBodies(world, bodies);
      },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
