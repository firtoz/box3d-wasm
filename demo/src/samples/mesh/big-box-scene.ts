import { BodyType, type BodyHandle, type Box3DRuntime, type HullHandle, type MeshHandle, type PhysicsWorld, type ShapeHandle, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

export const BIG_BOX_DEFAULT_SCALE: Vec3 = [1, 1, 1];
export const BIG_BOX_SCALE_MIN = -2;
export const BIG_BOX_SCALE_MAX = 2;
export const BIG_BOX_EXTENT: Vec3 = [50, 1, 50];
export const BIG_BOX_CENTER: Vec3 = [0, -1, 0];

export type BigBoxShapeType = "sphere" | "capsule" | "box" | "cylinder";

export function buildBigBoxGround(
  world: PhysicsWorld,
  scale: Vec3 = BIG_BOX_DEFAULT_SCALE,
): { ground: BodyHandle; mesh: MeshHandle; shape: ShapeHandle } {
  const ground = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  const mesh = world.createBoxMesh(BIG_BOX_CENTER, BIG_BOX_EXTENT, true);
  const shape = world.createMeshShape(ground, mesh, { friction: 0.5, scale });
  return { ground, mesh, shape };
}

export function spawnBigBoxBody(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  shapeType: BigBoxShapeType = "cylinder",
  cylinderHull?: HullHandle | null,
): BodyHandle {
  const body = world.createBody({
    type: BodyType.Dynamic,
    position: [0.5, 0, 0],
  });

  switch (shapeType) {
    case "sphere":
      world.createSphereShape(body, [0, 0, 0], 0.5, { rollingResistance: 0.05 });
      break;
    case "capsule":
      world.createCapsuleShape(body, [0, 0, 1.276], [0, 0, 0.476], 0.15, { rollingResistance: 0.1 });
      break;
    case "box":
      world.createHullShape(body, [0.5, 0.5, 0.5], { rollingResistance: 0.05 });
      break;
    case "cylinder": {
      const hull = cylinderHull ?? runtime.createCylinder(0.3, 0.15, 0, 32);
      runtime.createShapeFromHull(body, hull, { rollingResistance: 0.05 });
      if (cylinderHull === undefined || cylinderHull === null) runtime.destroyHull(hull);
      break;
    }
  }

  return body;
}

export function buildBigBoxDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  return [spawnBigBoxBody(world, runtime, "cylinder")];
}

export function createBigBox(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, mesh } = buildBigBoxGround(world);
  return {
    world,
    handles: [ground, ...buildBigBoxDynamicBodies(world, runtime)],
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}

export function bigBoxGroundSize(): Vec3 {
  return [50, 1, 50];
}

export function bigBoxBodyFor(shapeType: BigBoxShapeType): RenderBody {
  const position: [number, number, number] = [0.5, 0, 0];
  const color = 0x60a5fa;
  switch (shapeType) {
    case "sphere":
      return { kind: "sphere", radius: 0.5, position, color };
    case "capsule":
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
        radius: 0.15,
        height: 0.3,
        segments: 32,
        yOffset: 0.15,
        position,
        color,
      };
  }
}

export const bigBoxBodies: RenderBody[] = [bigBoxBodyFor("cylinder")];

export const bigBoxCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 6, [0, 0, 0]);

export const dumpSampleName = "Big Box";
export const dumpSampleId = "mesh/big-box";
export const dumpCppSampleName = "Big Box";
export const dumpCreate = createBigBox;
