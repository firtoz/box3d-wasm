import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3, type WorldCapacity } from "box3d-wasm";
import type { RenderSpec } from "../generic-host";
import { Box3DRng } from "../box3d-rng";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "../f32";

/** Match upstream release Destruction (`m_isDebug` false). */
export const DESTRUCTION_GRID_COUNT = 20;
export const DESTRUCTION_EXTENT = f32(2.5);
export const DESTRUCTION_RANDOM_RANGE = 2;
export const DESTRUCTION_SPAWN_STEP = 140;
export const DESTRUCTION_IMPULSE = 1000;
export const DESTRUCTION_MAX_BODY_COUNT = DESTRUCTION_GRID_COUNT * DESTRUCTION_GRID_COUNT * DESTRUCTION_GRID_COUNT;
export const DESTRUCTION_BOX_COLOR = 0x60a5fa;

const a = f32Div(DESTRUCTION_EXTENT, DESTRUCTION_GRID_COUNT);
const boxHalf = f32Mul(f32(0.8), a);
export const DESTRUCTION_BOX_SIZE = f32Mul(2, boxHalf);

export const destructionWorldCapacity: WorldCapacity = {
  dynamicShapeCount: DESTRUCTION_MAX_BODY_COUNT,
  dynamicBodyCount: DESTRUCTION_MAX_BODY_COUNT,
  contactCount: 50_000,
};

export function destructionCellSize(): number {
  return a;
}

export function destructionBoxHalfExtent(): number {
  return boxHalf;
}

/** Walk the spawn grid with Box3D RNG (same skip rule as upstream `Spawn`). */
export function forEachDestructionBox(rng: Box3DRng, callback: (position: Vec3) => void): number {
  let count = 0;
  for (let i = 0; i < DESTRUCTION_GRID_COUNT; i++) {
    for (let j = 0; j < DESTRUCTION_GRID_COUNT; j++) {
      for (let k = 0; k < DESTRUCTION_GRID_COUNT; k++) {
        if (rng.randomIntRange(1, DESTRUCTION_RANDOM_RANGE) === 1) continue;
        const x = f32Mul(f32Add(f32Sub(f32Mul(2, i), DESTRUCTION_GRID_COUNT), 1), a);
        const y = f32Mul(f32Add(f32Mul(2, j), 1), a);
        const z = f32Mul(f32Add(f32Sub(f32Mul(2, k), DESTRUCTION_GRID_COUNT), 1), a);
        callback([x, y, z]);
        count += 1;
      }
    }
  }
  return count;
}

export function buildDestructionGround(world: PhysicsWorld): number {
  const ground = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  const mesh = world.createGridMesh(40, 40, 1, 0, true);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });
  return ground;
}

export function spawnDestructionBodies(world: PhysicsWorld, runtime: Box3DRuntime, rng: Box3DRng): number[] {
  const handles: number[] = [];
  const hx = destructionBoxHalfExtent();
  forEachDestructionBox(rng, (position) => {
    const body = world.createBody({ type: BodyType.Dynamic, position });
    runtime.createHullShape(body, [hx, hx, hx]);
    handles.push(body);
  });
  explodeDestruction(world);
  return handles;
}

export function explodeDestruction(world: PhysicsWorld): void {
  const extent = DESTRUCTION_EXTENT;
  world.explode(
    [0, f32Mul(2, extent), 0],
    extent,
    f32Mul(f32(0.5), extent),
    DESTRUCTION_IMPULSE,
    0xFFFFFFFFn as unknown as number,
  );
}

export function destroyDestructionBodies(world: PhysicsWorld, handles: number[]): void {
  for (const handle of handles) world.destroyBody(handle);
}

export function buildDestructionDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  return spawnDestructionBodies(world, runtime, new Box3DRng());
}

export function createDestruction(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({
    gravity: [0, -10, 0],
    workerCount: 1,
    capacity: destructionWorldCapacity,
  });
  const ground = buildDestructionGround(world);
  return { world, handles: [ground, ...buildDestructionDynamicBodies(world, runtime)] };
}

export function destructionGroundSize(): Vec3 {
  return [20, 0.5, 20];
}

export const destructionCamera: RenderSpec["camera"] = cameraFromSetView(0, 40, 30, [0, 0, 0]);

export const dumpSampleName = "Destruction";
export const dumpSampleId = "benchmark/destruction";
export const dumpCppSampleName = "Destruction";
export const dumpCreate = createDestruction;
