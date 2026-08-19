import { type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  dumpCheckpointManifold,
  dumpCreateManifold,
  dumpStepManifold,
  IDENTITY_XF,
  manifoldCamera,
  type ManifoldScene,
} from "./manifold-dump";

const SPHERE = { center: [0, 0, 0] as Vec3, radius: 0.25 };
const TRIANGLE: readonly [Vec3, Vec3, Vec3] = [
  [0, 0, 0],
  [4, 0, 4],
  [4, 0, 0],
];
const XF_A = IDENTITY_XF;
const XF_B: typeof IDENTITY_XF = { position: [2, 0.5, 1], rotation: [0, 0, 0, 1] };

export const triangleVsSphereScene: ManifoldScene = {
  id: "manifold/triangle-vs-sphere",
  name: "Manifold / Triangle vs Sphere",
  cppName: "Triangle vs Sphere",
  info: "b3CollideTriangleAndSphere",
  camera: manifoldCamera(0, 30, 10, [0, 0, 0]),
  draw: [
    { kind: "triangle", transform: XF_A, vertices: TRIANGLE, color: 0x22d3ee },
    { kind: "sphere", transform: XF_B, center: SPHERE.center, radius: SPHERE.radius, color: 0x22c55e, opacity: 0.5 },
  ],
  create(_runtime: Box3DRuntime) {
    return {
      collide: (rt) => rt.collideTriangleAndSphere(TRIANGLE, SPHERE, XF_A, XF_B),
      dispose: () => {},
      contactFrame: XF_B,
    };
  },
};

export const dumpSampleName = triangleVsSphereScene.cppName;
export const dumpSampleId = triangleVsSphereScene.id;
export const dumpCppSampleName = triangleVsSphereScene.cppName;
export const dumpCreate = (runtime: Box3DRuntime) => dumpCreateManifold(runtime, triangleVsSphereScene);
export const dumpStep = dumpStepManifold;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointManifold;
