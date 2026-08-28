import * as THREE from "three";
import type { Box3DRuntime, JointId, PhysicsWorld } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBodies } from "./shared";
import { createWeldJointScene, weldJointCamera, weldJointGroundSize, weldJointVisibleBodies } from "./weld-scene";

function wake(world: PhysicsWorld, joint: JointId): void {
  world.wakeJointBodies(joint);
}

export const weldJointSample: DemoSample = {
  id: "joints/weld",
  name: "Joints / Weld",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    const half = weldJointGroundSize();
    addBox(world, scene, bodies, half, [0, -1, 0], 0x222222, true);
    const { handles, joint } = createWeldJointScene(world, runtime);
    addVisibleJointBodies(scene, bodies, handles, weldJointVisibleBodies);
    return {
      world,
      bodies,
      profile: true,
      camera: weldJointCamera,
      info: "weld joint — linear / angular hertz and damping",
      controls: [
        { key: "lin-hertz", label: "Linear Hertz", type: "range", min: 0, max: 10, step: 0.1, value: 0, onChange: (v) => { if (typeof v === "number") { world.setWeldLinearHertz(joint, v); wake(world, joint); } } },
        { key: "lin-damp", label: "Linear Damping", type: "range", min: 0, max: 2, step: 0.1, value: 0, onChange: (v) => { if (typeof v === "number") { world.setWeldLinearDampingRatio(joint, v); wake(world, joint); } } },
        { key: "ang-hertz", label: "Angular Hertz", type: "range", min: 0, max: 10, step: 0.1, value: 2, onChange: (v) => { if (typeof v === "number") { world.setWeldAngularHertz(joint, v); wake(world, joint); } } },
        { key: "ang-damp", label: "Angular Damping", type: "range", min: 0, max: 2, step: 0.1, value: 0.7, onChange: (v) => { if (typeof v === "number") { world.setWeldAngularDampingRatio(joint, v); wake(world, joint); } } },
      ],
      step(dt, subSteps) { world.step(dt, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
