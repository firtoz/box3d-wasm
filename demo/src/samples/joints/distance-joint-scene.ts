import {BodyType, type BodyId, type Box3DRuntime, type JointId, type PhysicsWorld, type Vec3} from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";
import type { RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Mul } from "../f32";

export const DISTANCE_JOINT_MAX_COUNT = 20;
export const DISTANCE_JOINT_Y_OFFSET = f32(20);
export const DISTANCE_JOINT_RADIUS = f32(0.25);
const LENGTH = f32(1);
const DENSITY = f32(20);

export type DistanceJointTuning = {
  count: number;
  length: number;
  hertz: number;
  dampingRatio: number;
  tensionForce: number;
  compressionForce: number;
  minLength: number;
  maxLength: number;
  enableSpring: boolean;
  enableLimit: boolean;
};

export function defaultDistanceJointTuning(): DistanceJointTuning {
  return {
    count: 1,
    length: LENGTH,
    hertz: 5,
    dampingRatio: 0.5,
    tensionForce: 2000,
    compressionForce: 100,
    minLength: LENGTH,
    maxLength: LENGTH,
    enableSpring: false,
    enableLimit: false,
  };
}

export function applyDistanceJointTuning(world: PhysicsWorld, joints: readonly JointId[], tuning: DistanceJointTuning): void {
  for (const joint of joints) {
    world.setDistanceJointLength(joint, tuning.length);
    world.enableDistanceSpring(joint, tuning.enableSpring);
    world.setDistanceSpringForceRange(joint, -tuning.tensionForce, tuning.compressionForce);
    world.setDistanceSpringHertz(joint, tuning.hertz);
    world.setDistanceSpringDampingRatio(joint, tuning.dampingRatio);
    world.enableDistanceLimit(joint, tuning.enableLimit);
    world.setDistanceLengthRange(joint, tuning.minLength, tuning.maxLength);
    world.wakeJointBodies(joint);
  }
}

export function createDistanceJointChain(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  tuning: DistanceJointTuning,
  existingAnchor?: BodyId,
): { anchor: BodyId; bodies: BodyId[]; joints: JointId[] } {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const anchor = existingAnchor === undefined ? objectWorld.createBody() : objectWorld.body(existingAnchor);
  const bodies: BodyId[] = [];
  const joints: JointId[] = [];
  let prev = anchor;
  for (let i = 0; i < tuning.count; i++) {
    const x = f32Mul(tuning.length, f32Add(f32(i), f32(1)));
    const body = objectWorld.createBody({
      type: BodyType.Dynamic,
      position: [x, DISTANCE_JOINT_Y_OFFSET, 0],
      angularDamping: 1,
    });
    body.createSphereShape([0, 0, 0], DISTANCE_JOINT_RADIUS, { density: DENSITY });
    const pivotA = prev.getLocalPointXYZ(f32Mul(tuning.length, f32(i)), DISTANCE_JOINT_Y_OFFSET, 0);
    const pivotB = body.getLocalPointXYZ(x, DISTANCE_JOINT_Y_OFFSET, 0);
    const joint = objectWorld.createDistanceJoint(prev, body, {
      localFrameA: { position: pivotA },
      localFrameB: { position: pivotB },
      length: tuning.length,
    });
    bodies.push(body.handle);
    joints.push(joint.handle);
    prev = body;
  }
  applyDistanceJointTuning(world, joints, tuning);
  return { anchor: anchor.handle, bodies, joints };
}

export function buildDistanceJointDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const anchor = objectWorld.createBody();
  const body = objectWorld.createBody({
    type: BodyType.Dynamic,
    position: [LENGTH, DISTANCE_JOINT_Y_OFFSET, 0],
    angularDamping: 1,
  });
  body.createSphereShape([0, 0, 0], DISTANCE_JOINT_RADIUS, { density: DENSITY });
  const pivotA = anchor.getLocalPointXYZ(0, DISTANCE_JOINT_Y_OFFSET, 0);
  const pivotB = body.getLocalPointXYZ(LENGTH, DISTANCE_JOINT_Y_OFFSET, 0);
  objectWorld.createDistanceJoint(anchor, body, {
    localFrameA: { position: pivotA },
    localFrameB: { position: pivotB },
    length: LENGTH,
  });
  return [anchor.handle, body.handle];
}

export function distanceJointGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const distanceJointCamera: RenderSpec["camera"] = cameraFromSetView(0, 0, 40, [0, 10, 0]);

export const dumpSampleName = "Distance Joint";
export const dumpSampleId = "joints/distance-joint";
export const dumpCppSampleName = "Distance Joint";
export const dumpGroundSize = distanceJointGroundSize;
export const dumpBuildDynamicBodies = buildDistanceJointDynamicBodies;
