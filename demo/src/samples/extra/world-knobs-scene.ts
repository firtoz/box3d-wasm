import { BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const HALF: Vec3 = [0.5, 0.5, 0.5];

export function buildExtraWorldKnobsDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const handles: BodyId[] = [];
  for (let i = 0; i < 4; i++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [0, 0.5 + i * 1.05, 0] });
    runtime.createHullShape(body, HALF);
    handles.push(body);
  }
  return handles;
}

export function extraWorldKnobsGroundSize(): Vec3 {
  return [8, 0.5, 8];
}

export const extraWorldKnobsBodies: RenderBody[] = Array.from({ length: 4 }, (_, i) => ({
  kind: "box" as const,
  size: [1, 1, 1] as [number, number, number],
  position: [0, 0.5 + i * 1.05, 0] as [number, number, number],
  color: 0x60a5fa,
}));

export const extraWorldKnobsCamera: RenderSpec["camera"] = cameraFromSetView(20, 18, 14, [0, 2, 0]);

export const EXTRA_WORLD_KNOBS_HUD_FLOATS = 16;
