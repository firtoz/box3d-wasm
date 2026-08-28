import {BodyType, type BodyId, type Box3DRuntime, type JointId, type PhysicsWorld, type Vec3} from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";

export const doorBodyIndex = 0;
export const doorLowerJointIndex = 0;
export const doorUpperJointIndex = 1;

export interface DoorScene {
  handles: BodyId[];
  joints: JointId[];
}

export function createDoorJoints(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  groundHandle: BodyId,
  doorHandle: BodyId,
  options: { twoJoints: boolean; constraintHertz: number; constraintDampingRatio: number },
): JointId[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const ground = objectWorld.body(groundHandle);
  const door = objectWorld.body(doorHandle);
  const axisQuat = runtime.makeQuatFromAxisAngle([1, 0, 0], -Math.PI / 2);
  const joints: JointId[] = [];
  joints.push(objectWorld.createRevoluteJoint(ground, door, {
    localFrameA: { position: [-0.75, 1, 0], rotation: axisQuat },
    localFrameB: { position: [-0.75, -1.5, 0], rotation: axisQuat },
    constraintHertz: options.constraintHertz,
    constraintDampingRatio: options.constraintDampingRatio,
    enableLimit: true,
    lowerAngle: -0.5 * Math.PI,
    upperAngle: 0.5 * Math.PI,
    enableSpring: true,
    hertz: 1,
    dampingRatio: 0.5,
    enableMotor: false,
    maxMotorTorque: 100,
    motorSpeed: 0,
  }).handle);
  if (options.twoJoints) {
    joints.push(objectWorld.createRevoluteJoint(ground, door, {
      localFrameA: { position: [-0.75, 4, 0], rotation: axisQuat },
      localFrameB: { position: [-0.75, 1.5, 0], rotation: axisQuat },
      constraintHertz: options.constraintHertz,
      constraintDampingRatio: options.constraintDampingRatio,
      enableLimit: true,
      lowerAngle: -0.5 * Math.PI,
      upperAngle: 0.5 * Math.PI,
      enableSpring: true,
      hertz: 1,
      dampingRatio: 0.5,
      enableMotor: false,
      maxMotorTorque: 100,
      motorSpeed: 0,
    }).handle);
  }
  return joints;
}

export function createDoorScene(world: PhysicsWorld, runtime: Box3DRuntime, groundHandle: BodyId): DoorScene {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const door = objectWorld.createBody({ type: BodyType.Dynamic, position: [0, 1.5, 0], gravityScale: 2 });
  door.createHullShape([0.75, 1.5, 0.1], { density: 1000 });
  const joints = createDoorJoints(world, runtime, groundHandle, door.handle, {
    twoJoints: true,
    constraintHertz: 120,
    constraintDampingRatio: 0,
  });
  return { handles: [door.handle], joints };
}

export function buildDoorDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime, groundHandle: BodyId): BodyId[] {
  return createDoorScene(world, runtime, groundHandle).handles;
}

export const doorGroundSize = (): Vec3 => [20, 1, 20];
export const doorVisibleBodies = [
  { index: doorBodyIndex, size: [0.75, 1.5, 0.1], position: [0, 1.5, 0], color: 0x38bdf8 },
] as const;
export const doorCamera = { position: [12, 10, 18] as [number, number, number], target: [0, 2, 0] as [number, number, number] };

export const dumpSampleName = "Door";
export const dumpSampleId = "joints/door";
export const dumpCppSampleName = "Door";
export const dumpInteractionSchedule = [
  { frame: 1, action: "impulse", args: [50000] },
] as const;

export function dumpRunInteraction(world: PhysicsWorld, _runtime: Box3DRuntime, handles: readonly BodyId[], interaction: { action: string; args?: readonly number[] }): void {
  if (interaction.action !== "impulse") throw new Error(`Unsupported door dump action: ${interaction.action}`);
  const doorHandle = handles[1]!;
  world.applyLinearImpulse(doorHandle, [0, 0, -(interaction.args?.[0] ?? 50000)], world.getBodyWorldPoint(doorHandle, [0.75, 0, 0]), true);
}

export const dumpCreate = (runtime: Box3DRuntime) => {
  const world = runtime.createWorld({ gravity: [0, -10, 0] });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  world.createHullShape(ground, [20, 1, 20]);
  return { world, handles: [ground, ...buildDoorDynamicBodies(world, runtime, ground)] };
};
