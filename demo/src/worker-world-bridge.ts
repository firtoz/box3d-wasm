import type { PhysicsWorld } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "./physics-worker-protocol";
import {
  SNAPSHOT_AWAKE_COUNT_INDEX,
  SNAPSHOT_BODY_COUNT_INDEX,
  SNAPSHOT_COLLIDE_MS_X100_INDEX,
  SNAPSHOT_PAIRS_MS_X100_INDEX,
  SNAPSHOT_PROJECTILE_COUNT_INDEX,
  SNAPSHOT_PUBLISH_MS_X100_INDEX,
  SNAPSHOT_SOLVE_MS_X100_INDEX,
  SNAPSHOT_STEP_MS_X100_INDEX,
  SNAPSHOT_STEPS_INDEX,
} from "./physics-worker-protocol";

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
      const c = st === null ? 0 : Atomics.load(st.state, SNAPSHOT_BODY_COUNT_INDEX);
      const pc = st === null ? 0 : Atomics.load(st.state, SNAPSHOT_PROJECTILE_COUNT_INDEX);
      return { bodyCount: c + pc, shapeCount: c + pc, contactCount: 0, jointCount: pc, islandCount: 0, staticTreeHeight: 0, treeHeight: 0 };
    },
    getAwakeBodyCount: () => {
      const st = getState();
      return st === null ? 0 : Atomics.load(st.state, SNAPSHOT_AWAKE_COUNT_INDEX);
    },
    getProfile: () => {
      const st = getState();
      const load = (index: number): number => (st === null ? 0 : Atomics.load(st.state, index) / 100);
      return {
        step: load(SNAPSHOT_STEP_MS_X100_INDEX),
        pairs: load(SNAPSHOT_PAIRS_MS_X100_INDEX),
        collide: load(SNAPSHOT_COLLIDE_MS_X100_INDEX),
        solve: load(SNAPSHOT_SOLVE_MS_X100_INDEX),
        // Web-only bridge publish cost, parked here so the HUD can show it without a second API.
        solverSetup: load(SNAPSHOT_PUBLISH_MS_X100_INDEX),
        // Catch-up step count for the last published tick (not milliseconds).
        constraints: st === null ? 0 : Atomics.load(st.state, SNAPSHOT_STEPS_INDEX),
        prepareConstraints: 0,
        integrateVelocities: 0,
        warmStart: 0,
        solveImpulses: 0,
        integratePositions: 0,
        relaxImpulses: 0,
        applyRestitution: 0,
        storeImpulses: 0,
        splitIslands: 0,
        transforms: 0,
        sensorHits: 0,
        jointEvents: 0,
        hitEvents: 0,
        refit: 0,
        bullets: 0,
        sleepIslands: 0,
        sensors: 0,
      };
    },
    getWorkerCount: () => getWorkerCount(),
    rayCastClosest: () => null,
    destroy: () => worker.postMessage({ type: "dispose" } satisfies PhysicsWorkerCommand),
  } as unknown as PhysicsWorld;
}
