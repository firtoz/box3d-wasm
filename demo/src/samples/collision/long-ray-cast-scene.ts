import {BodyType, type BodyId, type Box3DRuntime, type HeightFieldHandle, type HullHandle, type MeshHandle, type PhysicsWorld, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { rockHullPoints } from "../continuous/stall-scene";
import { f32, f32Mul } from "../f32";
import { cameraFromSetView } from "../shared";

export const LONG_RAY_CAST_SHAPE_COUNT = 5;
export const LONG_RAY_CAST_SPACING = f32(5);
export const LONG_RAY_CAST_AIM_HEIGHT = f32(2.5);

const HF_COUNT = 9;
const HF_SCALE: Vec3 = [f32(0.5), f32(0.5), f32(0.5)];
const HF_ROW_FREQUENCY = f32(0.08);
const HF_COLUMN_FREQUENCY = f32(0.16);

export interface LongRayCastResources {
  hull: HullHandle;
  mesh: MeshHandle;
  heightField: HeightFieldHandle;
}

export function longRayCastTargetX(i: number): number {
  return f32Mul(f32(i - 2), LONG_RAY_CAST_SPACING);
}

export function buildLongRayCastScene(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
): { handles: BodyId[]; resources: LongRayCastResources } {
  const hull = runtime.createRock(f32(1));
  const mesh = world.createWaveMesh(8, 8, f32(0.5), f32(0.25), f32(0.2), f32(0.2));
  const heightField = world.createWave(HF_COUNT, HF_COUNT, HF_SCALE, HF_ROW_FREQUENCY, HF_COLUMN_FREQUENCY, false);

  const handles: BodyId[] = [];

  // Sphere
  {
    const body = world.createBody({ type: BodyType.Static, position: [longRayCastTargetX(0), 0, 0] });
    world.createSphereShape(body, [0, 0, 0], f32(1));
    handles.push(body);
  }

  // Capsule along X
  {
    const body = world.createBody({ type: BodyType.Static, position: [longRayCastTargetX(1), 0, 0] });
    world.createCapsuleShape(body, [f32(-1), 0, 0], [f32(1), 0, 0], f32(0.7));
    handles.push(body);
  }

  // Rock hull
  {
    const body = world.createBody({ type: BodyType.Static, position: [longRayCastTargetX(2), 0, 0] });
    world.createShapeFromHull(body, hull);
    handles.push(body);
  }

  // Wave mesh
  {
    const body = world.createBody({ type: BodyType.Static, position: [longRayCastTargetX(3), 0, 0] });
    world.createMeshShape(body, mesh, { scale: [1, 1, 1] });
    handles.push(body);
  }

  // Height field — offset to center under the ray
  {
    const extentX = f32Mul(HF_SCALE[0], HF_COUNT - 1);
    const extentZ = f32Mul(HF_SCALE[2], HF_COUNT - 1);
    const x = f32(longRayCastTargetX(4) - f32Mul(f32(0.5), extentX));
    const body = world.createBody({
      type: BodyType.Static,
      position: [x, 0, f32Mul(f32(-0.5), extentZ)],
    });
    world.createHeightFieldShape(body, heightField);
    handles.push(body);
  }

  return { handles, resources: { hull, mesh, heightField } };
}

export function buildLongRayCastDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  return buildLongRayCastScene(world, runtime).handles;
}

export function longRayCastGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const longRayCastBodies: RenderBody[] = [
  {
    kind: "sphere",
    radius: 1,
    position: [longRayCastTargetX(0), 0, 0],
    type: BodyType.Static,
    color: 0x60a5fa,
  },
  {
    kind: "capsule",
    axis: "x",
    radius: 0.7,
    length: 2,
    position: [longRayCastTargetX(1), 0, 0],
    type: BodyType.Static,
    color: 0x34d399,
  },
  {
    kind: "hull",
    points: rockHullPoints(1),
    position: [longRayCastTargetX(2), 0, 0],
    type: BodyType.Static,
    color: 0xa78bfa,
  },
  {
    // Wave mesh drawn via overlay; placeholder keeps body index mapping.
    kind: "box",
    size: [0.01, 0.01, 0.01],
    position: [longRayCastTargetX(3), -10, 0],
    type: BodyType.Static,
    color: 0xfbbf24,
  },
  {
    // Heightfield drawn via overlay.
    kind: "box",
    size: [0.01, 0.01, 0.01],
    position: [longRayCastTargetX(4), -10, 0],
    type: BodyType.Static,
    color: 0xf97316,
  },
];

export const longRayCastCamera: RenderSpec["camera"] = cameraFromSetView(-35, 22, 34, [0, 1, 0]);

export const longRayCastWaveMeshVisual = {
  xCount: 8,
  zCount: 8,
  cellWidth: 0.5,
  amplitude: 0.25,
  rowFrequency: 0.2,
  columnFrequency: 0.2,
  position: [longRayCastTargetX(3), 0, 0] as Vec3,
} as const;

export const longRayCastHeightFieldVisual = {
  rowCount: HF_COUNT,
  columnCount: HF_COUNT,
  scale: HF_SCALE,
  rowFrequency: HF_ROW_FREQUENCY,
  columnFrequency: HF_COLUMN_FREQUENCY,
  position: [
    f32(longRayCastTargetX(4) - f32Mul(f32(0.5), f32Mul(HF_SCALE[0], HF_COUNT - 1))),
    0,
    f32Mul(f32(-0.5), f32Mul(HF_SCALE[2], HF_COUNT - 1)),
  ] as Vec3,
} as const;

export const dumpSampleName = "Long Ray Cast";
export const dumpSampleId = "collision/long-ray-cast";
export const dumpCppSampleName = "Long Ray Cast";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, 0, 0], workerCount: 1 });
  const { handles, resources } = buildLongRayCastScene(world, runtime);
  return {
    world,
    handles,
    dispose: () => {
      runtime.destroyHull(resources.hull);
      world.destroyMesh(resources.mesh);
      world.destroyHeightField(resources.heightField);
    },
  };
}
