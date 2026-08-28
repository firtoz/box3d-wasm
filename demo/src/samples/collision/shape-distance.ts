import * as THREE from "three";
import type { Box3DRuntime, HullHandle } from "box3d-wasm";
import type { DemoSample } from "../types";
import { createDebugLine, createDebugPoint, disposeDebugObject, updateDebugLine, updateDebugPoint } from "../debug-overlay";
import { disposeHullMesh, hullMeshFromHandle } from "../geometry/hull-draw";
import {
  ShapeDistanceType,
  runShapeDistance,
  shapeDistanceCamera,
  shapeDistanceSegment,
  shapeDistanceTransformB,
  shapeDistanceTriangle,
  type ShapeDistanceTypeId,
} from "./shape-distance-scene";

function addTriangle(scene: THREE.Scene, verts: readonly [readonly number[], readonly number[], readonly number[]], color: number): THREE.LineSegments {
  const [a, b, c] = verts;
  const geometry = new THREE.BufferGeometry();
  geometry.setAttribute("position", new THREE.BufferAttribute(new Float32Array([...a, ...b, ...b, ...c, ...c, ...a]), 3));
  const lines = new THREE.LineSegments(geometry, new THREE.LineBasicMaterial({ color, toneMapped: false }));
  scene.add(lines);
  return lines;
}

export const shapeDistanceSample: DemoSample = {
  id: "collision/shape-distance",
  name: "Collision / Shape Distance",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const axes = new THREE.AxesHelper(0.5);
    scene.add(axes);
    let typeA: ShapeDistanceTypeId = ShapeDistanceType.Triangle;
    let typeB: ShapeDistanceTypeId = ShapeDistanceType.Box;
    let radiusA = 0;
    let radiusB = 0;
    let boxHull: HullHandle | null = runtime.makeBoxHull([0.125, 0.25, 0.5]);
    let boxMesh: THREE.Object3D | null = hullMeshFromHandle(runtime, boxHull, 0xf5deb3);
    boxMesh.position.set(...shapeDistanceTransformB.position);
    scene.add(boxMesh);
    const triangle = addTriangle(scene, shapeDistanceTriangle, 0x22d3ee);
    const pointA = createDebugPoint(scene, 0x86efac, 10);
    const pointB = createDebugPoint(scene, 0x93c5fd, 10);
    const connector = createDebugLine(scene, 0xffffff);
    let info = "";
    const refresh = () => {
      const result = runShapeDistance(runtime, typeA, typeB, radiusA, radiusB);
      updateDebugPoint(pointA, result.pA);
      updateDebugPoint(pointB, result.pB);
      updateDebugLine(connector, result.pA, result.pB);
      info = `distance = ${result.d.toFixed(4)}, iterations = ${result.i}`;
    };
    refresh();
    return {
      world,
      bodies: [],
      camera: shapeDistanceCamera,
      getInfo: () => info,
      controls: [
        { key: "shape-a", label: "shape A", type: "range", min: 0, max: 3, step: 1, value: typeA, onChange: (v) => { if (typeof v === "number") { typeA = v as ShapeDistanceTypeId; refresh(); } } },
        { key: "radius-a", label: "radius A", type: "range", min: 0, max: 0.5, step: 0.01, value: radiusA, onChange: (v) => { if (typeof v === "number") { radiusA = v; refresh(); } } },
        { key: "shape-b", label: "shape B", type: "range", min: 0, max: 3, step: 1, value: typeB, onChange: (v) => { if (typeof v === "number") { typeB = v as ShapeDistanceTypeId; refresh(); } } },
        { key: "radius-b", label: "radius B", type: "range", min: 0, max: 0.5, step: 0.01, value: radiusB, onChange: (v) => { if (typeof v === "number") { radiusB = v; refresh(); } } },
      ],
      step() { refresh(); },
      dispose() {
        if (boxHull !== null) runtime.destroyHull(boxHull);
        disposeHullMesh(scene, boxMesh);
        scene.remove(axes);
        scene.remove(triangle);
        triangle.geometry.dispose();
        (triangle.material as THREE.Material).dispose();
        disposeDebugObject(scene, pointA);
        disposeDebugObject(scene, pointB);
        disposeDebugObject(scene, connector);
        void shapeDistanceSegment;
        world.destroy();
      },
    };
  },
};
