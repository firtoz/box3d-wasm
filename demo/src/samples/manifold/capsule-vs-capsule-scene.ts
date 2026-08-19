import { type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  dumpCheckpointManifold,
  dumpCreateManifold,
  dumpStepManifold,
  manifoldCamera,
  type ManifoldScene,
} from "./manifold-shared";

const CAPSULE = { center1: [-2, 0, 0] as Vec3, center2: [2, 0, 0] as Vec3, radius: 1 };
const XF_A = { position: [1, 1, 0] as Vec3, rotation: [0, 0, 0, 1] as [number, number, number, number] };
const XF_B = { position: [-4, 1, 0] as Vec3, rotation: [0, 0, 0, 1] as [number, number, number, number] };

export const capsuleVsCapsuleScene: ManifoldScene = {
  id: "manifold/capsule-vs-capsule",
  name: "Manifold / Capsule vs Capsule",
  cppName: "Capsule vs Capsule",
  info: "b3CollideCapsules",
  camera: manifoldCamera(35, 30, 50),
  draw: [
    { kind: "capsule", transform: XF_A, ...CAPSULE, color: 0x22c55e },
    { kind: "capsule", transform: XF_B, ...CAPSULE, color: 0x22d3ee },
  ],
  create(_runtime: Box3DRuntime) {
    return {
      collide: (rt) => rt.collideCapsules(CAPSULE, CAPSULE, XF_A, XF_B),
      dispose: () => {},
      contactFrame: XF_A,
    };
  },
};

export const dumpSampleName = capsuleVsCapsuleScene.cppName;
export const dumpSampleId = capsuleVsCapsuleScene.id;
export const dumpCppSampleName = capsuleVsCapsuleScene.cppName;
export const dumpCreate = (runtime: Box3DRuntime) => dumpCreateManifold(runtime, capsuleVsCapsuleScene);
export const dumpStep = dumpStepManifold;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointManifold;
