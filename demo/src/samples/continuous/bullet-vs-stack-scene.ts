import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32, f32Add, f32Mul } from "../f32";

const boxHalf: Vec3 = [f32(0.5), f32(0.5), f32(0.5)];

function stackBodyY(row: number): number {
  // Match upstream float32: 0.5f + 1.1f * row
  return f32Add(0.5, f32Mul(1.1, row));
}

function buildStack(world: PhysicsWorld, runtime: Box3DRuntime, handles: number[]): void {
  const wallBody = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  handles.push(wallBody);
  runtime.createTransformedHullShape(wallBody, [f32(0.1), f32(5), f32(10)], { position: [-1, 5, 0] });

  for (let row = 0; row < 10; row++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [0, stackBodyY(row), 0] });
    runtime.createHullShape(body, boxHalf, {});
    handles.push(body);
  }
}

export function launchBullet(world: PhysicsWorld, runtime: Box3DRuntime, handles: number[]): BodyHandle {
  const bullet = world.createBody({
    type: BodyType.Dynamic,
    isBullet: true,
    position: [f32(20.5), f32(5.5), 0],
    linearVelocity: [f32(-500), 0, 0],
  });
  runtime.createSphereShape(bullet, [0, 0, 0], f32(0.25), { density: 10000 });
  handles.push(bullet);
  return bullet;
}

export function buildBulletVsStackDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  buildStack(world, runtime, handles);
  return handles;
}

export function bulletVsStackGroundSize(): Vec3 { return [50, 1, 50]; }

export function createBulletVsStackBodies(): RenderBody[] {
  const bodies: RenderBody[] = [
    { kind: "box", size: [0.2, 10, 20], position: [-1, 5, 0], color: 0x94a3b8, type: BodyType.Static },
  ];
  for (let row = 0; row < 10; row++) {
    bodies.push({ kind: "box", size: [1, 1, 1], position: [0, stackBodyY(row), 0], color: 0x60a5fa });
  }
  return bodies;
}

export const bulletVsStackCamera: RenderSpec["camera"] = { position: [15, 20, 30], target: [0, 2, 0] };

export const dumpSampleName = "Bullet vs Stack";
export const dumpSampleId = "continuous/bullet-vs-stack";
export const dumpCppSampleName = "Bullet vs Stack";
export const dumpGroundSize = bulletVsStackGroundSize;

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, bulletVsStackGroundSize(), {});
  const handles = [ground, ...buildBulletVsStackDynamicBodies(world, runtime)];
  return { world, handles };
}

export const dumpInteractionSchedule = [{ frame: 1, action: "launch" }] as const;

export function dumpRunInteraction(world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly number[], interaction: { action: string }): void {
  if (interaction.action !== "launch") throw new Error(`Unsupported bullet-vs-stack dump action: ${interaction.action}`);
  launchBullet(world, runtime, handles as number[]);
}
