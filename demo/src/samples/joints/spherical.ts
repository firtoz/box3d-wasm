import * as THREE from "three";
import { B3_AXIS_X, B3_AXIS_Y, B3_AXIS_Z, B3_DEG_TO_RAD, type Box3DRuntime, type JointId, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { createSphericalJointScene, sphericalJointCamera, sphericalJointGroundSize, sphericalJointVisibleBodies } from "./spherical-scene";

function wake(world: PhysicsWorld, joint: JointId): void {
  world.wakeJointBodies(joint);
}

export const sphericalJointSample: DemoSample = {
  id: "joints/spherical",
  name: "Joints / Spherical",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    addBox(world, scene, bodies, sphericalJointGroundSize(), [0, -1, 0], 0x222222, true);
    const { handles, joint } = createSphericalJointScene(world, runtime);
    addVisibleJointBodies(scene, bodies, handles, sphericalJointVisibleBodies);
    let lowerTwist = -35;
    let upperTwist = 35;
    const motorVelocity: Vec3 = [0, 0, 0];
    const targetRotationDeg: Vec3 = [0, 0, 0];
    const applyTargetRotation = () => {
      const qx = runtime.makeQuatFromAxisAngle(B3_AXIS_X, B3_DEG_TO_RAD * targetRotationDeg[0]);
      const qy = runtime.makeQuatFromAxisAngle(B3_AXIS_Y, B3_DEG_TO_RAD * targetRotationDeg[1]);
      const qz = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, B3_DEG_TO_RAD * targetRotationDeg[2]);
      world.setSphericalTargetRotation(joint, runtime.mulQuat(qz, runtime.mulQuat(qy, qx)));
      wake(world, joint);
    };
    return {
      world,
      bodies,
      profile: true,
      camera: sphericalJointCamera,
      info: "spherical joint — cone / twist / motor / spring",
      controls: [
        { key: "cone", label: "Cone Limit", type: "toggle", value: false, onChange: (v) => { if (typeof v === "boolean") { world.enableSphericalConeLimit(joint, v); wake(world, joint); } } },
        { key: "cone-angle", label: "Cone Angle", type: "range", min: 0, max: 90, step: 1, value: 30, onChange: (v) => { if (typeof v === "number") { world.setSphericalConeLimit(joint, B3_DEG_TO_RAD * v); wake(world, joint); } } },
        { key: "twist", label: "Twist Limit", type: "toggle", value: false, onChange: (v) => { if (typeof v === "boolean") { world.enableSphericalTwistLimit(joint, v); wake(world, joint); } } },
        { key: "lower-twist", label: "Lower Twist", type: "range", min: -180, max: 180, step: 1, value: lowerTwist, onChange: (v) => { if (typeof v === "number") { lowerTwist = Math.min(v, upperTwist); world.setSphericalTwistLimits(joint, B3_DEG_TO_RAD * lowerTwist, B3_DEG_TO_RAD * upperTwist); wake(world, joint); } } },
        { key: "upper-twist", label: "Upper Twist", type: "range", min: -180, max: 180, step: 1, value: upperTwist, onChange: (v) => { if (typeof v === "number") { upperTwist = Math.max(v, lowerTwist); world.setSphericalTwistLimits(joint, B3_DEG_TO_RAD * lowerTwist, B3_DEG_TO_RAD * upperTwist); wake(world, joint); } } },
        { key: "motor", label: "Motor", type: "toggle", value: false, onChange: (v) => { if (typeof v === "boolean") { world.enableSphericalMotor(joint, v); wake(world, joint); } } },
        { key: "torque", label: "Max Torque", type: "range", min: 0, max: 10000, step: 10, value: 20, onChange: (v) => { if (typeof v === "number") { world.setSphericalMaxMotorTorque(joint, v); wake(world, joint); } } },
        { key: "vel-x", label: "Velocity X", type: "range", min: -10, max: 10, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { motorVelocity[0] = v; world.setSphericalMotorVelocity(joint, motorVelocity); wake(world, joint); } } },
        { key: "vel-y", label: "Velocity Y", type: "range", min: -10, max: 10, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { motorVelocity[1] = v; world.setSphericalMotorVelocity(joint, motorVelocity); wake(world, joint); } } },
        { key: "vel-z", label: "Velocity Z", type: "range", min: -10, max: 10, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { motorVelocity[2] = v; world.setSphericalMotorVelocity(joint, motorVelocity); wake(world, joint); } } },
        { key: "spring", label: "Spring", type: "toggle", value: true, onChange: (v) => { if (typeof v === "boolean") { world.enableSphericalSpring(joint, v); wake(world, joint); } } },
        { key: "hertz", label: "Hertz", type: "range", min: 0, max: 10, step: 0.1, value: 2, onChange: (v) => { if (typeof v === "number") { world.setSphericalSpringHertz(joint, v); wake(world, joint); } } },
        { key: "damping", label: "Damping", type: "range", min: 0, max: 2, step: 0.1, value: 0.7, onChange: (v) => { if (typeof v === "number") { world.setSphericalSpringDampingRatio(joint, v); wake(world, joint); } } },
        { key: "rot-x", label: "Rotation X", type: "range", min: -180, max: 180, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { targetRotationDeg[0] = v; applyTargetRotation(); } } },
        { key: "rot-y", label: "Rotation Y", type: "range", min: -180, max: 180, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { targetRotationDeg[1] = v; applyTargetRotation(); } } },
        { key: "rot-z", label: "Rotation Z", type: "range", min: -180, max: 180, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { targetRotationDeg[2] = v; applyTargetRotation(); } } },
      ],
      step(dt, subSteps) { world.step(dt ?? 1 / 60, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
