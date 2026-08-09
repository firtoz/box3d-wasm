import {B3_AXIS_X, B3_PI, BodyType, quatFromAxisAngle, type BodyId, type Box3DRuntime, type PhysicsWorld, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderPart, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "../f32";

const N = 24;
const R = f32(1);
const TUBE_RADIUS = f32Mul(0.1, R);
const AXIS_RADIUS = f32Sub(R, TUBE_RADIUS);
/** `13.0f * B3_PI / 180.0f` */
const TILT_RAD = f32Div(f32Mul(13, B3_PI), 180);
const GEM_RADIUS = f32(0.3);
const GEM_CENTER: Vec3 = [0, f32Mul(f32(-0.65), R), 0];
const SPIN_OMEGA: Vec3 = [0, f32(100), 0];

const STEP_MULTIPLIER = 16;
const STEP_HERTZ = f32Mul(STEP_MULTIPLIER, 60);
const STEP_SUBSTEPS = 8;

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

function ringVertices(): Vec3[] {
  const deltaAngle = f32Div(f32Mul(2, B3_PI), N);
  const cs = computeCosSin(deltaAngle);
  const verts: Vec3[] = [];
  let x = AXIS_RADIUS;
  let y = f32(0);
  for (let i = 0; i < N; i++) {
    verts.push([x, y, 0]);
    const x2 = f32Sub(f32Mul(cs.cosine, x), f32Mul(cs.sine, y));
    const y2 = f32Add(f32Mul(cs.sine, x), f32Mul(cs.cosine, y));
    x = x2;
    y = y2;
  }
  return verts;
}

export function buildClassRingDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const rotation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, TILT_RAD);
  const body = world.createBody({
    type: BodyType.Dynamic,
    position: [0, R, 0],
    rotation,
    allowFastRotation: true,
    enableContactRecycling: false,
  });

  const verts = ringVertices();
  for (let i = 0; i < N; i++) {
    runtime.createCapsuleShape(body, verts[i]!, verts[(i + 1) % N]!, TUBE_RADIUS, { density: 1 });
  }

  runtime.createSphereShape(body, GEM_CENTER, GEM_RADIUS, { density: 2 });

  const angularVelocity = runtime.rotateVector(rotation, SPIN_OMEGA);
  runtime.setBodyAngularVelocity(body, angularVelocity);

  return [body];
}

/** 16× steps at 960 Hz with 8 substeps (matches ClassRing::Step). */
export function stepClassRing(world: PhysicsWorld): void {
  const dt = f32Div(1, STEP_HERTZ);
  for (let i = 0; i < STEP_MULTIPLIER; i++) {
    world.step(dt, STEP_SUBSTEPS);
  }
}

export function classRingGroundSize(): Vec3 {
  return [100, 1, 100];
}

export function createClassRingBodies(): RenderBody[] {
  const verts = ringVertices();
  const parts: RenderPart[] = [];
  for (let i = 0; i < N; i++) {
    const a = verts[i]!;
    const b = verts[(i + 1) % N]!;
    parts.push({
      kind: "ragdoll-capsule",
      a,
      b,
      radius: TUBE_RADIUS,
      color: 0x60a5fa,
    });
  }
  parts.push({
    kind: "sphere",
    radius: GEM_RADIUS,
    position: [GEM_CENTER[0], GEM_CENTER[1], GEM_CENTER[2]],
    color: 0xf59e0b,
  });
  return [
    {
      kind: "compound",
      position: [0, R, 0],
      rotation: quatFromAxisAngle(B3_AXIS_X, TILT_RAD),
      parts: parts as [RenderPart, ...RenderPart[]],
    },
  ];
}

export const classRingCamera: RenderSpec["camera"] = cameraFromSetView(40, 30, 15, [0, 2, 0]);

export const dumpSampleName = "Class Ring";
export const dumpSampleId = "bodies/class-ring";
export const dumpCppSampleName = "Class Ring";
export const dumpGroundSize = classRingGroundSize;
export const dumpBuildDynamicBodies = buildClassRingDynamicBodies;
export const dumpOwnsStep = true;
export const dumpStep = (world: PhysicsWorld): void => {
  stepClassRing(world);
};
