import { B3_PI, BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "../f32";

/** Match upstream release Candy Cups (`m_isDebug` false): 16×16×16. */
const N = 16;
const M = 16;

export const CANDY_CUPS_COUNT = N * M * M;
export const CANDY_CUPS_COLOR = 0x60a5fa;

/** IEEE remainder for float32 operands (`remainderf`). */
function f32Remainder(x: number, y: number): number {
  const nx = f32(x);
  const ny = f32(y);
  const q = f32(nx / ny);
  // Ties to nearest even: match C remainder rounding for exact halfway cases.
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

/** Upstream `CandyCups::CreateConvex(0.6f, 0.0f, 0.95f, 1.0f)` vertex cloud (16 points). */
export function candyCupPoints(): number[] {
  const radius1 = f32(0.6);
  const height1 = f32(0);
  const radius2 = f32(0.95);
  const height2 = f32(1);
  const sideCount = 8;
  const deltaAlpha = f32Div(f32Mul(2, B3_PI), sideCount);
  const points: number[] = [];
  let alpha = f32(0);
  for (let sideIndex = 0; sideIndex < sideCount; sideIndex++) {
    const cs = computeCosSin(alpha);
    points.push(f32Mul(radius1, cs.cosine), height1, f32Mul(radius1, cs.sine));
    points.push(f32Mul(radius2, cs.cosine), height2, f32Mul(radius2, cs.sine));
    alpha = f32Add(alpha, deltaAlpha);
  }
  return points;
}

export function forEachCandyCup(callback: (position: Vec3) => void): void {
  for (let i = 0; i < N; i++) {
    for (let j = 0; j < M; j++) {
      for (let k = 0; k < M; k++) {
        callback([
          f32Add(f32(-10), f32Mul(f32(2.5), j)),
          f32Mul(f32(1), i),
          f32Add(f32(-10), f32Mul(f32(2.5), k)),
        ]);
      }
    }
  }
}

export function buildCandyCupsDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const hull = runtime.createHullFromPoints(candyCupPoints());
  forEachCandyCup((position) => {
    const body = world.createBody({ type: BodyType.Dynamic, position });
    runtime.createShapeFromHull(body, hull, {});
    handles.push(body);
  });
  runtime.destroyHull(hull);
  return handles;
}

export function candyCupsGroundSize(): Vec3 {
  return [60, 1, 60];
}

export const candyCupsCamera: RenderSpec["camera"] = cameraFromSetView(45, 20, 70, [0, 0, 0]);

export const dumpSampleName = "Candy Cups";
export const dumpSampleId = "benchmark/candy-cups";
export const dumpCppSampleName = "Candy Cups";
export const dumpGroundSize = candyCupsGroundSize;
export const dumpBuildDynamicBodies = buildCandyCupsDynamicBodies;
