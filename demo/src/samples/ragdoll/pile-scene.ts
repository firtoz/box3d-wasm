import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { collectHumanBoneHandles, ragdollRenderBodies } from "./ragdoll-scene-shared";
import { f32 } from "../f32";

const COUNT = 20;

export function buildRagdollPileDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  for (let i = 0; i < COUNT; i++) {
    const human = world.createHuman([f32(0.1 * i), f32(2 + 0.5 * i), f32(-0.1 * i)], {
      frictionTorque: 10,
      hertz: 0.5,
      dampingRatio: 0.7,
      groupIndex: i,
      colorize: false,
    });
    handles.push(...collectHumanBoneHandles(runtime, human));
  }
  return handles;
}

export function buildRagdollPileGround(world: PhysicsWorld): number {
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  const mesh = world.createGridMesh(20, 20, 1, 1, true);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });
  return ground;
}

export function ragdollPileGroundSize(): Vec3 { return [10, 0.5, 10]; }

export function createRagdollPileBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < COUNT; i++) {
    bodies.push(...ragdollRenderBodies([0.1 * i, 2 + 0.5 * i, -0.1 * i]));
  }
  return bodies;
}

export const ragdollPileCamera: RenderSpec["camera"] = { position: [180, 30, 20], target: [0, 2, 0] };

export const dumpSampleName = "Pile";
export const dumpSampleId = "ragdoll/pile";
export const dumpCppSampleName = "Pile";
export const dumpGroundSize = ragdollPileGroundSize;

export function createRagdollPile(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = buildRagdollPileGround(world);
  return { world, handles: [ground, ...buildRagdollPileDynamicBodies(world, runtime)] };
}

export const dumpCreate = createRagdollPile;
