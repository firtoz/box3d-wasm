import { BodyType, type BodyId, type Box3DRuntime, type HullHandle, type MeshHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32Add, f32Mul } from "../f32";
import { cameraFromSetView, getWasmBaseUrl } from "../shared";
import { parseObjText } from "../meshes/parse-obj";

const GROUND_HALF: Vec3 = [20, 1, 20];
const CYLINDER_HEIGHT = 0.3;
const CYLINDER_RADIUS = 0.15;
const MESH_POS: Vec3 = [0, 0.5, 6];

const VELOCITIES: ReadonlyArray<Vec3> = [
  [0, 0, 0],
  [0.7, 0, -0.2],
  [0.6, 0, 0.4],
  [0, 0, 1.3],
  [-0.6, 0, 0.4],
  [-0.75, 0, -0.4],
  [0, 0, -1.3],
];

function conveyorMaterials() {
  return VELOCITIES.map((v) => ({
    friction: 0.8,
    tangentVelocity: [f32Mul(2, v[0]), f32Mul(2, v[1]), f32Mul(2, v[2])] as Vec3,
  }));
}

function materialIndices(triangleCount: number): Uint8Array {
  const indices = new Uint8Array(triangleCount);
  const assign = (tri: number, mat: number) => {
    if (tri >= 0 && tri < triangleCount) indices[tri] = mat;
  };
  assign(0, 1); assign(4, 1);
  assign(9, 2); assign(12, 2);
  assign(21, 3); assign(38, 3);
  assign(43, 4); assign(46, 4);
  assign(30, 5); assign(33, 5);
  assign(18, 6); assign(24, 6);
  return indices;
}

export function conveyorCylinderPosition(index: number): Vec3 {
  return [f32Add(-8.5, f32Mul(0.9, index)), 1.5, -5.5];
}

export function buildConveyorCylinders(world: PhysicsWorld, runtime: Box3DRuntime): { handles: BodyId[]; hull: HullHandle } {
  const hull = runtime.createCylinder(CYLINDER_HEIGHT, CYLINDER_RADIUS, 0, 32);
  const handles: BodyId[] = [];
  for (let i = 0; i < 20; i++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: conveyorCylinderPosition(i) });
    runtime.createShapeFromHull(body, hull, {});
    handles.push(body);
  }
  return { handles, hull };
}

export async function attachConveyorMesh(world: PhysicsWorld, runtime: Box3DRuntime): Promise<{ mesh: MeshHandle; body: BodyId } | null> {
  try {
    const response = await fetch(`${getWasmBaseUrl()}meshes/conveyor.obj`);
    const text = await response.text();
    const parsed = parseObjText(text);
    const triangleCount = Math.floor(parsed.indices.length / 3);
    const rotation = runtime.makeQuatFromAxisAngle([0, 1, 0], 0.5 * Math.PI);
    const mesh = world.createMesh(parsed.vertices, parsed.indices, {
      useMedianSplit: true,
      identifyEdges: true,
      weldVertices: true,
      materialIndices: materialIndices(triangleCount),
    });
    const body = world.createBody({ position: MESH_POS, rotation });
    world.createMeshShape(body, mesh, { scale: [1, 1, 1], surfaceMaterials: conveyorMaterials() });
    return { mesh, body };
  } catch {
    return null;
  }
}

export function conveyorMeshGroundSize(): Vec3 {
  return GROUND_HALF;
}

export function buildConveyorMeshDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const { handles, hull } = buildConveyorCylinders(world, runtime);
  runtime.destroyHull(hull);
  return handles;
}

export function createConveyorMeshBodies(): RenderBody[] {
  const bodies: RenderBody[] = [
    {
      kind: "box",
      size: [4, 1, 4],
      position: MESH_POS,
      rotation: [0, 0.70710678118, 0, 0.70710678118],
      type: BodyType.Static,
      color: 0x64748b,
    },
  ];
  for (let i = 0; i < 20; i++) {
    bodies.push({
      kind: "cylinder",
      radius: CYLINDER_RADIUS,
      height: CYLINDER_HEIGHT,
      yOffset: 0,
      position: conveyorCylinderPosition(i),
      localPosition: [0, 0.5 * CYLINDER_HEIGHT, 0],
      color: 0x60a5fa,
    });
  }
  return bodies;
}

export const conveyorMeshCamera: RenderSpec["camera"] = cameraFromSetView(65, 25, 28, [0, 1, 0]);

export const dumpSampleName = "Conveyor Mesh";
export const dumpSampleId = "shapes/conveyor-mesh";
export const dumpCppSampleName = "Conveyor Mesh";
export const dumpGroundSize = conveyorMeshGroundSize;
export const dumpBuildDynamicBodies = buildConveyorMeshDynamicBodies;
