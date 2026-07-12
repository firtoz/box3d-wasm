import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { addBox, disposeBodies, syncBodies } from "../shared";
import { addVisibleJointBox } from "./shared";
import { bridgeCamera, bridgeGroundSize, buildBridgeDynamicBodies } from "./bridge-scene";

export const bridgeSample: DemoSample = {
  id: "joints/bridge",
  name: "Joints / Bridge",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    addBox(world, scene, bodies, bridgeGroundSize(), [0, -1, 0], 0x222222, true);
    const handles = buildBridgeDynamicBodies(world, runtime);
    const a = 0.125;
    const xbase = -160 * a;
    for (let i = 0; i < 150; i++) {
      addVisibleJointBox(scene, bodies, handles[i + 1]!, [a, 0.125, 0.5], [xbase + a * (1 + 2 * i), 20, 0], 0x38bdf8);
    }
    let gravityScale = 1;
    return {
      world,
      bodies,
      profile: true,
      camera: bridgeCamera,
      controls: [{ key: "gravity-scale", label: "Gravity Scale", type: "range", min: -1, max: 1, step: 0.1, value: gravityScale, onChange: (v) => { if (typeof v === "number") { gravityScale = v; for (let i = 1; i < handles.length; i++) runtime.setBodyGravityScale(handles[i]!, gravityScale); } } }],
      step(dt, subSteps) { world.step(dt ?? 1 / 60, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
