import { type Box3DRuntime } from "box3d-wasm";
import {
  dumpCheckpointManifold,
  dumpCreateManifold,
  dumpStepManifold,
  IDENTITY_XF,
  manifoldCamera,
  type ManifoldScene,
} from "./manifold-dump";

const XF = IDENTITY_XF;

export const hullVsHullScene: ManifoldScene = {
  id: "manifold/hull-vs-hull",
  name: "Manifold / Hull vs Hull",
  cppName: "Hull vs Hull",
  info: "b3CollideHulls with a transformed box versus a unit box",
  camera: manifoldCamera(0, 15, 4, [0, 0, 0]),
  draw: [
    { kind: "box", transform: XF, size: [1, 2, 2], localPosition: [1, 0.5, 0], color: 0x22c55e },
    { kind: "box", transform: XF, size: [1, 1, 1], color: 0x22d3ee },
  ],
  create(runtime: Box3DRuntime) {
    const hullA = runtime.makeTransformedBoxHull([0.5, 1, 1], { position: [1, 0.5, 0] });
    const hullB = runtime.makeBoxHull([0.5, 0.5, 0.5]);
    return {
      collide: (rt) => rt.collideHulls(hullA, hullB, XF, XF),
      dispose: () => {
        runtime.destroyHull(hullA);
        runtime.destroyHull(hullB);
      },
      contactFrame: XF,
    };
  },
};

export const dumpSampleName = hullVsHullScene.cppName;
export const dumpSampleId = hullVsHullScene.id;
export const dumpCppSampleName = hullVsHullScene.cppName;
export const dumpCreate = (runtime: Box3DRuntime) => dumpCreateManifold(runtime, hullVsHullScene);
export const dumpStep = dumpStepManifold;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointManifold;
