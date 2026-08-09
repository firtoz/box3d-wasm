import {BodyType, type Box3DRuntime, type MeshHandle, type PhysicsWorld, type Vec3, type BodyId} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Mul } from "../f32";

const CELL_WIDTH = f32(8);
const BOX_HALF: Vec3 = [f32(0.5), f32(0.05), f32(1)];

export function humpMeshVertices(cellWidth: number = CELL_WIDTH): { vertices: number[]; indices: number[] } {
  const vertices: number[] = [];
  let index = 0;
  let x = f32Mul(-0.5, cellWidth);
  for (let ix = 0; ix <= 1; ix++) {
    let z = f32Mul(-1, cellWidth);
    for (let iz = 0; iz <= 2; iz++) {
      let y = f32(0);
      if (iz === 1) y = f32Mul(0.05, cellWidth);
      vertices.push(x, y, z);
      z = f32Add(z, cellWidth);
      index += 1;
    }
    x = f32Add(x, cellWidth);
  }

  const indices: number[] = [];
  for (let ix = 0; ix < 1; ix++) {
    for (let iz = 0; iz < 2; iz++) {
      const index1 = iz + 3 * ix;
      const index2 = index1 + 1;
      const index3 = index2 + 3;
      const index4 = index3 - 1;
      indices.push(index1, index2, index3, index3, index4, index1);
    }
  }
  return { vertices, indices };
}

/** Port of Continuous HumpMesh::CreateHump. */
export function createHumpMeshData(world: PhysicsWorld, cellWidth: number = CELL_WIDTH): MeshHandle {
  const { vertices, indices } = humpMeshVertices(cellWidth);
  return world.createMesh(vertices, indices, { useMedianSplit: true, identifyEdges: true });
}

export interface HumpMeshScene {
  humpBody: BodyId;
  dynamic: BodyId;
  mesh: MeshHandle;
}

export function buildHumpMeshScene(world: PhysicsWorld, runtime: Box3DRuntime): HumpMeshScene {
  const mesh = createHumpMeshData(world);
  const humpBody = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  world.createMeshShape(humpBody, mesh, { scale: [1, 1, 1] });

  const dynamic = world.createBody({
    type: BodyType.Dynamic,
    position: [0, 5, 0],
    linearVelocity: [0, -50, 0],
  });
  runtime.createHullShape(dynamic, BOX_HALF, {});
  return { humpBody, dynamic, mesh };
}

/** After AddGroundBox: [humpBody, dynamic] — dump order matches C++. */
export function buildHumpMeshDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const { humpBody, dynamic } = buildHumpMeshScene(world, runtime);
  return [humpBody, dynamic];
}

export function humpMeshGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createHumpMeshBodies(): RenderBody[] {
  return [
    {
      kind: "box",
      size: [2 * BOX_HALF[0], 2 * BOX_HALF[1], 2 * BOX_HALF[2]],
      position: [0, 5, 0],
      color: 0xf59e0b,
    },
  ];
}

export const humpMeshCamera: RenderSpec["camera"] = cameraFromSetView(45, 25, 10, [0, 1.2, 0]);
export const humpMeshCellWidth = CELL_WIDTH;

export const dumpSampleName = "Hump Mesh";
export const dumpSampleId = "continuous/hump-mesh";
export const dumpCppSampleName = "Hump Mesh";
export const dumpGroundSize = humpMeshGroundSize;
export const dumpBuildDynamicBodies = buildHumpMeshDynamicBodies;
