import {
  BodyType,
  Box3DRuntime,
  type BodyDef,
  type BodyHandle,
  type BodyMassData,
  type BodyTransform,
  type BoxOptions,
  type CompoundHandle,
  type HullHandle,
  type HumanHandle,
  type JointHandle,
  type MeshHandle,
  type Mat3,
  type MeshShapeOptions,
  type MotorJointOptions,
  type PhysicsWorld,
  type Quat,
  type ShapeDef,
  type ShapeHandle,
  type SphereOptions,
  type Vec3,
  type WorldCounters,
  type WorldOptions,
  type WorldProfile,
} from "./index";

export {
  areObjectAssertsCompiledIn,
  areObjectAssertsEnabled,
} from "./object-assert-flags";
export {
  areObjectAssertGuardsInstalled,
  setObjectAssertGuardsEnabled,
} from "./object-assert-guards";
export { formatObjectAssertBenchResult, runObjectAssertBench } from "./object-assert-bench";
export type { ObjectAssertBenchOptions, ObjectAssertBenchResult } from "./object-assert-bench";

import { objectAssertsEnabled } from "./object-assert-flags";
import { installObjectAssertGuards } from "./object-assert-guards";

export class ObjectRuntime {
  private disposed = false;

  private constructor(private readonly runtime: Box3DRuntime, private readonly ownsRuntime: boolean) {}

  static async load(options?: Parameters<typeof Box3DRuntime.load>[0]): Promise<ObjectRuntime> {
    return new ObjectRuntime(await Box3DRuntime.load(options), true);
  }

  static fromRuntime(runtime: Box3DRuntime): ObjectRuntime {
    return new ObjectRuntime(runtime, false);
  }

  get raw(): Box3DRuntime {
    return this.runtime;
  }

  get isDisposed(): boolean {
    return this.disposed;
  }

  createWorld(options?: WorldOptions): ObjectWorld {
    return new ObjectWorld(this, this.runtime.createWorld(options));
  }

  wrapWorld(world: PhysicsWorld): ObjectWorld {
    return new ObjectWorld(this, world);
  }

  createCylinder(height: number, radius: number, yOffset = 0, sides = 12): HullRef {
    return new HullRef(this, this.runtime.createCylinder(height, radius, yOffset, sides));
  }

  createHullFromPoints(points: number[]): HullRef {
    return new HullRef(this, this.runtime.createHullFromPoints(points));
  }

  wrapMesh(meshHandle: MeshHandle): MeshRef {
    return new MeshRef(this, meshHandle);
  }

  dispose(): void {
    if (this.disposed) return;
    this.disposed = true;
    if (this.ownsRuntime) this.runtime.destroy();
  }

  assertActive(): void {
    if (!objectAssertsEnabled()) return;
    if (this.disposed) throw new Error("ObjectRuntime has been disposed");
  }
}

export class ObjectWorld {
  private disposed = false;

  constructor(private readonly runtime: ObjectRuntime, private readonly world: PhysicsWorld) {}

  get raw(): PhysicsWorld {
    return this.world;
  }

  get ownerRuntime(): ObjectRuntime {
    return this.runtime;
  }

  get handle() {
    return this.world.handle;
  }

  get isDisposed(): boolean {
    return this.disposed;
  }

  createBody(def: BodyDef = {}): BodyRef {
    return new BodyRef(this, this.world.createBody(def));
  }

  createBox(options: BoxOptions): BodyRef {
    return new BodyRef(this, this.world.createBox(options));
  }

  createBoxWithShape(options: BoxOptions): { body: BodyRef; shape: ShapeRef } {
    const shape = this.world.createBoxWithShape(options);
    return { body: new BodyRef(this, shape.bodyHandle), shape: new ShapeRef(this, shape) };
  }

  createSphere(options: SphereOptions): BodyRef {
    return new BodyRef(this, this.world.createSphere(options));
  }

  createSphereWithShape(options: SphereOptions): { body: BodyRef; shape: ShapeRef } {
    const shape = this.world.createSphereWithShape(options);
    return { body: new BodyRef(this, shape.bodyHandle), shape: new ShapeRef(this, shape) };
  }

  createGridMesh(xCount: number, zCount: number, cellWidth: number, materialCount = 1, identifyEdges = true): MeshRef {
    return new MeshRef(this.runtime, this.world.createGridMesh(xCount, zCount, cellWidth, materialCount, identifyEdges));
  }

  createWaveMesh(xCount: number, zCount: number, cellWidth: number, amplitude: number, rowFrequency: number, columnFrequency: number): MeshRef {
    return new MeshRef(this.runtime, this.world.createWaveMesh(xCount, zCount, cellWidth, amplitude, rowFrequency, columnFrequency));
  }

  createBoxMesh(center: Vec3, extent: Vec3, identifyEdges = true): MeshRef {
    return new MeshRef(this.runtime, this.world.createBoxMesh(center, extent, identifyEdges));
  }

  createTorusMesh(radialResolution: number, tubularResolution: number, radius: number, thickness: number): MeshRef {
    return new MeshRef(this.runtime, this.world.createTorusMesh(radialResolution, tubularResolution, radius, thickness));
  }

  body(handle: BodyHandle): BodyRef {
    return new BodyRef(this, handle);
  }

  createMotorJoint(bodyA: BodyRef, bodyB: BodyRef, options: MotorJointOptions = {}): JointRef {
    return new JointRef(this, this.world.createMotorJoint(bodyA.handle, bodyB.handle, options));
  }

  createFilterJoint(bodyA: BodyRef, bodyB: BodyRef): JointRef {
    return new JointRef(this, this.world.createFilterJoint(bodyA.handle, bodyB.handle));
  }

  createRevoluteJoint(bodyA: BodyRef, bodyB: BodyRef, options: Parameters<PhysicsWorld["createRevoluteJoint"]>[2] = {}): JointRef {
    return new JointRef(this, this.world.createRevoluteJoint(bodyA.handle, bodyB.handle, options));
  }

  createSphericalJoint(bodyA: BodyRef, bodyB: BodyRef, options: Parameters<PhysicsWorld["createSphericalJoint"]>[2] = {}): JointRef {
    return new JointRef(this, this.world.createSphericalJoint(bodyA.handle, bodyB.handle, options));
  }

  createPrismaticJoint(bodyA: BodyRef, bodyB: BodyRef, options: Parameters<PhysicsWorld["createPrismaticJoint"]>[2] = {}): JointRef {
    return new JointRef(this, this.world.createPrismaticJoint(bodyA.handle, bodyB.handle, options));
  }

  createWeldJoint(bodyA: BodyRef, bodyB: BodyRef, options: Parameters<PhysicsWorld["createWeldJoint"]>[2] = {}): JointRef {
    return new JointRef(this, this.world.createWeldJoint(bodyA.handle, bodyB.handle, options));
  }

  createDistanceJoint(bodyA: BodyRef, bodyB: BodyRef, options: Parameters<PhysicsWorld["createDistanceJoint"]>[2] = {}): JointRef {
    return new JointRef(this, this.world.createDistanceJoint(bodyA.handle, bodyB.handle, options));
  }

  createHuman(position: Vec3, options: Parameters<PhysicsWorld["createHuman"]>[1] = {}): HumanRef {
    return new HumanRef(this, this.world.createHuman(position, options));
  }

  step(dt = 1 / 60, substeps = 4): void {
    this.world.step(dt, substeps);
  }

  enableSleeping(flag: boolean): void {
    this.world.enableSleeping(flag);
  }

  enableContinuous(flag: boolean): void {
    this.world.enableContinuous(flag);
  }

  enableWarmStarting(flag: boolean): void {
    this.world.enableWarmStarting(flag);
  }

  setContactTuning(hertz: number, dampingRatio: number, contactSpeed: number): void {
    this.world.setContactTuning(hertz, dampingRatio, contactSpeed);
  }

  setWorkerCount(count: number): void {
    this.world.setWorkerCount(count);
  }

  getCounters(): WorldCounters {
    return this.world.getCounters();
  }

  getProfile(): WorldProfile {
    return this.world.getProfile();
  }

  getAwakeBodyCount(): number {
    return this.world.getAwakeBodyCount();
  }

  dispose(): void {
    if (this.disposed) return;
    this.disposed = true;
    this.world.destroy();
  }

  assertActive(): void {
    if (!objectAssertsEnabled()) return;
    if (this.runtime.isDisposed) throw new Error("ObjectRuntime has been disposed");
    if (this.disposed) throw new Error("ObjectWorld has been disposed");
  }
}

export class BodyRef {
  private disposed = false;

  constructor(private readonly world: ObjectWorld, public readonly handle: BodyHandle) {}

  get isDisposed(): boolean {
    return this.disposed;
  }

  createHullShape(halfWidths: Vec3, def: ShapeDef = {}): ShapeRef {
    return new ShapeRef(this.world, this.world.raw.createHullShape(this.handle, halfWidths, def));
  }

  createSphereShape(center: Vec3, radius: number, def: ShapeDef = {}): ShapeRef {
    return new ShapeRef(this.world, this.world.raw.createSphereShape(this.handle, center, radius, def));
  }

  createCapsuleShape(center1: Vec3, center2: Vec3, radius: number, def: ShapeDef = {}): ShapeRef {
    return new ShapeRef(this.world, this.world.raw.createCapsuleShape(this.handle, center1, center2, radius, def));
  }

  createTransformedHullShape(halfWidths: Vec3, transform?: { position?: Vec3; rotation?: Quat }, scale?: Vec3, def?: ShapeDef): ShapeRef {
    return new ShapeRef(this.world, this.world.raw.createTransformedHullShape(this.handle, halfWidths, transform, scale, def));
  }

  createShapeFromHull(hull: HullHandle | HullRef, def: ShapeDef = {}): ShapeRef {
    const handle = hull instanceof HullRef ? hull.handle : hull;
    return new ShapeRef(this.world, { bodyHandle: this.handle, shapeHandle: this.world.raw.createShapeFromHull(this.handle, handle, def) });
  }

  createCompoundShape(compound: CompoundHandle, density = 1): ShapeRef {
    return new ShapeRef(this.world, { bodyHandle: this.handle, shapeHandle: this.world.raw.createCompoundShape(this.handle, compound, density) });
  }

  createMeshShape(mesh: MeshHandle | MeshRef, def: MeshShapeOptions = {}): ShapeRef {
    const handle = mesh instanceof MeshRef ? mesh.handle : mesh;
    return new ShapeRef(this.world, this.world.raw.createMeshShape(this.handle, handle, def));
  }

  getShapes(): ShapeRef[] {
    return this.world.raw.getBodyShapes(this.handle).map((shapeHandle) => new ShapeRef(this.world, { bodyHandle: this.handle, shapeHandle }));
  }

  getTransform(): BodyTransform {
    return this.world.raw.getBodyTransform(this.handle);
  }

  setTransform(position: Vec3, rotation: Quat = [0, 0, 0, 1]): void {
    this.world.raw.setBodyTransform(this.handle, position, rotation);
  }


  setLinearVelocity(velocity: Vec3): void {
    this.world.raw.setBodyLinearVelocity(this.handle, velocity);
  }


  setAngularVelocity(velocity: Vec3): void {
    this.world.raw.setBodyAngularVelocity(this.handle, velocity);
  }


  getLinearVelocity(): Vec3 {
    return this.world.raw.getBodyLinearVelocity(this.handle);
  }

  getLinearVelocityTo(out: Vec3): Vec3 {
    return this.world.raw.getBodyLinearVelocityTo(this.handle, out);
  }


  getAngularVelocity(): Vec3 {
    return this.world.raw.getBodyAngularVelocity(this.handle);
  }

  getAngularVelocityTo(out: Vec3): Vec3 {
    return this.world.raw.getBodyAngularVelocityTo(this.handle, out);
  }

  applyLinearImpulse(impulse: Vec3, point: Vec3, wake = true): void {
    this.world.raw.applyLinearImpulse(this.handle, impulse, point, wake);
  }

  applyLinearImpulseToCenter(impulse: Vec3, wake = true): void {
    this.world.raw.applyLinearImpulseToCenter(this.handle, impulse, wake);
  }

  setAwake(awake: boolean): void {
    this.world.raw.setBodyAwake(this.handle, awake);
  }

  setDamping(linearDamping: number, angularDamping: number): void {
    this.world.raw.setBodyDamping(this.handle, linearDamping, angularDamping);
  }

  setGravityScale(scale: number): void {
    this.world.ownerRuntime.raw.setBodyGravityScale(this.handle, scale);
  }

  setSleepThreshold(threshold: number): void {
    this.world.ownerRuntime.raw.setBodySleepThreshold(this.handle, threshold);
  }

  enableSleep(enable: boolean): void {
    this.world.ownerRuntime.raw.enableBodySleep(this.handle, enable);
  }

  setBullet(flag: boolean): void {
    this.world.ownerRuntime.raw.setBodyBullet(this.handle, flag);
  }

  allowFastRotation(flag: boolean): void {
    this.world.ownerRuntime.raw.allowBodyFastRotation(this.handle, flag);
  }

  isFastRotationAllowed(): boolean {
    return this.world.ownerRuntime.raw.isBodyFastRotationAllowed(this.handle);
  }

  setMotionLocks(locks: { lockX?: boolean; lockY?: boolean; lockRotationX?: boolean; lockRotationY?: boolean; lockRotationZ?: boolean; lockLinearZ?: boolean } = {}): void {
    this.world.ownerRuntime.raw.setBodyMotionLocks(this.handle, locks);
  }

  setMassData(mass: number, center: Vec3, inertia?: Parameters<Box3DRuntime["setBodyMassData"]>[3]): void {
    this.world.ownerRuntime.raw.setBodyMassData(this.handle, mass, center, inertia);
  }

  applyMassFromShapes(): void {
    this.world.ownerRuntime.raw.applyBodyMassFromShapes(this.handle);
  }

  getMassData(): BodyMassData {
    return this.world.raw.getBodyMassData(this.handle);
  }

  getMass(): number {
    return this.world.raw.getBodyMass(this.handle);
  }

  getWorldCenter(): Vec3 {
    return this.world.raw.getBodyWorldCenter(this.handle);
  }

  getWorldCenterTo(out: Vec3): Vec3 {
    return this.world.raw.getBodyWorldCenterTo(this.handle, out);
  }

  getLocalPoint(worldPoint: Vec3): Vec3 {
    return this.world.raw.getBodyLocalPoint(this.handle, worldPoint);
  }

  getLocalPointXYZ(worldX: number, worldY: number, worldZ: number): Vec3 {
    return this.world.raw.getBodyLocalPointXYZ(this.handle, worldX, worldY, worldZ);
  }

  getLocalPointTo(worldPoint: Vec3, out: Vec3): Vec3 {
    return this.world.raw.getBodyLocalPointTo(this.handle, worldPoint, out);
  }

  getLocalPointXYZTo(worldX: number, worldY: number, worldZ: number, out: Vec3): Vec3 {
    return this.world.raw.getBodyLocalPointXYZTo(this.handle, worldX, worldY, worldZ, out);
  }

  getWorldPoint(localPoint: Vec3): Vec3 {
    return this.world.raw.getBodyWorldPoint(this.handle, localPoint);
  }

  getWorldPointXYZ(localX: number, localY: number, localZ: number): Vec3 {
    return this.world.raw.getBodyWorldPointXYZ(this.handle, localX, localY, localZ);
  }

  getWorldPointTo(localPoint: Vec3, out: Vec3): Vec3 {
    return this.world.raw.getBodyWorldPointTo(this.handle, localPoint, out);
  }

  getWorldPointXYZTo(localX: number, localY: number, localZ: number, out: Vec3): Vec3 {
    return this.world.raw.getBodyWorldPointXYZTo(this.handle, localX, localY, localZ, out);
  }

  getLocalPointVelocity(localPoint: Vec3): Vec3 {
    return this.world.raw.getBodyLocalPointVelocity(this.handle, localPoint);
  }

  getLocalPointVelocityXYZ(localX: number, localY: number, localZ: number): Vec3 {
    return this.world.raw.getBodyLocalPointVelocityXYZ(this.handle, localX, localY, localZ);
  }

  getLocalPointVelocityTo(localPoint: Vec3, out: Vec3): Vec3 {
    return this.world.raw.getBodyLocalPointVelocityTo(this.handle, localPoint, out);
  }

  getLocalPointVelocityXYZTo(localX: number, localY: number, localZ: number, out: Vec3): Vec3 {
    return this.world.raw.getBodyLocalPointVelocityXYZTo(this.handle, localX, localY, localZ, out);
  }

  getWorldPointVelocity(worldPoint: Vec3): Vec3 {
    return this.world.raw.getBodyWorldPointVelocity(this.handle, worldPoint);
  }

  getWorldPointVelocityXYZ(worldX: number, worldY: number, worldZ: number): Vec3 {
    return this.world.raw.getBodyWorldPointVelocityXYZ(this.handle, worldX, worldY, worldZ);
  }

  getWorldPointVelocityTo(worldPoint: Vec3, out: Vec3): Vec3 {
    return this.world.raw.getBodyWorldPointVelocityTo(this.handle, worldPoint, out);
  }

  getWorldPointVelocityXYZTo(worldX: number, worldY: number, worldZ: number, out: Vec3): Vec3 {
    return this.world.raw.getBodyWorldPointVelocityXYZTo(this.handle, worldX, worldY, worldZ, out);
  }

  getLocalRotationalInertia(): Mat3 {
    return this.world.raw.getBodyLocalRotationalInertia(this.handle);
  }

  getType(): BodyType {
    return this.world.raw.getBodyType(this.handle);
  }

  isAwake(): boolean {
    return this.world.raw.bodyIsAwake(this.handle);
  }

  isEnabled(): boolean {
    return this.world.raw.bodyIsEnabled(this.handle);
  }

  enable(): void {
    this.world.raw.bodyEnable(this.handle);
  }

  disable(): void {
    this.world.raw.bodyDisable(this.handle);
  }

  dispose(): void {
    if (this.disposed) return;
    this.disposed = true;
    this.world.raw.destroyBody(this.handle);
  }

  assertActive(): void {
    if (!objectAssertsEnabled()) return;
    this.world.assertActive();
    if (this.disposed) throw new Error("BodyRef has been disposed");
  }
}

export class ShapeRef {
  private disposed = false;

  constructor(private readonly world: ObjectWorld, public readonly handle: ShapeHandle) {}

  get isDisposed(): boolean {
    return this.disposed;
  }

  dispose(updateBodyMass = true): void {
    if (this.disposed) return;
    this.disposed = true;
    this.world.raw.destroyShape(this.handle, updateBodyMass);
  }

  setDensity(density: number, updateBodyMass = true): void {
    this.world.ownerRuntime.raw.setShapeDensity(this.handle, density, updateBodyMass);
  }

  setFriction(friction: number): void {
    this.world.ownerRuntime.raw.setShapeFriction(this.handle, friction);
  }

  setRestitution(restitution: number): void {
    this.world.ownerRuntime.raw.setShapeRestitution(this.handle, restitution);
  }

  setMaterial(material: Parameters<Box3DRuntime["setShapeSurfaceMaterial"]>[1]): void {
    this.world.ownerRuntime.raw.setShapeSurfaceMaterial(this.handle, material);
  }

  setFilter(categoryBits: number, maskBits: number, groupIndex = 0, invokeContacts = false): void {
    this.world.ownerRuntime.raw.setShapeFilter(this.handle, categoryBits, maskBits, groupIndex, invokeContacts);
  }

  enableSensorEvents(flag: boolean): void {
    this.world.ownerRuntime.raw.enableShapeSensorEvents(this.handle, flag);
  }

  enableContactEvents(flag: boolean): void {
    this.world.ownerRuntime.raw.enableShapeContactEvents(this.handle, flag);
  }

  enablePreSolveEvents(flag: boolean): void {
    this.world.ownerRuntime.raw.enableShapePreSolveEvents(this.handle, flag);
  }

  enableHitEvents(flag: boolean): void {
    this.world.ownerRuntime.raw.enableShapeHitEvents(this.handle, flag);
  }

  applyWind(wind: Vec3, drag: number, lift: number, maxSpeed: number, wake = true): void {
    this.world.ownerRuntime.raw.applyShapeWind(this.handle, wind, drag, lift, maxSpeed, wake);
  }

  assertActive(): void {
    if (!objectAssertsEnabled()) return;
    this.world.assertActive();
    if (this.disposed) throw new Error("ShapeRef has been disposed");
  }
}

export class JointRef {
  private disposed = false;

  constructor(private readonly world: ObjectWorld, public readonly handle: JointHandle) {}

  get isDisposed(): boolean {
    return this.disposed;
  }

  dispose(): void {
    if (this.disposed) return;
    if (objectAssertsEnabled()) this.world.assertActive();
    this.disposed = true;
    this.world.raw.destroyJoint(this.handle);
  }
}

export class HullRef {
  private disposed = false;

  constructor(private readonly runtime: ObjectRuntime, public readonly handle: HullHandle) {}

  get isDisposed(): boolean {
    return this.disposed;
  }

  dispose(): void {
    if (this.disposed) return;
    if (objectAssertsEnabled() && this.runtime.isDisposed) throw new Error("ObjectRuntime has been disposed");
    this.disposed = true;
    this.runtime.raw.destroyHull(this.handle);
  }
}

export class MeshRef {
  private disposed = false;

  constructor(private readonly runtime: ObjectRuntime, public readonly handle: MeshHandle) {}

  get isDisposed(): boolean {
    return this.disposed;
  }

  dispose(): void {
    if (this.disposed) return;
    if (objectAssertsEnabled() && this.runtime.isDisposed) throw new Error("ObjectRuntime has been disposed");
    this.disposed = true;
    this.runtime.raw.destroyMesh(this.handle);
  }
}

export class HumanRef {
  constructor(private readonly world: ObjectWorld, public readonly handle: HumanHandle) {}

  setVelocity(velocity: Vec3): void {
    this.world.ownerRuntime.raw.setHumanVelocity(this.handle, velocity);
  }

  setBullet(flag: boolean): void {
    this.world.ownerRuntime.raw.setHumanBullet(this.handle, flag);
  }

  setJointFrictionTorque(torque: number): void {
    this.world.ownerRuntime.raw.setHumanJointFrictionTorque(this.handle, torque);
  }

  setJointSpringHertz(hertz: number): void {
    this.world.ownerRuntime.raw.setHumanJointSpringHertz(this.handle, hertz);
  }

  setJointDampingRatio(dampingRatio: number): void {
    this.world.ownerRuntime.raw.setHumanJointDampingRatio(this.handle, dampingRatio);
  }

  getBoneBody(boneIndex: number): BodyRef {
    return this.world.body(this.world.ownerRuntime.raw.getHumanBoneBody(this.handle, boneIndex));
  }

  assertActive(): void {
    this.world.assertActive();
  }
}

export type ObjectPhysicsWorld = ObjectWorld;

// --- Assert guards: methods are authored bare; wrap once when compile asserts are on. ---

const OBJECT_RUNTIME_ASSERTED_METHODS = [
  "createWorld",
  "wrapWorld",
  "createCylinder",
  "createHullFromPoints",
  "wrapMesh",
] as const;

const OBJECT_WORLD_ASSERTED_METHODS = [
  "createBody",
  "createBox",
  "createBoxWithShape",
  "createSphere",
  "createSphereWithShape",
  "createGridMesh",
  "createWaveMesh",
  "createBoxMesh",
  "createTorusMesh",
  "body",
  "createHuman",
  "step",
  "enableSleeping",
  "enableContinuous",
  "enableWarmStarting",
  "setContactTuning",
  "setWorkerCount",
  "getCounters",
  "getProfile",
  "getAwakeBodyCount",
  "dispose",
] as const;

const OBJECT_WORLD_BODY_PAIR_METHODS = [
  "createMotorJoint",
  "createFilterJoint",
  "createRevoluteJoint",
  "createSphericalJoint",
  "createPrismaticJoint",
  "createWeldJoint",
  "createDistanceJoint",
] as const;

const BODY_REF_ASSERTED_METHODS = [
  "createHullShape",
  "createSphereShape",
  "createCapsuleShape",
  "createTransformedHullShape",
  "createShapeFromHull",
  "createCompoundShape",
  "createMeshShape",
  "getShapes",
  "getTransform",
  "setTransform",
  "setLinearVelocity",
  "setAngularVelocity",
  "getLinearVelocity",
  "getLinearVelocityTo",
  "getAngularVelocity",
  "getAngularVelocityTo",
  "applyLinearImpulse",
  "applyLinearImpulseToCenter",
  "setAwake",
  "setDamping",
  "setGravityScale",
  "setSleepThreshold",
  "enableSleep",
  "setBullet",
  "allowFastRotation",
  "isFastRotationAllowed",
  "setMotionLocks",
  "setMassData",
  "applyMassFromShapes",
  "getMassData",
  "getMass",
  "getWorldCenter",
  "getWorldCenterTo",
  "getLocalPoint",
  "getLocalPointXYZ",
  "getLocalPointTo",
  "getLocalPointXYZTo",
  "getWorldPoint",
  "getWorldPointXYZ",
  "getWorldPointTo",
  "getWorldPointXYZTo",
  "getLocalPointVelocity",
  "getLocalPointVelocityXYZ",
  "getLocalPointVelocityTo",
  "getLocalPointVelocityXYZTo",
  "getWorldPointVelocity",
  "getWorldPointVelocityXYZ",
  "getWorldPointVelocityTo",
  "getWorldPointVelocityXYZTo",
  "getLocalRotationalInertia",
  "getType",
  "isAwake",
  "isEnabled",
  "enable",
  "disable",
  "dispose",
] as const;

const SHAPE_REF_ASSERTED_METHODS = [
  "dispose",
  "setDensity",
  "setFriction",
  "setRestitution",
  "setMaterial",
  "setFilter",
  "enableSensorEvents",
  "enableContactEvents",
  "enablePreSolveEvents",
  "enableHitEvents",
  "applyWind",
] as const;

const HUMAN_REF_ASSERTED_METHODS = [
  "setVelocity",
  "setBullet",
  "setJointFrictionTorque",
  "setJointSpringHertz",
  "setJointDampingRatio",
  "getBoneBody",
] as const;

installObjectAssertGuards({
  targets: [
    {
      proto: ObjectRuntime.prototype,
      methods: OBJECT_RUNTIME_ASSERTED_METHODS,
      assert: (self) => (self as ObjectRuntime).assertActive(),
    },
    {
      proto: ObjectWorld.prototype,
      methods: OBJECT_WORLD_ASSERTED_METHODS,
      assert: (self) => (self as ObjectWorld).assertActive(),
    },
    {
      proto: BodyRef.prototype,
      methods: BODY_REF_ASSERTED_METHODS,
      assert: (self) => (self as BodyRef).assertActive(),
    },
    {
      proto: ShapeRef.prototype,
      methods: SHAPE_REF_ASSERTED_METHODS,
      assert: (self) => (self as ShapeRef).assertActive(),
    },
    {
      proto: HumanRef.prototype,
      methods: HUMAN_REF_ASSERTED_METHODS,
      assert: (self) => (self as HumanRef).assertActive(),
    },
  ],
  bodyPairs: [
    {
      proto: ObjectWorld.prototype,
      methods: OBJECT_WORLD_BODY_PAIR_METHODS,
      assertWorld: (self) => (self as ObjectWorld).assertActive(),
    },
  ],
});

