import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Mul } from "../f32";

export const RAY_CURTAIN_RAY_COUNT = Math.floor((8 - (-8)) / 0.1) + 1; // 161
export const RAY_CURTAIN_ABS_SPEED = 0.015;
/** SAB layout: offset f32 + per-ray { hit u8, pad[3], fraction f32, point xyz, normal xyz } */
export const RAY_CURTAIN_RAY_STRIDE_FLOATS = 8; // 32 bytes
export const RAY_CURTAIN_HEADER_FLOATS = 1;

export type RayCurtainHitDump = {
  h: number;
  f: number;
  p: Vec3;
  n: Vec3;
};

export type RayCurtainRaysDump = {
  o: number;
  r: RayCurtainHitDump[];
};

export type RayCurtainDumpState = {
  offset: number;
  speed: number;
  rays: RayCurtainRaysDump;
};

export function buildRayCurtainDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const ang: Vec3 = [0.8, 0.4, 0.8];
  const handles: number[] = [];

  const sphere = world.createBody({
    type: BodyType.Kinematic,
    position: [-6, 3, 0],
    angularVelocity: ang,
  });
  runtime.createSphereShape(sphere, [0, 0, 0], 0.9);
  handles.push(sphere);

  const capsule = world.createBody({
    type: BodyType.Kinematic,
    position: [-2, 3, 0],
    angularVelocity: ang,
  });
  runtime.createCapsuleShape(capsule, [-0.5, 0, 0], [0.5, 0, 0], 0.8);
  handles.push(capsule);

  const box = world.createBody({
    type: BodyType.Kinematic,
    position: [2, 3, 0],
    angularVelocity: ang,
  });
  runtime.createHullShape(box, [0.6, 0.6, 0.6]);
  handles.push(box);

  const meshBody = world.createBody({
    type: BodyType.Kinematic,
    position: [6, 3, 0],
    angularVelocity: ang,
  });
  const mesh = world.createTorusMesh(10, 12, 0.65, 0.35);
  world.createMeshShape(meshBody, mesh, { scale: [1, 1, 1] });
  handles.push(meshBody);

  return handles;
}

export function createRayCurtain(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: RayCurtainDumpState;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const handles = buildRayCurtainDynamicBodies(world, runtime);
  const state = createRayCurtainDumpState();
  captureRayCurtainRays(world, state);
  return { world, handles, state };
}

export function createRayCurtainDumpState(): RayCurtainDumpState {
  return {
    offset: 2,
    speed: -RAY_CURTAIN_ABS_SPEED,
    rays: { o: 2, r: [] },
  };
}

/** Mirror upstream Render(): cast with current offset, then bounce/advance. */
export function advanceRayCurtainOffset(state: RayCurtainDumpState): void {
  if (state.offset > 2) state.speed = -RAY_CURTAIN_ABS_SPEED;
  else if (state.offset < -2) state.speed = RAY_CURTAIN_ABS_SPEED;
  state.offset = f32Add(state.offset, state.speed);
}

export function rayCurtainRayX(index: number): number {
  return f32Add(-8, f32Mul(index, 0.1));
}

export function collectRayCurtainHits(world: PhysicsWorld, offset: number): RayCurtainRaysDump {
  const o = f32(offset);
  const r: RayCurtainHitDump[] = [];
  for (let i = 0; i < RAY_CURTAIN_RAY_COUNT; i++) {
    const x = rayCurtainRayX(i);
    const origin: Vec3 = [x, 8, o];
    const translation: Vec3 = [0, -8, 0];
    const hit = world.rayCastClosest(origin, translation);
    if (hit === null) {
      r.push({ h: 0, f: 1, p: [origin[0], origin[1] + translation[1], origin[2]], n: [0, 1, 0] });
    } else {
      r.push({
        h: 1,
        f: hit.fraction,
        p: [hit.point[0], hit.point[1], hit.point[2]],
        n: [hit.normal[0], hit.normal[1], hit.normal[2]],
      });
    }
  }
  return { o, r };
}

export function captureRayCurtainRays(world: PhysicsWorld, state: RayCurtainDumpState): void {
  state.rays = collectRayCurtainHits(world, state.offset);
}

/** Fill the live-demo SAB layout from a cast at `offset`. */
export function castRayCurtain(world: PhysicsWorld, offset: number, out: Float32Array): void {
  const hits = collectRayCurtainHits(world, offset);
  out[0] = hits.o;
  for (let i = 0; i < RAY_CURTAIN_RAY_COUNT; i++) {
    const base = RAY_CURTAIN_HEADER_FLOATS + i * RAY_CURTAIN_RAY_STRIDE_FLOATS;
    const ray = hits.r[i]!;
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
}

export function dumpPostStepRayCurtain(
  world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly number[],
  _frame: number,
  _dt: number,
  state: RayCurtainDumpState,
): void {
  // Upstream casts in Render after Step, then advances offset.
  captureRayCurtainRays(world, state);
  advanceRayCurtainOffset(state);
}

export function dumpCheckpointExtrasRayCurtain(
  _world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly number[],
  _frame: number,
  state: RayCurtainDumpState,
): Record<string, unknown> {
  return { rays: state.rays };
}

export function rayCurtainGroundSize(): Vec3 {
  return [20, 0.5, 20];
}

export const rayCurtainBodies: RenderBody[] = [
  { kind: "sphere", radius: 0.9, position: [-6, 3, 0], type: BodyType.Kinematic, color: 0x60a5fa },
  { kind: "capsule", radius: 0.8, length: 1, axis: "x", position: [-2, 3, 0], type: BodyType.Kinematic, color: 0x34d399 },
  { kind: "box", size: [1.2, 1.2, 1.2], position: [2, 3, 0], type: BodyType.Kinematic, color: 0xfbbf24 },
  { kind: "torus", radius: 0.65, tube: 0.35, radialSegments: 10, tubularSegments: 12, position: [6, 3, 0], type: BodyType.Kinematic, color: 0xf472b6 },
];

export const rayCurtainCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 20, [0, 0, 0]);

export const dumpSampleName = "Ray Curtain";
export const dumpSampleId = "collision/ray-curtain";
export const dumpCppSampleName = "Ray Curtain";
export const dumpCreate = createRayCurtain;
export const dumpPostStep = dumpPostStepRayCurtain;
export const dumpCheckpointExtras = dumpCheckpointExtrasRayCurtain;
