import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";

export function buildCapsuleStackDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  for (let i = 0, y = 0.75; i < 20; i++, y += 1) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [0, y, 0], isAwake: true });
    runtime.setBodyMotionLocks(body, { lockLinearZ: true, lockRotationX: true, lockRotationY: true, lockRotationZ: true });
    runtime.createCapsuleShape(body, [-1, 0, 0], [1, 0, 0], 0.5);
    handles.push(body);
  }
  return handles;
}

export function capsuleStackGroundSize(): Vec3 {
  return [40, 1, 40];
}

export const dumpSampleName = "Capsule Stack";
export const dumpSampleId = "capsule-stack";
export const dumpCppSampleName = "Capsule Stack";
export const dumpGroundSize = capsuleStackGroundSize;
export const dumpBuildDynamicBodies = buildCapsuleStackDynamicBodies;
