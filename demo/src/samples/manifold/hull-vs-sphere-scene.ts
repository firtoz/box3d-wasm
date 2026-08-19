import { type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  dumpCheckpointManifold,
  dumpCreateManifold,
  dumpStepManifold,
  IDENTITY_XF,
  manifoldCamera,
  type ManifoldScene,
} from "./manifold-shared";

const SPHERE = { center: [0, 0, 0] as Vec3, radius: 1 };
const XF_A = IDENTITY_XF;
const XF_B: typeof IDENTITY_XF = { position: [1.5, 0, 0], rotation: [0, 0, 0, 1] };

export const hullVsSphereScene: ManifoldScene = {
  id: "manifold/hull-vs-sphere",
  name: "Manifold / Hull vs Sphere",
  cppName: "Hull vs Sphere",
  info: "b3CollideHullAndSphere against a 2×0.5×0.5 box",
  camera: manifoldCamera(35, 30, 50),
  draw: [
    { kind: "box", transform: XF_A, size: [4, 1, 1], color: 0x22d3ee },
    { kind: "sphere", transform: XF_B, center: SPHERE.center, radius: SPHERE.radius, color: 0x22c55e },
  ],
  create(runtime: Box3DRuntime) {
    const hull = runtime.makeBoxHull([2, 0.5, 0.5]);
    return {
      collide: (rt) => rt.collideHullAndSphere(hull, SPHERE, XF_A, XF_B),
      dispose: () => runtime.destroyHull(hull),
      contactFrame: XF_A,
    };
  },
};

export const dumpSampleName = hullVsSphereScene.cppName;
export const dumpSampleId = hullVsSphereScene.id;
export const dumpCppSampleName = hullVsSphereScene.cppName;
export const dumpCreate = (runtime: Box3DRuntime) => dumpCreateManifold(runtime, hullVsSphereScene);
export const dumpStep = dumpStepManifold;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointManifold;
