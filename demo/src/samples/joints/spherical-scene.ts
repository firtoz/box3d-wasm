import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";

export const sphericalJointBodyIndex = 1;

export function buildSphericalJointDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const hiddenGround = objectWorld.createBody({ position: [0, -1, 0] });
  const body = objectWorld.createBody({ type: BodyType.Dynamic, position: [0, 4, 0], gravityScale: 0 });
  body.createHullShape([0.5, 1.5, 0.25], { density: 100 });
  objectWorld.createSphericalJoint(hiddenGround, body, {
    localFrameA: { position: [0, 6.5, 0] },
    localFrameB: { position: [0, 1.5, 0] },
    enableConeLimit: false,
    coneAngle: 30 * Math.PI / 180,
    enableTwistLimit: false,
    lowerTwistAngle: -35 * Math.PI / 180,
    upperTwistAngle: 35 * Math.PI / 180,
    enableSpring: true,
    hertz: 2,
    dampingRatio: 0.7,
    enableMotor: false,
    maxMotorTorque: 20,
    motorVelocity: [0, 0, 0],
  });
  return [hiddenGround.handle, body.handle];
}

export const sphericalJointGroundSize = (): Vec3 => [20, 1, 20];
export const sphericalJointVisibleBodies = [
  { index: sphericalJointBodyIndex, size: [0.5, 1.5, 0.25], position: [0, 4, 0], color: 0x38bdf8 },
] as const;
export const sphericalJointCamera = { position: [12, 10, 18] as [number, number, number], target: [0, 2, 0] as [number, number, number] };

export const dumpSampleName = "Spherical";
export const dumpSampleId = "joints/spherical";
export const dumpCppSampleName = "Spherical";
export const dumpGroundSize = sphericalJointGroundSize;
export const dumpBuildDynamicBodies = buildSphericalJointDynamicBodies;
