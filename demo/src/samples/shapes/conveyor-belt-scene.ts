import { BodyType, quatFromAxisAngle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const tiltAngle = 0.2;
const tiltQuat = quatFromAxisAngle([0, 1, 0], tiltAngle);

export function buildConveyorBeltDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const rotation = runtime.makeQuatFromAxisAngle([0, 1, 0], tiltAngle);

  const platform = world.createBody({ type: BodyType.Static, position: [-5, 5, 0], rotation });
  runtime.createHullShape(platform, [10, 0.25, 2], { friction: 0.8, tangentVelocity: [2, 0, 0] });
  handles.push(platform);

  for (let i = 0; i < 5; i++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [-10 + 2 * i, 7, 0] });
    runtime.createHullShape(body, [0.5, 0.5, 0.5], {});
    handles.push(body);
  }

  return handles;
}

export function conveyorBeltGroundSize(): Vec3 { return [20, 1, 20]; }

export function createConveyorBeltBodies(): RenderBody[] {
  const bodies: RenderBody[] = [
    { kind: "box", size: [20, 0.5, 4], position: [-5, 5, 0], rotation: tiltQuat, color: 0x94a3b8, type: BodyType.Static },
  ];
  for (let i = 0; i < 5; i++) {
    bodies.push({ kind: "box", size: [1, 1, 1], position: [-10 + 2 * i, 7, 0], color: 0x60a5fa });
  }
  return bodies;
}

export const conveyorBeltCamera: RenderSpec["camera"] = { position: [0, 25, 40], target: [0, 1, 0] };

export const dumpSampleName = "Conveyor Belt";
export const dumpSampleId = "shapes/conveyor-belt";
export const dumpCppSampleName = "Conveyor Belt";
export const dumpGroundSize = conveyorBeltGroundSize;
export const dumpBuildDynamicBodies = buildConveyorBeltDynamicBodies;
