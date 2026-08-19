import {
  B3_AXIS_Y,
  quatFromAxisAngle,
  type BodyId,
  type Box3DRuntime,
  type LocalManifold,
  type PhysicsWorld,
  type Vec3,
  type WorldTransform,
} from "box3d-wasm";
import { cameraFromSetView } from "../shared";

export const IDENTITY_XF: WorldTransform = { position: [0, 0, 0], rotation: [0, 0, 0, 1] };
export const DEFAULT_MANIFOLD_A: WorldTransform = {
  position: [3.5, 0.5, 0],
  rotation: quatFromAxisAngle(B3_AXIS_Y, 0.5 * Math.PI),
};
export const DEFAULT_MANIFOLD_B: WorldTransform = { position: [0, 1.5, 3.5], rotation: [0, 0, 0, 1] };

export function defaultManifoldA(runtime: Box3DRuntime): WorldTransform {
  return { position: [3.5, 0.5, 0], rotation: runtime.makeQuatFromAxisAngle([0, 1, 0], 0.5 * Math.PI) };
}

export function emptyLocalManifold(): LocalManifold {
  return {
    normal: [0, 0, 0],
    triangleNormal: [0, 0, 0],
    pointCount: 0,
    feature: 0,
    triangleIndex: 0,
    indices: [0, 0, 0],
    squaredDistance: 0,
    triangleFlags: 0,
    points: [],
  };
}

export function manifoldDumpJson(manifold: LocalManifold): Record<string, unknown> {
  return {
    manifold: {
      n: manifold.normal,
      tn: manifold.triangleNormal,
      c: Math.trunc(manifold.pointCount),
      f: Math.trunc(manifold.feature),
      ti: Math.trunc(manifold.triangleIndex),
      i: manifold.indices.map((index) => Math.trunc(index)),
      d: manifold.squaredDistance,
      tf: Math.trunc(manifold.triangleFlags),
      pts: manifold.points.map((point) => ({
        p: point.point,
        s: point.separation,
        pr: point.pair.map((value) => Math.trunc(value)),
      })),
    },
  };
}

export type ManifoldDraw =
  | { kind: "sphere"; transform: WorldTransform; center: Vec3; radius: number; color: number; opacity?: number }
  | { kind: "capsule"; transform: WorldTransform; center1: Vec3; center2: Vec3; radius: number; color: number; opacity?: number }
  | { kind: "box"; transform: WorldTransform; size: Vec3; color: number; localPosition?: Vec3 }
  | { kind: "triangle"; transform: WorldTransform; vertices: readonly [Vec3, Vec3, Vec3]; color: number };

export type ManifoldResources = {
  collide: (runtime: Box3DRuntime) => LocalManifold;
  dispose: () => void;
  contactFrame: WorldTransform;
};

export type ManifoldCamera = { position: [number, number, number]; target: [number, number, number] };

export type ManifoldScene = {
  id: string;
  name: string;
  cppName: string;
  info: string;
  camera: ManifoldCamera;
  draw: ManifoldDraw[];
  create: (runtime: Box3DRuntime) => ManifoldResources;
};

export function dumpCreateManifold(runtime: Box3DRuntime, scene: ManifoldScene): {
  world: PhysicsWorld;
  handles: BodyId[];
  state: { manifold: LocalManifold; resources: ManifoldResources };
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const resources = scene.create(runtime);
  return {
    world,
    handles: [],
    state: { manifold: emptyLocalManifold(), resources },
    dispose: () => resources.dispose(),
  };
}

export function dumpStepManifold(
  _world: PhysicsWorld,
  runtime: Box3DRuntime,
  _handles: readonly BodyId[],
  _frame: number,
  _dt: number,
  state: { manifold: LocalManifold; resources: ManifoldResources },
): void {
  state.manifold = state.resources.collide(runtime);
}

export function dumpCheckpointManifold(
  _world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly BodyId[],
  _frame: number,
  state: { manifold: LocalManifold; resources: ManifoldResources },
): Record<string, unknown> {
  return manifoldDumpJson(state.manifold);
}

export function manifoldCamera(yaw: number, pitch: number, radius: number, pivot: Vec3 = [0, 5, 0]): ManifoldCamera {
  return cameraFromSetView(yaw, pitch, radius, pivot);
}
