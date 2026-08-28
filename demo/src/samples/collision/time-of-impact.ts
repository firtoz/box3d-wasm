import * as THREE from "three";
import type { Box3DRuntime, HullHandle } from "box3d-wasm";
import { capsuleMesh } from "../shared";
import type { DemoSample } from "../types";
import { createDebugLine, createDebugPoint, disposeDebugObject, updateDebugLine, updateDebugPoint } from "../debug-overlay";
import { disposeHullMesh, hullMeshFromHandle } from "../geometry/hull-draw";
import {
  ToiShape,
  ToiState,
  runTimeOfImpact,
  timeOfImpactCamera,
  timeOfImpactCapsule,
  timeOfImpactSweepB,
  timeOfImpactTriangle,
  type ToiShapeId,
} from "./time-of-impact-scene";

function addTriangle(scene: THREE.Scene, color: number): THREE.LineSegments {
  const [a, b, c] = timeOfImpactTriangle;
  const geometry = new THREE.BufferGeometry();
  geometry.setAttribute("position", new THREE.BufferAttribute(new Float32Array([...a, ...b, ...b, ...c, ...c, ...a]), 3));
  const lines = new THREE.LineSegments(geometry, new THREE.LineBasicMaterial({ color, toneMapped: false }));
  scene.add(lines);
  return lines;
}

function addCapsule(scene: THREE.Scene, color: number): THREE.Mesh {
  const c1 = new THREE.Vector3(...timeOfImpactCapsule.center1);
  const c2 = new THREE.Vector3(...timeOfImpactCapsule.center2);
  const mesh = capsuleMesh(timeOfImpactCapsule.radius, c1.distanceTo(c2), color, 0.85, "y");
  const dir = c2.clone().sub(c1);
  if (dir.lengthSq() > 0) mesh.quaternion.setFromUnitVectors(new THREE.Vector3(0, 1, 0), dir.normalize());
  const mid = c1.clone().add(c2).multiplyScalar(0.5);
  mesh.position.copy(mid);
  scene.add(mesh);
  return mesh;
}

function applyWorld(mesh: THREE.Object3D, xf: { position: readonly number[]; rotation: readonly number[] }): void {
  mesh.position.set(xf.position[0]!, xf.position[1]!, xf.position[2]!);
  mesh.quaternion.set(xf.rotation[0]!, xf.rotation[1]!, xf.rotation[2]!, xf.rotation[3]!);
}

const STATE_LABEL: Record<number, string> = {
  [ToiState.Unknown]: "unknown",
  [ToiState.Failed]: "failed",
  [ToiState.Overlapped]: "overlapped",
  [ToiState.Hit]: "hit",
  [ToiState.Separated]: "separated",
};

export const timeOfImpactSample: DemoSample = {
  id: "collision/time-of-impact",
  name: "Collision / Time of Impact",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const axes = new THREE.AxesHelper(0.5);
    scene.add(axes);
    let typeA: ToiShapeId = ToiShape.Triangle;
    let typeB: ToiShapeId = ToiShape.Capsule;
    const triangle = addTriangle(scene, 0x22d3ee);
    const start = addCapsule(scene, 0x86efac);
    const end = addCapsule(scene, 0xf87171);
    const hit = addCapsule(scene, 0x67e8f9);
    const point = createDebugPoint(scene, 0x86efac, 10);
    const normal = createDebugLine(scene, 0x6b7280);
    let boxHull: HullHandle | null = runtime.makeBoxHull([0.02, 0.2, 0.04]);
    let boxMesh: THREE.Object3D | null = hullMeshFromHandle(runtime, boxHull, 0xf5deb3);
    boxMesh.visible = false;
    scene.add(boxMesh);
    let info = "";
    const refresh = () => {
      const result = runTimeOfImpact(runtime, typeA, typeB);
      const xf0 = runtime.getSweepTransform(timeOfImpactSweepB, 0);
      const xf1 = runtime.getSweepTransform(timeOfImpactSweepB, 1);
      applyWorld(start, xf0);
      applyWorld(end, xf1);
      hit.visible = result.fraction < 1;
      if (hit.visible) applyWorld(hit, runtime.getSweepTransform(timeOfImpactSweepB, result.fraction));
      updateDebugPoint(point, result.point);
      updateDebugLine(normal, result.point, [
        result.point[0] + 0.5 * result.normal[0],
        result.point[1] + 0.5 * result.normal[1],
        result.point[2] + 0.5 * result.normal[2],
      ]);
      const label = STATE_LABEL[result.state] ?? "unknown";
      info = `${label}${result.state === ToiState.Hit ? ` ${result.fraction}` : ""} | iterations / push / root = ${result.distanceIterations} / ${result.pushBackIterations} / ${result.rootIterations}`;
    };
    refresh();
    return {
      world,
      bodies: [],
      camera: timeOfImpactCamera,
      getInfo: () => info,
      controls: [
        { key: "shape-a", label: "shape A", type: "range", min: 0, max: 2, step: 1, value: typeA, onChange: (v) => { if (typeof v === "number") { typeA = v as ToiShapeId; refresh(); } } },
        { key: "shape-b", label: "shape B", type: "range", min: 0, max: 2, step: 1, value: typeB, onChange: (v) => { if (typeof v === "number") { typeB = v as ToiShapeId; refresh(); } } },
      ],
      step() { refresh(); },
      dispose() {
        if (boxHull !== null) runtime.destroyHull(boxHull);
        disposeHullMesh(scene, boxMesh);
        scene.remove(axes);
        scene.remove(triangle);
        triangle.geometry.dispose();
        (triangle.material as THREE.Material).dispose();
        for (const mesh of [start, end, hit]) {
          scene.remove(mesh);
          mesh.geometry.dispose();
          (mesh.material as THREE.Material).dispose();
        }
        disposeDebugObject(scene, point);
        disposeDebugObject(scene, normal);
        world.destroy();
      },
    };
  },
};
