import { type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  dumpCheckpointManifold,
  dumpCreateManifold,
  dumpStepManifold,
  IDENTITY_XF,
  manifoldCamera,
  type ManifoldScene,
} from "./manifold-shared";

const CAPSULE = { center1: [-2, 0, 0] as Vec3, center2: [2, 0, 0] as Vec3, radius: 1 };
const SPHERE = { center: [0, 0, 0] as Vec3, radius: 2 };
const XF_A = IDENTITY_XF;
const XF_B: typeof IDENTITY_XF = { position: [-4, 0, 0], rotation: [0, 0, 0, 1] };

export const capsuleVsSphereScene: ManifoldScene = {
  id: "manifold/capsule-vs-sphere",
  name: "Manifold / Capsule vs Sphere",
  cppName: "Capsule vs Sphere",
  info: "b3CollideCapsuleAndSphere",
  camera: manifoldCamera(35, 30, 50),
  draw: [
    { kind: "capsule", transform: XF_A, ...CAPSULE, color: 0x22d3ee },
    { kind: "sphere", transform: XF_B, center: SPHERE.center, radius: SPHERE.radius, color: 0x22c55e },
  ],
  create(_runtime: Box3DRuntime) {
    return {
      collide: (rt) => rt.collideCapsuleAndSphere(CAPSULE, SPHERE, XF_A, XF_B),
      dispose: () => {},
      contactFrame: XF_A,
    };
  },
};

export const dumpSampleName = capsuleVsSphereScene.cppName;
export const dumpSampleId = capsuleVsSphereScene.id;
export const dumpCppSampleName = capsuleVsSphereScene.cppName;
export const dumpCreate = (runtime: Box3DRuntime) => dumpCreateManifold(runtime, capsuleVsSphereScene);
export const dumpStep = dumpStepManifold;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointManifold;
