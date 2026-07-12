import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Mul } from "../f32";

const BOX_HALF = 0.5;
const BOX_COUNT = 6;
const BOX_COLORS = [0x60a5fa, 0x34d399, 0xfbbf24, 0xf472b6, 0xa78bfa, 0xfb7185] as const;

export function buildMultiplePrismaticDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime, ground: BodyHandle): BodyHandle[] {
  const handles: BodyHandle[] = [];
  let bodyA: BodyHandle = ground;
  let localFrameA: Vec3 = [0, 0, 0];

  for (let i = 0; i < BOX_COUNT; i++) {
    const y = f32Add(f32(0.6), f32Mul(f32(1.2), i));
    const body = world.createBody({ type: BodyType.Dynamic, position: [0, y, 0] });
    runtime.createHullShape(body, [BOX_HALF, BOX_HALF, BOX_HALF]);
    world.createPrismaticJoint(bodyA, body, {
      localFrameA: { position: localFrameA },
      localFrameB: { position: [0, -0.6, 0] },
      constraintHertz: 240,
      enableLimit: true,
      lowerTranslation: -6,
      upperTranslation: 6,
    });
    handles.push(body);
    bodyA = body;
    localFrameA = [0, 0.6, 0];
  }
  return handles;
}

export function createMultiplePrismatic(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyHandle[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  return { world, handles: [ground, ...buildMultiplePrismaticDynamicBodies(world, runtime, ground)] };
}

export function multiplePrismaticGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createMultiplePrismaticBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < BOX_COUNT; i++) {
    const y = f32Add(f32(0.6), f32Mul(f32(1.2), i));
    bodies.push({
      kind: "box",
      size: [2 * BOX_HALF, 2 * BOX_HALF, 2 * BOX_HALF],
      position: [0, y, 0],
      color: BOX_COLORS[i]!,
    });
  }
  return bodies;
}

export const multiplePrismaticCamera: RenderSpec["camera"] = cameraFromSetView(0, 0, 25, [0, 5, 0]);

export const dumpSampleName = "Multiple Prismatic";
export const dumpSampleId = "issues/multiple-prismatic";
export const dumpCppSampleName = "Multiple Prismatic";
export const dumpCreate = createMultiplePrismatic;
