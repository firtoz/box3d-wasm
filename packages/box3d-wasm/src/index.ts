export type Vec3 = [number, number, number];
export type Quat = [number, number, number, number];

export enum BodyType {
  Static = 0,
  Kinematic = 1,
  Dynamic = 2,
}

export interface WorldOptions {
  gravity?: Vec3;
}

export interface BoxOptions {
  size?: Vec3;
  position?: Vec3;
  static?: boolean;
  density?: number;
}

export interface SphereOptions {
  radius?: number;
  position?: Vec3;
  velocity?: Vec3;
  density?: number;
}

export interface BodyOptions {
  type?: BodyType;
  position?: Vec3;
  enableSleep?: boolean;
  awake?: boolean;
}

export interface MotorJointOptions {
  localFrameA?: Vec3;
  localFrameB?: Vec3;
  linearVelocity?: Vec3;
  angularVelocity?: Vec3;
  maxVelocityForce?: number;
  maxVelocityTorque?: number;
  linearHertz?: number;
  linearDampingRatio?: number;
  maxSpringForce?: number;
  angularHertz?: number;
  angularDampingRatio?: number;
  maxSpringTorque?: number;
}

export interface BodyTransform {
  position: Vec3;
  rotation: Quat;
}

export interface RuntimeLoadOptions {
  version?: string;
}

export interface RuntimeAPI {
  createWorld(options?: WorldOptions): PhysicsWorld;
}

type CModule = {
  cwrap(
    name: string,
    returnType: "number",
    argTypes: readonly string[],
  ): (...args: number[]) => number;
  cwrap(name: string, returnType: null, argTypes: readonly string[]): (...args: number[]) => void;
  HEAPF32: Float32Array;
  _malloc(size: number): number;
  _free(ptr: number): void;
};

type CreateWorldFn = (gravityX: number, gravityY: number, gravityZ: number) => number;
type CreateBodyFn = (
  worldHandle: number,
  bodyType: number,
  px: number,
  py: number,
  pz: number,
  enableSleep: number,
  awake: number,
) => number;
type DestroyWorldFn = (worldHandle: number) => void;
type CreateBoxFn = (
  worldHandle: number,
  px: number,
  py: number,
  pz: number,
  hx: number,
  hy: number,
  hz: number,
  isStatic: number,
  density: number,
) => number;
type CreateSphereFn = (
  worldHandle: number,
  px: number,
  py: number,
  pz: number,
  radius: number,
  vx: number,
  vy: number,
  vz: number,
  density: number,
) => number;
type DestroyBodyFn = (bodyHandle: number) => void;
type DestroyJointFn = (jointHandle: number) => void;
type SetBodyTransformFn = (
  bodyHandle: number,
  px: number,
  py: number,
  pz: number,
  qx: number,
  qy: number,
  qz: number,
  qw: number,
) => void;
type SetBodyAwakeFn = (bodyHandle: number, awake: number) => void;
type SetBodyDampingFn = (bodyHandle: number, linearDamping: number, angularDamping: number) => void;
type GetBodyLocalPointFn = (
  bodyHandle: number,
  worldX: number,
  worldY: number,
  worldZ: number,
  outPoint: number,
) => void;
type CreateMotorJointFn = (
  worldHandle: number,
  bodyAHandle: number,
  bodyBHandle: number,
  localAx: number,
  localAy: number,
  localAz: number,
  localBx: number,
  localBy: number,
  localBz: number,
  linearVx: number,
  linearVy: number,
  linearVz: number,
  maxVelocityForce: number,
  angularVx: number,
  angularVy: number,
  angularVz: number,
  maxVelocityTorque: number,
  linearHertz: number,
  linearDampingRatio: number,
  maxSpringForce: number,
  angularHertz: number,
  angularDampingRatio: number,
  maxSpringTorque: number,
) => number;
type StepFn = (worldHandle: number, timeStep: number, subStepCount: number) => void;
type GetBodyTransformFn = (bodyHandle: number, outTransform: number) => void;

type ModuleFactory = (options: { locateFile(path: string): string }) => Promise<CModule>;

type ModuleImport = {
  default: ModuleFactory;
};

function vec3(x = 0, y = 0, z = 0): Vec3 {
  return [x, y, z];
}

function versionedUrl(url: string, version: string | undefined): string {
  if (version === undefined || version.length === 0) return url;
  const separator = url.includes("?") ? "&" : "?";
  return `${url}${separator}v=${encodeURIComponent(version)}`;
}

export class Box3DRuntime implements RuntimeAPI {
  static async load(options: RuntimeLoadOptions = {}): Promise<Box3DRuntime> {
    const baseUrl =
      typeof document === "undefined" ? "/" : new URL(".", window.location.href).pathname;
    const moduleUrl = versionedUrl(`${baseUrl}wasm/box3d-web.js`, options.version);
    console.log(`[box3d-wasm] loading JS wrapper: ${moduleUrl}`);
    const response = await fetch(moduleUrl, { cache: "no-store" });
    if (!response.ok) {
      throw new Error(`Failed to load ${moduleUrl}: ${response.status} ${response.statusText}`);
    }

    const source = await response.text();
    if (source.startsWith("<!doctype html>") || source.startsWith("<html")) {
      throw new Error(
        `Expected JS at ${moduleUrl}, got HTML instead. First bytes: ${source.slice(0, 80)}`,
      );
    }
    console.log(`[box3d-wasm] fetched JS wrapper (${source.length} bytes)`);
    const blobUrl = URL.createObjectURL(new Blob([source], { type: "text/javascript" }));
    const moduleImport = (await import(/* @vite-ignore */ blobUrl)) as ModuleImport;
    URL.revokeObjectURL(blobUrl);

    const module = await moduleImport.default({
      locateFile(path: string): string {
        const resolved = versionedUrl(new URL(path, moduleUrl).href, options.version);
        console.log(`[box3d-wasm] locateFile(${path}) -> ${resolved}`);
        return resolved;
      },
    });

    return new Box3DRuntime(module);
  }

  private readonly module: CModule;
  private readonly createWorldFn: CreateWorldFn;
  private readonly createBodyFn: CreateBodyFn;
  private readonly destroyWorldFn: DestroyWorldFn;
  private readonly createBoxFn: CreateBoxFn;
  private readonly createSphereFn: CreateSphereFn;
  private readonly destroyBodyFn: DestroyBodyFn;
  private readonly destroyJointFn: DestroyJointFn;
  private readonly setBodyTransformFn: SetBodyTransformFn;
  private readonly setBodyAwakeFn: SetBodyAwakeFn;
  private readonly setBodyDampingFn: SetBodyDampingFn;
  private readonly getBodyLocalPointFn: GetBodyLocalPointFn;
  private readonly createMotorJointFn: CreateMotorJointFn;
  private readonly stepFn: StepFn;
  private readonly getBodyTransformFn: GetBodyTransformFn;
  private readonly transformPtr: number;
  private readonly pointPtr: number;

  constructor(module: CModule) {
    this.module = module;
    this.createWorldFn = module.cwrap("b3wCreateWorld", "number", ["number", "number", "number"]);
    this.createBodyFn = module.cwrap("b3wCreateBody", "number", [
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
    ]);
    this.destroyWorldFn = module.cwrap("b3wDestroyWorld", null, ["number"]);
    this.createBoxFn = module.cwrap("b3wCreateBox", "number", [
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
    ]);
    this.createSphereFn = module.cwrap("b3wCreateSphere", "number", [
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
    ]);
    this.destroyBodyFn = module.cwrap("b3wDestroyBody", null, ["number"]);
    this.destroyJointFn = module.cwrap("b3wDestroyJoint", null, ["number"]);
    this.setBodyTransformFn = module.cwrap("b3wSetBodyTransform", null, [
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
    ]);
    this.setBodyAwakeFn = module.cwrap("b3wSetBodyAwake", null, ["number", "number"]);
    this.setBodyDampingFn = module.cwrap("b3wSetBodyDamping", null, ["number", "number", "number"]);
    this.getBodyLocalPointFn = module.cwrap("b3wGetBodyLocalPoint", null, [
      "number",
      "number",
      "number",
      "number",
      "number",
    ]);
    this.createMotorJointFn = module.cwrap("b3wCreateMotorJoint", "number", [
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
    ]);
    this.stepFn = module.cwrap("b3wStep", null, ["number", "number", "number"]);
    this.getBodyTransformFn = module.cwrap("b3wGetBodyTransform", null, ["number", "number"]);
    this.transformPtr = module._malloc(7 * 4);
    this.pointPtr = module._malloc(3 * 4);
  }

  createWorld(options: WorldOptions = {}): PhysicsWorld {
    const gravity = options.gravity ?? vec3(0, -9.81, 0);
    const handle = this.createWorldFn(gravity[0], gravity[1], gravity[2]);
    return new PhysicsWorld(this, handle);
  }

  destroy(): void {
    this.module._free(this.transformPtr);
    this.module._free(this.pointPtr);
  }

  createBody(worldHandle: number, options: BodyOptions = {}): number {
    const position = options.position ?? vec3();
    const type = options.type ?? BodyType.Static;
    const enableSleep = options.enableSleep ?? true;
    const awake = options.awake ?? true;
    return this.createBodyFn(
      worldHandle,
      type,
      position[0],
      position[1],
      position[2],
      enableSleep ? 1 : 0,
      awake ? 1 : 0,
    );
  }

  createBox(worldHandle: number, options: Required<BoxOptions>): number {
    const size = options.size;
    const position = options.position;
    return this.createBoxFn(
      worldHandle,
      position[0],
      position[1],
      position[2],
      size[0],
      size[1],
      size[2],
      options.static ? 1 : 0,
      options.density,
    );
  }

  createSphere(worldHandle: number, options: Required<SphereOptions>): number {
    const position = options.position;
    const velocity = options.velocity;
    return this.createSphereFn(
      worldHandle,
      position[0],
      position[1],
      position[2],
      options.radius,
      velocity[0],
      velocity[1],
      velocity[2],
      options.density,
    );
  }

  destroyBody(bodyHandle: number): void {
    this.destroyBodyFn(bodyHandle);
  }

  destroyJoint(jointHandle: number): void {
    this.destroyJointFn(jointHandle);
  }

  setBodyTransform(bodyHandle: number, position: Vec3, rotation: Quat = [0, 0, 0, 1]): void {
    this.setBodyTransformFn(
      bodyHandle,
      position[0],
      position[1],
      position[2],
      rotation[0],
      rotation[1],
      rotation[2],
      rotation[3],
    );
  }

  setBodyAwake(bodyHandle: number, awake: boolean): void {
    this.setBodyAwakeFn(bodyHandle, awake ? 1 : 0);
  }

  setBodyDamping(bodyHandle: number, linearDamping: number, angularDamping: number): void {
    this.setBodyDampingFn(bodyHandle, linearDamping, angularDamping);
  }

  getBodyLocalPoint(bodyHandle: number, worldPoint: Vec3): Vec3 {
    this.getBodyLocalPointFn(
      bodyHandle,
      worldPoint[0],
      worldPoint[1],
      worldPoint[2],
      this.pointPtr,
    );
    const heap = this.module.HEAPF32;
    const base = this.pointPtr >> 2;
    return [heap[base + 0], heap[base + 1], heap[base + 2]];
  }

  createMotorJoint(
    worldHandle: number,
    bodyAHandle: number,
    bodyBHandle: number,
    options: MotorJointOptions = {},
  ): number {
    const localFrameA = options.localFrameA ?? vec3();
    const localFrameB = options.localFrameB ?? vec3();
    const linearVelocity = options.linearVelocity ?? vec3();
    const angularVelocity = options.angularVelocity ?? vec3();
    return this.createMotorJointFn(
      worldHandle,
      bodyAHandle,
      bodyBHandle,
      localFrameA[0],
      localFrameA[1],
      localFrameA[2],
      localFrameB[0],
      localFrameB[1],
      localFrameB[2],
      linearVelocity[0],
      linearVelocity[1],
      linearVelocity[2],
      options.maxVelocityForce ?? 0,
      angularVelocity[0],
      angularVelocity[1],
      angularVelocity[2],
      options.maxVelocityTorque ?? 0,
      options.linearHertz ?? 0,
      options.linearDampingRatio ?? 0,
      options.maxSpringForce ?? 0,
      options.angularHertz ?? 0,
      options.angularDampingRatio ?? 0,
      options.maxSpringTorque ?? 0,
    );
  }

  readBodyTransform(bodyHandle: number): BodyTransform {
    this.getBodyTransformFn(bodyHandle, this.transformPtr);
    const heap = this.module.HEAPF32;
    const base = this.transformPtr >> 2;
    return {
      position: [heap[base + 0], heap[base + 1], heap[base + 2]],
      rotation: [heap[base + 3], heap[base + 4], heap[base + 5], heap[base + 6]],
    };
  }

  step(worldHandle: number, dt: number, substeps: number): void {
    this.stepFn(worldHandle, dt, substeps);
  }

  destroyWorld(worldHandle: number): void {
    this.destroyWorldFn(worldHandle);
  }
}

export class PhysicsWorld {
  constructor(
    private readonly runtime: Box3DRuntime,
    public readonly handle: number,
  ) {}

  createBody(options: BodyOptions = {}): number {
    return this.runtime.createBody(this.handle, options);
  }

  createBox(options: BoxOptions = {}): number {
    return this.runtime.createBox(this.handle, {
      size: options.size ?? [1, 1, 1],
      position: options.position ?? [0, 0, 0],
      static: options.static ?? false,
      density: options.density ?? 1,
    });
  }

  createSphere(options: SphereOptions = {}): number {
    return this.runtime.createSphere(this.handle, {
      radius: options.radius ?? 0.5,
      position: options.position ?? [0, 0, 0],
      velocity: options.velocity ?? [0, 0, 0],
      density: options.density ?? 1,
    });
  }

  destroyBody(bodyHandle: number): void {
    this.runtime.destroyBody(bodyHandle);
  }

  destroyJoint(jointHandle: number): void {
    this.runtime.destroyJoint(jointHandle);
  }

  setBodyTransform(bodyHandle: number, position: Vec3, rotation: Quat = [0, 0, 0, 1]): void {
    this.runtime.setBodyTransform(bodyHandle, position, rotation);
  }

  setBodyAwake(bodyHandle: number, awake: boolean): void {
    this.runtime.setBodyAwake(bodyHandle, awake);
  }

  setBodyDamping(bodyHandle: number, linearDamping: number, angularDamping: number): void {
    this.runtime.setBodyDamping(bodyHandle, linearDamping, angularDamping);
  }

  getBodyLocalPoint(bodyHandle: number, worldPoint: Vec3): Vec3 {
    return this.runtime.getBodyLocalPoint(bodyHandle, worldPoint);
  }

  createMotorJoint(
    bodyAHandle: number,
    bodyBHandle: number,
    options: MotorJointOptions = {},
  ): number {
    return this.runtime.createMotorJoint(this.handle, bodyAHandle, bodyBHandle, options);
  }

  getBodyTransform(bodyHandle: number): BodyTransform {
    return this.runtime.readBodyTransform(bodyHandle);
  }

  step(dt = 1 / 60, substeps = 4): void {
    this.runtime.step(this.handle, dt, substeps);
  }

  destroy(): void {
    this.runtime.destroyWorld(this.handle);
  }
}
