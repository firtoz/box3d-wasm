import {
  BodyType,
  type BodyHandle,
  type Box3DRuntime,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import { Box3DRng } from "./box3d-rng";
import { f32, f32Add, f32Mul, f32Sub } from "./f32";

/** Matches `MESH_DROP_GRID_COUNT` in `box3d/shared/stability.h`. */
export const MESH_DROP_GRID_COUNT = 20;
/** Seed used by `CreateMeshDrop` (`g_randomSeed = 3963634789`). */
export const MESH_DROP_SEED = 3963634789;

export const MESH_DROP_WAVE = {
  xCount: 40,
  zCount: 40,
  cellWidth: f32(1),
  amplitude: f32(0.5),
  rowFrequency: f32(0.1),
  columnFrequency: f32(0.2),
} as const;

const BOX_HALF: Vec3 = [f32(0.02), f32(0.2), f32(0.04)];

export interface MeshDropResult {
  ground: BodyHandle;
  bodies: BodyHandle[];
  mesh: MeshHandle;
}

/**
 * Port of `CreateMeshDrop` from `box3d/shared/stability.c`.
 * Creates wave-mesh ground at `origin` plus a `MESH_DROP_GRID_COUNT`² box grid.
 */
export function createMeshDrop(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  origin: Vec3,
  seed: number = MESH_DROP_SEED,
): MeshDropResult {
  const ground = world.createBody({ type: BodyType.Static, position: origin });
  const mesh = world.createWaveMesh(
    MESH_DROP_WAVE.xCount,
    MESH_DROP_WAVE.zCount,
    MESH_DROP_WAVE.cellWidth,
    MESH_DROP_WAVE.amplitude,
    MESH_DROP_WAVE.rowFrequency,
    MESH_DROP_WAVE.columnFrequency,
  );
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1], categoryBits: 1 });

  const rng = new Box3DRng(seed);
  const grid = MESH_DROP_GRID_COUNT;
  const halfGrid = f32Mul(0.5, grid);
  const bodies: BodyHandle[] = [];

  for (let i = 0; i < grid; i++) {
    for (let j = 0; j < grid; j++) {
      const linearVelocity = rng.randomVec3Uniform(-1, 1);
      const angularVelocity = rng.randomVec3Uniform(-5, 5);
      const ox = f32Mul(0.5, f32Sub(f32(i), halfGrid));
      const oz = f32Mul(0.5, f32Sub(f32(j), halfGrid));
      const position: Vec3 = [f32Add(origin[0], ox), f32Add(origin[1], f32(5)), f32Add(origin[2], oz)];
      const body = world.createBody({
        type: BodyType.Dynamic,
        position,
        linearVelocity,
        angularVelocity,
      });
      runtime.createHullShape(body, BOX_HALF, {
        rollingResistance: 0.1,
        categoryBits: 2,
        maskBits: 1,
      });
      bodies.push(body);
    }
  }

  return { ground, bodies, mesh };
}

export function meshDropBoxSize(): [number, number, number] {
  return [2 * BOX_HALF[0], 2 * BOX_HALF[1], 2 * BOX_HALF[2]];
}

export function createMeshDropRenderBodies(origin: Vec3, color = 0x60a5fa): import("./generic-host").RenderBody[] {
  const grid = MESH_DROP_GRID_COUNT;
  const halfGrid = f32Mul(0.5, grid);
  const size = meshDropBoxSize();
  const bodies: import("./generic-host").RenderBody[] = [];
  for (let i = 0; i < grid; i++) {
    for (let j = 0; j < grid; j++) {
      const ox = f32Mul(0.5, f32Sub(f32(i), halfGrid));
      const oz = f32Mul(0.5, f32Sub(f32(j), halfGrid));
      bodies.push({
        kind: "box",
        size,
        position: [f32Add(origin[0], ox), f32Add(origin[1], f32(5)), f32Add(origin[2], oz)],
        color,
      });
    }
  }
  return bodies;
}
