import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { collectHumanBoneHandles, ragdollRenderBodies } from "../ragdoll/ragdoll-scene-shared";
import { f32, f32Add, f32Mul, f32Sub } from "../f32";

const OFFSET_F32 = 1_000_000;
const COUNT = 20;
const HALF_COUNT_F32 = f32Mul(0.5, COUNT);
const STEP_F32 = f32(0.15);

function farRagdollPosition(i: number): Vec3 {
  // Match upstream float32 offset arithmetic.
  const iF32 = f32(i);
  const x = f32Add(f32Mul(STEP_F32, f32Sub(iF32, HALF_COUNT_F32)), OFFSET_F32);
  const y = f32Add(2, f32Mul(0.25, iF32));
  const z = f32Mul(STEP_F32, f32Sub(HALF_COUNT_F32, iF32));
  return [x, y, z];
}

export function buildFarRagdollsDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];

  for (let i = 0; i < COUNT; i++) {
    const human = world.createHuman(farRagdollPosition(i), {
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

export function buildFarRagdollsGround(world: PhysicsWorld): number {
  const ground = world.createBody({ type: BodyType.Static, position: [OFFSET_F32, -1, 0] });
  const mesh = world.createGridMesh(20, 20, 1, 1, true);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });
  return ground;
}

export function createFarRagdolls(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = buildFarRagdollsGround(world);
  return { world, handles: [ground, ...buildFarRagdollsDynamicBodies(world, runtime)] };
}

export function farRagdollsGroundSize(): Vec3 {
  return [10, 0.5, 10];
}

export const farRagdollsGroundPosition: Vec3 = [OFFSET_F32, -1, 0];

export const farRagdollsBodies: RenderBody[] = (() => {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < COUNT; i++) {
    bodies.push(...ragdollRenderBodies(farRagdollPosition(i)));
  }
  return bodies;
})();

export const farRagdollsCamera: RenderSpec["camera"] = { position: [1_000_180, 30, 20], target: [1_000_000, 0, 0] };

export const dumpSampleName = "Far Ragdolls";
export const dumpSampleId = "world/far-ragdolls";
export const dumpCppSampleName = "Far Ragdolls";
export const dumpCreate = createFarRagdolls;
