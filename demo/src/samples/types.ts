import type * as THREE from "three";
import type { BodyHandle, BodyType, Box3DRuntime, PhysicsWorld, ShapeId, Vec3 } from "box3d-wasm";

export type DemoBody = { handle: BodyHandle; mesh: THREE.Mesh; extraMeshes?: THREE.Mesh[]; shapeIds?: ShapeId[]; type: BodyType; preserveColor?: boolean };

export type ControlSpec = {
  key: string;
  label: string;
  type?: "range" | "toggle" | "button";
  min?: number;
  max?: number;
  step?: number;
  value?: number | boolean;
  onChange?: (value: number | boolean) => void;
  onClick?: () => void;
};

export type SolverParams = {
  subSteps?: number;
  hertz?: number;
  recycleDistance?: number;
  sleep?: boolean;
  continuous?: boolean;
  warmStart?: boolean;
  workerCount?: number;
};

export type DemoSampleInstance = {
  world: PhysicsWorld;
  bodies: DemoBody[];
  controls: ControlSpec[];
  launchSpeed?: number;
  info?: string;
  getInfo?(): string | undefined;
  camera?: { position: [number, number, number]; target: [number, number, number] };
  profile?: boolean;
  spawnProjectile?: (origin: Vec3, velocity: Vec3, spin: boolean, ragdoll: boolean) => void;
  startMouseDragRay?: (origin: Vec3, translation: Vec3) => boolean;
  updateMouseDragRay?: (origin: Vec3, translation: Vec3) => void;
  stopMouseDrag?: () => void;
  onKey?: (key: string) => void;
  setPaused?(paused: boolean): void;
  stepOnce?(): void;
  sendSolverParams?: (params: SolverParams) => void;
  step(dt?: number, subSteps?: number): void;
  dispose(): void;
};

export type DemoSample = {
  id: string;
  name: string;
  create(runtime: Box3DRuntime, scene: THREE.Scene, solverParams?: SolverParams): DemoSampleInstance;
};

export type SampleId = DemoSample["id"];
