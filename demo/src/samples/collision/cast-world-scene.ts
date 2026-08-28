import {
  B3_PI,
  BodyType,
  WorldCastMode,
  WorldCastType,
  type AABB,
  type BodyId,
  type Box3DRuntime,
  type HeightFieldHandle,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
  type WorldCastHit,
  type WorldCastModeId,
  type WorldCastTypeId,
} from "box3d-wasm";
import { Box3DRng } from "../box3d-rng";
import { cameraFromSetView } from "../shared";

export const CAST_WORLD_MAX = 64;
export const CAST_WORLD_IGNORE_BASE = 0x7;
export const CAST_WORLD_HEADER_FLOATS = 16;
export const CAST_WORLD_HIT_STRIDE = 10;
export const CAST_WORLD_HIT_CAPACITY = 3;
export const CAST_WORLD_BODY_STRIDE = 8;

export const CAST_KIND_SPHERE = 0;
export const CAST_KIND_CAPSULE = 1;
export const CAST_KIND_HULL = 2;
export const CAST_KIND_MESH = 3;
export const CAST_KIND_HEIGHT = 4;

export const castWorldDefaultOrigin: Vec3 = [-20, 10, 0];
export const castWorldDefaultTranslation: Vec3 = [20, 10, 0];
export const castWorldDefaultRadius = 0.5;
export const castWorldDefaultMode: WorldCastModeId = WorldCastMode.Closest;
export const castWorldDefaultType: WorldCastTypeId = WorldCastType.Ray;

const SPHERE_RADIUS = 0.9;
const CAPSULE = { center1: [-0.5, 0, 0] as Vec3, center2: [0.5, 0, 0] as Vec3, radius: 0.8 };
const HULL_HALF: Vec3 = [0.6, 0.6, 0.6];
const MESH_SCALE: Vec3 = [4, 3, -2];
const HF_COUNT = 10;
const HF_SCALE: Vec3 = [0.5, 0.5, 0.5];
const HF_ROW_FREQUENCY = 0.03;
const HF_COLUMN_FREQUENCY = 0.09;

export type CastWorldHitDump = {
  f: number;
  p: Vec3;
  n: Vec3;
  m: number;
  t: number;
};

export type CastWorldDump = {
  c: number;
  h: CastWorldHitDump[];
};

export function castWorldGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const castWorldBodies: never[] = [];
export const castWorldCamera = cameraFromSetView(45, 30, 20, [0, 0, 0]);

export type CastWorldResources = {
  mesh: MeshHandle;
  heightField: HeightFieldHandle;
};

export function createCastWorldResources(world: PhysicsWorld): CastWorldResources {
  return {
    mesh: world.createTorusMesh(10, 12, 0.65, 0.35),
    heightField: world.createWave(HF_COUNT, HF_COUNT, HF_SCALE, HF_ROW_FREQUENCY, HF_COLUMN_FREQUENCY, false),
  };
}

function bodyTypeForIndex(index: number, kind: number): BodyType {
  if (kind === CAST_KIND_HEIGHT) return BodyType.Static;
  if (index % 3 === 0) return BodyType.Kinematic;
  if (index % 2 === 0) return BodyType.Dynamic;
  return BodyType.Static;
}

export function createCastWorldShape(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  resources: CastWorldResources,
  rng: Box3DRng,
  index: number,
  kind: number,
): BodyId {
  const position = rng.randomVec3Uniform(-20, 20);
  const axis = runtime.getLengthAndNormalize(rng.randomVec3Uniform(-1, 1)).direction;
  const angle = rng.randomFloatRange(-B3_PI, B3_PI);
  const rotation = runtime.makeQuatFromAxisAngle(axis, angle);
  const type = bodyTypeForIndex(index, kind);
  const body = world.createBody({ type, position, rotation, gravityScale: 0 });
  const ignore = (index & CAST_WORLD_IGNORE_BASE) === CAST_WORLD_IGNORE_BASE ? 1 : 0;
  if (kind === CAST_KIND_SPHERE) {
    runtime.createSphereShape(body, [0, 0, 0], SPHERE_RADIUS, { userMaterialId: 11, userData: ignore });
  } else if (kind === CAST_KIND_CAPSULE) {
    runtime.createCapsuleShape(body, CAPSULE.center1, CAPSULE.center2, CAPSULE.radius, { userMaterialId: 22, userData: ignore });
  } else if (kind === CAST_KIND_HULL) {
    runtime.createHullShape(body, HULL_HALF, { userMaterialId: 33, userData: ignore });
  } else if (kind === CAST_KIND_MESH) {
    world.createMeshShape(body, resources.mesh, { scale: MESH_SCALE, userMaterialId: 44, userData: ignore });
  } else {
    world.createHeightFieldShape(body, resources.heightField, {
      userMaterialId: 55,
      userData: ignore,
      extraUserMaterialIds: [111, 222, 333],
    });
  }
  return body;
}

export function runCastWorld(
  world: PhysicsWorld,
  origin: Vec3,
  translation: Vec3,
  mode: WorldCastModeId,
  type: WorldCastTypeId,
  radius: number,
  initialOverlap: boolean,
): WorldCastHit[] {
  return world.worldCast({ origin, translation, mode, type, radius, initialOverlap });
}

export function dumpCastWorldHits(hits: readonly WorldCastHit[]): CastWorldDump {
  return {
    c: hits.length,
    h: hits.map((hit) => ({
      f: hit.fraction,
      p: hit.point,
      n: hit.normal,
      m: hit.userMaterialId,
      t: hit.triangleIndex,
    })),
  };
}

export function collectCastWorldDefault(world: PhysicsWorld): CastWorldDump {
  return dumpCastWorldHits(runCastWorld(
    world,
    castWorldDefaultOrigin,
    castWorldDefaultTranslation,
    castWorldDefaultMode,
    castWorldDefaultType,
    castWorldDefaultRadius,
    false,
  ));
}

export function writeCastWorldBuffer(
  values: Float32Array,
  origin: Vec3,
  translation: Vec3,
  mode: number,
  type: number,
  radius: number,
  initialOverlap: boolean,
  hits: readonly WorldCastHit[],
  bodies: readonly { kind: number; ignore: number; aabb: AABB }[],
): void {
  values[0] = origin[0];
  values[1] = origin[1];
  values[2] = origin[2];
  values[3] = translation[0];
  values[4] = translation[1];
  values[5] = translation[2];
  values[6] = mode;
  values[7] = type;
  values[8] = radius;
  values[9] = initialOverlap ? 1 : 0;
  values[10] = hits.length;
  values[11] = bodies.length;
  const hitBase = CAST_WORLD_HEADER_FLOATS;
  for (let i = 0; i < CAST_WORLD_HIT_CAPACITY; i++) {
    const o = hitBase + i * CAST_WORLD_HIT_STRIDE;
    const hit = hits[i];
    if (hit === undefined) {
      for (let k = 0; k < CAST_WORLD_HIT_STRIDE; k++) values[o + k] = 0;
      continue;
    }
    values[o] = hit.fraction;
    values[o + 1] = hit.point[0];
    values[o + 2] = hit.point[1];
    values[o + 3] = hit.point[2];
    values[o + 4] = hit.normal[0];
    values[o + 5] = hit.normal[1];
    values[o + 6] = hit.normal[2];
    values[o + 7] = hit.userMaterialId;
    values[o + 8] = hit.triangleIndex;
    values[o + 9] = hit.childIndex;
  }
  const bodyBase = CAST_WORLD_HEADER_FLOATS + CAST_WORLD_HIT_CAPACITY * CAST_WORLD_HIT_STRIDE;
  for (let i = 0; i < CAST_WORLD_MAX; i++) {
    const o = bodyBase + i * CAST_WORLD_BODY_STRIDE;
    const body = bodies[i];
    if (body === undefined) {
      for (let k = 0; k < CAST_WORLD_BODY_STRIDE; k++) values[o + k] = 0;
      continue;
    }
    values[o] = body.kind;
    values[o + 1] = body.ignore;
    values[o + 2] = body.aabb.min[0];
    values[o + 3] = body.aabb.min[1];
    values[o + 4] = body.aabb.min[2];
    values[o + 5] = body.aabb.max[0];
    values[o + 6] = body.aabb.max[1];
    values[o + 7] = body.aabb.max[2];
  }
}

export const dumpSampleName = "Cast World";
export const dumpSampleId = "collision/cast-world";
export const dumpCppSampleName = "Cast World";
export const dumpGroundSize = castWorldGroundSize;
export const dumpBuildDynamicBodies = (): BodyId[] => [];

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: [] };
}

export function dumpCheckpointExtras(world: PhysicsWorld): Record<string, unknown> {
  return { worldCast: collectCastWorldDefault(world) };
}

export const castWorldHeightFieldVisual = {
  rowCount: HF_COUNT,
  columnCount: HF_COUNT,
  scale: HF_SCALE,
  rowFrequency: HF_ROW_FREQUENCY,
  columnFrequency: HF_COLUMN_FREQUENCY,
};

export const castWorldCapsuleLength = 1;
export const castWorldSphereRadius = SPHERE_RADIUS;
export const castWorldHullSize: [number, number, number] = [1.2, 1.2, 1.2];
export const castWorldMeshScale = MESH_SCALE;
export const castWorldTorus = { radius: 0.65, tube: 0.35, radialSegments: 10, tubularSegments: 12 };
