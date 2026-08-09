import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import {BodyType} from "box3d-wasm";
import {
  createDebugLine,
  createDebugPoint,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import { distanceJointBodies, distanceJointCamera, distanceJointGroundSize } from "./distance-joint-scene";

const half = distanceJointGroundSize();
/** Anchor body is at the origin; local pivot matches C++ `GetLocalPoint(0, 20, 0)`. */
const ANCHOR_LOCAL: [number, number, number] = [0, 20, 0];
const tmp = new THREE.Vector3();
const pA = [0, 0, 0] as [number, number, number];
const pB = [0, 0, 0] as [number, number, number];

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  // Index 0 = empty static anchor (tiny placeholder; joint draw shows the real attachment).
  bodies: [
    { kind: "sphere", radius: 0.05, position: [0, 20, 0], color: 0x64748b, type: BodyType.Static },
    ...distanceJointBodies,
  ],
  camera: distanceJointCamera,
  info: "distance joint (CreateScene count=1, spring off) — sphere hangs from empty static anchor",
  overlay: (scene) => {
    // Mirror `b3DrawDistanceJoint`: white A–B segment + white points (no limit/spring markers).
    const segment = createDebugLine(scene, 0xffffff);
    const pointA = createDebugPoint(scene, 0xffffff, 4);
    const pointB = createDebugPoint(scene, 0xffffff, 4);
    return {
      update({ bodies }) {
        const anchor = bodies[0]?.mesh;
        const sphere = bodies[1]?.mesh;
        if (anchor === undefined || sphere === undefined) return;
        tmp.set(...ANCHOR_LOCAL).applyQuaternion(anchor.quaternion).add(anchor.position);
        pA[0] = tmp.x;
        pA[1] = tmp.y;
        pA[2] = tmp.z;
        // Local pivot B is body origin (sphere center).
        pB[0] = sphere.position.x;
        pB[1] = sphere.position.y;
        pB[2] = sphere.position.z;
        updateDebugLine(segment, pA, pB);
        updateDebugPoint(pointA, pA);
        updateDebugPoint(pointB, pB);
      },
      dispose() {
        disposeDebugObject(scene, segment);
        disposeDebugObject(scene, pointA);
        disposeDebugObject(scene, pointB);
      },
    };
  },
};

export const distanceJointSample = createGenericSample(
  "joints/distance-joint",
  "Joints / Distance Joint",
  spec,
  () => new Worker(new URL("./distance-joint.worker.ts", import.meta.url), { type: "module" }),
);
