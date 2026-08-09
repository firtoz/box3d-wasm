import {
  BodyType,
  type BodyHandle,
  type Box3DRuntime,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Mul } from "../f32";

/** Matches `QUERY_SPAWN_COUNT` / cast radius in `box3d/shared/determinism.h`. */
export const QUERY_SPAWN_COUNT = 50;
export const QUERY_SPAWN_CAST_RADIUS = f32(0.5);
export const QUERY_SPAWN_SEED = 71689;

export interface QuerySpawnState {
  spawnCount: number;
  /** Matches sample `m_frameCount` — spawn on frames where `(frameCount % 10) === 1`. */
  frameCount: number;
  bodies: BodyHandle[];
}

export function createQuerySpawnState(): QuerySpawnState {
  return { spawnCount: 0, frameCount: 0, bodies: [] };
}

/**
 * Port of `QuerySpawnOnce` from `box3d/shared/determinism.c`.
 * Uses WASM Random after `setRandomSeed(71689)` in create.
 */
export function querySpawnOnce(world: PhysicsWorld, runtime: Box3DRuntime, state: QuerySpawnState): void {
  if (state.spawnCount >= QUERY_SPAWN_COUNT) return;

  const rayOrigin = runtime.randomVec3([-12, -12, -12], [12, 12, 12]);
  const unit = runtime.randomUnitVector();
  const rayTranslation: Vec3 = [f32Mul(30, unit[0]), f32Mul(30, unit[1]), f32Mul(30, unit[2])];

  const ray = world.rayCastClosest(rayOrigin, rayTranslation);
  let spawnPosition: Vec3;
  if (ray !== null) {
    spawnPosition = [
      f32Add(ray.point[0], f32Mul(1.2, ray.normal[0])),
      f32Add(ray.point[1], f32Mul(1.2, ray.normal[1])),
      f32Add(ray.point[2], f32Mul(1.2, ray.normal[2])),
    ];
  } else {
    spawnPosition = runtime.randomVec3([-6, -6, -6], [6, 6, 6]);
  }

  const center = runtime.randomVec3Uniform(-10, 10);
  const extent = runtime.randomFloatRange(1, 4);
  const overlapCount = world.overlapAABB(
    [center[0] - extent, center[1] - extent, center[2] - extent],
    [center[0] + extent, center[1] + extent, center[2] + extent],
  );

  const castFraction = world.castShapeSphere(rayOrigin, rayTranslation, QUERY_SPAWN_CAST_RADIUS);
  const size = f32Add(f32(0.3), f32Mul(f32(0.2), castFraction));

  const rotation = runtime.randomQuat();
  const linearVelocity = runtime.randomVec3Uniform(-0.2, 0.2);
  const angularVelocity = runtime.randomVec3Uniform(-0.5, 0.5);

  const body = world.createBody({
    type: BodyType.Dynamic,
    position: spawnPosition,
    rotation,
    linearVelocity,
    angularVelocity,
    linearDamping: 1,
    angularDamping: 1,
  });

  const shapeKind = (state.spawnCount + overlapCount) % 3;
  const shapeDef = { rollingResistance: f32(0.2) };
  if (shapeKind === 0) {
    runtime.createSphereShape(body, [0, 0, 0], size, shapeDef);
  } else if (shapeKind === 1) {
    runtime.createCapsuleShape(body, [0, -size, 0], [0, size, 0], f32Mul(0.7, size), shapeDef);
  } else {
    runtime.createHullShape(body, [size, f32Mul(0.7, size), f32Mul(0.5, size)], shapeDef);
  }

  state.bodies.push(body);
  state.spawnCount += 1;
}

/**
 * Mirror sample Step after Sample::Step: advance every 10th real step (`frameCount % 10 == 1`).
 * Used as dumpPostStep so spawn order matches C++.
 */
export function querySpawnDumpStep(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  _handles: readonly number[],
  _frame: number,
  _dt: number,
  state: QuerySpawnState,
): void {
  state.frameCount += 1;
  if (state.frameCount % 10 === 1) {
    querySpawnOnce(world, runtime, state);
  }
}

export function createQuerySpawn(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: QuerySpawnState;
} {
  const world = runtime.createWorld({ gravity: [0, 0, 0], workerCount: 1 });
  runtime.setRandomSeed(QUERY_SPAWN_SEED);
  const state = createQuerySpawnState();
  // handles grow as bodies spawn; dumpBodies filters by validity — expose mutable array via state.bodies
  return { world, handles: state.bodies as unknown as number[], state };
}

export function querySpawnGroundSize(): Vec3 {
  return [20, 1, 20];
}

/** Placeholder slots for generic-host; bodies become visible as the worker tracks them. */
export function createQuerySpawnBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < QUERY_SPAWN_COUNT; i++) {
    bodies.push({
      kind: "sphere",
      radius: 0.5,
      position: [0, -1000, 0],
      color: 0x60a5fa,
    });
  }
  return bodies;
}

export const querySpawnCamera: RenderSpec["camera"] = cameraFromSetView(45, 25, 30, [0, 0, 0]);

export const dumpSampleName = "Query Spawn";
export const dumpSampleId = "determinism/query-spawn";
export const dumpCppSampleName = "Query Spawn";
export const dumpCreate = createQuerySpawn;
/** C++ QuerySpawn spawns after Sample::Step — use dumpPostStep. */
export const dumpPostStep = querySpawnDumpStep;
