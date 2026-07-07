import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildSpinningStickDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const stickOmega: Vec3 = [41.000092, -42.434464, -37.792595];
  const handles: number[] = [];

  const wall = world.createBody({ type: BodyType.Static, position: [0, 0.5, 0] });
  runtime.createHullShape(wall, [0.125, 0.5, 10], {});
  handles.push(wall);

  const stick = world.createBody({
    type: BodyType.Dynamic,
    position: [0, 20, 0.5],
    linearVelocity: [0, Math.fround(-100), 0],
    angularVelocity: stickOmega,
  });
  runtime.createHullShape(stick, [2, 0.1, 0.1], { rollingResistance: 0.1 });
  handles.push(stick);

  return handles;
}

export function spinningStickGroundSize(): Vec3 { return [10, 1, 10]; }

export function createSpinningStickBodies(): RenderBody[] {
  return [
    { kind: "box", size: [0.25, 1, 20], position: [0, 0.5, 0], color: 0x94a3b8, type: BodyType.Static },
    { kind: "box", size: [4, 0.2, 0.2], position: [0, 20, 0.5], color: 0xf59e0b },
  ];
}

export const spinningStickCamera: RenderSpec["camera"] = { position: [45, 25, 20], target: [0, 2, 0] };

export const dumpSampleName = "Spinning Stick";
export const dumpSampleId = "continuous/spinning-stick";
export const dumpCppSampleName = "Spinning Stick";
export const dumpGroundSize = spinningStickGroundSize;
export const dumpBuildDynamicBodies = buildSpinningStickDynamicBodies;
