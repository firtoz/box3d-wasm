import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32 } from "../f32";

/** Match upstream release Explosion (`m_isDebug` false): n=16 → 33×33 cylinders. */
const N = 16;

export const EXPLOSION_CYLINDER_COUNT = (2 * N + 1) * (2 * N + 1);
export const EXPLOSION_CYLINDER_COLOR = 0x60a5fa;
export const EXPLOSION_CYLINDER_RADIUS = 0.2;
export const EXPLOSION_CYLINDER_HEIGHT = 0.5;
export const EXPLOSION_IMPULSE = 1000;

export function forEachExplosionCylinder(callback: (position: Vec3) => void): void {
  for (let i = -N; i <= N; i++) {
    for (let k = -N; k <= N; k++) {
      callback([f32(i), f32(0), f32(k)]);
    }
  }
}

/** Static arena: grid mesh floor + four walls on one body (matches upstream order). */
export function buildExplosionGround(world: PhysicsWorld, runtime: Box3DRuntime): number {
  const ground = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  const mesh = world.createGridMesh(40, 40, 1, 0, true);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });

  const hy = 1;
  runtime.createTransformedHullShape(ground, [20, hy, 0.1], { position: [0, hy, -20] });
  runtime.createTransformedHullShape(ground, [20, hy, 0.1], { position: [0, hy, 20] });
  runtime.createTransformedHullShape(ground, [0.1, hy, 20], { position: [-20, hy, 0] });
  runtime.createTransformedHullShape(ground, [0.1, hy, 20], { position: [20, hy, 0] });
  return ground;
}

export function buildExplosionDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const cylinder = runtime.createCylinder(EXPLOSION_CYLINDER_HEIGHT, EXPLOSION_CYLINDER_RADIUS, 0, 15);
  forEachExplosionCylinder((position) => {
    const body = world.createBody({ type: BodyType.Dynamic, position });
    runtime.createShapeFromHull(body, cylinder, { explosionScale: 2 });
    handles.push(body);
  });
  runtime.destroyHull(cylinder);
  return handles;
}

export function createExplosion(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = buildExplosionGround(world, runtime);
  return { world, handles: [ground, ...buildExplosionDynamicBodies(world, runtime)] };
}

export function explosionGroundSize(): Vec3 {
  return [20, 0.5, 20];
}

export const explosionCamera: RenderSpec["camera"] = cameraFromSetView(45, 20, 30, [0, 0, 0]);

export const dumpSampleName = "Explosion";
export const dumpSampleId = "benchmark/explosion";
export const dumpCppSampleName = "Explosion";
export const dumpCreate = createExplosion;

/** Upstream `Explode()` defaults: radius 16, position (0,-4,0), impulse 1000, falloff 0. */
export const dumpInteractionSchedule = [
  { frame: 1, action: "explode", args: [0, -4, 0, 16, 0, EXPLOSION_IMPULSE] },
] as const;

export function dumpRunInteraction(
  world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly number[],
  interaction: { action: string; args?: readonly number[] },
): void {
  if (interaction.action !== "explode") throw new Error(`Unsupported explosion dump action: ${interaction.action}`);
  const [x = 0, y = -4, z = 0, radius = 16, falloff = 0, impulsePerArea = EXPLOSION_IMPULSE] = interaction.args ?? [];
  world.explode([x, y, z], radius, falloff, impulsePerArea, 0xFFFFFFFFn as unknown as number);
}
