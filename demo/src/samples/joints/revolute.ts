import * as THREE from "three";
import { B3_DEG_TO_RAD, type Box3DRuntime, type JointId, type PhysicsWorld } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { createRevoluteJointScene, revoluteJointCamera, revoluteJointGroundSize, revoluteJointVisibleBodies } from "./revolute-scene";

function wake(world: PhysicsWorld, joint: JointId): void {
  world.wakeJointBodies(joint);
}

export const revoluteJointSample: DemoSample = {
  id: "joints/revolute",
  name: "Joints / Revolute",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    const half = revoluteJointGroundSize();
    addBox(world, scene, bodies, half, [0, -1, 0], 0x222222, true);
    const { handles, joint } = createRevoluteJointScene(world, runtime);
    addVisibleJointBodies(scene, bodies, handles, revoluteJointVisibleBodies);
    let lowerDegrees = -35;
    let upperDegrees = 35;
    return {
      world,
      bodies,
      profile: true,
      camera: revoluteJointCamera,
      info: "revolute joint — limit / motor / spring (C++ defaults off)",
      controls: [
        { key: "limit", label: "Limit", type: "toggle", value: false, onChange: (v) => { if (typeof v === "boolean") { world.enableRevoluteLimit(joint, v); wake(world, joint); } } },
        { key: "lower", label: "Lower Angle", type: "range", min: -180, max: 180, step: 1, value: lowerDegrees, onChange: (v) => { if (typeof v === "number") { lowerDegrees = Math.min(v, upperDegrees); world.setRevoluteLimits(joint, B3_DEG_TO_RAD * lowerDegrees, B3_DEG_TO_RAD * upperDegrees); wake(world, joint); } } },
        { key: "upper", label: "Upper Angle", type: "range", min: -180, max: 180, step: 1, value: upperDegrees, onChange: (v) => { if (typeof v === "number") { upperDegrees = Math.max(v, lowerDegrees); world.setRevoluteLimits(joint, B3_DEG_TO_RAD * lowerDegrees, B3_DEG_TO_RAD * upperDegrees); wake(world, joint); } } },
        { key: "motor", label: "Motor", type: "toggle", value: false, onChange: (v) => { if (typeof v === "boolean") { world.enableRevoluteMotor(joint, v); wake(world, joint); } } },
        { key: "torque", label: "Max Torque", type: "range", min: 0, max: 50000, step: 100, value: 5000, onChange: (v) => { if (typeof v === "number") { world.setRevoluteMaxMotorTorque(joint, v); wake(world, joint); } } },
        { key: "speed", label: "Speed", type: "range", min: -10, max: 10, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { world.setRevoluteMotorSpeed(joint, v); wake(world, joint); } } },
        { key: "spring", label: "Spring", type: "toggle", value: false, onChange: (v) => { if (typeof v === "boolean") { world.enableRevoluteSpring(joint, v); wake(world, joint); } } },
        { key: "hertz", label: "Hertz", type: "range", min: 0, max: 10, step: 0.1, value: 2, onChange: (v) => { if (typeof v === "number") { world.setRevoluteSpringHertz(joint, v); wake(world, joint); } } },
        { key: "damping", label: "Damping", type: "range", min: 0, max: 2, step: 0.1, value: 0.7, onChange: (v) => { if (typeof v === "number") { world.setRevoluteSpringDampingRatio(joint, v); wake(world, joint); } } },
        { key: "rotation", label: "Rotation", type: "range", min: -180, max: 180, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { world.setRevoluteJointTargetAngle(joint, B3_DEG_TO_RAD * v); wake(world, joint); } } },
      ],
      step(dt, subSteps) { world.step(dt, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
