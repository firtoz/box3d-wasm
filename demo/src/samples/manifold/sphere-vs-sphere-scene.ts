import { type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  DEFAULT_MANIFOLD_A,
  DEFAULT_MANIFOLD_B,
  defaultManifoldA,
  dumpCheckpointManifold,
  dumpCreateManifold,
  dumpStepManifold,
  manifoldCamera,
  type ManifoldScene,
} from "./manifold-shared";

const SPHERE = { center: [0.5, 0, -0.25] as Vec3, radius: 2 };

export const sphereVsSphereScene: ManifoldScene = {
  id: "manifold/sphere-vs-sphere",
  name: "Manifold / Sphere vs Sphere",
  cppName: "Sphere vs Sphere",
  info: "b3CollideSpheres at the default Manifold poses",
  camera: manifoldCamera(35, 30, 50),
  draw: [
    { kind: "sphere", transform: DEFAULT_MANIFOLD_A, center: SPHERE.center, radius: SPHERE.radius, color: 0x22c55e },
    { kind: "sphere", transform: DEFAULT_MANIFOLD_B, center: SPHERE.center, radius: SPHERE.radius, color: 0x22d3ee },
  ],
  create(runtime: Box3DRuntime) {
    const xfA = defaultManifoldA(runtime);
    const xfB = DEFAULT_MANIFOLD_B;
    return {
      collide: (rt) => rt.collideSpheres(SPHERE, SPHERE, xfA, xfB),
      dispose: () => {},
      contactFrame: xfA,
    };
  },
};

export const dumpSampleName = sphereVsSphereScene.cppName;
export const dumpSampleId = sphereVsSphereScene.id;
export const dumpCppSampleName = sphereVsSphereScene.cppName;
export const dumpCreate = (runtime: Box3DRuntime) => dumpCreateManifold(runtime, sphereVsSphereScene);
export const dumpStep = dumpStepManifold;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointManifold;
