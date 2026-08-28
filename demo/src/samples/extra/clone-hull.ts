import * as THREE from "three";
import { BodyType, type Box3DRuntime, type HullHandle } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { cameraFromSetView, disposeBodies, syncBodies } from "../shared";
import { disposeHullMesh, hullMeshFromHandle } from "../geometry/hull-draw";

const HALF: [number, number, number] = [0.6, 0.4, 0.25];
const LEFT: [number, number, number] = [-1.5, 0, 0];
const RIGHT: [number, number, number] = [1.5, 0, 0];

export const extraCloneHullSample: DemoSample = {
  id: "extra/clone-hull",
  name: "Extra / Clone Hull",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const body = world.createBody({ type: BodyType.Static, position: LEFT });
    const shape = runtime.createHullShape(body, HALF);
    const sourceHull = runtime.makeBoxHull(HALF);
    const cloned: HullHandle = runtime.cloneHullFromShape(shape.shapeHandle);
    const sourceMesh = hullMeshFromHandle(runtime, sourceHull, 0x60a5fa);
    sourceMesh.position.set(...LEFT);
    scene.add(sourceMesh);
    const cloneMesh = hullMeshFromHandle(runtime, cloned, 0xf97316);
    cloneMesh.position.set(...RIGHT);
    scene.add(cloneMesh);
    const bodies: DemoBody[] = [];
    const info = [
      "left: live hull shape (cloneHullFromShape source)",
      "right: cloned hull slot, display only",
      JSON.stringify(runtime.getHullInfo(cloned)),
    ].join("\n");
    return {
      world,
      bodies,
      camera: cameraFromSetView(0, 12, 8, [0, 0, 0]),
      getInfo: () => info,
      controls: [],
      step(dt, subSteps) {
        world.step(dt ?? 1 / 60, subSteps ?? 4);
        syncBodies(world, bodies);
      },
      dispose() {
        disposeBodies(scene, bodies);
        disposeHullMesh(scene, sourceMesh);
        disposeHullMesh(scene, cloneMesh);
        runtime.destroyHull(sourceHull);
        runtime.destroyHull(cloned);
        world.destroy();
      },
    };
  },
};
