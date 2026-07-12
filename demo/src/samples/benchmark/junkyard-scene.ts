import { B3_PI, BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Quat, type Vec3 } from "box3d-wasm";
import type { RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "../f32";
import { rockHullPoints } from "../continuous/stall-scene";

/** Match upstream release Junkyard (`BENCHMARK_DEBUG=0`): Y count 24, X/Z 0..20. */
export const JUNKYARD_ROCK_Y = 24;
export const JUNKYARD_ROCK_XZ = 21;
export const JUNKYARD_ROCK_COUNT = JUNKYARD_ROCK_Y * JUNKYARD_ROCK_XZ * JUNKYARD_ROCK_XZ;
export const JUNKYARD_ROCK_RADIUS = 1.5;
export const JUNKYARD_PUSHER_RADIUS = 35;
export const JUNKYARD_PUSHER_HEIGHT = 24;
export const JUNKYARD_PUSHER_CYLINDER_RADIUS = 4;
export const JUNKYARD_ROCK_COLOR = 0x78716c;
export const JUNKYARD_PUSHER_COLOR = 0x60a5fa;

const IDENTITY_QUAT: Quat = [0, 0, 0, 1];
const TIME_STEP = f32Div(1, 60);
const OMEGA_DEG = f32(-6);

/** IEEE remainder for float32 operands (`remainderf`). */
function f32Remainder(x: number, y: number): number {
  const nx = f32(x);
  const ny = f32(y);
  const q = f32(nx / ny);
  const n = Math.round(q);
  const nEven = n % 2 === 0 ? n : n - Math.sign(n || 1);
  const nTied = Math.abs(q - n) === 0.5 ? nEven : n;
  return f32(nx - f32(nTied * ny));
}

/** Port of `b3ComputeCosSin` (custom approx, not libm cosf/sinf). */
function computeCosSin(radians: number): { cosine: number; sine: number } {
  const x = f32Remainder(radians, f32Mul(2, B3_PI));
  const pi2 = f32Mul(B3_PI, B3_PI);
  const halfPi = f32Mul(0.5, B3_PI);

  let c: number;
  if (x < -halfPi) {
    const y = f32Add(x, B3_PI);
    const y2 = f32Mul(y, y);
    c = f32(-f32Div(f32Sub(pi2, f32Mul(4, y2)), f32Add(pi2, y2)));
  } else if (x > halfPi) {
    const y = f32Sub(x, B3_PI);
    const y2 = f32Mul(y, y);
    c = f32(-f32Div(f32Sub(pi2, f32Mul(4, y2)), f32Add(pi2, y2)));
  } else {
    const y2 = f32Mul(x, x);
    c = f32Div(f32Sub(pi2, f32Mul(4, y2)), f32Add(pi2, y2));
  }

  let s: number;
  if (x < 0) {
    const y = f32Add(x, B3_PI);
    const term = f32Mul(y, f32Sub(B3_PI, y));
    s = f32(-f32Div(f32Mul(16, term), f32Sub(f32Mul(5, pi2), f32Mul(4, term))));
  } else {
    const term = f32Mul(x, f32Sub(B3_PI, x));
    s = f32Div(f32Mul(16, term), f32Sub(f32Mul(5, pi2), f32Mul(4, term)));
  }

  const mag = f32(Math.sqrt(f32Add(f32Mul(s, s), f32Mul(c, c))));
  const invMag = mag > 0 ? f32Div(1, mag) : 0;
  return { cosine: f32Mul(c, invMag), sine: f32Mul(s, invMag) };
}

export interface JunkyardState {
  pusher: BodyHandle;
  degrees: number;
  radius: number;
}

export function buildJunkyardGround(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle {
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, [120, 1, 120]);
  runtime.createTransformedHullShape(ground, [1, 8, 50], { position: [-50, 8, 0] });
  runtime.createTransformedHullShape(ground, [1, 8, 50], { position: [50, 8, 0] });
  runtime.createTransformedHullShape(ground, [50, 8, 1], { position: [0, 8, -50] });
  runtime.createTransformedHullShape(ground, [50, 8, 1], { position: [0, 8, 50] });
  return ground;
}

export function forEachJunkyardRock(callback: (position: Vec3) => void): void {
  const height = f32(24);
  for (let Y = 0; Y < JUNKYARD_ROCK_Y; Y++) {
    for (let X = 0; X <= 20; X++) {
      for (let Z = 0; Z <= 20; Z++) {
        callback([
          f32Add(f32(-40), f32Mul(f32(4), X)),
          f32Add(f32Add(f32Mul(f32(4), Y), height), f32(1)),
          f32Add(f32(-40), f32Mul(f32(4), Z)),
        ]);
      }
    }
  }
}

export function buildJunkyardRocksAndPusher(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
): { rocks: BodyHandle[]; pusher: BodyHandle; state: JunkyardState } {
  const rocks: BodyHandle[] = [];
  const rockHull = runtime.createRock(JUNKYARD_ROCK_RADIUS);
  forEachJunkyardRock((position) => {
    const body = world.createBody({ type: BodyType.Dynamic, position });
    runtime.createShapeFromHull(body, rockHull, {});
    rocks.push(body);
  });
  runtime.destroyHull(rockHull);

  const radius = f32(JUNKYARD_PUSHER_RADIUS);
  const cylinder = runtime.createCylinder(JUNKYARD_PUSHER_HEIGHT, JUNKYARD_PUSHER_CYLINDER_RADIUS, 0, 16);
  const pusher = world.createBody({
    type: BodyType.Kinematic,
    position: [radius, 0, 0],
  });
  runtime.createShapeFromHull(pusher, cylinder, {});
  runtime.destroyHull(cylinder);

  return {
    rocks,
    pusher,
    state: { pusher, degrees: f32(0), radius },
  };
}

export function buildJunkyardDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const { rocks, pusher } = buildJunkyardRocksAndPusher(world, runtime);
  return [...rocks, pusher];
}

export function stepJunkyard(runtime: Box3DRuntime, state: JunkyardState): void {
  state.degrees = f32Add(state.degrees, f32Mul(OMEGA_DEG, TIME_STEP));
  const cs = computeCosSin(f32Mul(state.degrees, f32Div(B3_PI, 180)));
  const r = state.radius;
  const targetPos: Vec3 = [f32Mul(r, cs.cosine), 0, f32Mul(r, cs.sine)];
  runtime.setBodyTargetTransform(state.pusher, targetPos, IDENTITY_QUAT, TIME_STEP, false);
}

export function junkyardGroundSize(): Vec3 {
  return [120, 1, 120];
}

export function junkyardRockPoints(): [number, number, number][] {
  return rockHullPoints(JUNKYARD_ROCK_RADIUS);
}

export const junkyardCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 125, [0, 0, 0]);

export const dumpSampleName = "Junkyard";
export const dumpSampleId = "benchmark/junkyard";
export const dumpCppSampleName = "Junkyard";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: JunkyardState;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = buildJunkyardGround(world, runtime);
  const { rocks, pusher, state } = buildJunkyardRocksAndPusher(world, runtime);
  return { world, handles: [ground, ...rocks, pusher], state };
}

export function dumpStep(
  _world: PhysicsWorld,
  runtime: Box3DRuntime,
  _handles: readonly number[],
  _frame: number,
  _dt: number,
  state: JunkyardState,
): void {
  stepJunkyard(runtime, state);
}
