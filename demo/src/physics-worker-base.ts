import { Box3DRuntime, type BodyBatchBuffers, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand, PhysicsWorkerMessage, SolverParams } from "./physics-worker-protocol";
import { MAX_PROJECTILES, RAGDOLL_RENDER_BONE_COUNT, SNAPSHOT_AWAKE_COUNT_INDEX, SNAPSHOT_CUMULATIVE_STEPS_INDEX, SNAPSHOT_DROPPED_MS_X100_INDEX, SNAPSHOT_LAG_MS_X100_INDEX, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_PUBLISH_MS_X100_INDEX, SNAPSHOT_STEP_MS_X100_INDEX, SNAPSHOT_STEPS_INDEX, SNAPSHOT_VERSION_INDEX, SNAPSHOT_STATE_COUNT } from "./physics-worker-protocol";

const MAX_CATCHUP_STEPS = 4;
const MOUSE_FORCE_SCALE = 100;
const GRAVITY_MAGNITUDE = 9.81;

export abstract class PhysicsWorkerBase<TInit = void> {
  protected runtime: Box3DRuntime | null = null;
  protected world: PhysicsWorld | null = null;
  protected bodyBatch: BodyBatchBuffers | null = null;
  protected projectileBatch: BodyBatchBuffers | null = null;
  protected bodyCount = 0;
  protected currentWorkerCount = 4;
  protected maxWorkerCount = 127;
  protected groundSize: Vec3 = [160, 1, 160];

  protected positions: Float32Array | null = null;
  protected rotations: Float32Array | null = null;
  protected awake: Uint8Array | null = null;
  protected colors: Uint32Array | null = null;
  protected projectilePositions: Float32Array | null = null;
  protected projectileRotations: Float32Array | null = null;
  protected projectileAwake: Uint8Array | null = null;
  protected projectileColors: Uint32Array | null = null;
  protected state: Int32Array | null = null;

  protected timer: number | undefined;
  protected paused = false;
  protected lastTickTime = 0;
  protected accumulator = 0;
  protected totalSteps = 0;
  protected lastPublishTime = 0;
  protected publishIntervalMs = 15;
  protected useLightTransforms = true;

  protected projectileHandles: number[] = [];
  protected dragBody = 0;
  protected dragJoint = 0;
  protected dragDistance = 0;
  protected subSteps = 4;
  protected fixedTimeStep = 1 / 60;
  protected lastSolverParams: SolverParams = {};

  private initData!: TInit;

  constructor() {
    self.addEventListener("message", (event: MessageEvent<PhysicsWorkerCommand>) => {
      try {
        this.handleCommand(event.data);
      } catch (error) {
        this.publishError(error);
      }
    });
  }

  // --- Subclass hooks ---

  protected abstract buildScene(initData: TInit): Promise<number[]>;

  protected getReadyExtra(): Record<string, unknown> {
    return {};
  }

  protected handleCustomCommand(_cmd: PhysicsWorkerCommand): boolean {
    return false;
  }

  protected getGroundSize(_initData: TInit): Vec3 {
    return this.groundSize;
  }

  // --- Command routing ---

  private handleCommand(cmd: PhysicsWorkerCommand): void {
    switch (cmd.type) {
      case "init":
        void this.handleInit(cmd).catch((err) => this.publishError(err));
        break;
      case "spawn-projectile":
        this.spawnProjectile(cmd.origin, cmd.velocity);
        break;
      case "spawn-ragdoll":
        this.spawnRagdoll(cmd.origin, cmd.velocity);
        break;
      case "drag-start":
        this.startDrag(cmd.origin, cmd.translation);
        break;
      case "drag-update":
        this.updateDrag(cmd.origin, cmd.translation);
        break;
      case "drag-end":
        this.endDrag();
        break;
      case "set-paused":
        this.paused = cmd.paused;
        break;
      case "step-once":
        this.stepOnce();
        break;
      case "toggle-worker-count":
        this.toggleWorkerCount();
        break;
      case "set-solver-params":
        this.applySolverParams(cmd.params);
        break;
      case "dispose":
        this.dispose();
        break;
      default:
        if (!this.handleCustomCommand(cmd)) {
          console.warn(`[worker] Unknown command type: ${(cmd as Record<string, unknown>).type as string}`);
        }
    }
  }

  // --- Initialization ---

  private async handleInit(cmd: PhysicsWorkerCommand & { type: "init" }): Promise<void> {
    this.initData = cmd.data as TInit;
    this.maxWorkerCount = cmd.maxWorkers ?? 127;
    this.currentWorkerCount = cmd.workerCount ?? this.currentWorkerCount;

    this.runtime = await Box3DRuntime.load({ version: cmd.wasmVersion, variant: cmd.wasmVariant, poolSize: cmd.poolSize });
    this.world = this.runtime.createWorld({ gravity: [0, -9.81, 0], workerCount: this.currentWorkerCount });
    console.log("[worker]", "checkThreadingSupport:", this.runtime.checkThreadingSupport());
    console.log("[worker]", "workerCount:", this.world.getWorkerCount());

    if (cmd.solverParams) this.applySolverParams(cmd.solverParams);
    console.log("[worker]", "solverParams:", this.lastSolverParams);

    const groundBody = this.world.createBody({ type: 0, position: [0, -1, 0] });
    this.runtime.createHullShape(groundBody, this.getGroundSize(this.initData));

    const handles = await this.buildScene(this.initData);
    this.bodyCount = handles.length;

    this.bodyBatch = this.world.allocBodyBatchBuffers(this.bodyCount);
    this.world.writeBodyHandles(this.bodyBatch, handles);
    this.projectileBatch = this.world.allocBodyBatchBuffers(MAX_PROJECTILES);

    this.allocateSharedBuffers();
    this.totalSteps = 0;

    this.publishSnapshot(0, 0, 0, 0);
    this.postReady();

    this.lastTickTime = performance.now();
    this.accumulator = 0;
    this.timer = self.setInterval(() => this.tick(), 1000 / 60);
  }

  private allocateSharedBuffers(): void {
    const positionBuffer = new SharedArrayBuffer(this.bodyCount * 3 * 4);
    const rotationBuffer = new SharedArrayBuffer(this.bodyCount * 4 * 4);
    const awakeBuffer = new SharedArrayBuffer(this.bodyCount);
    const colorBuffer = new SharedArrayBuffer(this.bodyCount * 4);
    const projectilePositionBuffer = new SharedArrayBuffer(MAX_PROJECTILES * 3 * 4);
    const projectileRotationBuffer = new SharedArrayBuffer(MAX_PROJECTILES * 4 * 4);
    const projectileAwakeBuffer = new SharedArrayBuffer(MAX_PROJECTILES);
    const projectileColorBuffer = new SharedArrayBuffer(MAX_PROJECTILES * 4);
    const stateBuffer = new SharedArrayBuffer(SNAPSHOT_STATE_COUNT * 4);

    this.positions = new Float32Array(positionBuffer);
    this.rotations = new Float32Array(rotationBuffer);
    this.awake = new Uint8Array(awakeBuffer);
    this.colors = new Uint32Array(colorBuffer);
    this.projectilePositions = new Float32Array(projectilePositionBuffer);
    this.projectileRotations = new Float32Array(projectileRotationBuffer);
    this.projectileAwake = new Uint8Array(projectileAwakeBuffer);
    this.projectileColors = new Uint32Array(projectileColorBuffer);
    this.state = new Int32Array(stateBuffer);
  }

  private postReady(): void {
    const message: PhysicsWorkerMessage = {
      type: "ready",
      count: this.bodyCount,
      workerCount: this.currentWorkerCount,
      positions: (this.positions as Float32Array).buffer as SharedArrayBuffer,
      rotations: (this.rotations as Float32Array).buffer as SharedArrayBuffer,
      awake: (this.awake as Uint8Array).buffer as SharedArrayBuffer,
      colors: (this.colors as Uint32Array).buffer as SharedArrayBuffer,
      projectilePositions: (this.projectilePositions as Float32Array).buffer as SharedArrayBuffer,
      projectileRotations: (this.projectileRotations as Float32Array).buffer as SharedArrayBuffer,
      projectileAwake: (this.projectileAwake as Uint8Array).buffer as SharedArrayBuffer,
      projectileColors: (this.projectileColors as Uint32Array).buffer as SharedArrayBuffer,
      state: (this.state as Int32Array).buffer as SharedArrayBuffer,
      extra: this.getReadyExtra(),
    };
    self.postMessage(message);
  }

  // --- Tick ---

  private stepPhysics(): number {
    if (this.world === null) return 0;
    const start = performance.now();
    this.world.step(this.fixedTimeStep, this.subSteps);
    return performance.now() - start;
  }

  private stepOnce(): void {
    const stepMs = this.stepPhysics();
    this.lastPublishTime = 0;
    this.publishSnapshot(stepMs, 1, this.accumulator * 1000, 0);
  }

  private tick(): void {
    if (this.paused) {
      this.lastTickTime = performance.now();
      return;
    }
    const now = performance.now();
    if (this.lastTickTime === 0) this.lastTickTime = now;
    this.accumulator += Math.min((now - this.lastTickTime) / 1000, 0.25);
    this.lastTickTime = now;

    let steps = 0;
    let stepMs = 0;
    while (this.accumulator >= this.fixedTimeStep && steps < MAX_CATCHUP_STEPS) {
      stepMs = this.stepPhysics();
      this.accumulator -= this.fixedTimeStep;
      steps++;
    }

    let droppedMs = 0;
    if (steps === MAX_CATCHUP_STEPS && this.accumulator >= this.fixedTimeStep) {
      droppedMs = this.accumulator * 1000;
      this.accumulator = 0;
    }

    if (steps > 0 && this.shouldPublish()) this.publishSnapshot(stepMs, steps, this.accumulator * 1000, droppedMs);
  }

  private shouldPublish(): boolean {
    const now = performance.now();
    if (now - this.lastPublishTime >= this.publishIntervalMs) {
      return true;
    }
    return false;
  }

  private publishSnapshot(stepMs: number, steps: number, lagMs: number, droppedMs: number): void {
    if (this.world === null || this.bodyBatch === null || this.positions === null || this.rotations === null || this.awake === null || this.colors === null || this.state === null) return;
    this.totalSteps += steps;
    const publishStart = performance.now();

    const writeFn = this.useLightTransforms ? this.world.writeBodyTransformsLight.bind(this.world) : this.world.writeBodyTransforms.bind(this.world);
    writeFn(this.bodyCount, this.bodyBatch.bodyHandlesPtr, this.bodyBatch.positionsPtr, this.bodyBatch.rotationsPtr, this.bodyBatch.awakePtr, this.bodyBatch.colorsPtr);
    const memory = this.world.getMemoryView();

    this.positions.set(new Float32Array(memory.heapF32.buffer, this.bodyBatch.positionsPtr, this.bodyCount * 3));
    this.rotations.set(new Float32Array(memory.heapF32.buffer, this.bodyBatch.rotationsPtr, this.bodyCount * 4));
    this.awake.set(new Uint8Array(memory.heapU8.buffer, this.bodyBatch.awakePtr, this.bodyCount));
    this.colors.set(new Uint32Array(memory.heap32.buffer, this.bodyBatch.colorsPtr, this.bodyCount));

    const awakeCount = this.world.getAwakeBodyCount();

    const projectileCount = this.projectileHandles.length;
    if (projectileCount > 0 && this.projectileBatch !== null && this.projectilePositions !== null && this.projectileRotations !== null && this.projectileAwake !== null && this.projectileColors !== null) {
      this.world.writeBodyTransforms(projectileCount, this.projectileBatch.bodyHandlesPtr, this.projectileBatch.positionsPtr, this.projectileBatch.rotationsPtr, this.projectileBatch.awakePtr, this.projectileBatch.colorsPtr);
      this.projectilePositions.set(new Float32Array(memory.heapF32.buffer, this.projectileBatch.positionsPtr, projectileCount * 3).subarray(0, projectileCount * 3));
      this.projectileRotations.set(new Float32Array(memory.heapF32.buffer, this.projectileBatch.rotationsPtr, projectileCount * 4).subarray(0, projectileCount * 4));
      this.projectileAwake.set(new Uint8Array(memory.heapU8.buffer, this.projectileBatch.awakePtr, projectileCount).subarray(0, projectileCount));
      this.projectileColors.set(new Uint32Array(memory.heap32.buffer, this.projectileBatch.colorsPtr, projectileCount).subarray(0, projectileCount));
    }

    const publishMs = performance.now() - publishStart;

    Atomics.store(this.state, SNAPSHOT_AWAKE_COUNT_INDEX, awakeCount);
    Atomics.store(this.state, SNAPSHOT_PROJECTILE_COUNT_INDEX, projectileCount);
    Atomics.store(this.state, SNAPSHOT_STEP_MS_X100_INDEX, Math.round(stepMs * 100));
    Atomics.store(this.state, SNAPSHOT_PUBLISH_MS_X100_INDEX, Math.round(publishMs * 100));
    Atomics.store(this.state, SNAPSHOT_LAG_MS_X100_INDEX, Math.round(lagMs * 100));
    Atomics.store(this.state, SNAPSHOT_STEPS_INDEX, steps);
    Atomics.store(this.state, SNAPSHOT_DROPPED_MS_X100_INDEX, Math.round(droppedMs * 100));
    Atomics.store(this.state, SNAPSHOT_CUMULATIVE_STEPS_INDEX, this.totalSteps);
    Atomics.add(this.state, SNAPSHOT_VERSION_INDEX, 1);

    this.lastPublishTime = performance.now();
  }

  // --- Projectiles ---

  private spawnProjectile(origin: Vec3, velocity: Vec3): void {
    if (this.runtime === null || this.world === null || this.projectileBatch === null || this.projectileHandles.length >= MAX_PROJECTILES) return;
    const bodyHandle = this.runtime.createSphere(this.world.handle, {
      radius: 0.25,
      position: origin,
      velocity,
      density: 4000,
      isBullet: true,
    });
    this.projectileHandles.push(bodyHandle);
    this.world.writeBodyHandles(this.projectileBatch, this.projectileHandles);
  }

  private spawnRagdoll(origin: Vec3, velocity: Vec3): void {
    if (this.runtime === null || this.world === null || this.projectileBatch === null) return;
    const humanHandle = this.world.createHuman(origin, { frictionTorque: 1, hertz: 1, dampingRatio: 1, groupIndex: 0, colorize: true });
    if (humanHandle === 0) return;
    this.runtime.setHumanBullet(humanHandle, true);
    this.runtime.setHumanVelocity(humanHandle, velocity);
    const boneCount = Math.min(this.runtime.getHumanBoneCount(), RAGDOLL_RENDER_BONE_COUNT);
    for (let i = 0; i < boneCount && this.projectileHandles.length < MAX_PROJECTILES; i++) {
      const bodyHandle = this.runtime.getHumanBoneBody(humanHandle, i);
      if (bodyHandle !== 0) this.projectileHandles.push(bodyHandle);
    }
    this.world.writeBodyHandles(this.projectileBatch, this.projectileHandles);
  }

  // --- Drag ---

  private endDrag(): void {
    if (this.world === null) return;
    if (this.dragJoint !== 0) {
      this.world.destroyJoint(this.dragJoint);
      this.dragJoint = 0;
    }
    if (this.dragBody !== 0) {
      this.world.destroyBody(this.dragBody);
      this.dragBody = 0;
    }
  }

  private dragPoint(origin: Vec3, translation: Vec3): Vec3 {
    const len = Math.hypot(translation[0], translation[1], translation[2]) || 1;
    const scale = this.dragDistance / len;
    return [origin[0] + translation[0] * scale, origin[1] + translation[1] * scale, origin[2] + translation[2] * scale];
  }

  private startDrag(origin: Vec3, translation: Vec3): void {
    if (this.world === null) return;
    this.endDrag();
    const hit = this.world.rayCastClosest(origin, translation);
    if (hit === null || hit.bodyHandle === 0 || this.world.getBodyType(hit.bodyHandle) !== 2) return;
    const point = hit.point;
    this.dragDistance = Math.hypot(point[0] - origin[0], point[1] - origin[1], point[2] - origin[2]);
    this.dragBody = this.world.createBody({ type: 1, position: point, enableSleep: false });
    const localBodyPoint = this.world.getBodyLocalPoint(hit.bodyHandle, point);
    const massData = this.world.getBodyMassData(hit.bodyHandle);
    const mg = massData.mass * GRAVITY_MAGNITUDE;
    const lever = massData.mass > 0 ? Math.sqrt(massData.inertiaTrace / (3 * massData.mass)) : 0;
    this.dragJoint = this.world.createMotorJoint(this.dragBody, hit.bodyHandle, {
      localFrameA: [0, 0, 0],
      localFrameB: localBodyPoint,
      linearHertz: 7.5,
      linearDampingRatio: 1,
      maxSpringForce: MOUSE_FORCE_SCALE * mg,
      maxVelocityTorque: 0.5 * lever * mg,
    });
    this.world.setBodyAwake(hit.bodyHandle, true);
  }

  private updateDrag(origin: Vec3, translation: Vec3): void {
    if (this.world === null || this.dragBody === 0) return;
    this.world.setBodyTransform(this.dragBody, this.dragPoint(origin, translation));
  }

  // --- Solver params ---

  protected applySolverParams(params: SolverParams): void {
    if (this.runtime === null || this.world === null) return;
    if (params.subSteps !== undefined) { this.subSteps = params.subSteps; this.lastSolverParams.subSteps = params.subSteps; }
    if (params.hertz !== undefined) { this.fixedTimeStep = 1 / params.hertz; this.lastSolverParams.hertz = params.hertz; }
    if (params.recycleDistance !== undefined) { this.runtime.setWorldContactRecycleDistance(this.world.handle, params.recycleDistance); this.lastSolverParams.recycleDistance = params.recycleDistance; }
    if (params.sleep !== undefined) { this.runtime.enableWorldSleeping(this.world.handle, params.sleep); this.lastSolverParams.sleep = params.sleep; }
    if (params.continuous !== undefined) { this.runtime.enableWorldContinuous(this.world.handle, params.continuous); this.lastSolverParams.continuous = params.continuous; }
    if (params.warmStart !== undefined) { this.runtime.enableWorldWarmStarting(this.world.handle, params.warmStart); this.lastSolverParams.warmStart = params.warmStart; }
    if (params.workerCount !== undefined) {
      const changed = params.workerCount !== this.currentWorkerCount;
      this.currentWorkerCount = params.workerCount;
      this.lastSolverParams.workerCount = params.workerCount;
      if (changed) void this.restartScene();
    }
  }

  // --- Worker toggle ---

  private toggleWorkerCount(): void {
    const newCount = this.currentWorkerCount === 1 ? this.maxWorkerCount : 1;
    this.currentWorkerCount = newCount;
    void this.restartScene();
  }

  private async restartScene(): Promise<void> {
    this.disposeWorld();

    this.world = this.runtime!.createWorld({ gravity: [0, -9.81, 0], workerCount: this.currentWorkerCount });
    console.log("[worker]", "workerCount:", this.world.getWorkerCount());

    this.applySolverParams(this.lastSolverParams);

    const groundBody = this.world.createBody({ type: 0, position: [0, -1, 0] });
    this.runtime!.createHullShape(groundBody, this.getGroundSize(this.initData));

    const handles = await this.buildScene(this.initData);
    this.bodyCount = handles.length;

    this.bodyBatch = this.world.allocBodyBatchBuffers(this.bodyCount);
    this.world.writeBodyHandles(this.bodyBatch, handles);
    this.projectileBatch = this.world.allocBodyBatchBuffers(MAX_PROJECTILES);

    this.projectileHandles.length = 0;

    this.allocateSharedBuffers();
    this.totalSteps = 0;

    this.stepOnce();
    this.postReady();
    this.lastTickTime = performance.now();
    this.accumulator = 0;
  }

  // --- Dispose ---

  private disposeWorld(): void {
    if (this.timer !== undefined) self.clearInterval(this.timer);
    this.timer = undefined;
    if (this.world !== null && this.bodyBatch !== null) this.world.freeBodyBatchBuffers(this.bodyBatch);
    if (this.world !== null && this.projectileBatch !== null) this.world.freeBodyBatchBuffers(this.projectileBatch);
    this.world?.destroy();
    this.world = null;
    this.bodyBatch = null;
    this.projectileBatch = null;
    this.projectileHandles.length = 0;
    this.dragBody = 0;
    this.dragJoint = 0;
    this.dragDistance = 0;
    this.totalSteps = 0;
    this.lastPublishTime = 0;
    this.positions = null;
    this.rotations = null;
    this.awake = null;
    this.projectilePositions = null;
    this.projectileRotations = null;
    this.projectileAwake = null;
    this.state = null;
    this.lastTickTime = 0;
    this.accumulator = 0;
    this.paused = false;
  }

  private dispose(): void {
    this.disposeWorld();
    this.runtime?.destroy();
    this.runtime = null;
  }

  private publishError(error: unknown): void {
    self.postMessage({ type: "error", message: error instanceof Error ? error.message : String(error) });
  }
}
