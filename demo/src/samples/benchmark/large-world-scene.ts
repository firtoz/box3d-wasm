import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3, type WorldCapacity } from "box3d-wasm";
import type { RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "../f32";

/**
 * Live demo scale: halfway between upstream debug (32²) and release (1000²).
 * Stays under B3W_MAX_BODIES/SHAPES (65536): 200² = 40_000 statics + spheres.
 *
 * Dump / reference-dump stay on debug 32² so C++ DumpLargeWorld parity holds.
 */
export const LARGE_WORLD_GRID = 200;
export const LARGE_WORLD_SPHERES = 100;
export const LARGE_WORLD_DROP_INTERVAL = 5;

/** Match `BENCHMARK_DEBUG` / `DumpLargeWorld` in reference-dump. */
export const LARGE_WORLD_DUMP_GRID = 32;
export const LARGE_WORLD_DUMP_SPHERES = 16;
export const LARGE_WORLD_DUMP_DROP_INTERVAL = 8;

export const LARGE_WORLD_CELL = f32(10);
export const LARGE_WORLD_SPHERE_RADIUS = 0.5;
export const LARGE_WORLD_SPHERE_COLOR = 0x60a5fa;

export type LargeWorldScale = {
  grid: number;
  spheres: number;
  dropInterval: number;
};

export const largeWorldLiveScale: LargeWorldScale = {
  grid: LARGE_WORLD_GRID,
  spheres: LARGE_WORLD_SPHERES,
  dropInterval: LARGE_WORLD_DROP_INTERVAL,
};

export const largeWorldDumpScale: LargeWorldScale = {
  grid: LARGE_WORLD_DUMP_GRID,
  spheres: LARGE_WORLD_DUMP_SPHERES,
  dropInterval: LARGE_WORLD_DUMP_DROP_INTERVAL,
};

export function largeWorldCapacityFor(scale: LargeWorldScale): WorldCapacity {
  const floorCount = scale.grid * scale.grid;
  return {
    staticShapeCount: floorCount,
    staticBodyCount: floorCount,
    dynamicShapeCount: scale.spheres,
    dynamicBodyCount: scale.spheres,
    contactCount: Math.max(1024, 8 * scale.spheres),
  };
}

export const largeWorldCapacity: WorldCapacity = largeWorldCapacityFor(largeWorldLiveScale);

export interface LargeWorldState {
  spheresDropped: number;
  scale: LargeWorldScale;
}

export function largeWorldHalfSpan(grid = LARGE_WORLD_GRID): number {
  return f32Mul(f32Mul(0.5, LARGE_WORLD_CELL), grid);
}

export function largeWorldTileCount(grid = LARGE_WORLD_GRID): number {
  return grid * grid;
}

/** Walk floor tile centers (same order as `buildLargeWorldFloor`). */
export function forEachLargeWorldTile(callback: (position: Vec3) => void, grid = LARGE_WORLD_GRID): void {
  const cell = LARGE_WORLD_CELL;
  const halfSpan = largeWorldHalfSpan(grid);
  for (let i = 0; i < grid; i++) {
    const x = f32Add(f32(-halfSpan), f32Mul(f32Add(i, 0.5), cell));
    for (let j = 0; j < grid; j++) {
      const z = f32Add(f32(-halfSpan), f32Mul(f32Add(j, 0.5), cell));
      callback([x, 0, z]);
    }
  }
}

export function buildLargeWorldFloor(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  grid = LARGE_WORLD_GRID,
): BodyHandle[] {
  const handles: BodyHandle[] = [];
  const half = f32Mul(0.5, LARGE_WORLD_CELL);

  forEachLargeWorldTile((position) => {
    const body = world.createBody({ type: BodyType.Static, position });
    runtime.createHullShape(body, [half, 0.25, half], { invokeContactCreation: true });
    handles.push(body);
  }, grid);
  return handles;
}

export function stepLargeWorld(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  handles: number[],
  stepCount: number,
  state: LargeWorldState,
): void {
  const { spheres, dropInterval, grid } = state.scale;
  if (state.spheresDropped >= spheres) return;
  if (stepCount === 0) return;
  if (stepCount % dropInterval !== 0) return;

  let side = 1;
  while (side * side < spheres) side += 1;

  const idx = state.spheresDropped;
  const gi = idx % side;
  const gj = Math.floor(idx / side);

  const halfSpan = largeWorldHalfSpan(grid);
  const inset = f32Mul(0.1, f32Mul(2, halfSpan));
  const usable = f32Sub(f32Mul(2, halfSpan), f32Mul(2, inset));
  const x = f32Add(f32Add(f32(-halfSpan), inset), f32Mul(f32Add(gi, 0.5), f32Div(usable, side)));
  const z = f32Add(f32Add(f32(-halfSpan), inset), f32Mul(f32Add(gj, 0.5), f32Div(usable, side)));

  const body = world.createBody({
    type: BodyType.Dynamic,
    position: [x, 1.5, z],
  });
  runtime.createSphereShape(body, [0, 0, 0], LARGE_WORLD_SPHERE_RADIUS, {});
  handles.push(body);
  state.spheresDropped += 1;
}

export function largeWorldGroundSize(grid = LARGE_WORLD_GRID): Vec3 {
  const halfSpan = largeWorldHalfSpan(grid);
  return [halfSpan, 0.25, halfSpan];
}

/** Match upstream `SetView(0, 10, 250)` — looks at the center of the tiled floor. */
export const largeWorldCamera: RenderSpec["camera"] = cameraFromSetView(0, 10, 250, [0, 0, 0]);

export const dumpSampleName = "Large World";
export const dumpSampleId = "benchmark/large-world";
export const dumpCppSampleName = "Large World";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: LargeWorldState;
} {
  const scale = largeWorldDumpScale;
  const world = runtime.createWorld({
    gravity: [0, -10, 0],
    workerCount: 1,
    capacity: largeWorldCapacityFor(scale),
  });
  const floors = buildLargeWorldFloor(world, runtime, scale.grid);
  return { world, handles: floors, state: { spheresDropped: 0, scale } };
}

export function dumpStep(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  handles: readonly number[],
  frame: number,
  _dt: number,
  state: LargeWorldState,
): void {
  stepLargeWorld(world, runtime, handles as number[], frame - 1, state);
}
