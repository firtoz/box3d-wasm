import {
  B3_AXIS_X,
  B3_AXIS_Y,
  B3_AXIS_Z,
  B3_PI,
  BodyType,
  quatFromAxisAngle,
  type BodyHandle,
  type Box3DRuntime,
  type HeightFieldHandle,
  type JointHandle,
  type PhysicsWorld,
  type Quat,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Mul } from "../f32";

export const DRIVING_WAVE_ROW_COUNT = 50;
export const DRIVING_WAVE_COLUMN_COUNT = 50;
export const DRIVING_WAVE_SCALE: Vec3 = [4, 2, 4];
export const DRIVING_WAVE_ROW_FREQUENCY = 0.02;
export const DRIVING_WAVE_COLUMN_FREQUENCY = 0.04;
export const DRIVING_GROUND_POSITION: Vec3 = [-20, 0, -20];

export const DRIVING_SUSPENSION_HERTZ = 4;
export const DRIVING_SUSPENSION_DAMPING = 0.7;
export const DRIVING_LOWER_SUSPENSION = -0.2;
export const DRIVING_UPPER_SUSPENSION = 0.2;
export const DRIVING_MAX_SPIN_TORQUE = 5;
export const DRIVING_STEERING_HERTZ = 10;
export const DRIVING_STEERING_DAMPING = 0.7;
export const DRIVING_MAX_STEERING_TORQUE = 5;
export const DRIVING_LOWER_STEERING_DEG = -45;
export const DRIVING_UPPER_STEERING_DEG = 45;

/** Matches `b3ComputeQuatBetweenUnitVectors(Y, Z)` for wheel body rotation. */
export const DRIVING_WHEEL_BODY_ROTATION: Quat = quatFromAxisAngle(B3_AXIS_X, -0.5 * B3_PI);

const WHEEL_RADIUS = 0.4;
const CHASSIS_HALF: Vec3 = [2, 0.5, 1];
const CHASSIS_POS: Vec3 = [0, 2.5, 0];

const WHEEL_POSITIONS: readonly Vec3[] = [
  [1.5, 2, 0.8],
  [1.5, 2, -0.8],
  [-1.5, 2, 0.8],
  [-1.5, 2, -0.8],
] as const;

const WHEEL_JOINT_ANCHORS: readonly Vec3[] = [
  [1.5, -0.5, 0.8],
  [1.5, -0.5, -0.8],
  [-1.5, -0.5, 0.8],
  [-1.5, -0.5, -0.8],
] as const;

export interface DrivingScene {
  handles: BodyHandle[];
  heightField: HeightFieldHandle;
  joints: {
    rearLeft: JointHandle;
    rearRight: JointHandle;
    frontLeft: JointHandle;
    frontRight: JointHandle;
  };
}

function createWheelJoint(
  world: PhysicsWorld,
  chassis: BodyHandle,
  wheel: BodyHandle,
  anchor: Vec3,
  options: { enableSteering: boolean; enableSpinMotor: boolean },
  frameA: Quat,
  frameB: Quat,
): JointHandle {
  const lowerSteeringLimit = f32Mul(B3_PI / 180, DRIVING_LOWER_STEERING_DEG);
  const upperSteeringLimit = f32Mul(B3_PI / 180, DRIVING_UPPER_STEERING_DEG);
  return world.createWheelJoint(chassis, wheel, {
    localFrameA: { position: anchor, rotation: frameA },
    localFrameB: { rotation: frameB },
    enableSuspensionLimit: true,
    lowerSuspensionLimit: DRIVING_LOWER_SUSPENSION,
    upperSuspensionLimit: DRIVING_UPPER_SUSPENSION,
    enableSuspensionSpring: true,
    suspensionHertz: DRIVING_SUSPENSION_HERTZ,
    suspensionDampingRatio: DRIVING_SUSPENSION_DAMPING,
    enableSpinMotor: options.enableSpinMotor,
    maxSpinTorque: DRIVING_MAX_SPIN_TORQUE,
    enableSteering: options.enableSteering,
    steeringHertz: DRIVING_STEERING_HERTZ,
    steeringDampingRatio: DRIVING_STEERING_DAMPING,
    targetSteeringAngle: 0,
    maxSteeringTorque: DRIVING_MAX_STEERING_TORQUE,
    enableSteeringLimit: true,
    lowerSteeringLimit,
    upperSteeringLimit,
  });
}

export function buildDrivingScene(world: PhysicsWorld, runtime: Box3DRuntime): DrivingScene {
  const heightField = world.createWave(
    DRIVING_WAVE_ROW_COUNT,
    DRIVING_WAVE_COLUMN_COUNT,
    DRIVING_WAVE_SCALE,
    DRIVING_WAVE_ROW_FREQUENCY,
    DRIVING_WAVE_COLUMN_FREQUENCY,
    false,
  );

  const ground = world.createBody({ position: DRIVING_GROUND_POSITION });
  world.createHeightFieldShape(ground, heightField, {});

  const chassis = world.createBody({
    type: BodyType.Dynamic,
    position: CHASSIS_POS,
  });
  runtime.createHullShape(chassis, CHASSIS_HALF, { density: 0.5 });

  const uprightQ = runtime.computeQuatBetweenUnitVectors(B3_AXIS_Z, B3_AXIS_Y);
  world.createParallelJoint(ground, chassis, {
    localFrameA: { rotation: uprightQ },
    localFrameB: { rotation: uprightQ },
    hertz: 0.5,
    dampingRatio: 1.0,
    collideConnected: true,
  });

  const wheelRotation = runtime.computeQuatBetweenUnitVectors(B3_AXIS_Y, B3_AXIS_Z);
  const frameA = runtime.computeQuatBetweenUnitVectors(B3_AXIS_X, B3_AXIS_Y);
  const frameB = runtime.computeQuatBetweenUnitVectors(B3_AXIS_Z, B3_AXIS_Y);

  const wheelShape = { density: 2, friction: 3 } as const;
  const wheels: BodyHandle[] = [];
  for (let i = 0; i < WHEEL_POSITIONS.length; i++) {
    const wheel = world.createBody({
      type: BodyType.Dynamic,
      position: WHEEL_POSITIONS[i]!,
      rotation: wheelRotation,
      allowFastRotation: true,
    });
    world.createSphereShape(wheel, [0, 0, 0], WHEEL_RADIUS, wheelShape);
    wheels.push(wheel);
  }

  const frontLeft = createWheelJoint(world, chassis, wheels[0]!, WHEEL_JOINT_ANCHORS[0]!, { enableSteering: true, enableSpinMotor: false }, frameA, frameB);
  const frontRight = createWheelJoint(world, chassis, wheels[1]!, WHEEL_JOINT_ANCHORS[1]!, { enableSteering: true, enableSpinMotor: false }, frameA, frameB);
  const rearLeft = createWheelJoint(world, chassis, wheels[2]!, WHEEL_JOINT_ANCHORS[2]!, { enableSteering: false, enableSpinMotor: true }, frameA, frameB);
  const rearRight = createWheelJoint(world, chassis, wheels[3]!, WHEEL_JOINT_ANCHORS[3]!, { enableSteering: false, enableSpinMotor: true }, frameA, frameB);

  return {
    handles: [ground, chassis, wheels[0]!, wheels[1]!, wheels[2]!, wheels[3]!],
    heightField,
    joints: { frontLeft, frontRight, rearLeft, rearRight },
  };
}

export function buildDrivingDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  return buildDrivingScene(world, runtime).handles;
}

export function drivingGroundSize(): Vec3 {
  return [
    f32Mul(0.5, f32Mul(DRIVING_WAVE_SCALE[0], DRIVING_WAVE_COLUMN_COUNT - 1)),
    f32(1),
    f32Mul(0.5, f32Mul(DRIVING_WAVE_SCALE[2], DRIVING_WAVE_ROW_COUNT - 1)),
  ];
}

export const drivingBodies: RenderBody[] = [
  {
    kind: "box",
    size: [0.01, 0.01, 0.01],
    position: DRIVING_GROUND_POSITION,
    type: BodyType.Static,
    color: 0x64748b,
  },
  {
    kind: "box",
    size: [4, 1, 2],
    position: CHASSIS_POS,
    color: 0x38bdf8,
  },
  ...WHEEL_POSITIONS.map((position, index) => ({
    kind: "sphere" as const,
    radius: WHEEL_RADIUS,
    position,
    rotation: DRIVING_WHEEL_BODY_ROTATION,
    color: index < 2 ? 0xfbbf24 : 0xf97316,
  })),
];

export const drivingCamera: RenderSpec["camera"] = cameraFromSetView(25, 20, 7, [0, 2, 0]);

export const drivingHeightFieldVisual = {
  rowCount: DRIVING_WAVE_ROW_COUNT,
  columnCount: DRIVING_WAVE_COLUMN_COUNT,
  scale: DRIVING_WAVE_SCALE,
  rowFrequency: DRIVING_WAVE_ROW_FREQUENCY,
  columnFrequency: DRIVING_WAVE_COLUMN_FREQUENCY,
  position: DRIVING_GROUND_POSITION,
} as const;

export const dumpSampleName = "Driving";
export const dumpSampleId = "joints/driving";
export const dumpCppSampleName = "Driving";
export const dumpGroundSize = drivingGroundSize;
export const dumpBuildDynamicBodies = buildDrivingDynamicBodies;

export interface DrivingDumpState {
  joints: DrivingScene["joints"];
}

/** Passive upstream Step (no keyboard): throttle stays zero. */
export function dumpStep(
  _world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly BodyHandle[],
  _frame: number,
  _dt: number,
  state: DrivingDumpState,
): void {
  // Mirrors Driving::Step with throttle = {0, 0}: rear spin motors at 0, front steering target 0.
  // Wheel-joint setters are not yet wrapped; spinSpeed defaults to 0 at creation so physics matches.
  void state.joints.rearLeft;
  void state.joints.rearRight;
  void state.joints.frontLeft;
  void state.joints.frontRight;
}

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
  state: DrivingDumpState;
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const scene = buildDrivingScene(world, runtime);
  return {
    world,
    handles: scene.handles,
    state: { joints: scene.joints },
    dispose: () => {
      world.destroyHeightField(scene.heightField);
    },
  };
}
