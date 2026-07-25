import {
  BodyType,
  type BodyHandle,
  type Box3DRuntime,
  type PhysicsWorld,
  type Vec3,
  type WorldCapacity,
} from "box3d-wasm";
import type { RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "../f32";

/** Match upstream Release (`BENCHMARK_DEBUG=0`): 8×8×80. */
export const CONVEX_PILE_COUNT_X = 8;
export const CONVEX_PILE_COUNT_Z = 8;
export const CONVEX_PILE_LAYERS = 80;
export const CONVEX_PILE_BODY_COUNT = CONVEX_PILE_COUNT_X * CONVEX_PILE_COUNT_Z * CONVEX_PILE_LAYERS;
export const CONVEX_PILE_AMPLITUDE = f32(2);
export const CONVEX_PILE_POINT_COUNT = 32;
export const CONVEX_PILE_COLOR = 0x60a5fa;

const FLT_MIN = 1.1754943508222875e-38;

export const convexPileWorldCapacity: WorldCapacity = {
  dynamicShapeCount: CONVEX_PILE_BODY_COUNT,
  dynamicBodyCount: CONVEX_PILE_BODY_COUNT,
  contactCount: 50 * 1024,
};

/** PEEL BasicRandom LCG — keep identical to `ConvexPileRandom` in `benchmarks.c`. */
class ConvexPileRandom {
  state: number;
  constructor(seed: number) {
    this.state = seed >>> 0;
  }
  next(): number {
    this.state = (Math.imul(this.state, 2147001325) + 715136305) >>> 0;
    return this.state;
  }
  /** Float in [-0.5, 0.5]. */
  nextFloat(): number {
    return f32(f32Div(this.next() & 0xffff, 65535) - 0.5);
  }
}

/** Match upstream `b3Normalize` (sqrtf path). */
function normalize(v: Vec3): Vec3 {
  const lengthSquared = f32(f32Mul(v[0], v[0]) + f32Mul(v[1], v[1]) + f32Mul(v[2], v[2]));
  if (lengthSquared > f32Mul(1000, FLT_MIN)) {
    const s = f32Div(1, Math.fround(Math.sqrt(lengthSquared)));
    return [f32Mul(s, v[0]), f32Mul(s, v[1]), f32Mul(s, v[2])];
  }
  return [0, 0, 0];
}

function unitRandomPoint(rng: ConvexPileRandom): Vec3 {
  let point: Vec3;
  let lengthSq: number;
  do {
    point = [rng.nextFloat(), rng.nextFloat(), rng.nextFloat()];
    lengthSq = f32(f32Mul(point[0], point[0]) + f32Mul(point[1], point[1]) + f32Mul(point[2], point[2]));
  } while (lengthSq > 0.25);
  return normalize(point);
}

/** Hull of 32 unit-sphere points scaled by amplitude (seed 42). */
export function convexPilePoints(): number[] {
  const rng = new ConvexPileRandom(42);
  const points: number[] = [];
  for (let i = 0; i < CONVEX_PILE_POINT_COUNT; i++) {
    const p = unitRandomPoint(rng);
    points.push(
      f32Mul(CONVEX_PILE_AMPLITUDE, p[0]),
      f32Mul(CONVEX_PILE_AMPLITUDE, p[1]),
      f32Mul(CONVEX_PILE_AMPLITUDE, p[2]),
    );
  }
  return points;
}

export function forEachConvexPileBody(callback: (position: Vec3) => void): void {
  const scatter = f32Mul(2, CONVEX_PILE_AMPLITUDE);
  const halfX = f32Mul(0.5, CONVEX_PILE_COUNT_X);
  const halfZ = f32Mul(0.5, CONVEX_PILE_COUNT_Z);
  for (let layer = 0; layer < CONVEX_PILE_LAYERS; layer++) {
    for (let z = 0; z < CONVEX_PILE_COUNT_Z; z++) {
      for (let x = 0; x < CONVEX_PILE_COUNT_X; x++) {
        const posX = f32Mul(f32Sub(x, halfX), scatter);
        const posZ = f32Mul(f32Sub(z, halfZ), scatter);
        const posY = f32Add(CONVEX_PILE_AMPLITUDE, f32Mul(f32Mul(2, CONVEX_PILE_AMPLITUDE), layer));
        callback([posX, posY, posZ]);
      }
    }
  }
}

export function buildConvexPileGround(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle {
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, [250, 1, 250]);
  return ground;
}

export function buildConvexPileDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const hull = runtime.createHullFromPoints(convexPilePoints());
  const handles: BodyHandle[] = [];
  forEachConvexPileBody((position) => {
    const body = world.createBody({ type: BodyType.Dynamic, position });
    runtime.createShapeFromHull(body, hull, {});
    handles.push(body);
  });
  runtime.destroyHull(hull);
  return handles;
}

export function convexPileGroundSize(): Vec3 {
  return [250, 1, 250];
}

export const convexPileCamera: RenderSpec["camera"] = cameraFromSetView(45, 20, 150, [0, 15, 0]);

export const dumpSampleName = "Convex Pile";
export const dumpSampleId = "benchmark/convex-pile";
export const dumpCppSampleName = "Convex Pile";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
  dispose: () => void;
} {
  const world = runtime.createWorld({
    gravity: [0, -10, 0],
    workerCount: 1,
    capacity: convexPileWorldCapacity,
  });
  const ground = buildConvexPileGround(world, runtime);
  const dynamics = buildConvexPileDynamicBodies(world, runtime);
  return {
    world,
    handles: [ground, ...dynamics],
    dispose: () => {},
  };
}
