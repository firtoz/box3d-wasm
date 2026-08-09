import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const GROUND_HALF: Vec3 = [50, 0.1, 50];
const CAPSULE_CENTER1: Vec3 = [0, -0.5, 0];
const CAPSULE_CENTER2: Vec3 = [0, 0.5, 0];
const CAPSULE_RADIUS = 0.3;
const CAPSULE_POS: Vec3 = [0, 4, 10];

/**
 * Ground + player capsule only. Upstream also loads `building.obj` as a static mesh at (0, 0.1, 0);
 * that mesh is not embedded here — dump match needs a generated `building-mesh-data` module once
 * mesh assets can ship in the browser worker.
 */
export function buildCapsuleMeshDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const ground = world.createBody({ position: [0, 0, 0] });
  runtime.createHullShape(ground, GROUND_HALF, {});

  const capsule = world.createBody({
    type: BodyType.Dynamic,
    position: CAPSULE_POS,
    enableSleep: false,
    enableContactRecycling: false,
  });
  runtime.createCapsuleShape(capsule, CAPSULE_CENTER1, CAPSULE_CENTER2, CAPSULE_RADIUS, { friction: 0.3 });

  return [ground, capsule];
}

export function capsuleMeshGroundSize(): Vec3 {
  return GROUND_HALF;
}

export const capsuleMeshBodies: RenderBody[] = [
  {
    kind: "box",
    size: [100, 0.2, 100],
    position: [0, 0, 0],
    type: BodyType.Static,
    color: 0x64748b,
  },
  {
    kind: "capsule",
    axis: "y",
    radius: CAPSULE_RADIUS,
    length: 1,
    position: CAPSULE_POS,
    color: 0xd946ef,
  },
];

export const capsuleMeshCamera: RenderSpec["camera"] = cameraFromSetView(20, 10, 30, [0, 2, 0]);

export const dumpSampleName = "Capsule Mesh";
export const dumpSampleId = "issues/capsule-mesh";
export const dumpCppSampleName = "Capsule Mesh";
export const dumpGroundSize = capsuleMeshGroundSize;
export const dumpBuildDynamicBodies = buildCapsuleMeshDynamicBodies;

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildCapsuleMeshDynamicBodies(world, runtime) };
}
