import {
  B3_AXIS_X,
  B3_AXIS_Z,
  B3_PI,
  BodyType,
  type BodyHandle,
  type Box3DRuntime,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const BOX_HALF = 0.6;
const SPHERE_RADIUS = 0.9;
const CAPSULE_A: Vec3 = [-0.5, 0, 0];
const CAPSULE_B: Vec3 = [0.5, 0, 0];
const CAPSULE_RADIUS = 0.7;
const TORUS_RADIAL = 10;
const TORUS_TUBULAR = 12;
const TORUS_RADIUS = 0.65;
const TORUS_THICKNESS = 0.35;

const ROW_X = [-6, -2, 2, 6] as const;

export function buildShapeCastScene(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  torusMesh: MeshHandle,
): BodyHandle[] {
  const handles: BodyHandle[] = [];

  for (let index = 0; index < 3; index++) {
    const y = 3 + 2 * index;

    const sphereBody = world.createBody({
      type: BodyType.Static,
      position: [ROW_X[0], y, 0],
      rotation: runtime.makeQuatFromAxisAngle(B3_AXIS_X, 0.5 * B3_PI),
    });
    runtime.createSphereShape(sphereBody, [0, 0, 0], SPHERE_RADIUS);
    handles.push(sphereBody);

    const capsuleBody = world.createBody({
      type: BodyType.Static,
      position: [ROW_X[1], y, 0],
      rotation: runtime.makeQuatFromAxisAngle(B3_AXIS_Z, 0.25 * B3_PI),
    });
    world.createCapsuleShape(capsuleBody, CAPSULE_A, CAPSULE_B, CAPSULE_RADIUS);
    handles.push(capsuleBody);

    const hullBody = world.createBody({
      type: BodyType.Static,
      position: [ROW_X[2], y, 0],
      rotation: runtime.makeQuatFromAxisAngle(B3_AXIS_Z, 0.25 * B3_PI),
    });
    runtime.createHullShape(hullBody, [BOX_HALF, BOX_HALF, BOX_HALF]);
    handles.push(hullBody);

    const meshBody = world.createBody({
      type: BodyType.Static,
      position: [ROW_X[3], y, 0],
      rotation: runtime.makeQuatFromAxisAngle(B3_AXIS_X, 0.5 * B3_PI),
    });
    world.createMeshShape(meshBody, torusMesh, { scale: [1, 1, 1] });
    handles.push(meshBody);
  }

  return handles;
}

export function buildShapeCastDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const torusMesh = world.createTorusMesh(TORUS_RADIAL, TORUS_TUBULAR, TORUS_RADIUS, TORUS_THICKNESS);
  return buildShapeCastScene(world, runtime, torusMesh);
}

export function shapeCastGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createShapeCastBodies(runtime: Box3DRuntime): RenderBody[] {
  const bodies: RenderBody[] = [];
  const sphereRotation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, 0.5 * B3_PI);
  const zRotation = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, 0.25 * B3_PI);

  for (let index = 0; index < 3; index++) {
    const y = 3 + 2 * index;

    bodies.push({
      kind: "sphere",
      radius: SPHERE_RADIUS,
      position: [ROW_X[0], y, 0],
      rotation: sphereRotation,
      type: BodyType.Static,
      color: 0x60a5fa,
    });

    bodies.push({
      kind: "capsule",
      radius: CAPSULE_RADIUS,
      length: 1,
      axis: "x",
      position: [ROW_X[1], y, 0],
      rotation: zRotation,
      type: BodyType.Static,
      color: 0x34d399,
    });

    bodies.push({
      kind: "box",
      size: [2 * BOX_HALF, 2 * BOX_HALF, 2 * BOX_HALF],
      position: [ROW_X[2], y, 0],
      rotation: zRotation,
      type: BodyType.Static,
      color: 0xf97316,
    });

    bodies.push({
      kind: "torus",
      radius: TORUS_RADIUS,
      tube: TORUS_THICKNESS,
      radialSegments: TORUS_RADIAL,
      tubularSegments: TORUS_TUBULAR,
      position: [ROW_X[3], y, 0],
      rotation: sphereRotation,
      type: BodyType.Static,
      color: 0xa78bfa,
    });
  }

  return bodies;
}

export const shapeCastCamera: RenderSpec["camera"] = cameraFromSetView(120, 30, 20, [0, 1.5, 0]);

export const dumpSampleName = "Shape Cast";
export const dumpSampleId = "collision/shape-cast";
export const dumpCppSampleName = "Shape Cast";
export const dumpGroundSize = shapeCastGroundSize;

export function createShapeCast(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
  state: { mesh: MeshHandle };
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, 0, 0], workerCount: 1 });
  const mesh = world.createTorusMesh(TORUS_RADIAL, TORUS_TUBULAR, TORUS_RADIUS, TORUS_THICKNESS);
  return {
    world,
    handles: buildShapeCastScene(world, runtime, mesh),
    state: { mesh },
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}

export const dumpCreate = createShapeCast;
