import { type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  dumpCheckpointManifold,
  dumpCreateManifold,
  dumpStepManifold,
  IDENTITY_XF,
  manifoldCamera,
  type ManifoldScene,
} from "./manifold-shared";

const TRIANGLE: readonly [Vec3, Vec3, Vec3] = [
  [0.299769998, -1.01549578, -0.744717002],
  [0.299769998, -1.01549578, 1.28728306],
  [0.299769998, -0.913895786, 0.271283031],
];
const HALF: Vec3 = [0.304800004, 0.914399981, 0.304800004];
const XF = IDENTITY_XF;

export const triangleVsHullScene: ManifoldScene = {
  id: "manifold/triangle-vs-hull",
  name: "Manifold / Triangle vs Hull",
  cppName: "Triangle vs Hull",
  info: "b3CollideTriangleAndHull",
  camera: manifoldCamera(0, 30, 3, [0, 0, 0]),
  draw: [
    { kind: "triangle", transform: XF, vertices: TRIANGLE, color: 0x22d3ee },
    { kind: "box", transform: XF, size: [2 * HALF[0], 2 * HALF[1], 2 * HALF[2]], color: 0x22c55e },
  ],
  create(runtime: Box3DRuntime) {
    const hull = runtime.makeBoxHull(HALF);
    return {
      collide: (rt) => rt.collideTriangleAndHull(TRIANGLE, hull, XF, XF, { triangleFlags: 0, enableSpeculative: true }),
      dispose: () => runtime.destroyHull(hull),
      contactFrame: XF,
    };
  },
};

export const dumpSampleName = triangleVsHullScene.cppName;
export const dumpSampleId = triangleVsHullScene.id;
export const dumpCppSampleName = triangleVsHullScene.cppName;
export const dumpCreate = (runtime: Box3DRuntime) => dumpCreateManifold(runtime, triangleVsHullScene);
export const dumpStep = dumpStepManifold;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointManifold;
