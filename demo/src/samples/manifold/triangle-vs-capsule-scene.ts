import { type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  dumpCheckpointManifold,
  dumpCreateManifold,
  dumpStepManifold,
  IDENTITY_XF,
  manifoldCamera,
  type ManifoldScene,
} from "./manifold-shared";

const CAPSULE = { center1: [-0.5, 0, 0] as Vec3, center2: [0.5, 0, 0] as Vec3, radius: 0.05 };
const TRIANGLE: readonly [Vec3, Vec3, Vec3] = [
  [-4, 0, -4],
  [-4, 0, 0],
  [0, 0, 0],
];
const XF_A = IDENTITY_XF;
const XF_B = { position: [-1, 0, -1] as Vec3, rotation: [0, 0, 0, 1] as [number, number, number, number] };

export const triangleVsCapsuleScene: ManifoldScene = {
  id: "manifold/triangle-vs-capsule",
  name: "Manifold / Triangle vs Capsule",
  cppName: "Triangle vs Capsule",
  info: "b3CollideTriangleAndCapsule",
  camera: manifoldCamera(0, 30, 10, [0, 0, 0]),
  draw: [
    { kind: "triangle", transform: XF_A, vertices: TRIANGLE, color: 0x22d3ee },
    { kind: "capsule", transform: XF_B, ...CAPSULE, color: 0x22c55e, opacity: 0.5 },
  ],
  create(_runtime: Box3DRuntime) {
    return {
      collide: (rt) => rt.collideTriangleAndCapsule(TRIANGLE, CAPSULE, XF_A, XF_B),
      dispose: () => {},
      contactFrame: XF_B,
    };
  },
};

export const dumpSampleName = triangleVsCapsuleScene.cppName;
export const dumpSampleId = triangleVsCapsuleScene.id;
export const dumpCppSampleName = triangleVsCapsuleScene.cppName;
export const dumpCreate = (runtime: Box3DRuntime) => dumpCreateManifold(runtime, triangleVsCapsuleScene);
export const dumpStep = dumpStepManifold;
export const dumpOwnsStep = true;
export const dumpCheckpointExtras = dumpCheckpointManifold;
