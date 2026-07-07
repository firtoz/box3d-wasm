import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { RAGDOLL_RENDER_BONES } from "../../ragdoll-render";

const f32 = Math.fround;
const OFFSET_F32 = 1_000_000;
const COUNT = 20;
const HALF_COUNT_F32 = f32(0.5 * COUNT);
const STEP_F32 = f32(0.15);

function midpoint(a: Vec3, b: Vec3): Vec3 {
  return [(a[0] + b[0]) * 0.5, (a[1] + b[1]) * 0.5, (a[2] + b[2]) * 0.5];
}

function length(a: Vec3, b: Vec3): number {
  const dx = b[0] - a[0];
  const dy = b[1] - a[1];
  const dz = b[2] - a[2];
  return Math.hypot(dx, dy, dz);
}

function quatFromUnitVectors(from: Vec3, to: Vec3): [number, number, number, number] {
  const dot = from[0] * to[0] + from[1] * to[1] + from[2] * to[2];
  if (dot < -0.999999) return [0, 0, 1, 0];
  const cross: Vec3 = [
    from[1] * to[2] - from[2] * to[1],
    from[2] * to[0] - from[0] * to[2],
    from[0] * to[1] - from[1] * to[0],
  ];
  const q: [number, number, number, number] = [cross[0], cross[1], cross[2], 1 + dot];
  const qLen = Math.hypot(q[0], q[1], q[2], q[3]);
  return [q[0] / qLen, q[1] / qLen, q[2] / qLen, q[3] / qLen];
}

function normalize(v: Vec3): Vec3 {
  const len = Math.hypot(v[0], v[1], v[2]);
  return [v[0] / len, v[1] / len, v[2] / len];
}

export function buildFarRagdollsDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];

  for (let i = 0; i < COUNT; i++) {
    // Match upstream float32 arithmetic for the ragdoll spawn offsets.
    const iF32 = f32(i);
    const x = f32(f32(STEP_F32 * f32(iF32 - HALF_COUNT_F32)) + OFFSET_F32);
    const y = f32(2 + 0.25 * iF32);
    const z = f32(STEP_F32 * f32(HALF_COUNT_F32 - iF32));
    const human = world.createHuman(
      [x, y, z],
      { frictionTorque: 10, hertz: 0.5, dampingRatio: 0.7, groupIndex: i, colorize: false },
    );
    const boneCount = runtime.getHumanBoneCount();
    for (let j = 0; j < boneCount; j++) {
      const boneBody = runtime.getHumanBoneBody(human, j);
      if (boneBody !== 0) handles.push(boneBody);
    }
  }

  return handles;
}

export function buildFarRagdollsGround(world: PhysicsWorld): number {
  const ground = world.createBody({ type: BodyType.Static, position: [OFFSET_F32, -1, 0] });
  const mesh = world.createGridMesh(20, 20, 1, 1, true);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });
  return ground;
}

export function createFarRagdolls(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = buildFarRagdollsGround(world);
  return { world, handles: [ground, ...buildFarRagdollsDynamicBodies(world, runtime)] };
}

export function farRagdollsGroundSize(): Vec3 {
  return [10, 0.5, 10];
}

export const farRagdollsGroundPosition: Vec3 = [OFFSET_F32, -1, 0];

export const farRagdollsBodies: RenderBody[] = (() => {
  const bodies: RenderBody[] = [];
  const localAxis: Vec3 = [1, 0, 0];
  for (let i = 0; i < COUNT; i++) {
    const iF32 = f32(i);
    const x = f32(f32(STEP_F32 * f32(iF32 - HALF_COUNT_F32)) + OFFSET_F32);
    const y = f32(2 + 0.25 * iF32);
    const z = f32(STEP_F32 * f32(HALF_COUNT_F32 - iF32));
    for (const bone of RAGDOLL_RENDER_BONES) {
      const delta: Vec3 = [bone.b[0] - bone.a[0], bone.b[1] - bone.a[1], bone.b[2] - bone.a[2]];
      bodies.push({
        kind: "capsule",
        axis: "x",
        radius: bone.radius,
        length: length(bone.a, bone.b),
        position: [x + bone.position[0], y + bone.position[1], z + bone.position[2]],
        rotation: bone.rotation,
        localPosition: midpoint(bone.a, bone.b),
        localRotation: quatFromUnitVectors(localAxis, normalize(delta)),
        color: bone.color,
      });
    }
  }
  return bodies;
})();

export const farRagdollsCamera: RenderSpec["camera"] = { position: [1_000_180, 30, 20], target: [1_000_000, 0, 0] };

export const dumpSampleName = "Far Ragdolls";
export const dumpSampleId = "world/far-ragdolls";
export const dumpCppSampleName = "Far Ragdolls";
export const dumpCreate = createFarRagdolls;
