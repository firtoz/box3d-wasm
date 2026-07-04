import type { PhysicsWorld } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "./physics-worker-protocol";
import { SNAPSHOT_AWAKE_COUNT_INDEX, SNAPSHOT_DROPPED_MS_X100_INDEX, SNAPSHOT_LAG_MS_X100_INDEX, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_PUBLISH_MS_X100_INDEX, SNAPSHOT_STEP_MS_X100_INDEX, SNAPSHOT_STEPS_INDEX } from "./physics-worker-protocol";

export type WorkerWorldState = {
  count: number;
  workerCount: number;
  positions: Float32Array;
  rotations: Float32Array;
  awake: Uint8Array;
  colors: Uint32Array;
  projectilePositions: Float32Array;
  projectileRotations: Float32Array;
  projectileAwake: Uint8Array;
  projectileColors: Uint32Array;
  state: Int32Array;
  extra?: Record<string, unknown>;
};

export function createWorkerWorld(
  worker: Worker,
  getState: () => WorkerWorldState | null,
  getWorkerCount: () => number,
): PhysicsWorld {
  return {
    handle: 0,
    getCounters: () => {
      const st = getState();
      const c = st?.count ?? 0;
      const pc = st === null ? 0 : Atomics.load(st.state, SNAPSHOT_PROJECTILE_COUNT_INDEX);
      return { bodyCount: c + 1 + pc, shapeCount: c + 1 + pc, contactCount: 0, jointCount: pc, islandCount: 0, staticTreeHeight: 0, treeHeight: 0 };
    },
    getAwakeBodyCount: () => {
      const st = getState();
      return st === null ? 0 : Atomics.load(st.state, SNAPSHOT_AWAKE_COUNT_INDEX);
    },
    getProfile: () => {
      const st = getState();
      const step = st === null ? 0 : Atomics.load(st.state, SNAPSHOT_STEP_MS_X100_INDEX) / 100;
      const publish = st === null ? 0 : Atomics.load(st.state, SNAPSHOT_PUBLISH_MS_X100_INDEX) / 100;
      const lag = st === null ? 0 : Atomics.load(st.state, SNAPSHOT_LAG_MS_X100_INDEX) / 100;
      const steps = st === null ? 0 : Atomics.load(st.state, SNAPSHOT_STEPS_INDEX);
      const dropped = st === null ? 0 : Atomics.load(st.state, SNAPSHOT_DROPPED_MS_X100_INDEX) / 100;
      return { step, pairs: lag, collide: dropped, solve: steps, solverSetup: publish, constraints: 0, prepareConstraints: 0, integrateVelocities: 0, warmStart: 0, solveImpulses: 0, integratePositions: 0, relaxImpulses: 0, applyRestitution: 0, storeImpulses: 0, splitIslands: 0, transforms: 0, sensorHits: 0, jointEvents: 0, hitEvents: 0, refit: 0, bullets: 0, sleepIslands: 0, sensors: 0 };
    },
    getWorkerCount: () => getWorkerCount(),
    rayCastClosest: () => null,
    destroy: () => worker.postMessage({ type: "dispose" } satisfies PhysicsWorkerCommand),
  } as unknown as PhysicsWorld;
}
