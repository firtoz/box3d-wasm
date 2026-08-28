import * as THREE from "three";
import type { Box3DRuntime, HullHandle, Vec3 } from "box3d-wasm";
import type { DemoSample } from "../types";
import { hullReductionCamera } from "./hull-reduction-scene";
import { disposeHullMesh, hullMeshFromHandle } from "./hull-draw";

const CAPACITY = 128;

export const hullReductionSample: DemoSample = {
  id: "geometry/hull-reduction",
  name: "Geometry / Hull Reduction",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    let type: "box" | "sphere" = "sphere";
    let count = 16;
    const points: Vec3[] = [];
    let hull: HullHandle | null = null;
    let mesh: THREE.Object3D | null = null;
    let info = "";
    const axes = new THREE.AxesHelper(1);
    scene.add(axes);

    const generatePoints = () => {
      runtime.setRandomSeed(42);
      points.length = 0;
      if (type === "box") {
        for (let i = 0; i < CAPACITY; i++) {
          const p = runtime.randomVec3([-2, -2, -2], [2, 2, 2]);
          const clamped: Vec3 = [
            Math.min(1, Math.max(-1, p[0])),
            Math.min(1, Math.max(-1, p[1])),
            Math.min(1, Math.max(-1, p[2])),
          ];
          const noise = runtime.randomVec3([-0.001, -0.001, -0.001], [0.001, 0.001, 0.001]);
          points.push([clamped[0] + noise[0], clamped[1] + noise[1], clamped[2] + noise[2]]);
        }
      } else {
        for (let i = 0; i < CAPACITY; i++) points.push(runtime.randomUnitVector());
      }
    };

    const generateHull = () => {
      disposeHullMesh(scene, mesh);
      mesh = null;
      if (hull !== null) runtime.destroyHull(hull);
      const flat = points.flat();
      hull = runtime.createHullFromPoints(flat, count);
      mesh = hullMeshFromHandle(runtime, hull, 0xeab308);
      scene.add(mesh);
      const stats = runtime.getHullInfo(hull);
      info = `v/f/e = ${stats.vertexCount}/${stats.faceCount}/${stats.edgeCount}`;
    };

    generatePoints();
    generateHull();

    return {
      world,
      bodies: [],
      camera: hullReductionCamera,
      getInfo: () => info,
      controls: [
        { key: "box", label: "Box", type: "button", onClick: () => { type = "box"; generatePoints(); generateHull(); } },
        { key: "sphere", label: "Sphere", type: "button", onClick: () => { type = "sphere"; generatePoints(); generateHull(); } },
        { key: "count", label: "count", type: "range", min: 4, max: CAPACITY, step: 1, value: count, onChange: (v) => { if (typeof v === "number") { count = Math.round(v); generateHull(); } } },
      ],
      step() {},
      dispose() {
        disposeHullMesh(scene, mesh);
        if (hull !== null) runtime.destroyHull(hull);
        scene.remove(axes);
        axes.dispose();
        world.destroy();
      },
    };
  },
};
