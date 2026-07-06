import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";

export const doorBodyIndex = 0;

export function buildDoorDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime, groundHandle: BodyHandle): BodyHandle[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const ground = objectWorld.body(groundHandle);
  const axisQuat = runtime.makeQuatFromAxisAngle([1, 0, 0], -Math.PI / 2);
  const door = objectWorld.createBody({ type: BodyType.Dynamic, position: [0, 1.5, 0], gravityScale: 2 });
  door.createHullShape([0.75, 1.5, 0.1], { density: 1000 });
  objectWorld.createRevoluteJoint(ground, door, {
    localFrameA: { position: [-0.75, 1, 0], rotation: axisQuat },
    localFrameB: { position: [-0.75, -1.5, 0], rotation: axisQuat },
    constraintHertz: 120,
    constraintDampingRatio: 0,
    enableLimit: true,
    lowerAngle: -0.5 * Math.PI,
    upperAngle: 0.5 * Math.PI,
    enableSpring: true,
    hertz: 1,
    dampingRatio: 0.5,
    enableMotor: false,
    maxMotorTorque: 100,
    motorSpeed: 0,
  });
  objectWorld.createRevoluteJoint(ground, door, {
    localFrameA: { position: [-0.75, 4, 0], rotation: axisQuat },
    localFrameB: { position: [-0.75, 1.5, 0], rotation: axisQuat },
    constraintHertz: 120,
    constraintDampingRatio: 0,
    enableLimit: true,
    lowerAngle: -0.5 * Math.PI,
    upperAngle: 0.5 * Math.PI,
    enableSpring: true,
    hertz: 1,
    dampingRatio: 0.5,
    enableMotor: false,
    maxMotorTorque: 100,
    motorSpeed: 0,
  });
  return [door.handle];
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

export function dumpRunInteraction(world: PhysicsWorld, _runtime: Box3DRuntime, handles: readonly BodyHandle[], interaction: { action: string; args?: readonly number[] }): void {
  if (interaction.action !== "impulse") throw new Error(`Unsupported door dump action: ${interaction.action}`);
  const doorHandle = handles[1]!;
  world.applyLinearImpulse(doorHandle, [0, 0, -(interaction.args?.[0] ?? 50000)], world.getBodyWorldPoint(doorHandle, [0.75, 0, 0]), true);
}

export const dumpCreate = (runtime: Box3DRuntime) => {
  const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  world.createHullShape(ground, [20, 1, 20]);
  return { world, handles: [ground, ...buildDoorDynamicBodies(world, runtime, ground)] };
};
