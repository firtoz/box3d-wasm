import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoSample } from "../types";
import { createDebugLine, createDebugPoint, disposeDebugObject, updateDebugLine, updateDebugPoint } from "../debug-overlay";
import { disposeHullMesh, hullMeshFromHandle } from "../geometry/hull-draw";
import {
  distanceDebugCamera,
  distanceDebugTransformA,
  distanceDebugTransformB,
  runDistanceDebug,
} from "./distance-debug-scene";

export const distanceDebugSample: DemoSample = {
  id: "collision/distance-debug",
  name: "Collision / Distance Debug",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const grid = new THREE.GridHelper(10, 10, 0x4b5563, 0x4b5563);
    const axes = new THREE.AxesHelper(1);
    scene.add(grid);
    scene.add(axes);
    const hullA = runtime.makeBoxHull([40, 1, 40]);
    const hullB = runtime.makeTransformedBoxHull([0.5, 10, 0.5], { position: [0, 10, 0] });
    const meshA = hullMeshFromHandle(runtime, hullA, 0x22c55e);
    const meshB = hullMeshFromHandle(runtime, hullB, 0x22d3ee);
    meshA.position.set(...distanceDebugTransformA.position);
    meshB.position.set(...distanceDebugTransformB.position);
    meshB.quaternion.set(...distanceDebugTransformB.rotation);
    scene.add(meshA);
    scene.add(meshB);
    const pointA = createDebugPoint(scene, 0xffffff, 8);
    const pointB = createDebugPoint(scene, 0xffffff, 8);
    const normal = createDebugLine(scene, 0xffffff);
    let info = "";
    const refresh = () => {
      const result = runDistanceDebug(runtime);
      updateDebugPoint(pointA, result.pA);
      updateDebugPoint(pointB, result.pB);
      updateDebugLine(normal, result.pA, [
        result.pA[0] + result.n[0],
        result.pA[1] + result.n[1],
        result.pA[2] + result.n[2],
      ]);
      info = `distance = ${result.d}  normal = ${result.n[0]}, ${result.n[1]}, ${result.n[2]}`;
    };
    refresh();
    return {
      world,
      bodies: [],
      camera: distanceDebugCamera,
      controls: [],
      getInfo: () => info,
      step() { refresh(); },
      dispose() {
        runtime.destroyHull(hullA);
        runtime.destroyHull(hullB);
        disposeHullMesh(scene, meshA);
        disposeHullMesh(scene, meshB);
        scene.remove(grid);
        scene.remove(axes);
        disposeDebugObject(scene, pointA);
        disposeDebugObject(scene, pointB);
        disposeDebugObject(scene, normal);
        world.destroy();
      },
    };
  },
};
