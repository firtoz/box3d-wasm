import {
  type BodyId,
  type Box3DRuntime,
  type HeightFieldHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Mul } from "../f32";

export const BENCHMARK_HEIGHT_FIELD_ROW_COUNT = 50;
export const BENCHMARK_HEIGHT_FIELD_COLUMN_COUNT = 50;
export const BENCHMARK_HEIGHT_FIELD_SCALE: Vec3 = [1, 1, 1];
export const BENCHMARK_HEIGHT_FIELD_ROW_FREQUENCY = f32(0.02);
export const BENCHMARK_HEIGHT_FIELD_COLUMN_FREQUENCY = f32(0.04);
export const BENCHMARK_HEIGHT_FIELD_HOLES = true;

export function benchmarkHeightFieldGroundPosition(): Vec3 {
  return [
    f32Mul(f32(-0.5), BENCHMARK_HEIGHT_FIELD_COLUMN_COUNT),
    0,
    f32Mul(f32(-0.5), BENCHMARK_HEIGHT_FIELD_ROW_COUNT),
  ];
}

export interface BenchmarkHeightFieldScene {
  ground: BodyId;
  heightField: HeightFieldHandle;
}

export function createBenchmarkHeightFieldScene(world: PhysicsWorld): BenchmarkHeightFieldScene {
  const heightField = world.createWave(
    BENCHMARK_HEIGHT_FIELD_ROW_COUNT,
    BENCHMARK_HEIGHT_FIELD_COLUMN_COUNT,
    BENCHMARK_HEIGHT_FIELD_SCALE,
    BENCHMARK_HEIGHT_FIELD_ROW_FREQUENCY,
    BENCHMARK_HEIGHT_FIELD_COLUMN_FREQUENCY,
    BENCHMARK_HEIGHT_FIELD_HOLES,
  );
  const ground = world.createBody({ position: benchmarkHeightFieldGroundPosition() });
  world.createHeightFieldShape(ground, heightField, {});
  return { ground, heightField };
}

/** Upstream BenchmarkHeightField only creates the static heightfield ground. */
export function buildBenchmarkHeightFieldDynamicBodies(world: PhysicsWorld, _runtime: Box3DRuntime): BodyId[] {
  createBenchmarkHeightFieldScene(world);
  return [];
}

export function benchmarkHeightFieldGroundSize(): Vec3 {
  return [
    f32Mul(f32(0.5), BENCHMARK_HEIGHT_FIELD_COLUMN_COUNT),
    f32(1),
    f32Mul(f32(0.5), BENCHMARK_HEIGHT_FIELD_ROW_COUNT),
  ];
}

export const benchmarkHeightFieldBodies: RenderBody[] = [];

export const benchmarkHeightFieldCamera: RenderSpec["camera"] = cameraFromSetView(0, 20, 50, [0, 0, 0]);

export const benchmarkHeightFieldVisual = {
  rowCount: BENCHMARK_HEIGHT_FIELD_ROW_COUNT,
  columnCount: BENCHMARK_HEIGHT_FIELD_COLUMN_COUNT,
  scale: BENCHMARK_HEIGHT_FIELD_SCALE,
  rowFrequency: BENCHMARK_HEIGHT_FIELD_ROW_FREQUENCY,
  columnFrequency: BENCHMARK_HEIGHT_FIELD_COLUMN_FREQUENCY,
  position: benchmarkHeightFieldGroundPosition(),
  holes: BENCHMARK_HEIGHT_FIELD_HOLES,
} as const;

export const dumpSampleName = "Height Field";
export const dumpSampleId = "benchmark/height-field";
/** Disambiguate from Mesh / Height Field (reference-dump accepts Category/Name). */
export const dumpCppSampleName = "Benchmark/Height Field";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  state: { heightField: HeightFieldHandle };
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, heightField } = createBenchmarkHeightFieldScene(world);
  return {
    world,
    handles: [ground],
    state: { heightField },
    dispose: () => {
      world.destroyHeightField(heightField);
    },
  };
}
