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
  | { type: "init"; data: unknown; workerCount?: number; maxWorkers?: number; poolSize?: number; solverParams?: SolverParams; wasmVersion?: string; wasmVariant?: RuntimeLoadOptions["variant"]; wasmBaseUrl?: string }
  | { type: "spawn-projectile"; origin: Vec3; velocity: Vec3 }
  | { type: "spawn-ragdoll"; origin: Vec3; velocity: Vec3 }
  | { type: "drag-start"; origin: Vec3; translation: Vec3 }
  | { type: "drag-update"; origin: Vec3; translation: Vec3 }
  | { type: "drag-end" }
  | { type: "set-paused"; paused: boolean }
  | { type: "step-once" }
  | { type: "toggle-worker-count" }
  | { type: "set-solver-params"; params: SolverParams }
  | { type: "set-color-mode"; mode: "light" | "full" }
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
/** b3Profile.step (ms × 100), summed across catch-up steps in a publish. */
export const SNAPSHOT_STEP_MS_X100_INDEX = 3;
/** b3Profile.pairs (ms × 100) */
export const SNAPSHOT_PAIRS_MS_X100_INDEX = 4;
/** b3Profile.collide (ms × 100) */
export const SNAPSHOT_COLLIDE_MS_X100_INDEX = 5;
/** b3Profile.solve (ms × 100) */
export const SNAPSHOT_SOLVE_MS_X100_INDEX = 6;
/** Host-side transform publish cost (ms × 100); web-only, not from b3Profile. */
export const SNAPSHOT_PUBLISH_MS_X100_INDEX = 7;
/** Physics steps executed in the last published tick (catch-up count). */
export const SNAPSHOT_STEPS_INDEX = 8;
export const SNAPSHOT_CUMULATIVE_STEPS_INDEX = 9;
/** Active tracked body count written into the shared snapshot (may grow up to ready.count). */
export const SNAPSHOT_BODY_COUNT_INDEX = 10;
export const SNAPSHOT_STATE_COUNT = 11;
export const MAX_PROJECTILES = 2048;
export const RAGDOLL_RENDER_BONE_COUNT = 14;
