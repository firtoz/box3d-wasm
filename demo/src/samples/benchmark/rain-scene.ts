import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderSpec } from "../generic-host";
import { buildBenchmarkTileGround, spawnHumanGroup } from "./benchmark-tile-shared";
import { f32 } from "../f32";

const GRID_COUNT = 10;
const GROUP_SIZE = 3;
const GRID_SIZE = f32(15);
const SPAWN_Y = 20;
const STEP_DELAY = 0x2f;

interface RainDumpState {
  columnCount: number;
}

export function buildRainGround(world: PhysicsWorld): number[] {
  return buildBenchmarkTileGround(world, GRID_COUNT);
}

export function stepRain(world: PhysicsWorld, runtime: Box3DRuntime, handles: number[], stepCount: number, state: RainDumpState): void {
  if ((stepCount & STEP_DELAY) !== 0) return;
  if (state.columnCount >= GRID_COUNT) return;
  for (let i = 0; i < GRID_COUNT; i++) {
    spawnHumanGroup(world, runtime, handles, GRID_COUNT, GRID_SIZE, GROUP_SIZE, i, state.columnCount, SPAWN_Y, 5, 1, 0.7);
  }
  state.columnCount += 1;
}

export function buildRainDynamicBodies(world: PhysicsWorld, _runtime: Box3DRuntime): number[] {
  return buildRainGround(world);
}

export function rainGroundSize(): Vec3 { return [150, 1, 150]; }

export const rainCamera: RenderSpec["camera"] = { position: [25, 10, 70], target: [0, 0, 0] };

export const dumpSampleName = "Rain";
export const dumpSampleId = "benchmark/rain";
export const dumpCppSampleName = "Rain";
export const dumpGroundSize = rainGroundSize;

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[]; state: RainDumpState } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const tileHandles = buildRainGround(world);
  return { world, handles: tileHandles, state: { columnCount: 0 } };
}

export function dumpStep(world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], frame: number, _dt: number, state: RainDumpState): void {
  stepRain(world, runtime, handles as number[], frame - 1, state);
}
