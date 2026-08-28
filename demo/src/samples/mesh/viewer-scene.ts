import { BodyType, type BodyId, type Box3DRuntime, type MeshHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView, getWasmBaseUrl } from "../shared";
import { parseObjText, transformObjVertices } from "../meshes/parse-obj";

const FILES = ["voxel_mesh_01.obj", "voxel_mesh_02.obj", "voxel_mesh_03.obj", "voxel_mesh_04.obj"];

export type ViewerOptions = {
  meshIndex: number;
  medianSplit: boolean;
  concaveEdges: boolean;
  weldVertices: boolean;
  weldToleranceMillimeters: number;
};

export const viewerDefaultOptions: ViewerOptions = {
  meshIndex: 0,
  medianSplit: true,
  concaveEdges: true,
  weldVertices: true,
  weldToleranceMillimeters: 1.5,
};

export async function loadViewerMesh(world: PhysicsWorld, options: ViewerOptions): Promise<{ body: BodyId; mesh: MeshHandle | null }> {
  const body = world.createBody();
  const response = await fetch(`${getWasmBaseUrl()}meshes/${FILES[options.meshIndex] ?? FILES[0]}`);
  if (!response.ok) return { body, mesh: null };
  const parsed = parseObjText(await response.text());
  const vertices = transformObjVertices(parsed.vertices, 0.01, true);
  const mesh = world.createMesh(vertices, parsed.indices, {
    useMedianSplit: options.medianSplit,
    identifyEdges: options.concaveEdges,
    weldVertices: options.weldVertices,
    weldTolerance: 0.001 * options.weldToleranceMillimeters,
  });
  world.createMeshShape(body, mesh, { scale: [1, 1, 1] });
  return { body, mesh };
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: [world.createBody()] };
}

export function viewerGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const viewerBodies: RenderBody[] = [
  { kind: "box", size: [20, 4, 20], position: [0, 0, 0], type: BodyType.Static, color: 0x64748b },
];

export const viewerCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 50, [0, 0, 0]);

export const dumpSampleName = "Viewer";
export const dumpSampleId = "mesh/viewer";
export const dumpCppSampleName = "Viewer";
export const dumpGroundSize = viewerGroundSize;
