import {BodyType, type Box3DRuntime, type HumanHandle, type PhysicsWorld, type Vec3, type BodyId} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { collectHumanBoneHandles, ragdollRenderBodies } from "./ragdoll-scene-shared";

export function spawnRagdollBox(world: PhysicsWorld, runtime: Box3DRuntime, options: { frictionTorque: number; hertz: number; dampingRatio: number }): { human: HumanHandle; handles: BodyId[] } {
  const human = world.createHuman([0, 2, 0], {
    frictionTorque: options.frictionTorque,
    hertz: options.hertz,
    dampingRatio: options.dampingRatio,
    groupIndex: 1,
    colorize: false,
  });
  return { human, handles: collectHumanBoneHandles(runtime, human) };
}

export function buildRagdollBoxDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  return spawnRagdollBox(world, runtime, { frictionTorque: 5, hertz: 1, dampingRatio: 0.7 }).handles;
}

export function ragdollBoxGroundSize(): Vec3 { return [20, 1, 20]; }

export function createRagdollBoxBodies(): RenderBody[] {
  return ragdollRenderBodies([0, 2, 0]);
}

export const ragdollBoxCamera: RenderSpec["camera"] = { position: [45, 30, 6], target: [0, 1, 0] };

export const dumpSampleName = "Box";
export const dumpSampleId = "ragdoll/box";
export const dumpCppSampleName = "Ragdoll/Box";
export const dumpGroundSize = ragdollBoxGroundSize;
export const dumpBuildDynamicBodies = buildRagdollBoxDynamicBodies;

export function createRagdollBox(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, ragdollBoxGroundSize(), {});
  return { world, handles: [ground, ...buildRagdollBoxDynamicBodies(world, runtime)] };
}

export const dumpCreate = createRagdollBox;
