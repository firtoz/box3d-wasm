import { BodyType, type BodyId, type Box3DRuntime, type MeshHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32Add, f32Div, f32Mul, f32Sub } from "../f32";

const SRC = 0.0254;
const HALF_LENGTH_U = 256;
const HALF_WIDTH_U = 64;
const TILE_SIZE_U = 32;
const BEAM_PITCH_U = 22;
const BEAM_WIDTH_U = 12;
const CHAMFER_WIDTH_U = 1.5;
const CHAMFER_DROP_U = 1;
const PIT_DEPTH_U = 24;
const BEAM_COUNT = 9;
const BEAM_REGION0 = -94;
const BEAM_REGION1 = 94;
const BODY_HALF_WIDTH = f32Mul(16, SRC);
const BODY_HALF_HEIGHT = f32Mul(36, SRC);
const CHARACTER_MASS = 500;
const WALK_RANGE_X = 3.5;
const WALK_RANGE_Z = 0.5;
const WALK_SPEED_X = f32Mul(350, SRC);
const WALK_SPEED_Z = f32Mul(20, SRC);

export type GhostWalkState = { dirX: number; dirZ: number };

function hashU32(x: number): number {
  let v = x >>> 0;
  v ^= v >>> 16;
  v = Math.imul(v, 0x7feb352d) >>> 0;
  v ^= v >>> 15;
  v = Math.imul(v, 0x846ca68b) >>> 0;
  v ^= v >>> 16;
  return v >>> 0;
}

function emitTriangle(vertices: number[], indices: number[], a: Vec3, b: Vec3, c: Vec3): void {
  const base = vertices.length / 3;
  vertices.push(a[0], a[1], a[2], b[0], b[1], b[2], c[0], c[1], c[2]);
  indices.push(base, base + 1, base + 2);
}

function emitPatch(vertices: number[], indices: number[], x0: number, x1: number, z0: number, z1: number, y: number, cell: number): void {
  if (f32Sub(x1, x0) < 0.01) return;
  const countX = Math.trunc(f32Add(f32Div(f32Sub(x1, x0), cell), 0.99));
  const countZ = Math.trunc(f32Add(f32Div(f32Sub(z1, z0), cell), 0.99));
  for (let ix = 0; ix < countX; ix++) {
    for (let iz = 0; iz < countZ; iz++) {
      const cx0 = f32Add(x0, f32Mul(f32Sub(x1, x0), f32Div(ix, countX)));
      const cx1 = f32Add(x0, f32Mul(f32Sub(x1, x0), f32Div(ix + 1, countX)));
      const cz0 = f32Add(z0, f32Mul(f32Sub(z1, z0), f32Div(iz, countZ)));
      const cz1 = f32Add(z0, f32Mul(f32Sub(z1, z0), f32Div(iz + 1, countZ)));
      const a: Vec3 = [f32Mul(SRC, cx0), f32Mul(SRC, y), f32Mul(SRC, cz0)];
      const b: Vec3 = [f32Mul(SRC, cx1), f32Mul(SRC, y), f32Mul(SRC, cz0)];
      const c: Vec3 = [f32Mul(SRC, cx1), f32Mul(SRC, y), f32Mul(SRC, cz1)];
      const d: Vec3 = [f32Mul(SRC, cx0), f32Mul(SRC, y), f32Mul(SRC, cz1)];
      if ((ix + iz) & 1) {
        emitTriangle(vertices, indices, a, d, c);
        emitTriangle(vertices, indices, a, c, b);
      } else {
        emitTriangle(vertices, indices, a, d, b);
        emitTriangle(vertices, indices, b, d, c);
      }
    }
  }
}

function emitSlope(vertices: number[], indices: number[], xLow: number, yLow: number, xHigh: number, yHigh: number, zCell: number): void {
  const countZ = Math.trunc(f32Add(f32Div(f32Mul(2, HALF_WIDTH_U), zCell), 0.99));
  for (let iz = 0; iz < countZ; iz++) {
    const z0 = f32Add(-HALF_WIDTH_U, f32Mul(f32Mul(2, HALF_WIDTH_U), f32Div(iz, countZ)));
    const z1 = f32Add(-HALF_WIDTH_U, f32Mul(f32Mul(2, HALF_WIDTH_U), f32Div(iz + 1, countZ)));
    const l0: Vec3 = [f32Mul(SRC, xLow), f32Mul(SRC, yLow), f32Mul(SRC, z0)];
    const l1: Vec3 = [f32Mul(SRC, xLow), f32Mul(SRC, yLow), f32Mul(SRC, z1)];
    const h0: Vec3 = [f32Mul(SRC, xHigh), f32Mul(SRC, yHigh), f32Mul(SRC, z0)];
    const h1: Vec3 = [f32Mul(SRC, xHigh), f32Mul(SRC, yHigh), f32Mul(SRC, z1)];
    emitTriangle(vertices, indices, l0, l1, h1);
    emitTriangle(vertices, indices, l0, h1, h0);
  }
}

function emitWall(vertices: number[], indices: number[], x: number, y0: number, y1: number, facing: number, zCell: number): void {
  const countZ = Math.trunc(f32Add(f32Div(f32Mul(2, HALF_WIDTH_U), zCell), 0.99));
  for (let iz = 0; iz < countZ; iz++) {
    const z0 = f32Add(-HALF_WIDTH_U, f32Mul(f32Mul(2, HALF_WIDTH_U), f32Div(iz, countZ)));
    const z1 = f32Add(-HALF_WIDTH_U, f32Mul(f32Mul(2, HALF_WIDTH_U), f32Div(iz + 1, countZ)));
    const b0: Vec3 = [f32Mul(SRC, x), f32Mul(SRC, y0), f32Mul(SRC, z0)];
    const b1: Vec3 = [f32Mul(SRC, x), f32Mul(SRC, y0), f32Mul(SRC, z1)];
    const t0: Vec3 = [f32Mul(SRC, x), f32Mul(SRC, y1), f32Mul(SRC, z0)];
    const t1: Vec3 = [f32Mul(SRC, x), f32Mul(SRC, y1), f32Mul(SRC, z1)];
    if (facing > 0) {
      emitTriangle(vertices, indices, b0, b1, t1);
      emitTriangle(vertices, indices, b0, t1, t0);
    } else {
      emitTriangle(vertices, indices, b0, t0, t1);
      emitTriangle(vertices, indices, b0, t1, b1);
    }
  }
}

function clipSpan(a0: number, a1: number, c0: number, c1: number): [number, number] | null {
  const o0 = a0 > c0 ? a0 : c0;
  const o1 = a1 < c1 ? a1 : c1;
  if (f32Sub(o1, o0) <= 0.01) return null;
  return [o0, o1];
}

function createFloorChunk(world: PhysicsWorld, chunk: number, x0U: number, x1U: number): { body: BodyId; mesh: MeshHandle } {
  const vertices: number[] = [];
  const indices: number[] = [];
  const slabSpans: [number, number][] = [[-HALF_LENGTH_U, BEAM_REGION0], [BEAM_REGION1, HALF_LENGTH_U]];
  for (let i = 0; i < 2; i++) {
    const span = clipSpan(slabSpans[i]![0], slabSpans[i]![1], x0U, x1U);
    if (span === null) continue;
    const [s0, s1] = span;
    for (let tx = s0; tx < s1; tx = f32Add(tx, TILE_SIZE_U)) {
      const tx1 = Math.min(f32Add(tx, TILE_SIZE_U), s1);
      for (let tz = -HALF_WIDTH_U; tz < HALF_WIDTH_U; tz += TILE_SIZE_U) {
        const h = hashU32((((Math.trunc(tx) * 73856093) ^ (tz * 19349663) ^ Math.imul(chunk, 2654435761)) >>> 0));
        const cells = [4, 8, 16];
        emitPatch(vertices, indices, tx, tx1, tz, tz + TILE_SIZE_U, 0, cells[h % 3]!);
      }
    }
  }

  const pitTop = -CHAMFER_DROP_U;
  const pitBottom = -PIT_DEPTH_U;
  for (let k = 0; k < BEAM_COUNT; k++) {
    const bx = f32Add(BEAM_REGION0, f32Mul(BEAM_PITCH_U, k));
    const pitLeft = k > 0;
    const pitRight = k < BEAM_COUNT - 1;
    const top0 = pitLeft ? f32Add(bx, CHAMFER_WIDTH_U) : bx;
    const top1 = pitRight ? f32Sub(f32Add(bx, BEAM_WIDTH_U), CHAMFER_WIDTH_U) : f32Add(bx, BEAM_WIDTH_U);
    const topSpan = clipSpan(top0, top1, x0U, x1U);
    if (topSpan !== null) emitPatch(vertices, indices, topSpan[0], topSpan[1], -HALF_WIDTH_U, HALF_WIDTH_U, 0, 8);
    if (pitLeft && bx >= x0U && bx < x1U) emitSlope(vertices, indices, bx, pitTop, f32Add(bx, CHAMFER_WIDTH_U), 0, 8);
    if (pitRight && f32Add(bx, BEAM_WIDTH_U) > x0U && f32Add(bx, BEAM_WIDTH_U) <= x1U) {
      emitSlope(vertices, indices, f32Add(bx, BEAM_WIDTH_U), pitTop, f32Sub(f32Add(bx, BEAM_WIDTH_U), CHAMFER_WIDTH_U), 0, 8);
    }
    if (pitRight) {
      const pitL = f32Add(bx, BEAM_WIDTH_U);
      const pitR = f32Add(bx, BEAM_PITCH_U);
      if (pitL >= x0U && pitL < x1U) emitWall(vertices, indices, pitL, pitBottom, pitTop, 1, 16);
      if (pitR > x0U && pitR <= x1U) emitWall(vertices, indices, pitR, pitBottom, pitTop, -1, 16);
      const pitSpan = clipSpan(pitL, pitR, x0U, x1U);
      if (pitSpan !== null) emitPatch(vertices, indices, pitSpan[0], pitSpan[1], -HALF_WIDTH_U, HALF_WIDTH_U, pitBottom, 16);
    }
  }

  const mesh = world.createMesh(vertices, indices, { weldVertices: true, weldTolerance: 0.005, identifyEdges: true });
  const body = world.createBody();
  world.createMeshShape(body, mesh, { scale: [1, 1, 1] });
  return { body, mesh };
}

export function buildSboxGhostDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): {
  handles: BodyId[];
  meshes: MeshHandle[];
  character: BodyId;
} {
  const left = createFloorChunk(world, 0, -HALF_LENGTH_U, 0);
  const right = createFloorChunk(world, 1, 0, HALF_LENGTH_U);
  const character = world.createBody({
    type: BodyType.Dynamic,
    position: [-WALK_RANGE_X, f32Add(BODY_HALF_HEIGHT, 0.1), 0],
    enableSleep: false,
    enableContactRecycling: false,
    gravityScale: 2.03,
  });
  runtime.setBodyMotionLocks(character, { lockRotationX: true, lockRotationY: true, lockRotationZ: true });
  const volume = f32Mul(f32Mul(f32Mul(8, BODY_HALF_WIDTH), BODY_HALF_HEIGHT), BODY_HALF_WIDTH);
  runtime.createHullShape(character, [BODY_HALF_WIDTH, BODY_HALF_HEIGHT, BODY_HALF_WIDTH], {
    friction: 0,
    restitution: 0,
    density: CHARACTER_MASS / volume,
    enableSpeculativeContact: false,
  });
  return { handles: [left.body, right.body, character], meshes: [left.mesh, right.mesh], character };
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[]; state: GhostWalkState } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const built = buildSboxGhostDynamicBodies(world, runtime);
  return { world, handles: built.handles, state: { dirX: 1, dirZ: 1 } };
}

export function dumpStep(world: PhysicsWorld, _runtime: Box3DRuntime, handles: readonly BodyId[], _frame: number, _dt: number, state: unknown): void {
  const walk = state as GhostWalkState;
  const character = handles[handles.length - 1]!;
  const p = world.getBodyTransform(character).position;
  if (p[0] > WALK_RANGE_X) walk.dirX = -1;
  else if (p[0] < -WALK_RANGE_X) walk.dirX = 1;
  if (p[2] > WALK_RANGE_Z) walk.dirZ = -1;
  else if (p[2] < -WALK_RANGE_Z) walk.dirZ = 1;
  const v = world.getBodyLinearVelocity(character);
  world.setBodyLinearVelocity(character, [f32Mul(walk.dirX, WALK_SPEED_X), v[1], f32Mul(walk.dirZ, WALK_SPEED_Z)]);
}

export function sboxGhostGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const sboxGhostBodies: RenderBody[] = [
  { kind: "box", size: [13, 0.05, 3.25], position: [0, 0, 0], type: BodyType.Static, color: 0x475569 },
  { kind: "box", size: [2 * BODY_HALF_WIDTH, 2 * BODY_HALF_HEIGHT, 2 * BODY_HALF_WIDTH], position: [-WALK_RANGE_X, BODY_HALF_HEIGHT + 0.1, 0], color: 0x22c55e },
];

export const sboxGhostCamera: RenderSpec["camera"] = cameraFromSetView(90, 25, 10, [0, 1, 0]);

export const dumpSampleName = "s&box Ghost Collisions";
export const dumpSampleId = "issues/sbox-ghost-collisions";
export const dumpCppSampleName = "s&box Ghost Collisions";
export const dumpGroundSize = sboxGhostGroundSize;
export { WALK_SPEED_X, WALK_SPEED_Z, SRC };
