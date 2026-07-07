import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32 } from "../f32";

export function buildStallScene(world: PhysicsWorld, runtime: Box3DRuntime): { handles: number[]; torusMesh: number; savedThreshold: number } {
  const handles: number[] = [];
  const savedThreshold = runtime.getStallThreshold();
  runtime.setStallThreshold(0.001);

  const torusMesh = world.createTorusMesh(200, 200, 2, 1);
  const torus = world.createBody({ type: BodyType.Static, position: [0, 2, 0] });
  world.createMeshShape(torus, torusMesh, { scale: [1, 1, 1] });
  handles.push(torus);

  const bullet = launchStallBullet(world, runtime);
  handles.push(bullet);

  return { handles, torusMesh, savedThreshold };
}

export function launchStallBullet(world: PhysicsWorld, runtime: Box3DRuntime, existing?: BodyHandle): BodyHandle {
  if (existing !== undefined) runtime.destroyBody(existing);
  const bullet = world.createBody({
    type: BodyType.Dynamic,
    isBullet: true,
    position: [0, 1, -10],
    linearVelocity: [0, 0, 600],
    angularVelocity: [0, 0, 20],
  });
  const rock = runtime.createRock(0.25);
  runtime.createShapeFromHull(bullet, rock, {});
  runtime.destroyHull(rock);
  return bullet;
}

export function buildStallDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  return buildStallScene(world, runtime).handles;
}

export function stallGroundSize(): Vec3 { return [500, 1, 500]; }

export const stallBodies: RenderBody[] = [
  { kind: "sphere", radius: 0.25, position: [0, 1, -10], color: 0x78716c },
];

export const stallCamera: RenderSpec["camera"] = { position: [130, 15, 15], target: [0, 2, 0] };

export const dumpSampleName = "Stall";
export const dumpSampleId = "continuous/stall";
export const dumpCppSampleName = "Stall";
export const dumpGroundSize = stallGroundSize;

interface StallDumpState {
  bullet?: BodyHandle;
  savedThreshold: number;
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[]; state: StallDumpState } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, stallGroundSize(), {});
  const { handles, savedThreshold } = buildStallScene(world, runtime);
  return { world, handles: [ground, ...handles], state: { bullet: handles[1], savedThreshold } };
}
