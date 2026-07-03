import { Box3DRuntime, type BodyBatchBuffers, type Box3DRuntime as RuntimeType, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand, PhysicsWorkerMessage } from "./physics-worker-protocol";
import { MAX_PROJECTILES, RAGDOLL_RENDER_BONE_COUNT, SNAPSHOT_AWAKE_COUNT_INDEX, SNAPSHOT_DROPPED_MS_X100_INDEX, SNAPSHOT_LAG_MS_X100_INDEX, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_STEP_MS_X100_INDEX, SNAPSHOT_STEPS_INDEX, SNAPSHOT_VERSION_INDEX } from "./physics-worker-protocol";

const FIXED_TIME_STEP = 1 / 60;
const MAX_CATCHUP_STEPS = 4;

let dominoCount = 30 * 180;
let runtime: RuntimeType | null = null;
let world: PhysicsWorld | null = null;
let batch: BodyBatchBuffers | null = null;
let projectileBatch: BodyBatchBuffers | null = null;
let positions: Float32Array | null = null;
let rotations: Float32Array | null = null;
let awake: Uint8Array | null = null;
let projectilePositions: Float32Array | null = null;
let projectileRotations: Float32Array | null = null;
let projectileAwake: Uint8Array | null = null;
let state: Int32Array | null = null;
let timer: number | undefined;
let paused = false;
let lastTickTime = 0;
let accumulator = 0;
let currentWorkerCount = 4;
let maxWorkerCount = 127;
const projectileHandles: number[] = [];
let dragBody = 0;
let dragJoint = 0;
let dragDistance = 0;

function post(message: PhysicsWorkerMessage): void {
  self.postMessage(message);
}

function publishError(error: unknown): void {
  post({ type: "error", message: error instanceof Error ? error.message : String(error) });
}

function stepPhysics(): number {
  if (world === null) return 0;
  const start = performance.now();
  world.step(FIXED_TIME_STEP, 4);
  return performance.now() - start;
}

function publishSnapshot(stepMs: number, steps: number, lagMs: number, droppedMs: number): void {
  if (world === null || batch === null || positions === null || rotations === null || awake === null || state === null) return;
  world.writeBodyTransforms(dominoCount, batch.bodyHandlesPtr, batch.positionsPtr, batch.rotationsPtr, batch.awakePtr);
  const memory = world.getMemoryView();
  positions.set(new Float32Array(memory.heapF32.buffer, batch.positionsPtr, dominoCount * 3));
  rotations.set(new Float32Array(memory.heapF32.buffer, batch.rotationsPtr, dominoCount * 4));
  awake.set(new Uint8Array(memory.heapU8.buffer, batch.awakePtr, dominoCount));
  let awakeCount = 0;
  for (let i = 0; i < dominoCount; i++) awakeCount += awake[i] !== 0 ? 1 : 0;
  const projectileCount = projectileHandles.length;
  if (projectileCount > 0 && projectileBatch !== null && projectilePositions !== null && projectileRotations !== null && projectileAwake !== null) {
    world.writeBodyTransforms(projectileCount, projectileBatch.bodyHandlesPtr, projectileBatch.positionsPtr, projectileBatch.rotationsPtr, projectileBatch.awakePtr);
    projectilePositions.set(new Float32Array(memory.heapF32.buffer, projectileBatch.positionsPtr, projectileCount * 3).subarray(0, projectileCount * 3));
    projectileRotations.set(new Float32Array(memory.heapF32.buffer, projectileBatch.rotationsPtr, projectileCount * 4).subarray(0, projectileCount * 4));
    projectileAwake.set(new Uint8Array(memory.heapU8.buffer, projectileBatch.awakePtr, projectileCount).subarray(0, projectileCount));
  }
  Atomics.store(state, SNAPSHOT_AWAKE_COUNT_INDEX, awakeCount);
  Atomics.store(state, SNAPSHOT_PROJECTILE_COUNT_INDEX, projectileCount);
  Atomics.store(state, SNAPSHOT_STEP_MS_X100_INDEX, Math.round(stepMs * 100));
  Atomics.store(state, SNAPSHOT_LAG_MS_X100_INDEX, Math.round(lagMs * 100));
  Atomics.store(state, SNAPSHOT_STEPS_INDEX, steps);
  Atomics.store(state, SNAPSHOT_DROPPED_MS_X100_INDEX, Math.round(droppedMs * 100));
  Atomics.add(state, SNAPSHOT_VERSION_INDEX, 1);
}

function stepOnce(): void {
  const stepMs = stepPhysics();
  publishSnapshot(stepMs, 1, accumulator * 1000, 0);
}

function tick(): void {
  if (paused) {
    lastTickTime = performance.now();
    return;
  }
  const now = performance.now();
  if (lastTickTime === 0) lastTickTime = now;
  accumulator += Math.min((now - lastTickTime) / 1000, 0.25);
  lastTickTime = now;

  let steps = 0;
  let stepMs = 0;
  while (accumulator >= FIXED_TIME_STEP && steps < MAX_CATCHUP_STEPS) {
    stepMs = stepPhysics();
    accumulator -= FIXED_TIME_STEP;
    steps++;
  }

  let droppedMs = 0;
  if (steps === MAX_CATCHUP_STEPS && accumulator >= FIXED_TIME_STEP) {
    droppedMs = accumulator * 1000;
    accumulator = 0;
  }

  if (steps > 0) publishSnapshot(stepMs, steps, accumulator * 1000, droppedMs);
}

function spawnProjectile(origin: Vec3, velocity: Vec3): void {
  if (runtime === null || world === null || projectileBatch === null || projectileHandles.length >= MAX_PROJECTILES) return;
  const bodyHandle = runtime.createSphere(world.handle, {
    radius: 0.25,
    position: origin,
    velocity,
    density: 4000,
    isBullet: true,
  });
  projectileHandles.push(bodyHandle);
  world.writeBodyHandles(projectileBatch, projectileHandles);
}

function spawnRagdoll(origin: Vec3, velocity: Vec3): void {
  if (runtime === null || world === null || projectileBatch === null) return;
  const humanHandle = world.createHuman(origin, { frictionTorque: 1, hertz: 1, dampingRatio: 1, groupIndex: 0, colorize: true });
  if (humanHandle === 0) return;
  runtime.setHumanBullet(humanHandle, true);
  runtime.setHumanVelocity(humanHandle, velocity);
  const boneCount = Math.min(runtime.getHumanBoneCount(), RAGDOLL_RENDER_BONE_COUNT);
  for (let i = 0; i < boneCount && projectileHandles.length < MAX_PROJECTILES; i++) {
    const bodyHandle = runtime.getHumanBoneBody(humanHandle, i);
    if (bodyHandle !== 0) projectileHandles.push(bodyHandle);
  }
  world.writeBodyHandles(projectileBatch, projectileHandles);
}

function endDrag(): void {
  if (world === null) return;
  if (dragJoint !== 0) {
    world.destroyJoint(dragJoint);
    dragJoint = 0;
  }
  if (dragBody !== 0) {
    world.destroyBody(dragBody);
    dragBody = 0;
  }
}

function dragPoint(origin: Vec3, translation: Vec3): Vec3 {
  const len = Math.hypot(translation[0], translation[1], translation[2]) || 1;
  const scale = dragDistance / len;
  return [origin[0] + translation[0] * scale, origin[1] + translation[1] * scale, origin[2] + translation[2] * scale];
}

function startDrag(origin: Vec3, translation: Vec3): void {
  if (world === null) return;
  endDrag();
  const hit = world.rayCastClosest(origin, translation);
  if (hit === null || hit.bodyHandle === 0 || world.getBodyType(hit.bodyHandle) !== 2) return;
  const point = hit.point;
  dragDistance = Math.hypot(point[0] - origin[0], point[1] - origin[1], point[2] - origin[2]);
  dragBody = world.createBody({ type: 1, position: point });
  const localBodyPoint = world.getBodyLocalPoint(hit.bodyHandle, point);
  dragJoint = world.createMotorJoint(dragBody, hit.bodyHandle, {
    localFrameA: [0, 0, 0],
    localFrameB: localBodyPoint,
    linearHertz: 5,
    linearDampingRatio: 0.9,
    maxSpringForce: 800,
    angularHertz: 2,
    angularDampingRatio: 1,
    maxSpringTorque: 35,
  });
}

function updateDrag(origin: Vec3, translation: Vec3): void {
  if (world === null || dragBody === 0) return;
  world.setBodyTransform(dragBody, dragPoint(origin, translation));
}

function disposeWorld(): void {
  if (timer !== undefined) self.clearInterval(timer);
  timer = undefined;
  if (world !== null && batch !== null) world.freeBodyBatchBuffers(batch);
  if (world !== null && projectileBatch !== null) world.freeBodyBatchBuffers(projectileBatch);
  world?.destroy();
  world = null;
  batch = null;
  projectileBatch = null;
  projectileHandles.length = 0;
}

function createWorld(workerCount: number, count: number): void {
  if (runtime === null) return;
  disposeWorld();
  currentWorkerCount = workerCount;
  dominoCount = count;
  world = runtime.createWorld({ gravity: [0, -9.81, 0], workerCount });
  console.log("[worker]", "checkThreadingSupport:", runtime.checkThreadingSupport());
  console.log("[worker]", "workerCount:", world.getWorkerCount());

  const groundBody = world.createBody({ type: 0, position: [0, -1, 0] });
  runtime.createHullShape(groundBody, [160, 1, 160]);

      const rings = count / 180;
  const handles = new Array<number>(count);
  let idx = 0;
  for (let ring = 0; ring < rings; ring++) {
    const scale = 0.5 + ring * 0.0585;
    const radius = 7.0 + (1.5 + ring * 0.015) * ring;
    const n = 1.515 + ring * 0.03;
    for (let deg = 0; deg < 360; deg += 2) {
      const rad = deg * Math.PI / 180;
      const cs = Math.cos(rad);
      const sn = Math.sin(rad);
      const px = radius * cs + (deg * n / 716) * cs;
      const pz = radius * sn + (deg * n / 716) * sn;
      const p: [number, number, number] = [px, 0.8 * scale, pz];
      const bodyHandle = world.createBody({ type: 2, position: p, rotation: [0, -Math.sin(rad / 2), 0, Math.cos(rad / 2)], isAwake: true });
      runtime.createHullShape(bodyHandle, [0.2 * scale, 0.8 * scale, 0.05 * scale]);
      handles[idx] = bodyHandle;
      if (ring % 2 === 0 ? Math.abs(deg - 358) < 0.1 : Math.abs(deg) < 0.1) {
        const dir = ring % 2 === 0 ? -1 : 1;
        world.applyLinearImpulse(bodyHandle, [0, 0, dir * 25 * scale * scale * scale], [p[0], p[1] + 0.8 * scale, p[2]]);
      }
      idx++;
    }
  }

  batch = world.allocBodyBatchBuffers(count);
  world.writeBodyHandles(batch, handles);
  projectileBatch = world.allocBodyBatchBuffers(MAX_PROJECTILES);

  const positionBuffer = new SharedArrayBuffer(count * 3 * 4);
  const rotationBuffer = new SharedArrayBuffer(count * 4 * 4);
  const awakeBuffer = new SharedArrayBuffer(count);
  const projectilePositionBuffer = new SharedArrayBuffer(MAX_PROJECTILES * 3 * 4);
  const projectileRotationBuffer = new SharedArrayBuffer(MAX_PROJECTILES * 4 * 4);
  const projectileAwakeBuffer = new SharedArrayBuffer(MAX_PROJECTILES);
  const stateBuffer = new SharedArrayBuffer(7 * 4);
  positions = new Float32Array(positionBuffer);
  rotations = new Float32Array(rotationBuffer);
  awake = new Uint8Array(awakeBuffer);
  projectilePositions = new Float32Array(projectilePositionBuffer);
  projectileRotations = new Float32Array(projectileRotationBuffer);
  projectileAwake = new Uint8Array(projectileAwakeBuffer);
  state = new Int32Array(stateBuffer);

  stepOnce();
  post({ type: "ready", count, workerCount, positions: positionBuffer, rotations: rotationBuffer, awake: awakeBuffer, projectilePositions: projectilePositionBuffer, projectileRotations: projectileRotationBuffer, projectileAwake: projectileAwakeBuffer, state: stateBuffer });
  lastTickTime = performance.now();
  accumulator = 0;
  timer = self.setInterval(tick, 1000 / 120);
}

async function initDominoes(multiplier: number = 1, workerCount: number = 4, maxWorkers: number = 127): Promise<void> {
  runtime = await Box3DRuntime.load();
  maxWorkerCount = maxWorkers;
  createWorld(workerCount, 30 * 180 * multiplier);
}

function dispose(): void {
  disposeWorld();
  runtime?.destroy();
  runtime = null;
}

self.addEventListener("message", (event: MessageEvent<PhysicsWorkerCommand>) => {
  const command = event.data;
  try {
    if (command.type === "init-dominoes") {
      void initDominoes(command.multiplier, command.workerCount, command.maxWorkers).catch(publishError);
    } else if (command.type === "toggle-worker-count") {
      createWorld(currentWorkerCount === 1 ? maxWorkerCount : 1, dominoCount);
    } else if (command.type === "spawn-projectile") {
      spawnProjectile(command.origin, command.velocity);
    } else if (command.type === "spawn-ragdoll") {
      spawnRagdoll(command.origin, command.velocity);
    } else if (command.type === "drag-start") {
      startDrag(command.origin, command.translation);
    } else if (command.type === "drag-update") {
      updateDrag(command.origin, command.translation);
    } else if (command.type === "drag-end") {
      endDrag();
    } else if (command.type === "set-paused") {
      paused = command.paused;
    } else if (command.type === "step-once") {
      stepOnce();
    } else if (command.type === "dispose") {
      dispose();
    }
  } catch (error) {
    publishError(error);
  }
});
