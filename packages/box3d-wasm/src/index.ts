export type Vec3 = [number, number, number];
export type Quat = [number, number, number, number];

export const B3_PI = 3.14159265359;
export const B3_DEG_TO_RAD = 0.01745329251;
export const B3_AXIS_X: Vec3 = [1, 0, 0];
export const B3_AXIS_Y: Vec3 = [0, 1, 0];
export const B3_AXIS_Z: Vec3 = [0, 0, 1];

export function quatFromAxisAngle(axis: Vec3, radians: number): Quat {
  const halfAngle = 0.5 * radians;
  const sine = Math.sin(halfAngle);
  return [axis[0] * sine, axis[1] * sine, axis[2] * sine, Math.cos(halfAngle)];
}

declare global { var BOX3D_POOL_SIZE: number | undefined; }

export enum BodyType { Static = 0, Kinematic = 1, Dynamic = 2 }

export interface WorldOptions { gravity?: Vec3; workerCount?: number; }

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
export interface MotorJointOptions { localFrameA?: Vec3; localFrameB?: Vec3; linearVelocity?: Vec3; angularVelocity?: Vec3; maxVelocityForce?: number; maxVelocityTorque?: number; linearHertz?: number; linearDampingRatio?: number; maxSpringForce?: number; angularHertz?: number; angularDampingRatio?: number; maxSpringTorque?: number; }
export interface BodyTransform { position: Vec3; rotation: Quat; }
export interface BodyMassData { mass: number; inertiaTrace: number; }
export type Mat3 = [number, number, number, number, number, number, number, number, number];
export interface WorldCounters { bodyCount: number; shapeCount: number; contactCount: number; jointCount: number; islandCount: number; staticTreeHeight: number; treeHeight: number; }
export interface BodyBatchBuffers { bodyHandlesPtr: number; positionsPtr: number; rotationsPtr: number; awakePtr: number; colorsPtr: number; capacity: number; }
export interface RuntimeMemoryView { heapF32: Float32Array; heapU8: Uint8Array; }
export interface RuntimeMemoryView32 extends RuntimeMemoryView { heap32: Int32Array; }
export interface CompoundHullEntry { halfWidths: Vec3; transform: BodyTransform; friction?: number; restitution?: number; rollingResistance?: number; }
export interface CompoundSphereEntry { center: Vec3; radius: number; friction?: number; restitution?: number; rollingResistance?: number; }
export interface ShapeHandle { bodyHandle: number; shapeHandle: number; }
export interface RuntimeLoadOptions { version?: string; variant?: "release" | "profile"; poolSize?: number; }
export interface RuntimeAPI { createWorld(options?: WorldOptions): PhysicsWorld; checkThreadingSupport(): number; }

type CModule = { cwrap(name: string, returnType: "number", argTypes: readonly string[]): (...args: number[]) => number; cwrap(name: string, returnType: null, argTypes: readonly string[]): (...args: number[]) => void; HEAPF32: Float32Array; HEAPU8: Uint8Array; HEAP32: Int32Array; wasmMemory?: WebAssembly.Memory; _malloc(size: number): number; _free(ptr: number): void; };
type CreateWorldFn = (gravityX: number, gravityY: number, gravityZ: number, workerCount: number) => number;
type CreateBodyFn = (worldHandle: number, bodyType: number, px: number, py: number, pz: number, enableSleep: number, awake: number) => number;
type DestroyWorldFn = (worldHandle: number) => void;
type CreateBoxFn = (worldHandle: number, px: number, py: number, pz: number, hx: number, hy: number, hz: number, isStatic: number, density: number) => number;
type CreateSphereFn = (worldHandle: number, px: number, py: number, pz: number, radius: number, vx: number, vy: number, vz: number, density: number) => number;
type CreateHullShapeFn = (bodyHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, updateBodyMass: number, tx: number, ty: number, tz: number, qx: number, qy: number, qz: number, qw: number, hx: number, hy: number, hz: number) => number;
type CreateTransformedHullShapeFn = (bodyHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, tx: number, ty: number, tz: number, qx: number, qy: number, qz: number, qw: number, hx: number, hy: number, hz: number, sx: number, sy: number, sz: number) => number;
type CreateSphereShapeFn = (bodyHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, px: number, py: number, pz: number, radius: number) => number;
type CreateCapsuleShapeFn = (bodyHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, ax: number, ay: number, az: number, bx: number, by: number, bz: number, radius: number) => number;
type CreateShapeFromHullFn = (bodyHandle: number, hullHandle: number, density: number, friction: number, restitution: number, rollingResistance: number, updateBodyMass: number) => number;
type CreateCylinderFn = (height: number, radius: number, yOffset: number, sides: number) => number;
type CreateHullFromPointsFn = (numPoints: number, points: number) => number;
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
type CreateMotorJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localBx: number, localBy: number, localBz: number, linearVx: number, linearVy: number, linearVz: number, maxVelocityForce: number, angularVx: number, angularVy: number, angularVz: number, maxVelocityTorque: number, linearHertz: number, linearDampingRatio: number, maxSpringForce: number, angularHertz: number, angularDampingRatio: number, maxSpringTorque: number) => number;
type CreateFilterJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number) => number;
type CreateRevoluteJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, targetAngle: number, enableSpring: number, hertz: number, dampingRatio: number, enableLimit: number, lowerAngle: number, upperAngle: number, enableMotor: number, maxMotorTorque: number, motorSpeed: number) => number;
type CreateSphericalJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, enableSpring: number, hertz: number, dampingRatio: number, targetQx: number, targetQy: number, targetQz: number, targetQw: number, enableConeLimit: number, coneAngle: number, enableTwistLimit: number, lowerTwistAngle: number, upperTwistAngle: number, enableMotor: number, maxMotorTorque: number, motorVx: number, motorVy: number, motorVz: number) => number;
type CreateHumanFn = (worldHandle: number, px: number, py: number, pz: number, frictionTorque: number, hertz: number, dampingRatio: number, groupIndex: number, colorize: number) => number;
type GetHumanBoneBodyFn = (humanHandle: number, boneIndex: number) => number;
type GetHumanBoneCountFn = () => number;
type HumanSetVelocityFn = (humanHandle: number, x: number, y: number, z: number) => void;
type HumanSetBulletFn = (humanHandle: number, flag: number) => void;
type HumanSetJointFloatFn = (humanHandle: number, value: number) => void;
type StepFn = (worldHandle: number, timeStep: number, subStepCount: number) => void;
type GetBodyTransformFn = (bodyHandle: number, outTransform: number) => void;
type ShapeSetSurfaceMaterialFn = (shapeHandle: number, friction: number, restitution: number, rollingResistance: number) => void;
type ShapeSetFilterFn = (shapeHandle: number, categoryBits: number, maskBits: number, groupIndex: number, invokeContacts: number) => void;
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
type CreatePrismaticJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, enableSpring: number, hertz: number, dampingRatio: number, targetTranslation: number, enableLimit: number, lowerTranslation: number, upperTranslation: number, enableMotor: number, maxMotorForce: number, motorSpeed: number) => number;
type CreateWeldJointFn = (worldHandle: number, bodyAHandle: number, bodyBHandle: number, localAx: number, localAy: number, localAz: number, localAqx: number, localAqy: number, localAqz: number, localAqw: number, localBx: number, localBy: number, localBz: number, localBqx: number, localBqy: number, localBqz: number, localBqw: number, linearHertz: number, angularHertz: number, linearDampingRatio: number, angularDampingRatio: number) => number;
type WorldExplodeFn = (worldHandle: number, px: number, py: number, pz: number, radius: number, falloff: number, impulsePerArea: number, maskBits: number) => void;
type GetShapeBodyHandleFn = (shapeHandle: number) => number;
type ShapeSetFrictionFn = (shapeHandle: number, friction: number) => void;
type ShapeSetRestitutionFn = (shapeHandle: number, restitution: number) => void;

type ModuleFactory = (options: { locateFile(path: string): string }) => Promise<CModule>;
type ModuleImport = { default: ModuleFactory };

function vec3(x = 0, y = 0, z = 0): Vec3 { return [x, y, z]; }
function versionedUrl(url: string, version: string | undefined): string { if (!version) return url; return `${url}${url.includes("?") ? "&" : "?"}v=${encodeURIComponent(version)}`; }
function wasmDirectory(variant: RuntimeLoadOptions["variant"]): string { return variant === "profile" ? "wasm/profile" : "wasm"; }

const U64_MAX = 0xFFFFFFFF;

function defaults<T>(val: T | undefined, def: T): T { return val !== undefined ? val : def; }

export class Box3DRuntime implements RuntimeAPI {
  static async load(options: RuntimeLoadOptions = {}): Promise<Box3DRuntime> {
    if (options.poolSize !== undefined) globalThis.BOX3D_POOL_SIZE = options.poolSize;
    const locationHref = typeof window !== "undefined" ? window.location.href : globalThis.location.href;
    const baseUrl = typeof document === "undefined" ? "/" : new URL(".", locationHref).pathname;
    const moduleUrl = versionedUrl(`${baseUrl}${wasmDirectory(options.variant)}/box3d-web.js`, options.version);
    const absServerUrl = new URL(moduleUrl, locationHref).href;
    const moduleImport = (await import(/* @vite-ignore */ absServerUrl)) as ModuleImport;
    const module = await moduleImport.default({ locateFile(path: string): string { return versionedUrl(new URL(path, absServerUrl).href, options.version); } });
    return new Box3DRuntime(module);
  }

  private readonly module: CModule;
  private readonly createWorldFn: CreateWorldFn;
  private readonly createBodyFn: CreateBodyFn;
  private readonly destroyWorldFn: DestroyWorldFn;
  private readonly createBoxFn: CreateBoxFn;
  private readonly createSphereFn: CreateSphereFn;
  private readonly createHullShapeFn: CreateHullShapeFn;
  private readonly createTransformedHullShapeFn: CreateTransformedHullShapeFn;
  private readonly createSphereShapeFn: CreateSphereShapeFn;
  private readonly createCapsuleShapeFn: CreateCapsuleShapeFn;
  private readonly createShapeFromHullFn: CreateShapeFromHullFn;
  private readonly createCylinderFn: CreateCylinderFn;
  private readonly createHullFromPointsFn: CreateHullFromPointsFn;
  private readonly destroyHullFn: DestroyHullFn;
  private readonly createCompoundFn: CreateCompoundFn;
  private readonly createCompoundFromHullsFn: CreateCompoundFromHullsFn;
  private readonly createCompoundFromSpheresFn: CreateCompoundFromSpheresFn;
  private readonly destroyCompoundFn: DestroyCompoundFn;
  private readonly getCompoundTreeHeightFn: GetCompoundTreeHeightFn;
  private readonly createCompoundShapeFn: CreateCompoundShapeFn;
  private readonly destroyBodyFn: DestroyBodyFn;
  private readonly destroyJointFn: DestroyJointFn;
  private readonly setBodyTransformFn: SetBodyTransformFn;
  private readonly setBodyLinearVelocityFn: SetBodyLinearVelocityFn;
  private readonly setBodyAngularVelocityFn: SetBodyAngularVelocityFn;
  private readonly getBodyLinearVelocityFn: GetBodyVelocityFn;
  private readonly getBodyAngularVelocityFn: GetBodyVelocityFn;
  private readonly setBodyAwakeFn: SetBodyAwakeFn;
  private readonly setBodyDampingFn: SetBodyDampingFn;
  private readonly getBodyLocalPointFn: GetBodyLocalPointFn;
  private readonly createMotorJointFn: CreateMotorJointFn;
  private readonly createFilterJointFn: CreateFilterJointFn;
  private readonly createRevoluteJointFn: CreateRevoluteJointFn;
  private readonly createSphericalJointFn: CreateSphericalJointFn;
  private readonly createHumanFn: CreateHumanFn;
  private readonly getHumanBoneBodyFn: GetHumanBoneBodyFn;
  private readonly getHumanBoneCountFn: GetHumanBoneCountFn;
  private readonly humanSetVelocityFn: HumanSetVelocityFn;
  private readonly humanSetBulletFn: HumanSetBulletFn;
  private readonly humanSetJointFrictionTorqueFn: HumanSetJointFloatFn;
  private readonly humanSetJointSpringHertzFn: HumanSetJointFloatFn;
  private readonly humanSetJointDampingRatioFn: HumanSetJointFloatFn;
  private readonly stepFn: StepFn;
  private readonly getBodyTransformFn: GetBodyTransformFn;
  private readonly setSurfaceMaterialFn: ShapeSetSurfaceMaterialFn;
  private readonly setFilterFn: ShapeSetFilterFn;
  private readonly enableShapeSensorEventsFn: ShapeEnableBoolFn;
  private readonly enableShapeContactEventsFn: ShapeEnableBoolFn;
  private readonly enableShapePreSolveEventsFn: ShapeEnableBoolFn;
  private readonly enableShapeHitEventsFn: ShapeEnableBoolFn;
  private readonly setShapeSphereFn: ShapeSetSphereFn;
  private readonly setShapeCapsuleFn: ShapeSetCapsuleFn;
  private readonly applyShapeWindFn: ShapeApplyWindFn;
  private readonly bodyIsAwakeFn: BodyIsAwakeFn;
  private readonly getBodyDebugColorFn: GetBodyDebugColorFn;
  private readonly getBodyTypeFn: GetBodyTypeFn;
  private readonly setBodyTypeFn: BodySetTypeFn;
  private readonly setBodyNameFn: BodySetNameFn;
  private readonly setBodyGravityScaleFn: BodySetGravityScaleFn;
  private readonly setBodySleepThresholdFn: BodySetSleepThresholdFn;
  private readonly enableBodySleepFn: BodyEnableSleepFn;
  private readonly setBodyBulletFn: BodySetBulletFn;
  private readonly enableBodyContactRecyclingFn: BodyEnableContactRecyclingFn;
  private readonly enableBodyHitEventsFn: BodyEnableHitEventsFn;
  private readonly setBodyMotionLocksFn: BodySetMotionLocksFn;
  private readonly setBodyMassDataFn: BodySetMassDataFn;
  private readonly getBodyMassDataFn: BodyGetMassDataFn;
  private readonly applyBodyMassFromShapesFn: BodyApplyMassFromShapesFn;
  private readonly setBodyTargetTransformFn: BodySetTargetTransformFn;
  private readonly applyLinearImpulseFn: ApplyLinearImpulseFn;
  private readonly applyLinearImpulseToCenterFn: ApplyLinearImpulseToCenterFn;
  private readonly enableWorldSleepFn: WorldEnableBoolFn;
  private readonly enableWorldContinuousFn: WorldEnableBoolFn;
  private readonly enableWorldWarmStartingFn: WorldEnableBoolFn;
  private readonly setWorldContactTuningFn: WorldSetContactTuningFn;
  private readonly setWorldContactRecycleDistanceFn: WorldSetFloatFn;
  private readonly setWorldWorkerCountFn: WorldSetWorkerCountFn;
  private readonly getWorldCountersFn: GetWorldCountersFn;
  private readonly getWorldProfileFn: GetWorldProfileFn;
  private readonly getWorldAwakeBodyCountFn: GetWorldAwakeBodyCountFn;
  private readonly checkThreadingSupportFn: CheckThreadingSupportFn;
  private readonly getWorldWorkerCountFn: GetWorldWorkerCountFn;
  private readonly writeBodyTransformsFn: WriteBodyTransformsFn;
  private readonly writeBodyTransformsLightFn: WriteBodyTransformsLightFn;
  private readonly rayCastClosestFn: RayCastClosestFn;
  private readonly bodyEnableFn: BodyEnableFn;
  private readonly bodyDisableFn: BodyEnableFn;
  private readonly bodyIsEnabledFn: BodyIsEnabledFn;
  private readonly getBodyMassFn: GetBodyMassFn;
  private readonly getBodyLocalRotationalInertiaFn: GetBodyLocalRotationalInertiaFn;
  private readonly getBodyWorldCenterFn: GetBodyWorldCenterFn;
  private readonly getBodyWorldPointFn: GetBodyWorldPointFn;
  private readonly getBodyLocalPointVelocityFn: GetBodyLocalPointVelocityFn;
  private readonly getBodyWorldPointVelocityFn: GetBodyWorldPointVelocityFn;
  private readonly createPrismaticJointFn: CreatePrismaticJointFn;
  private readonly createWeldJointFn: CreateWeldJointFn;
  private readonly worldExplodeFn: WorldExplodeFn;
  private readonly getShapeBodyHandleFn: GetShapeBodyHandleFn;
  private readonly setDensityFn: ShapeSetDensityFn;
  private readonly setFrictionFn: ShapeSetFrictionFn;
  private readonly setRestitutionFn: ShapeSetRestitutionFn;
  private readonly b3wSinFn: (radians: number) => number;
  private readonly b3wCosFn: (radians: number) => number;
  private readonly makeQuatFromAxisAngleFn: MakeQuatFromAxisAngleFn;
  private readonly transformPtr: number;
  private readonly pointPtr: number;
  private readonly massDataPtr: number;
  private readonly profilePtr: number;
  private readonly inertiaPtr: number;
  private readonly bodyBatchBuffers = new Map<number, BodyBatchBuffers>();

  constructor(module: CModule) {
    this.module = module;
    this.createWorldFn = module.cwrap("b3wCreateWorld", "number", ["number", "number", "number", "number"]);
    this.createBodyFn = module.cwrap("b3wCreateBody", "number", ["number", "number", "number", "number", "number", "number", "number"]);
    this.destroyWorldFn = module.cwrap("b3wDestroyWorld", null, ["number"]);
    this.createBoxFn = module.cwrap("b3wCreateBox", "number", ["number", "number", "number", "number", "number", "number", "number", "number", "number"]);
    this.createSphereFn = module.cwrap("b3wCreateSphere", "number", ["number", "number", "number", "number", "number", "number", "number", "number", "number"]);
    this.createHullShapeFn = module.cwrap("b3wCreateHullShape", "number", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
    this.createTransformedHullShapeFn = module.cwrap("b3wCreateTransformedHullShape", "number", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
    this.createSphereShapeFn = module.cwrap("b3wCreateSphereShape", "number", ["number","number","number","number","number","number","number","number","number"]);
    this.createCapsuleShapeFn = module.cwrap("b3wCreateCapsuleShape", "number", ["number","number","number","number","number","number","number","number","number","number","number","number","number"]);
    this.createShapeFromHullFn = module.cwrap("b3wCreateShapeFromHull", "number", ["number","number","number","number","number","number","number"]);
    this.createCylinderFn = module.cwrap("b3wCreateCylinder", "number", ["number","number","number","number"]);
    this.createHullFromPointsFn = module.cwrap("b3wCreateHullFromPoints", "number", ["number","number"]);
    this.destroyHullFn = module.cwrap("b3wDestroyHull", null, ["number"]);
    this.createCompoundFn = module.cwrap("b3wCreateCompound", "number", ["number","number","number","number","number","number","number","number"]);
    this.createCompoundFromHullsFn = module.cwrap("b3wCreateCompoundFromHulls", "number", ["number","number","number"]);
    this.createCompoundFromSpheresFn = module.cwrap("b3wCreateCompoundFromSpheres", "number", ["number","number","number"]);
    this.destroyCompoundFn = module.cwrap("b3wDestroyCompound", null, ["number"]);
    this.getCompoundTreeHeightFn = module.cwrap("b3wGetCompoundTreeHeight", "number", ["number"]);
    this.createCompoundShapeFn = module.cwrap("b3wCreateCompoundShape", "number", ["number","number","number"]);
    this.destroyBodyFn = module.cwrap("b3wDestroyBody", null, ["number"]);
    this.destroyJointFn = module.cwrap("b3wDestroyJoint", null, ["number"]);
    this.setBodyTransformFn = module.cwrap("b3wSetBodyTransform", null, ["number","number","number","number","number","number","number","number"]);
    this.setBodyLinearVelocityFn = module.cwrap("b3wSetBodyLinearVelocity", null, ["number","number","number","number"]);
    this.setBodyAngularVelocityFn = module.cwrap("b3wSetBodyAngularVelocity", null, ["number","number","number","number"]);
    this.getBodyLinearVelocityFn = module.cwrap("b3wGetBodyLinearVelocity", null, ["number", "number"]);
    this.getBodyAngularVelocityFn = module.cwrap("b3wGetBodyAngularVelocity", null, ["number", "number"]);
    this.setBodyAwakeFn = module.cwrap("b3wSetBodyAwake", null, ["number","number"]);
    this.setBodyDampingFn = module.cwrap("b3wSetBodyDamping", null, ["number","number","number"]);
    this.getBodyLocalPointFn = module.cwrap("b3wGetBodyLocalPoint", null, ["number","number","number","number","number"]);
    this.createMotorJointFn = module.cwrap("b3wCreateMotorJoint", "number", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
    this.createFilterJointFn = module.cwrap("b3wCreateFilterJoint", "number", ["number","number","number"]);
    this.createRevoluteJointFn = module.cwrap("b3wCreateRevoluteJoint", "number", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
    this.createSphericalJointFn = module.cwrap("b3wCreateSphericalJoint", "number", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
    this.createHumanFn = module.cwrap("b3wCreateHuman", "number", ["number","number","number","number","number","number","number","number","number"]);
    this.getHumanBoneBodyFn = module.cwrap("b3wGetHumanBoneBody", "number", ["number","number"]);
    this.getHumanBoneCountFn = module.cwrap("b3wGetHumanBoneCount", "number", []);
    this.humanSetVelocityFn = module.cwrap("b3wHumanSetVelocity", null, ["number","number","number","number"]);
    this.humanSetBulletFn = module.cwrap("b3wHumanSetBullet", null, ["number","number"]);
    this.humanSetJointFrictionTorqueFn = module.cwrap("b3wHumanSetJointFrictionTorque", null, ["number","number"]);
    this.humanSetJointSpringHertzFn = module.cwrap("b3wHumanSetJointSpringHertz", null, ["number","number"]);
    this.humanSetJointDampingRatioFn = module.cwrap("b3wHumanSetJointDampingRatio", null, ["number","number"]);
    this.enableWorldSleepFn = module.cwrap("b3wEnableSleeping", null, ["number", "number"]);
    this.enableWorldContinuousFn = module.cwrap("b3wEnableContinuous", null, ["number", "number"]);
    this.enableWorldWarmStartingFn = module.cwrap("b3wEnableWarmStarting", null, ["number", "number"]);
    this.setWorldContactTuningFn = module.cwrap("b3wSetContactTuning", null, ["number", "number", "number", "number"]);
    this.setWorldContactRecycleDistanceFn = module.cwrap("b3wSetContactRecycleDistance", null, ["number", "number"]);
    this.setWorldWorkerCountFn = module.cwrap("b3wSetWorkerCount", null, ["number", "number"]);
    this.getWorldCountersFn = module.cwrap("b3wGetWorldCounters", null, ["number", "number"]);
    this.getWorldProfileFn = module.cwrap("b3wGetWorldProfile", null, ["number", "number"]);
    this.getWorldAwakeBodyCountFn = module.cwrap("b3wGetWorldAwakeBodyCount", "number", ["number"]);
    this.checkThreadingSupportFn = module.cwrap("b3wCheckThreadingSupport", "number", []);
    this.getWorldWorkerCountFn = module.cwrap("b3wGetWorldWorkerCount", "number", ["number"]);
    this.writeBodyTransformsFn = module.cwrap("b3wWriteBodyTransforms", null, ["number", "number", "number", "number", "number", "number"]);
    this.writeBodyTransformsLightFn = module.cwrap("b3wWriteBodyTransformsLight", null, ["number", "number", "number", "number", "number", "number"]);
    this.rayCastClosestFn = module.cwrap("b3wRayCastClosest", null, ["number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number"]);
    this.bodyEnableFn = module.cwrap("b3wBodyEnable", null, ["number"]);
    this.bodyDisableFn = module.cwrap("b3wBodyDisable", null, ["number"]);
    this.bodyIsEnabledFn = module.cwrap("b3wBodyIsEnabled", "number", ["number"]);
    this.getBodyMassFn = module.cwrap("b3wGetBodyMass", "number", ["number"]);
    this.getBodyLocalRotationalInertiaFn = module.cwrap("b3wGetBodyLocalRotationalInertia", null, ["number", "number"]);
    this.getBodyWorldCenterFn = module.cwrap("b3wGetBodyWorldCenter", null, ["number", "number"]);
    this.getBodyWorldPointFn = module.cwrap("b3wGetBodyWorldPoint", null, ["number", "number", "number", "number", "number"]);
    this.getBodyLocalPointVelocityFn = module.cwrap("b3wGetBodyLocalPointVelocity", null, ["number", "number", "number", "number", "number"]);
    this.getBodyWorldPointVelocityFn = module.cwrap("b3wGetBodyWorldPointVelocity", null, ["number", "number", "number", "number", "number"]);
    this.createPrismaticJointFn = module.cwrap("b3wCreatePrismaticJoint", "number", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
    this.createWeldJointFn = module.cwrap("b3wCreateWeldJoint", "number", ["number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number","number"]);
    this.worldExplodeFn = module.cwrap("b3wWorldExplode", null, ["number", "number", "number", "number", "number", "number", "number", "number"]);
    this.getShapeBodyHandleFn = module.cwrap("b3wGetShapeBodyHandle", "number", ["number"]);
    this.stepFn = module.cwrap("b3wStep", null, ["number", "number", "number"]);
    this.getBodyTransformFn = module.cwrap("b3wGetBodyTransform", null, ["number", "number"]);
    this.setDensityFn = module.cwrap("b3wShapeSetDensity", null, ["number", "number", "number"]);
    this.setFrictionFn = module.cwrap("b3wShapeSetFriction", null, ["number", "number"]);
    this.setRestitutionFn = module.cwrap("b3wShapeSetRestitution", null, ["number", "number"]);
    this.setSurfaceMaterialFn = module.cwrap("b3wShapeSetSurfaceMaterial", null, ["number", "number", "number", "number"]);
    this.setFilterFn = module.cwrap("b3wShapeSetFilter", null, ["number", "number", "number", "number", "number"]);
    this.enableShapeSensorEventsFn = module.cwrap("b3wShapeEnableSensorEvents", null, ["number", "number"]);
    this.enableShapeContactEventsFn = module.cwrap("b3wShapeEnableContactEvents", null, ["number", "number"]);
    this.enableShapePreSolveEventsFn = module.cwrap("b3wShapeEnablePreSolveEvents", null, ["number", "number"]);
    this.enableShapeHitEventsFn = module.cwrap("b3wShapeEnableHitEvents", null, ["number", "number"]);
    this.setShapeSphereFn = module.cwrap("b3wShapeSetSphere", null, ["number", "number", "number", "number", "number"]);
    this.setShapeCapsuleFn = module.cwrap("b3wShapeSetCapsule", null, ["number", "number", "number", "number", "number", "number", "number", "number"]);
    this.applyShapeWindFn = module.cwrap("b3wShapeApplyWind", null, ["number", "number", "number", "number", "number", "number", "number", "number"]);
    this.bodyIsAwakeFn = module.cwrap("b3wBodyIsAwake", "number", ["number"]);
    this.getBodyDebugColorFn = module.cwrap("b3wGetBodyDebugColor", "number", ["number"]);
    this.getBodyTypeFn = module.cwrap("b3wGetBodyType", "number", ["number"]);
    this.setBodyTypeFn = module.cwrap("b3wSetBodyType", null, ["number", "number"]);
    this.setBodyNameFn = module.cwrap("b3wSetBodyName", null, ["number", "number"]);
    this.setBodyGravityScaleFn = module.cwrap("b3wSetBodyGravityScale", null, ["number", "number"]);
    this.setBodySleepThresholdFn = module.cwrap("b3wSetBodySleepThreshold", null, ["number", "number"]);
    this.enableBodySleepFn = module.cwrap("b3wEnableBodySleep", null, ["number", "number"]);
    this.setBodyBulletFn = module.cwrap("b3wSetBodyBullet", null, ["number", "number"]);
    this.enableBodyContactRecyclingFn = module.cwrap("b3wEnableBodyContactRecycling", null, ["number", "number"]);
    this.enableBodyHitEventsFn = module.cwrap("b3wEnableBodyHitEvents", null, ["number", "number"]);
    this.setBodyMotionLocksFn = module.cwrap("b3wSetBodyMotionLocks", null, ["number", "number", "number", "number", "number", "number", "number"]);
    this.setBodyMassDataFn = module.cwrap("b3wSetBodyMassData", null, ["number", "number", "number", "number", "number", "number"]);
    this.getBodyMassDataFn = module.cwrap("b3wGetBodyMassData", null, ["number", "number"]);
    this.applyBodyMassFromShapesFn = module.cwrap("b3wApplyBodyMassFromShapes", null, ["number"]);
    this.setBodyTargetTransformFn = module.cwrap("b3wSetBodyTargetTransform", null, ["number", "number", "number", "number", "number", "number", "number", "number", "number", "number"]);
    this.applyLinearImpulseFn = module.cwrap("b3wApplyLinearImpulse", null, ["number", "number", "number", "number", "number", "number", "number", "number"]);
    this.applyLinearImpulseToCenterFn = module.cwrap("b3wApplyLinearImpulseToCenter", null, ["number", "number", "number", "number", "number"]);
    this.b3wSinFn = module.cwrap("b3wSin", "number", ["number"]);
    this.b3wCosFn = module.cwrap("b3wCos", "number", ["number"]);
    this.makeQuatFromAxisAngleFn = module.cwrap("b3wMakeQuatFromAxisAngle", null, ["number", "number", "number", "number", "number"]);
    this.transformPtr = module._malloc(7 * 4);
    this.pointPtr = module._malloc(3 * 4);
    this.massDataPtr = module._malloc(2 * 4);
    this.profilePtr = module._malloc(23 * 4);
    this.inertiaPtr = module._malloc(9 * 4);
  }

  createWorld(options: WorldOptions = {}): PhysicsWorld { const gravity = options.gravity ?? vec3(0, -9.81, 0); const workerCount = options.workerCount ?? 4; return new PhysicsWorld(this, this.createWorldFn(gravity[0], gravity[1], gravity[2], workerCount)); }
  destroy(): void { this.module._free(this.transformPtr); this.module._free(this.pointPtr); this.module._free(this.massDataPtr); this.module._free(this.profilePtr); this.module._free(this.inertiaPtr); }

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

  getMemoryView(): RuntimeMemoryView32 {
    return { heapF32: this.module.HEAPF32, heapU8: this.module.HEAPU8, heap32: this.module.HEAP32 };
  }

  writeBodyHandles(buffers: BodyBatchBuffers, bodyHandles: readonly number[]): void {
    const view = new Int32Array(this.module.HEAP32.buffer, buffers.bodyHandlesPtr, bodyHandles.length);
    for (let i = 0; i < bodyHandles.length; i++) view[i] = bodyHandles[i];
  }

  private applyBodyDef(bodyHandle: number, def: BodyDef): void {
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

  private applyShapeDef(shapeHandle: number, def: ShapeDef): void {
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
    if (def.categoryBits !== undefined || def.maskBits !== undefined || def.groupIndex !== undefined) {
      this.setShapeFilter(shapeHandle, def.categoryBits ?? U64_MAX, def.maskBits ?? U64_MAX, def.groupIndex ?? 0, false);
    }
  }

  createBody(worldHandle: number, def: BodyDef = {}): number {
    const p = def.position ?? vec3();
    const bodyHandle = this.createBodyFn(worldHandle, def.type ?? BodyType.Static, p[0], p[1], p[2], defaults(def.enableSleep, true) ? 1 : 0, defaults(def.isAwake, true) ? 1 : 0);
    if (bodyHandle) this.applyBodyDef(bodyHandle, def);
    return bodyHandle;
  }

  createBox(worldHandle: number, options: BoxOptions): number {
    const s = options.size;
    const p = options.position ?? vec3();
    const bodyHandle = this.createBoxFn(worldHandle, p[0], p[1], p[2], s[0], s[1], s[2], options.static ? 1 : 0, options.density ?? 1);
    if (bodyHandle && (options.friction !== undefined || options.restitution !== undefined || options.rollingResistance !== undefined || options.isSensor || options.enableContactEvents || options.enableHitEvents)) {
      const shape = { bodyHandle, shapeHandle: bodyHandle } as ShapeHandle; // createBox returns body handle, shape is implicit
      if (options.friction !== undefined || options.restitution !== undefined || options.rollingResistance !== undefined)
        this.setShapeSurfaceMaterial(shape, { friction: options.friction, restitution: options.restitution, rollingResistance: options.rollingResistance });
    }
    return bodyHandle;
  }

  createSphere(worldHandle: number, options: SphereOptions): number {
    const p = options.position ?? vec3();
    const v = options.velocity ?? vec3();
    const bodyHandle = this.createSphereFn(worldHandle, p[0], p[1], p[2], options.radius, v[0], v[1], v[2], options.density ?? 1);
    if (bodyHandle) {
      if (options.isBullet) this.setBodyBullet(bodyHandle, true);
      if (options.friction !== undefined || options.restitution !== undefined || options.rollingResistance !== undefined)
        this.setShapeSurfaceMaterial(bodyHandle as any, { friction: options.friction, restitution: options.restitution, rollingResistance: options.rollingResistance });
    }
    return bodyHandle;
  }

  createSphereShape(bodyHandle: number, center: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = this.createSphereShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, center[0], center[1], center[2], radius);
    const shape = { bodyHandle, shapeHandle };
    this.applyShapeDef(shapeHandle, def);
    return shape;
  }

  createCapsuleShape(bodyHandle: number, center1: Vec3, center2: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = this.createCapsuleShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, center1[0], center1[1], center1[2], center2[0], center2[1], center2[2], radius);
    const shape = { bodyHandle, shapeHandle };
    this.applyShapeDef(shapeHandle, def);
    return shape;
  }

  createHullShape(bodyHandle: number, halfWidths: Vec3, def: ShapeDef = {}): ShapeHandle {
    const shapeHandle = this.createHullShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, def.updateBodyMass === false ? 0 : 1, 0, 0, 0, 0, 0, 0, 1, halfWidths[0], halfWidths[1], halfWidths[2]);
    const shape = { bodyHandle, shapeHandle };
    this.applyShapeDef(shapeHandle, def);
    return shape;
  }

  createTransformedHullShape(bodyHandle: number, halfWidths: Vec3, transform: { position?: Vec3; rotation?: Quat } = {}, scale: Vec3 = [1,1,1], def: ShapeDef = {}): ShapeHandle {
    const pos = transform.position ?? vec3();
    const rot = transform.rotation ?? [0,0,0,1];
    const shapeHandle = this.createTransformedHullShapeFn(bodyHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, pos[0], pos[1], pos[2], rot[0], rot[1], rot[2], rot[3], halfWidths[0], halfWidths[1], halfWidths[2], scale[0], scale[1], scale[2]);
    const shape = { bodyHandle, shapeHandle };
    this.applyShapeDef(shapeHandle, def);
    return shape;
  }

  createShapeFromHull(bodyHandle: number, hullHandle: number, def: ShapeDef = {}): number {
    const shapeHandle = this.createShapeFromHullFn(bodyHandle, hullHandle, def.density ?? 1000, def.friction ?? 0.6, def.restitution ?? 0, def.rollingResistance ?? 0, def.updateBodyMass === false ? 0 : 1);
    this.applyShapeDef(shapeHandle, def);
    return shapeHandle;
  }

  createCylinder(height: number, radius: number, yOffset = 0, sides = 12): number { return this.createCylinderFn(height, radius, yOffset, sides); }
  createHullFromPoints(points: number[]): number {
    const ptr = this.module._malloc(points.length * 4);
    const heap = this.module.HEAPF32;
    const base = ptr >> 2;
    for (let i = 0; i < points.length; i++) heap[base + i] = points[i];
    const hullHandle = this.createHullFromPointsFn(points.length / 3, ptr);
    this.module._free(ptr);
    return hullHandle;
  }
  destroyHull(hullHandle: number): void { this.destroyHullFn(hullHandle); }
  /** Match Box3D's b3Sin deterministically using Bhāskara I approximation. */
  b3wSin(radians: number): number { return this.b3wSinFn(radians); }
  /** Match Box3D's b3Cos deterministically using Bhāskara I approximation. */
  b3wCos(radians: number): number { return this.b3wCosFn(radians); }
  makeQuatFromAxisAngle(axis: Vec3, radians: number): Quat {
    this.makeQuatFromAxisAngleFn(axis[0], axis[1], axis[2], radians, this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2], heap[base + 3]];
  }
  createCompound(capsules: number, hulls: number, meshes: number, spheres: number): number { return this.createCompoundFn(capsules, hulls, meshes, spheres, 0, 0, 0, 0); }
  createCompoundFromHulls(entries: CompoundHullEntry[]): number {
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
    return result;
  }
  createCompoundFromSpheres(entries: CompoundSphereEntry[]): number {
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
    return result;
  }
  destroyCompound(compoundHandle: number): void { this.destroyCompoundFn(compoundHandle); }
  getCompoundTreeHeight(compoundHandle: number): number { return this.getCompoundTreeHeightFn(compoundHandle); }
  createCompoundShape(bodyHandle: number, compoundHandle: number, density = 1): number { return this.createCompoundShapeFn(bodyHandle, compoundHandle, density); }
  destroyBody(bodyHandle: number): void { this.destroyBodyFn(bodyHandle); }
  destroyJoint(jointHandle: number): void { this.destroyJointFn(jointHandle); }
  setBodyTransform(bodyHandle: number, position: Vec3, rotation: Quat = [0,0,0,1]): void { this.setBodyTransformFn(bodyHandle, position[0], position[1], position[2], rotation[0], rotation[1], rotation[2], rotation[3]); }
  setBodyLinearVelocity(bodyHandle: number, velocity: Vec3): void { this.setBodyLinearVelocityFn(bodyHandle, velocity[0], velocity[1], velocity[2]); }
  setBodyAngularVelocity(bodyHandle: number, velocity: Vec3): void { this.setBodyAngularVelocityFn(bodyHandle, velocity[0], velocity[1], velocity[2]); }
  getBodyLinearVelocity(bodyHandle: number): Vec3 { this.getBodyLinearVelocityFn(bodyHandle, this.pointPtr); const heap = this.module.HEAPF32; const base = this.pointPtr >> 2; return [heap[base + 0], heap[base + 1], heap[base + 2]]; }
  getBodyAngularVelocity(bodyHandle: number): Vec3 { this.getBodyAngularVelocityFn(bodyHandle, this.pointPtr); const heap = this.module.HEAPF32; const base = this.pointPtr >> 2; return [heap[base + 0], heap[base + 1], heap[base + 2]]; }
  bodyIsAwake(bodyHandle: number): boolean { return this.bodyIsAwakeFn(bodyHandle) !== 0; }
  getBodyDebugColor(bodyHandle: number): number { return this.getBodyDebugColorFn(bodyHandle); }
  getBodyType(bodyHandle: number): BodyType { return this.getBodyTypeFn(bodyHandle) as BodyType; }
  setBodyAwake(bodyHandle: number, awake: boolean): void { this.setBodyAwakeFn(bodyHandle, awake ? 1 : 0); }
  setBodyDamping(bodyHandle: number, linearDamping: number, angularDamping: number): void { this.setBodyDampingFn(bodyHandle, linearDamping, angularDamping); }
  getBodyLocalPoint(bodyHandle: number, worldPoint: Vec3): Vec3 { this.getBodyLocalPointFn(bodyHandle, worldPoint[0], worldPoint[1], worldPoint[2], this.pointPtr); const heap = this.module.HEAPF32; const base = this.pointPtr >> 2; return [heap[base + 0], heap[base + 1], heap[base + 2]]; }
  createMotorJoint(worldHandle: number, bodyAHandle: number, bodyBHandle: number, options: MotorJointOptions = {}): number { const a = options.localFrameA ?? vec3(); const b = options.localFrameB ?? vec3(); const lv = options.linearVelocity ?? vec3(); const av = options.angularVelocity ?? vec3(); return this.createMotorJointFn(worldHandle, bodyAHandle, bodyBHandle, a[0], a[1], a[2], b[0], b[1], b[2], lv[0], lv[1], lv[2], options.maxVelocityForce ?? 0, av[0], av[1], av[2], options.maxVelocityTorque ?? 0, options.linearHertz ?? 0, options.linearDampingRatio ?? 0, options.maxSpringForce ?? 0, options.angularHertz ?? 0, options.angularDampingRatio ?? 0, options.maxSpringTorque ?? 0); }
  createFilterJoint(worldHandle: number, bodyAHandle: number, bodyBHandle: number): number { return this.createFilterJointFn(worldHandle, bodyAHandle, bodyBHandle); }
  createRevoluteJoint(worldHandle: number, bodyAHandle: number, bodyBHandle: number, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; targetAngle?: number; enableSpring?: boolean; hertz?: number; dampingRatio?: number; enableLimit?: boolean; lowerAngle?: number; upperAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorSpeed?: number } = {}): number { const a = options.localFrameA?.position ?? vec3(); const aq = options.localFrameA?.rotation ?? [0, 0, 0, 1]; const b = options.localFrameB?.position ?? vec3(); const bq = options.localFrameB?.rotation ?? [0, 0, 0, 1]; return this.createRevoluteJointFn(worldHandle, bodyAHandle, bodyBHandle, a[0], a[1], a[2], aq[0], aq[1], aq[2], aq[3], b[0], b[1], b[2], bq[0], bq[1], bq[2], bq[3], options.targetAngle ?? 0, options.enableSpring ? 1 : 0, options.hertz ?? 0, options.dampingRatio ?? 0, options.enableLimit ? 1 : 0, options.lowerAngle ?? 0, options.upperAngle ?? 0, options.enableMotor ? 1 : 0, options.maxMotorTorque ?? 0, options.motorSpeed ?? 0); }
  createSphericalJoint(worldHandle: number, bodyAHandle: number, bodyBHandle: number, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetRotation?: Quat; enableConeLimit?: boolean; coneAngle?: number; enableTwistLimit?: boolean; lowerTwistAngle?: number; upperTwistAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorVelocity?: Vec3 } = {}): number { const a = options.localFrameA?.position ?? vec3(); const aq = options.localFrameA?.rotation ?? [0, 0, 0, 1]; const b = options.localFrameB?.position ?? vec3(); const bq = options.localFrameB?.rotation ?? [0, 0, 0, 1]; const tq = options.targetRotation ?? [0, 0, 0, 1]; const mv = options.motorVelocity ?? vec3(); return this.createSphericalJointFn(worldHandle, bodyAHandle, bodyBHandle, a[0], a[1], a[2], aq[0], aq[1], aq[2], aq[3], b[0], b[1], b[2], bq[0], bq[1], bq[2], bq[3], options.enableSpring ? 1 : 0, options.hertz ?? 0, options.dampingRatio ?? 0, tq[0], tq[1], tq[2], tq[3], options.enableConeLimit ? 1 : 0, options.coneAngle ?? 0, options.enableTwistLimit ? 1 : 0, options.lowerTwistAngle ?? 0, options.upperTwistAngle ?? 0, options.enableMotor ? 1 : 0, options.maxMotorTorque ?? 0, mv[0], mv[1], mv[2]); }
  createHuman(worldHandle: number, position: Vec3, options: { frictionTorque?: number; hertz?: number; dampingRatio?: number; groupIndex?: number; colorize?: boolean } = {}): number { return this.createHumanFn(worldHandle, position[0], position[1], position[2], options.frictionTorque ?? 1, options.hertz ?? 1, options.dampingRatio ?? 1, options.groupIndex ?? 0, options.colorize ?? true ? 1 : 0); }
  getHumanBoneBody(humanHandle: number, boneIndex: number): number { return this.getHumanBoneBodyFn(humanHandle, boneIndex); }
  getHumanBoneCount(): number { return this.getHumanBoneCountFn(); }
  setHumanVelocity(humanHandle: number, velocity: Vec3): void { this.humanSetVelocityFn(humanHandle, velocity[0], velocity[1], velocity[2]); }
  setHumanBullet(humanHandle: number, flag: boolean): void { this.humanSetBulletFn(humanHandle, flag ? 1 : 0); }
  setHumanJointFrictionTorque(humanHandle: number, torque: number): void { this.humanSetJointFrictionTorqueFn(humanHandle, torque); }
  setHumanJointSpringHertz(humanHandle: number, hertz: number): void { this.humanSetJointSpringHertzFn(humanHandle, hertz); }
  setHumanJointDampingRatio(humanHandle: number, dampingRatio: number): void { this.humanSetJointDampingRatioFn(humanHandle, dampingRatio); }
  readBodyTransform(bodyHandle: number): BodyTransform { this.getBodyTransformFn(bodyHandle, this.transformPtr); const heap = this.module.HEAPF32; const base = this.transformPtr >> 2; return { position: [heap[base + 0], heap[base + 1], heap[base + 2]], rotation: [heap[base + 3], heap[base + 4], heap[base + 5], heap[base + 6]] }; }
  getWorldCounters(worldHandle: number): WorldCounters { const ptr = this.module._malloc(7 * 4); this.getWorldCountersFn(worldHandle, ptr); const heap32 = new Int32Array(this.module.HEAPF32.buffer); const base = ptr >> 2; const counters = { bodyCount: heap32[base + 0], shapeCount: heap32[base + 1], contactCount: heap32[base + 2], jointCount: heap32[base + 3], islandCount: heap32[base + 4], staticTreeHeight: heap32[base + 5], treeHeight: heap32[base + 6] }; this.module._free(ptr); return counters; }
  getWorldAwakeBodyCount(worldHandle: number): number { return this.getWorldAwakeBodyCountFn(worldHandle); }
  getWorldProfile(worldHandle: number): WorldProfile { this.getWorldProfileFn(worldHandle, this.profilePtr); const heap = this.module.HEAPF32; const base = this.profilePtr >> 2; return { step: heap[base + 0], pairs: heap[base + 1], collide: heap[base + 2], solve: heap[base + 3], solverSetup: heap[base + 4], constraints: heap[base + 5], prepareConstraints: heap[base + 6], integrateVelocities: heap[base + 7], warmStart: heap[base + 8], solveImpulses: heap[base + 9], integratePositions: heap[base + 10], relaxImpulses: heap[base + 11], applyRestitution: heap[base + 12], storeImpulses: heap[base + 13], splitIslands: heap[base + 14], transforms: heap[base + 15], sensorHits: heap[base + 16], jointEvents: heap[base + 17], hitEvents: heap[base + 18], refit: heap[base + 19], bullets: heap[base + 20], sleepIslands: heap[base + 21], sensors: heap[base + 22] }; }
  checkThreadingSupport(): number { return this.checkThreadingSupportFn(); }
  getWorldWorkerCount(worldHandle: number): number { return this.getWorldWorkerCountFn(worldHandle); }
  enableWorldSleeping(worldHandle: number, flag: boolean): void { this.enableWorldSleepFn(worldHandle, flag ? 1 : 0); }
  enableWorldContinuous(worldHandle: number, flag: boolean): void { this.enableWorldContinuousFn(worldHandle, flag ? 1 : 0); }
  enableWorldWarmStarting(worldHandle: number, flag: boolean): void { this.enableWorldWarmStartingFn(worldHandle, flag ? 1 : 0); }
  setWorldContactTuning(worldHandle: number, hertz: number, dampingRatio: number, contactSpeed: number): void { this.setWorldContactTuningFn(worldHandle, hertz, dampingRatio, contactSpeed); }
  setWorldContactRecycleDistance(worldHandle: number, distance: number): void { this.setWorldContactRecycleDistanceFn(worldHandle, distance); }
  setWorldWorkerCount(worldHandle: number, count: number): void { this.setWorldWorkerCountFn(worldHandle, count); }

  rayCastClosest(worldHandle: number, origin: Vec3, translation: Vec3, categoryBits = U64_MAX, maskBits = U64_MAX): { shapeHandle: number; bodyHandle: number; point: Vec3; normal: Vec3; fraction: number } | null {
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
    const bodyHandle = this.getShapeBodyHandleFn(shapeHandle);
    const result = { shapeHandle, bodyHandle, point: [heap[pBase + 0], heap[pBase + 1], heap[pBase + 2]] as Vec3, normal: [heap[nBase + 0], heap[nBase + 1], heap[nBase + 2]] as Vec3, fraction };
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

  step(worldHandle: number, dt: number, substeps: number): void { this.stepFn(worldHandle, dt, substeps); }
  destroyWorld(worldHandle: number): void { this.destroyWorldFn(worldHandle); }
  setShapeDensity(shapeHandle: number | ShapeHandle, density: number, updateBodyMass = true): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setDensityFn(handle, density, updateBodyMass ? 1 : 0); }
  setShapeFriction(shapeHandle: number | ShapeHandle, friction: number): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setFrictionFn(handle, friction); }
  setShapeRestitution(shapeHandle: number | ShapeHandle, restitution: number): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setRestitutionFn(handle, restitution); }
  setShapeSurfaceMaterial(shapeHandle: number | ShapeHandle, material: SurfaceMaterial = {}): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setSurfaceMaterialFn(handle, material.friction ?? 0.6, material.restitution ?? 0, material.rollingResistance ?? 0); }
  setShapeFilter(shapeHandle: number | ShapeHandle, categoryBits: number, maskBits: number, groupIndex = 0, invokeContacts = false): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setFilterFn(handle, categoryBits, maskBits, groupIndex, invokeContacts ? 1 : 0); }
  enableShapeSensorEvents(shapeHandle: number | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapeSensorEventsFn(handle, flag ? 1 : 0); }
  enableShapeContactEvents(shapeHandle: number | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapeContactEventsFn(handle, flag ? 1 : 0); }
  enableShapePreSolveEvents(shapeHandle: number | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapePreSolveEventsFn(handle, flag ? 1 : 0); }
  enableShapeHitEvents(shapeHandle: number | ShapeHandle, flag: boolean): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.enableShapeHitEventsFn(handle, flag ? 1 : 0); }
  setShapeSphere(shapeHandle: number | ShapeHandle, position: Vec3, radius: number): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setShapeSphereFn(handle, position[0], position[1], position[2], radius); }
  setShapeCapsule(shapeHandle: number | ShapeHandle, a: Vec3, b: Vec3, radius: number): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.setShapeCapsuleFn(handle, a[0], a[1], a[2], b[0], b[1], b[2], radius); }
  applyShapeWind(shapeHandle: number | ShapeHandle, wind: Vec3, drag: number, lift: number, maxSpeed: number, wake = true): void { const handle = typeof shapeHandle === "number" ? shapeHandle : shapeHandle.shapeHandle; this.applyShapeWindFn(handle, wind[0], wind[1], wind[2], drag, lift, maxSpeed, wake ? 1 : 0); }
  setBodyType(bodyHandle: number, type: BodyType): void { this.setBodyTypeFn(bodyHandle, type); }
  setBodyName(bodyHandle: number, name: string): void { const ptr = this.module._malloc(name.length + 1); const heap8 = new Uint8Array(this.module.HEAPU8.buffer); for (let i = 0; i < name.length; i++) heap8[ptr + i] = name.charCodeAt(i); heap8[ptr + name.length] = 0; this.setBodyNameFn(bodyHandle, ptr); this.module._free(ptr); }
  setBodyGravityScale(bodyHandle: number, scale: number): void { this.setBodyGravityScaleFn(bodyHandle, scale); }
  setBodySleepThreshold(bodyHandle: number, threshold: number): void { this.setBodySleepThresholdFn(bodyHandle, threshold); }
  enableBodySleep(bodyHandle: number, enable: boolean): void { this.enableBodySleepFn(bodyHandle, enable ? 1 : 0); }
  setBodyBullet(bodyHandle: number, flag: boolean): void { this.setBodyBulletFn(bodyHandle, flag ? 1 : 0); }
  enableBodyContactRecycling(bodyHandle: number, flag: boolean): void { this.enableBodyContactRecyclingFn(bodyHandle, flag ? 1 : 0); }
  enableBodyHitEvents(bodyHandle: number, flag: boolean): void { this.enableBodyHitEventsFn(bodyHandle, flag ? 1 : 0); }
  setBodyMotionLocks(bodyHandle: number, locks: { lockX?: boolean; lockY?: boolean; lockRotationX?: boolean; lockRotationY?: boolean; lockRotationZ?: boolean; lockLinearZ?: boolean } = {}): void { this.setBodyMotionLocksFn(bodyHandle, locks.lockX ? 1 : 0, locks.lockY ? 1 : 0, locks.lockLinearZ ? 1 : 0, locks.lockRotationX ? 1 : 0, locks.lockRotationY ? 1 : 0, locks.lockRotationZ ? 1 : 0); }
  setBodyMassData(bodyHandle: number, mass: number, center: Vec3, inertia?: Mat3): void { if (inertia) { const heap = this.module.HEAPF32; const base = this.inertiaPtr >> 2; for (let i = 0; i < 9; i++) heap[base + i] = inertia[i]; this.setBodyMassDataFn(bodyHandle, mass, center[0], center[1], center[2], this.inertiaPtr); } else { this.setBodyMassDataFn(bodyHandle, mass, center[0], center[1], center[2], 0); } }
  getBodyMassData(bodyHandle: number): BodyMassData { this.getBodyMassDataFn(bodyHandle, this.massDataPtr); const heap = this.module.HEAPF32; const base = this.massDataPtr >> 2; return { mass: heap[base], inertiaTrace: heap[base + 1] }; }
  applyBodyMassFromShapes(bodyHandle: number): void { this.applyBodyMassFromShapesFn(bodyHandle); }
  setBodyTargetTransform(bodyHandle: number, position: Vec3, rotation: Quat, timeStep: number, wake = true): void { this.setBodyTargetTransformFn(bodyHandle, position[0], position[1], position[2], rotation[0], rotation[1], rotation[2], rotation[3], timeStep, wake ? 1 : 0); }
  bodyEnable(bodyHandle: number): void { this.bodyEnableFn(bodyHandle); }
  bodyDisable(bodyHandle: number): void { this.bodyDisableFn(bodyHandle); }
  bodyIsEnabled(bodyHandle: number): boolean { return this.bodyIsEnabledFn(bodyHandle) !== 0; }
  getBodyMass(bodyHandle: number): number { return this.getBodyMassFn(bodyHandle); }
  getBodyLocalRotationalInertia(bodyHandle: number): Mat3 { this.getBodyLocalRotationalInertiaFn(bodyHandle, this.inertiaPtr); const heap = this.module.HEAPF32; const base = this.inertiaPtr >> 2; return [heap[base + 0], heap[base + 1], heap[base + 2], heap[base + 3], heap[base + 4], heap[base + 5], heap[base + 6], heap[base + 7], heap[base + 8]]; }
  getBodyWorldCenter(bodyHandle: number): Vec3 { this.getBodyWorldCenterFn(bodyHandle, this.pointPtr); const heap = this.module.HEAPF32; const base = this.pointPtr >> 2; return [heap[base + 0], heap[base + 1], heap[base + 2]]; }
  getBodyWorldPoint(bodyHandle: number, localPoint: Vec3): Vec3 { this.getBodyWorldPointFn(bodyHandle, localPoint[0], localPoint[1], localPoint[2], this.pointPtr); const heap = this.module.HEAPF32; const base = this.pointPtr >> 2; return [heap[base + 0], heap[base + 1], heap[base + 2]]; }
  getBodyLocalPointVelocity(bodyHandle: number, localPoint: Vec3): Vec3 { this.getBodyLocalPointVelocityFn(bodyHandle, localPoint[0], localPoint[1], localPoint[2], this.pointPtr); const heap = this.module.HEAPF32; const base = this.pointPtr >> 2; return [heap[base + 0], heap[base + 1], heap[base + 2]]; }
  getBodyWorldPointVelocity(bodyHandle: number, worldPoint: Vec3): Vec3 { this.getBodyWorldPointVelocityFn(bodyHandle, worldPoint[0], worldPoint[1], worldPoint[2], this.pointPtr); const heap = this.module.HEAPF32; const base = this.pointPtr >> 2; return [heap[base + 0], heap[base + 1], heap[base + 2]]; }
  createPrismaticJoint(worldHandle: number, bodyAHandle: number, bodyBHandle: number, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetTranslation?: number; enableLimit?: boolean; lowerTranslation?: number; upperTranslation?: number; enableMotor?: boolean; maxMotorForce?: number; motorSpeed?: number } = {}): number { const la = options.localFrameA?.position ?? [0,0,0]; const laq = options.localFrameA?.rotation ?? [0,0,0,1]; const lb = options.localFrameB?.position ?? [0,0,0]; const lbq = options.localFrameB?.rotation ?? [0,0,0,1]; return this.createPrismaticJointFn(worldHandle, bodyAHandle, bodyBHandle, la[0], la[1], la[2], laq[0], laq[1], laq[2], laq[3], lb[0], lb[1], lb[2], lbq[0], lbq[1], lbq[2], lbq[3], options.enableSpring ? 1 : 0, options.hertz ?? 0, options.dampingRatio ?? 0, options.targetTranslation ?? 0, options.enableLimit ? 1 : 0, options.lowerTranslation ?? 0, options.upperTranslation ?? 0, options.enableMotor ? 1 : 0, options.maxMotorForce ?? 0, options.motorSpeed ?? 0); }
  createWeldJoint(worldHandle: number, bodyAHandle: number, bodyBHandle: number, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; linearHertz?: number; angularHertz?: number; linearDampingRatio?: number; angularDampingRatio?: number } = {}): number { const la = options.localFrameA?.position ?? [0,0,0]; const laq = options.localFrameA?.rotation ?? [0,0,0,1]; const lb = options.localFrameB?.position ?? [0,0,0]; const lbq = options.localFrameB?.rotation ?? [0,0,0,1]; return this.createWeldJointFn(worldHandle, bodyAHandle, bodyBHandle, la[0], la[1], la[2], laq[0], laq[1], laq[2], laq[3], lb[0], lb[1], lb[2], lbq[0], lbq[1], lbq[2], lbq[3], options.linearHertz ?? 0, options.angularHertz ?? 0, options.linearDampingRatio ?? 0, options.angularDampingRatio ?? 0); }
  worldExplode(worldHandle: number, position: Vec3, radius: number, falloff: number, impulsePerArea: number, maskBits = 0xFFFFFFFFFFFFFFFF): void { this.worldExplodeFn(worldHandle, position[0], position[1], position[2], radius, falloff, impulsePerArea, maskBits); }

  applyLinearImpulse(bodyHandle: number, impulse: Vec3, point: Vec3, wake = true): void { this.applyLinearImpulseFn(bodyHandle, impulse[0], impulse[1], impulse[2], point[0], point[1], point[2], wake ? 1 : 0); }
  applyLinearImpulseToCenter(bodyHandle: number, impulse: Vec3, wake = true): void { this.applyLinearImpulseToCenterFn(bodyHandle, impulse[0], impulse[1], impulse[2], wake ? 1 : 0); }
}

export class PhysicsWorld {
  constructor(private readonly runtime: Box3DRuntime, public readonly handle: number) {}
  createBody(def: BodyDef = {}): number { return this.runtime.createBody(this.handle, def); }
  createBox(options: BoxOptions): number { return this.runtime.createBox(this.handle, options); }
  createSphere(options: SphereOptions): number { return this.runtime.createSphere(this.handle, options); }
  createSphereShape(bodyHandle: number, center: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle { return this.runtime.createSphereShape(bodyHandle, center, radius, def); }
  createCapsuleShape(bodyHandle: number, center1: Vec3, center2: Vec3, radius: number, def: ShapeDef = {}): ShapeHandle { return this.runtime.createCapsuleShape(bodyHandle, center1, center2, radius, def); }
  createHullShape(bodyHandle: number, halfWidths: Vec3, def: ShapeDef = {}): ShapeHandle { return this.runtime.createHullShape(bodyHandle, halfWidths, def); }
  createTransformedHullShape(bodyHandle: number, halfWidths: Vec3, transform?: { position?: Vec3; rotation?: Quat }, scale?: Vec3, def?: ShapeDef): ShapeHandle { return this.runtime.createTransformedHullShape(bodyHandle, halfWidths, transform, scale, def); }
  createShapeFromHull(bodyHandle: number, hullHandle: number, def?: ShapeDef): number { return this.runtime.createShapeFromHull(bodyHandle, hullHandle, def); }
  createCompoundShape(bodyHandle: number, compoundHandle: number, density = 1): number { return this.runtime.createCompoundShape(bodyHandle, compoundHandle, density); }
  getCompoundTreeHeight(compoundHandle: number): number { return this.runtime.getCompoundTreeHeight(compoundHandle); }
  destroyCompound(compoundHandle: number): void { this.runtime.destroyCompound(compoundHandle); }
  destroyBody(bodyHandle: number): void { this.runtime.destroyBody(bodyHandle); }
  destroyJoint(jointHandle: number): void { this.runtime.destroyJoint(jointHandle); }
  setBodyTransform(bodyHandle: number, position: Vec3, rotation: Quat = [0,0,0,1]): void { this.runtime.setBodyTransform(bodyHandle, position, rotation); }
  setBodyLinearVelocity(bodyHandle: number, velocity: Vec3): void { this.runtime.setBodyLinearVelocity(bodyHandle, velocity); }
  setBodyAngularVelocity(bodyHandle: number, velocity: Vec3): void { this.runtime.setBodyAngularVelocity(bodyHandle, velocity); }
  getBodyLinearVelocity(bodyHandle: number): Vec3 { return this.runtime.getBodyLinearVelocity(bodyHandle); }
  getBodyAngularVelocity(bodyHandle: number): Vec3 { return this.runtime.getBodyAngularVelocity(bodyHandle); }
  applyLinearImpulse(bodyHandle: number, impulse: Vec3, point: Vec3, wake = true): void { this.runtime.applyLinearImpulse(bodyHandle, impulse, point, wake); }
  applyLinearImpulseToCenter(bodyHandle: number, impulse: Vec3, wake = true): void { this.runtime.applyLinearImpulseToCenter(bodyHandle, impulse, wake); }
  bodyIsAwake(bodyHandle: number): boolean { return this.runtime.bodyIsAwake(bodyHandle); }
  getBodyDebugColor(bodyHandle: number): number { return this.runtime.getBodyDebugColor(bodyHandle); }
  getBodyType(bodyHandle: number): BodyType { return this.runtime.getBodyType(bodyHandle); }
  setBodyAwake(bodyHandle: number, awake: boolean): void { this.runtime.setBodyAwake(bodyHandle, awake); }
  setBodyDamping(bodyHandle: number, linearDamping: number, angularDamping: number): void { this.runtime.setBodyDamping(bodyHandle, linearDamping, angularDamping); }
  getBodyLocalPoint(bodyHandle: number, worldPoint: Vec3): Vec3 { return this.runtime.getBodyLocalPoint(bodyHandle, worldPoint); }
  createMotorJoint(bodyAHandle: number, bodyBHandle: number, options: MotorJointOptions = {}): number { return this.runtime.createMotorJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createFilterJoint(bodyAHandle: number, bodyBHandle: number): number { return this.runtime.createFilterJoint(this.handle, bodyAHandle, bodyBHandle); }
  createRevoluteJoint(bodyAHandle: number, bodyBHandle: number, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; targetAngle?: number; enableSpring?: boolean; hertz?: number; dampingRatio?: number; enableLimit?: boolean; lowerAngle?: number; upperAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorSpeed?: number } = {}): number { return this.runtime.createRevoluteJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createSphericalJoint(bodyAHandle: number, bodyBHandle: number, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetRotation?: Quat; enableConeLimit?: boolean; coneAngle?: number; enableTwistLimit?: boolean; lowerTwistAngle?: number; upperTwistAngle?: number; enableMotor?: boolean; maxMotorTorque?: number; motorVelocity?: Vec3 } = {}): number { return this.runtime.createSphericalJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createHuman(position: Vec3, options: { frictionTorque?: number; hertz?: number; dampingRatio?: number; groupIndex?: number; colorize?: boolean } = {}): number { return this.runtime.createHuman(this.handle, position, options); }
  getBodyTransform(bodyHandle: number): BodyTransform { return this.runtime.readBodyTransform(bodyHandle); }
  getBodyMassData(bodyHandle: number): BodyMassData { return this.runtime.getBodyMassData(bodyHandle); }
  bodyEnable(bodyHandle: number): void { this.runtime.bodyEnable(bodyHandle); }
  bodyDisable(bodyHandle: number): void { this.runtime.bodyDisable(bodyHandle); }
  bodyIsEnabled(bodyHandle: number): boolean { return this.runtime.bodyIsEnabled(bodyHandle); }
  getBodyMass(bodyHandle: number): number { return this.runtime.getBodyMass(bodyHandle); }
  getBodyLocalRotationalInertia(bodyHandle: number): Mat3 { return this.runtime.getBodyLocalRotationalInertia(bodyHandle); }
  getBodyWorldCenter(bodyHandle: number): Vec3 { return this.runtime.getBodyWorldCenter(bodyHandle); }
  getBodyWorldPoint(bodyHandle: number, localPoint: Vec3): Vec3 { return this.runtime.getBodyWorldPoint(bodyHandle, localPoint); }
  getBodyLocalPointVelocity(bodyHandle: number, localPoint: Vec3): Vec3 { return this.runtime.getBodyLocalPointVelocity(bodyHandle, localPoint); }
  getBodyWorldPointVelocity(bodyHandle: number, worldPoint: Vec3): Vec3 { return this.runtime.getBodyWorldPointVelocity(bodyHandle, worldPoint); }
  createPrismaticJoint(bodyAHandle: number, bodyBHandle: number, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; enableSpring?: boolean; hertz?: number; dampingRatio?: number; targetTranslation?: number; enableLimit?: boolean; lowerTranslation?: number; upperTranslation?: number; enableMotor?: boolean; maxMotorForce?: number; motorSpeed?: number } = {}): number { return this.runtime.createPrismaticJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  createWeldJoint(bodyAHandle: number, bodyBHandle: number, options: { localFrameA?: { position?: Vec3; rotation?: Quat }; localFrameB?: { position?: Vec3; rotation?: Quat }; linearHertz?: number; angularHertz?: number; linearDampingRatio?: number; angularDampingRatio?: number } = {}): number { return this.runtime.createWeldJoint(this.handle, bodyAHandle, bodyBHandle, options); }
  explode(position: Vec3, radius: number, falloff: number, impulsePerArea: number, maskBits = 0xFFFFFFFFFFFFFFFF): void { this.runtime.worldExplode(this.handle, position, radius, falloff, impulsePerArea, maskBits); }
  getCounters(): WorldCounters { return this.runtime.getWorldCounters(this.handle); }
  getAwakeBodyCount(): number { return this.runtime.getWorldAwakeBodyCount(this.handle); }
  getWorkerCount(): number { return this.runtime.getWorldWorkerCount(this.handle); }
  getProfile(): WorldProfile { return this.runtime.getWorldProfile(this.handle); }
  rayCastClosest(origin: Vec3, translation: Vec3, categoryBits = U64_MAX, maskBits = U64_MAX): { shapeHandle: number; bodyHandle: number; point: Vec3; normal: Vec3; fraction: number } | null { return this.runtime.rayCastClosest(this.handle, origin, translation, categoryBits, maskBits); }
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
