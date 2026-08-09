import {BodyType, type BodyId, type Box3DRuntime, type JointId, type PhysicsWorld, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const BOX_HALF: Vec3 = [1, 1, 0.5];
const BOX_SIZE: Vec3 = [2 * BOX_HALF[0], 2 * BOX_HALF[1], 2 * BOX_HALF[2]];
const START_POSITION: Vec3 = [-12.5, 10, 0];
const FORCE_THRESHOLD = 3000;
const TORQUE_THRESHOLD = 10000;
const DYNAMIC_COLOR = 0x60a5fa;

const jointOptions = {
  forceThreshold: FORCE_THRESHOLD,
  torqueThreshold: TORQUE_THRESHOLD,
  collideConnected: true,
} as const;

export interface JointEventScene {
  anchor: BodyId;
  bodies: BodyId[];
  joints: JointId[];
}

function createJointBox(world: PhysicsWorld, runtime: Box3DRuntime, position: Vec3): BodyId {
  const body = world.createBody({
    type: BodyType.Dynamic,
    position,
    enableSleep: false,
  });
  runtime.createHullShape(body, BOX_HALF, { density: 1 });
  return body;
}

export function createJointEventScene(world: PhysicsWorld, runtime: Box3DRuntime): JointEventScene {
  const anchor = world.createBody({ type: BodyType.Static });
  const bodies: BodyId[] = [];
  const joints: JointId[] = [];

  let position: Vec3 = [...START_POSITION];

  {
    const body = createJointBox(world, runtime, position);
    bodies.push(body);
    const length = 2;
    const pivotA: Vec3 = [position[0], position[1] + 1 + length, 0];
    const pivotB: Vec3 = [position[0], position[1] + 1, 0];
    joints.push(world.createDistanceJoint(anchor, body, {
      localFrameA: { position: world.getBodyLocalPoint(anchor, pivotA) },
      localFrameB: { position: world.getBodyLocalPoint(body, pivotB) },
      length,
      ...jointOptions,
    }));
  }

  position = [position[0] + 5, position[1], position[2]];
  // Motor joint slot is disabled upstream (#if 0), but it still advances position.x.
  position = [position[0] + 5, position[1], position[2]];

  {
    const body = createJointBox(world, runtime, position);
    bodies.push(body);
    const pivot: Vec3 = [position[0] - 1, position[1], 0];
    joints.push(world.createPrismaticJoint(anchor, body, {
      localFrameA: { position: world.getBodyLocalPoint(anchor, pivot) },
      localFrameB: { position: world.getBodyLocalPoint(body, pivot) },
      ...jointOptions,
    }));
  }

  position = [position[0] + 5, position[1], position[2]];

  {
    const body = createJointBox(world, runtime, position);
    bodies.push(body);
    const pivot: Vec3 = [position[0] - 1, position[1], 0];
    joints.push(world.createRevoluteJoint(anchor, body, {
      localFrameA: { position: world.getBodyLocalPoint(anchor, pivot) },
      localFrameB: { position: world.getBodyLocalPoint(body, pivot) },
      ...jointOptions,
    }));
  }

  position = [position[0] + 5, position[1], position[2]];

  {
    const body = createJointBox(world, runtime, position);
    bodies.push(body);
    const pivot: Vec3 = [position[0] - 1, position[1], 0];
    joints.push(world.createWeldJoint(anchor, body, {
      localFrameA: { position: world.getBodyLocalPoint(anchor, pivot) },
      localFrameB: { position: world.getBodyLocalPoint(body, pivot) },
      angularHertz: 2,
      angularDampingRatio: 0.5,
      ...jointOptions,
    }));
  }

  position = [position[0] + 5, position[1], position[2]];
  // Wheel joint slot is disabled upstream (#if 0), leaving four active joints total.
  void position;

  return { anchor, bodies, joints };
}

export function buildJointEventDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const scene = createJointEventScene(world, runtime);
  return [scene.anchor, ...scene.bodies];
}

export function jointEventGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const jointEventBodies: RenderBody[] = [
  { kind: "box", size: BOX_SIZE, position: [-12.5, 10, 0], color: DYNAMIC_COLOR },
  { kind: "box", size: BOX_SIZE, position: [-2.5, 10, 0], color: DYNAMIC_COLOR },
  { kind: "box", size: BOX_SIZE, position: [2.5, 10, 0], color: DYNAMIC_COLOR },
  { kind: "box", size: BOX_SIZE, position: [7.5, 10, 0], color: DYNAMIC_COLOR },
];

export const jointEventCamera: RenderSpec["camera"] = cameraFromSetView(0, 30, 40, [0, 5, 0]);

export const dumpSampleName = "Joint";
export const dumpSampleId = "events/joint";
export const dumpCppSampleName = "Joint";
export const dumpGroundSize = jointEventGroundSize;
export const dumpBuildDynamicBodies = buildJointEventDynamicBodies;

export function dumpPostStep(
  world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly BodyId[],
  _frame: number,
  _dt: number,
  _state: unknown,
): void {
  for (const joint of world.getJointEventHandles()) {
    world.destroyJoint(joint);
  }
}
