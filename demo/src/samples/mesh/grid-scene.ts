import { BodyType, type BodyHandle, type Box3DRuntime, type MeshHandle, type PhysicsWorld, type ShapeHandle, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

export const MESH_GRID_DEFAULT_SCALE: Vec3 = [2, 2, 2];
/** Upstream ImGui sliders use [-2, 2]. */
export const MESH_GRID_SCALE_MIN = -2;
export const MESH_GRID_SCALE_MAX = 2;
export const MESH_GRID_CELL_COUNT = 20;
export const MESH_GRID_CELL_WIDTH = 1;

export type MeshGridShapeType = "sphere" | "capsule" | "box" | "cylinder";

export function buildMeshGridGround(
  world: PhysicsWorld,
  scale: Vec3 = MESH_GRID_DEFAULT_SCALE,
): { ground: BodyHandle; mesh: MeshHandle; shape: ShapeHandle } {
  const ground = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  const mesh = world.createGridMesh(MESH_GRID_CELL_COUNT, MESH_GRID_CELL_COUNT, MESH_GRID_CELL_WIDTH, 0, true);
  const shape = world.createMeshShape(ground, mesh, { scale });
  return { ground, mesh, shape };
}

export function spawnMeshGridBody(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  shapeType: MeshGridShapeType = "cylinder",
): BodyHandle {
  const body = world.createBody({
    type: BodyType.Dynamic,
    position: [0.1, 1.0, -0.1],
    angularDamping: shapeType === "cylinder" ? 0.1 : 0,
  });

  switch (shapeType) {
    case "sphere":
      world.createSphereShape(body, [0, 0, 0], 0.5, { rollingResistance: 0.05 });
      break;
    case "capsule":
      world.createCapsuleShape(body, [0, 0, 1.276], [0, 0, 0.476], 0.15, { rollingResistance: 0.05 });
      break;
    case "box":
      world.createHullShape(body, [0.5, 0.5, 0.5]);
      break;
    case "cylinder": {
      const cylinder = runtime.createCylinder(1, 0.25, 0, 15);
      runtime.createShapeFromHull(body, cylinder, { rollingResistance: 0.02 });
      runtime.destroyHull(cylinder);
      break;
    }
  }

  return body;
}

export function buildMeshGridDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  return [spawnMeshGridBody(world, runtime, "cylinder")];
}

export function createMeshGrid(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyHandle[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground } = buildMeshGridGround(world);
  return { world, handles: [ground, ...buildMeshGridDynamicBodies(world, runtime)] };
}

export function meshGridGroundSize(): Vec3 {
  const half = 0.5 * MESH_GRID_CELL_COUNT * MESH_GRID_CELL_WIDTH * MESH_GRID_DEFAULT_SCALE[0];
  return [half, 0.5, half];
}

export function meshGridBodyFor(shapeType: MeshGridShapeType): RenderBody {
  const position: [number, number, number] = [0.1, 1.0, -0.1];
  const color = 0x60a5fa;
  switch (shapeType) {
    case "sphere":
      return { kind: "sphere", radius: 0.5, position, color };
    case "capsule":
      // Centers at z=1.276 and z=0.476 → length 0.8, midpoint z=0.876.
      return {
        kind: "capsule",
        radius: 0.15,
        length: 0.8,
        axis: "z",
        localPosition: [0, 0, 0.876],
        position,
        color,
      };
    case "box":
      return { kind: "box", size: [1, 1, 1], position, color };
    case "cylinder":
      return {
        kind: "cylinder",
        radius: 0.25,
        height: 1,
        segments: 15,
        yOffset: 0.5,
        position,
        color,
      };
  }
}

export const meshGridBodies: RenderBody[] = [meshGridBodyFor("cylinder")];

export const meshGridCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 6, [0, 0, 0]);

export const dumpSampleName = "Grid";
export const dumpSampleId = "mesh/grid";
export const dumpCppSampleName = "Grid";
export const dumpCreate = createMeshGrid;
