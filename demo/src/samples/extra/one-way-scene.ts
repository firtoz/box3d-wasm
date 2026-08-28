import { B3W_ONE_WAY_USER_BIT, BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const PLATFORM_HALF: Vec3 = [2, 0.1, 2];
const CUBE_HALF: Vec3 = [0.4, 0.4, 0.4];

export const extraOneWayHandleIndex = { drop: 0, launch: 1 } as const;

export function buildExtraOneWayDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  world.setPreSolveOneWay(true, 0.5);
  const platform = world.createBody({ type: BodyType.Static, position: [0, 3, 0] });
  runtime.createHullShape(platform, PLATFORM_HALF, {
    enablePreSolveEvents: true,
    userData: B3W_ONE_WAY_USER_BIT,
  });
  const drop = world.createBody({ type: BodyType.Dynamic, position: [0, 6, 0] });
  runtime.createHullShape(drop, CUBE_HALF, { enablePreSolveEvents: true });
  const launch = world.createBody({ type: BodyType.Dynamic, position: [0, 0.5, 0] });
  runtime.createHullShape(launch, CUBE_HALF, { enablePreSolveEvents: true });
  return [drop, launch, platform];
}

export function extraOneWayGroundSize(): Vec3 {
  return [8, 0.5, 8];
}

export const extraOneWayBodies: RenderBody[] = [
  { kind: "box", size: [0.8, 0.8, 0.8], position: [0, 6, 0], color: 0x60a5fa },
  { kind: "box", size: [0.8, 0.8, 0.8], position: [0, 0.5, 0], color: 0xf97316 },
  {
    kind: "box",
    size: [4, 0.2, 4],
    position: [0, 3, 0],
    type: BodyType.Static,
    color: 0x34d399,
  },
];

export const extraOneWayCamera: RenderSpec["camera"] = cameraFromSetView(0, 22, 16, [0, 3, 0]);

export const EXTRA_ONE_WAY_HUD_INTS = 2;
