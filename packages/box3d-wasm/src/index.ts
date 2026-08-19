export type Vec3 = [number, number, number];
export type Quat = [number, number, number, number];

export type WorldTransform = { position: Vec3; rotation: Quat };

export type LocalManifoldPoint = {
  point: Vec3;
  separation: number;
  pair: [number, number, number, number];
};

export type LocalManifold = {
  normal: Vec3;
  triangleNormal: Vec3;
  pointCount: number;
  feature: number;
  triangleIndex: number;
  indices: [number, number, number];
  squaredDistance: number;
  triangleFlags: number;
  points: LocalManifoldPoint[];
};

export type ShapeCastHit = {
  hit: boolean;
  fraction: number;
  point: Vec3;
  normal: Vec3;
  iterations: number;
};

export const B3W_MANIFOLD_MAX_POINTS = 64;
export const B3W_MANIFOLD_HEADER_FLOATS = 14;
export const B3W_MANIFOLD_POINT_FLOATS = 8;

declare const handleBrand: unique symbol;

type NumberHandle<Name extends string> = number & { readonly [handleBrand]: Name };
type BigIntHandle<Name extends string> = bigint & { readonly [handleBrand]: Name };

export type WorldHandle = NumberHandle<"WorldHandle">; // still int slot
export type BodyId = BigIntHandle<"BodyId">;
export type ShapeId = BigIntHandle<"ShapeId">;
export type JointId = BigIntHandle<"JointId">;
/** @deprecated Use BodyId */
export type BodyHandle = BodyId;
/** @deprecated Use JointId */
export type JointHandle = JointId;

export type HullHandle = NumberHandle<"HullHandle">;
export type MeshHandle = NumberHandle<"MeshHandle">;
export type CompoundHandle = NumberHandle<"CompoundHandle">;
export type HumanHandle = NumberHandle<"HumanHandle">;
export type HeightFieldHandle = NumberHandle<"HeightFieldHandle">;

export const B3_PI = 3.14159265359;
export const B3_DEG_TO_RAD = 0.01745329251;
export const B3_AXIS_X: Vec3 = [1, 0, 0];
export const B3_AXIS_Y: Vec3 = [0, 1, 0];
export const B3_AXIS_Z: Vec3 = [0, 0, 1];

const DEFAULT_JOINT_FORCE_THRESHOLD = 3.402823466e+38;
type JointThresholdOptions = { forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean };
function jointThresholdArgs(options: JointThresholdOptions = {}): [number, number, number] {
  return [
    options.forceThreshold ?? DEFAULT_JOINT_FORCE_THRESHOLD,
    options.torqueThreshold ?? DEFAULT_JOINT_FORCE_THRESHOLD,
    options.collideConnected ? 1 : 0,
  ];
}

export function quatFromAxisAngle(axis: Vec3, radians: number): Quat {
  const halfAngle = 0.5 * radians;
  const sine = Math.sin(halfAngle);
  return [axis[0] * sine, axis[1] * sine, axis[2] * sine, Math.cos(halfAngle)];
}

declare global { var BOX3D_POOL_SIZE: number | undefined; }

export enum BodyType {
  Static = 0,
  Kinematic = 1,
  Dynamic = 2,
}

export interface WorldCapacity {
  staticShapeCount?: number;
  dynamicShapeCount?: number;
  staticBodyCount?: number;
  dynamicBodyCount?: number;
  contactCount?: number;
}

export interface WorldOptions { gravity?: Vec3; workerCount?: number; capacity?: WorldCapacity; }

export type SlotKind = "worlds" | "hulls" | "meshes" | "compounds" | "humans" | "heightFields";

export interface SlotLimits {
  worlds: number;
  hulls: number;
  meshes: number;
  compounds: number;
  humans: number;
  heightFields: number;
}

export type SlotUsage = SlotLimits;

export class SlotExhaustedError extends Error {
  readonly kind: SlotKind;
  readonly used: number;
  readonly max: number;

  constructor(kind: SlotKind, used: number, max: number) {
    super(`Box3D WASM ${kind} slot pool exhausted (${used}/${max}). Rebuild with higher B3W_MAX_* limits, use the growable WASM variant, or reduce tracked entities.`);
    this.name = "SlotExhaustedError";
    this.kind = kind;
    this.used = used;
    this.max = max;
  }
}

export type ProfileLevel = "off" | "coarse" | "full";

export function profileLevelToInt(level: ProfileLevel): number {
  if (level === "off") return 0;
  if (level === "coarse") return 1;
  return 2;
}

export function profileLevelFromInt(level: number): ProfileLevel {
  if (level <= 0) return "off";
  if (level === 1) return "coarse";
  return "full";
}

export interface WorldProfile {
  step: number; pairs: number; collide: number; solve: number;
  solverSetup: number; constraints: number; prepareConstraints: number;
  integrateVelocities: number; warmStart: number; solveImpulses: number;
  integratePositions: number; relaxImpulses: number; applyRestitution: number;
  storeImpulses: number; splitIslands: number; transforms: number;
  sensorHits: number; jointEvents: number; hitEvents: number;
  refit: number; bullets: number; sleepIslands: number; sensors: number;
}

export interface BodyDef {
  type?: BodyType;
  position?: Vec3;
  rotation?: Quat;
  linearVelocity?: Vec3;
  angularVelocity?: Vec3;
  linearDamping?: number;
  angularDamping?: number;
  gravityScale?: number;
  sleepThreshold?: number;
  isBullet?: boolean;
  isEnabled?: boolean;
  allowFastRotation?: boolean;
  enableSleep?: boolean;
  isAwake?: boolean;
  enableContactRecycling?: boolean;
}

export interface ShapeDef {
  density?: number;
  friction?: number;
  restitution?: number;
  rollingResistance?: number;
  tangentVelocity?: Vec3;
  explosionScale?: number;
  isSensor?: boolean;
  enableSensorEvents?: boolean;
  enableContactEvents?: boolean;
  enableHitEvents?: boolean;
  enablePreSolveEvents?: boolean;
  categoryBits?: number;
  maskBits?: number;
  groupIndex?: number;
  enableCustomFiltering?: boolean;
  updateBodyMass?: boolean;
  /** When true, creating/updating this shape forces contact creation (static-vs-dynamic overlaps). */
  invokeContactCreation?: boolean;
}

export interface SphereOptions {
  radius: number;
  position?: Vec3;
  velocity?: Vec3;
  density?: number;
  friction?: number;
  restitution?: number;
  rollingResistance?: number;
  isBullet?: boolean;
  isSensor?: boolean;
  enableContactEvents?: boolean;
  enableHitEvents?: boolean;
}

export interface BoxOptions {
  size: Vec3;
  position?: Vec3;
  static?: boolean;
  density?: number;
  friction?: number;
  restitution?: number;
  rollingResistance?: number;
  isSensor?: boolean;
  enableContactEvents?: boolean;
  enableHitEvents?: boolean;
}

export interface SurfaceMaterial { friction?: number; restitution?: number; rollingResistance?: number; tangentVelocity?: Vec3; }
export interface MotorJointOptions { localFrameA?: Vec3; localFrameB?: Vec3; linearVelocity?: Vec3; angularVelocity?: Vec3; maxVelocityForce?: number; maxVelocityTorque?: number; collideConnected?: boolean; linearHertz?: number; linearDampingRatio?: number; maxSpringForce?: number; angularHertz?: number; angularDampingRatio?: number; maxSpringTorque?: number; }
export interface BodyTransform { position: Vec3; rotation: Quat; }
export interface BodyMassData { mass: number; inertiaTrace: number; }
export type Mat3 = [number, number, number, number, number, number, number, number, number];
export interface WorldCounters { bodyCount: number; shapeCount: number; contactCount: number; jointCount: number; islandCount: number; staticTreeHeight: number; treeHeight: number; }
export interface BodyBatchBuffers { bodyHandlesPtr: number; positionsPtr: number; rotationsPtr: number; awakePtr: number; colorsPtr: number; capacity: number; }
export interface RuntimeMemoryView { heapF32: Float32Array; heapU8: Uint8Array; }
export interface RuntimeMemoryView32 extends RuntimeMemoryView { heap32: Int32Array; }
export interface CompoundHullEntry { halfWidths: Vec3; transform: BodyTransform; friction?: number; restitution?: number; rollingResistance?: number; }
export interface CompoundSphereEntry { center: Vec3; radius: number; friction?: number; restitution?: number; rollingResistance?: number; }
export interface CompoundMeshEntry {
  meshHandle: MeshHandle;
  transform: BodyTransform;
  scale?: Vec3;
  friction?: number;
  restitution?: number;
  rollingResistance?: number;
}
export interface ShapeHandle { bodyHandle: BodyId; shapeHandle: ShapeId; }
export interface MeshShapeOptions extends ShapeDef { scale?: Vec3; }
export interface SensorBeginEvent { sensorShapeHandle: ShapeId; visitorShapeHandle: ShapeId; }
export interface RuntimeLoadOptions {
  version?: string;
  variant?: "release" | "profile" | "growable";
  poolSize?: number;
  /** Asset base path for `wasm/` (must end with `/`). Needed in workers and on subpath hosts like GitHub Pages. */
  baseUrl?: string;
}
export interface RuntimeAPI {
  readonly limits: SlotLimits;
  createWorld(options?: WorldOptions): PhysicsWorld;
  getSlotUsage(): SlotUsage;
  checkThreadingSupport(): number;
}

type CModule = {
  cwrap(name: string, returnType: "number" | "bigint", argTypes: readonly string[]): (...args: any[]) => any;
  cwrap(name: string, returnType: null, argTypes: readonly string[]): (...args: any[]) => void;
  HEAPF32: Float32Array;
  HEAPU8: Uint8Array;
  HEAP32: Int32Array;
  HEAPU64: BigUint64Array;
  wasmMemory?: WebAssembly.Memory;
  _malloc(size: number): number;
  _free(ptr: number): void;
};
type CreateWorldFn = (
  gravityX: number, gravityY: number, gravityZ: number, workerCount: number,
  staticShapeCount: number, dynamicShapeCount: number, staticBodyCount: number, dynamicBodyCount: number, contactCount: number,
) => number;
type GetSlotLimitsFn = (outLimits: number) => void;
type GetSlotUsageFn = (outUsage: number) => void;
type CreateBodyFn = (worldHandle: number, bodyType: number, px: number, py: number, pz: number, enableSleep: number, awake: number) => bigint;
type DestroyWorldFn = (worldHandle: number) => void;
type CreateBoxFn = (worldHandle: number, px: number, py: number, pz: number, hx: number, hy: number, hz: number, isStatic: number, density: number) => bigint;
type CreateSphereFn = (worldHandle: number, px: number, py: number, pz: number, radius: number, vx: number, vy: number, vz: number, density: number) => bigint;
type CreateHullShapeFn = (bodyHandle: bigint, density: number, friction: number, restitution: number, rollingResistance: number, updateBodyMass: number, tx: number, ty: number, tz: number, qx: number, qy: number, qz: number, qw: number, hx: number, hy: number, hz: number, isSensor: number) => bigint;
type CreateTransformedHullShapeFn = (bodyHandle: bigint, density: number, friction: number, restitution: number, rollingResistance: number, tx: number, ty: number, tz: number, qx: number, qy: number, qz: number, qw: number, hx: number, hy: number, hz: number, sx: number, sy: number, sz: number) => bigint;
type CreateOffsetHullShapeFn = (bodyHandle: bigint, density: number, friction: number, restitution: number, rollingResistance: number, updateBodyMass: number, hx: number, hy: number, hz: number, ox: number, oy: number, oz: number) => bigint;
type CreateSphereShapeFn = (bodyHandle: bigint, density: number, friction: number, restitution: number, rollingResistance: number, px: number, py: number, pz: number, radius: number, invokeContactCreation: number) => bigint;
type CreateCapsuleShapeFn = (bodyHandle: bigint, density: number, friction: number, restitution: number, rollingResistance: number, ax: number, ay: number, az: number, bx: number, by: number, bz: number, radius: number, isSensor: number) => bigint;
type CreateShapeFromHullFn = (bodyHandle: bigint, hullHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, updateBodyMass: number, explosionScale: number, isSensor: number) => bigint;
type CreateTransformedShapeFromHullFn = (bodyHandle: bigint, hullHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, updateBodyMass: number, tx: number, ty: number, tz: number, qx: number, qy: number, qz: number, qw: number, sx: number, sy: number, sz: number) => bigint;
type CreateCylinderFn = (height: number, radius: number, yOffset: number, sides: number) => number;
type CreateGridMeshFn = (worldHandle: number, xCount: number, zCount: number, cellWidth: number, materialCount: number, identifyEdges: number) => number;
type CreateWaveMeshFn = (worldHandle: number, xCount: number, zCount: number, cellWidth: number, amplitude: number, rowFrequency: number, columnFrequency: number) => number;
type CreateBoxMeshFn = (worldHandle: number, cx: number, cy: number, cz: number, ex: number, ey: number, ez: number, identifyEdges: number) => number;
type CreateTorusMeshFn = (worldHandle: number, radialResolution: number, tubularResolution: number, radius: number, thickness: number) => number;
type CreateMeshFn = (worldHandle: number, vertices: number, vertexCount: number, indices: number, triangleCount: number, useMedianSplit: number, identifyEdges: number) => number;
type DestroyMeshFn = (meshHandle: number) => void;
type CreateMeshShapeFn = (bodyHandle: bigint, meshHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, sx: number, sy: number, sz: number, isSensor: number) => bigint;
type ShapeSetMeshFn = (shapeHandle: bigint, meshHandle: number, sx: number, sy: number, sz: number) => void;
type CreateHullFromPointsFn = (numPoints: number, points: number) => number;
type CreateRockFn = (radius: number) => number;
type DestroyHullFn = (hullHandle: number) => void;
type MakeBoxHullFn = (hx: number, hy: number, hz: number) => number;
type MakeTransformedBoxHullFn = (hx: number, hy: number, hz: number, px: number, py: number, pz: number, qx: number, qy: number, qz: number, qs: number) => number;
type CollideSpheresFn = (ax: number, ay: number, az: number, ar: number, bx: number, by: number, bz: number, br: number, xfA: number, xfB: number, capacity: number, out: number, capacityFloats: number) => void;
type CollideCapsuleSphereFn = (c1x: number, c1y: number, c1z: number, c2x: number, c2y: number, c2z: number, cr: number, sx: number, sy: number, sz: number, sr: number, xfA: number, xfB: number, capacity: number, out: number, capacityFloats: number) => void;
type CollideHullSphereFn = (hullHandle: number, sx: number, sy: number, sz: number, sr: number, xfA: number, xfB: number, capacity: number, out: number, capacityFloats: number) => void;
type CollideCapsulesFn = (a1x: number, a1y: number, a1z: number, a2x: number, a2y: number, a2z: number, ar: number, b1x: number, b1y: number, b1z: number, b2x: number, b2y: number, b2z: number, br: number, xfA: number, xfB: number, capacity: number, out: number, capacityFloats: number) => void;
type CollideHullCapsuleFn = (hullHandle: number, c1x: number, c1y: number, c1z: number, c2x: number, c2y: number, c2z: number, cr: number, xfA: number, xfB: number, capacity: number, out: number, capacityFloats: number) => void;
type CollideHullsFn = (hullA: number, hullB: number, xfA: number, xfB: number, capacity: number, out: number, capacityFloats: number) => void;
type CollideTriangleSphereFn = (triangle: number, sx: number, sy: number, sz: number, sr: number, xfA: number, xfB: number, capacity: number, out: number, capacityFloats: number) => void;
type CollideTriangleCapsuleFn = (triangle: number, c1x: number, c1y: number, c1z: number, c2x: number, c2y: number, c2z: number, cr: number, xfA: number, xfB: number, capacity: number, out: number, capacityFloats: number) => void;
type CollideTriangleHullFn = (triangle: number, flags: number, hullHandle: number, xfA: number, xfB: number, capacity: number, enableSpeculative: number, out: number, capacityFloats: number) => void;
type ShapeCastPairFn = (pointsA: number, countA: number, radiusA: number, pointsB: number, countB: number, radiusB: number, transform: number, translation: number, maxFraction: number, canEncroach: number, out: number) => void;
type GetHullVertexCountFn = (hullHandle: number) => number;
type GetHullPointsFn = (hullHandle: number, outPoints: number, capacityFloats: number) => number;
type CreateCompoundFn = (capsuleCount: number, hullCount: number, meshCount: number, sphereCount: number, capsules: number, hulls: number, meshes: number, spheres: number) => number;
type CreateCompoundFromHullsFn = (hullCount: number, hullData: number, strideFloats: number) => number;
type CreateCompoundFromSpheresFn = (sphereCount: number, sphereData: number, strideFloats: number) => number;
type CreateCompoundFromMeshesFn = (meshCount: number, meshData: number, strideFloats: number) => number;
type DestroyCompoundFn = (compoundHandle: number) => void;
type GetCompoundTreeHeightFn = (compoundHandle: number) => number;
type CreateCompoundShapeFn = (bodyHandle: bigint, compoundHandle: number, density: number) => bigint;
type DestroyBodyFn = (bodyHandle: bigint) => void;
type DestroyJointFn = (jointHandle: bigint) => void;
type SetBodyTransformFn = (bodyHandle: bigint, px: number, py: number, pz: number, qx: number, qy: number, qz: number, qw: number) => void;
type SetBodyLinearVelocityFn = (bodyHandle: bigint, x: number, y: number, z: number) => void;
type SetBodyAngularVelocityFn = (bodyHandle: bigint, x: number, y: number, z: number) => void;
type GetBodyVelocityFn = (bodyHandle: bigint, outVelocity: number) => void;
type SetBodyAwakeFn = (bodyHandle: bigint, awake: number) => void;
type SetBodyDampingFn = (bodyHandle: bigint, linearDamping: number, angularDamping: number) => void;
type GetBodyLocalPointFn = (bodyHandle: bigint, worldX: number, worldY: number, worldZ: number, outPoint: number) => void;
type CreateMotorJointFn = (worldHandle: number, bodyAHandle: bigint, bodyBHandle: bigint, localAx: number, localAy: number, localAz: number, localBx: number, localBy: number, localBz: number, linearVx: number, linearVy: number, linearVz: number, maxVelocityForce: number, angularVx: number, angularVy: number, angularVz: number, maxVelocityTorque: number, collideConnected: number, linearHertz: number, linearDampingRatio: number, maxSpringForce: number, angularHertz: number, angularDampingRatio: number, maxSpringTorque: number) => bigint;
type CreateFilterJointFn = (worldHandle: number, bodyAHandle: bigint, bodyBHandle: bigint) => bigint;
type CreateRevoluteJointFn = (worldHandle: number, bodyAHandle: bigint, bodyBHandle: bigint, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, constraintHertz: number, constraintDampingRatio: number, targetAngle: number, enableSpring: number, hertz: number, dampingRatio: number, enableLimit: number, lowerAngle: number, upperAngle: number, enableMotor: number, maxMotorTorque: number, motorSpeed: number, forceThreshold: number, torqueThreshold: number, collideConnected: number) => bigint;
type CreateSphericalJointFn = (worldHandle: number, bodyAHandle: bigint, bodyBHandle: bigint, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, enableSpring: number, hertz: number, dampingRatio: number, targetQx: number, targetQy: number, targetQz: number, targetQw: number, enableConeLimit: number, coneAngle: number, enableTwistLimit: number, lowerTwistAngle: number, upperTwistAngle: number, enableMotor: number, maxMotorTorque: number, motorVx: number, motorVy: number, motorVz: number) => bigint;
type CreateHumanFn = (worldHandle: number, px: number, py: number, pz: number, frictionTorque: number, hertz: number, dampingRatio: number, groupIndex: number, colorize: number) => number;
type GetHumanBoneBodyFn = (humanHandle: number, boneIndex: number) => bigint;
type GetHumanBoneCountFn = () => number;
type HumanSetVelocityFn = (humanHandle: number, x: number, y: number, z: number) => void;
type HumanSetBulletFn = (humanHandle: number, flag: number) => void;
type HumanSetJointFloatFn = (humanHandle: number, value: number) => void;
type StepFn = (worldHandle: number, timeStep: number, subStepCount: number) => void;
type GetBodyTransformFn = (bodyHandle: bigint, outTransform: number) => void;
type ShapeSetSurfaceMaterialFn = (shapeHandle: bigint, friction: number, restitution: number, rollingResistance: number, tvx: number, tvy: number, tvz: number) => void;
type ShapeSetFilterFn = (shapeHandle: bigint, categoryBits: number, maskBits: number, groupIndex: number, invokeContacts: number) => void;
type GetBodyShapeCountFn = (bodyHandle: bigint) => number;
type GetBodyShapesFn = (bodyHandle: bigint, outShapeHandles: number, capacity: number) => number;
type DestroyShapeFn = (shapeHandle: bigint, updateBodyMass: number) => void;
type ShapeEnableBoolFn = (shapeHandle: bigint, flag: number) => void;
type ShapeSetSphereFn = (shapeHandle: bigint, px: number, py: number, pz: number, radius: number) => void;
type ShapeSetCapsuleFn = (shapeHandle: bigint, ax: number, ay: number, az: number, bx: number, by: number, bz: number, radius: number) => void;
type ShapeApplyWindFn = (shapeHandle: bigint, windX: number, windY: number, windZ: number, drag: number, lift: number, maxSpeed: number, wake: number) => void;
type BodyIsAwakeFn = (bodyHandle: bigint) => number;
type GetBodyDebugColorFn = (bodyHandle: bigint) => number;
type GetBodyTypeFn = (bodyHandle: bigint) => number;
type BodySetTypeFn = (bodyHandle: bigint, type: number) => void;
type BodySetNameFn = (bodyHandle: bigint, name: number) => void;
type BodySetGravityScaleFn = (bodyHandle: bigint, scale: number) => void;
type BodySetSleepThresholdFn = (bodyHandle: bigint, threshold: number) => void;
type BodyEnableSleepFn = (bodyHandle: bigint, enableSleep: number) => void;
type BodySetBulletFn = (bodyHandle: bigint, flag: number) => void;
type BodyAllowFastRotationFn = (bodyHandle: bigint, flag: number) => void;
type BodyIsFastRotationAllowedFn = (bodyHandle: bigint) => number;
type BodyEnableContactRecyclingFn = (bodyHandle: bigint, flag: number) => void;
type BodyEnableHitEventsFn = (bodyHandle: bigint, flag: number) => void;
type BodySetMotionLocksFn = (bodyHandle: bigint, lockBodyX: number, lockBodyY: number, lockBodyRotationX: number, lockBodyRotationY: number, lockBodyRotationZ: number, lockLinearZ: number) => void;
type BodySetMassDataFn = (bodyHandle: bigint, mass: number, cx: number, cy: number, cz: number, inertia: number) => void;
type BodyGetMassDataFn = (bodyHandle: bigint, outMassData: number) => void;
type BodyApplyMassFromShapesFn = (bodyHandle: bigint) => void;
type BodySetTargetTransformFn = (bodyHandle: bigint, px: number, py: number, pz: number, qx: number, qy: number, qz: number, qw: number, timeStep: number, wake: number) => void;
type ApplyLinearImpulseFn = (bodyHandle: bigint, ix: number, iy: number, iz: number, px: number, py: number, pz: number, wake: number) => void;
type ApplyLinearImpulseToCenterFn = (bodyHandle: bigint, ix: number, iy: number, iz: number, wake: number) => void;
type MakeQuatFromAxisAngleFn = (axisX: number, axisY: number, axisZ: number, radians: number, outQuat: number) => void;
type RotateVectorFn = (qx: number, qy: number, qz: number, qs: number, vx: number, vy: number, vz: number, outVec: number) => void;
type RandomVec3Fn = (lox: number, loy: number, loz: number, hix: number, hiy: number, hiz: number, outVec: number) => void;
type LerpVec3Fn = (ax: number, ay: number, az: number, bx: number, by: number, bz: number, alpha: number, outVec: number) => void;
type GetLengthAndNormalizeFn = (vx: number, vy: number, vz: number, outDirection: number) => number;
type ComputeQuatBetweenUnitVectorsFn = (ax: number, ay: number, az: number, bx: number, by: number, bz: number, outQuat: number) => void;
type InvMulQuatFn = (aqx: number, aqy: number, aqz: number, aqs: number, bqx: number, bqy: number, bqz: number, bqs: number, outQuat: number) => void;
type ShapeSetDensityFn = (shapeHandle: bigint, density: number, updateBodyMass: number) => void;
type WorldEnableBoolFn = (worldHandle: number, flag: number) => void;
type WorldSetProfileLevelFn = (worldHandle: number, level: number) => void;
type WorldGetProfileLevelFn = (worldHandle: number) => number;
type WorldSetContactTuningFn = (worldHandle: number, hertz: number, dampingRatio: number, contactSpeed: number) => void;
type WorldSetFloatFn = (worldHandle: number, value: number) => void;
type WorldSetWorkerCountFn = (worldHandle: number, count: number) => void;
type GetWorldCountersFn = (worldHandle: number, outCounters: number) => void;
type GetWorldProfileFn = (worldHandle: number, outProfile: number) => void;
type GetWorldAwakeBodyCountFn = (worldHandle: number) => number;
type CheckThreadingSupportFn = () => number;
type GetWorldWorkerCountFn = (worldHandle: number) => number;
type WriteBodyTransformsFn = (count: number, bodyHandles: number, outPositions: number, outRotations: number, outAwake: number, outColors: number) => void;
type WriteBodyTransformsLightFn = WriteBodyTransformsFn;
type ConfigureBodyMoveTrackingFn = (count: number, bodyHandles: number) => void;
type ClearBodyMoveTrackingFn = () => void;
type ScatterBodyMoveEventsFn = (worldHandle: number, outPositions: number, outRotations: number, outAwake: number, outColors: number, useLightColors: number) => number;
type GetBodyMoveEventCountFn = (worldHandle: number) => number;
type RayCastClosestFn = (worldHandle: number, ox: number, oy: number, oz: number, tx: number, ty: number, tz: number, categoryBits: number, maskBits: number, outShapeHandle: number, outPoint: number, outNormal: number, outFraction: number) => void;
type BodyEnableFn = (bodyHandle: bigint) => void;
type BodyIsEnabledFn = (bodyHandle: bigint) => number;
type GetBodyMassFn = (bodyHandle: bigint) => number;
type GetBodyLocalRotationalInertiaFn = (bodyHandle: bigint, outInertia: number) => void;
type GetBodyWorldCenterFn = (bodyHandle: bigint, outPoint: number) => void;
type GetBodyWorldPointFn = (bodyHandle: bigint, lx: number, ly: number, lz: number, outPoint: number) => void;
type GetBodyLocalPointVelocityFn = (bodyHandle: bigint, lx: number, ly: number, lz: number, outVelocity: number) => void;
type GetBodyWorldPointVelocityFn = (bodyHandle: bigint, wx: number, wy: number, wz: number, outVelocity: number) => void;
type CreatePrismaticJointFn = (worldHandle: number, bodyAHandle: bigint, bodyBHandle: bigint, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, constraintHertz: number, constraintDampingRatio: number, enableSpring: number, hertz: number, dampingRatio: number, targetTranslation: number, enableLimit: number, lowerTranslation: number, upperTranslation: number, enableMotor: number, maxMotorForce: number, motorSpeed: number, forceThreshold: number, torqueThreshold: number, collideConnected: number) => bigint;
type CreateWeldJointFn = (worldHandle: number, bodyAHandle: bigint, bodyBHandle: bigint, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, linearHertz: number, angularHertz: number, linearDampingRatio: number, angularDampingRatio: number, forceThreshold: number, torqueThreshold: number, collideConnected: number) => bigint;
type CreateDistanceJointFn = (worldHandle: number, bodyAHandle: bigint, bodyBHandle: bigint, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, length: number, forceThreshold: number, torqueThreshold: number, collideConnected: number) => bigint;
type CreateParallelJointFn = (worldHandle: number, bodyAHandle: bigint, bodyBHandle: bigint, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, hertz: number, dampingRatio: number, maxTorque: number, forceThreshold: number, torqueThreshold: number, collideConnected: number) => bigint;
type CreateWheelJointFn = (
  worldHandle: number, bodyAHandle: bigint, bodyBHandle: bigint,
  localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number,
  localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number,
  enableSuspensionSpring: number, suspensionHertz: number, suspensionDampingRatio: number,
  enableSuspensionLimit: number, lowerSuspensionLimit: number, upperSuspensionLimit: number,
  enableSpinMotor: number, maxSpinTorque: number, spinSpeed: number,
  enableSteering: number, steeringHertz: number, steeringDampingRatio: number, targetSteeringAngle: number, maxSteeringTorque: number,
  enableSteeringLimit: number, lowerSteeringLimit: number, upperSteeringLimit: number,
  collideConnected: number,
) => bigint;
type GetStallThresholdFn = () => number;
type SetStallThresholdFn = (seconds: number) => void;
type WorldExplodeFn = (worldHandle: number, px: number, py: number, pz: number, radius: number, falloff: number, impulsePerArea: number, maskBits: number) => void;
type GetJointVec3Fn = (jointHandle: bigint, outVec3: number) => void;
type GetJointLinearSeparationFn = (jointHandle: bigint) => number;
type RevoluteJointSetTargetAngleFn = (jointHandle: bigint, targetRadians: number) => void;
type PrismaticJointSetMotorSpeedFn = (jointHandle: bigint, motorSpeed: number) => void;
type PrismaticJointGetTranslationFn = (jointHandle: bigint) => number;
type GetShapeBodyHandleFn = (shapeHandle: bigint) => bigint;
type ShapeSetFrictionFn = (shapeHandle: bigint, friction: number) => void;
type ShapeSetRestitutionFn = (shapeHandle: bigint, restitution: number) => void;

type ModuleFactory = (options: { locateFile(path: string): string }) => Promise<CModule>;
type ModuleImport = { default: ModuleFactory };

function vec3(x = 0, y = 0, z = 0): Vec3 { return [x, y, z]; }
function versionedUrl(url: string, version: string | undefined): string { if (!version) return url; return `${url}${url.includes("?") ? "&" : "?"}v=${encodeURIComponent(version)}`; }
function wasmDirectory(variant: RuntimeLoadOptions["variant"]): string {
  if (variant === "profile") return "wasm/profile";
  if (variant === "growable") return "wasm/growable";
  return "wasm";
}

function readSlotCounts(ptr: number, heap32: Int32Array): SlotLimits {
  const base = ptr >> 2;
  return {
    worlds: heap32[base + 0]!,
    hulls: heap32[base + 1]!,
    meshes: heap32[base + 2]!,
    compounds: heap32[base + 3]!,
    humans: heap32[base + 4]!,
    heightFields: heap32[base + 5]!,
  };
}

function asBodyId(id: bigint): BodyId { return id as BodyId; }
function asShapeId(id: bigint): ShapeId { return id as ShapeId; }
function asJointId(id: bigint): JointId { return id as JointId; }

function writeVec3(out: Vec3, x: number, y: number, z: number): Vec3 {
  out[0] = x;
  out[1] = y;
  out[2] = z;
  return out;
}

const U64_MAX = 0xFFFFFFFF;

function defaults<T>(val: T | undefined, def: T): T { return val !== undefined ? val : def; }

class RuntimeBindings {
  constructor(protected readonly module: CModule) {}

  protected wrapNumber<T extends (...args: any[]) => number>(name: string, argTypes: readonly string[]): T {
    return this.module.cwrap(name, "number", argTypes) as T;
  }

  protected wrapBigInt<T extends (...args: any[]) => bigint>(name: string, argTypes: readonly string[]): T {
    return this.module.cwrap(name, "bigint", argTypes) as T;
  }

  protected wrapVoid<T extends (...args: any[]) => void>(name: string, argTypes: readonly string[]): T {
    return this.module.cwrap(name, null, argTypes) as T;
  }
}

export class Box3DRuntime extends RuntimeBindings implements RuntimeAPI {
  static async load(options: RuntimeLoadOptions = {}): Promise<Box3DRuntime> {
    if (options.poolSize !== undefined) globalThis.BOX3D_POOL_SIZE = options.poolSize;
    const locationHref = typeof window !== "undefined" ? window.location.href : globalThis.location.href;
    const configuredBase = options.baseUrl?.trim();
    const baseUrl =
      configuredBase !== undefined && configuredBase.length > 0
        ? configuredBase.endsWith("/")
          ? configuredBase
          : `${configuredBase}/`
        : typeof document === "undefined"
          ? "/"
          : new URL(".", locationHref).pathname;
    const moduleUrl = versionedUrl(`${baseUrl}${wasmDirectory(options.variant)}/box3d-web.js`, options.version);
    const absServerUrl = new URL(moduleUrl, locationHref).href;
    const moduleImport = (await import(/* @vite-ignore */ absServerUrl)) as ModuleImport;
    const module = await moduleImport.default({ locateFile(path: string): string { return versionedUrl(new URL(path, absServerUrl).href, options.version); } });
    return new Box3DRuntime(module);
  }

  private readonly createWorldFn = this.wrapNumber<CreateWorldFn>("b3wCreateWorld", ["number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly getSlotLimitsFn = this.wrapVoid<GetSlotLimitsFn>("b3wGetSlotLimits", ["number"]);
  private readonly getSlotUsageFn = this.wrapVoid<GetSlotUsageFn>("b3wGetSlotUsage", ["number"]);
  private readonly createBodyFn = this.wrapBigInt<CreateBodyFn>("b3wCreateBody", ["number", "number", "number", "number", "number", "number", "number"]);
  private readonly destroyWorldFn = this.wrapVoid<DestroyWorldFn>("b3wDestroyWorld", ["number"]);
  private readonly createBoxFn = this.wrapBigInt<CreateBoxFn>("b3wCreateBox", ["number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly createSphereFn = this.wrapBigInt<CreateSphereFn>("b3wCreateSphere", ["number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly createHullShapeFn = this.wrapBigInt<CreateHullShapeFn>("b3wCreateHullShape", ["bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createTransformedHullShapeFn = this.wrapBigInt<CreateTransformedHullShapeFn>("b3wCreateTransformedHullShape", ["bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createOffsetHullShapeFn = this.wrapBigInt<CreateOffsetHullShapeFn>("b3wCreateOffsetHullShape", ["bigint","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createSphereShapeFn = this.wrapBigInt<CreateSphereShapeFn>("b3wCreateSphereShape", ["bigint","number","number","number","number","number","number","number","number","number"]);
  private readonly createCapsuleShapeFn = this.wrapBigInt<CreateCapsuleShapeFn>("b3wCreateCapsuleShape", ["bigint","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createShapeFromHullFn = this.wrapBigInt<CreateShapeFromHullFn>("b3wCreateShapeFromHull", ["bigint","number","number","number","number","number","number","number","number"]);
  private readonly createTransformedShapeFromHullFn = this.wrapBigInt<CreateTransformedShapeFromHullFn>("b3wCreateTransformedShapeFromHull", ["bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createCylinderFn = this.wrapNumber<CreateCylinderFn>("b3wCreateCylinder", ["number","number","number","number"]);
  private readonly makeBoxHullFn = this.wrapNumber<MakeBoxHullFn>("b3wMakeBoxHull", ["number","number","number"]);
  private readonly makeTransformedBoxHullFn = this.wrapNumber<MakeTransformedBoxHullFn>("b3wMakeTransformedBoxHull", ["number","number","number","number","number","number","number","number","number","number"]);
  private readonly collideSpheresFn = this.wrapVoid<CollideSpheresFn>("b3wCollideSpheres", ["number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly collideCapsuleAndSphereFn = this.wrapVoid<CollideCapsuleSphereFn>("b3wCollideCapsuleAndSphere", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly collideHullAndSphereFn = this.wrapVoid<CollideHullSphereFn>("b3wCollideHullAndSphere", ["number","number","number","number","number","number","number","number","number","number"]);
  private readonly collideCapsulesFn = this.wrapVoid<CollideCapsulesFn>("b3wCollideCapsules", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly collideHullAndCapsuleFn = this.wrapVoid<CollideHullCapsuleFn>("b3wCollideHullAndCapsule", ["number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly collideHullsFn = this.wrapVoid<CollideHullsFn>("b3wCollideHulls", ["number","number","number","number","number","number","number"]);
  private readonly collideTriangleAndSphereFn = this.wrapVoid<CollideTriangleSphereFn>("b3wCollideTriangleAndSphere", ["number","number","number","number","number","number","number","number","number","number"]);
  private readonly collideTriangleAndCapsuleFn = this.wrapVoid<CollideTriangleCapsuleFn>("b3wCollideTriangleAndCapsule", ["number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly collideTriangleAndHullFn = this.wrapVoid<CollideTriangleHullFn>("b3wCollideTriangleAndHull", ["number","number","number","number","number","number","number","number","number"]);
  private readonly shapeCastPairFn = this.wrapVoid<ShapeCastPairFn>("b3wShapeCast", ["number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createGridMeshFn = this.wrapNumber<CreateGridMeshFn>("b3wCreateGridMesh", ["number","number","number","number","number","number"]);
  private readonly createWaveMeshFn = this.wrapNumber<CreateWaveMeshFn>("b3wCreateWaveMesh", ["number","number","number","number","number","number","number"]);
  private readonly createBoxMeshFn = this.wrapNumber<CreateBoxMeshFn>("b3wCreateBoxMesh", ["number","number","number","number","number","number","number","number"]);
  private readonly createHollowBoxMeshFn = this.wrapNumber<(worldHandle: number, cx: number, cy: number, cz: number, ex: number, ey: number, ez: number) => number>("b3wCreateHollowBoxMesh", ["number","number","number","number","number","number","number"]);
  private readonly createTorusMeshFn = this.wrapNumber<CreateTorusMeshFn>("b3wCreateTorusMesh", ["number","number","number","number","number"]);
  private readonly createMeshFn = this.wrapNumber<CreateMeshFn>("b3wCreateMesh", ["number","number","number","number","number","number","number"]);
  private readonly destroyMeshFn = this.wrapVoid<DestroyMeshFn>("b3wDestroyMesh", ["number"]);
  private readonly createMeshShapeFn = this.wrapBigInt<CreateMeshShapeFn>("b3wCreateMeshShape", ["bigint","number","number","number","number","number","number","number","number","number"]);
  private readonly shapeSetMeshFn = this.wrapVoid<ShapeSetMeshFn>("b3wShapeSetMesh", ["bigint","number","number","number","number"]);
  private readonly createWaveFn = this.wrapNumber<(worldHandle: number, rowCount: number, columnCount: number, scaleX: number, scaleY: number, scaleZ: number, rowFrequency: number, columnFrequency: number, makeHoles: number) => number>("b3wCreateWave", ["number","number","number","number","number","number","number","number","number"]);
  private readonly createGridHeightFieldFn = this.wrapNumber<(worldHandle: number, rowCount: number, columnCount: number, scaleX: number, scaleY: number, scaleZ: number, makeHoles: number) => number>("b3wCreateGridHeightField", ["number","number","number","number","number","number","number"]);
  private readonly destroyHeightFieldFn = this.wrapVoid<(heightFieldHandle: number) => void>("b3wDestroyHeightField", ["number"]);
  private readonly createHeightFieldShapeFn = this.wrapBigInt<(bodyHandle: bigint, heightFieldHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, isSensor: number) => bigint>("b3wCreateHeightFieldShape", ["bigint","number","number","number","number","number","number"]);
  private readonly createHullFromPointsFn = this.wrapNumber<CreateHullFromPointsFn>("b3wCreateHullFromPoints", ["number","number"]);
  private readonly createRockFn = this.wrapNumber<CreateRockFn>("b3wCreateRock", ["number"]);
  private readonly destroyHullFn = this.wrapVoid<DestroyHullFn>("b3wDestroyHull", ["number"]);
  private readonly getHullVertexCountFn = this.wrapNumber<GetHullVertexCountFn>("b3wGetHullVertexCount", ["number"]);
  private readonly getHullPointsFn = this.wrapNumber<GetHullPointsFn>("b3wGetHullPoints", ["number","number","number"]);
  private readonly createCompoundFn = this.wrapNumber<CreateCompoundFn>("b3wCreateCompound", ["number","number","number","number","number","number","number","number"]);
  private readonly createCompoundFromHullsFn = this.wrapNumber<CreateCompoundFromHullsFn>("b3wCreateCompoundFromHulls", ["number","number","number"]);
  private readonly createCompoundFromSpheresFn = this.wrapNumber<CreateCompoundFromSpheresFn>("b3wCreateCompoundFromSpheres", ["number","number","number"]);
  private readonly createCompoundFromMeshesFn = this.wrapNumber<CreateCompoundFromMeshesFn>("b3wCreateCompoundFromMeshes", ["number","number","number"]);
  private readonly destroyCompoundFn = this.wrapVoid<DestroyCompoundFn>("b3wDestroyCompound", ["number"]);
  private readonly getCompoundTreeHeightFn = this.wrapNumber<GetCompoundTreeHeightFn>("b3wGetCompoundTreeHeight", ["number"]);
  private readonly createCompoundShapeFn = this.wrapBigInt<CreateCompoundShapeFn>("b3wCreateCompoundShape", ["bigint","number","number"]);
  private readonly destroyBodyFn = this.wrapVoid<DestroyBodyFn>("b3wDestroyBody", ["bigint"]);
  private readonly bodyIsValidFn = this.wrapNumber<(bodyHandle: bigint) => number>("b3wBodyIsValid", ["bigint"]);
  private readonly bodyCastRayFn = this.wrapNumber<(bodyHandle: bigint, originX: number, originY: number, originZ: number, translationX: number, translationY: number, translationZ: number, categoryBits: number, maskBits: number, maxFraction: number, bodyPx: number, bodyPy: number, bodyPz: number, bodyQx: number, bodyQy: number, bodyQz: number, bodyQw: number, outHit: number, outPoint: number, outNormal: number, outFraction: number) => number>("b3wBodyCastRay", ["bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly destroyJointFn = this.wrapVoid<DestroyJointFn>("b3wDestroyJoint", ["bigint"]);
  private readonly getSensorBeginEventCountFn = this.wrapNumber<(worldHandle: number) => number>("b3wGetSensorBeginEventCount", ["number"]);
  private readonly getSensorBeginEventFn = this.wrapNumber<(worldHandle: number, index: number, outSensor: number, outVisitor: number) => number>("b3wGetSensorBeginEvent", ["number","number","number","number"]);
  private readonly getJointEventCountFn = this.wrapNumber<(worldHandle: number) => number>("b3wGetJointEventCount", ["number"]);
  private readonly getJointEventHandleFn = this.wrapBigInt<(worldHandle: number, index: number) => bigint>("b3wGetJointEventHandle", ["number","number"]);
  private readonly overlapAABBFn = this.wrapNumber<(worldHandle: number, minX: number, minY: number, minZ: number, maxX: number, maxY: number, maxZ: number, categoryBits: number, maskBits: number) => number>("b3wOverlapAABB", ["number","number","number","number","number","number","number","number","number"]);
  private readonly castShapeSphereFn = this.wrapNumber<(worldHandle: number, originX: number, originY: number, originZ: number, translationX: number, translationY: number, translationZ: number, radius: number, categoryBits: number, maskBits: number) => number>("b3wCastShapeSphere", ["number","number","number","number","number","number","number","number","number","number"]);
  private readonly setRandomSeedFn = this.wrapVoid<(seed: number) => void>("b3wSetRandomSeed", ["number"]);
  private readonly getRandomSeedFn = this.wrapNumber<() => number>("b3wGetRandomSeed", []);
  private readonly randomFloatRangeFn = this.wrapNumber<(lo: number, hi: number) => number>("b3wRandomFloatRange", ["number","number"]);
  private readonly randomVec3UniformFn = this.wrapVoid<(lo: number, hi: number, outVec: number) => void>("b3wRandomVec3Uniform", ["number","number","number"]);
  private readonly randomUnitVectorFn = this.wrapVoid<(outVec: number) => void>("b3wRandomUnitVector", ["number"]);
  private readonly randomQuatFn = this.wrapVoid<(outQuat: number) => void>("b3wRandomQuat", ["number"]);
  private readonly setBodyTransformFn = this.wrapVoid<SetBodyTransformFn>("b3wSetBodyTransform", ["bigint","number","number","number","number","number","number","number"]);
  private readonly setBodyLinearVelocityFn = this.wrapVoid<SetBodyLinearVelocityFn>("b3wSetBodyLinearVelocity", ["bigint","number","number","number"]);
  private readonly setBodyAngularVelocityFn = this.wrapVoid<SetBodyAngularVelocityFn>("b3wSetBodyAngularVelocity", ["bigint","number","number","number"]);
  private readonly getBodyLinearVelocityFn = this.wrapVoid<GetBodyVelocityFn>("b3wGetBodyLinearVelocity", ["bigint","number"]);
  private readonly getBodyAngularVelocityFn = this.wrapVoid<GetBodyVelocityFn>("b3wGetBodyAngularVelocity", ["bigint","number"]);
  private readonly setBodyAwakeFn = this.wrapVoid<SetBodyAwakeFn>("b3wSetBodyAwake", ["bigint","number"]);
  private readonly setBodyDampingFn = this.wrapVoid<SetBodyDampingFn>("b3wSetBodyDamping", ["bigint","number","number"]);
  private readonly getBodyLocalPointFn = this.wrapVoid<GetBodyLocalPointFn>("b3wGetBodyLocalPoint", ["bigint","number","number","number","number"]);
  private readonly createMotorJointFn = this.wrapBigInt<CreateMotorJointFn>("b3wCreateMotorJoint", ["number","bigint","bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createFilterJointFn = this.wrapBigInt<CreateFilterJointFn>("b3wCreateFilterJoint", ["number","bigint","bigint"]);
  private readonly createRevoluteJointFn = this.wrapBigInt<CreateRevoluteJointFn>("b3wCreateRevoluteJoint", ["number","bigint","bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createSphericalJointFn = this.wrapBigInt<CreateSphericalJointFn>("b3wCreateSphericalJoint", ["number","bigint","bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createHumanFn = this.wrapNumber<CreateHumanFn>("b3wCreateHuman", ["number","number","number","number","number","number","number","number","number"]);
  private readonly getHumanBoneBodyFn = this.wrapBigInt<GetHumanBoneBodyFn>("b3wGetHumanBoneBody", ["number","number"]);
  private readonly getHumanBoneCountFn = this.wrapNumber<GetHumanBoneCountFn>("b3wGetHumanBoneCount", []);
  private readonly humanSetVelocityFn = this.wrapVoid<HumanSetVelocityFn>("b3wHumanSetVelocity", ["number","number","number","number"]);
  private readonly humanSetBulletFn = this.wrapVoid<HumanSetBulletFn>("b3wHumanSetBullet", ["number","number"]);
  private readonly humanSetJointFrictionTorqueFn = this.wrapVoid<HumanSetJointFloatFn>("b3wHumanSetJointFrictionTorque", ["number","number"]);
  private readonly humanSetJointSpringHertzFn = this.wrapVoid<HumanSetJointFloatFn>("b3wHumanSetJointSpringHertz", ["number","number"]);
  private readonly humanSetJointDampingRatioFn = this.wrapVoid<HumanSetJointFloatFn>("b3wHumanSetJointDampingRatio", ["number","number"]);
  private readonly humanCreateParallelAnchorsFn = this.wrapVoid<(humanHandle: number) => void>("b3wHumanCreateParallelAnchors", ["number"]);
  private readonly getHumanAnchorBodyFn = this.wrapBigInt<GetHumanBoneBodyFn>("b3wGetHumanAnchorBody", ["number","number"]);
  private readonly enableWorldSleepFn = this.wrapVoid<WorldEnableBoolFn>("b3wEnableSleeping", ["number", "number"]);
  private readonly enableWorldContinuousFn = this.wrapVoid<WorldEnableBoolFn>("b3wEnableContinuous", ["number", "number"]);
  private readonly enableWorldWarmStartingFn = this.wrapVoid<WorldEnableBoolFn>("b3wEnableWarmStarting", ["number", "number"]);
  private readonly setWorldProfileLevelFn = this.wrapVoid<WorldSetProfileLevelFn>("b3wSetProfileLevel", ["number", "number"]);
  private readonly getWorldProfileLevelFn = this.wrapNumber<WorldGetProfileLevelFn>("b3wGetProfileLevel", ["number"]);
  private readonly setWorldContactTuningFn = this.wrapVoid<WorldSetContactTuningFn>("b3wSetContactTuning", ["number", "number", "number", "number"]);
  private readonly setWorldContactRecycleDistanceFn = this.wrapVoid<WorldSetFloatFn>("b3wSetContactRecycleDistance", ["number", "number"]);
  private readonly setWorldWorkerCountFn = this.wrapVoid<WorldSetWorkerCountFn>("b3wSetWorkerCount", ["number", "number"]);
  private readonly getWorldCountersFn = this.wrapVoid<GetWorldCountersFn>("b3wGetWorldCounters", ["number", "number"]);
  private readonly getWorldProfileFn = this.wrapVoid<GetWorldProfileFn>("b3wGetWorldProfile", ["number", "number"]);
  private readonly getWorldAwakeBodyCountFn = this.wrapNumber<GetWorldAwakeBodyCountFn>("b3wGetWorldAwakeBodyCount", ["number"]);
  private readonly checkThreadingSupportFn = this.wrapNumber<CheckThreadingSupportFn>("b3wCheckThreadingSupport", []);
  private readonly getWorldWorkerCountFn = this.wrapNumber<GetWorldWorkerCountFn>("b3wGetWorldWorkerCount", ["number"]);
  private readonly writeBodyTransformsFn = this.wrapVoid<WriteBodyTransformsFn>("b3wWriteBodyTransforms", ["number", "number", "number", "number", "number", "number"]);
  private readonly writeBodyTransformsLightFn = this.wrapVoid<WriteBodyTransformsLightFn>("b3wWriteBodyTransformsLight", ["number", "number", "number", "number", "number", "number"]);
  private readonly configureBodyMoveTrackingFn = this.wrapVoid<ConfigureBodyMoveTrackingFn>("b3wConfigureBodyMoveTracking", ["number", "number"]);
  private readonly clearBodyMoveTrackingFn = this.wrapVoid<ClearBodyMoveTrackingFn>("b3wClearBodyMoveTracking", []);
  private readonly scatterBodyMoveEventsFn = this.wrapNumber<ScatterBodyMoveEventsFn>("b3wScatterBodyMoveEvents", ["number", "number", "number", "number", "number", "number"]);
  private readonly getBodyMoveEventCountFn = this.wrapNumber<GetBodyMoveEventCountFn>("b3wGetBodyMoveEventCount", ["number"]);
  private readonly rayCastClosestFn = this.wrapVoid<RayCastClosestFn>("b3wRayCastClosest", ["number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly bodyEnableFn = this.wrapVoid<BodyEnableFn>("b3wBodyEnable", ["bigint"]);
  private readonly bodyDisableFn = this.wrapVoid<BodyEnableFn>("b3wBodyDisable", ["bigint"]);
  private readonly bodyIsEnabledFn = this.wrapNumber<BodyIsEnabledFn>("b3wBodyIsEnabled", ["bigint"]);
  private readonly getBodyMassFn = this.wrapNumber<GetBodyMassFn>("b3wGetBodyMass", ["bigint"]);
  private readonly getBodyLocalRotationalInertiaFn = this.wrapVoid<GetBodyLocalRotationalInertiaFn>("b3wGetBodyLocalRotationalInertia", ["bigint","number"]);
  private readonly getBodyWorldCenterFn = this.wrapVoid<GetBodyWorldCenterFn>("b3wGetBodyWorldCenter", ["bigint","number"]);
  private readonly getBodyWorldPointFn = this.wrapVoid<GetBodyWorldPointFn>("b3wGetBodyWorldPoint", ["bigint","number","number","number","number"]);
  private readonly getBodyLocalPointVelocityFn = this.wrapVoid<GetBodyLocalPointVelocityFn>("b3wGetBodyLocalPointVelocity", ["bigint","number","number","number","number"]);
  private readonly getBodyWorldPointVelocityFn = this.wrapVoid<GetBodyWorldPointVelocityFn>("b3wGetBodyWorldPointVelocity", ["bigint","number","number","number","number"]);
  private readonly createPrismaticJointFn = this.wrapBigInt<CreatePrismaticJointFn>("b3wCreatePrismaticJoint", ["number","bigint","bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createWeldJointFn = this.wrapBigInt<CreateWeldJointFn>("b3wCreateWeldJoint", ["number","bigint","bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createDistanceJointFn = this.wrapBigInt<CreateDistanceJointFn>("b3wCreateDistanceJoint", ["number","bigint","bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createParallelJointFn = this.wrapBigInt<CreateParallelJointFn>("b3wCreateParallelJoint", ["number","bigint","bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createWheelJointFn = this.wrapBigInt<CreateWheelJointFn>("b3wCreateWheelJoint", ["number","bigint","bigint","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly getStallThresholdFn = this.wrapNumber<GetStallThresholdFn>("b3wGetStallThreshold", []);
  private readonly setStallThresholdFn = this.wrapVoid<SetStallThresholdFn>("b3wSetStallThreshold", ["number"]);
  private readonly worldExplodeFn = this.wrapVoid<WorldExplodeFn>("b3wWorldExplode", ["number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly getJointConstraintForceFn = this.wrapVoid<GetJointVec3Fn>("b3wGetJointConstraintForce", ["bigint","number"]);
  private readonly getJointConstraintTorqueFn = this.wrapVoid<GetJointVec3Fn>("b3wGetJointConstraintTorque", ["bigint","number"]);
  private readonly getJointLinearSeparationFn = this.wrapNumber<GetJointLinearSeparationFn>("b3wGetJointLinearSeparation", ["bigint"]);
  private readonly revoluteJointSetTargetAngleFn = this.wrapVoid<RevoluteJointSetTargetAngleFn>("b3wRevoluteJointSetTargetAngle", ["bigint","number"]);
  private readonly prismaticJointSetMotorSpeedFn = this.wrapVoid<PrismaticJointSetMotorSpeedFn>("b3wPrismaticJointSetMotorSpeed", ["bigint","number"]);
  private readonly prismaticJointGetTranslationFn = this.wrapNumber<PrismaticJointGetTranslationFn>("b3wPrismaticJointGetTranslation", ["bigint"]);
  private readonly getShapeBodyHandleFn = this.wrapBigInt<GetShapeBodyHandleFn>("b3wGetShapeBodyHandle", ["bigint"]);
  private readonly stepFn = this.wrapVoid<StepFn>("b3wStep", ["number", "number", "number"]);
  private readonly getBodyTransformFn = this.wrapVoid<GetBodyTransformFn>("b3wGetBodyTransform", ["bigint","number"]);
  private readonly setDensityFn = this.wrapVoid<ShapeSetDensityFn>("b3wShapeSetDensity", ["bigint","number","number"]);
  private readonly setFrictionFn = this.wrapVoid<ShapeSetFrictionFn>("b3wShapeSetFriction", ["bigint","number"]);
  private readonly setRestitutionFn = this.wrapVoid<ShapeSetRestitutionFn>("b3wShapeSetRestitution", ["bigint","number"]);
  private readonly setSurfaceMaterialFn = this.wrapVoid<ShapeSetSurfaceMaterialFn>("b3wShapeSetSurfaceMaterial", ["bigint","number","number","number","number","number","number"]);
  private readonly setFilterFn = this.wrapVoid<ShapeSetFilterFn>("b3wShapeSetFilter", ["bigint","number","number","number","number"]);
  private readonly getBodyShapeCountFn = this.wrapNumber<GetBodyShapeCountFn>("b3wGetBodyShapeCount", ["bigint"]);
  private readonly getBodyShapesFn = this.wrapNumber<GetBodyShapesFn>("b3wGetBodyShapes", ["bigint","number","number"]);
  private readonly destroyShapeFn = this.wrapVoid<DestroyShapeFn>("b3wDestroyShape", ["bigint","number"]);
  private readonly enableShapeSensorEventsFn = this.wrapVoid<ShapeEnableBoolFn>("b3wShapeEnableSensorEvents", ["bigint","number"]);
  private readonly enableShapeContactEventsFn = this.wrapVoid<ShapeEnableBoolFn>("b3wShapeEnableContactEvents", ["bigint","number"]);
  private readonly enableShapePreSolveEventsFn = this.wrapVoid<ShapeEnableBoolFn>("b3wShapeEnablePreSolveEvents", ["bigint","number"]);
  private readonly enableShapeHitEventsFn = this.wrapVoid<ShapeEnableBoolFn>("b3wShapeEnableHitEvents", ["bigint","number"]);
  private readonly setShapeSphereFn = this.wrapVoid<ShapeSetSphereFn>("b3wShapeSetSphere", ["bigint","number","number","number","number"]);
  private readonly setShapeCapsuleFn = this.wrapVoid<ShapeSetCapsuleFn>("b3wShapeSetCapsule", ["bigint","number","number","number","number","number","number","number"]);
  private readonly applyShapeWindFn = this.wrapVoid<ShapeApplyWindFn>("b3wShapeApplyWind", ["bigint","number","number","number","number","number","number","number"]);
  private readonly bodyIsAwakeFn = this.wrapNumber<BodyIsAwakeFn>("b3wBodyIsAwake", ["bigint"]);
  private readonly getBodyDebugColorFn = this.wrapNumber<GetBodyDebugColorFn>("b3wGetBodyDebugColor", ["bigint"]);
  private readonly getBodyTypeFn = this.wrapNumber<GetBodyTypeFn>("b3wGetBodyType", ["bigint"]);
  private readonly setBodyTypeFn = this.wrapVoid<BodySetTypeFn>("b3wSetBodyType", ["bigint","number"]);
  private readonly setBodyNameFn = this.wrapVoid<BodySetNameFn>("b3wSetBodyName", ["bigint","number"]);
  private readonly setBodyGravityScaleFn = this.wrapVoid<BodySetGravityScaleFn>("b3wSetBodyGravityScale", ["bigint","number"]);
  private readonly setBodySleepThresholdFn = this.wrapVoid<BodySetSleepThresholdFn>("b3wSetBodySleepThreshold", ["bigint","number"]);
  private readonly enableBodySleepFn = this.wrapVoid<BodyEnableSleepFn>("b3wEnableBodySleep", ["bigint","number"]);
  private readonly setBodyBulletFn = this.wrapVoid<BodySetBulletFn>("b3wSetBodyBullet", ["bigint","number"]);
  private readonly allowBodyFastRotationFn = this.wrapVoid<BodyAllowFastRotationFn>("b3wAllowBodyFastRotation", ["bigint","number"]);
  private readonly isBodyFastRotationAllowedFn = this.wrapNumber<BodyIsFastRotationAllowedFn>("b3wIsBodyFastRotationAllowed", ["bigint"]);
  private readonly enableBodyContactRecyclingFn = this.wrapVoid<BodyEnableContactRecyclingFn>("b3wEnableBodyContactRecycling", ["bigint","number"]);
  private readonly enableBodyHitEventsFn = this.wrapVoid<BodyEnableHitEventsFn>("b3wEnableBodyHitEvents", ["bigint","number"]);
  private readonly setBodyMotionLocksFn = this.wrapVoid<BodySetMotionLocksFn>("b3wSetBodyMotionLocks", ["bigint","number","number","number","number","number","number"]);
  private readonly setBodyMassDataFn = this.wrapVoid<BodySetMassDataFn>("b3wSetBodyMassData", ["bigint","number","number","number","number","number"]);
  private readonly getBodyMassDataFn = this.wrapVoid<BodyGetMassDataFn>("b3wGetBodyMassData", ["bigint","number"]);
  private readonly applyBodyMassFromShapesFn = this.wrapVoid<BodyApplyMassFromShapesFn>("b3wApplyBodyMassFromShapes", ["bigint"]);
  private readonly setBodyTargetTransformFn = this.wrapVoid<BodySetTargetTransformFn>("b3wSetBodyTargetTransform", ["bigint","number","number","number","number","number","number","number","number","number"]);
  private readonly applyLinearImpulseFn = this.wrapVoid<ApplyLinearImpulseFn>("b3wApplyLinearImpulse", ["bigint","number","number","number","number","number","number","number"]);
  private readonly applyLinearImpulseToCenterFn = this.wrapVoid<ApplyLinearImpulseToCenterFn>("b3wApplyLinearImpulseToCenter", ["bigint","number","number","number","number"]);
  private readonly b3wSinFn = this.wrapNumber<(radians: number) => number>("b3wSin", ["number"]);
  private readonly b3wCosFn = this.wrapNumber<(radians: number) => number>("b3wCos", ["number"]);
  private readonly b3wCosfFn = this.wrapNumber<(radians: number) => number>("b3wCosf", ["number"]);
  private readonly b3wSinfFn = this.wrapNumber<(radians: number) => number>("b3wSinf", ["number"]);
  private readonly makeQuatFromAxisAngleFn = this.wrapVoid<MakeQuatFromAxisAngleFn>("b3wMakeQuatFromAxisAngle", ["number", "number", "number", "number", "number"]);
  private readonly rotateVectorFn = this.wrapVoid<RotateVectorFn>("b3wRotateVector", ["number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly randomVec3Fn = this.wrapVoid<RandomVec3Fn>("b3wRandomVec3", ["number", "number", "number", "number", "number", "number", "number"]);
  private readonly lerpVec3Fn = this.wrapVoid<LerpVec3Fn>("b3wLerpVec3", ["number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly getLengthAndNormalizeFn = this.wrapNumber<GetLengthAndNormalizeFn>("b3wGetLengthAndNormalize", ["number", "number", "number", "number"]);
  private readonly computeQuatBetweenUnitVectorsFn = this.wrapVoid<ComputeQuatBetweenUnitVectorsFn>("b3wComputeQuatBetweenUnitVectors", ["number", "number", "number", "number", "number", "number", "number"]);
  private readonly invMulQuatFn = this.wrapVoid<InvMulQuatFn>("b3wInvMulQuat", ["number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly transformPtr: number;
  private readonly pointPtr: number;
  private readonly massDataPtr: number;
  private readonly profilePtr: number;
  private readonly inertiaPtr: number;
  private readonly slotCountsPtr: number;
  private readonly xfAPtr: number;
  private readonly xfBPtr: number;
  private readonly trianglePtr: number;
  private readonly manifoldPtr: number;
  private readonly shapeCastOutPtr: number;
  readonly limits: SlotLimits;

  constructor(module: CModule) {
    super(module);
    this.slotCountsPtr = module._malloc(6 * 4);
    this.getSlotLimitsFn(this.slotCountsPtr);
    this.limits = readSlotCounts(this.slotCountsPtr, module.HEAP32);
    this.transformPtr = module._malloc(7 * 4);
    this.pointPtr = module._malloc(3 * 4);
    this.massDataPtr = module._malloc(2 * 4);
    this.profilePtr = module._malloc(23 * 4);
    this.inertiaPtr = module._malloc(9 * 4);
    this.xfAPtr = module._malloc(7 * 4);
    this.xfBPtr = module._malloc(7 * 4);
    this.trianglePtr = module._malloc(9 * 4);
    this.manifoldPtr = module._malloc((B3W_MANIFOLD_HEADER_FLOATS + B3W_MANIFOLD_MAX_POINTS * B3W_MANIFOLD_POINT_FLOATS) * 4);
    this.shapeCastOutPtr = module._malloc(9 * 4);
  }

  getSlotUsage(): SlotUsage {
    this.getSlotUsageFn(this.slotCountsPtr);
    return readSlotCounts(this.slotCountsPtr, this.module.HEAP32);
  }

  private requireSlotHandle<T>(handle: number, kind: SlotKind): T {
    if (handle !== 0) return handle as T;
    const usage = this.getSlotUsage();
    throw new SlotExhaustedError(kind, usage[kind], this.limits[kind]);
  }

  createWorld(options: WorldOptions = {}): PhysicsWorld {
    const gravity = options.gravity ?? vec3(0, -10, 0);
    const workerCount = options.workerCount ?? 4;
    const capacity = options.capacity ?? {};
    const worldHandle = this.createWorldFn(
      gravity[0], gravity[1], gravity[2], workerCount,
      capacity.staticShapeCount ?? 0,
      capacity.dynamicShapeCount ?? 0,
      capacity.staticBodyCount ?? 0,
      capacity.dynamicBodyCount ?? 0,
      capacity.contactCount ?? 0,
    );
    return new PhysicsWorld(this, this.requireSlotHandle<WorldHandle>(worldHandle, "worlds"));
  }

  destroy(): void {
    this.module._free(this.slotCountsPtr);
    this.module._free(this.transformPtr);
    this.module._free(this.pointPtr);
    this.module._free(this.massDataPtr);
    this.module._free(this.profilePtr);
    this.module._free(this.inertiaPtr);
    this.module._free(this.xfAPtr);
    this.module._free(this.xfBPtr);
    this.module._free(this.trianglePtr);
    this.module._free(this.manifoldPtr);
    this.module._free(this.shapeCastOutPtr);
  }

  /**
   * Allocate transform-batch scratch for one body list.
   * Must not share buffers across lists: caching by capacity alone breaks multi-world
   * sync (same count ⇒ last writer’s handles win, every mesh gets that world’s poses).
   */
  allocBodyBatchBuffers(capacity: number): BodyBatchBuffers {
    return {
      bodyHandlesPtr: this.module._malloc(capacity * 8),
      positionsPtr: this.module._malloc(capacity * 3 * 4),
      rotationsPtr: this.module._malloc(capacity * 4 * 4),
      awakePtr: this.module._malloc(capacity),
      colorsPtr: this.module._malloc(capacity * 4),
      capacity,
    };
  }

  freeBodyBatchBuffers(buffers: BodyBatchBuffers): void {
    this.module._free(buffers.bodyHandlesPtr);
    this.module._free(buffers.positionsPtr);
    this.module._free(buffers.rotationsPtr);
    this.module._free(buffers.awakePtr);
    this.module._free(buffers.colorsPtr);
  }

  private readPointInto(out: Vec3): Vec3 {
    const heap = this.module.HEAPF32;
    const base = this.pointPtr >> 2;
    return writeVec3(out, heap[base + 0], heap[base + 1], heap[base + 2]);
  }

  getMemoryView(): RuntimeMemoryView32 {
    return { heapF32: this.module.HEAPF32, heapU8: this.module.HEAPU8, heap32: this.module.HEAP32 };
  }

  /** Shared WASM memory when built with pthreads (fixed or profile variants). Undefined/growable may still expose it. */
  getWasmMemory(): WebAssembly.Memory | undefined {
    return this.module.wasmMemory;
  }

  writeBodyHandles(buffers: BodyBatchBuffers, bodyHandles: readonly BodyId[]): void {
    const heap = this.module.HEAPU64;
    const base = buffers.bodyHandlesPtr >>> 3; // byte offset / 8
    for (let i = 0; i < bodyHandles.length; i++) heap[base + i] = bodyHandles[i];
  }

  private applyBodyDef(bodyHandle: BodyId, def: BodyDef): void {
    if (def.rotation) this.setBodyTransform(bodyHandle, def.position ?? vec3(), def.rotation);
    if (def.linearVelocity) this.setBodyLinearVelocity(bodyHandle, def.linearVelocity);
    if (def.angularVelocity) this.setBodyAngularVelocity(bodyHandle, def.angularVelocity);
    if (def.linearDamping !== undefined || def.angularDamping !== undefined) this.setBodyDamping(bodyHandle, def.linearDamping ?? 0, def.angularDamping ?? 0);
    if (def.gravityScale !== undefined) this.setBodyGravityScale(bodyHandle, def.gravityScale);
    if (def.sleepThreshold !== undefined) this.setBodySleepThreshold(bodyHandle, def.sleepThreshold);
    if (def.isBullet !== undefined) this.setBodyBullet(bodyHandle, def.isBullet);
    if (def.isEnabled !== undefined && !def.isEnabled) this.setBodyType(bodyHandle, BodyType.Static); // approximate
    if (def.allowFastRotation !== undefined) this.allowBodyFastRotation(bodyHandle, def.allowFastRotation);
    if (def.enableSleep !== undefined) this.enableBodySleep(bodyHandle, def.enableSleep);
    if (def.isAwake !== undefined) this.setBodyAwake(bodyHandle, def.isAwake);
    if (def.enableContactRecycling !== undefined) this.enableBodyContactRecycling(bodyHandle, def.enableContactRecycling);
  }

  private applyShapeDef(shapeHandle: ShapeId, def: ShapeDef): void {
    if (def.friction !== undefined || def.restitution !== undefined || def.rollingResistance !== undefined || def.tangentVelocity !== undefined) {
      this.setShapeSurfaceMaterial(shapeHandle, {
        friction: def.friction,
        restitution: def.restitution,
        rollingResistance: def.rollingResistance,
        tangentVelocity: def.tangentVelocity,
      });
    }
    if (def.isSensor) { /* isSensor is applied at shape creation */ }
    if (def.enableSensorEvents !== undefined) this.enableShapeSensorEvents(shapeHandle, def.enableSensorEvents);
    if (def.enableContactEvents !== undefined) this.enableShapeContactEvents(shapeHandle, def.enableContactEvents);
    if (def.enableHitEvents !== undefined) this.enableShapeHitEvents(shapeHandle, def.enableHitEvents);
    if (def.enablePreSolveEvents !== undefined) this.enableShapePreSolveEvents(shapeHandle, def.enablePreSolveEvents);
    if (def.categoryBits !== undefined || def.maskBits !== undefined || def.groupIndex !== undefined || def.invokeContactCreation !== undefined) {
      this.setShapeFilter(shapeHandle, def.categoryBits ?? U64_MAX, def.maskBits ?? U64_MAX, def.groupIndex ?? 0, def.invokeContactCreation ?? false);
    }
  }

  createBody(worldHandle: WorldHandle, def: BodyDef = {}): BodyId {
    const p = def.position ?? vec3();
    const bodyId = asBodyId(this.createBodyFn(worldHandle, def.type ?? BodyType.Static, p[0], p[1], p[2], defaults(def.enableSleep, true) ? 1 : 0, defaults(def.isAwake, true) ? 1 : 0));
    if (bodyId === 0n) throw new Error("b3wCreateBody failed");
    this.applyBodyDef(bodyId, def);
    return bodyId;
  }

  createBox(worldHandle: WorldHandle, options: BoxOptions): BodyId {
    const s = options.size;
    const p = options.position ?? vec3();
    const bodyId = asBodyId(this.createBoxFn(worldHandle, p[0], p[1], p[2], s[0], s[1], s[2], options.static ? 1 : 0, options.density ?? 1));
    if (bodyId === 0n) throw new Error("b3wCreateBox failed");
    if (options.friction !== undefined || options.restitution !== undefined || options.rollingResistance !== undefined) {
      const shapeHandle = this.getBodyShapes(bodyId)[0];
      if (shapeHandle !== undefined)
        this.setShapeSurfaceMaterial(shapeHandle, { friction: options.friction, restitution: options.restitution, rollingResistance: options.rollingResistance });
    }
    return bodyId;
  }

  createBoxWithShape(worldHandle: WorldHandle, options: BoxOptions): ShapeHandle {
    const bodyHandle = this.createBox(worldHandle, options);
    const shapeHandle = this.getBodyShapes(bodyHandle)[0];
    if (shapeHandle === undefined) throw new Error("createBox did not produce a shape handle");
    return { bodyHandle, shapeHandle };
  }

  createSphere(worldHandle: WorldHandle, options: SphereOptions): BodyId {
    const p = options.position ?? vec3();
    const v = options.velocity ?? vec3();
    const bodyId = asBodyId(this.createSphereFn(worldHandle, p[0], p[1], p[2], options.radius, v[0], v[1], v[2], options.density ?? 1));
    if (bodyId === 0n) throw new Error("b3wCreateSphere failed");
    if (options.isBullet) this.setBodyBullet(bodyId, true);
    if (options.friction !== undefined || options.restitution !== undefined || options.rollingResistance !== undefined) {
      const shapeHandle = this.getBodyShapes(bodyId)[0];
      if (shapeHandle !== undefined)
        this.setShapeSurfaceMaterial(shapeHandle, { friction: options.friction, restitution: options.restitution, rollingResistance: options.rollingResistance });
    }
    return bodyId;
  }

  createSphereWithShape(worldHandle: WorldHandle, options: SphereOptions): ShapeHandle {
    const bodyHandle = this.createSphere(worldHandle, options);
    const shapeHandle = this.getBodyShapes(bodyHandle)[0];
    if (shapeHandle === undefined) throw new Error("createSphere did not produce a shape handle");
    return { bodyHandle, shapeHandle };
  }

  createSphereShape(bodyHandle: BodyId, center: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = asShapeId(this.createSphereShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, center[0], center[1], center[2], radius, def.invokeContactCreation === false ? 0 : 1));
    if (shapeHandle === 0n) throw new Error("createSphereShapeFn failed");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }

  createCapsuleShape(bodyHandle: BodyId, center1: Vec3, center2: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = asShapeId(this.createCapsuleShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, center1[0], center1[1], center1[2], center2[0], center2[1], center2[2], radius, def.isSensor ? 1 : 0));
    if (shapeHandle === 0n) throw new Error("createCapsuleShapeFn failed");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }

  createHullShape(bodyHandle: BodyId, halfWidths: Vec3, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = asShapeId(this.createHullShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, def.updateBodyMass === false ? 0 : 1, 0, 0, 0, 0, 0, 0, 1, halfWidths[0], halfWidths[1], halfWidths[2], def.isSensor ? 1 : 0));
    if (shapeHandle === 0n) throw new Error("createHullShapeFn failed");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }

  createTransformedHullShape(bodyHandle: BodyId, halfWidths: Vec3, transform: { position?: Vec3; rotation?: Quat } = {}, scale: Vec3 = [1,1,1], def: ShapeDef = {}): ShapeHandle {
    const pos = transform.position ?? vec3();
    const rot = transform.rotation ?? [0,0,0,1];
    const shapeHandle = asShapeId(this.createTransformedHullShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, pos[0], pos[1], pos[2], rot[0], rot[1], rot[2], rot[3], halfWidths[0], halfWidths[1], halfWidths[2], scale[0], scale[1], scale[2]));
    if (shapeHandle === 0n) throw new Error("createTransformedHullShapeFn failed");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }

  /** `b3MakeOffsetBoxHull` + `b3CreateHullShape` — offset baked into hull vertices. */
  createOffsetHullShape(bodyHandle: BodyId, halfWidths: Vec3, offset: Vec3, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = asShapeId(this.createOffsetHullShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, def.updateBodyMass === false ? 0 : 1, halfWidths[0], halfWidths[1], halfWidths[2], offset[0], offset[1], offset[2]));
    if (shapeHandle === 0n) throw new Error("createOffsetHullShapeFn failed");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }

  createShapeFromHull(bodyHandle: BodyId, hullHandle: HullHandle, def: ShapeDef = {}): ShapeId {
    const shapeHandle = asShapeId(this.createShapeFromHullFn(bodyHandle, hullHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, def.updateBodyMass === false ? 0 : 1, def.explosionScale ?? 1, def.isSensor ? 1 : 0));
    if (shapeHandle === 0n) throw new Error("createShapeFromHullFn failed");
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return asShapeId(shapeHandle);
  }

  createTransformedShapeFromHull(bodyHandle: BodyId, hullHandle: HullHandle, transform: { position?: Vec3; rotation?: Quat } = {}, scale: Vec3 = [1, 1, 1], def: ShapeDef = {}): ShapeId {
    const pos = transform.position ?? vec3();
    const rot = transform.rotation ?? [0, 0, 0, 1];
    const shapeHandle = asShapeId(this.createTransformedShapeFromHullFn(bodyHandle, hullHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, def.updateBodyMass === false ? 0 : 1, pos[0], pos[1], pos[2], rot[0], rot[1], rot[2], rot[3], scale[0], scale[1], scale[2]));
    if (shapeHandle === 0n) throw new Error("createTransformedShapeFromHullFn failed");
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return asShapeId(shapeHandle);
  }

  createCylinder(height: number, radius: number, yOffset = 0, sides = 12): HullHandle {
    return this.requireSlotHandle<HullHandle>(this.createCylinderFn(height, radius, yOffset, sides), "hulls");
  }
  createGridMesh(worldHandle: WorldHandle, xCount: number, zCount: number, cellWidth: number, materialCount = 1, identifyEdges = true): MeshHandle {
    return this.requireSlotHandle<MeshHandle>(this.createGridMeshFn(worldHandle, xCount, zCount, cellWidth, materialCount, identifyEdges ? 1 : 0), "meshes");
  }
  createWaveMesh(worldHandle: WorldHandle, xCount: number, zCount: number, cellWidth: number, amplitude: number, rowFrequency: number, columnFrequency: number): MeshHandle {
    return this.requireSlotHandle<MeshHandle>(this.createWaveMeshFn(worldHandle, xCount, zCount, cellWidth, amplitude, rowFrequency, columnFrequency), "meshes");
  }
  createBoxMesh(worldHandle: WorldHandle, center: Vec3, extent: Vec3, identifyEdges = true): MeshHandle {
    return this.requireSlotHandle<MeshHandle>(this.createBoxMeshFn(worldHandle, center[0], center[1], center[2], extent[0], extent[1], extent[2], identifyEdges ? 1 : 0), "meshes");
  }
  createHollowBoxMesh(worldHandle: WorldHandle, center: Vec3, extent: Vec3): MeshHandle {
    return this.requireSlotHandle<MeshHandle>(this.createHollowBoxMeshFn(worldHandle, center[0], center[1], center[2], extent[0], extent[1], extent[2]), "meshes");
  }
  createTorusMesh(worldHandle: WorldHandle, radialResolution: number, tubularResolution: number, radius: number, thickness: number): MeshHandle {
    return this.requireSlotHandle<MeshHandle>(this.createTorusMeshFn(worldHandle, radialResolution, tubularResolution, radius, thickness), "meshes");
  }
  /** Create a triangle mesh from packed xyz vertices and triangle indices (`b3CreateMesh`). */
  createMesh(
    worldHandle: WorldHandle,
    vertices: ArrayLike<number>,
    indices: ArrayLike<number>,
    options: { useMedianSplit?: boolean; identifyEdges?: boolean } = {},
  ): MeshHandle {
    const vertexCount = Math.floor(vertices.length / 3);
    const triangleCount = Math.floor(indices.length / 3);
    const vertPtr = this.module._malloc(vertexCount * 3 * 4);
    const indexPtr = this.module._malloc(triangleCount * 3 * 4);
    const heapF = this.module.HEAPF32;
    const heapI = this.module.HEAP32;
    const vertBase = vertPtr >> 2;
    const indexBase = indexPtr >> 2;
    for (let i = 0; i < vertexCount * 3; i++) heapF[vertBase + i] = vertices[i]!;
    for (let i = 0; i < triangleCount * 3; i++) heapI[indexBase + i] = indices[i]!;
    const meshHandle = this.createMeshFn(
      worldHandle,
      vertPtr,
      vertexCount,
      indexPtr,
      triangleCount,
      options.useMedianSplit === false ? 0 : 1,
      options.identifyEdges ? 1 : 0,
    );
    this.module._free(vertPtr);
    this.module._free(indexPtr);
    return this.requireSlotHandle<MeshHandle>(meshHandle, "meshes");
  }
  destroyMesh(meshHandle: MeshHandle): void { this.destroyMeshFn(meshHandle); }
  createHullFromPoints(points: number[]): HullHandle {
    const ptr = this.module._malloc(points.length * 4);
    const heap = this.module.HEAPF32;
    const base = ptr >> 2;
    for (let i = 0; i < points.length; i++) heap[base + i] = points[i];
    const hullHandle = this.createHullFromPointsFn(points.length / 3, ptr);
    this.module._free(ptr);
    return this.requireSlotHandle<HullHandle>(hullHandle, "hulls");
  }
  destroyHull(hullHandle: HullHandle): void { this.destroyHullFn(hullHandle); }
  makeBoxHull(halfWidths: Vec3): HullHandle {
    return this.requireSlotHandle<HullHandle>(this.makeBoxHullFn(halfWidths[0], halfWidths[1], halfWidths[2]), "hulls");
  }
  makeTransformedBoxHull(halfWidths: Vec3, transform: { position?: Vec3; rotation?: Quat } = {}): HullHandle {
    const pos = transform.position ?? [0, 0, 0];
    const rot = transform.rotation ?? [0, 0, 0, 1];
    return this.requireSlotHandle<HullHandle>(this.makeTransformedBoxHullFn(halfWidths[0], halfWidths[1], halfWidths[2], pos[0], pos[1], pos[2], rot[0], rot[1], rot[2], rot[3]), "hulls");
  }

  private writeWorldTransform(ptr: number, transform: WorldTransform): void {
    const heap = this.module.HEAPF32;
    const base = ptr >> 2;
    heap[base + 0] = transform.position[0];
    heap[base + 1] = transform.position[1];
    heap[base + 2] = transform.position[2];
    heap[base + 3] = transform.rotation[0];
    heap[base + 4] = transform.rotation[1];
    heap[base + 5] = transform.rotation[2];
    heap[base + 6] = transform.rotation[3];
  }

  private writeTriangle(triangle: readonly [Vec3, Vec3, Vec3]): void {
    const heap = this.module.HEAPF32;
    const base = this.trianglePtr >> 2;
    for (let i = 0; i < 3; i++) {
      heap[base + i * 3 + 0] = triangle[i]![0];
      heap[base + i * 3 + 1] = triangle[i]![1];
      heap[base + i * 3 + 2] = triangle[i]![2];
    }
  }

  private readLocalManifold(capacity: number): LocalManifold {
    const heap = this.module.HEAPF32;
    const base = this.manifoldPtr >> 2;
    const pointCount = Math.max(0, Math.min(capacity, B3W_MANIFOLD_MAX_POINTS, Math.trunc(heap[base + 6]!)));
    const points: LocalManifoldPoint[] = [];
    for (let i = 0; i < pointCount; i++) {
      const p = base + B3W_MANIFOLD_HEADER_FLOATS + i * B3W_MANIFOLD_POINT_FLOATS;
      points.push({
        point: [heap[p]!, heap[p + 1]!, heap[p + 2]!],
        separation: heap[p + 3]!,
        pair: [Math.trunc(heap[p + 4]!), Math.trunc(heap[p + 5]!), Math.trunc(heap[p + 6]!), Math.trunc(heap[p + 7]!)],
      });
    }
    return {
      normal: [heap[base]!, heap[base + 1]!, heap[base + 2]!],
      triangleNormal: [heap[base + 3]!, heap[base + 4]!, heap[base + 5]!],
      pointCount,
      feature: Math.trunc(heap[base + 7]!),
      triangleIndex: Math.trunc(heap[base + 8]!),
      indices: [Math.trunc(heap[base + 9]!), Math.trunc(heap[base + 10]!), Math.trunc(heap[base + 11]!)],
      squaredDistance: heap[base + 12]!,
      triangleFlags: Math.trunc(heap[base + 13]!),
      points,
    };
  }

  private manifoldCapacityFloats(capacity: number): number {
    return B3W_MANIFOLD_HEADER_FLOATS + Math.min(capacity, B3W_MANIFOLD_MAX_POINTS) * B3W_MANIFOLD_POINT_FLOATS;
  }

  collideSpheres(sphereA: { center: Vec3; radius: number }, sphereB: { center: Vec3; radius: number }, transformA: WorldTransform, transformB: WorldTransform, capacity = 64): LocalManifold {
    this.writeWorldTransform(this.xfAPtr, transformA);
    this.writeWorldTransform(this.xfBPtr, transformB);
    this.collideSpheresFn(sphereA.center[0], sphereA.center[1], sphereA.center[2], sphereA.radius, sphereB.center[0], sphereB.center[1], sphereB.center[2], sphereB.radius, this.xfAPtr, this.xfBPtr, capacity, this.manifoldPtr, this.manifoldCapacityFloats(capacity));
    return this.readLocalManifold(capacity);
  }

  collideCapsuleAndSphere(capsule: { center1: Vec3; center2: Vec3; radius: number }, sphere: { center: Vec3; radius: number }, transformA: WorldTransform, transformB: WorldTransform, capacity = 64): LocalManifold {
    this.writeWorldTransform(this.xfAPtr, transformA);
    this.writeWorldTransform(this.xfBPtr, transformB);
    this.collideCapsuleAndSphereFn(capsule.center1[0], capsule.center1[1], capsule.center1[2], capsule.center2[0], capsule.center2[1], capsule.center2[2], capsule.radius, sphere.center[0], sphere.center[1], sphere.center[2], sphere.radius, this.xfAPtr, this.xfBPtr, capacity, this.manifoldPtr, this.manifoldCapacityFloats(capacity));
    return this.readLocalManifold(capacity);
  }

  collideHullAndSphere(hullHandle: HullHandle, sphere: { center: Vec3; radius: number }, transformA: WorldTransform, transformB: WorldTransform, capacity = 64): LocalManifold {
    this.writeWorldTransform(this.xfAPtr, transformA);
    this.writeWorldTransform(this.xfBPtr, transformB);
    this.collideHullAndSphereFn(hullHandle, sphere.center[0], sphere.center[1], sphere.center[2], sphere.radius, this.xfAPtr, this.xfBPtr, capacity, this.manifoldPtr, this.manifoldCapacityFloats(capacity));
    return this.readLocalManifold(capacity);
  }

  collideCapsules(capsuleA: { center1: Vec3; center2: Vec3; radius: number }, capsuleB: { center1: Vec3; center2: Vec3; radius: number }, transformA: WorldTransform, transformB: WorldTransform, capacity = 64): LocalManifold {
    this.writeWorldTransform(this.xfAPtr, transformA);
    this.writeWorldTransform(this.xfBPtr, transformB);
    this.collideCapsulesFn(capsuleA.center1[0], capsuleA.center1[1], capsuleA.center1[2], capsuleA.center2[0], capsuleA.center2[1], capsuleA.center2[2], capsuleA.radius, capsuleB.center1[0], capsuleB.center1[1], capsuleB.center1[2], capsuleB.center2[0], capsuleB.center2[1], capsuleB.center2[2], capsuleB.radius, this.xfAPtr, this.xfBPtr, capacity, this.manifoldPtr, this.manifoldCapacityFloats(capacity));
    return this.readLocalManifold(capacity);
  }

  collideHullAndCapsule(hullHandle: HullHandle, capsule: { center1: Vec3; center2: Vec3; radius: number }, transformA: WorldTransform, transformB: WorldTransform, capacity = 64): LocalManifold {
    this.writeWorldTransform(this.xfAPtr, transformA);
    this.writeWorldTransform(this.xfBPtr, transformB);
    this.collideHullAndCapsuleFn(hullHandle, capsule.center1[0], capsule.center1[1], capsule.center1[2], capsule.center2[0], capsule.center2[1], capsule.center2[2], capsule.radius, this.xfAPtr, this.xfBPtr, capacity, this.manifoldPtr, this.manifoldCapacityFloats(capacity));
    return this.readLocalManifold(capacity);
  }

  collideHulls(hullA: HullHandle, hullB: HullHandle, transformA: WorldTransform, transformB: WorldTransform, capacity = 64): LocalManifold {
    this.writeWorldTransform(this.xfAPtr, transformA);
    this.writeWorldTransform(this.xfBPtr, transformB);
    this.collideHullsFn(hullA, hullB, this.xfAPtr, this.xfBPtr, capacity, this.manifoldPtr, this.manifoldCapacityFloats(capacity));
    return this.readLocalManifold(capacity);
  }

  collideTriangleAndSphere(triangle: readonly [Vec3, Vec3, Vec3], sphere: { center: Vec3; radius: number }, transformA: WorldTransform, transformB: WorldTransform, capacity = 8): LocalManifold {
    this.writeTriangle(triangle);
    this.writeWorldTransform(this.xfAPtr, transformA);
    this.writeWorldTransform(this.xfBPtr, transformB);
    this.collideTriangleAndSphereFn(this.trianglePtr, sphere.center[0], sphere.center[1], sphere.center[2], sphere.radius, this.xfAPtr, this.xfBPtr, capacity, this.manifoldPtr, this.manifoldCapacityFloats(capacity));
    return this.readLocalManifold(capacity);
  }

  collideTriangleAndCapsule(triangle: readonly [Vec3, Vec3, Vec3], capsule: { center1: Vec3; center2: Vec3; radius: number }, transformA: WorldTransform, transformB: WorldTransform, capacity = 8): LocalManifold {
    this.writeTriangle(triangle);
    this.writeWorldTransform(this.xfAPtr, transformA);
    this.writeWorldTransform(this.xfBPtr, transformB);
    this.collideTriangleAndCapsuleFn(this.trianglePtr, capsule.center1[0], capsule.center1[1], capsule.center1[2], capsule.center2[0], capsule.center2[1], capsule.center2[2], capsule.radius, this.xfAPtr, this.xfBPtr, capacity, this.manifoldPtr, this.manifoldCapacityFloats(capacity));
    return this.readLocalManifold(capacity);
  }

  collideTriangleAndHull(triangle: readonly [Vec3, Vec3, Vec3], hullHandle: HullHandle, transformA: WorldTransform, transformB: WorldTransform, options: { triangleFlags?: number; enableSpeculative?: boolean; capacity?: number } = {}): LocalManifold {
    const capacity = options.capacity ?? 8;
    this.writeTriangle(triangle);
    this.writeWorldTransform(this.xfAPtr, transformA);
    this.writeWorldTransform(this.xfBPtr, transformB);
    this.collideTriangleAndHullFn(this.trianglePtr, options.triangleFlags ?? 0, hullHandle, this.xfAPtr, this.xfBPtr, capacity, options.enableSpeculative === true ? 1 : 0, this.manifoldPtr, this.manifoldCapacityFloats(capacity));
    return this.readLocalManifold(capacity);
  }

  shapeCast(proxyA: { points: ArrayLike<number>; radius?: number }, proxyB: { points: ArrayLike<number>; radius?: number }, transformBtoA: WorldTransform, translationB: Vec3, options: { maxFraction?: number; canEncroach?: boolean } = {}): ShapeCastHit {
    const countA = Math.trunc(proxyA.points.length / 3);
    const countB = Math.trunc(proxyB.points.length / 3);
    const ptrA = this.module._malloc(countA * 3 * 4);
    const ptrB = this.module._malloc(countB * 3 * 4);
    const heap = this.module.HEAPF32;
    const baseA = ptrA >> 2;
    const baseB = ptrB >> 2;
    for (let i = 0; i < countA * 3; i++) heap[baseA + i] = proxyA.points[i]!;
    for (let i = 0; i < countB * 3; i++) heap[baseB + i] = proxyB.points[i]!;
    this.writeWorldTransform(this.xfAPtr, transformBtoA);
    const tBase = this.pointPtr >> 2;
    heap[tBase + 0] = translationB[0];
    heap[tBase + 1] = translationB[1];
    heap[tBase + 2] = translationB[2];
    this.shapeCastPairFn(ptrA, countA, proxyA.radius ?? 0, ptrB, countB, proxyB.radius ?? 0, this.xfAPtr, this.pointPtr, options.maxFraction ?? 1, options.canEncroach ? 1 : 0, this.shapeCastOutPtr);
    this.module._free(ptrA);
    this.module._free(ptrB);
    const out = this.shapeCastOutPtr >> 2;
    return {
      hit: heap[out]! !== 0,
      fraction: heap[out + 1]!,
      point: [heap[out + 2]!, heap[out + 3]!, heap[out + 4]!],
      normal: [heap[out + 5]!, heap[out + 6]!, heap[out + 7]!],
      iterations: heap[out + 8]!,
    };
  }
  getHullVertexCount(hullHandle: HullHandle): number { return this.getHullVertexCountFn(hullHandle); }
  /** Packed xyz floats from `b3GetHullPoints` (post-hull-construction vertices). */
  getHullPoints(hullHandle: HullHandle): number[] {
    const vertexCount = this.getHullVertexCountFn(hullHandle);
    if (vertexCount <= 0) return [];
    const floats = vertexCount * 3;
    const ptr = this.module._malloc(floats * 4);
    const written = this.getHullPointsFn(hullHandle, ptr, floats);
    const heap = this.module.HEAPF32;
    const base = ptr >> 2;
    const points = new Array<number>(written * 3);
    for (let i = 0; i < points.length; i++) points[i] = heap[base + i]!;
    this.module._free(ptr);
    return points;
  }
  createRock(radius: number): HullHandle { return this.requireSlotHandle<HullHandle>(this.createRockFn(radius), "hulls"); }
  getStallThreshold(): number { return this.getStallThresholdFn(); }
  setStallThreshold(seconds: number): void { this.setStallThresholdFn(seconds); }
  /** Match Box3D's b3Sin deterministically using Bhāskara I approximation. */
  b3wSin(radians: number): number { return this.b3wSinFn(radians); }
  /** Match Box3D's b3Cos deterministically using Bhāskara I approximation. */
  b3wCos(radians: number): number { return this.b3wCosFn(radians); }
  /** Float32 cosf from math.h (not Box3D's approximation). Use when matching upstream C++ code that calls cosf directly. */
  b3wCosf(radians: number): number { return this.b3wCosfFn(radians); }
  /** Float32 sinf from math.h (not Box3D's approximation). Use when matching upstream C++ code that calls sinf directly. */
  b3wSinf(radians: number): number { return this.b3wSinfFn(radians); }
  makeQuatFromAxisAngle(axis: Vec3, radians: number): Quat {
    this.makeQuatFromAxisAngleFn(axis[0], axis[1], axis[2], radians, this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2], heap[base + 3]];
  }
  rotateVector(quat: Quat, vec: Vec3): Vec3 {
    this.rotateVectorFn(quat[0], quat[1], quat[2], quat[3], vec[0], vec[1], vec[2], this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2]];
  }
  /** Box3D shared RNG (seed 12345), matching upstream RandomVec3. */
  randomVec3(lo: Vec3, hi: Vec3): Vec3 {
    this.randomVec3Fn(lo[0], lo[1], lo[2], hi[0], hi[1], hi[2], this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2]];
  }
  setRandomSeed(seed: number): void { this.setRandomSeedFn(seed >>> 0); }
  getRandomSeed(): number { return this.getRandomSeedFn() >>> 0; }
  randomFloatRange(lo: number, hi: number): number { return this.randomFloatRangeFn(lo, hi); }
  randomVec3Uniform(lo: number, hi: number): Vec3 {
    this.randomVec3UniformFn(lo, hi, this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2]];
  }
  randomUnitVector(): Vec3 {
    this.randomUnitVectorFn(this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2]];
  }
  randomQuat(): Quat {
    this.randomQuatFn(this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2], heap[base + 3]];
  }
  /** Box3D b3Lerp for Vec3. */
  lerpVec3(a: Vec3, b: Vec3, alpha: number): Vec3 {
    this.lerpVec3Fn(a[0], a[1], a[2], b[0], b[1], b[2], alpha, this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2]];
  }
  /** Box3D b3GetLengthAndNormalize: returns length and writes the normalized direction. */
  getLengthAndNormalize(vec: Vec3): { length: number; direction: Vec3 } {
    const length = this.getLengthAndNormalizeFn(vec[0], vec[1], vec[2], this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return { length, direction: [heap[base + 0], heap[base + 1], heap[base + 2]] };
  }
  /** Box3D `b3ComputeQuatBetweenUnitVectors`. */
  computeQuatBetweenUnitVectors(from: Vec3, to: Vec3): Quat {
    this.computeQuatBetweenUnitVectorsFn(from[0], from[1], from[2], to[0], to[1], to[2], this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2], heap[base + 3]];
  }
  /** Box3D `b3InvMulQuat(q1, q2)`. */
  invMulQuat(q1: Quat, q2: Quat): Quat {
    this.invMulQuatFn(q1[0], q1[1], q1[2], q1[3], q2[0], q2[1], q2[2], q2[3], this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2], heap[base + 3]];
  }
  createCompound(capsules: number, hulls: number, meshes: number, spheres: number): CompoundHandle {
    return this.requireSlotHandle<CompoundHandle>(this.createCompoundFn(capsules, hulls, meshes, spheres, 0, 0, 0, 0), "compounds");
  }
  createCompoundFromHulls(entries: CompoundHullEntry[]): CompoundHandle {
    const stride = 13;
    const floatCount = entries.length * stride;
    const ptr = this.module._malloc(floatCount * 4);
    const heap = this.module.HEAPF32;
    const base = ptr >> 2;
    for (let i = 0; i < entries.length; i++) {
      const e = entries[i];
      const off = i * stride;
      const hw = e.halfWidths; const t = e.transform.position; const r = e.transform.rotation;
      heap[base + off + 0] = hw[0]; heap[base + off + 1] = hw[1]; heap[base + off + 2] = hw[2];
      heap[base + off + 3] = t[0]; heap[base + off + 4] = t[1]; heap[base + off + 5] = t[2];
      heap[base + off + 6] = r[0]; heap[base + off + 7] = r[1]; heap[base + off + 8] = r[2]; heap[base + off + 9] = r[3];
      heap[base + off + 10] = e.friction ?? 0.5;
      heap[base + off + 11] = e.restitution ?? 0;
      heap[base + off + 12] = e.rollingResistance ?? 0;
    }
    const result = this.createCompoundFromHullsFn(entries.length, ptr, stride);
    this.module._free(ptr);
    return this.requireSlotHandle<CompoundHandle>(result, "compounds");
  }
  createCompoundFromSpheres(entries: CompoundSphereEntry[]): CompoundHandle {
    const stride = 7;
    const floatCount = entries.length * stride;
    const ptr = this.module._malloc(floatCount * 4);
    const heap = this.module.HEAPF32;
    const base = ptr >> 2;
    for (let i = 0; i < entries.length; i++) {
      const e = entries[i];
      const off = i * stride;
      heap[base + off + 0] = e.center[0]; heap[base + off + 1] = e.center[1]; heap[base + off + 2] = e.center[2];
      heap[base + off + 3] = e.radius;
      heap[base + off + 4] = e.friction ?? 0.5;
      heap[base + off + 5] = e.restitution ?? 0;
      heap[base + off + 6] = e.rollingResistance ?? 0;
    }
    const result = this.createCompoundFromSpheresFn(entries.length, ptr, stride);
    this.module._free(ptr);
    return this.requireSlotHandle<CompoundHandle>(result, "compounds");
  }
  createCompoundFromMeshes(entries: CompoundMeshEntry[]): CompoundHandle {
    const stride = 14;
    const floatCount = entries.length * stride;
    const ptr = this.module._malloc(floatCount * 4);
    const heap = this.module.HEAPF32;
    const base = ptr >> 2;
    for (let i = 0; i < entries.length; i++) {
      const e = entries[i];
      const off = i * stride;
      const t = e.transform.position;
      const r = e.transform.rotation;
      const scale = e.scale ?? [1, 1, 1];
      heap[base + off + 0] = e.meshHandle;
      heap[base + off + 1] = t[0]; heap[base + off + 2] = t[1]; heap[base + off + 3] = t[2];
      heap[base + off + 4] = r[0]; heap[base + off + 5] = r[1]; heap[base + off + 6] = r[2]; heap[base + off + 7] = r[3];
      heap[base + off + 8] = scale[0]; heap[base + off + 9] = scale[1]; heap[base + off + 10] = scale[2];
      heap[base + off + 11] = e.friction ?? 0.5;
      heap[base + off + 12] = e.restitution ?? 0;
      heap[base + off + 13] = e.rollingResistance ?? 0;
    }
    const result = this.createCompoundFromMeshesFn(entries.length, ptr, stride);
    this.module._free(ptr);
    return this.requireSlotHandle<CompoundHandle>(result, "compounds");
  }
  destroyCompound(compoundHandle: CompoundHandle): void { this.destroyCompoundFn(compoundHandle); }
  getCompoundTreeHeight(compoundHandle: CompoundHandle): number { return this.getCompoundTreeHeightFn(compoundHandle); }
  createCompoundShape(bodyHandle: BodyId, compoundHandle: CompoundHandle, density = 1): ShapeId {
    const shapeId = asShapeId(this.createCompoundShapeFn(bodyHandle, compoundHandle, density));
    if (shapeId === 0n) throw new Error("b3wCreateCompoundShape failed");
    return shapeId;
  }
  createMeshShape(bodyHandle: BodyId, meshHandle: MeshHandle, def: MeshShapeOptions = {}): ShapeHandle {
    const scale = def.scale ?? [1, 1, 1];
    const shapeHandle = asShapeId(this.createMeshShapeFn(bodyHandle, meshHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, scale[0], scale[1], scale[2], def.isSensor ? 1 : 0));
    if (shapeHandle === 0n) throw new Error("createMeshShapeFn failed");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }
  createWave(worldHandle: WorldHandle, rowCount: number, columnCount: number, scale: Vec3, rowFrequency: number, columnFrequency: number, makeHoles = false): HeightFieldHandle {
    return this.requireSlotHandle<HeightFieldHandle>(this.createWaveFn(worldHandle, rowCount, columnCount, scale[0], scale[1], scale[2], rowFrequency, columnFrequency, makeHoles ? 1 : 0), "heightFields");
  }
  createGridHeightField(worldHandle: WorldHandle, rowCount: number, columnCount: number, scale: Vec3, makeHoles = false): HeightFieldHandle {
    return this.requireSlotHandle<HeightFieldHandle>(this.createGridHeightFieldFn(worldHandle, rowCount, columnCount, scale[0], scale[1], scale[2], makeHoles ? 1 : 0), "heightFields");
  }
  destroyHeightField(heightFieldHandle: HeightFieldHandle): void { this.destroyHeightFieldFn(heightFieldHandle); }
  createHeightFieldShape(bodyHandle: BodyId, heightFieldHandle: HeightFieldHandle, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = asShapeId(this.createHeightFieldShapeFn(bodyHandle, heightFieldHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, def.isSensor ? 1 : 0));
    if (shapeHandle === 0n) throw new Error("createHeightFieldShapeFn failed");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }
  setMesh(shapeHandle: ShapeId | ShapeHandle, meshHandle: MeshHandle, scale: Vec3 = [1, 1, 1]): void {
    const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle;
    this.shapeSetMeshFn(handle, meshHandle, scale[0], scale[1], scale[2]);
  }
  getBodyShapes(bodyHandle: BodyId): ShapeId[] {
    const count = this.getBodyShapeCountFn(bodyHandle);
    if (count <= 0) return [];
    const ptr = this.module._malloc(count * 8);
    const written = this.getBodyShapesFn(bodyHandle, ptr, count);
    const heap = this.module.HEAPU64;
    const base = ptr >>> 3;
    const handles: ShapeId[] = [];
    for (let i = 0; i < written; i++) handles.push(asShapeId(heap[base + i]!));
    this.module._free(ptr);
    return handles;
  }
  destroyBody(bodyHandle: BodyId): void { this.destroyBodyFn(bodyHandle); }
  bodyIsValid(bodyHandle: BodyId): boolean { return this.bodyIsValidFn(bodyHandle) !== 0; }
  destroyShape(shapeHandle: ShapeId | ShapeHandle, updateBodyMass = true): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.destroyShapeFn(handle, updateBodyMass ? 1 : 0); }
  destroyJoint(jointHandle: JointId): void { this.destroyJointFn(jointHandle); }
  setBodyTransform(bodyHandle: BodyId, position: Vec3, rotation: Quat = [0,0,0,1]): void { this.setBodyTransformFn(bodyHandle, position[0], position[1], position[2], rotation[0], rotation[1], rotation[2], rotation[3]); }
  setBodyLinearVelocity(bodyHandle: BodyId, velocity: Vec3): void { this.setBodyLinearVelocityFn(bodyHandle, velocity[0], velocity[1], velocity[2]); }
  setBodyAngularVelocity(bodyHandle: BodyId, velocity: Vec3): void { this.setBodyAngularVelocityFn(bodyHandle, velocity[0], velocity[1], velocity[2]); }
  getBodyLinearVelocity(bodyHandle: BodyId): Vec3 { return this.getBodyLinearVelocityTo(bodyHandle, [0, 0, 0]); }
  getBodyLinearVelocityTo(bodyHandle: BodyId, out: Vec3): Vec3 { this.getBodyLinearVelocityFn(bodyHandle, this.pointPtr); return this.readPointInto(out); }
  getBodyAngularVelocity(bodyHandle: BodyId): Vec3 { return this.getBodyAngularVelocityTo(bodyHandle, [0, 0, 0]); }
  getBodyAngularVelocityTo(bodyHandle: BodyId, out: Vec3): Vec3 { this.getBodyAngularVelocityFn(bodyHandle, this.pointPtr); return this.readPointInto(out); }
  bodyIsAwake(bodyHandle: BodyId): boolean { return this.bodyIsAwakeFn(bodyHandle) !== 0; }
  getBodyDebugColor(bodyHandle: BodyId): number { return this.getBodyDebugColorFn(bodyHandle); }
  getBodyType(bodyHandle: BodyId): BodyType { return this.getBodyTypeFn(bodyHandle) as BodyType; }
  setBodyAwake(bodyHandle: BodyId, awake: boolean): void { this.setBodyAwakeFn(bodyHandle, awake ? 1 : 0); }
  setBodyDamping(bodyHandle: BodyId, linearDamping: number, angularDamping: number): void { this.setBodyDampingFn(bodyHandle, linearDamping, angularDamping); }
  getBodyLocalPoint(bodyHandle: BodyId, worldPoint: Vec3): Vec3 { return this.getBodyLocalPointTo(bodyHandle, worldPoint, [0, 0, 0]); }
  getBodyLocalPointXYZ(bodyHandle: BodyId, worldX: number, worldY: number, worldZ: number): Vec3 { return this.getBodyLocalPointXYZTo(bodyHandle, worldX, worldY, worldZ, [0, 0, 0]); }
  getBodyLocalPointTo(bodyHandle: BodyId, worldPoint: Vec3, out: Vec3): Vec3 { return this.getBodyLocalPointXYZTo(bodyHandle, worldPoint[0], worldPoint[1], worldPoint[2], out); }
  getBodyLocalPointXYZTo(bodyHandle: BodyId, worldX: number, worldY: number, worldZ: number, out: Vec3): Vec3 { this.getBodyLocalPointFn(bodyHandle, worldX, worldY, worldZ, this.pointPtr); return this.readPointInto(out); }
  createMotorJoint(worldHandle: WorldHandle, bodyAHandle: BodyId, bodyBHandle: BodyId, options: MotorJointOptions = {}): JointId { const a = options.localFrameA ?? vec3(); const b = options.localFrameB ?? vec3(); const lv = options.linearVelocity ?? vec3(); const av = options.angularVelocity ?? vec3(); { const jointId = asJointId(this.createMotorJointFn(worldHandle, bodyAHandle, bodyBHandle, a[0], a[1], a[2], b[0], b[1], b[2], lv[0], lv[1], lv[2], options.maxVelocityForce ?? 0, av[0], av[1], av[2], options.maxVelocityTorque ?? 0, options.collideConnected ? 1 : 0, options.linearHertz ?? 0, options.linearDampingRatio ?? 0, options.maxSpringForce ?? 0, options.angularHertz ?? 0, options.angularDampingRatio ?? 0, options.maxSpringTorque ?? 0));
    if (jointId === 0n) throw new Error("createMotorJointFn failed");
    return jointId; } }
  createFilterJoint(worldHandle: WorldHandle, bodyAHandle: BodyId, bodyBHandle: BodyId): JointId { { const jointId = asJointId(this.createFilterJointFn(worldHandle, bodyAHandle, bodyBHandle));
    if (jointId === 0n) throw new Error("createFilterJointFn failed");
    return jointId; } }
  createRevoluteJoint(worldHandle: WorldHandle, bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; constraintHertz?: number; constraintDampingRatio?: number; targetAngle?: number; enableSpring?: boolean; hertz?: number; dampingRatio?: number; enableLimit?: boolean; lowerAngle?: number; upperAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorSpeed?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointId { const a = options.localFrameA?.position ?? vec3(); const aq = options.localFrameA?.rotation ?? [0, 0, 0, 1]; const b = options.localFrameB?.position ?? vec3(); const bq = options.localFrameB?.rotation ?? [0, 0, 0, 1]; const [forceThreshold, torqueThreshold, collideConnected] = jointThresholdArgs(options); { const jointId = asJointId(this.createRevoluteJointFn(worldHandle, bodyAHandle, bodyBHandle, a[0], a[1], a[2], aq[0], aq[1], aq[2], aq[3], b[0], b[1], b[2], bq[0], bq[1], bq[2], bq[3], options.constraintHertz ?? 60, options.constraintDampingRatio ?? 2, options.targetAngle ?? 0, options.enableSpring ? 1 : 0, options.hertz ?? 0, options.dampingRatio ?? 0, options.enableLimit ? 1 : 0, options.lowerAngle ?? 0, options.upperAngle ?? 0, options.enableMotor ? 1 : 0, options.maxMotorTorque ?? 0, options.motorSpeed ?? 0, forceThreshold, torqueThreshold, collideConnected));
    if (jointId === 0n) throw new Error("createRevoluteJointFn failed");
    return jointId; } }
  createSphericalJoint(worldHandle: WorldHandle, bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetRotation?: Quat; enableConeLimit?: boolean; coneAngle?: number; enableTwistLimit?: boolean; lowerTwistAngle?: number; upperTwistAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorVelocity?: Vec3 } = {}): JointId { const a = options.localFrameA?.position ?? vec3(); const aq = options.localFrameA?.rotation ?? [0, 0, 0, 1]; const b = options.localFrameB?.position ?? vec3(); const bq = options.localFrameB?.rotation ?? [0, 0, 0, 1]; const tq = options.targetRotation ?? [0, 0, 0, 1]; const mv = options.motorVelocity ?? vec3(); { const jointId = asJointId(this.createSphericalJointFn(worldHandle, bodyAHandle, bodyBHandle, a[0], a[1], a[2], aq[0], aq[1], aq[2], aq[3], b[0], b[1], b[2], bq[0], bq[1], bq[2], bq[3], options.enableSpring ? 1 : 0, options.hertz ?? 0, options.dampingRatio ?? 0, tq[0], tq[1], tq[2], tq[3], options.enableConeLimit ? 1 : 0, options.coneAngle ?? 0, options.enableTwistLimit ? 1 : 0, options.lowerTwistAngle ?? 0, options.upperTwistAngle ?? 0, options.enableMotor ? 1 : 0, options.maxMotorTorque ?? 0, mv[0], mv[1], mv[2]));
    if (jointId === 0n) throw new Error("createSphericalJointFn failed");
    return jointId; } }
  createHuman(worldHandle: WorldHandle, position: Vec3, options: { frictionTorque?: number; hertz?: number; dampingRatio?: number; groupIndex?: number; colorize?: boolean } = {}): HumanHandle {
    return this.requireSlotHandle<HumanHandle>(
      this.createHumanFn(worldHandle, position[0], position[1], position[2], options.frictionTorque ?? 1, options.hertz ?? 1, options.dampingRatio ?? 1, options.groupIndex ?? 0, options.colorize ?? true ? 1 : 0),
      "humans",
    );
  }
  getHumanBoneBody(humanHandle: HumanHandle, boneIndex: number): BodyId { return asBodyId(this.getHumanBoneBodyFn(humanHandle, boneIndex)); }
  getHumanBoneCount(): number { return this.getHumanBoneCountFn(); }
  setHumanVelocity(humanHandle: number, velocity: Vec3): void { this.humanSetVelocityFn(humanHandle, velocity[0], velocity[1], velocity[2]); }
  setHumanBullet(humanHandle: number, flag: boolean): void { this.humanSetBulletFn(humanHandle, flag ? 1 : 0); }
  setHumanJointFrictionTorque(humanHandle: number, torque: number): void { this.humanSetJointFrictionTorqueFn(humanHandle, torque); }
  setHumanJointSpringHertz(humanHandle: number, hertz: number): void { this.humanSetJointSpringHertzFn(humanHandle, hertz); }
  setHumanJointDampingRatio(humanHandle: number, dampingRatio: number): void { this.humanSetJointDampingRatioFn(humanHandle, dampingRatio); }
  createHumanParallelAnchors(humanHandle: HumanHandle): void { this.humanCreateParallelAnchorsFn(humanHandle); }
  getHumanAnchorBody(humanHandle: HumanHandle, boneIndex: number): BodyId { return asBodyId(this.getHumanAnchorBodyFn(humanHandle, boneIndex)); }
  readBodyTransform(bodyHandle: BodyId): BodyTransform { this.getBodyTransformFn(bodyHandle, this.transformPtr); const heap = this.module.HEAPF32; const base = this.transformPtr >> 2; return { position: [heap[base + 0], heap[base + 1], heap[base + 2]], rotation: [heap[base + 3], heap[base + 4], heap[base + 5], heap[base + 6]] }; }
  getWorldCounters(worldHandle: WorldHandle): WorldCounters { const ptr = this.module._malloc(7 * 4); this.getWorldCountersFn(worldHandle, ptr); const heap32 = new Int32Array(this.module.HEAPF32.buffer); const base = ptr >> 2; const counters = { bodyCount: heap32[base + 0], shapeCount: heap32[base + 1], contactCount: heap32[base + 2], jointCount: heap32[base + 3], islandCount: heap32[base + 4], staticTreeHeight: heap32[base + 5], treeHeight: heap32[base + 6] }; this.module._free(ptr); return counters; }
  getWorldAwakeBodyCount(worldHandle: WorldHandle): number { return this.getWorldAwakeBodyCountFn(worldHandle); }
  getWorldProfile(worldHandle: WorldHandle): WorldProfile { this.getWorldProfileFn(worldHandle, this.profilePtr); const heap = this.module.HEAPF32; const base = this.profilePtr >> 2; return { step: heap[base + 0], pairs: heap[base + 1], collide: heap[base + 2], solve: heap[base + 3], solverSetup: heap[base + 4], constraints: heap[base + 5], prepareConstraints: heap[base + 6], integrateVelocities: heap[base + 7], warmStart: heap[base + 8], solveImpulses: heap[base + 9], integratePositions: heap[base + 10], relaxImpulses: heap[base + 11], applyRestitution: heap[base + 12], storeImpulses: heap[base + 13], splitIslands: heap[base + 14], transforms: heap[base + 15], sensorHits: heap[base + 16], jointEvents: heap[base + 17], hitEvents: heap[base + 18], refit: heap[base + 19], bullets: heap[base + 20], sleepIslands: heap[base + 21], sensors: heap[base + 22] }; }
  checkThreadingSupport(): number { return this.checkThreadingSupportFn(); }
  getWorldWorkerCount(worldHandle: WorldHandle): number { return this.getWorldWorkerCountFn(worldHandle); }
  enableWorldSleeping(worldHandle: WorldHandle, flag: boolean): void { this.enableWorldSleepFn(worldHandle, flag ? 1 : 0); }
  enableWorldContinuous(worldHandle: WorldHandle, flag: boolean): void { this.enableWorldContinuousFn(worldHandle, flag ? 1 : 0); }
  enableWorldWarmStarting(worldHandle: WorldHandle, flag: boolean): void { this.enableWorldWarmStartingFn(worldHandle, flag ? 1 : 0); }
  setWorldProfileLevel(worldHandle: WorldHandle, level: ProfileLevel): void { this.setWorldProfileLevelFn(worldHandle, profileLevelToInt(level)); }
  getWorldProfileLevel(worldHandle: WorldHandle): ProfileLevel { return profileLevelFromInt(this.getWorldProfileLevelFn(worldHandle)); }
  setWorldContactTuning(worldHandle: WorldHandle, hertz: number, dampingRatio: number, contactSpeed: number): void { this.setWorldContactTuningFn(worldHandle, hertz, dampingRatio, contactSpeed); }
  setWorldContactRecycleDistance(worldHandle: WorldHandle, distance: number): void { this.setWorldContactRecycleDistanceFn(worldHandle, distance); }
  setWorldWorkerCount(worldHandle: WorldHandle, count: number): void { this.setWorldWorkerCountFn(worldHandle, count); }

  rayCastClosest(worldHandle: WorldHandle, origin: Vec3, translation: Vec3, categoryBits = U64_MAX, maskBits = U64_MAX): { shapeHandle: ShapeId; bodyHandle: BodyId; point: Vec3; normal: Vec3; fraction: number } | null {
    const outShapePtr = this.module._malloc(8);
    const outPointPtr = this.module._malloc(3 * 4);
    const outNormalPtr = this.module._malloc(3 * 4);
    const outFractionPtr = this.module._malloc(4);
    this.rayCastClosestFn(worldHandle, origin[0], origin[1], origin[2], translation[0], translation[1], translation[2], categoryBits, maskBits, outShapePtr, outPointPtr, outNormalPtr, outFractionPtr);
    const heapU64 = this.module.HEAPU64;
    const heap = this.module.HEAPF32;
    const pBase = outPointPtr >> 2;
    const nBase = outNormalPtr >> 2;
    const fBase = outFractionPtr >> 2;
    const shapeHandle = asShapeId(heapU64[outShapePtr >>> 3]!);
    const fraction = heap[fBase + 0]!;
    if (shapeHandle === 0n || fraction <= 0) {
      this.module._free(outShapePtr); this.module._free(outPointPtr); this.module._free(outNormalPtr); this.module._free(outFractionPtr);
      return null;
    }
    const bodyHandle = asBodyId(this.getShapeBodyHandleFn(shapeHandle));
    const result = { shapeHandle, bodyHandle, point: [heap[pBase + 0]!, heap[pBase + 1]!, heap[pBase + 2]!] as Vec3, normal: [heap[nBase + 0]!, heap[nBase + 1]!, heap[nBase + 2]!] as Vec3, fraction };
    this.module._free(outShapePtr); this.module._free(outPointPtr); this.module._free(outNormalPtr); this.module._free(outFractionPtr);
    return result;
  }

  overlapAABB(worldHandle: WorldHandle, min: Vec3, max: Vec3, categoryBits = U64_MAX, maskBits = U64_MAX): number {
    return this.overlapAABBFn(worldHandle, min[0], min[1], min[2], max[0], max[1], max[2], categoryBits, maskBits);
  }

  castShapeSphere(worldHandle: WorldHandle, origin: Vec3, translation: Vec3, radius: number, categoryBits = U64_MAX, maskBits = U64_MAX): number {
    return this.castShapeSphereFn(worldHandle, origin[0], origin[1], origin[2], translation[0], translation[1], translation[2], radius, categoryBits, maskBits);
  }

  bodyCastRay(
    bodyHandle: BodyId,
    origin: Vec3,
    translation: Vec3,
    options: { categoryBits?: number; maskBits?: number; maxFraction?: number; bodyTransform?: BodyTransform } = {},
  ): { hit: boolean; point: Vec3; normal: Vec3; fraction: number } {
    const categoryBits = options.categoryBits ?? U64_MAX;
    const maskBits = options.maskBits ?? U64_MAX;
    const maxFraction = options.maxFraction ?? 1;
    const xf = options.bodyTransform ?? { position: [0, 0, 0], rotation: [0, 0, 0, 1] };
    const outHitPtr = this.module._malloc(4);
    const outPointPtr = this.module._malloc(3 * 4);
    const outNormalPtr = this.module._malloc(3 * 4);
    const outFractionPtr = this.module._malloc(4);
    this.bodyCastRayFn(
      bodyHandle,
      origin[0], origin[1], origin[2],
      translation[0], translation[1], translation[2],
      categoryBits, maskBits, maxFraction,
      xf.position[0], xf.position[1], xf.position[2],
      xf.rotation[0], xf.rotation[1], xf.rotation[2], xf.rotation[3],
      outHitPtr, outPointPtr, outNormalPtr, outFractionPtr,
    );
    const heap32 = this.module.HEAP32;
    const heap = this.module.HEAPF32;
    const hit = heap32[outHitPtr >> 2] !== 0;
    const pBase = outPointPtr >> 2;
    const nBase = outNormalPtr >> 2;
    const fraction = heap[outFractionPtr >> 2]!;
    const result = {
      hit,
      point: [heap[pBase]!, heap[pBase + 1]!, heap[pBase + 2]!] as Vec3,
      normal: [heap[nBase]!, heap[nBase + 1]!, heap[nBase + 2]!] as Vec3,
      fraction,
    };
    this.module._free(outHitPtr); this.module._free(outPointPtr); this.module._free(outNormalPtr); this.module._free(outFractionPtr);
    return result;
  }

  getSensorBeginEvents(worldHandle: WorldHandle): SensorBeginEvent[] {
    const count = this.getSensorBeginEventCountFn(worldHandle);
    if (count <= 0) return [];
    const events: SensorBeginEvent[] = [];
    const sensorPtr = this.module._malloc(8);
    const visitorPtr = this.module._malloc(8);
    for (let i = 0; i < count; i++) {
      this.getSensorBeginEventFn(worldHandle, i, sensorPtr, visitorPtr);
      const heap = this.module.HEAPU64;
      events.push({
        sensorShapeHandle: asShapeId(heap[sensorPtr >>> 3]!),
        visitorShapeHandle: asShapeId(heap[visitorPtr >>> 3]!),
      });
    }
    this.module._free(sensorPtr);
    this.module._free(visitorPtr);
    return events;
  }

  getJointEventHandles(worldHandle: WorldHandle): JointId[] {
    const count = this.getJointEventCountFn(worldHandle);
    if (count <= 0) return [];
    const handles: JointId[] = [];
    for (let i = 0; i < count; i++) {
      const handle = asJointId(this.getJointEventHandleFn(worldHandle, i));
      if (handle !== 0n) handles.push(handle);
    }
    return handles;
  }

  // Batched transform read: writes transforms + awake flags for all bodies at once.
  // Buffers must be pre-allocated with _malloc.
  writeBodyTransforms(count: number, bodyHandlesPtr: number, outPositionsPtr: number, outRotationsPtr: number, outAwakePtr: number, outColorsPtr: number): void {
    this.writeBodyTransformsFn(count, bodyHandlesPtr, outPositionsPtr, outRotationsPtr, outAwakePtr, outColorsPtr);
  }

  writeBodyTransformsLight(count: number, bodyHandlesPtr: number, outPositionsPtr: number, outRotationsPtr: number, outAwakePtr: number, outColorsPtr: number): void {
    this.writeBodyTransformsLightFn(count, bodyHandlesPtr, outPositionsPtr, outRotationsPtr, outAwakePtr, outColorsPtr);
  }

  configureBodyMoveTracking(count: number, bodyHandlesPtr: number): void {
    this.configureBodyMoveTrackingFn(count, bodyHandlesPtr);
  }

  clearBodyMoveTracking(): void {
    this.clearBodyMoveTrackingFn();
  }

  scatterBodyMoveEvents(worldHandle: WorldHandle, outPositionsPtr: number, outRotationsPtr: number, outAwakePtr: number, outColorsPtr: number, useLightColors: boolean): number {
    return this.scatterBodyMoveEventsFn(worldHandle, outPositionsPtr, outRotationsPtr, outAwakePtr, outColorsPtr, useLightColors ? 1 : 0);
  }

  getBodyMoveEventCount(worldHandle: WorldHandle): number {
    return this.getBodyMoveEventCountFn(worldHandle);
  }

  step(worldHandle: WorldHandle, dt: number, substeps: number): void { this.stepFn(worldHandle, dt, substeps); }
  destroyWorld(worldHandle: WorldHandle): void { this.destroyWorldFn(worldHandle); }
  setShapeDensity(shapeHandle: ShapeId | ShapeHandle, density: number, updateBodyMass = true): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.setDensityFn(handle, density, updateBodyMass ? 1 : 0); }
  setShapeFriction(shapeHandle: ShapeId | ShapeHandle, friction: number): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.setFrictionFn(handle, friction); }
  setShapeRestitution(shapeHandle: ShapeId | ShapeHandle, restitution: number): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.setRestitutionFn(handle, restitution); }
  setShapeSurfaceMaterial(shapeHandle: ShapeId | ShapeHandle, material: SurfaceMaterial = {}): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; const tv = material.tangentVelocity ?? [0,0,0]; this.setSurfaceMaterialFn(handle, material.friction ?? 0.6, material.restitution ?? 0, material.rollingResistance ?? 0, tv[0], tv[1], tv[2]); }
  setShapeFilter(shapeHandle: ShapeId | ShapeHandle, categoryBits: number, maskBits: number, groupIndex = 0, invokeContacts = false): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.setFilterFn(handle, categoryBits, maskBits, groupIndex, invokeContacts ? 1 : 0); }
  enableShapeSensorEvents(shapeHandle: ShapeId | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapeSensorEventsFn(handle, flag ? 1 : 0); }
  enableShapeContactEvents(shapeHandle: ShapeId | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapeContactEventsFn(handle, flag ? 1 : 0); }
  enableShapePreSolveEvents(shapeHandle: ShapeId | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapePreSolveEventsFn(handle, flag ? 1 : 0); }
  enableShapeHitEvents(shapeHandle: ShapeId | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapeHitEventsFn(handle, flag ? 1 : 0); }
  setShapeSphere(shapeHandle: ShapeId | ShapeHandle, position: Vec3, radius: number): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.setShapeSphereFn(handle, position[0], position[1], position[2], radius); }
  setShapeCapsule(shapeHandle: ShapeId | ShapeHandle, a: Vec3, b: Vec3, radius: number): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.setShapeCapsuleFn(handle, a[0], a[1], a[2], b[0], b[1], b[2], radius); }
  applyShapeWind(shapeHandle: ShapeId | ShapeHandle, wind: Vec3, drag: number, lift: number, maxSpeed: number, wake = true): void { const handle = typeof shapeHandle === "bigint" ? shapeHandle : shapeHandle.shapeHandle; this.applyShapeWindFn(handle, wind[0], wind[1], wind[2], drag, lift, maxSpeed, wake ? 1 : 0); }
  setBodyType(bodyHandle: BodyId, type: BodyType): void { this.setBodyTypeFn(bodyHandle, type); }
  setBodyName(bodyHandle: BodyId, name: string): void { const ptr = this.module._malloc(name.length + 1); const heap8 = new Uint8Array(this.module.HEAPU8.buffer); for (let i = 0; i < name.length; i++) heap8[ptr + i] = name.charCodeAt(i); heap8[ptr + name.length] = 0; this.setBodyNameFn(bodyHandle, ptr); this.module._free(ptr); }
  setBodyGravityScale(bodyHandle: BodyId, scale: number): void { this.setBodyGravityScaleFn(bodyHandle, scale); }
  setBodySleepThreshold(bodyHandle: BodyId, threshold: number): void { this.setBodySleepThresholdFn(bodyHandle, threshold); }
  enableBodySleep(bodyHandle: BodyId, enable: boolean): void { this.enableBodySleepFn(bodyHandle, enable ? 1 : 0); }
  setBodyBullet(bodyHandle: BodyId, flag: boolean): void { this.setBodyBulletFn(bodyHandle, flag ? 1 : 0); }
  allowBodyFastRotation(bodyHandle: BodyId, flag: boolean): void { this.allowBodyFastRotationFn(bodyHandle, flag ? 1 : 0); }
  isBodyFastRotationAllowed(bodyHandle: BodyId): boolean { return this.isBodyFastRotationAllowedFn(bodyHandle) !== 0; }
  enableBodyContactRecycling(bodyHandle: BodyId, flag: boolean): void { this.enableBodyContactRecyclingFn(bodyHandle, flag ? 1 : 0); }
  enableBodyHitEvents(bodyHandle: BodyId, flag: boolean): void { this.enableBodyHitEventsFn(bodyHandle, flag ? 1 : 0); }
  setBodyMotionLocks(bodyHandle: BodyId, locks: { lockX?: boolean; lockY?: boolean; lockRotationX?: boolean; lockRotationY?: boolean; lockRotationZ?: boolean; lockLinearZ?: boolean } = {}): void { this.setBodyMotionLocksFn(bodyHandle, locks.lockX ? 1 : 0, locks.lockY ? 1 : 0, locks.lockLinearZ ? 1 : 0, locks.lockRotationX ? 1 : 0, locks.lockRotationY ? 1 : 0, locks.lockRotationZ ? 1 : 0); }
  setBodyMassData(bodyHandle: BodyId, mass: number, center: Vec3, inertia?: Mat3): void { if (inertia) { const heap = this.module.HEAPF32; const base = this.inertiaPtr >> 2; for (let i = 0; i < 9; i++) heap[base + i] = inertia[i]; this.setBodyMassDataFn(bodyHandle, mass, center[0], center[1], center[2], this.inertiaPtr); } else { this.setBodyMassDataFn(bodyHandle, mass, center[0], center[1], center[2], 0); } }
  getBodyMassData(bodyHandle: BodyId): BodyMassData { this.getBodyMassDataFn(bodyHandle, this.massDataPtr); const heap = this.module.HEAPF32; const base = this.massDataPtr >> 2; return { mass: heap[base], inertiaTrace: heap[base + 1] }; }
  applyBodyMassFromShapes(bodyHandle: BodyId): void { this.applyBodyMassFromShapesFn(bodyHandle); }
  setBodyTargetTransform(bodyHandle: BodyId, position: Vec3, rotation: Quat, timeStep: number, wake = true): void { this.setBodyTargetTransformFn(bodyHandle, position[0], position[1], position[2], rotation[0], rotation[1], rotation[2], rotation[3], timeStep, wake ? 1 : 0); }
  bodyEnable(bodyHandle: BodyId): void { this.bodyEnableFn(bodyHandle); }
  bodyDisable(bodyHandle: BodyId): void { this.bodyDisableFn(bodyHandle); }
  bodyIsEnabled(bodyHandle: BodyId): boolean { return this.bodyIsEnabledFn(bodyHandle) !== 0; }
  getBodyMass(bodyHandle: BodyId): number { return this.getBodyMassFn(bodyHandle); }
  getBodyLocalRotationalInertia(bodyHandle: BodyId): Mat3 { this.getBodyLocalRotationalInertiaFn(bodyHandle, this.inertiaPtr); const heap = this.module.HEAPF32; const base = this.inertiaPtr >> 2; return [heap[base + 0], heap[base + 1], heap[base + 2], heap[base + 3], heap[base + 4], heap[base + 5], heap[base + 6], heap[base + 7], heap[base + 8]]; }
  getBodyWorldCenter(bodyHandle: BodyId): Vec3 { return this.getBodyWorldCenterTo(bodyHandle, [0, 0, 0]); }
  getBodyWorldCenterTo(bodyHandle: BodyId, out: Vec3): Vec3 { this.getBodyWorldCenterFn(bodyHandle, this.pointPtr); return this.readPointInto(out); }
  getBodyWorldPoint(bodyHandle: BodyId, localPoint: Vec3): Vec3 { return this.getBodyWorldPointTo(bodyHandle, localPoint, [0, 0, 0]); }
  getBodyWorldPointXYZ(bodyHandle: BodyId, localX: number, localY: number, localZ: number): Vec3 { return this.getBodyWorldPointXYZTo(bodyHandle, localX, localY, localZ, [0, 0, 0]); }
  getBodyWorldPointTo(bodyHandle: BodyId, localPoint: Vec3, out: Vec3): Vec3 { return this.getBodyWorldPointXYZTo(bodyHandle, localPoint[0], localPoint[1], localPoint[2], out); }
  getBodyWorldPointXYZTo(bodyHandle: BodyId, localX: number, localY: number, localZ: number, out: Vec3): Vec3 { this.getBodyWorldPointFn(bodyHandle, localX, localY, localZ, this.pointPtr); return this.readPointInto(out); }
  getBodyLocalPointVelocity(bodyHandle: BodyId, localPoint: Vec3): Vec3 { return this.getBodyLocalPointVelocityTo(bodyHandle, localPoint, [0, 0, 0]); }
  getBodyLocalPointVelocityXYZ(bodyHandle: BodyId, localX: number, localY: number, localZ: number): Vec3 { return this.getBodyLocalPointVelocityXYZTo(bodyHandle, localX, localY, localZ, [0, 0, 0]); }
  getBodyLocalPointVelocityTo(bodyHandle: BodyId, localPoint: Vec3, out: Vec3): Vec3 { return this.getBodyLocalPointVelocityXYZTo(bodyHandle, localPoint[0], localPoint[1], localPoint[2], out); }
  getBodyLocalPointVelocityXYZTo(bodyHandle: BodyId, localX: number, localY: number, localZ: number, out: Vec3): Vec3 { this.getBodyLocalPointVelocityFn(bodyHandle, localX, localY, localZ, this.pointPtr); return this.readPointInto(out); }
  getBodyWorldPointVelocity(bodyHandle: BodyId, worldPoint: Vec3): Vec3 { return this.getBodyWorldPointVelocityTo(bodyHandle, worldPoint, [0, 0, 0]); }
  getBodyWorldPointVelocityXYZ(bodyHandle: BodyId, worldX: number, worldY: number, worldZ: number): Vec3 { return this.getBodyWorldPointVelocityXYZTo(bodyHandle, worldX, worldY, worldZ, [0, 0, 0]); }
  getBodyWorldPointVelocityTo(bodyHandle: BodyId, worldPoint: Vec3, out: Vec3): Vec3 { return this.getBodyWorldPointVelocityXYZTo(bodyHandle, worldPoint[0], worldPoint[1], worldPoint[2], out); }
  getBodyWorldPointVelocityXYZTo(bodyHandle: BodyId, worldX: number, worldY: number, worldZ: number, out: Vec3): Vec3 { this.getBodyWorldPointVelocityFn(bodyHandle, worldX, worldY, worldZ, this.pointPtr); return this.readPointInto(out); }
  getJointConstraintForce(jointHandle: JointId): Vec3 { this.getJointConstraintForceFn(jointHandle, this.pointPtr); return this.readPointInto([0, 0, 0]); }
  getJointConstraintTorque(jointHandle: JointId): Vec3 { this.getJointConstraintTorqueFn(jointHandle, this.pointPtr); return this.readPointInto([0, 0, 0]); }
  getJointLinearSeparation(jointHandle: JointId): number { return this.getJointLinearSeparationFn(jointHandle); }
  setRevoluteJointTargetAngle(jointHandle: JointId, targetRadians: number): void { this.revoluteJointSetTargetAngleFn(jointHandle, targetRadians); }
  setPrismaticMotorSpeed(jointHandle: JointId, motorSpeed: number): void { this.prismaticJointSetMotorSpeedFn(jointHandle, motorSpeed); }
  getPrismaticTranslation(jointHandle: JointId): number { return this.prismaticJointGetTranslationFn(jointHandle); }
  createPrismaticJoint(worldHandle: WorldHandle, bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; constraintHertz?: number; constraintDampingRatio?: number; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetTranslation?: number; enableLimit?: boolean; lowerTranslation?: number; upperTranslation?: number; enableMotor?: boolean; maxMotorForce?: number; motorSpeed?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointId { const la = options.localFrameA?.position ?? [0,0,0]; const laq = options.localFrameA?.rotation ?? [0,0,0,1]; const lb = options.localFrameB?.position ?? [0,0,0]; const lbq = options.localFrameB?.rotation ?? [0,0,0,1]; const [forceThreshold, torqueThreshold, collideConnected] = jointThresholdArgs(options); { const jointId = asJointId(this.createPrismaticJointFn(worldHandle, bodyAHandle, bodyBHandle, la[0], la[1], la[2], laq[0], laq[1], laq[2], laq[3], lb[0], lb[1], lb[2], lbq[0], lbq[1], lbq[2], lbq[3], options.constraintHertz ?? 60, options.constraintDampingRatio ?? 2, options.enableSpring ? 1 : 0, options.hertz ?? 0, options.dampingRatio ?? 0, options.targetTranslation ?? 0, options.enableLimit ? 1 : 0, options.lowerTranslation ?? 0, options.upperTranslation ?? 0, options.enableMotor ? 1 : 0, options.maxMotorForce ?? 0, options.motorSpeed ?? 0, forceThreshold, torqueThreshold, collideConnected));
    if (jointId === 0n) throw new Error("createPrismaticJointFn failed");
    return jointId; } }
  createWeldJoint(worldHandle: WorldHandle, bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; linearHertz?: number; angularHertz?: number; linearDampingRatio?: number; angularDampingRatio?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointId { const la = options.localFrameA?.position ?? [0,0,0]; const laq = options.localFrameA?.rotation ?? [0,0,0,1]; const lb = options.localFrameB?.position ?? [0,0,0]; const lbq = options.localFrameB?.rotation ?? [0,0,0,1]; const [forceThreshold, torqueThreshold, collideConnected] = jointThresholdArgs(options); { const jointId = asJointId(this.createWeldJointFn(worldHandle, bodyAHandle, bodyBHandle, la[0], la[1], la[2], laq[0], laq[1], laq[2], laq[3], lb[0], lb[1], lb[2], lbq[0], lbq[1], lbq[2], lbq[3], options.linearHertz ?? 0, options.angularHertz ?? 0, options.linearDampingRatio ?? 0, options.angularDampingRatio ?? 0, forceThreshold, torqueThreshold, collideConnected));
    if (jointId === 0n) throw new Error("createWeldJointFn failed");
    return jointId; } }
  createDistanceJoint(worldHandle: WorldHandle, bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; length?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointId { const la = options.localFrameA?.position ?? [0,0,0]; const laq = options.localFrameA?.rotation ?? [0,0,0,1]; const lb = options.localFrameB?.position ?? [0,0,0]; const lbq = options.localFrameB?.rotation ?? [0,0,0,1]; const [forceThreshold, torqueThreshold, collideConnected] = jointThresholdArgs(options); { const jointId = asJointId(this.createDistanceJointFn(worldHandle, bodyAHandle, bodyBHandle, la[0], la[1], la[2], laq[0], laq[1], laq[2], laq[3], lb[0], lb[1], lb[2], lbq[0], lbq[1], lbq[2], lbq[3], options.length ?? 0, forceThreshold, torqueThreshold, collideConnected));
    if (jointId === 0n) throw new Error("createDistanceJointFn failed");
    return jointId; } }
  createParallelJoint(worldHandle: WorldHandle, bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; hertz?: number; dampingRatio?: number; maxTorque?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointId {
    const la = options.localFrameA?.position ?? [0, 0, 0];
    const laq = options.localFrameA?.rotation ?? [0, 0, 0, 1];
    const lb = options.localFrameB?.position ?? [0, 0, 0];
    const lbq = options.localFrameB?.rotation ?? [0, 0, 0, 1];
    const [forceThreshold, torqueThreshold, collideConnected] = jointThresholdArgs(options);
    const jointId = asJointId(this.createParallelJointFn(
      worldHandle, bodyAHandle, bodyBHandle,
      la[0], la[1], la[2], laq[0], laq[1], laq[2], laq[3],
      lb[0], lb[1], lb[2], lbq[0], lbq[1], lbq[2], lbq[3],
      options.hertz ?? 1, options.dampingRatio ?? 1, options.maxTorque ?? DEFAULT_JOINT_FORCE_THRESHOLD,
      forceThreshold, torqueThreshold, collideConnected,
    ));
    if (jointId === 0n) throw new Error("b3wCreateParallelJoint failed");
    return jointId;
  }
  createWheelJoint(worldHandle: WorldHandle, bodyAHandle: BodyId, bodyBHandle: BodyId, options: {
    localFrameA?: { position?: Vec3; rotation?: Quat };
    localFrameB?: { position?: Vec3; rotation?: Quat };
    enableSuspensionSpring?: boolean;
    suspensionHertz?: number;
    suspensionDampingRatio?: number;
    enableSuspensionLimit?: boolean;
    lowerSuspensionLimit?: number;
    upperSuspensionLimit?: number;
    enableSpinMotor?: boolean;
    maxSpinTorque?: number;
    spinSpeed?: number;
    enableSteering?: boolean;
    steeringHertz?: number;
    steeringDampingRatio?: number;
    targetSteeringAngle?: number;
    maxSteeringTorque?: number;
    enableSteeringLimit?: boolean;
    lowerSteeringLimit?: number;
    upperSteeringLimit?: number;
    collideConnected?: boolean;
  } = {}): JointId {
    const la = options.localFrameA?.position ?? [0, 0, 0];
    const laq = options.localFrameA?.rotation ?? [0, 0, 0, 1];
    const lb = options.localFrameB?.position ?? [0, 0, 0];
    const lbq = options.localFrameB?.rotation ?? [0, 0, 0, 1];
    const jointId = asJointId(this.createWheelJointFn(
      worldHandle, bodyAHandle, bodyBHandle,
      la[0], la[1], la[2], laq[0], laq[1], laq[2], laq[3],
      lb[0], lb[1], lb[2], lbq[0], lbq[1], lbq[2], lbq[3],
      options.enableSuspensionSpring === false ? 0 : 1,
      options.suspensionHertz ?? 1,
      options.suspensionDampingRatio ?? 0.7,
      options.enableSuspensionLimit ? 1 : 0,
      options.lowerSuspensionLimit ?? 0,
      options.upperSuspensionLimit ?? 0,
      options.enableSpinMotor ? 1 : 0,
      options.maxSpinTorque ?? 0,
      options.spinSpeed ?? 0,
      options.enableSteering ? 1 : 0,
      options.steeringHertz ?? 1,
      options.steeringDampingRatio ?? 0.7,
      options.targetSteeringAngle ?? 0,
      options.maxSteeringTorque ?? 0,
      options.enableSteeringLimit ? 1 : 0,
      options.lowerSteeringLimit ?? 0,
      options.upperSteeringLimit ?? 0,
      options.collideConnected ? 1 : 0,
    ));
    if (jointId === 0n) throw new Error("b3wCreateWheelJoint failed");
    return jointId;
  }
  worldExplode(worldHandle: WorldHandle, position: Vec3, radius: number, falloff: number, impulsePerArea: number, maskBits = U64_MAX): void { this.worldExplodeFn(worldHandle, position[0], position[1], position[2], radius, falloff, impulsePerArea, maskBits); }

  applyLinearImpulse(bodyHandle: BodyId, impulse: Vec3, point: Vec3, wake = true): void { this.applyLinearImpulseFn(bodyHandle, impulse[0], impulse[1], impulse[2], point[0], point[1], point[2], wake ? 1 : 0); }
  applyLinearImpulseToCenter(bodyHandle: BodyId, impulse: Vec3, wake = true): void { this.applyLinearImpulseToCenterFn(bodyHandle, impulse[0], impulse[1], impulse[2], wake ? 1 : 0); }
}

export class PhysicsWorld {
  constructor(private readonly runtime: Box3DRuntime, public readonly handle: WorldHandle) {}
  createBody(def: BodyDef = {}): BodyId { return this.runtime.createBody(this.handle, def); }
  createBox(options: BoxOptions): BodyId { return this.runtime.createBox(this.handle, options); }
  createBoxWithShape(options: BoxOptions): ShapeHandle { return this.runtime.createBoxWithShape(this.handle, options); }
  createSphere(options: SphereOptions): BodyId { return this.runtime.createSphere(this.handle, options); }
  createSphereWithShape(options: SphereOptions): ShapeHandle { return this.runtime.createSphereWithShape(this.handle, options); }
  createSphereShape(bodyHandle: BodyId, center: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle { return this.runtime.createSphereShape(bodyHandle, center, radius, def); }
  createCapsuleShape(bodyHandle: BodyId, center1: Vec3, center2: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle { return this.runtime.createCapsuleShape(bodyHandle, center1, center2, radius, def); }
  createHullShape(bodyHandle: BodyId, halfWidths: Vec3, def: ShapeDef = {}): ShapeHandle { return this.runtime.createHullShape(bodyHandle, halfWidths, def); }
  createTransformedHullShape(bodyHandle: BodyId, halfWidths: Vec3, transform?: { position?: Vec3; rotation?: Quat }, scale?: Vec3, def?: ShapeDef): ShapeHandle { return this.runtime.createTransformedHullShape(bodyHandle, halfWidths, transform, scale, def); }
  createOffsetHullShape(bodyHandle: BodyId, halfWidths: Vec3, offset: Vec3, def?: ShapeDef): ShapeHandle { return this.runtime.createOffsetHullShape(bodyHandle, halfWidths, offset, def); }
  createShapeFromHull(bodyHandle: BodyId, hullHandle: HullHandle, def?: ShapeDef): ShapeId { return this.runtime.createShapeFromHull(bodyHandle, hullHandle, def); }
  createGridMesh(xCount: number, zCount: number, cellWidth: number, materialCount = 1, identifyEdges = true): MeshHandle { return this.runtime.createGridMesh(this.handle, xCount, zCount, cellWidth, materialCount, identifyEdges); }
  createWaveMesh(xCount: number, zCount: number, cellWidth: number, amplitude: number, rowFrequency: number, columnFrequency: number): MeshHandle { return this.runtime.createWaveMesh(this.handle, xCount, zCount, cellWidth, amplitude, rowFrequency, columnFrequency); }
  createBoxMesh(center: Vec3, extent: Vec3, identifyEdges = true): MeshHandle { return this.runtime.createBoxMesh(this.handle, center, extent, identifyEdges); }
  createHollowBoxMesh(center: Vec3, extent: Vec3): MeshHandle { return this.runtime.createHollowBoxMesh(this.handle, center, extent); }
  createTorusMesh(radialResolution: number, tubularResolution: number, radius: number, thickness: number): MeshHandle { return this.runtime.createTorusMesh(this.handle, radialResolution, tubularResolution, radius, thickness); }
  createMesh(vertices: ArrayLike<number>, indices: ArrayLike<number>, options?: { useMedianSplit?: boolean; identifyEdges?: boolean }): MeshHandle {
    return this.runtime.createMesh(this.handle, vertices, indices, options);
  }
  destroyMesh(meshHandle: MeshHandle): void { this.runtime.destroyMesh(meshHandle); }
  createMeshShape(bodyHandle: BodyId, meshHandle: MeshHandle, def: MeshShapeOptions = {}): ShapeHandle { return this.runtime.createMeshShape(bodyHandle, meshHandle, def); }
  createWave(rowCount: number, columnCount: number, scale: Vec3, rowFrequency: number, columnFrequency: number, makeHoles = false): HeightFieldHandle {
    return this.runtime.createWave(this.handle, rowCount, columnCount, scale, rowFrequency, columnFrequency, makeHoles);
  }
  createGridHeightField(rowCount: number, columnCount: number, scale: Vec3, makeHoles = false): HeightFieldHandle {
    return this.runtime.createGridHeightField(this.handle, rowCount, columnCount, scale, makeHoles);
  }
  destroyHeightField(heightFieldHandle: HeightFieldHandle): void { this.runtime.destroyHeightField(heightFieldHandle); }
  createHeightFieldShape(bodyHandle: BodyId, heightFieldHandle: HeightFieldHandle, def: ShapeDef = {}): ShapeHandle {
    return this.runtime.createHeightFieldShape(bodyHandle, heightFieldHandle, def);
  }
  setMesh(shapeHandle: ShapeId | ShapeHandle, meshHandle: MeshHandle, scale: Vec3 = [1, 1, 1]): void { this.runtime.setMesh(shapeHandle, meshHandle, scale); }
  createCompoundShape(bodyHandle: BodyId, compoundHandle: CompoundHandle, density = 1): ShapeId { return this.runtime.createCompoundShape(bodyHandle, compoundHandle, density); }
  getBodyShapes(bodyHandle: BodyId): ShapeId[] { return this.runtime.getBodyShapes(bodyHandle); }
  getCompoundTreeHeight(compoundHandle: CompoundHandle): number { return this.runtime.getCompoundTreeHeight(compoundHandle); }
  destroyCompound(compoundHandle: CompoundHandle): void { this.runtime.destroyCompound(compoundHandle); }
  destroyBody(bodyHandle: BodyId): void { this.runtime.destroyBody(bodyHandle); }
  bodyIsValid(bodyHandle: BodyId): boolean { return this.runtime.bodyIsValid(bodyHandle); }
  destroyShape(shapeHandle: ShapeId | ShapeHandle, updateBodyMass = true): void { this.runtime.destroyShape(shapeHandle, updateBodyMass); }
  destroyJoint(jointHandle: JointId): void { this.runtime.destroyJoint(jointHandle); }
  setBodyTransform(bodyHandle: BodyId, position: Vec3, rotation: Quat = [0,0,0,1]): void { this.runtime.setBodyTransform(bodyHandle, position, rotation); }
  setBodyLinearVelocity(bodyHandle: BodyId, velocity: Vec3): void { this.runtime.setBodyLinearVelocity(bodyHandle, velocity); }
  setBodyAngularVelocity(bodyHandle: BodyId, velocity: Vec3): void { this.runtime.setBodyAngularVelocity(bodyHandle, velocity); }
  getBodyLinearVelocity(bodyHandle: BodyId): Vec3 { return this.runtime.getBodyLinearVelocity(bodyHandle); }
  getBodyLinearVelocityTo(bodyHandle: BodyId, out: Vec3): Vec3 { return this.runtime.getBodyLinearVelocityTo(bodyHandle, out); }
  getBodyAngularVelocity(bodyHandle: BodyId): Vec3 { return this.runtime.getBodyAngularVelocity(bodyHandle); }
  getBodyAngularVelocityTo(bodyHandle: BodyId, out: Vec3): Vec3 { return this.runtime.getBodyAngularVelocityTo(bodyHandle, out); }
  applyLinearImpulse(bodyHandle: BodyId, impulse: Vec3, point: Vec3, wake = true): void { this.runtime.applyLinearImpulse(bodyHandle, impulse, point, wake); }
  applyLinearImpulseToCenter(bodyHandle: BodyId, impulse: Vec3, wake = true): void { this.runtime.applyLinearImpulseToCenter(bodyHandle, impulse, wake); }
  bodyIsAwake(bodyHandle: BodyId): boolean { return this.runtime.bodyIsAwake(bodyHandle); }
  getBodyDebugColor(bodyHandle: BodyId): number { return this.runtime.getBodyDebugColor(bodyHandle); }
  getBodyType(bodyHandle: BodyId): BodyType { return this.runtime.getBodyType(bodyHandle); }
  setBodyAwake(bodyHandle: BodyId, awake: boolean): void { this.runtime.setBodyAwake(bodyHandle, awake); }
  setBodyDamping(bodyHandle: BodyId, linearDamping: number, angularDamping: number): void { this.runtime.setBodyDamping(bodyHandle, linearDamping, angularDamping); }
  getBodyLocalPoint(bodyHandle: BodyId, worldPoint: Vec3): Vec3 { return this.runtime.getBodyLocalPoint(bodyHandle, worldPoint); }
  getBodyLocalPointXYZ(bodyHandle: BodyId, worldX: number, worldY: number, worldZ: number): Vec3 { return this.runtime.getBodyLocalPointXYZ(bodyHandle, worldX, worldY, worldZ); }
  getBodyLocalPointTo(bodyHandle: BodyId, worldPoint: Vec3, out: Vec3): Vec3 { return this.runtime.getBodyLocalPointTo(bodyHandle, worldPoint, out); }
  getBodyLocalPointXYZTo(bodyHandle: BodyId, worldX: number, worldY: number, worldZ: number, out: Vec3): Vec3 { return this.runtime.getBodyLocalPointXYZTo(bodyHandle, worldX, worldY, worldZ, out); }
  createMotorJoint(bodyAHandle: BodyId, bodyBHandle: BodyId, options: MotorJointOptions = {}): JointId { return this.runtime.createMotorJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createFilterJoint(bodyAHandle: BodyId, bodyBHandle: BodyId): JointId { return this.runtime.createFilterJoint(this.handle, bodyAHandle, bodyBHandle); }
  createRevoluteJoint(bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; constraintHertz?: number; constraintDampingRatio?: number; targetAngle?: number; enableSpring?: boolean; hertz?: number; dampingRatio?: number; enableLimit?: boolean; lowerAngle?: number; upperAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorSpeed?: number } = {}): JointId { return this.runtime.createRevoluteJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createSphericalJoint(bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetRotation?: Quat; enableConeLimit?: boolean; coneAngle?: number; enableTwistLimit?: boolean; lowerTwistAngle?: number; upperTwistAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorVelocity?: Vec3 } = {}): JointId { return this.runtime.createSphericalJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createHuman(position: Vec3, options: { frictionTorque?: number; hertz?: number; dampingRatio?: number; groupIndex?: number; colorize?: boolean } = {}): HumanHandle { return this.runtime.createHuman(this.handle, position, options); }
  createHumanParallelAnchors(humanHandle: HumanHandle): void { this.runtime.createHumanParallelAnchors(humanHandle); }
  getHumanBoneBody(humanHandle: HumanHandle, boneIndex: number): BodyId { return this.runtime.getHumanBoneBody(humanHandle, boneIndex); }
  getHumanBoneCount(): number { return this.runtime.getHumanBoneCount(); }
  getHumanAnchorBody(humanHandle: HumanHandle, boneIndex: number): BodyId { return this.runtime.getHumanAnchorBody(humanHandle, boneIndex); }
  getBodyTransform(bodyHandle: BodyId): BodyTransform { return this.runtime.readBodyTransform(bodyHandle); }
  getBodyMassData(bodyHandle: BodyId): BodyMassData { return this.runtime.getBodyMassData(bodyHandle); }
  bodyEnable(bodyHandle: BodyId): void { this.runtime.bodyEnable(bodyHandle); }
  bodyDisable(bodyHandle: BodyId): void { this.runtime.bodyDisable(bodyHandle); }
  bodyIsEnabled(bodyHandle: BodyId): boolean { return this.runtime.bodyIsEnabled(bodyHandle); }
  getBodyMass(bodyHandle: BodyId): number { return this.runtime.getBodyMass(bodyHandle); }
  getBodyLocalRotationalInertia(bodyHandle: BodyId): Mat3 { return this.runtime.getBodyLocalRotationalInertia(bodyHandle); }
  getBodyWorldCenter(bodyHandle: BodyId): Vec3 { return this.runtime.getBodyWorldCenter(bodyHandle); }
  getBodyWorldCenterTo(bodyHandle: BodyId, out: Vec3): Vec3 { return this.runtime.getBodyWorldCenterTo(bodyHandle, out); }
  getBodyWorldPoint(bodyHandle: BodyId, localPoint: Vec3): Vec3 { return this.runtime.getBodyWorldPoint(bodyHandle, localPoint); }
  getBodyWorldPointXYZ(bodyHandle: BodyId, localX: number, localY: number, localZ: number): Vec3 { return this.runtime.getBodyWorldPointXYZ(bodyHandle, localX, localY, localZ); }
  getBodyWorldPointTo(bodyHandle: BodyId, localPoint: Vec3, out: Vec3): Vec3 { return this.runtime.getBodyWorldPointTo(bodyHandle, localPoint, out); }
  getBodyWorldPointXYZTo(bodyHandle: BodyId, localX: number, localY: number, localZ: number, out: Vec3): Vec3 { return this.runtime.getBodyWorldPointXYZTo(bodyHandle, localX, localY, localZ, out); }
  getBodyLocalPointVelocity(bodyHandle: BodyId, localPoint: Vec3): Vec3 { return this.runtime.getBodyLocalPointVelocity(bodyHandle, localPoint); }
  getBodyLocalPointVelocityXYZ(bodyHandle: BodyId, localX: number, localY: number, localZ: number): Vec3 { return this.runtime.getBodyLocalPointVelocityXYZ(bodyHandle, localX, localY, localZ); }
  getBodyLocalPointVelocityTo(bodyHandle: BodyId, localPoint: Vec3, out: Vec3): Vec3 { return this.runtime.getBodyLocalPointVelocityTo(bodyHandle, localPoint, out); }
  getBodyLocalPointVelocityXYZTo(bodyHandle: BodyId, localX: number, localY: number, localZ: number, out: Vec3): Vec3 { return this.runtime.getBodyLocalPointVelocityXYZTo(bodyHandle, localX, localY, localZ, out); }
  getBodyWorldPointVelocity(bodyHandle: BodyId, worldPoint: Vec3): Vec3 { return this.runtime.getBodyWorldPointVelocity(bodyHandle, worldPoint); }
  getBodyWorldPointVelocityXYZ(bodyHandle: BodyId, worldX: number, worldY: number, worldZ: number): Vec3 { return this.runtime.getBodyWorldPointVelocityXYZ(bodyHandle, worldX, worldY, worldZ); }
  getBodyWorldPointVelocityTo(bodyHandle: BodyId, worldPoint: Vec3, out: Vec3): Vec3 { return this.runtime.getBodyWorldPointVelocityTo(bodyHandle, worldPoint, out); }
  getBodyWorldPointVelocityXYZTo(bodyHandle: BodyId, worldX: number, worldY: number, worldZ: number, out: Vec3): Vec3 { return this.runtime.getBodyWorldPointVelocityXYZTo(bodyHandle, worldX, worldY, worldZ, out); }
  getJointConstraintForce(jointHandle: JointId): Vec3 { return this.runtime.getJointConstraintForce(jointHandle); }
  getJointConstraintTorque(jointHandle: JointId): Vec3 { return this.runtime.getJointConstraintTorque(jointHandle); }
  getJointLinearSeparation(jointHandle: JointId): number { return this.runtime.getJointLinearSeparation(jointHandle); }
  setRevoluteJointTargetAngle(jointHandle: JointId, targetRadians: number): void { this.runtime.setRevoluteJointTargetAngle(jointHandle, targetRadians); }
  setPrismaticMotorSpeed(jointHandle: JointId, motorSpeed: number): void { this.runtime.setPrismaticMotorSpeed(jointHandle, motorSpeed); }
  getPrismaticTranslation(jointHandle: JointId): number { return this.runtime.getPrismaticTranslation(jointHandle); }
  createPrismaticJoint(bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; constraintHertz?: number; constraintDampingRatio?: number; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetTranslation?: number; enableLimit?: boolean; lowerTranslation?: number; upperTranslation?: number; enableMotor?: boolean; maxMotorForce?: number; motorSpeed?: number } = {}): JointId { return this.runtime.createPrismaticJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createWeldJoint(bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; linearHertz?: number; angularHertz?: number; linearDampingRatio?: number; angularDampingRatio?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointId { return this.runtime.createWeldJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createDistanceJoint(bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; length?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointId { return this.runtime.createDistanceJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createParallelJoint(bodyAHandle: BodyId, bodyBHandle: BodyId, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; hertz?: number; dampingRatio?: number; maxTorque?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointId { return this.runtime.createParallelJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createWheelJoint(bodyAHandle: BodyId, bodyBHandle: BodyId, options: Parameters<Box3DRuntime["createWheelJoint"]>[3] = {}): JointId {
    return this.runtime.createWheelJoint(this.handle, bodyAHandle, bodyBHandle, options);
  }
  explode(position: Vec3, radius: number, falloff: number, impulsePerArea: number, maskBits = U64_MAX): void { this.runtime.worldExplode(this.handle, position, radius, falloff, impulsePerArea, maskBits); }
  getCounters(): WorldCounters { return this.runtime.getWorldCounters(this.handle); }
  getAwakeBodyCount(): number { return this.runtime.getWorldAwakeBodyCount(this.handle); }
  getWorkerCount(): number { return this.runtime.getWorldWorkerCount(this.handle); }
  getProfile(): WorldProfile { return this.runtime.getWorldProfile(this.handle); }
  rayCastClosest(origin: Vec3, translation: Vec3, categoryBits = U64_MAX, maskBits = U64_MAX): { shapeHandle: ShapeId; bodyHandle: BodyId; point: Vec3; normal: Vec3; fraction: number } | null { return this.runtime.rayCastClosest(this.handle, origin, translation, categoryBits, maskBits); }
  overlapAABB(min: Vec3, max: Vec3, categoryBits = U64_MAX, maskBits = U64_MAX): number {
    return this.runtime.overlapAABB(this.handle, min, max, categoryBits, maskBits);
  }
  castShapeSphere(origin: Vec3, translation: Vec3, radius: number, categoryBits = U64_MAX, maskBits = U64_MAX): number {
    return this.runtime.castShapeSphere(this.handle, origin, translation, radius, categoryBits, maskBits);
  }
  bodyCastRay(
    bodyHandle: BodyId,
    origin: Vec3,
    translation: Vec3,
    options?: { categoryBits?: number; maskBits?: number; maxFraction?: number; bodyTransform?: BodyTransform },
  ): { hit: boolean; point: Vec3; normal: Vec3; fraction: number } {
    return this.runtime.bodyCastRay(bodyHandle, origin, translation, options);
  }
  getSensorBeginEvents(): SensorBeginEvent[] { return this.runtime.getSensorBeginEvents(this.handle); }
  getJointEventHandles(): JointId[] { return this.runtime.getJointEventHandles(this.handle); }
  allocBodyBatchBuffers(capacity: number): BodyBatchBuffers { return this.runtime.allocBodyBatchBuffers(capacity); }
  freeBodyBatchBuffers(buffers: BodyBatchBuffers): void { this.runtime.freeBodyBatchBuffers(buffers); }
  getMemoryView(): RuntimeMemoryView32 { return this.runtime.getMemoryView(); }
  getWasmMemory(): WebAssembly.Memory | undefined { return this.runtime.getWasmMemory(); }
  writeBodyHandles(buffers: BodyBatchBuffers, bodyHandles: readonly BodyId[]): void { this.runtime.writeBodyHandles(buffers, bodyHandles); }
  writeBodyTransforms(count: number, bodyHandlesPtr: number, outPositionsPtr: number, outRotationsPtr: number, outAwakePtr: number, outColorsPtr: number): void {
    this.runtime.writeBodyTransforms(count, bodyHandlesPtr, outPositionsPtr, outRotationsPtr, outAwakePtr, outColorsPtr);
  }

  writeBodyTransformsLight(count: number, bodyHandlesPtr: number, outPositionsPtr: number, outRotationsPtr: number, outAwakePtr: number, outColorsPtr: number): void {
    this.runtime.writeBodyTransformsLight(count, bodyHandlesPtr, outPositionsPtr, outRotationsPtr, outAwakePtr, outColorsPtr);
  }

  configureBodyMoveTracking(count: number, bodyHandlesPtr: number): void {
    this.runtime.configureBodyMoveTracking(count, bodyHandlesPtr);
  }

  clearBodyMoveTracking(): void {
    this.runtime.clearBodyMoveTracking();
  }

  scatterBodyMoveEvents(outPositionsPtr: number, outRotationsPtr: number, outAwakePtr: number, outColorsPtr: number, useLightColors = true): number {
    return this.runtime.scatterBodyMoveEvents(this.handle, outPositionsPtr, outRotationsPtr, outAwakePtr, outColorsPtr, useLightColors);
  }

  getBodyMoveEventCount(): number {
    return this.runtime.getBodyMoveEventCount(this.handle);
  }

  step(dt = 1 / 60, substeps = 4): void { this.runtime.step(this.handle, dt, substeps); }
  enableSleeping(flag: boolean): void { this.runtime.enableWorldSleeping(this.handle, flag); }
  enableContinuous(flag: boolean): void { this.runtime.enableWorldContinuous(this.handle, flag); }
  enableWarmStarting(flag: boolean): void { this.runtime.enableWorldWarmStarting(this.handle, flag); }
  setProfileLevel(level: ProfileLevel): void { this.runtime.setWorldProfileLevel(this.handle, level); }
  getProfileLevel(): ProfileLevel { return this.runtime.getWorldProfileLevel(this.handle); }
  setContactTuning(hertz: number, dampingRatio: number, contactSpeed: number): void { this.runtime.setWorldContactTuning(this.handle, hertz, dampingRatio, contactSpeed); }
  setContactRecycleDistance(distance: number): void { this.runtime.setWorldContactRecycleDistance(this.handle, distance); }
  setWorkerCount(count: number): void { this.runtime.setWorldWorkerCount(this.handle, count); }
  destroy(): void { this.runtime.destroyWorld(this.handle); }
}
