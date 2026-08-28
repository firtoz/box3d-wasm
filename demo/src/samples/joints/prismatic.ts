import * as THREE from "three";
import type { Box3DRuntime, JointId, PhysicsWorld } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { createPrismaticJointScene, prismaticJointCamera, prismaticJointGroundSize, prismaticJointVisibleBodies } from "./prismatic-scene";

function wake(world: PhysicsWorld, joint: JointId): void {
  world.wakeJointBodies(joint);
}

export const prismaticJointSample: DemoSample = {
  id: "joints/prismatic",
  name: "Joints / Prismatic",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    const half = prismaticJointGroundSize();
    addBox(world, scene, bodies, half, [0, -1, 0], 0x222222, true);
    const { handles, joint } = createPrismaticJointScene(world, runtime);
    addVisibleJointBodies(scene, bodies, handles, prismaticJointVisibleBodies);
    let lower = -1;
    let upper = 1;
    return {
      world,
      bodies,
      profile: true,
      camera: prismaticJointCamera,
      info: "prismatic joint — limit / motor / spring (spring on by default)",
      controls: [
        { key: "limit", label: "Limit", type: "toggle", value: false, onChange: (v) => { if (typeof v === "boolean") { world.enablePrismaticLimit(joint, v); wake(world, joint); } } },
        { key: "lower", label: "Lower Translation", type: "range", min: -10, max: 10, step: 0.1, value: lower, onChange: (v) => { if (typeof v === "number") { lower = Math.min(v, upper); world.setPrismaticLimits(joint, lower, upper); wake(world, joint); } } },
        { key: "upper", label: "Upper Translation", type: "range", min: -10, max: 10, step: 0.1, value: upper, onChange: (v) => { if (typeof v === "number") { upper = Math.max(v, lower); world.setPrismaticLimits(joint, lower, upper); wake(world, joint); } } },
        { key: "motor", label: "Motor", type: "toggle", value: false, onChange: (v) => { if (typeof v === "boolean") { world.enablePrismaticMotor(joint, v); wake(world, joint); } } },
        { key: "force", label: "Max Force", type: "range", min: 0, max: 100000, step: 100, value: 20, onChange: (v) => { if (typeof v === "number") { world.setPrismaticMaxMotorForce(joint, v); wake(world, joint); } } },
        { key: "speed", label: "Speed", type: "range", min: -10, max: 10, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { world.setPrismaticMotorSpeed(joint, v); wake(world, joint); } } },
        { key: "spring", label: "Spring", type: "toggle", value: true, onChange: (v) => { if (typeof v === "boolean") { world.enablePrismaticSpring(joint, v); wake(world, joint); } } },
        { key: "hertz", label: "Hertz", type: "range", min: 0, max: 10, step: 0.1, value: 2, onChange: (v) => { if (typeof v === "number") { world.setPrismaticSpringHertz(joint, v); wake(world, joint); } } },
        { key: "damping", label: "Damping", type: "range", min: 0, max: 2, step: 0.1, value: 0.7, onChange: (v) => { if (typeof v === "number") { world.setPrismaticSpringDampingRatio(joint, v); wake(world, joint); } } },
        { key: "translation", label: "Translation", type: "range", min: -20, max: 20, step: 0.1, value: 0, onChange: (v) => { if (typeof v === "number") { world.setPrismaticTargetTranslation(joint, v); wake(world, joint); } } },
      ],
      step(dt, subSteps) { world.step(dt, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
