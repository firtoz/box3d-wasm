import { type Box3DRuntime, type Vec3, type WorldTransform } from "box3d-wasm";
import {
  dumpCheckpointManifold,
  dumpCreateManifold,
  dumpStepManifold,
  IDENTITY_XF,
  manifoldCamera,
  type ManifoldScene,
} from "./manifold-shared";

const CAPSULE = { center1: [-1, 0, 0] as Vec3, center2: [1, 0, 0] as Vec3, radius: 0.15 };
const XF_A = IDENTITY_XF;
const XF_B: WorldTransform = {
  position: [1.58523774, 0.729615569, 0.451690674],
  rotation: [-0.00256555085, -0.0201825816, 0.126076236, 0.991811991],
};

export const capsuleVsHullScene: ManifoldScene = {
  id: "manifold/capsule-vs-hull",
  name: "Manifold / Capsule vs Hull",
  cppName: "Capsule vs Hull",
  info: "b3CollideHullAndCapsule",
  camera: manifoldCamera(0, 30, 5, [0, 0, 0]),
  draw: [
    { kind: "box", transform: XF_A, size: [2, 1, 1], color: 0x22d3ee },
    { kind: "capsule", transform: XF_B, ...CAPSULE, color: 0x22c55e },
  ],
  create(runtime: Box3DRuntime) {
    const hull = runtime.makeBoxHull([1, 0.5, 0.5]);
    return {
      collide: (rt) => rt.collideHullAndCapsule(hull, CAPSULE, XF_A, XF_B),
      dispose: () => runtime.destroyHull(hull),
      contactFrame: XF_A,
    };
  },
};

export const dumpSampleName = capsuleVsHullScene.cppName;
export const dumpSampleId = capsuleVsHullScene.id;
export const dumpCppSampleName = capsuleVsHullScene.cppName;
export const dumpCreate = (runtime: Box3DRuntime) => dumpCreateManifold(runtime, capsuleVsHullScene);
export const dumpStep = dumpStepManifold;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointManifold;
