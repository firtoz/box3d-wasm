import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderSpec } from "../generic-host";
import { createBenchmarkTileMeshes, spawnHumanGroup } from "../benchmark/benchmark-tile-shared";
import { f32 } from "../f32";

const GRID_COUNT = 2;
const GROUP_SIZE = 2;
const GRID_SIZE = f32(15);
const SPAWN_Y = 15;

export function buildFallingRagdollsDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const { gridSize, gridMesh, torusMesh } = createBenchmarkTileMeshes(world);
  const handles: number[] = [];
  const span = f32(gridSize * GRID_COUNT);
  let x = f32(f32(-0.5) * span + f32(0.5) * gridSize);
  for (let i = 0; i < GRID_COUNT; i++) {
    let z = f32(f32(-0.5) * span + f32(0.5) * gridSize);
    for (let j = 0; j < GRID_COUNT; j++) {
      const body = world.createBody({ type: BodyType.Static, position: [x, 0, z] });
      world.createMeshShape(body, gridMesh, { scale: [1, 1, 1] });
      world.createMeshShape(body, torusMesh, { scale: [1, 1, 1] });
      handles.push(body);
      spawnHumanGroup(world, runtime, handles, GRID_COUNT, gridSize, GROUP_SIZE, i, j, SPAWN_Y, 5, 1, 0.7);
      z = f32(z + gridSize);
    }
    x = f32(x + gridSize);
  }
  return handles;
}

export function fallingRagdollsGroundSize(): Vec3 { return [30, 1, 30]; }

export const fallingRagdollsCamera: RenderSpec["camera"] = { position: [45, 30, 40], target: [0, 0, 0] };

export const dumpSampleName = "Falling Ragdolls";
export const dumpSampleId = "determinism/falling-ragdolls";
export const dumpCppSampleName = "Falling Ragdolls";
export const dumpGroundSize = fallingRagdollsGroundSize;
export const dumpBuildDynamicBodies = buildFallingRagdollsDynamicBodies;

export function createFallingRagdolls(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildFallingRagdollsDynamicBodies(world, runtime) };
}

export const dumpCreate = createFallingRagdolls;
