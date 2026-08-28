import { BodyType, type BodyId, type Box3DRuntime, type HeightFieldHandle, type MeshHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const HF_POS: Vec3 = [-10, 0, -10];
const HF_SCALE: Vec3 = [0.5, 1, 0.5];
const BOX_HALF: Vec3 = [0.25, 1, 0.25];
const BOX_POS: Vec3 = [0, 3.5, 0];

export function buildSboxMoverDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): {
  handles: BodyId[];
  heightField: HeightFieldHandle;
  meshes: MeshHandle[];
} {
  const hfBody = world.createBody({ position: HF_POS });
  const heightField = world.createGridHeightField(40, 40, HF_SCALE, false);
  world.createHeightFieldShape(hfBody, heightField, {});
  const gridMesh = world.createGridMesh(40, 40, 0.5, 1, true);

  const platformBody = world.createBody();
  const platform = world.createPlatformMesh([0, 0.5, 0], 1, 2, 5);
  world.createMeshShape(platformBody, platform, { scale: [1, 1, 1] });

  const box = world.createBody({
    type: BodyType.Dynamic,
    position: BOX_POS,
    enableContactRecycling: false,
  });
  runtime.setBodyMotionLocks(box, { lockRotationX: true, lockRotationY: true, lockRotationZ: true });
  runtime.createHullShape(box, BOX_HALF, {});

  return { handles: [hfBody, platformBody, box], heightField, meshes: [gridMesh, platform] };
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildSboxMoverDynamicBodies(world, runtime).handles };
}

export function sboxMoverGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const sboxMoverBodies: RenderBody[] = [
  {
    kind: "box",
    size: [20, 0.05, 20],
    position: [0, 0, 0],
    type: BodyType.Static,
    color: 0x334155,
  },
  {
    kind: "box",
    size: [5, 1, 5],
    position: [0, 0.5, 0],
    type: BodyType.Static,
    color: 0x94a3b8,
  },
  {
    kind: "box",
    size: [0.5, 2, 0.5],
    position: BOX_POS,
    color: 0x38bdf8,
  },
];

export const sboxMoverCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 12, [0, 0, 0]);

export const dumpSampleName = "s&box mover";
export const dumpSampleId = "issues/sbox-mover";
export const dumpCppSampleName = "s&box mover";
export const dumpGroundSize = sboxMoverGroundSize;
