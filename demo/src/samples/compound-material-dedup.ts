import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, disposeBodies, syncBodies } from "./shared";

export const compoundMaterialDedupSample: DemoSample = {
  id: "compound-material-dedup",
  name: "Compound Material Dedup",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const bodies: DemoBody[] = [];
    addBox(world, scene, bodies, [12, 0.5, 12], [0, -1, 0], 0x334155, true);
    const box = addBox(world, scene, bodies, [1, 1, 1], [-2, 4, 0], 0x38bdf8);
    const other = addBox(world, scene, bodies, [1, 1, 1], [2, 4, 0], 0xf97316);
    if (box.shapeIds === undefined || box.shapeIds[0] === undefined || other.shapeIds === undefined || other.shapeIds[0] === undefined) {
      throw new Error("Expected simple box bodies to expose their shape handles");
    }
    runtime.setShapeFriction(box.shapeIds[0], 0.3);
    runtime.setShapeRestitution(other.shapeIds[0], 0.5);
    runtime.setShapeDensity(box.shapeIds[0], 3.0, true);
    return {
      world,
      bodies,
      controls: [],
      step(dt: number, subSteps?: number) { world.step(dt, subSteps ?? 4); syncBodies(world, bodies); },
      dispose() { disposeBodies(scene, bodies); world.destroy(); },
    };
  },
};
