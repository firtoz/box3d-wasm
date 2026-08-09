import {BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const BOX_HALF = 0.5;
const FLOOR_HALF_XZ = 0.375;
const FLOOR_HALF_Y = 0.25;
const DROP_HEIGHT = 10;

export function buildRestitutionOvershootBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const floor = world.createBody({
    type: BodyType.Static,
    position: [0, -FLOOR_HALF_Y, 0],
  });
  runtime.createHullShape(floor, [FLOOR_HALF_XZ, FLOOR_HALF_Y, FLOOR_HALF_XZ]);

  const box = world.createBody({
    type: BodyType.Dynamic,
    position: [0, DROP_HEIGHT, 0],
  });
  runtime.createHullShape(box, [BOX_HALF, BOX_HALF, BOX_HALF], { restitution: 1 });

  return [floor, box];
}

export function createRestitutionOvershoot(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildRestitutionOvershootBodies(world, runtime) };
}

export function restitutionOvershootGroundSize(): Vec3 {
  // Visual-only placeholder; physics uses a custom small floor via dumpCreate / setupGround override.
  return [FLOOR_HALF_XZ, FLOOR_HALF_Y, FLOOR_HALF_XZ];
}

export const restitutionOvershootBodies: RenderBody[] = [
  {
    kind: "box",
    size: [2 * FLOOR_HALF_XZ, 2 * FLOOR_HALF_Y, 2 * FLOOR_HALF_XZ],
    position: [0, -FLOOR_HALF_Y, 0],
    color: 0x94a3b8,
    type: BodyType.Static,
  },
  {
    kind: "box",
    size: [2 * BOX_HALF, 2 * BOX_HALF, 2 * BOX_HALF],
    position: [0, DROP_HEIGHT, 0],
    color: 0xf59e0b,
  },
];

export const restitutionOvershootCamera: RenderSpec["camera"] = cameraFromSetView(
  20,
  0,
  28,
  [0, DROP_HEIGHT + BOX_HALF, 0],
);

export const dumpSampleName = "Restitution Overshoot";
export const dumpSampleId = "issues/restitution-overshoot";
export const dumpCppSampleName = "Restitution Overshoot";
export const dumpCreate = createRestitutionOvershoot;
