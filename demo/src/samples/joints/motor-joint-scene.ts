import { B3_AXIS_Z, BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";

export const motorJointTargetIndex = 1;
export const motorJointBodyIndex = 2;
export const motorJointSpringBodyIndex = 3;

export function buildMotorJointDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);

  const hiddenGround = objectWorld.createBody({ position: [0, -1, 0] });

  const target = objectWorld.createBody({ type: BodyType.Kinematic, position: [0, 10, 0] });

  const body = objectWorld.createBody({ type: BodyType.Dynamic, position: [0, 10, 0] });
  body.createHullShape([1, 0.25, 0.25]);
  objectWorld.createMotorJoint(target, body, {
    linearHertz: 4,
    linearDampingRatio: 0.7,
    angularHertz: 4,
    angularDampingRatio: 0.7,
    maxSpringForce: 400000,
    maxSpringTorque: 500000,
  });

  const springBody = objectWorld.createBody({ type: BodyType.Dynamic, position: [-2, 2, 0] });
  springBody.createHullShape([0.5, 0.5, 0.5]);
  objectWorld.createMotorJoint(hiddenGround, springBody, {
    localFrameA: [-1.75, 3.25, 0],
    localFrameB: [0.25, 0.25, 0],
    linearHertz: 7.5,
    linearDampingRatio: 0.7,
    angularHertz: 7.5,
    angularDampingRatio: 0.7,
    maxSpringForce: 200000,
    maxSpringTorque: 10000,
  });

  return [hiddenGround.handle, target.handle, body.handle, springBody.handle];
}

export const motorJointGroundSize = (): Vec3 => [20, 1, 20];
export const motorJointVisibleBodies = [
  { index: motorJointBodyIndex, size: [1, 0.25, 0.25], position: [0, 10, 0], color: 0x38bdf8 },
  { index: motorJointSpringBodyIndex, size: [0.5, 0.5, 0.5], position: [-2, 2, 0], color: 0xf97316 },
] as const;
export const motorJointCamera = { position: [0, 8, 25] as [number, number, number], target: [0, 8, 0] as [number, number, number] };

export const dumpSampleName = "Motor Joint";
export const dumpSampleId = "joints/motor-joint";
export const dumpCppSampleName = "Motor Joint";
export const dumpGroundSize = motorJointGroundSize;
export const dumpBuildDynamicBodies = buildMotorJointDynamicBodies;

interface MotorJointDumpState {
  speed: number;
  time: number;
}

export const dumpInteractionSchedule = [
  { frame: 100, action: "set-speed", args: [1] },
] as const;

export function dumpRunInteraction(_world: PhysicsWorld, _runtime: Box3DRuntime, _handles: readonly BodyHandle[], interaction: { action: string; args?: readonly number[] }, _frame: number, state: MotorJointDumpState): void {
  if (interaction.action !== "set-speed") throw new Error(`Unsupported motor-joint dump action: ${interaction.action}`);
  state.speed = Math.fround(interaction.args?.[0] ?? 0);
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyHandle[]; state: MotorJointDumpState } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, motorJointGroundSize());
  return { world, handles: [ground, ...buildMotorJointDynamicBodies(world, runtime)], state: { speed: 0, time: 0 } };
}

export function dumpStep(_world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly BodyHandle[], _frame: number, dt: number, state: MotorJointDumpState): void {
  if (dt <= 0) return;
  const timeStep = Math.fround(dt);
  state.time = Math.fround(state.time + Math.fround(state.speed * timeStep));
  const angularOffset = Math.fround(2 * state.time);
  const position: Vec3 = [Math.fround(6 * Math.sin(angularOffset)), Math.fround(10 + 4 * Math.sin(state.time)), 0];
  const rotation = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, angularOffset);
  runtime.setBodyTargetTransform(handles[motorJointTargetIndex + 1]!, position, rotation, timeStep, true);
}
