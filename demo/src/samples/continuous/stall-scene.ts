import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "../f32";
import { cameraFromSetView } from "../shared";

/** Match `b3CreateRock` Fibonacci lattice (10 points) for hull rendering. */
export function rockHullPoints(radius: number): [number, number, number][] {
  const pointCount = 10;
  const phi = f32Div(f32Add(1, Math.fround(Math.sqrt(5))), 2);
  const theta = f32Mul(f32Mul(2, Math.fround(Math.PI)), f32Div(1, phi));
  let cosine = 1;
  let sine = 0;
  const deltaCos = Math.fround(Math.cos(theta));
  const deltaSin = Math.fround(Math.sin(theta));
  const points: [number, number, number][] = [];
  const r = f32(radius);
  for (let i = 0; i < pointCount; i++) {
    const z = f32Sub(1, f32Div(f32Add(f32Mul(2, i), 1), pointCount));
    const radiusXY = Math.fround(Math.sqrt(f32Sub(1, f32Mul(z, z))));
    points.push([
      f32Mul(r, f32Mul(radiusXY, cosine)),
      f32Mul(r, f32Mul(radiusXY, sine)),
      f32Mul(r, z),
    ]);
    const nextCos = f32Sub(f32Mul(deltaCos, cosine), f32Mul(deltaSin, sine));
    const nextSin = f32Add(f32Mul(deltaSin, cosine), f32Mul(deltaCos, sine));
    cosine = nextCos;
    sine = nextSin;
  }
  return points;
}

export function createStallBullet(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle {
  const bullet = world.createBody({
    type: BodyType.Dynamic,
    isBullet: true,
    position: [0, 1, -10],
    linearVelocity: [0, 0, 600],
    angularVelocity: [0, 0, 20],
  });
  const rock = runtime.createRock(0.25);
  runtime.createShapeFromHull(bullet, rock, {});
  runtime.destroyHull(rock);
  return bullet;
}

export function buildStallScene(world: PhysicsWorld, runtime: Box3DRuntime): { handles: number[]; savedThreshold: number } {
  const savedThreshold = runtime.getStallThreshold();
  // Log any CCD that takes longer than 1 ms (matches upstream Stall).
  runtime.setStallThreshold(0.001);

  const torusMesh = world.createTorusMesh(200, 200, 2, 1);
  const torus = world.createBody({ type: BodyType.Static, position: [0, 2, 0] });
  world.createMeshShape(torus, torusMesh, { scale: [1, 1, 1] });

  const bullet = createStallBullet(world, runtime);
  return { handles: [torus, bullet], savedThreshold };
}

export function buildStallDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  return buildStallScene(world, runtime).handles;
}

export function stallGroundSize(): Vec3 { return [500, 1, 500]; }

export const stallBodies: RenderBody[] = [
  {
    kind: "torus",
    radius: 2,
    tube: 1,
    // Match upstream `b3CreateTorusMesh(200, 200, ...)`.
    radialSegments: 200,
    tubularSegments: 200,
    position: [0, 2, 0],
    type: BodyType.Static,
    color: 0x64748b,
  },
  {
    kind: "hull",
    points: rockHullPoints(0.25),
    position: [0, 1, -10],
    color: 0x78716c,
  },
];

export const stallCamera: RenderSpec["camera"] = cameraFromSetView(130, 15, 15, [0, 2, 0]);

export const dumpSampleName = "Stall";
export const dumpSampleId = "continuous/stall";
export const dumpCppSampleName = "Stall";
export const dumpGroundSize = stallGroundSize;

interface StallDumpState {
  bullet?: BodyHandle;
  savedThreshold: number;
}

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: StallDumpState;
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, stallGroundSize(), {});
  const { handles, savedThreshold } = buildStallScene(world, runtime);
  return {
    world,
    handles: [ground, ...handles],
    state: { bullet: handles[1] as BodyHandle, savedThreshold },
    dispose: () => runtime.setStallThreshold(savedThreshold),
  };
}
