import type { RuntimeLoadOptions, Vec3 } from "box3d-wasm";

export type SolverParams = {
  subSteps?: number;
  hertz?: number;
  recycleDistance?: number;
  sleep?: boolean;
  continuous?: boolean;
  warmStart?: boolean;
  workerCount?: number;
};

export type PhysicsWorkerCommand =
  | { type: "init"; data: unknown; workerCount?: number; maxWorkers?: number; poolSize?: number; solverParams?: SolverParams; wasmVersion?: string; wasmVariant?: RuntimeLoadOptions["variant"] }
  | { type: "spawn-projectile"; origin: Vec3; velocity: Vec3 }
  | { type: "spawn-ragdoll"; origin: Vec3; velocity: Vec3 }
  | { type: "drag-start"; origin: Vec3; translation: Vec3 }
  | { type: "drag-update"; origin: Vec3; translation: Vec3 }
  | { type: "drag-end" }
  | { type: "set-paused"; paused: boolean }
  | { type: "step-once" }
  | { type: "toggle-worker-count" }
  | { type: "set-solver-params"; params: SolverParams }
  | { type: "dispose" };

export type PhysicsWorkerReady = {
  type: "ready";
  count: number;
  workerCount: number;
  positions: SharedArrayBuffer;
  rotations: SharedArrayBuffer;
  awake: SharedArrayBuffer;
  colors: SharedArrayBuffer;
  projectilePositions: SharedArrayBuffer;
  projectileRotations: SharedArrayBuffer;
  projectileAwake: SharedArrayBuffer;
  projectileColors: SharedArrayBuffer;
  state: SharedArrayBuffer;
  extra?: Record<string, unknown>;
};

export type PhysicsWorkerError = { type: "error"; message: string };

export type PhysicsWorkerMessage =
  | PhysicsWorkerReady
  | PhysicsWorkerError;

export const SNAPSHOT_VERSION_INDEX = 0;
export const SNAPSHOT_AWAKE_COUNT_INDEX = 1;
export const SNAPSHOT_PROJECTILE_COUNT_INDEX = 2;
export const SNAPSHOT_STEP_MS_X100_INDEX = 3;
export const SNAPSHOT_LAG_MS_X100_INDEX = 4;
export const SNAPSHOT_STEPS_INDEX = 5;
export const SNAPSHOT_DROPPED_MS_X100_INDEX = 6;
export const SNAPSHOT_CUMULATIVE_STEPS_INDEX = 7;
export const SNAPSHOT_PUBLISH_MS_X100_INDEX = 8;
export const SNAPSHOT_STATE_COUNT = 10;
export const MAX_PROJECTILES = 2048;
export const RAGDOLL_RENDER_BONE_COUNT = 14;
