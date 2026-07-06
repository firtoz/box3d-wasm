import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";

export const weldJointBodyIndex = 1;

export function buildWeldJointDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const hiddenGround = objectWorld.createBody({ position: [0, -1, 0] });
  const body = objectWorld.createBody({ type: BodyType.Dynamic, position: [0, 4, 0], gravityScale: 0 });
  body.createHullShape([0.5, 1.5, 0.25]);
  objectWorld.createWeldJoint(hiddenGround, body, {
    localFrameA: { position: [0, 6.5, 0] },
    localFrameB: { position: [0, 1.5, 0] },
    linearHertz: 0,
    linearDampingRatio: 0,
    angularHertz: 2,
    angularDampingRatio: 0.7,
  });
  return [hiddenGround.handle, body.handle];
}

export const weldJointGroundSize = (): Vec3 => [20, 1, 20];
export const weldJointVisibleBodies = [
  { index: weldJointBodyIndex, size: [0.5, 1.5, 0.25], position: [0, 4, 0], color: 0x38bdf8 },
] as const;
export const weldJointCamera = { position: [12, 10, 18] as [number, number, number], target: [0, 2, 0] as [number, number, number] };

export const dumpSampleName = "Weld";
export const dumpSampleId = "joints/weld";
export const dumpCppSampleName = "Weld";
export const dumpGroundSize = weldJointGroundSize;
export const dumpBuildDynamicBodies = buildWeldJointDynamicBodies;
