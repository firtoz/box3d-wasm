import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";
import type { RenderBody, RenderSpec } from "../generic-host";

const BOX_HALF: Vec3 = [1, 1, 0.5];
const FORCE_THRESHOLD = 20000;
const TORQUE_THRESHOLD = 10000;
const JOINT_OPTS = { forceThreshold: FORCE_THRESHOLD, torqueThreshold: TORQUE_THRESHOLD, collideConnected: true } as const;

export function buildMotionLocksDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const anchor = objectWorld.createBody();
  const handles: BodyHandle[] = [anchor.handle];

  const positions = [-12.5, -7.5, -2.5, 2.5] as const;
  for (let index = 0; index < positions.length; index++) {
    const x = positions[index]!;
    const body = objectWorld.createBody({ type: BodyType.Dynamic, position: [x, 10, 0], enableSleep: false });
    body.createHullShape(BOX_HALF, { density: 1 });
    handles.push(body.handle);

    if (index === 0) {
      objectWorld.createDistanceJoint(anchor, body, {
        localFrameA: { position: anchor.getLocalPointXYZ(x, 13, 0) },
        localFrameB: { position: body.getLocalPointXYZ(x, 11, 0) },
        length: 2,
        ...JOINT_OPTS,
      });
    } else if (index === 1) {
      const pivot = anchor.getLocalPointXYZ(x - 1, 10, 0);
      objectWorld.createPrismaticJoint(anchor, body, {
        localFrameA: { position: pivot },
        localFrameB: { position: body.getLocalPointXYZ(x - 1, 10, 0) },
        ...JOINT_OPTS,
      });
    } else if (index === 2) {
      const pivot = anchor.getLocalPointXYZ(x - 1, 10, 0);
      objectWorld.createRevoluteJoint(anchor, body, {
        localFrameA: { position: pivot },
        localFrameB: { position: body.getLocalPointXYZ(x - 1, 10, 0) },
        ...JOINT_OPTS,
      });
    } else {
      const pivot = anchor.getLocalPointXYZ(x - 1, 10, 0);
      objectWorld.createWeldJoint(anchor, body, {
        localFrameA: { position: pivot },
        localFrameB: { position: body.getLocalPointXYZ(x - 1, 10, 0) },
        angularHertz: 2,
        angularDampingRatio: 0.5,
        ...JOINT_OPTS,
      });
    }
  }

  return handles;
}

export function motionLocksGroundSize(): Vec3 { return [20, 1, 20]; }

export const motionLocksBodies: RenderBody[] = [
  { kind: "box", size: [2, 2, 1], position: [-12.5, 10, 0], color: 0x38bdf8 },
  { kind: "box", size: [2, 2, 1], position: [-7.5, 10, 0], color: 0x60a5fa },
  { kind: "box", size: [2, 2, 1], position: [-2.5, 10, 0], color: 0x818cf8 },
  { kind: "box", size: [2, 2, 1], position: [2.5, 10, 0], color: 0xa78bfa },
];

export const motionLocksCamera: RenderSpec["camera"] = { position: [0, 30, 40], target: [0, 5, 0] };

export const dumpSampleName = "Motion Locks";
export const dumpSampleId = "joints/motion-locks";
export const dumpCppSampleName = "Motion Locks";
export const dumpGroundSize = motionLocksGroundSize;
export const dumpBuildDynamicBodies = buildMotionLocksDynamicBodies;

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyHandle[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, motionLocksGroundSize(), {});
  return { world, handles: [ground, ...buildMotionLocksDynamicBodies(world, runtime)] };
}
