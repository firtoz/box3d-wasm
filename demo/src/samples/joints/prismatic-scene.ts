import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";

export const prismaticJointBodyIndex = 1;

export function buildPrismaticJointDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const hiddenGround = objectWorld.createBody({ position: [0, -1, 0] });
  const body = objectWorld.createBody({ type: BodyType.Dynamic, position: [0, 4, 0], gravityScale: 0 });
  body.createHullShape([0.5, 1.5, 0.25]);
  objectWorld.createPrismaticJoint(hiddenGround, body, {
    localFrameA: { position: [0, 6.5, 0] },
    localFrameB: { position: [0, 1.5, 0] },
    enableSpring: true,
    hertz: 2,
    dampingRatio: 0.7,
    targetTranslation: 0,
    enableMotor: false,
    maxMotorForce: 20,
    motorSpeed: 0,
    enableLimit: false,
    lowerTranslation: -1,
    upperTranslation: 1,
  });
  return [hiddenGround.handle, body.handle];
}

export const prismaticJointGroundSize = (): Vec3 => [20, 1, 20];
export const prismaticJointVisibleBodies = [
  { index: prismaticJointBodyIndex, size: [0.5, 1.5, 0.25], position: [0, 4, 0], color: 0x38bdf8 },
] as const;
export const prismaticJointCamera = { position: [12, 10, 18] as [number, number, number], target: [0, 2, 0] as [number, number, number] };

export const dumpSampleName = "Prismatic";
export const dumpSampleId = "joints/prismatic";
export const dumpCppSampleName = "Prismatic";
export const dumpGroundSize = prismaticJointGroundSize;
export const dumpBuildDynamicBodies = buildPrismaticJointDynamicBodies;
