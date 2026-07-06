import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";

export const revoluteJointBodyIndex = 1;

export function buildRevoluteJointDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const hiddenGround = objectWorld.createBody({ position: [0, -1, 0] });
  const body = objectWorld.createBody({ type: BodyType.Dynamic, position: [0, 4, 0] });
  body.createHullShape([0.5, 1.5, 0.25]);
  objectWorld.createRevoluteJoint(hiddenGround, body, {
    localFrameA: { position: [0, 6.5, 0] },
    localFrameB: { position: [0, 1.5, 0] },
    enableLimit: false,
    lowerAngle: -35 * Math.PI / 180,
    upperAngle: 35 * Math.PI / 180,
    enableSpring: false,
    hertz: 2,
    dampingRatio: 0.7,
    enableMotor: false,
    maxMotorTorque: 5000,
    motorSpeed: 0,
  });
  return [hiddenGround.handle, body.handle];
}

export const revoluteJointGroundSize = (): Vec3 => [20, 1, 20];
export const revoluteJointVisibleBodies = [
  { index: revoluteJointBodyIndex, size: [0.5, 1.5, 0.25], position: [0, 4, 0], color: 0x38bdf8 },
] as const;
export const revoluteJointCamera = { position: [12, 10, 18] as [number, number, number], target: [0, 2, 0] as [number, number, number] };

export const dumpSampleName = "Revolute";
export const dumpSampleId = "joints/revolute";
export const dumpCppSampleName = "Revolute";
export const dumpCreate = (runtime: Box3DRuntime) => {
  const world = runtime.createWorld({ gravity: [0, -9.81, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, revoluteJointGroundSize());
  return { world, handles: [ground, ...buildRevoluteJointDynamicBodies(world, runtime)] };
};
