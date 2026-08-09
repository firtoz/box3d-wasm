import {B3_AXIS_Y, B3_PI, BodyType, type BodyId, type Box3DRuntime, type MeshHandle, type PhysicsWorld, type ShapeHandle, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Mul } from "../f32";

export const MESH_BOX_DEFAULT_SCALE: Vec3 = [1, 1, 1];
export const MESH_BOX_SCALE_MIN = -2;
export const MESH_BOX_SCALE_MAX = 2;
export const MESH_BOX_EXTENT: Vec3 = [1, 1, 1];
export const MESH_BOX_CENTER: Vec3 = [0, 1, 0];
export const MESH_BOX_PLATFORM_POS: Vec3 = [0, -1, 0];

export type MeshBoxShapeType = "sphere" | "capsule" | "box" | "cylinder";

export function meshBoxPlatformRotation(runtime: Box3DRuntime): [number, number, number, number] {
  return runtime.makeQuatFromAxisAngle(B3_AXIS_Y, f32Mul(0.25, B3_PI));
}

export function buildMeshBoxGroundBox(world: PhysicsWorld, runtime: Box3DRuntime): BodyId {
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, [20, 1, 20]);
  return ground;
}

export function buildMeshBoxPlatform(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  scale: Vec3 = MESH_BOX_DEFAULT_SCALE,
): { platform: BodyId; mesh: MeshHandle; shape: ShapeHandle } {
  const platform = world.createBody({
    type: BodyType.Static,
    position: MESH_BOX_PLATFORM_POS,
    rotation: meshBoxPlatformRotation(runtime),
  });
  const mesh = world.createBoxMesh(MESH_BOX_CENTER, MESH_BOX_EXTENT, true);
  const shape = world.createMeshShape(platform, mesh, { scale });
  return { platform, mesh, shape };
}

export function spawnMeshBoxBody(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  shapeType: MeshBoxShapeType = "box",
): BodyId {
  const position: Vec3 = [0, f32(1.5), 0];
  if (shapeType === "cylinder") {
    position[1] = f32(position[1] - 0.5);
  }
  const body = world.createBody({ type: BodyType.Dynamic, position });

  switch (shapeType) {
    case "sphere":
      world.createSphereShape(body, [0, 0, 0], 0.5);
      break;
    case "capsule":
      world.createCapsuleShape(body, [-0.5, 0, 0], [0.5, 0, 0], 0.1);
      break;
    case "box":
      world.createHullShape(body, [0.5, 0.5, 0.5]);
      break;
    case "cylinder": {
      const cylinder = runtime.createCylinder(1, 0.75, 0, 8);
      runtime.createShapeFromHull(body, cylinder);
      runtime.destroyHull(cylinder);
      break;
    }
  }

  return body;
}

export function buildMeshBoxDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  return [spawnMeshBoxBody(world, runtime, "box")];
}

export function createMeshBox(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = buildMeshBoxGroundBox(world, runtime);
  const { platform, mesh } = buildMeshBoxPlatform(world, runtime);
  return {
    world,
    handles: [ground, platform, ...buildMeshBoxDynamicBodies(world, runtime)],
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}

export function meshBoxGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function meshBoxBodyFor(shapeType: MeshBoxShapeType): RenderBody {
  const position: [number, number, number] = [0, 1.5, 0];
  const color = 0x60a5fa;
  switch (shapeType) {
    case "sphere":
      return { kind: "sphere", radius: 0.5, position, color };
    case "capsule":
      return { kind: "capsule", radius: 0.1, length: 1, axis: "x", position, color };
    case "box":
      return { kind: "box", size: [1, 1, 1], position, color };
    case "cylinder":
      return {
        kind: "cylinder",
        radius: 0.75,
        height: 1,
        segments: 8,
        yOffset: 0.5,
        position: [0, 1, 0],
        color,
      };
  }
}

export const meshBoxBodies: RenderBody[] = [meshBoxBodyFor("box")];

export const meshBoxCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 6, [0, 0, 0]);

export const dumpSampleName = "Box";
export const dumpSampleId = "mesh/box";
/** Slash form — RegisterSample name "Box" also exists under Ragdoll. */
export const dumpCppSampleName = "Mesh/Box";
export const dumpCreate = createMeshBox;
