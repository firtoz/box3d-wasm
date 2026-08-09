import {
  B3_PI,
  BodyType,
  type Box3DRuntime,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Div, f32Mul } from "../f32";

const BOX_HALF: Vec3 = [f32(0.3), f32(0.01), f32(0.3)];
const SLICES = 8;

/** Port of Continuous NeedleMesh::CreateNeedle using `b3ComputeCosSin` via b3wSin/b3wCos. */
export function createNeedleMesh(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  height: number,
  radius: number,
  center: Vec3,
  slices: number = SLICES,
): MeshHandle {
  const vertexCount = slices + 1;
  const vertices: number[] = new Array(vertexCount * 3);
  vertices[0] = f32Add(0, center[0]);
  vertices[1] = f32Add(height, center[1]);
  vertices[2] = f32Add(0, center[2]);

  let alpha = f32(0);
  const deltaAlpha = f32Div(f32Mul(2, B3_PI), slices);
  for (let index = 1; index < vertexCount; index++) {
    const cosine = runtime.b3wCos(alpha);
    const sine = runtime.b3wSin(alpha);
    vertices[index * 3] = f32Add(f32Mul(radius, cosine), center[0]);
    vertices[index * 3 + 1] = f32Add(0, center[1]);
    vertices[index * 3 + 2] = f32Add(f32Mul(radius, sine), center[2]);
    alpha = f32Add(alpha, deltaAlpha);
  }

  const triangleCount = slices;
  const indices: number[] = new Array(3 * triangleCount);
  let index1 = vertexCount - 1;
  for (let index = 0; index < triangleCount; index++) {
    const index2 = index + 1;
    indices[3 * index] = 0;
    indices[3 * index + 1] = index2;
    indices[3 * index + 2] = index1;
    index1 = index2;
  }

  return world.createMesh(vertices, indices, { useMedianSplit: true, identifyEdges: false });
}

export interface NeedleMeshScene {
  ground: number;
  dynamic: number;
  meshes: MeshHandle[];
}

export function buildNeedleMeshScene(world: PhysicsWorld, runtime: Box3DRuntime): NeedleMeshScene {
  const needle1 = createNeedleMesh(world, runtime, f32(0.99), f32(0.1), [f32(0.2), 0, f32(0.2)]);
  const needle2 = createNeedleMesh(world, runtime, f32(1.01), f32(0.1), [f32(0.2), 0, f32(-0.2)]);
  const needle3 = createNeedleMesh(world, runtime, f32(0.98), f32(0.1), [f32(-0.2), 0, f32(-0.2)]);
  const needle4 = createNeedleMesh(world, runtime, f32(1.02), f32(0.1), [f32(-0.2), 0, f32(0.2)]);
  const meshes = [needle1, needle2, needle3, needle4];

  const ground = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  for (const mesh of meshes) {
    world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });
  }

  const dynamic = world.createBody({
    type: BodyType.Dynamic,
    position: [0, 5, 0],
    linearVelocity: [0, -10, 0],
  });
  runtime.createHullShape(dynamic, BOX_HALF, {});

  return { ground, dynamic, meshes };
}

export function needleMeshGroundSize(): Vec3 {
  return [5, 1, 5];
}

export function createNeedleMeshBodies(): RenderBody[] {
  return [
    {
      kind: "box",
      size: [2 * BOX_HALF[0], 2 * BOX_HALF[1], 2 * BOX_HALF[2]],
      position: [0, 5, 0],
      color: 0xf59e0b,
    },
  ];
}

export const needleMeshCamera: RenderSpec["camera"] = cameraFromSetView(45, 25, 4, [0, 1.2, 0]);

export const dumpSampleName = "Needle Mesh";
export const dumpSampleId = "continuous/needle-mesh";
export const dumpCppSampleName = "Needle Mesh";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: { meshes: MeshHandle[] };
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, dynamic, meshes } = buildNeedleMeshScene(world, runtime);
  return {
    world,
    handles: [ground, dynamic],
    state: { meshes },
    dispose: () => {
      for (let i = meshes.length - 1; i >= 0; i--) world.destroyMesh(meshes[i]!);
    },
  };
}

/** Needle tip specs for host overlay (matches CreateNeedle params). */
export const needleOverlaySpecs = [
  { height: 0.99, radius: 0.1, center: [0.2, 0, 0.2] as Vec3 },
  { height: 1.01, radius: 0.1, center: [0.2, 0, -0.2] as Vec3 },
  { height: 0.98, radius: 0.1, center: [-0.2, 0, -0.2] as Vec3 },
  { height: 1.02, radius: 0.1, center: [-0.2, 0, 0.2] as Vec3 },
] as const;
