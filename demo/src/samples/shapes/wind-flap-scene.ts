import { B3_AXIS_X, B3_PI, BodyType, type BodyHandle, type Box3DRuntime, type JointHandle, type PhysicsWorld, type ShapeId, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32 } from "../f32";

const A = f32(0.4);
const Y = f32(20);
const DRAG = f32(1);
const LIFT = f32(2);
const wingAngle = f32(0.1);
const limit = f32(-30 * B3_PI / 180);

export interface WindFlapState {
  shapeId1: ShapeId;
  shapeId2: ShapeId;
  jointId1: JointHandle;
  jointId2: JointHandle;
  time: number;
}

export function buildWindFlapDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): { handles: number[]; state: WindFlapState } {
  const handles: number[] = [];
  const wingRotation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, wingAngle);

  const wing1 = world.createBody({ type: BodyType.Dynamic, position: [f32(-2 * A), Y, 0] });
  runtime.createTransformedHullShape(wing1, [f32(2 * A), f32(0.01), A], { rotation: wingRotation }, [1, 1, 1], { density: 5 });
  const wing1Shapes = world.getBodyShapes(wing1);
  handles.push(wing1);

  const wing2 = world.createBody({ type: BodyType.Dynamic, position: [f32(2 * A), Y, 0] });
  runtime.createTransformedHullShape(wing2, [f32(2 * A), f32(0.01), A], { rotation: wingRotation }, [1, 1, 1], { density: 5 });
  const wing2Shapes = world.getBodyShapes(wing2);
  handles.push(wing2);

  const torso = world.createBody({ type: BodyType.Dynamic, position: [0, Y, 0] });
  runtime.createCapsuleShape(torso, [0, 0, -A], [0, 0, A], f32(0.25 * A), { density: 10 });
  handles.push(torso);

  const jointId1 = world.createRevoluteJoint(torso, wing1, {
    localFrameA: { position: [0, 0, 0] },
    localFrameB: { position: [f32(2 * A), 0, 0] },
    enableSpring: true,
    hertz: 6,
    dampingRatio: f32(0.5),
    enableLimit: true,
    lowerAngle: limit,
    upperAngle: f32(-limit),
  });
  const jointId2 = world.createRevoluteJoint(torso, wing2, {
    localFrameA: { position: [0, 0, 0] },
    localFrameB: { position: [f32(-2 * A), 0, 0] },
    enableSpring: true,
    hertz: 6,
    dampingRatio: f32(0.5),
    enableLimit: true,
    lowerAngle: limit,
    upperAngle: f32(-limit),
  });
  world.createFilterJoint(wing1, wing2);

  return {
    handles,
    state: {
      shapeId1: wing1Shapes[0]!,
      shapeId2: wing2Shapes[0]!,
      jointId1,
      jointId2,
      time: 0,
    },
  };
}

export function windFlapGroundSize(): Vec3 { return [50, 1, 50]; }

export function createWindFlapBodies(): RenderBody[] {
  return [
    { kind: "box", size: [4 * A, 0.02, 2 * A], position: [-2 * A, Y, 0], color: 0x60a5fa },
    { kind: "box", size: [4 * A, 0.02, 2 * A], position: [2 * A, Y, 0], color: 0x34d399 },
    { kind: "capsule", axis: "z", radius: 0.25 * A, length: 2 * A, position: [0, Y, 0], color: 0xf59e0b },
  ];
}

export const windFlapCamera: RenderSpec["camera"] = { position: [-35, 15, 65], target: [0, 5, 10] };

export const dumpSampleName = "Wind Flap";
export const dumpSampleId = "shapes/wind-flap";
export const dumpCppSampleName = "Wind Flap";

export function createWindFlap(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[]; state: WindFlapState } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, windFlapGroundSize(), {});
  const built = buildWindFlapDynamicBodies(world, runtime);
  return { world, handles: [ground, ...built.handles], state: built.state };
}

export const dumpCreate = createWindFlap;

export function dumpPostStep(_world: PhysicsWorld, runtime: Box3DRuntime, _handles: readonly BodyHandle[], _frame: number, dt: number, state: unknown): void {
  const s = state as WindFlapState;
  runtime.applyShapeWind(s.shapeId1, [0, 0, 0], DRAG, LIFT, 10, false);
  runtime.applyShapeWind(s.shapeId2, [0, 0, 0], DRAG, LIFT, 10, false);
  const angle = f32(runtime.b3wSin(f32(10 * s.time)));
  runtime.setRevoluteJointTargetAngle(s.jointId1, angle);
  runtime.setRevoluteJointTargetAngle(s.jointId2, f32(-angle));
  s.time = f32(s.time + dt);
}
