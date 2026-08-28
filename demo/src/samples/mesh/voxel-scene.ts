import { BodyType, type BodyId, type Box3DRuntime, type MeshHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32Mul } from "../f32";
import { cameraFromSetView, getWasmBaseUrl } from "../shared";
import { parseObjText, transformObjVertices } from "../meshes/parse-obj";

const ORIGIN: Vec3 = [5000, 3500, -7000];
const CYLINDER_POS: Vec3 = [5020.27734, 3506.22559, -6986.48584];
const CYLINDER_ROT: [number, number, number, number] = [0.664546967, 0.669287264, 0.135021493, 0.303646326];

const RAW_POINTS: ReadonlyArray<Vec3> = [
  [-3.13548756, 3.81141949, 237.289047],
  [-16.2333279, -23.4977913, 235.486603],
  [-13.8834839, 6.20244455, 23.7760544],
  [14.0794125, 4.63170528, 24.9530792],
  [3.98322797, -16.4192238, 236.704071],
  [-23.3520412, -3.26714420, 236.071594],
  [13.4517860, -6.94963741, 24.4085312],
  [-5.24953651, 13.9316301, 24.5058060],
  [-4.65071201, -24.1484108, 235.974121],
  [-14.5111103, -5.37889385, 23.2315063],
  [6.33307076, 13.2810068, 24.9935150],
  [4.81784487, -14.6788225, 23.6787796],
  [-14.7180958, 4.46204281, 236.801331],
  [-23.9796677, -14.8484812, 235.527039],
  [4.61085415, -4.83788204, 237.248611],
  [-6.76476669, -14.0281992, 23.1910706],
];

function cylinderPoints(): number[] {
  const points: number[] = [];
  for (const p of RAW_POINTS) {
    const x = f32Mul(0.01, p[1]);
    const y = f32Mul(0.01, p[2]);
    const z = f32Mul(0.01, p[0]);
    points.push(x, y, z);
  }
  return points;
}

export function buildVoxelDumpBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const ground = world.createBody({ type: BodyType.Static, position: ORIGIN });
  const hull = runtime.createHullFromPoints(cylinderPoints(), 16);
  const body = world.createBody({
    type: BodyType.Dynamic,
    position: CYLINDER_POS,
    rotation: CYLINDER_ROT,
  });
  runtime.createShapeFromHull(body, hull, { rollingResistance: 0.1 });
  runtime.destroyHull(hull);
  return [ground, body];
}

export async function buildVoxelLive(world: PhysicsWorld, runtime: Box3DRuntime): Promise<{
  handles: BodyId[];
  meshes: MeshHandle[];
}> {
  const handles = buildVoxelDumpBodies(world, runtime);
  const meshes: MeshHandle[] = [];
  try {
    const response = await fetch(`${getWasmBaseUrl()}meshes/collision_mesh_01.obj`);
    const text = await response.text();
    const parsed = parseObjText(text);
    const vertices = transformObjVertices(parsed.vertices, 0.01, true);
    const mesh = world.createMesh(vertices, parsed.indices, {
      useMedianSplit: true,
      identifyEdges: true,
      weldVertices: true,
      weldTolerance: 0.002,
    });
    world.createMeshShape(handles[0]!, mesh, { scale: [1, 1, 1] });
    meshes.push(mesh);
  } catch {
    // LoadTempMesh stub returns false in dumps.
  }
  return { handles, meshes };
}

export function voxelGroundSize(): Vec3 {
  return [10, 1, 10];
}

export function createVoxelBodies(): RenderBody[] {
  return [
    { kind: "box", size: [40, 0.2, 40], position: ORIGIN, type: BodyType.Static, color: 0x475569 },
    { kind: "hull", points: chunkPoints(cylinderPoints()), position: CYLINDER_POS, rotation: CYLINDER_ROT, color: 0xf59e0b },
  ];
}

function chunkPoints(flat: number[]): [number, number, number][] {
  const points: [number, number, number][] = [];
  for (let i = 0; i + 2 < flat.length; i += 3) points.push([flat[i]!, flat[i + 1]!, flat[i + 2]!]);
  return points;
}

export const voxelCamera: RenderSpec["camera"] = cameraFromSetView(-115, 5, 5, [ORIGIN[0], ORIGIN[1] + 10, ORIGIN[2]]);

export const dumpSampleName = "Voxel";
export const dumpSampleId = "mesh/voxel";
export const dumpCppSampleName = "Voxel";
export const dumpGroundSize = voxelGroundSize;

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildVoxelDumpBodies(world, runtime) };
}
