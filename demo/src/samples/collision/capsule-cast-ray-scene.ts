import {BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

export const CAPSULE_CAST_RAY_STRIDE_FLOATS = 8; // { hit u8, pad[3], fraction f32, point xyz, normal xyz }
export const CAPSULE_CAST_RAY_HEADER_FLOATS = 1;

export type CapsuleCastRayHitDump = {
  h: number;
  f: number;
  p: Vec3;
  n: Vec3;
};

export type CapsuleCastRayRaysDump = {
  o: number;
  r: [CapsuleCastRayHitDump];
};

export type CapsuleCastRayDumpState = {
  rays: CapsuleCastRayRaysDump;
};

export const capsuleCastRayOrigin: Vec3 = [-1, 0.5, 0];
export const capsuleCastRayTranslation: Vec3 = [2, 0, 0];
const capsuleCastRayIdentity = { position: [0, 0, 0] as Vec3, rotation: [0, 0, 0, 1] as [number, number, number, number] };

export function buildCapsuleCastRayDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const body = world.createBody({ type: BodyType.Kinematic });
  runtime.createCapsuleShape(body, [0, 0, 0], [0, 1, 0], 0.5);
  return [body];
}

export function collectCapsuleCastRay(world: PhysicsWorld, bodyHandle: BodyId): CapsuleCastRayRaysDump {
  const result = world.bodyCastRay(bodyHandle, capsuleCastRayOrigin, capsuleCastRayTranslation, {
    maxFraction: 1,
    bodyTransform: capsuleCastRayIdentity,
  });
  const end: Vec3 = [
    capsuleCastRayOrigin[0] + capsuleCastRayTranslation[0],
    capsuleCastRayOrigin[1] + capsuleCastRayTranslation[1],
    capsuleCastRayOrigin[2] + capsuleCastRayTranslation[2],
  ];
  const hit: CapsuleCastRayHitDump = result.hit
    ? { h: 1, f: result.fraction, p: result.point, n: result.normal }
    : { h: 0, f: 1, p: end, n: [0, 1, 0] };
  return { o: 0, r: [hit] };
}

export function captureCapsuleCastRay(world: PhysicsWorld, handles: readonly BodyId[], state: CapsuleCastRayDumpState): void {
  state.rays = collectCapsuleCastRay(world, handles[0]!);
}

export function createCapsuleCastRayDumpState(world: PhysicsWorld, handles: readonly BodyId[]): CapsuleCastRayDumpState {
  const state: CapsuleCastRayDumpState = { rays: { o: 0, r: [{ h: 0, f: 1, p: [1, 0.5, 0], n: [0, 1, 0] }] } };
  captureCapsuleCastRay(world, handles, state);
  return state;
}

export function createCapsuleCastRay(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  state: CapsuleCastRayDumpState;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const handles = buildCapsuleCastRayDynamicBodies(world, runtime);
  return { world, handles, state: createCapsuleCastRayDumpState(world, handles) };
}

export function writeCapsuleCastRayBuffer(rays: CapsuleCastRayRaysDump, out: Float32Array): void {
  out[0] = rays.o;
  const base = CAPSULE_CAST_RAY_HEADER_FLOATS;
  const ray = rays.r[0];
  const u8 = new Uint8Array(out.buffer, out.byteOffset + base * 4, 4);
  u8[0] = ray.h;
  out[base + 1] = ray.f;
  out[base + 2] = ray.p[0];
  out[base + 3] = ray.p[1];
  out[base + 4] = ray.p[2];
  out[base + 5] = ray.n[0];
  out[base + 6] = ray.n[1];
  out[base + 7] = ray.n[2];
}

export function castCapsuleCastRay(world: PhysicsWorld, handles: readonly BodyId[], out: Float32Array): void {
  writeCapsuleCastRayBuffer(collectCapsuleCastRay(world, handles[0]!), out);
}

export function dumpPostStepCapsuleCastRay(
  world: PhysicsWorld,
  _runtime: Box3DRuntime,
  handles: readonly BodyId[],
  _frame: number,
  _dt: number,
  state: CapsuleCastRayDumpState,
): void {
  captureCapsuleCastRay(world, handles, state);
}

export function dumpCheckpointExtrasCapsuleCastRay(
  _world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly BodyId[],
  _frame: number,
  state: CapsuleCastRayDumpState,
): Record<string, unknown> {
  return { rays: state.rays };
}

export function capsuleCastRayGroundSize(): Vec3 {
  return [5, 0.5, 5];
}

export const capsuleCastRayBodies: RenderBody[] = [
  {
    kind: "capsule",
    radius: 0.5,
    length: 1,
    axis: "y",
    position: [0, 0, 0],
    localPosition: [0, 0.5, 0],
    type: BodyType.Kinematic,
    color: 0x34d399,
  },
];

export const capsuleCastRayCamera: RenderSpec["camera"] = cameraFromSetView(120, 30, 20, [0, 1.5, 0]);

export const dumpSampleName = "Capsule Cast Ray";
export const dumpSampleId = "collision/capsule-cast-ray";
export const dumpCppSampleName = "Capsule Cast Ray";
export const dumpCreate = createCapsuleCastRay;
export const dumpPostStep = dumpPostStepCapsuleCastRay;
export const dumpCheckpointExtras = dumpCheckpointExtrasCapsuleCastRay;
