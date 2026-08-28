import {BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type ShapeId, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32 } from "../f32";

export const WIND_MAX_COUNT = 60;
export const WIND_DEFAULT_COUNT = 10;
export const WIND_RADIUS = f32(0.1);
export const WIND_VERTICAL_OFFSET = f32(2);
export const WIND_DEFAULT: Vec3 = [f32(6), 0, 0];
export const WIND_DEFAULT_DRAG = f32(1);
export const WIND_DEFAULT_LIFT = f32(0.75);
const NOISE_ALPHA = f32(0.05);

export type WindShapeType = "sphere" | "capsule" | "box";

export interface WindState {
  shapeIds: ShapeId[];
  noise: Vec3;
  wind: Vec3;
  drag: number;
  lift: number;
}

export function buildWindDynamicBodies(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  options: { count?: number; shapeType?: WindShapeType } = {},
): { handles: BodyId[]; state: WindState } {
  const count = options.count ?? WIND_DEFAULT_COUNT;
  const shapeType = options.shapeType ?? "box";
  const handles: BodyId[] = [];
  const shapeIds: ShapeId[] = [];
  const radius = WIND_RADIUS;

  const anchor = world.createBody({ type: BodyType.Static });
  handles.push(anchor);

  let jointBodyA = anchor;
  for (let i = 0; i < count; i++) {
    const body = world.createBody({
      type: BodyType.Dynamic,
      position: [f32(f32(2 * i + 1) * radius), WIND_VERTICAL_OFFSET, 0],
      gravityScale: f32(0.5),
      enableSleep: false,
    });
    if (shapeType === "sphere") {
      runtime.createSphereShape(body, [0, 0, 0], radius, { density: 20 });
    } else if (shapeType === "capsule") {
      runtime.createCapsuleShape(body, [-radius, 0, 0], [radius, 0, 0], f32(0.5 * radius), { density: 20 });
    } else {
      runtime.createHullShape(body, [f32(1.25 * radius), f32(0.75 * radius), f32(0.125 * radius)], { density: 20 });
    }
    const shapes = world.getBodyShapes(body);
    if (shapes[0] !== undefined) shapeIds.push(shapes[0]);
    handles.push(body);

    world.createSphericalJoint(jointBodyA, body, {
      localFrameA: { position: jointBodyA === anchor ? [0, WIND_VERTICAL_OFFSET, 0] : [radius, 0, 0] },
      localFrameB: { position: [-radius, 0, 0] },
    });
    jointBodyA = body;
  }

  return {
    handles,
    state: {
      shapeIds,
      noise: [0, 0, 0],
      wind: [...WIND_DEFAULT],
      drag: WIND_DEFAULT_DRAG,
      lift: WIND_DEFAULT_LIFT,
    },
  };
}

export function windGroundSize(): Vec3 { return [20, 1, 20]; }

export function createWindBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < WIND_DEFAULT_COUNT; i++) {
    bodies.push({
      kind: "box",
      size: [2.5 * WIND_RADIUS, 1.5 * WIND_RADIUS, 0.25 * WIND_RADIUS],
      position: [(2 * i + 1) * WIND_RADIUS, WIND_VERTICAL_OFFSET, 0],
      color: 0x60a5fa,
    });
  }
  return bodies;
}

export const windCamera: RenderSpec["camera"] = { position: [0, 0, 5], target: [0, 1, 0] };

export const dumpSampleName = "Wind";
export const dumpSampleId = "shapes/wind";
export const dumpCppSampleName = "Wind";

export function createWind(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[]; state: WindState } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, windGroundSize(), {});
  const built = buildWindDynamicBodies(world, runtime);
  return { world, handles: [ground, ...built.handles], state: built.state };
}

export const dumpCreate = createWind;

export function applyWindForces(runtime: Box3DRuntime, state: WindState): Vec3 {
  const { length: speed, direction } = runtime.getLengthAndNormalize(state.wind);
  const windVec: Vec3 = [
    f32(speed * f32(direction[0] + state.noise[0])),
    f32(speed * f32(direction[1] + state.noise[1])),
    f32(speed * f32(direction[2] + state.noise[2])),
  ];
  for (const shapeId of state.shapeIds) {
    runtime.applyShapeWind(shapeId, windVec, state.drag, state.lift, 10, true);
  }
  const rand = runtime.randomVec3([-0.3, -0.3, -0.3], [0.3, 0.3, 0.3]);
  state.noise = runtime.lerpVec3(state.noise, rand, NOISE_ALPHA);
  return windVec;
}

export function dumpPostStep(_world: PhysicsWorld, runtime: Box3DRuntime, _handles: readonly BodyId[], _frame: number, _dt: number, state: unknown): void {
  applyWindForces(runtime, state as WindState);
}
