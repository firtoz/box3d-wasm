import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import { createDebugLine, createWireSphere, disposeDebugObject, updateDebugLine } from "../debug-overlay";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { createDoorScene, doorCamera, doorGroundSize, doorLowerJointIndex, doorUpperJointIndex, doorVisibleBodies } from "./door-scene";

export const doorSample: DemoSample = {
  id: "joints/door",
  name: "Joints / Door",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];
    const ground = addBox(world, scene, bodies, doorGroundSize(), [0, -1, 0], 0x222222, true);
    const { handles, joints } = createDoorScene(world, runtime, ground.handle);
    addVisibleJointBodies(scene, bodies, handles, doorVisibleBodies);
    const doorHandle = handles[0]!;
    const lowerJoint = joints[doorLowerJointIndex];
    const upperJoint = joints[doorUpperJointIndex];
    let magnitude = 50000;
    const impulseMarker = createWireSphere(scene, 0.15, 0xf59e0b);
    const impulseLine = createDebugLine(scene, 0xf59e0b);
    const lineEnd = [0, 0, 0] as [number, number, number];
    return {
      world,
      bodies,
      profile: true,
      camera: doorCamera,
      getInfo() {
        const error1 = world.getJointLinearSeparation(lowerJoint);
        const error2 = upperJoint === undefined ? undefined : world.getJointLinearSeparation(upperJoint);
        return error2 === undefined ? `translation error 1 = ${error1.toPrecision(3)}` : `translation error 1 = ${error1.toPrecision(3)} | translation error 2 = ${error2.toPrecision(3)}`;
      },
      controls: [
        { key: "impulse", label: "Impulse", type: "button", onClick: () => world.applyLinearImpulse(doorHandle, [0, 0, -magnitude], world.getBodyWorldPoint(doorHandle, [0.75, 0, 0])) },
        { key: "magnitude", label: "Magnitude", type: "range", min: 1000, max: 100000, step: 1000, value: magnitude, onChange: (v) => { if (typeof v === "number") magnitude = v; } },
      ],
      step(dt, subSteps) {
        world.step(dt ?? 1 / 60, subSteps ?? 4);
        syncBodies(world, bodies);
        const point = world.getBodyWorldPoint(doorHandle, [0.75, 0, 0]);
        impulseMarker.position.set(point[0], point[1], point[2]);
        const lineScale = 0.5 + 2.5 * (magnitude / 100000);
        lineEnd[0] = point[0];
        lineEnd[1] = point[1];
        lineEnd[2] = point[2] - lineScale;
        updateDebugLine(impulseLine, point, lineEnd);
      },
      dispose() { disposeDebugObject(scene, impulseMarker); disposeDebugObject(scene, impulseLine); disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
