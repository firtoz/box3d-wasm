import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3, type WorldCapacity } from "box3d-wasm";
import type { RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "../f32";

/**
 * Debug-scale Large World matching `BENCHMARK_DEBUG` in `benchmarks.c`.
 * Upstream release is GRID=1000 (1M statics) / SPHERES=100 / DROP_INTERVAL=5 — not dumpable.
 */
export const LARGE_WORLD_GRID = 32;
export const LARGE_WORLD_SPHERES = 16;
export const LARGE_WORLD_DROP_INTERVAL = 8;
export const LARGE_WORLD_CELL = f32(10);
export const LARGE_WORLD_SPHERE_RADIUS = 0.5;
export const LARGE_WORLD_SPHERE_COLOR = 0x60a5fa;

export const largeWorldCapacity: WorldCapacity = {
  staticShapeCount: 1024,
  staticBodyCount: 1024,
  dynamicShapeCount: LARGE_WORLD_SPHERES,
  dynamicBodyCount: LARGE_WORLD_SPHERES,
  contactCount: Math.max(1024, 8 * LARGE_WORLD_SPHERES),
};

export interface LargeWorldState {
  spheresDropped: number;
}

export function largeWorldHalfSpan(): number {
  return f32Mul(f32Mul(0.5, LARGE_WORLD_CELL), LARGE_WORLD_GRID);
}

export function largeWorldTileCount(): number {
  return LARGE_WORLD_GRID * LARGE_WORLD_GRID;
}

/** Walk floor tile centers (same order as `buildLargeWorldFloor`). */
export function forEachLargeWorldTile(callback: (position: Vec3) => void): void {
  const cell = LARGE_WORLD_CELL;
  const halfSpan = largeWorldHalfSpan();
  for (let i = 0; i < LARGE_WORLD_GRID; i++) {
    const x = f32Add(f32(-halfSpan), f32Mul(f32Add(i, 0.5), cell));
    for (let j = 0; j < LARGE_WORLD_GRID; j++) {
      const z = f32Add(f32(-halfSpan), f32Mul(f32Add(j, 0.5), cell));
      callback([x, 0, z]);
    }
  }
}

export function buildLargeWorldFloor(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const handles: BodyHandle[] = [];
  const half = f32Mul(0.5, LARGE_WORLD_CELL);

  forEachLargeWorldTile((position) => {
    const body = world.createBody({ type: BodyType.Static, position });
    runtime.createHullShape(body, [half, 0.25, half], { invokeContactCreation: true });
    handles.push(body);
  });
  return handles;
}

export function stepLargeWorld(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  handles: number[],
  stepCount: number,
  state: LargeWorldState,
): void {
  if (state.spheresDropped >= LARGE_WORLD_SPHERES) return;
  if (stepCount === 0) return;
  if (stepCount % LARGE_WORLD_DROP_INTERVAL !== 0) return;

  let side = 1;
  while (side * side < LARGE_WORLD_SPHERES) side += 1;

  const idx = state.spheresDropped;
  const gi = idx % side;
  const gj = Math.floor(idx / side);

  const halfSpan = largeWorldHalfSpan();
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

export function largeWorldGroundSize(): Vec3 {
  const halfSpan = largeWorldHalfSpan();
  return [halfSpan, 0.25, halfSpan];
}

export const largeWorldCamera: RenderSpec["camera"] = cameraFromSetView(0, 10, 250, [0, 0, 0]);

export const dumpSampleName = "Large World";
export const dumpSampleId = "benchmark/large-world";
export const dumpCppSampleName = "Large World";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: LargeWorldState;
} {
  const world = runtime.createWorld({
    gravity: [0, -10, 0],
    workerCount: 1,
    capacity: largeWorldCapacity,
  });
  const floors = buildLargeWorldFloor(world, runtime);
  return { world, handles: floors, state: { spheresDropped: 0 } };
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
