import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
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
