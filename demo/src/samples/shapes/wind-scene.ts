import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type ShapeId, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32 } from "../f32";

const COUNT = 10;
const RADIUS = f32(0.1);
const VERTICAL_OFFSET = f32(2);
const WIND: Vec3 = [f32(6), 0, 0];
const DRAG = f32(1);
const LIFT = f32(0.75);
const NOISE_ALPHA = f32(0.05);

export interface WindState {
  shapeIds: ShapeId[];
  noise: Vec3;
}

export function buildWindDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): { handles: number[]; state: WindState } {
  const handles: number[] = [];
  const shapeIds: ShapeId[] = [];

  const anchor = world.createBody({ type: BodyType.Static });
  handles.push(anchor);

  let jointBodyA = anchor;
  for (let i = 0; i < COUNT; i++) {
    const body = world.createBody({
      type: BodyType.Dynamic,
      position: [f32(f32(2 * i + 1) * RADIUS), VERTICAL_OFFSET, 0],
      gravityScale: f32(0.5),
      enableSleep: false,
    });
    runtime.createHullShape(body, [f32(1.25 * RADIUS), f32(0.75 * RADIUS), f32(0.125 * RADIUS)], { density: 20 });
    const shapes = world.getBodyShapes(body);
    if (shapes[0] !== undefined) shapeIds.push(shapes[0]);
    handles.push(body);

    world.createSphericalJoint(jointBodyA, body, {
      localFrameA: { position: jointBodyA === anchor ? [0, VERTICAL_OFFSET, 0] : [RADIUS, 0, 0] },
      localFrameB: { position: [-RADIUS, 0, 0] },
    });
    jointBodyA = body;
  }

  return { handles, state: { shapeIds, noise: [0, 0, 0] } };
}

export function windGroundSize(): Vec3 { return [20, 1, 20]; }

export function createWindBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < COUNT; i++) {
    bodies.push({
      kind: "box",
      size: [2.5 * RADIUS, 1.5 * RADIUS, 0.25 * RADIUS],
      position: [(2 * i + 1) * RADIUS, VERTICAL_OFFSET, 0],
      color: 0x60a5fa,
    });
  }
  return bodies;
}

export const windCamera: RenderSpec["camera"] = { position: [0, 0, 5], target: [0, 1, 0] };

export const dumpSampleName = "Wind";
export const dumpSampleId = "shapes/wind";
export const dumpCppSampleName = "Wind";

export function createWind(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[]; state: WindState } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, windGroundSize(), {});
  const built = buildWindDynamicBodies(world, runtime);
  return { world, handles: [ground, ...built.handles], state: built.state };
}

export const dumpCreate = createWind;

export function dumpPostStep(_world: PhysicsWorld, runtime: Box3DRuntime, _handles: readonly BodyHandle[], _frame: number, _dt: number, state: unknown): void {
  const s = state as WindState;
  const { length: speed, direction } = runtime.getLengthAndNormalize(WIND);
  const windVec: Vec3 = [
    f32(speed * f32(direction[0] + s.noise[0])),
    f32(speed * f32(direction[1] + s.noise[1])),
    f32(speed * f32(direction[2] + s.noise[2])),
  ];
  for (const shapeId of s.shapeIds) {
    runtime.applyShapeWind(shapeId, windVec, DRAG, LIFT, 10, true);
  }
  const rand = runtime.randomVec3([-0.3, -0.3, -0.3], [0.3, 0.3, 0.3]);
  s.noise = runtime.lerpVec3(s.noise, rand, NOISE_ALPHA);
}
