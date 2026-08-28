import { BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const HALF: Vec3 = [0.5, 0.5, 0.5];

export const extraForcesHandleIndex = { hover: 0, impulse: 1, spin: 2 } as const;

export function buildExtraForcesDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const hover = world.createBody({ type: BodyType.Dynamic, position: [-3, 1.5, 0] });
  runtime.createHullShape(hover, HALF);
  world.setBodyDamping(hover, 0.2, 0.2);
  const impulse = world.createBody({ type: BodyType.Dynamic, position: [0, 0.5, 0] });
  runtime.createHullShape(impulse, HALF);
  const spin = world.createBody({ type: BodyType.Dynamic, position: [3, 0.5, 0] });
  runtime.createHullShape(spin, HALF);
  return [hover, impulse, spin];
}

export function extraForcesGroundSize(): Vec3 {
  return [10, 0.5, 10];
}

export const extraForcesBodies: RenderBody[] = [
  { kind: "box", size: [1, 1, 1], position: [-3, 1.5, 0], color: 0x22c55e },
  { kind: "box", size: [1, 1, 1], position: [0, 0.5, 0], color: 0x60a5fa },
  { kind: "box", size: [1, 1, 1], position: [3, 0.5, 0], color: 0xf97316 },
];

export const extraForcesCamera: RenderSpec["camera"] = cameraFromSetView(0, 20, 16, [0, 1, 0]);
