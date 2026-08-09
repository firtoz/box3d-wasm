import {
  B3_AXIS_Z,
  B3_PI,
  BodyType,
  quatFromAxisAngle,
  type BodyHandle,
  type Box3DRuntime,
  type JointHandle,
  type MeshHandle,
  type PhysicsWorld,
  type Quat,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32, f32Mul } from "../f32";
import { cameraFromSetView } from "../shared";

/** Dump uses fixed launch speed 250 (upstream RandomFloatRange(200, 300)). */
export const SENSOR_HITS_LAUNCH_SPEED = f32(250);
export const SENSOR_HITS_GROUND_EXTENT = f32(10);

const WALL_HALF: Vec3 = [f32(0.1), f32(5), f32(5)];
const WALL_LOCAL: Vec3 = [f32(10), f32(5), 0];
const SENSOR_ROTATION_ANGLE = f32Mul(f32(0.5), B3_PI);
const GRID_CELL_COUNT = 2;
const GRID_CELL_WIDTH = f32(5);

export interface SensorHitsState {
  mesh: MeshHandle;
  wallBody: BodyHandle;
  staticSensorBody: BodyHandle;
  kinematicBody: BodyHandle;
  dynamicBody: BodyHandle;
  launchBody: BodyHandle;
  joint: JointHandle;
}

export function sensorHitsSensorRotation(runtime: Box3DRuntime): Quat {
  return runtime.makeQuatFromAxisAngle(B3_AXIS_Z, SENSOR_ROTATION_ANGLE);
}

export function buildSensorHitsDynamicBodies(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
): { handles: BodyHandle[]; state: SensorHitsState } {
  // Extra wall hull on a dedicated static body (joint anchor). Upstream also calls AddGroundBox(10)
  // before this — dumpCreate / worker setupGround provide that ground.
  const wallBody = world.createBody({ type: BodyType.Static });
  runtime.createTransformedHullShape(wallBody, WALL_HALF, { position: WALL_LOCAL });

  const mesh = world.createGridMesh(GRID_CELL_COUNT, GRID_CELL_COUNT, GRID_CELL_WIDTH, 0, true);
  const sensorRotation = sensorHitsSensorRotation(runtime);
  const sensorShapeDef = { isSensor: true, enableSensorEvents: true } as const;

  const staticSensorBody = world.createBody({
    type: BodyType.Static,
    position: [f32(-4), f32(6), 0],
    rotation: sensorRotation,
  });
  world.createMeshShape(staticSensorBody, mesh, { scale: [1, 1, 1], ...sensorShapeDef });

  const kinematicBody = world.createBody({
    type: BodyType.Kinematic,
    position: [0, f32(6), 0],
    rotation: sensorRotation,
    linearVelocity: [f32(0.5), 0, 0],
  });
  world.createMeshShape(kinematicBody, mesh, { scale: [1, 1, 1], ...sensorShapeDef });

  const dynamicBody = world.createBody({
    type: BodyType.Dynamic,
    position: [f32(4), f32(1), 0],
  });
  world.createCapsuleShape(dynamicBody, [0, f32(1), 0], [0, f32(9), 0], f32(0.1), sensorShapeDef);

  const pivot: Vec3 = [f32(4), f32(7), 0];
  const localA = world.getBodyLocalPoint(wallBody, pivot);
  const localB = world.getBodyLocalPoint(dynamicBody, pivot);
  const joint = world.createPrismaticJoint(wallBody, dynamicBody, {
    localFrameA: { position: localA },
    localFrameB: { position: localB },
    enableMotor: true,
    maxMotorForce: f32(1000),
    motorSpeed: f32(0.5),
  });

  const launchBody = world.createBody({
    type: BodyType.Dynamic,
    position: [f32(-26.7), f32(6), 0],
    linearVelocity: [SENSOR_HITS_LAUNCH_SPEED, 0, 0],
    isBullet: true,
  });
  world.createSphereShape(launchBody, [0, 0, 0], f32(0.25), {
    enableSensorEvents: true,
    friction: f32(0.8),
    rollingResistance: f32(0.01),
  });

  const handles = [wallBody, staticSensorBody, kinematicBody, dynamicBody, launchBody];
  return {
    handles,
    state: {
      mesh,
      wallBody,
      staticSensorBody,
      kinematicBody,
      dynamicBody,
      launchBody,
      joint,
    },
  };
}

/** Mirror C++ SensorHits::Step reverse logic (runs before Sample::Step). */
export function sensorHitsPreStep(world: PhysicsWorld, state: SensorHitsState): void {
  const p = world.getBodyTransform(state.kinematicBody).position;
  if (p[0] > 1) {
    world.setBodyLinearVelocity(state.kinematicBody, [f32(-0.5), 0, 0]);
  } else if (p[0] < -1) {
    world.setBodyLinearVelocity(state.kinematicBody, [f32(0.5), 0, 0]);
  }

  const x = world.getPrismaticTranslation(state.joint);
  if (x > 1) {
    world.setPrismaticMotorSpeed(state.joint, f32(-0.5));
  } else if (x < -1) {
    world.setPrismaticMotorSpeed(state.joint, f32(0.5));
  }
}

export function sensorHitsGroundSize(): Vec3 {
  return [SENSOR_HITS_GROUND_EXTENT, f32(1), SENSOR_HITS_GROUND_EXTENT];
}

const sensorHitsRenderRotation = quatFromAxisAngle(B3_AXIS_Z, SENSOR_ROTATION_ANGLE);

export const sensorHitsBodies: RenderBody[] = [
  {
    kind: "box",
    size: [2 * WALL_HALF[0], 2 * WALL_HALF[1], 2 * WALL_HALF[2]],
    position: [0, 0, 0],
    localPosition: [...WALL_LOCAL] as [number, number, number],
    type: BodyType.Static,
    color: 0x94a3b8,
  },
  {
    kind: "box",
    size: [GRID_CELL_COUNT * GRID_CELL_WIDTH, 0.05, GRID_CELL_COUNT * GRID_CELL_WIDTH],
    position: [-4, 6, 0],
    rotation: sensorHitsRenderRotation,
    type: BodyType.Static,
    color: 0x34d399,
  },
  {
    kind: "box",
    size: [GRID_CELL_COUNT * GRID_CELL_WIDTH, 0.05, GRID_CELL_COUNT * GRID_CELL_WIDTH],
    position: [0, 6, 0],
    rotation: sensorHitsRenderRotation,
    type: BodyType.Kinematic,
    color: 0x22c55e,
  },
  {
    kind: "capsule",
    axis: "y",
    radius: 0.1,
    length: 8,
    position: [4, 1, 0],
    localPosition: [0, 5, 0],
    color: 0xfbbf24,
  },
  {
    kind: "sphere",
    radius: 0.25,
    position: [-26.7, 6, 0],
    color: 0x60a5fa,
  },
];

export const sensorHitsCamera: RenderSpec["camera"] = cameraFromSetView(0, 30, 40, [0, 5, 0]);

export const dumpSampleName = "Sensor Hits";
export const dumpSampleId = "events/sensor-hits";
export const dumpCppSampleName = "Sensor Hits";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: SensorHitsState;
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, sensorHitsGroundSize());
  const { handles, state } = buildSensorHitsDynamicBodies(world, runtime);
  return {
    world,
    handles: [ground, ...handles],
    state,
    dispose: () => {
      world.destroyMesh(state.mesh);
    },
  };
}

export function dumpStep(
  world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly number[],
  _frame: number,
  _dt: number,
  state: unknown,
): void {
  sensorHitsPreStep(world, state as SensorHitsState);
}
