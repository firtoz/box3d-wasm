import {type BodyId, type Box3DRuntime, type HeightFieldHandle, type PhysicsWorld, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32, f32Mul } from "../f32";
import { cameraFromSetView } from "../shared";

/** Matches upstream HeightField under NDEBUG (reference-dump release). */
export const HEIGHT_FIELD_ROW_COUNT = 400;
export const HEIGHT_FIELD_COLUMN_COUNT = 400;
export const HEIGHT_FIELD_AMPLITUDE = f32(0.75);
export const HEIGHT_FIELD_HOLES = false;
export const HEIGHT_FIELD_ROW_FREQUENCY = f32(0.1);
export const HEIGHT_FIELD_COLUMN_FREQUENCY = f32(0.03333);

export function heightFieldScale(): Vec3 {
  return [f32(2), f32Mul(f32(2), HEIGHT_FIELD_AMPLITUDE), f32(2)];
}

export function heightFieldGroundPosition(): Vec3 {
  const scale = heightFieldScale();
  return [
    f32Mul(f32Mul(f32(-0.5), scale[0]), HEIGHT_FIELD_COLUMN_COUNT - 1),
    0,
    f32Mul(f32Mul(f32(-0.5), scale[2]), HEIGHT_FIELD_ROW_COUNT - 1),
  ];
}

export interface HeightFieldScene {
  ground: BodyId;
  heightField: HeightFieldHandle;
}

export function createHeightFieldScene(world: PhysicsWorld, _runtime: Box3DRuntime): HeightFieldScene {
  const scale = heightFieldScale();
  const heightField = world.createWave(
    HEIGHT_FIELD_ROW_COUNT,
    HEIGHT_FIELD_COLUMN_COUNT,
    scale,
    HEIGHT_FIELD_ROW_FREQUENCY,
    HEIGHT_FIELD_COLUMN_FREQUENCY,
    HEIGHT_FIELD_HOLES,
  );
  const ground = world.createBody({ position: heightFieldGroundPosition() });
  world.createHeightFieldShape(ground, heightField, {});
  return { ground, heightField };
}

/** Upstream dynamic bodies are `#if 0` disabled — dump/live only have the heightfield ground. */
export function buildHeightFieldDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  createHeightFieldScene(world, runtime);
  return [];
}

export function heightFieldGroundSize(): Vec3 {
  const scale = heightFieldScale();
  return [
    f32Mul(f32(0.5), f32Mul(scale[0], HEIGHT_FIELD_COLUMN_COUNT - 1)),
    f32(1),
    f32Mul(f32(0.5), f32Mul(scale[2], HEIGHT_FIELD_ROW_COUNT - 1)),
  ];
}

export const heightFieldBodies: RenderBody[] = [];

export const heightFieldCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 40, [0, 0, 0]);

export const heightFieldVisual = {
  rowCount: HEIGHT_FIELD_ROW_COUNT,
  columnCount: HEIGHT_FIELD_COLUMN_COUNT,
  scale: heightFieldScale(),
  rowFrequency: HEIGHT_FIELD_ROW_FREQUENCY,
  columnFrequency: HEIGHT_FIELD_COLUMN_FREQUENCY,
  position: heightFieldGroundPosition(),
} as const;

export const dumpSampleName = "Height Field";
export const dumpSampleId = "mesh/height-field";
/** Disambiguate from Benchmark / Height Field (reference-dump accepts Category/Name). */
export const dumpCppSampleName = "Mesh/Height Field";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  state: { heightField: HeightFieldHandle };
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, heightField } = createHeightFieldScene(world, runtime);
  return {
    world,
    handles: [ground],
    state: { heightField },
    dispose: () => {
      world.destroyHeightField(heightField);
    },
  };
}
