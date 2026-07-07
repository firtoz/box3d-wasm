import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildIsFastDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const half: Vec3 = [0.5, 10, 0.5];

  const configs: { position: Vec3; angularVelocity: Vec3 }[] = [
    { position: [-12, 20, 0], angularVelocity: [0, 0, 4] },
    { position: [0, 20, 0], angularVelocity: [0, 4, 0] },
    { position: [12, 20, 0], angularVelocity: [4, 0, 0] },
  ];

  for (const cfg of configs) {
    const body = world.createBody({
      type: BodyType.Dynamic,
      position: cfg.position,
      gravityScale: 0,
      angularVelocity: cfg.angularVelocity,
    });
    runtime.createHullShape(body, half, {});
    handles.push(body);
  }

  return handles;
}

export function isFastGroundSize(): Vec3 { return [40, 1, 40]; }

export function createIsFastBodies(): RenderBody[] {
  return [
    { kind: "box", size: [1, 20, 1], position: [-12, 20, 0], color: 0x60a5fa },
    { kind: "box", size: [1, 20, 1], position: [0, 20, 0], color: 0x34d399 },
    { kind: "box", size: [1, 20, 1], position: [12, 20, 0], color: 0xf59e0b },
  ];
}

export const isFastCamera: RenderSpec["camera"] = { position: [0, 15, 50], target: [0, 15, 0] };

export const dumpSampleName = "Is Fast";
export const dumpSampleId = "continuous/is-fast";
export const dumpCppSampleName = "Is Fast";
export const dumpGroundSize = isFastGroundSize;
export const dumpBuildDynamicBodies = buildIsFastDynamicBodies;
