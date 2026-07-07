import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildBounceHouseDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createTransformedHullShape(ground, [0.1, 5, 10], { position: [10, 5, 0] });
  runtime.createTransformedHullShape(ground, [0.1, 5, 10], { position: [-10, 5, 0] });
  runtime.createTransformedHullShape(ground, [10, 5, 0.1], { position: [0, 5, -10] });
  runtime.createTransformedHullShape(ground, [10, 5, 0.1], { position: [0, 5, 10] });
  handles.push(ground);

  const sphere = world.createBody({
    type: BodyType.Dynamic, position: [-8, 4, 0], linearVelocity: [120, 0, 120], gravityScale: 0,
  });
  runtime.createSphereShape(sphere, [0, 0, 0], 0.5, { friction: 0, restitution: 1 });
  handles.push(sphere);

  return handles;
}

export function bounceHouseGroundSize(): Vec3 { return [10, 1, 10]; }

export function createBounceHouseBodies(): RenderBody[] {
  return [
    { kind: "compound", position: [0, -1, 0], type: BodyType.Static, parts: [
      { kind: "box", size: [0.2, 10, 20], position: [10, 5, 0], color: 0x94a3b8 },
      { kind: "box", size: [0.2, 10, 20], position: [-10, 5, 0], color: 0x94a3b8 },
      { kind: "box", size: [20, 10, 0.2], position: [0, 5, -10], color: 0x94a3b8 },
      { kind: "box", size: [20, 10, 0.2], position: [0, 5, 10], color: 0x94a3b8 },
    ] },
    { kind: "sphere", radius: 0.5, position: [-8, 4, 0], color: 0xf59e0b },
  ];
}

export const bounceHouseCamera: RenderSpec["camera"] = { position: [45, 45, 50], target: [0, 0, 0] };

export const dumpSampleName = "Bounce House";
export const dumpSampleId = "continuous/bounce-house";
export const dumpCppSampleName = "Bounce House";
export const dumpGroundSize = bounceHouseGroundSize;
export const dumpBuildDynamicBodies = buildBounceHouseDynamicBodies;
