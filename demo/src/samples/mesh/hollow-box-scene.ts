import {
  BodyType,
  type BodyHandle,
  type Box3DRuntime,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const CYLINDER_HEIGHT = 1.0;
const CYLINDER_RADIUS = 0.25;
const CYLINDER_SIDES = 8;

const CYLINDER_POSITIONS: readonly Vec3[] = [
  [0, -10.2, 0],
  [0, 9.2, 0],
  [-9.8, 0, 0],
  [9.8, 0, 0],
  [0, 0, -9.8],
  [0, 0, 9.8],
];

const CAPSULE_POSITIONS: readonly Vec3[] = [
  [0, -10.2, 2],
  [0, 9.2, 2],
  [0, -9.9, 4],
  [0, 8.9, 4],
  [-9.8, 2, 0],
  [9.8, 2, 0],
  [0, 2, -9.8],
  [0, 2, 9.8],
];

const CAPSULE_A: Vec3 = [0, -0.5, 0];
const CAPSULE_B: Vec3 = [0, 0.5, 0];
const CAPSULE_RADIUS = 0.25;

export function buildHollowBoxGround(world: PhysicsWorld): { ground: BodyHandle; mesh: MeshHandle } {
  const ground = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  const mesh = world.createHollowBoxMesh([0, 0, 0], [10, 10, 10]);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });
  return { ground, mesh };
}

export function buildHollowBoxDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const handles: BodyHandle[] = [];
  const cylinderHull = runtime.createCylinder(CYLINDER_HEIGHT, CYLINDER_RADIUS, 0, CYLINDER_SIDES);

  for (const position of CYLINDER_POSITIONS) {
    const body = world.createBody({
      type: BodyType.Dynamic,
      position,
      gravityScale: 0,
      enableSleep: false,
    });
    runtime.createShapeFromHull(body, cylinderHull);
    handles.push(body);
  }

  runtime.destroyHull(cylinderHull);

  for (const position of CAPSULE_POSITIONS) {
    const body = world.createBody({
      type: BodyType.Dynamic,
      position,
      gravityScale: 0,
      enableSleep: false,
    });
    world.createCapsuleShape(body, CAPSULE_A, CAPSULE_B, CAPSULE_RADIUS);
    handles.push(body);
  }

  return handles;
}

export function hollowBoxGroundSize(): Vec3 {
  return [10, 10, 10];
}

export function createHollowBoxBodies(): RenderBody[] {
  const cylinderColor = 0x60a5fa;
  const capsuleColor = 0x34d399;
  const bodies: RenderBody[] = [];

  for (const position of CYLINDER_POSITIONS) {
    bodies.push({
      kind: "cylinder",
      radius: CYLINDER_RADIUS,
      height: CYLINDER_HEIGHT,
      segments: CYLINDER_SIDES,
      position,
      color: cylinderColor,
    });
  }

  for (const position of CAPSULE_POSITIONS) {
    bodies.push({
      kind: "capsule",
      radius: CAPSULE_RADIUS,
      length: 1,
      axis: "y",
      position,
      color: capsuleColor,
    });
  }

  return bodies;
}

export const hollowBoxCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 30, [0, 0, 0]);

export const dumpSampleName = "Hollow Box";
export const dumpSampleId = "mesh/hollow-box";
export const dumpCppSampleName = "Mesh/Hollow Box";

export function createHollowBox(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, mesh } = buildHollowBoxGround(world);
  return {
    world,
    handles: [ground, ...buildHollowBoxDynamicBodies(world, runtime)],
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}

export const dumpCreate = createHollowBox;
