import { MAX_PROJECTILES, type PhysicsWorkerReady } from "./physics-worker-protocol";

const PUBLISH_LOCK_INDEX = 0;

/**
 * Try to acquire the snapshot publication lock.
 * Main thread must hold this lock across attribute upload when using heap backing
 * (views alias the live WASM heap). Worker also acquires before writing.
 */
export function tryAcquirePublishLock(lock: Int32Array): boolean {
  return Atomics.compareExchange(lock, PUBLISH_LOCK_INDEX, 0, 1) === 0;
}

export function releasePublishLock(lock: Int32Array): void {
  Atomics.store(lock, PUBLISH_LOCK_INDEX, 0);
}

export type WorkerSnapshotViews = {
  positions: Float32Array;
  rotations: Float32Array;
  awake: Uint8Array;
  colors: Uint32Array;
  projectilePositions: Float32Array;
  projectileRotations: Float32Array;
  projectileAwake: Uint8Array;
  projectileColors: Uint32Array;
  state: Int32Array;
  publishLock: Int32Array;
  snapshotBacking: "heap" | "external";
};

/**
 * Build typed-array views from a worker ready message.
 * Heap mode uses wasmMemory + byte offsets; external mode uses dedicated SABs.
 */
export function createWorkerSnapshotViews(ready: PhysicsWorkerReady): WorkerSnapshotViews {
  const capacity = ready.count;
  const state = new Int32Array(ready.state);
  const publishLock = new Int32Array(ready.publishLock);

  if (ready.snapshotBacking === "heap") {
    const memory = ready.wasmMemory;
    const heap = ready.heap;
    if (memory === undefined || heap === undefined) {
      throw new Error("Heap snapshot ready message missing wasmMemory/heap offsets");
    }
    return {
      positions: new Float32Array(memory, heap.positions, capacity * 3),
      rotations: new Float32Array(memory, heap.rotations, capacity * 4),
      awake: new Uint8Array(memory, heap.awake, capacity),
      colors: new Uint32Array(memory, heap.colors, capacity),
      projectilePositions: new Float32Array(memory, heap.projectilePositions, MAX_PROJECTILES * 3),
      projectileRotations: new Float32Array(memory, heap.projectileRotations, MAX_PROJECTILES * 4),
      projectileAwake: new Uint8Array(memory, heap.projectileAwake, MAX_PROJECTILES),
      projectileColors: new Uint32Array(memory, heap.projectileColors, MAX_PROJECTILES),
      state,
      publishLock,
      snapshotBacking: "heap",
    };
  }

  if (
    ready.positions === undefined
    || ready.rotations === undefined
    || ready.awake === undefined
    || ready.colors === undefined
    || ready.projectilePositions === undefined
    || ready.projectileRotations === undefined
    || ready.projectileAwake === undefined
    || ready.projectileColors === undefined
  ) {
    throw new Error("External snapshot ready message missing SharedArrayBuffers");
  }

  return {
    positions: new Float32Array(ready.positions),
    rotations: new Float32Array(ready.rotations),
    awake: new Uint8Array(ready.awake),
    colors: new Uint32Array(ready.colors),
    projectilePositions: new Float32Array(ready.projectilePositions),
    projectileRotations: new Float32Array(ready.projectileRotations),
    projectileAwake: new Uint8Array(ready.projectileAwake),
    projectileColors: new Uint32Array(ready.projectileColors),
    state,
    publishLock,
    snapshotBacking: "external",
  };
}
