import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoSample } from "../types";
import { disposeHullMesh, hullMeshFromHandle } from "../geometry/hull-draw";
import { benchmarkHullCamera } from "./hull-scene";

const CAPACITY = 100;
const COUNT = 16;
const TRIALS = 200;

export const benchmarkHullSample: DemoSample = {
  id: "benchmark/hull",
  name: "Benchmark / Hull",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    runtime.setRandomSeed(42);
    const points: number[] = [];
    for (let i = 0; i < CAPACITY; i++) {
      const p = runtime.randomVec3([-1, -1, -1], [1, 1, 1]);
      points.push(p[0], p[1], p[2]);
    }
    const hull = runtime.createHullFromPoints(points, COUNT);
    const transformed = runtime.cloneAndTransformHull(hull, {}, [-1, 1, 1]);
    const meshA = hullMeshFromHandle(runtime, hull, 0x22c55e);
    const meshB = hullMeshFromHandle(runtime, transformed, 0xeab308);
    meshA.position.set(-2, 0, 0);
    meshB.position.set(2, 0, 0);
    scene.add(meshA);
    scene.add(meshB);
    let info = "";
    const runTrials = () => {
      const t0 = performance.now();
      let area = 0;
      for (let i = 0; i < TRIALS; i++) {
        const h = runtime.createHullFromPoints(points, COUNT);
        area += runtime.getHullInfo(h).surfaceArea;
        runtime.destroyHull(h);
      }
      const createMs = performance.now() - t0;
      const t1 = performance.now();
      let scaledArea = 0;
      for (let i = 0; i < TRIALS; i++) {
        const h = runtime.cloneAndTransformHull(hull, {}, [-1, 1, 1]);
        scaledArea += runtime.getHullInfo(h).surfaceArea;
        runtime.destroyHull(h);
      }
      const cloneMs = performance.now() - t1;
      info = `trials = ${TRIALS} | createTime (us) = ${((1000 * createMs) / TRIALS).toFixed(2)}, area = ${(area / TRIALS).toFixed(2)} | cloneTime (us) = ${((1000 * cloneMs) / TRIALS).toFixed(2)}, area = ${(scaledArea / TRIALS).toFixed(2)} | create/clone = ${(createMs / cloneMs).toFixed(2)}`;
    };
    runTrials();
    return {
      world,
      bodies: [],
      camera: benchmarkHullCamera,
      getInfo: () => info,
      controls: [],
      step() { runTrials(); },
      dispose() {
        runtime.destroyHull(transformed);
        runtime.destroyHull(hull);
        disposeHullMesh(scene, meshA);
        disposeHullMesh(scene, meshB);
        world.destroy();
      },
    };
  },
};
