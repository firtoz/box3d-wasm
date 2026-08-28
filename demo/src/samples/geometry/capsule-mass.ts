import * as THREE from "three";
import { B3_PI, type Box3DRuntime, type HullHandle } from "box3d-wasm";
import type { DemoSample } from "../types";
import { capsuleMesh } from "../shared";
import { capsuleMassCamera } from "./capsule-mass-scene";
import { disposeHullMesh, hullMeshFromHandle } from "./hull-draw";

const RADIUS = 1;
const LENGTH = 2;
const MAX_SIDES = 6;

export const capsuleMassSample: DemoSample = {
  id: "geometry/capsule-mass",
  name: "Geometry / Capsule Mass",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    let sides = 6;
    let hull: HullHandle | null = null;
    let hullMesh: THREE.Object3D | null = null;
    let info = "";
    const capsule = capsuleMesh(RADIUS, LENGTH, 0x22d3ee, 0.5, "x");
    const capsuleMaterial = capsule.material as THREE.Material;
    capsuleMaterial.transparent = true;
    capsuleMaterial.opacity = 0.65;
    scene.add(capsule);
    const boxHull = runtime.makeBoxHull([RADIUS + 0.5 * LENGTH, RADIUS, RADIUS]);
    const boxMesh = hullMeshFromHandle(runtime, boxHull, 0x8b5cf6);
    scene.add(boxMesh);
    const axes = new THREE.AxesHelper(1);
    scene.add(axes);

    const createCapsuleHull = (sideCount: number) => {
      disposeHullMesh(scene, hullMesh);
      hullMesh = null;
      if (hull !== null) runtime.destroyHull(hull);
      hull = null;
      if (sideCount > MAX_SIDES) return;
      const count = 2 * sideCount * sideCount;
      const d = B3_PI / (sideCount - 1);
      const points: number[] = [];
      let angle1 = -0.5 * B3_PI;
      for (let i = 0; i < sideCount; i++) {
        const s1 = runtime.b3wSinf(angle1);
        const c1 = runtime.b3wCosf(angle1);
        let angle2 = -0.5 * B3_PI;
        for (let j = 0; j < sideCount; j++) {
          points.push(1 + RADIUS * c1, RADIUS * s1 * runtime.b3wCosf(angle2), RADIUS * s1 * runtime.b3wSinf(angle2));
          angle2 += d;
        }
        angle1 += d;
      }
      angle1 = 0.5 * B3_PI;
      for (let i = 0; i < sideCount; i++) {
        const s1 = runtime.b3wSinf(angle1);
        const c1 = runtime.b3wCosf(angle1);
        let angle2 = -0.5 * B3_PI;
        for (let j = 0; j < sideCount; j++) {
          points.push(-1 + RADIUS * c1, RADIUS * s1 * runtime.b3wCosf(angle2), RADIUS * s1 * runtime.b3wSinf(angle2));
          angle2 += d;
        }
        angle1 += d;
      }
      if (points.length / 3 !== count) throw new Error("capsule hull point count mismatch");
      hull = runtime.createHullFromPoints(points);
      hullMesh = hullMeshFromHandle(runtime, hull, 0xeab308);
      scene.add(hullMesh);
      const hullMass = runtime.computeHullMass(hull);
      const capsuleMass = runtime.computeCapsuleMass({ a: [-0.5 * LENGTH, 0, 0], b: [0.5 * LENGTH, 0, 0], radius: RADIUS });
      const boxMass = runtime.computeHullMass(boxHull);
      info = [
        `mass hull: ${hullMass.mass.toPrecision(6)} | mass capsule: ${capsuleMass.mass.toPrecision(6)} | mass box: ${boxMass.mass.toPrecision(6)}`,
        `Ixx hull: ${hullMass.ixx.toPrecision(6)} | Ixx capsule: ${capsuleMass.ixx.toPrecision(6)} | Ixx box: ${boxMass.ixx.toPrecision(6)}`,
        `Iyy hull: ${hullMass.iyy.toPrecision(6)} | Iyy capsule: ${capsuleMass.iyy.toPrecision(6)} | Iyy box: ${boxMass.iyy.toPrecision(6)}`,
        `Izz hull: ${hullMass.izz.toPrecision(6)} | Izz capsule: ${capsuleMass.izz.toPrecision(6)} | Izz box: ${boxMass.izz.toPrecision(6)}`,
      ].join(" | ");
    };
    createCapsuleHull(sides);

    return {
      world,
      bodies: [],
      camera: capsuleMassCamera,
      getInfo: () => info,
      controls: [
        { key: "sides", label: "sides", type: "range", min: 3, max: MAX_SIDES, step: 1, value: sides, onChange: (v) => { if (typeof v === "number") { sides = Math.round(v); createCapsuleHull(sides); } } },
      ],
      step() {},
      dispose() {
        disposeHullMesh(scene, hullMesh);
        disposeHullMesh(scene, boxMesh);
        if (hull !== null) runtime.destroyHull(hull);
        runtime.destroyHull(boxHull);
        scene.remove(capsule);
        capsule.geometry.dispose();
        capsuleMaterial.dispose();
        scene.remove(axes);
        axes.dispose();
        world.destroy();
      },
    };
  },
};
