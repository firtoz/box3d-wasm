export type Vec3 = [number, number, number];
export type Quat = [number, number, number, number];

declare const handleBrand: unique symbol;

type Handle<Name extends string> = number & { readonly [handleBrand]: Name };

export type WorldHandle = Handle<"WorldHandle">;
export type BodyHandle = Handle<"BodyHandle">;
export type ShapeId = Handle<"ShapeId">;
export type JointHandle = Handle<"JointHandle">;
export type HullHandle = Handle<"HullHandle">;
export type MeshHandle = Handle<"MeshHandle">;
export type CompoundHandle = Handle<"CompoundHandle">;
export type HumanHandle = Handle<"HumanHandle">;

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

export enum BodyType { Static = 0, Kinematic = 1, Dynamic = 2 }

export interface WorldCapacity {
  staticShapeCount?: number;
  dynamicShapeCount?: number;
  staticBodyCount?: number;
  dynamicBodyCount?: number;
  contactCount?: number;
}

export interface WorldOptions { gravity?: Vec3; workerCount?: number; capacity?: WorldCapacity; }

export type SlotKind = "worlds" | "bodies" | "joints" | "hulls" | "shapes" | "meshes" | "compounds" | "humans";

export interface SlotLimits {
  worlds: number;
  bodies: number;
  joints: number;
  hulls: number;
  shapes: number;
  meshes: number;
  compounds: number;
  humans: number;
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
export interface ShapeHandle { bodyHandle: BodyHandle; shapeHandle: ShapeId; }
export interface MeshShapeOptions extends ShapeDef { scale?: Vec3; }
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

type CModule = { cwrap(name: string, returnType: "number", argTypes: readonly string[]): (...args: number[]) => number; cwrap(name: string, returnType: null, argTypes: readonly string[]): (...args: number[]) => void; HEAPF32: Float32Array; HEAPU8: Uint8Array; HEAP32: Int32Array; wasmMemory?: WebAssembly.Memory; _malloc(size: number): number; _free(ptr: number): void; };
type CreateWorldFn = (
  gravityX: number, gravityY: number, gravityZ: number, workerCount: number,
  staticShapeCount: number, dynamicShapeCount: number, staticBodyCount: number, dynamicBodyCount: number, contactCount: number,
) => number;
type GetSlotLimitsFn = (outLimits: number) => void;
type GetSlotUsageFn = (outUsage: number) => void;
type CreateBodyFn = (worldHandle: number, bodyType: number, px: number, py: number, pz: number, enableSleep: number, awake: number) => number;
type DestroyWorldFn = (worldHandle: number) => void;
type CreateBoxFn = (worldHandle: number, px: number, py: number, pz: number, hx: number, hy: number, hz: number, isStatic: number, density: number) => number;
type CreateSphereFn = (worldHandle: number, px: number, py: number, pz: number, radius: number, vx: number, vy: number, vz: number, density: number) => number;
type CreateHullShapeFn = (bodyHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, updateBodyMass: number, tx: number, ty: number, tz: number, qx: number, qy: number, qz: number, qw: number, hx: number, hy: number, hz: number) => number;
type CreateTransformedHullShapeFn = (bodyHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, tx: number, ty: number, tz: number, qx: number, qy: number, qz: number, qw: number, hx: number, hy: number, hz: number, sx: number, sy: number, sz: number) => number;
type CreateSphereShapeFn = (bodyHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, px: number, py: number, pz: number, radius: number, invokeContactCreation: number) => number;
type CreateCapsuleShapeFn = (bodyHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, ax: number, ay: number, az: number, bx: number, by: number, bz: number, radius: number) => number;
type CreateShapeFromHullFn = (bodyHandle: number, hullHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, updateBodyMass: number, explosionScale: number) => number;
type CreateTransformedShapeFromHullFn = (bodyHandle: number, hullHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, updateBodyMass: number, tx: number, ty: number, tz: number, qx: number, qy: number, qz: number, qw: number, sx: number, sy: number, sz: number) => number;
type CreateCylinderFn = (height: number, radius: number, yOffset: number, sides: number) => number;
type CreateGridMeshFn = (worldHandle: number, xCount: number, zCount: number, cellWidth: number, materialCount: number, identifyEdges: number) => number;
type CreateTorusMeshFn = (worldHandle: number, radialResolution: number, tubularResolution: number, radius: number, thickness: number) => number;
type DestroyMeshFn = (meshHandle: number) => void;
type CreateMeshShapeFn = (bodyHandle: number, meshHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, sx: number, sy: number, sz: number) => number;
type ShapeSetMeshFn = (shapeHandle: number, meshHandle: number, sx: number, sy: number, sz: number) => void;
type CreateHullFromPointsFn = (numPoints: number, points: number) => number;
type CreateRockFn = (radius: number) => number;
type DestroyHullFn = (hullHandle: number) => void;
type CreateCompoundFn = (capsuleCount: number, hullCount: number, meshCount: number, sphereCount: number, capsules: number, hulls: number, meshes: number, spheres: number) => number;
type CreateCompoundFromHullsFn = (hullCount: number, hullData: number, strideFloats: number) => number;
type CreateCompoundFromSpheresFn = (sphereCount: number, sphereData: number, strideFloats: number) => number;
type DestroyCompoundFn = (compoundHandle: number) => void;
type GetCompoundTreeHeightFn = (compoundHandle: number) => number;
type CreateCompoundShapeFn = (bodyHandle: number, compoundHandle: number, density: number) => number;
type DestroyBodyFn = (bodyHandle: number) => void;
type DestroyJointFn = (jointHandle: number) => void;
type SetBodyTransformFn = (bodyHandle: number, px: number, py: number, pz: number, qx: number, qy: number, qz: number, qw: number) => void;
type SetBodyLinearVelocityFn = (bodyHandle: number, x: number, y: number, z: number) => void;
type SetBodyAngularVelocityFn = (bodyHandle: number, x: number, y: number, z: number) => void;
type GetBodyVelocityFn = (bodyHandle: number, outVelocity: number) => void;
type SetBodyAwakeFn = (bodyHandle: number, awake: number) => void;
type SetBodyDampingFn = (bodyHandle: number, linearDamping: number, angularDamping: number) => void;
type GetBodyLocalPointFn = (bodyHandle: number, worldX: number, worldY: number, worldZ: number, outPoint: number) => void;
type CreateMotorJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localBx: number, localBy: number, localBz: number, linearVx: number, linearVy: number, linearVz: number, maxVelocityForce: number, angularVx: number, angularVy: number, angularVz: number, maxVelocityTorque: number, collideConnected: number, linearHertz: number, linearDampingRatio: number, maxSpringForce: number, angularHertz: number, angularDampingRatio: number, maxSpringTorque: number) => number;
type CreateFilterJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number) => number;
type CreateRevoluteJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, constraintHertz: number, constraintDampingRatio: number, targetAngle: number, enableSpring: number, hertz: number, dampingRatio: number, enableLimit: number, lowerAngle: number, upperAngle: number, enableMotor: number, maxMotorTorque: number, motorSpeed: number, forceThreshold: number, torqueThreshold: number, collideConnected: number) => number;
type CreateSphericalJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, enableSpring: number, hertz: number, dampingRatio: number, targetQx: number, targetQy: number, targetQz: number, targetQw: number, enableConeLimit: number, coneAngle: number, enableTwistLimit: number, lowerTwistAngle: number, upperTwistAngle: number, enableMotor: number, maxMotorTorque: number, motorVx: number, motorVy: number, motorVz: number) => number;
type CreateHumanFn = (worldHandle: number, px: number, py: number, pz: number, frictionTorque: number, hertz: number, dampingRatio: number, groupIndex: number, colorize: number) => number;
type GetHumanBoneBodyFn = (humanHandle: number, boneIndex: number) => number;
type GetHumanBoneCountFn = () => number;
type HumanSetVelocityFn = (humanHandle: number, x: number, y: number, z: number) => void;
type HumanSetBulletFn = (humanHandle: number, flag: number) => void;
type HumanSetJointFloatFn = (humanHandle: number, value: number) => void;
type StepFn = (worldHandle: number, timeStep: number, subStepCount: number) => void;
type GetBodyTransformFn = (bodyHandle: number, outTransform: number) => void;
type ShapeSetSurfaceMaterialFn = (shapeHandle: number, friction: number, restitution: number, rollingResistance: number, tvx: number, tvy: number, tvz: number) => void;
type ShapeSetFilterFn = (shapeHandle: number, categoryBits: number, maskBits: number, groupIndex: number, invokeContacts: number) => void;
type GetBodyShapeCountFn = (bodyHandle: number) => number;
type GetBodyShapesFn = (bodyHandle: number, outShapeHandles: number, capacity: number) => number;
type DestroyShapeFn = (shapeHandle: number, updateBodyMass: number) => void;
type ShapeEnableBoolFn = (shapeHandle: number, flag: number) => void;
type ShapeSetSphereFn = (shapeHandle: number, px: number, py: number, pz: number, radius: number) => void;
type ShapeSetCapsuleFn = (shapeHandle: number, ax: number, ay: number, az: number, bx: number, by: number, bz: number, radius: number) => void;
type ShapeApplyWindFn = (shapeHandle: number, windX: number, windY: number, windZ: number, drag: number, lift: number, maxSpeed: number, wake: number) => void;
type BodyIsAwakeFn = (bodyHandle: number) => number;
type GetBodyDebugColorFn = (bodyHandle: number) => number;
type GetBodyTypeFn = (bodyHandle: number) => number;
type BodySetTypeFn = (bodyHandle: number, type: number) => void;
type BodySetNameFn = (bodyHandle: number, name: number) => void;
type BodySetGravityScaleFn = (bodyHandle: number, scale: number) => void;
type BodySetSleepThresholdFn = (bodyHandle: number, threshold: number) => void;
type BodyEnableSleepFn = (bodyHandle: number, enableSleep: number) => void;
type BodySetBulletFn = (bodyHandle: number, flag: number) => void;
type BodyEnableContactRecyclingFn = (bodyHandle: number, flag: number) => void;
type BodyEnableHitEventsFn = (bodyHandle: number, flag: number) => void;
type BodySetMotionLocksFn = (bodyHandle: number, lockBodyX: number, lockBodyY: number, lockBodyRotationX: number, lockBodyRotationY: number, lockBodyRotationZ: number, lockLinearZ: number) => void;
type BodySetMassDataFn = (bodyHandle: number, mass: number, cx: number, cy: number, cz: number, inertia: number) => void;
type BodyGetMassDataFn = (bodyHandle: number, outMassData: number) => void;
type BodyApplyMassFromShapesFn = (bodyHandle: number) => void;
type BodySetTargetTransformFn = (bodyHandle: number, px: number, py: number, pz: number, qx: number, qy: number, qz: number, qw: number, timeStep: number, wake: number) => void;
type ApplyLinearImpulseFn = (bodyHandle: number, ix: number, iy: number, iz: number, px: number, py: number, pz: number, wake: number) => void;
type ApplyLinearImpulseToCenterFn = (bodyHandle: number, ix: number, iy: number, iz: number, wake: number) => void;
type MakeQuatFromAxisAngleFn = (axisX: number, axisY: number, axisZ: number, radians: number, outQuat: number) => void;
type RotateVectorFn = (qx: number, qy: number, qz: number, qs: number, vx: number, vy: number, vz: number, outVec: number) => void;
type RandomVec3Fn = (lox: number, loy: number, loz: number, hix: number, hiy: number, hiz: number, outVec: number) => void;
type LerpVec3Fn = (ax: number, ay: number, az: number, bx: number, by: number, bz: number, alpha: number, outVec: number) => void;
type GetLengthAndNormalizeFn = (vx: number, vy: number, vz: number, outDirection: number) => number;
type ShapeSetDensityFn = (shapeHandle: number, density: number, updateBodyMass: number) => void;
type WorldEnableBoolFn = (worldHandle: number, flag: number) => void;
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
type RayCastClosestFn = (worldHandle: number, ox: number, oy: number, oz: number, tx: number, ty: number, tz: number, categoryBits: number, maskBits: number, outShapeHandle: number, outPoint: number, outNormal: number, outFraction: number) => void;
type BodyEnableFn = (bodyHandle: number) => void;
type BodyIsEnabledFn = (bodyHandle: number) => number;
type GetBodyMassFn = (bodyHandle: number) => number;
type GetBodyLocalRotationalInertiaFn = (bodyHandle: number, outInertia: number) => void;
type GetBodyWorldCenterFn = (bodyHandle: number, outPoint: number) => void;
type GetBodyWorldPointFn = (bodyHandle: number, lx: number, ly: number, lz: number, outPoint: number) => void;
type GetBodyLocalPointVelocityFn = (bodyHandle: number, lx: number, ly: number, lz: number, outVelocity: number) => void;
type GetBodyWorldPointVelocityFn = (bodyHandle: number, wx: number, wy: number, wz: number, outVelocity: number) => void;
type CreatePrismaticJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, constraintHertz: number, constraintDampingRatio: number, enableSpring: number, hertz: number, dampingRatio: number, targetTranslation: number, enableLimit: number, lowerTranslation: number, upperTranslation: number, enableMotor: number, maxMotorForce: number, motorSpeed: number, forceThreshold: number, torqueThreshold: number, collideConnected: number) => number;
type CreateWeldJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, linearHertz: number, angularHertz: number, linearDampingRatio: number, angularDampingRatio: number, forceThreshold: number, torqueThreshold: number, collideConnected: number) => number;
type CreateDistanceJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, length: number, forceThreshold: number, torqueThreshold: number, collideConnected: number) => number;
type GetStallThresholdFn = () => number;
type SetStallThresholdFn = (seconds: number) => void;
type WorldExplodeFn = (worldHandle: number, px: number, py: number, pz: number, radius: number, falloff: number, impulsePerArea: number, maskBits: number) => void;
type GetJointVec3Fn = (jointHandle: number, outVec3: number) => void;
type GetJointLinearSeparationFn = (jointHandle: number) => number;
type RevoluteJointSetTargetAngleFn = (jointHandle: number, targetRadians: number) => void;
type GetShapeBodyHandleFn = (shapeHandle: number) => number;
type ShapeSetFrictionFn = (shapeHandle: number, friction: number) => void;
type ShapeSetRestitutionFn = (shapeHandle: number, restitution: number) => void;

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
    bodies: heap32[base + 1]!,
    joints: heap32[base + 2]!,
    hulls: heap32[base + 3]!,
    shapes: heap32[base + 4]!,
    meshes: heap32[base + 5]!,
    compounds: heap32[base + 6]!,
    humans: heap32[base + 7]!,
  };
}

function asBodyHandle(handle: number): BodyHandle { return handle as BodyHandle; }
function asShapeId(handle: number): ShapeId { return handle as ShapeId; }

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

  protected wrapNumber<T extends (...args: number[]) => number>(name: string, argTypes: readonly string[]): T {
    return this.module.cwrap(name, "number", argTypes) as T;
  }

  protected wrapVoid<T extends (...args: number[]) => void>(name: string, argTypes: readonly string[]): T {
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
  private readonly createBodyFn = this.wrapNumber<CreateBodyFn>("b3wCreateBody", ["number", "number", "number", "number", "number", "number", "number"]);
  private readonly destroyWorldFn = this.wrapVoid<DestroyWorldFn>("b3wDestroyWorld", ["number"]);
  private readonly createBoxFn = this.wrapNumber<CreateBoxFn>("b3wCreateBox", ["number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly createSphereFn = this.wrapNumber<CreateSphereFn>("b3wCreateSphere", ["number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly createHullShapeFn = this.wrapNumber<CreateHullShapeFn>("b3wCreateHullShape", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createTransformedHullShapeFn = this.wrapNumber<CreateTransformedHullShapeFn>("b3wCreateTransformedHullShape", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createSphereShapeFn = this.wrapNumber<CreateSphereShapeFn>("b3wCreateSphereShape", ["number","number","number","number","number","number","number","number","number","number"]);
  private readonly createCapsuleShapeFn = this.wrapNumber<CreateCapsuleShapeFn>("b3wCreateCapsuleShape", ["number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createShapeFromHullFn = this.wrapNumber<CreateShapeFromHullFn>("b3wCreateShapeFromHull", ["number","number","number","number","number","number","number","number"]);
  private readonly createTransformedShapeFromHullFn = this.wrapNumber<CreateTransformedShapeFromHullFn>("b3wCreateTransformedShapeFromHull", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createCylinderFn = this.wrapNumber<CreateCylinderFn>("b3wCreateCylinder", ["number","number","number","number"]);
  private readonly createGridMeshFn = this.wrapNumber<CreateGridMeshFn>("b3wCreateGridMesh", ["number","number","number","number","number","number"]);
  private readonly createTorusMeshFn = this.wrapNumber<CreateTorusMeshFn>("b3wCreateTorusMesh", ["number","number","number","number","number"]);
  private readonly destroyMeshFn = this.wrapVoid<DestroyMeshFn>("b3wDestroyMesh", ["number"]);
  private readonly createMeshShapeFn = this.wrapNumber<CreateMeshShapeFn>("b3wCreateMeshShape", ["number","number","number","number","number","number","number","number","number"]);
  private readonly shapeSetMeshFn = this.wrapVoid<ShapeSetMeshFn>("b3wShapeSetMesh", ["number","number","number","number","number"]);
  private readonly createHullFromPointsFn = this.wrapNumber<CreateHullFromPointsFn>("b3wCreateHullFromPoints", ["number","number"]);
  private readonly createRockFn = this.wrapNumber<CreateRockFn>("b3wCreateRock", ["number"]);
  private readonly destroyHullFn = this.wrapVoid<DestroyHullFn>("b3wDestroyHull", ["number"]);
  private readonly createCompoundFn = this.wrapNumber<CreateCompoundFn>("b3wCreateCompound", ["number","number","number","number","number","number","number","number"]);
  private readonly createCompoundFromHullsFn = this.wrapNumber<CreateCompoundFromHullsFn>("b3wCreateCompoundFromHulls", ["number","number","number"]);
  private readonly createCompoundFromSpheresFn = this.wrapNumber<CreateCompoundFromSpheresFn>("b3wCreateCompoundFromSpheres", ["number","number","number"]);
  private readonly destroyCompoundFn = this.wrapVoid<DestroyCompoundFn>("b3wDestroyCompound", ["number"]);
  private readonly getCompoundTreeHeightFn = this.wrapNumber<GetCompoundTreeHeightFn>("b3wGetCompoundTreeHeight", ["number"]);
  private readonly createCompoundShapeFn = this.wrapNumber<CreateCompoundShapeFn>("b3wCreateCompoundShape", ["number","number","number"]);
  private readonly destroyBodyFn = this.wrapVoid<DestroyBodyFn>("b3wDestroyBody", ["number"]);
  private readonly destroyJointFn = this.wrapVoid<DestroyJointFn>("b3wDestroyJoint", ["number"]);
  private readonly setBodyTransformFn = this.wrapVoid<SetBodyTransformFn>("b3wSetBodyTransform", ["number","number","number","number","number","number","number","number"]);
  private readonly setBodyLinearVelocityFn = this.wrapVoid<SetBodyLinearVelocityFn>("b3wSetBodyLinearVelocity", ["number","number","number","number"]);
  private readonly setBodyAngularVelocityFn = this.wrapVoid<SetBodyAngularVelocityFn>("b3wSetBodyAngularVelocity", ["number","number","number","number"]);
  private readonly getBodyLinearVelocityFn = this.wrapVoid<GetBodyVelocityFn>("b3wGetBodyLinearVelocity", ["number", "number"]);
  private readonly getBodyAngularVelocityFn = this.wrapVoid<GetBodyVelocityFn>("b3wGetBodyAngularVelocity", ["number", "number"]);
  private readonly setBodyAwakeFn = this.wrapVoid<SetBodyAwakeFn>("b3wSetBodyAwake", ["number","number"]);
  private readonly setBodyDampingFn = this.wrapVoid<SetBodyDampingFn>("b3wSetBodyDamping", ["number","number","number"]);
  private readonly getBodyLocalPointFn = this.wrapVoid<GetBodyLocalPointFn>("b3wGetBodyLocalPoint", ["number","number","number","number","number"]);
  private readonly createMotorJointFn = this.wrapNumber<CreateMotorJointFn>("b3wCreateMotorJoint", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createFilterJointFn = this.wrapNumber<CreateFilterJointFn>("b3wCreateFilterJoint", ["number","number","number"]);
  private readonly createRevoluteJointFn = this.wrapNumber<CreateRevoluteJointFn>("b3wCreateRevoluteJoint", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createSphericalJointFn = this.wrapNumber<CreateSphericalJointFn>("b3wCreateSphericalJoint", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createHumanFn = this.wrapNumber<CreateHumanFn>("b3wCreateHuman", ["number","number","number","number","number","number","number","number","number"]);
  private readonly getHumanBoneBodyFn = this.wrapNumber<GetHumanBoneBodyFn>("b3wGetHumanBoneBody", ["number","number"]);
  private readonly getHumanBoneCountFn = this.wrapNumber<GetHumanBoneCountFn>("b3wGetHumanBoneCount", []);
  private readonly humanSetVelocityFn = this.wrapVoid<HumanSetVelocityFn>("b3wHumanSetVelocity", ["number","number","number","number"]);
  private readonly humanSetBulletFn = this.wrapVoid<HumanSetBulletFn>("b3wHumanSetBullet", ["number","number"]);
  private readonly humanSetJointFrictionTorqueFn = this.wrapVoid<HumanSetJointFloatFn>("b3wHumanSetJointFrictionTorque", ["number","number"]);
  private readonly humanSetJointSpringHertzFn = this.wrapVoid<HumanSetJointFloatFn>("b3wHumanSetJointSpringHertz", ["number","number"]);
  private readonly humanSetJointDampingRatioFn = this.wrapVoid<HumanSetJointFloatFn>("b3wHumanSetJointDampingRatio", ["number","number"]);
  private readonly enableWorldSleepFn = this.wrapVoid<WorldEnableBoolFn>("b3wEnableSleeping", ["number", "number"]);
  private readonly enableWorldContinuousFn = this.wrapVoid<WorldEnableBoolFn>("b3wEnableContinuous", ["number", "number"]);
  private readonly enableWorldWarmStartingFn = this.wrapVoid<WorldEnableBoolFn>("b3wEnableWarmStarting", ["number", "number"]);
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
  private readonly rayCastClosestFn = this.wrapVoid<RayCastClosestFn>("b3wRayCastClosest", ["number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly bodyEnableFn = this.wrapVoid<BodyEnableFn>("b3wBodyEnable", ["number"]);
  private readonly bodyDisableFn = this.wrapVoid<BodyEnableFn>("b3wBodyDisable", ["number"]);
  private readonly bodyIsEnabledFn = this.wrapNumber<BodyIsEnabledFn>("b3wBodyIsEnabled", ["number"]);
  private readonly getBodyMassFn = this.wrapNumber<GetBodyMassFn>("b3wGetBodyMass", ["number"]);
  private readonly getBodyLocalRotationalInertiaFn = this.wrapVoid<GetBodyLocalRotationalInertiaFn>("b3wGetBodyLocalRotationalInertia", ["number", "number"]);
  private readonly getBodyWorldCenterFn = this.wrapVoid<GetBodyWorldCenterFn>("b3wGetBodyWorldCenter", ["number", "number"]);
  private readonly getBodyWorldPointFn = this.wrapVoid<GetBodyWorldPointFn>("b3wGetBodyWorldPoint", ["number", "number", "number", "number", "number"]);
  private readonly getBodyLocalPointVelocityFn = this.wrapVoid<GetBodyLocalPointVelocityFn>("b3wGetBodyLocalPointVelocity", ["number", "number", "number", "number", "number"]);
  private readonly getBodyWorldPointVelocityFn = this.wrapVoid<GetBodyWorldPointVelocityFn>("b3wGetBodyWorldPointVelocity", ["number", "number", "number", "number", "number"]);
  private readonly createPrismaticJointFn = this.wrapNumber<CreatePrismaticJointFn>("b3wCreatePrismaticJoint", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createWeldJointFn = this.wrapNumber<CreateWeldJointFn>("b3wCreateWeldJoint", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly createDistanceJointFn = this.wrapNumber<CreateDistanceJointFn>("b3wCreateDistanceJoint", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
  private readonly getStallThresholdFn = this.wrapNumber<GetStallThresholdFn>("b3wGetStallThreshold", []);
  private readonly setStallThresholdFn = this.wrapVoid<SetStallThresholdFn>("b3wSetStallThreshold", ["number"]);
  private readonly worldExplodeFn = this.wrapVoid<WorldExplodeFn>("b3wWorldExplode", ["number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly getJointConstraintForceFn = this.wrapVoid<GetJointVec3Fn>("b3wGetJointConstraintForce", ["number", "number"]);
  private readonly getJointConstraintTorqueFn = this.wrapVoid<GetJointVec3Fn>("b3wGetJointConstraintTorque", ["number", "number"]);
  private readonly getJointLinearSeparationFn = this.wrapNumber<GetJointLinearSeparationFn>("b3wGetJointLinearSeparation", ["number"]);
  private readonly revoluteJointSetTargetAngleFn = this.wrapVoid<RevoluteJointSetTargetAngleFn>("b3wRevoluteJointSetTargetAngle", ["number", "number"]);
  private readonly getShapeBodyHandleFn = this.wrapNumber<GetShapeBodyHandleFn>("b3wGetShapeBodyHandle", ["number"]);
  private readonly stepFn = this.wrapVoid<StepFn>("b3wStep", ["number", "number", "number"]);
  private readonly getBodyTransformFn = this.wrapVoid<GetBodyTransformFn>("b3wGetBodyTransform", ["number", "number"]);
  private readonly setDensityFn = this.wrapVoid<ShapeSetDensityFn>("b3wShapeSetDensity", ["number", "number", "number"]);
  private readonly setFrictionFn = this.wrapVoid<ShapeSetFrictionFn>("b3wShapeSetFriction", ["number", "number"]);
  private readonly setRestitutionFn = this.wrapVoid<ShapeSetRestitutionFn>("b3wShapeSetRestitution", ["number", "number"]);
  private readonly setSurfaceMaterialFn = this.wrapVoid<ShapeSetSurfaceMaterialFn>("b3wShapeSetSurfaceMaterial", ["number", "number", "number", "number", "number", "number", "number"]);
  private readonly setFilterFn = this.wrapVoid<ShapeSetFilterFn>("b3wShapeSetFilter", ["number", "number", "number", "number", "number"]);
  private readonly getBodyShapeCountFn = this.wrapNumber<GetBodyShapeCountFn>("b3wGetBodyShapeCount", ["number"]);
  private readonly getBodyShapesFn = this.wrapNumber<GetBodyShapesFn>("b3wGetBodyShapes", ["number", "number", "number"]);
  private readonly destroyShapeFn = this.wrapVoid<DestroyShapeFn>("b3wDestroyShape", ["number", "number"]);
  private readonly enableShapeSensorEventsFn = this.wrapVoid<ShapeEnableBoolFn>("b3wShapeEnableSensorEvents", ["number", "number"]);
  private readonly enableShapeContactEventsFn = this.wrapVoid<ShapeEnableBoolFn>("b3wShapeEnableContactEvents", ["number", "number"]);
  private readonly enableShapePreSolveEventsFn = this.wrapVoid<ShapeEnableBoolFn>("b3wShapeEnablePreSolveEvents", ["number", "number"]);
  private readonly enableShapeHitEventsFn = this.wrapVoid<ShapeEnableBoolFn>("b3wShapeEnableHitEvents", ["number", "number"]);
  private readonly setShapeSphereFn = this.wrapVoid<ShapeSetSphereFn>("b3wShapeSetSphere", ["number", "number", "number", "number", "number"]);
  private readonly setShapeCapsuleFn = this.wrapVoid<ShapeSetCapsuleFn>("b3wShapeSetCapsule", ["number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly applyShapeWindFn = this.wrapVoid<ShapeApplyWindFn>("b3wShapeApplyWind", ["number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly bodyIsAwakeFn = this.wrapNumber<BodyIsAwakeFn>("b3wBodyIsAwake", ["number"]);
  private readonly getBodyDebugColorFn = this.wrapNumber<GetBodyDebugColorFn>("b3wGetBodyDebugColor", ["number"]);
  private readonly getBodyTypeFn = this.wrapNumber<GetBodyTypeFn>("b3wGetBodyType", ["number"]);
  private readonly setBodyTypeFn = this.wrapVoid<BodySetTypeFn>("b3wSetBodyType", ["number", "number"]);
  private readonly setBodyNameFn = this.wrapVoid<BodySetNameFn>("b3wSetBodyName", ["number", "number"]);
  private readonly setBodyGravityScaleFn = this.wrapVoid<BodySetGravityScaleFn>("b3wSetBodyGravityScale", ["number", "number"]);
  private readonly setBodySleepThresholdFn = this.wrapVoid<BodySetSleepThresholdFn>("b3wSetBodySleepThreshold", ["number", "number"]);
  private readonly enableBodySleepFn = this.wrapVoid<BodyEnableSleepFn>("b3wEnableBodySleep", ["number", "number"]);
  private readonly setBodyBulletFn = this.wrapVoid<BodySetBulletFn>("b3wSetBodyBullet", ["number", "number"]);
  private readonly enableBodyContactRecyclingFn = this.wrapVoid<BodyEnableContactRecyclingFn>("b3wEnableBodyContactRecycling", ["number", "number"]);
  private readonly enableBodyHitEventsFn = this.wrapVoid<BodyEnableHitEventsFn>("b3wEnableBodyHitEvents", ["number", "number"]);
  private readonly setBodyMotionLocksFn = this.wrapVoid<BodySetMotionLocksFn>("b3wSetBodyMotionLocks", ["number", "number", "number", "number", "number", "number", "number"]);
  private readonly setBodyMassDataFn = this.wrapVoid<BodySetMassDataFn>("b3wSetBodyMassData", ["number", "number", "number", "number", "number", "number"]);
  private readonly getBodyMassDataFn = this.wrapVoid<BodyGetMassDataFn>("b3wGetBodyMassData", ["number", "number"]);
  private readonly applyBodyMassFromShapesFn = this.wrapVoid<BodyApplyMassFromShapesFn>("b3wApplyBodyMassFromShapes", ["number"]);
  private readonly setBodyTargetTransformFn = this.wrapVoid<BodySetTargetTransformFn>("b3wSetBodyTargetTransform", ["number", "number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly applyLinearImpulseFn = this.wrapVoid<ApplyLinearImpulseFn>("b3wApplyLinearImpulse", ["number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly applyLinearImpulseToCenterFn = this.wrapVoid<ApplyLinearImpulseToCenterFn>("b3wApplyLinearImpulseToCenter", ["number", "number", "number", "number", "number"]);
  private readonly b3wSinFn = this.wrapNumber<(radians: number) => number>("b3wSin", ["number"]);
  private readonly b3wCosFn = this.wrapNumber<(radians: number) => number>("b3wCos", ["number"]);
  private readonly b3wCosfFn = this.wrapNumber<(radians: number) => number>("b3wCosf", ["number"]);
  private readonly b3wSinfFn = this.wrapNumber<(radians: number) => number>("b3wSinf", ["number"]);
  private readonly makeQuatFromAxisAngleFn = this.wrapVoid<MakeQuatFromAxisAngleFn>("b3wMakeQuatFromAxisAngle", ["number", "number", "number", "number", "number"]);
  private readonly rotateVectorFn = this.wrapVoid<RotateVectorFn>("b3wRotateVector", ["number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly randomVec3Fn = this.wrapVoid<RandomVec3Fn>("b3wRandomVec3", ["number", "number", "number", "number", "number", "number", "number"]);
  private readonly lerpVec3Fn = this.wrapVoid<LerpVec3Fn>("b3wLerpVec3", ["number", "number", "number", "number", "number", "number", "number", "number"]);
  private readonly getLengthAndNormalizeFn = this.wrapNumber<GetLengthAndNormalizeFn>("b3wGetLengthAndNormalize", ["number", "number", "number", "number"]);
  private readonly transformPtr: number;
  private readonly pointPtr: number;
  private readonly massDataPtr: number;
  private readonly profilePtr: number;
  private readonly inertiaPtr: number;
  private readonly slotCountsPtr: number;
  readonly limits: SlotLimits;
  private readonly bodyBatchBuffers = new Map<number, BodyBatchBuffers>();

  constructor(module: CModule) {
    super(module);
    this.slotCountsPtr = module._malloc(8 * 4);
    this.getSlotLimitsFn(this.slotCountsPtr);
    this.limits = readSlotCounts(this.slotCountsPtr, module.HEAP32);
    this.transformPtr = module._malloc(7 * 4);
    this.pointPtr = module._malloc(3 * 4);
    this.massDataPtr = module._malloc(2 * 4);
    this.profilePtr = module._malloc(23 * 4);
    this.inertiaPtr = module._malloc(9 * 4);
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
  }

  allocBodyBatchBuffers(capacity: number): BodyBatchBuffers {
    const cached = this.bodyBatchBuffers.get(capacity);
    if (cached !== undefined) return cached;
    const buffers = {
      bodyHandlesPtr: this.module._malloc(capacity * 4),
      positionsPtr: this.module._malloc(capacity * 3 * 4),
      rotationsPtr: this.module._malloc(capacity * 4 * 4),
      awakePtr: this.module._malloc(capacity),
      colorsPtr: this.module._malloc(capacity * 4),
      capacity,
    };
    this.bodyBatchBuffers.set(capacity, buffers);
    return buffers;
  }

  freeBodyBatchBuffers(buffers: BodyBatchBuffers): void {
    if (this.bodyBatchBuffers.get(buffers.capacity) !== buffers) return;
    this.module._free(buffers.bodyHandlesPtr);
    this.module._free(buffers.positionsPtr);
    this.module._free(buffers.rotationsPtr);
    this.module._free(buffers.awakePtr);
    this.module._free(buffers.colorsPtr);
    this.bodyBatchBuffers.delete(buffers.capacity);
  }

  private readPointInto(out: Vec3): Vec3 {
    const heap = this.module.HEAPF32;
    const base = this.pointPtr >> 2;
    return writeVec3(out, heap[base + 0], heap[base + 1], heap[base + 2]);
  }

  getMemoryView(): RuntimeMemoryView32 {
    return { heapF32: this.module.HEAPF32, heapU8: this.module.HEAPU8, heap32: this.module.HEAP32 };
  }

  writeBodyHandles(buffers: BodyBatchBuffers, bodyHandles: readonly number[]): void {
    const view = new Int32Array(this.module.HEAP32.buffer, buffers.bodyHandlesPtr, bodyHandles.length);
    for (let i = 0; i < bodyHandles.length; i++) view[i] = bodyHandles[i];
  }

  private applyBodyDef(bodyHandle: BodyHandle, def: BodyDef): void {
    if (def.rotation) this.setBodyTransform(bodyHandle, def.position ?? vec3(), def.rotation);
    if (def.linearVelocity) this.setBodyLinearVelocity(bodyHandle, def.linearVelocity);
    if (def.angularVelocity) this.setBodyAngularVelocity(bodyHandle, def.angularVelocity);
    if (def.linearDamping !== undefined || def.angularDamping !== undefined) this.setBodyDamping(bodyHandle, def.linearDamping ?? 0, def.angularDamping ?? 0);
    if (def.gravityScale !== undefined) this.setBodyGravityScale(bodyHandle, def.gravityScale);
    if (def.sleepThreshold !== undefined) this.setBodySleepThreshold(bodyHandle, def.sleepThreshold);
    if (def.isBullet !== undefined) this.setBodyBullet(bodyHandle, def.isBullet);
    if (def.isEnabled !== undefined && !def.isEnabled) this.setBodyType(bodyHandle, BodyType.Static); // approximate
    if (def.allowFastRotation !== undefined) {} // not exposed in bindings
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
    if (def.isSensor) { this.enableShapeSensorEvents(shapeHandle, true); /* isSensor not directly settable after creation */ }
    if (def.enableSensorEvents !== undefined) this.enableShapeSensorEvents(shapeHandle, def.enableSensorEvents);
    if (def.enableContactEvents !== undefined) this.enableShapeContactEvents(shapeHandle, def.enableContactEvents);
    if (def.enableHitEvents !== undefined) this.enableShapeHitEvents(shapeHandle, def.enableHitEvents);
    if (def.enablePreSolveEvents !== undefined) this.enableShapePreSolveEvents(shapeHandle, def.enablePreSolveEvents);
    if (def.categoryBits !== undefined || def.maskBits !== undefined || def.groupIndex !== undefined || def.invokeContactCreation !== undefined) {
      this.setShapeFilter(shapeHandle, def.categoryBits ?? U64_MAX, def.maskBits ?? U64_MAX, def.groupIndex ?? 0, def.invokeContactCreation ?? false);
    }
  }

  createBody(worldHandle: WorldHandle, def: BodyDef = {}): BodyHandle {
    const p = def.position ?? vec3();
    const bodyHandle = this.createBodyFn(worldHandle, def.type ?? BodyType.Static, p[0], p[1], p[2], defaults(def.enableSleep, true) ? 1 : 0, defaults(def.isAwake, true) ? 1 : 0);
    const handle = this.requireSlotHandle<BodyHandle>(bodyHandle, "bodies");
    this.applyBodyDef(handle, def);
    return handle;
  }

  createBox(worldHandle: WorldHandle, options: BoxOptions): BodyHandle {
    const s = options.size;
    const p = options.position ?? vec3();
    const bodyHandle = this.createBoxFn(worldHandle, p[0], p[1], p[2], s[0], s[1], s[2], options.static ? 1 : 0, options.density ?? 1);
    const handle = this.requireSlotHandle<BodyHandle>(bodyHandle, "bodies");
    if (options.friction !== undefined || options.restitution !== undefined || options.rollingResistance !== undefined || options.isSensor || options.enableContactEvents || options.enableHitEvents) {
      const shape = { bodyHandle: handle, shapeHandle: asShapeId(bodyHandle) } as ShapeHandle;
      if (options.friction !== undefined || options.restitution !== undefined || options.rollingResistance !== undefined)
        this.setShapeSurfaceMaterial(shape, { friction: options.friction, restitution: options.restitution, rollingResistance: options.rollingResistance });
    }
    return handle;
  }

  createBoxWithShape(worldHandle: WorldHandle, options: BoxOptions): ShapeHandle {
    const bodyHandle = this.createBox(worldHandle, options);
    const shapeHandle = this.getBodyShapes(bodyHandle)[0];
    if (shapeHandle === undefined) throw new Error("createBox did not produce a shape handle");
    return { bodyHandle, shapeHandle };
  }

  createSphere(worldHandle: WorldHandle, options: SphereOptions): BodyHandle {
    const p = options.position ?? vec3();
    const v = options.velocity ?? vec3();
    const bodyHandle = this.createSphereFn(worldHandle, p[0], p[1], p[2], options.radius, v[0], v[1], v[2], options.density ?? 1);
    const handle = this.requireSlotHandle<BodyHandle>(bodyHandle, "bodies");
    if (options.isBullet) this.setBodyBullet(handle, true);
    if (options.friction !== undefined || options.restitution !== undefined || options.rollingResistance !== undefined)
      this.setShapeSurfaceMaterial(asShapeId(bodyHandle), { friction: options.friction, restitution: options.restitution, rollingResistance: options.rollingResistance });
    return handle;
  }

  createSphereWithShape(worldHandle: WorldHandle, options: SphereOptions): ShapeHandle {
    const bodyHandle = this.createSphere(worldHandle, options);
    const shapeHandle = this.getBodyShapes(bodyHandle)[0];
    if (shapeHandle === undefined) throw new Error("createSphere did not produce a shape handle");
    return { bodyHandle, shapeHandle };
  }

  createSphereShape(bodyHandle: BodyHandle, center: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = this.requireSlotHandle<number>(this.createSphereShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, center[0], center[1], center[2], radius, def.invokeContactCreation === false ? 0 : 1), "shapes");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }

  createCapsuleShape(bodyHandle: BodyHandle, center1: Vec3, center2: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = this.requireSlotHandle<number>(this.createCapsuleShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, center1[0], center1[1], center1[2], center2[0], center2[1], center2[2], radius), "shapes");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }

  createHullShape(bodyHandle: BodyHandle, halfWidths: Vec3, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = this.requireSlotHandle<number>(this.createHullShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, def.updateBodyMass === false ? 0 : 1, 0, 0, 0, 0, 0, 0, 1, halfWidths[0], halfWidths[1], halfWidths[2]), "shapes");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }

  createTransformedHullShape(bodyHandle: BodyHandle, halfWidths: Vec3, transform: { position?: Vec3; rotation?: Quat } = {}, scale: Vec3 = [1,1,1], def: ShapeDef = {}): ShapeHandle {
    const pos = transform.position ?? vec3();
    const rot = transform.rotation ?? [0,0,0,1];
    const shapeHandle = this.requireSlotHandle<number>(this.createTransformedHullShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, pos[0], pos[1], pos[2], rot[0], rot[1], rot[2], rot[3], halfWidths[0], halfWidths[1], halfWidths[2], scale[0], scale[1], scale[2]), "shapes");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }

  createShapeFromHull(bodyHandle: BodyHandle, hullHandle: HullHandle, def: ShapeDef = {}): ShapeId {
    const shapeHandle = this.requireSlotHandle<number>(this.createShapeFromHullFn(bodyHandle, hullHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, def.updateBodyMass === false ? 0 : 1, def.explosionScale ?? 1), "shapes");
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return asShapeId(shapeHandle);
  }

  createTransformedShapeFromHull(bodyHandle: BodyHandle, hullHandle: HullHandle, transform: { position?: Vec3; rotation?: Quat } = {}, scale: Vec3 = [1, 1, 1], def: ShapeDef = {}): ShapeId {
    const pos = transform.position ?? vec3();
    const rot = transform.rotation ?? [0, 0, 0, 1];
    const shapeHandle = this.requireSlotHandle<number>(this.createTransformedShapeFromHullFn(bodyHandle, hullHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, def.updateBodyMass === false ? 0 : 1, pos[0], pos[1], pos[2], rot[0], rot[1], rot[2], rot[3], scale[0], scale[1], scale[2]), "shapes");
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return asShapeId(shapeHandle);
  }

  createCylinder(height: number, radius: number, yOffset = 0, sides = 12): HullHandle {
    return this.requireSlotHandle<HullHandle>(this.createCylinderFn(height, radius, yOffset, sides), "hulls");
  }
  createGridMesh(worldHandle: WorldHandle, xCount: number, zCount: number, cellWidth: number, materialCount = 1, identifyEdges = true): MeshHandle {
    return this.requireSlotHandle<MeshHandle>(this.createGridMeshFn(worldHandle, xCount, zCount, cellWidth, materialCount, identifyEdges ? 1 : 0), "meshes");
  }
  createTorusMesh(worldHandle: WorldHandle, radialResolution: number, tubularResolution: number, radius: number, thickness: number): MeshHandle {
    return this.requireSlotHandle<MeshHandle>(this.createTorusMeshFn(worldHandle, radialResolution, tubularResolution, radius, thickness), "meshes");
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
  destroyCompound(compoundHandle: CompoundHandle): void { this.destroyCompoundFn(compoundHandle); }
  getCompoundTreeHeight(compoundHandle: CompoundHandle): number { return this.getCompoundTreeHeightFn(compoundHandle); }
  createCompoundShape(bodyHandle: BodyHandle, compoundHandle: CompoundHandle, density = 1): ShapeId {
    return asShapeId(this.requireSlotHandle<number>(this.createCompoundShapeFn(bodyHandle, compoundHandle, density), "shapes"));
  }
  createMeshShape(bodyHandle: BodyHandle, meshHandle: MeshHandle, def: MeshShapeOptions = {}): ShapeHandle {
    const scale = def.scale ?? [1, 1, 1];
    const shapeHandle = this.requireSlotHandle<number>(this.createMeshShapeFn(bodyHandle, meshHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, scale[0], scale[1], scale[2]), "shapes");
    const shape = { bodyHandle, shapeHandle: asShapeId(shapeHandle) };
    this.applyShapeDef(asShapeId(shapeHandle), def);
    return shape;
  }
  setMesh(shapeHandle: ShapeId | ShapeHandle, meshHandle: MeshHandle, scale: Vec3 = [1, 1, 1]): void {
    const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle;
    this.shapeSetMeshFn(handle, meshHandle, scale[0], scale[1], scale[2]);
  }
  getBodyShapes(bodyHandle: BodyHandle): ShapeId[] {
    const count = this.getBodyShapeCountFn(bodyHandle);
    if (count <= 0) return [];
    const ptr = this.module._malloc(count * 4);
    const written = this.getBodyShapesFn(bodyHandle, ptr, count);
    const heap32 = this.module.HEAP32;
    const base = ptr >> 2;
    const handles: ShapeId[] = [];
    for (let i = 0; i < written; i++) handles.push(asShapeId(heap32[base + i]));
    this.module._free(ptr);
    return handles;
  }
  destroyBody(bodyHandle: BodyHandle): void { this.destroyBodyFn(bodyHandle); }
  destroyShape(shapeHandle: ShapeId | ShapeHandle, updateBodyMass = true): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.destroyShapeFn(handle, updateBodyMass ? 1 : 0); }
  destroyJoint(jointHandle: JointHandle): void { this.destroyJointFn(jointHandle); }
  setBodyTransform(bodyHandle: BodyHandle, position: Vec3, rotation: Quat = [0,0,0,1]): void { this.setBodyTransformFn(bodyHandle, position[0], position[1], position[2], rotation[0], rotation[1], rotation[2], rotation[3]); }
  setBodyLinearVelocity(bodyHandle: BodyHandle, velocity: Vec3): void { this.setBodyLinearVelocityFn(bodyHandle, velocity[0], velocity[1], velocity[2]); }
  setBodyAngularVelocity(bodyHandle: BodyHandle, velocity: Vec3): void { this.setBodyAngularVelocityFn(bodyHandle, velocity[0], velocity[1], velocity[2]); }
  getBodyLinearVelocity(bodyHandle: BodyHandle): Vec3 { return this.getBodyLinearVelocityTo(bodyHandle, [0, 0, 0]); }
  getBodyLinearVelocityTo(bodyHandle: BodyHandle, out: Vec3): Vec3 { this.getBodyLinearVelocityFn(bodyHandle, this.pointPtr); return this.readPointInto(out); }
  getBodyAngularVelocity(bodyHandle: BodyHandle): Vec3 { return this.getBodyAngularVelocityTo(bodyHandle, [0, 0, 0]); }
  getBodyAngularVelocityTo(bodyHandle: BodyHandle, out: Vec3): Vec3 { this.getBodyAngularVelocityFn(bodyHandle, this.pointPtr); return this.readPointInto(out); }
  bodyIsAwake(bodyHandle: BodyHandle): boolean { return this.bodyIsAwakeFn(bodyHandle) !== 0; }
  getBodyDebugColor(bodyHandle: BodyHandle): number { return this.getBodyDebugColorFn(bodyHandle); }
  getBodyType(bodyHandle: BodyHandle): BodyType { return this.getBodyTypeFn(bodyHandle) as BodyType; }
  setBodyAwake(bodyHandle: BodyHandle, awake: boolean): void { this.setBodyAwakeFn(bodyHandle, awake ? 1 : 0); }
  setBodyDamping(bodyHandle: BodyHandle, linearDamping: number, angularDamping: number): void { this.setBodyDampingFn(bodyHandle, linearDamping, angularDamping); }
  getBodyLocalPoint(bodyHandle: BodyHandle, worldPoint: Vec3): Vec3 { return this.getBodyLocalPointTo(bodyHandle, worldPoint, [0, 0, 0]); }
  getBodyLocalPointXYZ(bodyHandle: BodyHandle, worldX: number, worldY: number, worldZ: number): Vec3 { return this.getBodyLocalPointXYZTo(bodyHandle, worldX, worldY, worldZ, [0, 0, 0]); }
  getBodyLocalPointTo(bodyHandle: BodyHandle, worldPoint: Vec3, out: Vec3): Vec3 { return this.getBodyLocalPointXYZTo(bodyHandle, worldPoint[0], worldPoint[1], worldPoint[2], out); }
  getBodyLocalPointXYZTo(bodyHandle: BodyHandle, worldX: number, worldY: number, worldZ: number, out: Vec3): Vec3 { this.getBodyLocalPointFn(bodyHandle, worldX, worldY, worldZ, this.pointPtr); return this.readPointInto(out); }
  createMotorJoint(worldHandle: WorldHandle, bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: MotorJointOptions = {}): JointHandle { const a = options.localFrameA ?? vec3(); const b = options.localFrameB ?? vec3(); const lv = options.linearVelocity ?? vec3(); const av = options.angularVelocity ?? vec3(); return this.requireSlotHandle<JointHandle>(this.createMotorJointFn(worldHandle, bodyAHandle, bodyBHandle, a[0], a[1], a[2], b[0], b[1], b[2], lv[0], lv[1], lv[2], options.maxVelocityForce ?? 0, av[0], av[1], av[2], options.maxVelocityTorque ?? 0, options.collideConnected ? 1 : 0, options.linearHertz ?? 0, options.linearDampingRatio ?? 0, options.maxSpringForce ?? 0, options.angularHertz ?? 0, options.angularDampingRatio ?? 0, options.maxSpringTorque ?? 0), "joints"); }
  createFilterJoint(worldHandle: WorldHandle, bodyAHandle: BodyHandle, bodyBHandle: BodyHandle): JointHandle { return this.requireSlotHandle<JointHandle>(this.createFilterJointFn(worldHandle, bodyAHandle, bodyBHandle), "joints"); }
  createRevoluteJoint(worldHandle: WorldHandle, bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; constraintHertz?: number; constraintDampingRatio?: number; targetAngle?: number; enableSpring?: boolean; hertz?: number; dampingRatio?: number; enableLimit?: boolean; lowerAngle?: number; upperAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorSpeed?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointHandle { const a = options.localFrameA?.position ?? vec3(); const aq = options.localFrameA?.rotation ?? [0, 0, 0, 1]; const b = options.localFrameB?.position ?? vec3(); const bq = options.localFrameB?.rotation ?? [0, 0, 0, 1]; const [forceThreshold, torqueThreshold, collideConnected] = jointThresholdArgs(options); return this.requireSlotHandle<JointHandle>(this.createRevoluteJointFn(worldHandle, bodyAHandle, bodyBHandle, a[0], a[1], a[2], aq[0], aq[1], aq[2], aq[3], b[0], b[1], b[2], bq[0], bq[1], bq[2], bq[3], options.constraintHertz ?? 60, options.constraintDampingRatio ?? 2, options.targetAngle ?? 0, options.enableSpring ? 1 : 0, options.hertz ?? 0, options.dampingRatio ?? 0, options.enableLimit ? 1 : 0, options.lowerAngle ?? 0, options.upperAngle ?? 0, options.enableMotor ? 1 : 0, options.maxMotorTorque ?? 0, options.motorSpeed ?? 0, forceThreshold, torqueThreshold, collideConnected), "joints"); }
  createSphericalJoint(worldHandle: WorldHandle, bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetRotation?: Quat; enableConeLimit?: boolean; coneAngle?: number; enableTwistLimit?: boolean; lowerTwistAngle?: number; upperTwistAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorVelocity?: Vec3 } = {}): JointHandle { const a = options.localFrameA?.position ?? vec3(); const aq = options.localFrameA?.rotation ?? [0, 0, 0, 1]; const b = options.localFrameB?.position ?? vec3(); const bq = options.localFrameB?.rotation ?? [0, 0, 0, 1]; const tq = options.targetRotation ?? [0, 0, 0, 1]; const mv = options.motorVelocity ?? vec3(); return this.requireSlotHandle<JointHandle>(this.createSphericalJointFn(worldHandle, bodyAHandle, bodyBHandle, a[0], a[1], a[2], aq[0], aq[1], aq[2], aq[3], b[0], b[1], b[2], bq[0], bq[1], bq[2], bq[3], options.enableSpring ? 1 : 0, options.hertz ?? 0, options.dampingRatio ?? 0, tq[0], tq[1], tq[2], tq[3], options.enableConeLimit ? 1 : 0, options.coneAngle ?? 0, options.enableTwistLimit ? 1 : 0, options.lowerTwistAngle ?? 0, options.upperTwistAngle ?? 0, options.enableMotor ? 1 : 0, options.maxMotorTorque ?? 0, mv[0], mv[1], mv[2]), "joints"); }
  createHuman(worldHandle: WorldHandle, position: Vec3, options: { frictionTorque?: number; hertz?: number; dampingRatio?: number; groupIndex?: number; colorize?: boolean } = {}): HumanHandle {
    return this.requireSlotHandle<HumanHandle>(
      this.createHumanFn(worldHandle, position[0], position[1], position[2], options.frictionTorque ?? 1, options.hertz ?? 1, options.dampingRatio ?? 1, options.groupIndex ?? 0, options.colorize ?? true ? 1 : 0),
      "humans",
    );
  }
  getHumanBoneBody(humanHandle: HumanHandle, boneIndex: number): BodyHandle { return asBodyHandle(this.getHumanBoneBodyFn(humanHandle, boneIndex)); }
  getHumanBoneCount(): number { return this.getHumanBoneCountFn(); }
  setHumanVelocity(humanHandle: number, velocity: Vec3): void { this.humanSetVelocityFn(humanHandle, velocity[0], velocity[1], velocity[2]); }
  setHumanBullet(humanHandle: number, flag: boolean): void { this.humanSetBulletFn(humanHandle, flag ? 1 : 0); }
  setHumanJointFrictionTorque(humanHandle: number, torque: number): void { this.humanSetJointFrictionTorqueFn(humanHandle, torque); }
  setHumanJointSpringHertz(humanHandle: number, hertz: number): void { this.humanSetJointSpringHertzFn(humanHandle, hertz); }
  setHumanJointDampingRatio(humanHandle: number, dampingRatio: number): void { this.humanSetJointDampingRatioFn(humanHandle, dampingRatio); }
  readBodyTransform(bodyHandle: BodyHandle): BodyTransform { this.getBodyTransformFn(bodyHandle, this.transformPtr); const heap = this.module.HEAPF32; const base = this.transformPtr >> 2; return { position: [heap[base + 0], heap[base + 1], heap[base + 2]], rotation: [heap[base + 3], heap[base + 4], heap[base + 5], heap[base + 6]] }; }
  getWorldCounters(worldHandle: WorldHandle): WorldCounters { const ptr = this.module._malloc(7 * 4); this.getWorldCountersFn(worldHandle, ptr); const heap32 = new Int32Array(this.module.HEAPF32.buffer); const base = ptr >> 2; const counters = { bodyCount: heap32[base + 0], shapeCount: heap32[base + 1], contactCount: heap32[base + 2], jointCount: heap32[base + 3], islandCount: heap32[base + 4], staticTreeHeight: heap32[base + 5], treeHeight: heap32[base + 6] }; this.module._free(ptr); return counters; }
  getWorldAwakeBodyCount(worldHandle: WorldHandle): number { return this.getWorldAwakeBodyCountFn(worldHandle); }
  getWorldProfile(worldHandle: WorldHandle): WorldProfile { this.getWorldProfileFn(worldHandle, this.profilePtr); const heap = this.module.HEAPF32; const base = this.profilePtr >> 2; return { step: heap[base + 0], pairs: heap[base + 1], collide: heap[base + 2], solve: heap[base + 3], solverSetup: heap[base + 4], constraints: heap[base + 5], prepareConstraints: heap[base + 6], integrateVelocities: heap[base + 7], warmStart: heap[base + 8], solveImpulses: heap[base + 9], integratePositions: heap[base + 10], relaxImpulses: heap[base + 11], applyRestitution: heap[base + 12], storeImpulses: heap[base + 13], splitIslands: heap[base + 14], transforms: heap[base + 15], sensorHits: heap[base + 16], jointEvents: heap[base + 17], hitEvents: heap[base + 18], refit: heap[base + 19], bullets: heap[base + 20], sleepIslands: heap[base + 21], sensors: heap[base + 22] }; }
  checkThreadingSupport(): number { return this.checkThreadingSupportFn(); }
  getWorldWorkerCount(worldHandle: WorldHandle): number { return this.getWorldWorkerCountFn(worldHandle); }
  enableWorldSleeping(worldHandle: WorldHandle, flag: boolean): void { this.enableWorldSleepFn(worldHandle, flag ? 1 : 0); }
  enableWorldContinuous(worldHandle: WorldHandle, flag: boolean): void { this.enableWorldContinuousFn(worldHandle, flag ? 1 : 0); }
  enableWorldWarmStarting(worldHandle: WorldHandle, flag: boolean): void { this.enableWorldWarmStartingFn(worldHandle, flag ? 1 : 0); }
  setWorldContactTuning(worldHandle: WorldHandle, hertz: number, dampingRatio: number, contactSpeed: number): void { this.setWorldContactTuningFn(worldHandle, hertz, dampingRatio, contactSpeed); }
  setWorldContactRecycleDistance(worldHandle: WorldHandle, distance: number): void { this.setWorldContactRecycleDistanceFn(worldHandle, distance); }
  setWorldWorkerCount(worldHandle: WorldHandle, count: number): void { this.setWorldWorkerCountFn(worldHandle, count); }

  rayCastClosest(worldHandle: WorldHandle, origin: Vec3, translation: Vec3, categoryBits = U64_MAX, maskBits = U64_MAX): { shapeHandle: ShapeId; bodyHandle: BodyHandle; point: Vec3; normal: Vec3; fraction: number } | null {
    const outShapePtr = this.module._malloc(4);
    const outPointPtr = this.module._malloc(3 * 4);
    const outNormalPtr = this.module._malloc(3 * 4);
    const outFractionPtr = this.module._malloc(4);
    this.rayCastClosestFn(worldHandle, origin[0], origin[1], origin[2], translation[0], translation[1], translation[2], categoryBits, maskBits, outShapePtr, outPointPtr, outNormalPtr, outFractionPtr);
    const heap32 = new Int32Array(this.module.HEAPF32.buffer);
    const heap = this.module.HEAPF32;
    const sBase = outShapePtr >> 2;
    const pBase = outPointPtr >> 2;
    const nBase = outNormalPtr >> 2;
    const fBase = outFractionPtr >> 2;
    const shapeHandle = heap32[sBase + 0];
    const fraction = heap[fBase + 0];
    if (shapeHandle === 0 || fraction <= 0) {
      this.module._free(outShapePtr); this.module._free(outPointPtr); this.module._free(outNormalPtr); this.module._free(outFractionPtr);
      return null;
    }
    const bodyHandle = asBodyHandle(this.getShapeBodyHandleFn(shapeHandle));
    const result = { shapeHandle: asShapeId(shapeHandle), bodyHandle, point: [heap[pBase + 0], heap[pBase + 1], heap[pBase + 2]] as Vec3, normal: [heap[nBase + 0], heap[nBase + 1], heap[nBase + 2]] as Vec3, fraction };
    this.module._free(outShapePtr); this.module._free(outPointPtr); this.module._free(outNormalPtr); this.module._free(outFractionPtr);
    return result;
  }

  // Batched transform read: writes transforms + awake flags for all bodies at once.
  // Buffers must be pre-allocated with _malloc.
  writeBodyTransforms(count: number, bodyHandlesPtr: number, outPositionsPtr: number, outRotationsPtr: number, outAwakePtr: number, outColorsPtr: number): void {
    this.writeBodyTransformsFn(count, bodyHandlesPtr, outPositionsPtr, outRotationsPtr, outAwakePtr, outColorsPtr);
  }

  writeBodyTransformsLight(count: number, bodyHandlesPtr: number, outPositionsPtr: number, outRotationsPtr: number, outAwakePtr: number, outColorsPtr: number): void {
    this.writeBodyTransformsLightFn(count, bodyHandlesPtr, outPositionsPtr, outRotationsPtr, outAwakePtr, outColorsPtr);
  }

  step(worldHandle: WorldHandle, dt: number, substeps: number): void { this.stepFn(worldHandle, dt, substeps); }
  destroyWorld(worldHandle: WorldHandle): void { this.destroyWorldFn(worldHandle); }
  setShapeDensity(shapeHandle: ShapeId | ShapeHandle, density: number, updateBodyMass = true): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setDensityFn(handle, density, updateBodyMass ? 1 : 0); }
  setShapeFriction(shapeHandle: ShapeId | ShapeHandle, friction: number): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setFrictionFn(handle, friction); }
  setShapeRestitution(shapeHandle: ShapeId | ShapeHandle, restitution: number): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setRestitutionFn(handle, restitution); }
  setShapeSurfaceMaterial(shapeHandle: ShapeId | ShapeHandle, material: SurfaceMaterial = {}): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; const tv = material.tangentVelocity ?? [0,0,0]; this.setSurfaceMaterialFn(handle, material.friction ?? 0.6, material.restitution ?? 0, material.rollingResistance ?? 0, tv[0], tv[1], tv[2]); }
  setShapeFilter(shapeHandle: ShapeId | ShapeHandle, categoryBits: number, maskBits: number, groupIndex = 0, invokeContacts = false): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setFilterFn(handle, categoryBits, maskBits, groupIndex, invokeContacts ? 1 : 0); }
  enableShapeSensorEvents(shapeHandle: ShapeId | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapeSensorEventsFn(handle, flag ? 1 : 0); }
  enableShapeContactEvents(shapeHandle: ShapeId | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapeContactEventsFn(handle, flag ? 1 : 0); }
  enableShapePreSolveEvents(shapeHandle: ShapeId | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapePreSolveEventsFn(handle, flag ? 1 : 0); }
  enableShapeHitEvents(shapeHandle: ShapeId | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapeHitEventsFn(handle, flag ? 1 : 0); }
  setShapeSphere(shapeHandle: ShapeId | ShapeHandle, position: Vec3, radius: number): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setShapeSphereFn(handle, position[0], position[1], position[2], radius); }
  setShapeCapsule(shapeHandle: ShapeId | ShapeHandle, a: Vec3, b: Vec3, radius: number): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setShapeCapsuleFn(handle, a[0], a[1], a[2], b[0], b[1], b[2], radius); }
  applyShapeWind(shapeHandle: ShapeId | ShapeHandle, wind: Vec3, drag: number, lift: number, maxSpeed: number, wake = true): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.applyShapeWindFn(handle, wind[0], wind[1], wind[2], drag, lift, maxSpeed, wake ? 1 : 0); }
  setBodyType(bodyHandle: BodyHandle, type: BodyType): void { this.setBodyTypeFn(bodyHandle, type); }
  setBodyName(bodyHandle: BodyHandle, name: string): void { const ptr = this.module._malloc(name.length + 1); const heap8 = new Uint8Array(this.module.HEAPU8.buffer); for (let i = 0; i < name.length; i++) heap8[ptr + i] = name.charCodeAt(i); heap8[ptr + name.length] = 0; this.setBodyNameFn(bodyHandle, ptr); this.module._free(ptr); }
  setBodyGravityScale(bodyHandle: BodyHandle, scale: number): void { this.setBodyGravityScaleFn(bodyHandle, scale); }
  setBodySleepThreshold(bodyHandle: BodyHandle, threshold: number): void { this.setBodySleepThresholdFn(bodyHandle, threshold); }
  enableBodySleep(bodyHandle: BodyHandle, enable: boolean): void { this.enableBodySleepFn(bodyHandle, enable ? 1 : 0); }
  setBodyBullet(bodyHandle: BodyHandle, flag: boolean): void { this.setBodyBulletFn(bodyHandle, flag ? 1 : 0); }
  enableBodyContactRecycling(bodyHandle: BodyHandle, flag: boolean): void { this.enableBodyContactRecyclingFn(bodyHandle, flag ? 1 : 0); }
  enableBodyHitEvents(bodyHandle: BodyHandle, flag: boolean): void { this.enableBodyHitEventsFn(bodyHandle, flag ? 1 : 0); }
  setBodyMotionLocks(bodyHandle: BodyHandle, locks: { lockX?: boolean; lockY?: boolean; lockRotationX?: boolean; lockRotationY?: boolean; lockRotationZ?: boolean; lockLinearZ?: boolean } = {}): void { this.setBodyMotionLocksFn(bodyHandle, locks.lockX ? 1 : 0, locks.lockY ? 1 : 0, locks.lockLinearZ ? 1 : 0, locks.lockRotationX ? 1 : 0, locks.lockRotationY ? 1 : 0, locks.lockRotationZ ? 1 : 0); }
  setBodyMassData(bodyHandle: BodyHandle, mass: number, center: Vec3, inertia?: Mat3): void { if (inertia) { const heap = this.module.HEAPF32; const base = this.inertiaPtr >> 2; for (let i = 0; i < 9; i++) heap[base + i] = inertia[i]; this.setBodyMassDataFn(bodyHandle, mass, center[0], center[1], center[2], this.inertiaPtr); } else { this.setBodyMassDataFn(bodyHandle, mass, center[0], center[1], center[2], 0); } }
  getBodyMassData(bodyHandle: BodyHandle): BodyMassData { this.getBodyMassDataFn(bodyHandle, this.massDataPtr); const heap = this.module.HEAPF32; const base = this.massDataPtr >> 2; return { mass: heap[base], inertiaTrace: heap[base + 1] }; }
  applyBodyMassFromShapes(bodyHandle: BodyHandle): void { this.applyBodyMassFromShapesFn(bodyHandle); }
  setBodyTargetTransform(bodyHandle: BodyHandle, position: Vec3, rotation: Quat, timeStep: number, wake = true): void { this.setBodyTargetTransformFn(bodyHandle, position[0], position[1], position[2], rotation[0], rotation[1], rotation[2], rotation[3], timeStep, wake ? 1 : 0); }
  bodyEnable(bodyHandle: BodyHandle): void { this.bodyEnableFn(bodyHandle); }
  bodyDisable(bodyHandle: BodyHandle): void { this.bodyDisableFn(bodyHandle); }
  bodyIsEnabled(bodyHandle: BodyHandle): boolean { return this.bodyIsEnabledFn(bodyHandle) !== 0; }
  getBodyMass(bodyHandle: BodyHandle): number { return this.getBodyMassFn(bodyHandle); }
  getBodyLocalRotationalInertia(bodyHandle: BodyHandle): Mat3 { this.getBodyLocalRotationalInertiaFn(bodyHandle, this.inertiaPtr); const heap = this.module.HEAPF32; const base = this.inertiaPtr >> 2; return [heap[base + 0], heap[base + 1], heap[base + 2], heap[base + 3], heap[base + 4], heap[base + 5], heap[base + 6], heap[base + 7], heap[base + 8]]; }
  getBodyWorldCenter(bodyHandle: BodyHandle): Vec3 { return this.getBodyWorldCenterTo(bodyHandle, [0, 0, 0]); }
  getBodyWorldCenterTo(bodyHandle: BodyHandle, out: Vec3): Vec3 { this.getBodyWorldCenterFn(bodyHandle, this.pointPtr); return this.readPointInto(out); }
  getBodyWorldPoint(bodyHandle: BodyHandle, localPoint: Vec3): Vec3 { return this.getBodyWorldPointTo(bodyHandle, localPoint, [0, 0, 0]); }
  getBodyWorldPointXYZ(bodyHandle: BodyHandle, localX: number, localY: number, localZ: number): Vec3 { return this.getBodyWorldPointXYZTo(bodyHandle, localX, localY, localZ, [0, 0, 0]); }
  getBodyWorldPointTo(bodyHandle: BodyHandle, localPoint: Vec3, out: Vec3): Vec3 { return this.getBodyWorldPointXYZTo(bodyHandle, localPoint[0], localPoint[1], localPoint[2], out); }
  getBodyWorldPointXYZTo(bodyHandle: BodyHandle, localX: number, localY: number, localZ: number, out: Vec3): Vec3 { this.getBodyWorldPointFn(bodyHandle, localX, localY, localZ, this.pointPtr); return this.readPointInto(out); }
  getBodyLocalPointVelocity(bodyHandle: BodyHandle, localPoint: Vec3): Vec3 { return this.getBodyLocalPointVelocityTo(bodyHandle, localPoint, [0, 0, 0]); }
  getBodyLocalPointVelocityXYZ(bodyHandle: BodyHandle, localX: number, localY: number, localZ: number): Vec3 { return this.getBodyLocalPointVelocityXYZTo(bodyHandle, localX, localY, localZ, [0, 0, 0]); }
  getBodyLocalPointVelocityTo(bodyHandle: BodyHandle, localPoint: Vec3, out: Vec3): Vec3 { return this.getBodyLocalPointVelocityXYZTo(bodyHandle, localPoint[0], localPoint[1], localPoint[2], out); }
  getBodyLocalPointVelocityXYZTo(bodyHandle: BodyHandle, localX: number, localY: number, localZ: number, out: Vec3): Vec3 { this.getBodyLocalPointVelocityFn(bodyHandle, localX, localY, localZ, this.pointPtr); return this.readPointInto(out); }
  getBodyWorldPointVelocity(bodyHandle: BodyHandle, worldPoint: Vec3): Vec3 { return this.getBodyWorldPointVelocityTo(bodyHandle, worldPoint, [0, 0, 0]); }
  getBodyWorldPointVelocityXYZ(bodyHandle: BodyHandle, worldX: number, worldY: number, worldZ: number): Vec3 { return this.getBodyWorldPointVelocityXYZTo(bodyHandle, worldX, worldY, worldZ, [0, 0, 0]); }
  getBodyWorldPointVelocityTo(bodyHandle: BodyHandle, worldPoint: Vec3, out: Vec3): Vec3 { return this.getBodyWorldPointVelocityXYZTo(bodyHandle, worldPoint[0], worldPoint[1], worldPoint[2], out); }
  getBodyWorldPointVelocityXYZTo(bodyHandle: BodyHandle, worldX: number, worldY: number, worldZ: number, out: Vec3): Vec3 { this.getBodyWorldPointVelocityFn(bodyHandle, worldX, worldY, worldZ, this.pointPtr); return this.readPointInto(out); }
  getJointConstraintForce(jointHandle: JointHandle): Vec3 { this.getJointConstraintForceFn(jointHandle, this.pointPtr); return this.readPointInto([0, 0, 0]); }
  getJointConstraintTorque(jointHandle: JointHandle): Vec3 { this.getJointConstraintTorqueFn(jointHandle, this.pointPtr); return this.readPointInto([0, 0, 0]); }
  getJointLinearSeparation(jointHandle: JointHandle): number { return this.getJointLinearSeparationFn(jointHandle); }
  setRevoluteJointTargetAngle(jointHandle: JointHandle, targetRadians: number): void { this.revoluteJointSetTargetAngleFn(jointHandle, targetRadians); }
  createPrismaticJoint(worldHandle: WorldHandle, bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; constraintHertz?: number; constraintDampingRatio?: number; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetTranslation?: number; enableLimit?: boolean; lowerTranslation?: number; upperTranslation?: number; enableMotor?: boolean; maxMotorForce?: number; motorSpeed?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointHandle { const la = options.localFrameA?.position ?? [0,0,0]; const laq = options.localFrameA?.rotation ?? [0,0,0,1]; const lb = options.localFrameB?.position ?? [0,0,0]; const lbq = options.localFrameB?.rotation ?? [0,0,0,1]; const [forceThreshold, torqueThreshold, collideConnected] = jointThresholdArgs(options); return this.requireSlotHandle<JointHandle>(this.createPrismaticJointFn(worldHandle, bodyAHandle, bodyBHandle, la[0], la[1], la[2], laq[0], laq[1], laq[2], laq[3], lb[0], lb[1], lb[2], lbq[0], lbq[1], lbq[2], lbq[3], options.constraintHertz ?? 60, options.constraintDampingRatio ?? 2, options.enableSpring ? 1 : 0, options.hertz ?? 0, options.dampingRatio ?? 0, options.targetTranslation ?? 0, options.enableLimit ? 1 : 0, options.lowerTranslation ?? 0, options.upperTranslation ?? 0, options.enableMotor ? 1 : 0, options.maxMotorForce ?? 0, options.motorSpeed ?? 0, forceThreshold, torqueThreshold, collideConnected), "joints"); }
  createWeldJoint(worldHandle: WorldHandle, bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; linearHertz?: number; angularHertz?: number; linearDampingRatio?: number; angularDampingRatio?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointHandle { const la = options.localFrameA?.position ?? [0,0,0]; const laq = options.localFrameA?.rotation ?? [0,0,0,1]; const lb = options.localFrameB?.position ?? [0,0,0]; const lbq = options.localFrameB?.rotation ?? [0,0,0,1]; const [forceThreshold, torqueThreshold, collideConnected] = jointThresholdArgs(options); return this.requireSlotHandle<JointHandle>(this.createWeldJointFn(worldHandle, bodyAHandle, bodyBHandle, la[0], la[1], la[2], laq[0], laq[1], laq[2], laq[3], lb[0], lb[1], lb[2], lbq[0], lbq[1], lbq[2], lbq[3], options.linearHertz ?? 0, options.angularHertz ?? 0, options.linearDampingRatio ?? 0, options.angularDampingRatio ?? 0, forceThreshold, torqueThreshold, collideConnected), "joints"); }
  createDistanceJoint(worldHandle: WorldHandle, bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; length?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointHandle { const la = options.localFrameA?.position ?? [0,0,0]; const laq = options.localFrameA?.rotation ?? [0,0,0,1]; const lb = options.localFrameB?.position ?? [0,0,0]; const lbq = options.localFrameB?.rotation ?? [0,0,0,1]; const [forceThreshold, torqueThreshold, collideConnected] = jointThresholdArgs(options); return this.requireSlotHandle<JointHandle>(this.createDistanceJointFn(worldHandle, bodyAHandle, bodyBHandle, la[0], la[1], la[2], laq[0], laq[1], laq[2], laq[3], lb[0], lb[1], lb[2], lbq[0], lbq[1], lbq[2], lbq[3], options.length ?? 0, forceThreshold, torqueThreshold, collideConnected), "joints"); }
  worldExplode(worldHandle: WorldHandle, position: Vec3, radius: number, falloff: number, impulsePerArea: number, maskBits = U64_MAX): void { this.worldExplodeFn(worldHandle, position[0], position[1], position[2], radius, falloff, impulsePerArea, maskBits); }

  applyLinearImpulse(bodyHandle: BodyHandle, impulse: Vec3, point: Vec3, wake = true): void { this.applyLinearImpulseFn(bodyHandle, impulse[0], impulse[1], impulse[2], point[0], point[1], point[2], wake ? 1 : 0); }
  applyLinearImpulseToCenter(bodyHandle: BodyHandle, impulse: Vec3, wake = true): void { this.applyLinearImpulseToCenterFn(bodyHandle, impulse[0], impulse[1], impulse[2], wake ? 1 : 0); }
}

export class PhysicsWorld {
  constructor(private readonly runtime: Box3DRuntime, public readonly handle: WorldHandle) {}
  createBody(def: BodyDef = {}): BodyHandle { return this.runtime.createBody(this.handle, def); }
  createBox(options: BoxOptions): BodyHandle { return this.runtime.createBox(this.handle, options); }
  createBoxWithShape(options: BoxOptions): ShapeHandle { return this.runtime.createBoxWithShape(this.handle, options); }
  createSphere(options: SphereOptions): BodyHandle { return this.runtime.createSphere(this.handle, options); }
  createSphereWithShape(options: SphereOptions): ShapeHandle { return this.runtime.createSphereWithShape(this.handle, options); }
  createSphereShape(bodyHandle: BodyHandle, center: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle { return this.runtime.createSphereShape(bodyHandle, center, radius, def); }
  createCapsuleShape(bodyHandle: BodyHandle, center1: Vec3, center2: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle { return this.runtime.createCapsuleShape(bodyHandle, center1, center2, radius, def); }
  createHullShape(bodyHandle: BodyHandle, halfWidths: Vec3, def: ShapeDef = {}): ShapeHandle { return this.runtime.createHullShape(bodyHandle, halfWidths, def); }
  createTransformedHullShape(bodyHandle: BodyHandle, halfWidths: Vec3, transform?: { position?: Vec3; rotation?: Quat }, scale?: Vec3, def?: ShapeDef): ShapeHandle { return this.runtime.createTransformedHullShape(bodyHandle, halfWidths, transform, scale, def); }
  createShapeFromHull(bodyHandle: BodyHandle, hullHandle: HullHandle, def?: ShapeDef): ShapeId { return this.runtime.createShapeFromHull(bodyHandle, hullHandle, def); }
  createGridMesh(xCount: number, zCount: number, cellWidth: number, materialCount = 1, identifyEdges = true): MeshHandle { return this.runtime.createGridMesh(this.handle, xCount, zCount, cellWidth, materialCount, identifyEdges); }
  createTorusMesh(radialResolution: number, tubularResolution: number, radius: number, thickness: number): MeshHandle { return this.runtime.createTorusMesh(this.handle, radialResolution, tubularResolution, radius, thickness); }
  destroyMesh(meshHandle: MeshHandle): void { this.runtime.destroyMesh(meshHandle); }
  createMeshShape(bodyHandle: BodyHandle, meshHandle: MeshHandle, def: MeshShapeOptions = {}): ShapeHandle { return this.runtime.createMeshShape(bodyHandle, meshHandle, def); }
  setMesh(shapeHandle: ShapeId | ShapeHandle, meshHandle: MeshHandle, scale: Vec3 = [1, 1, 1]): void { this.runtime.setMesh(shapeHandle, meshHandle, scale); }
  createCompoundShape(bodyHandle: BodyHandle, compoundHandle: CompoundHandle, density = 1): ShapeId { return this.runtime.createCompoundShape(bodyHandle, compoundHandle, density); }
  getBodyShapes(bodyHandle: BodyHandle): ShapeId[] { return this.runtime.getBodyShapes(bodyHandle); }
  getCompoundTreeHeight(compoundHandle: CompoundHandle): number { return this.runtime.getCompoundTreeHeight(compoundHandle); }
  destroyCompound(compoundHandle: CompoundHandle): void { this.runtime.destroyCompound(compoundHandle); }
  destroyBody(bodyHandle: BodyHandle): void { this.runtime.destroyBody(bodyHandle); }
  destroyShape(shapeHandle: ShapeId | ShapeHandle, updateBodyMass = true): void { this.runtime.destroyShape(shapeHandle, updateBodyMass); }
  destroyJoint(jointHandle: JointHandle): void { this.runtime.destroyJoint(jointHandle); }
  setBodyTransform(bodyHandle: BodyHandle, position: Vec3, rotation: Quat = [0,0,0,1]): void { this.runtime.setBodyTransform(bodyHandle, position, rotation); }
  setBodyLinearVelocity(bodyHandle: BodyHandle, velocity: Vec3): void { this.runtime.setBodyLinearVelocity(bodyHandle, velocity); }
  setBodyAngularVelocity(bodyHandle: BodyHandle, velocity: Vec3): void { this.runtime.setBodyAngularVelocity(bodyHandle, velocity); }
  getBodyLinearVelocity(bodyHandle: BodyHandle): Vec3 { return this.runtime.getBodyLinearVelocity(bodyHandle); }
  getBodyLinearVelocityTo(bodyHandle: BodyHandle, out: Vec3): Vec3 { return this.runtime.getBodyLinearVelocityTo(bodyHandle, out); }
  getBodyAngularVelocity(bodyHandle: BodyHandle): Vec3 { return this.runtime.getBodyAngularVelocity(bodyHandle); }
  getBodyAngularVelocityTo(bodyHandle: BodyHandle, out: Vec3): Vec3 { return this.runtime.getBodyAngularVelocityTo(bodyHandle, out); }
  applyLinearImpulse(bodyHandle: BodyHandle, impulse: Vec3, point: Vec3, wake = true): void { this.runtime.applyLinearImpulse(bodyHandle, impulse, point, wake); }
  applyLinearImpulseToCenter(bodyHandle: BodyHandle, impulse: Vec3, wake = true): void { this.runtime.applyLinearImpulseToCenter(bodyHandle, impulse, wake); }
  bodyIsAwake(bodyHandle: BodyHandle): boolean { return this.runtime.bodyIsAwake(bodyHandle); }
  getBodyDebugColor(bodyHandle: BodyHandle): number { return this.runtime.getBodyDebugColor(bodyHandle); }
  getBodyType(bodyHandle: BodyHandle): BodyType { return this.runtime.getBodyType(bodyHandle); }
  setBodyAwake(bodyHandle: BodyHandle, awake: boolean): void { this.runtime.setBodyAwake(bodyHandle, awake); }
  setBodyDamping(bodyHandle: BodyHandle, linearDamping: number, angularDamping: number): void { this.runtime.setBodyDamping(bodyHandle, linearDamping, angularDamping); }
  getBodyLocalPoint(bodyHandle: BodyHandle, worldPoint: Vec3): Vec3 { return this.runtime.getBodyLocalPoint(bodyHandle, worldPoint); }
  getBodyLocalPointXYZ(bodyHandle: BodyHandle, worldX: number, worldY: number, worldZ: number): Vec3 { return this.runtime.getBodyLocalPointXYZ(bodyHandle, worldX, worldY, worldZ); }
  getBodyLocalPointTo(bodyHandle: BodyHandle, worldPoint: Vec3, out: Vec3): Vec3 { return this.runtime.getBodyLocalPointTo(bodyHandle, worldPoint, out); }
  getBodyLocalPointXYZTo(bodyHandle: BodyHandle, worldX: number, worldY: number, worldZ: number, out: Vec3): Vec3 { return this.runtime.getBodyLocalPointXYZTo(bodyHandle, worldX, worldY, worldZ, out); }
  createMotorJoint(bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: MotorJointOptions = {}): JointHandle { return this.runtime.createMotorJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createFilterJoint(bodyAHandle: BodyHandle, bodyBHandle: BodyHandle): JointHandle { return this.runtime.createFilterJoint(this.handle, bodyAHandle, bodyBHandle); }
  createRevoluteJoint(bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; constraintHertz?: number; constraintDampingRatio?: number; targetAngle?: number; enableSpring?: boolean; hertz?: number; dampingRatio?: number; enableLimit?: boolean; lowerAngle?: number; upperAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorSpeed?: number } = {}): JointHandle { return this.runtime.createRevoluteJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createSphericalJoint(bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetRotation?: Quat; enableConeLimit?: boolean; coneAngle?: number; enableTwistLimit?: boolean; lowerTwistAngle?: number; upperTwistAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorVelocity?: Vec3 } = {}): JointHandle { return this.runtime.createSphericalJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createHuman(position: Vec3, options: { frictionTorque?: number; hertz?: number; dampingRatio?: number; groupIndex?: number; colorize?: boolean } = {}): HumanHandle { return this.runtime.createHuman(this.handle, position, options); }
  getBodyTransform(bodyHandle: BodyHandle): BodyTransform { return this.runtime.readBodyTransform(bodyHandle); }
  getBodyMassData(bodyHandle: BodyHandle): BodyMassData { return this.runtime.getBodyMassData(bodyHandle); }
  bodyEnable(bodyHandle: BodyHandle): void { this.runtime.bodyEnable(bodyHandle); }
  bodyDisable(bodyHandle: BodyHandle): void { this.runtime.bodyDisable(bodyHandle); }
  bodyIsEnabled(bodyHandle: BodyHandle): boolean { return this.runtime.bodyIsEnabled(bodyHandle); }
  getBodyMass(bodyHandle: BodyHandle): number { return this.runtime.getBodyMass(bodyHandle); }
  getBodyLocalRotationalInertia(bodyHandle: BodyHandle): Mat3 { return this.runtime.getBodyLocalRotationalInertia(bodyHandle); }
  getBodyWorldCenter(bodyHandle: BodyHandle): Vec3 { return this.runtime.getBodyWorldCenter(bodyHandle); }
  getBodyWorldCenterTo(bodyHandle: BodyHandle, out: Vec3): Vec3 { return this.runtime.getBodyWorldCenterTo(bodyHandle, out); }
  getBodyWorldPoint(bodyHandle: BodyHandle, localPoint: Vec3): Vec3 { return this.runtime.getBodyWorldPoint(bodyHandle, localPoint); }
  getBodyWorldPointXYZ(bodyHandle: BodyHandle, localX: number, localY: number, localZ: number): Vec3 { return this.runtime.getBodyWorldPointXYZ(bodyHandle, localX, localY, localZ); }
  getBodyWorldPointTo(bodyHandle: BodyHandle, localPoint: Vec3, out: Vec3): Vec3 { return this.runtime.getBodyWorldPointTo(bodyHandle, localPoint, out); }
  getBodyWorldPointXYZTo(bodyHandle: BodyHandle, localX: number, localY: number, localZ: number, out: Vec3): Vec3 { return this.runtime.getBodyWorldPointXYZTo(bodyHandle, localX, localY, localZ, out); }
  getBodyLocalPointVelocity(bodyHandle: BodyHandle, localPoint: Vec3): Vec3 { return this.runtime.getBodyLocalPointVelocity(bodyHandle, localPoint); }
  getBodyLocalPointVelocityXYZ(bodyHandle: BodyHandle, localX: number, localY: number, localZ: number): Vec3 { return this.runtime.getBodyLocalPointVelocityXYZ(bodyHandle, localX, localY, localZ); }
  getBodyLocalPointVelocityTo(bodyHandle: BodyHandle, localPoint: Vec3, out: Vec3): Vec3 { return this.runtime.getBodyLocalPointVelocityTo(bodyHandle, localPoint, out); }
  getBodyLocalPointVelocityXYZTo(bodyHandle: BodyHandle, localX: number, localY: number, localZ: number, out: Vec3): Vec3 { return this.runtime.getBodyLocalPointVelocityXYZTo(bodyHandle, localX, localY, localZ, out); }
  getBodyWorldPointVelocity(bodyHandle: BodyHandle, worldPoint: Vec3): Vec3 { return this.runtime.getBodyWorldPointVelocity(bodyHandle, worldPoint); }
  getBodyWorldPointVelocityXYZ(bodyHandle: BodyHandle, worldX: number, worldY: number, worldZ: number): Vec3 { return this.runtime.getBodyWorldPointVelocityXYZ(bodyHandle, worldX, worldY, worldZ); }
  getBodyWorldPointVelocityTo(bodyHandle: BodyHandle, worldPoint: Vec3, out: Vec3): Vec3 { return this.runtime.getBodyWorldPointVelocityTo(bodyHandle, worldPoint, out); }
  getBodyWorldPointVelocityXYZTo(bodyHandle: BodyHandle, worldX: number, worldY: number, worldZ: number, out: Vec3): Vec3 { return this.runtime.getBodyWorldPointVelocityXYZTo(bodyHandle, worldX, worldY, worldZ, out); }
  getJointConstraintForce(jointHandle: JointHandle): Vec3 { return this.runtime.getJointConstraintForce(jointHandle); }
  getJointConstraintTorque(jointHandle: JointHandle): Vec3 { return this.runtime.getJointConstraintTorque(jointHandle); }
  getJointLinearSeparation(jointHandle: JointHandle): number { return this.runtime.getJointLinearSeparation(jointHandle); }
  setRevoluteJointTargetAngle(jointHandle: JointHandle, targetRadians: number): void { this.runtime.setRevoluteJointTargetAngle(jointHandle, targetRadians); }
  createPrismaticJoint(bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; constraintHertz?: number; constraintDampingRatio?: number; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetTranslation?: number; enableLimit?: boolean; lowerTranslation?: number; upperTranslation?: number; enableMotor?: boolean; maxMotorForce?: number; motorSpeed?: number } = {}): JointHandle { return this.runtime.createPrismaticJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createWeldJoint(bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; linearHertz?: number; angularHertz?: number; linearDampingRatio?: number; angularDampingRatio?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointHandle { return this.runtime.createWeldJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createDistanceJoint(bodyAHandle: BodyHandle, bodyBHandle: BodyHandle, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; length?: number; forceThreshold?: number; torqueThreshold?: number; collideConnected?: boolean } = {}): JointHandle { return this.runtime.createDistanceJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  explode(position: Vec3, radius: number, falloff: number, impulsePerArea: number, maskBits = U64_MAX): void { this.runtime.worldExplode(this.handle, position, radius, falloff, impulsePerArea, maskBits); }
  getCounters(): WorldCounters { return this.runtime.getWorldCounters(this.handle); }
  getAwakeBodyCount(): number { return this.runtime.getWorldAwakeBodyCount(this.handle); }
  getWorkerCount(): number { return this.runtime.getWorldWorkerCount(this.handle); }
  getProfile(): WorldProfile { return this.runtime.getWorldProfile(this.handle); }
  rayCastClosest(origin: Vec3, translation: Vec3, categoryBits = U64_MAX, maskBits = U64_MAX): { shapeHandle: ShapeId; bodyHandle: BodyHandle; point: Vec3; normal: Vec3; fraction: number } | null { return this.runtime.rayCastClosest(this.handle, origin, translation, categoryBits, maskBits); }
  allocBodyBatchBuffers(capacity: number): BodyBatchBuffers { return this.runtime.allocBodyBatchBuffers(capacity); }
  freeBodyBatchBuffers(buffers: BodyBatchBuffers): void { this.runtime.freeBodyBatchBuffers(buffers); }
  getMemoryView(): RuntimeMemoryView32 { return this.runtime.getMemoryView(); }
  writeBodyHandles(buffers: BodyBatchBuffers, bodyHandles: readonly number[]): void { this.runtime.writeBodyHandles(buffers, bodyHandles); }
  writeBodyTransforms(count: number, bodyHandlesPtr: number, outPositionsPtr: number, outRotationsPtr: number, outAwakePtr: number, outColorsPtr: number): void {
    this.runtime.writeBodyTransforms(count, bodyHandlesPtr, outPositionsPtr, outRotationsPtr, outAwakePtr, outColorsPtr);
  }

  writeBodyTransformsLight(count: number, bodyHandlesPtr: number, outPositionsPtr: number, outRotationsPtr: number, outAwakePtr: number, outColorsPtr: number): void {
    this.runtime.writeBodyTransformsLight(count, bodyHandlesPtr, outPositionsPtr, outRotationsPtr, outAwakePtr, outColorsPtr);
  }
  step(dt = 1 / 60, substeps = 4): void { this.runtime.step(this.handle, dt, substeps); }
  enableSleeping(flag: boolean): void { this.runtime.enableWorldSleeping(this.handle, flag); }
  enableContinuous(flag: boolean): void { this.runtime.enableWorldContinuous(this.handle, flag); }
  enableWarmStarting(flag: boolean): void { this.runtime.enableWorldWarmStarting(this.handle, flag); }
  setContactTuning(hertz: number, dampingRatio: number, contactSpeed: number): void { this.runtime.setWorldContactTuning(this.handle, hertz, dampingRatio, contactSpeed); }
  setContactRecycleDistance(distance: number): void { this.runtime.setWorldContactRecycleDistance(this.handle, distance); }
  setWorkerCount(count: number): void { this.runtime.setWorldWorkerCount(this.handle, count); }
  destroy(): void { this.runtime.destroyWorld(this.handle); }
}
