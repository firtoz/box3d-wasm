import {
  BodyType,
  type BodyHandle,
  type Box3DRuntime,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderPart, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Mul, f32Sub } from "../f32";

const SHAPE_COUNT = 22;
const SHAPES_PER_BODY = 3;
const GRID_CELL_COUNT = 20;
const GRID_CELL_WIDTH = 8;
const GRID_MATERIAL_COUNT = 6;
const COLOR = 0x60a5fa;

export interface HitEventState {
  mesh: MeshHandle | null;
}

function buildCapsuleChain(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
): { bodies: BodyHandle[]; capsules: { a: Vec3; b: Vec3; radius: number }[][] } {
  let r = f32(0.75);
  let y = r;
  const l = f32(1.5);
  let offset = f32(0.05);
  let velocityScale = f32(0.5);

  const shapeDef = {
    enableHitEvents: true,
    rollingResistance: f32(0.2),
  };

  const origin: Vec3 = [0, 0, 0];
  const bodies: BodyHandle[] = [];
  const capsules: { a: Vec3; b: Vec3; radius: number }[][] = [];
  let currentCapsules: { a: Vec3; b: Vec3; radius: number }[] = [];

  let prevBody: BodyHandle | 0 = 0;
  let body = world.createBody({
    type: BodyType.Dynamic,
    position: origin,
  });
  bodies.push(body);

  for (let i = 0; i < SHAPE_COUNT; i++) {
    const a: Vec3 = [offset, y, 0];
    const b: Vec3 = [0, f32Add(y, l), f32Mul(-1, offset)];
    runtime.createCapsuleShape(body, a, b, r, shapeDef);
    currentCapsules.push({ a, b, radius: r });

    if ((i + 1) % SHAPES_PER_BODY === 0 || i === SHAPE_COUNT - 1) {
      runtime.applyBodyMassFromShapes(body);
      capsules.push(currentCapsules);
      currentCapsules = [];

      const center = world.getBodyWorldCenter(body);
      const omega: Vec3 = [0, 0, f32Mul(-1, velocityScale)];
      const dx = f32Sub(center[0], origin[0]);
      const dy = f32Sub(center[1], origin[1]);
      // cross(omega, center - origin) with omega = (0,0,ωz)
      const v: Vec3 = [f32Mul(-omega[2], dy), f32Mul(omega[2], dx), 0];
      runtime.setBodyAngularVelocity(body, omega);
      world.setBodyLinearVelocity(body, v);

      if (i < SHAPE_COUNT - 1) {
        prevBody = body;
        body = world.createBody({
          type: BodyType.Dynamic,
          position: origin,
        });
        bodies.push(body);

        if (prevBody !== 0) {
          const jointY = f32Add(y, f32Add(l, r));
          world.createWeldJoint(prevBody, body, {
            localFrameA: { position: [0, jointY, 0] },
            localFrameB: { position: [0, jointY, 0] },
            angularHertz: 10,
            angularDampingRatio: 2,
          });
        }

        velocityScale = f32Mul(velocityScale, f32(0.75));
      }
    }

    y = f32Add(y, f32Add(l, f32Mul(2, r)));
    r = f32Mul(f32(0.95), r);
    offset = f32Mul(-1, offset);
  }

  return { bodies, capsules };
}

export function buildHitEventScene(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
): { ground: BodyHandle; bodies: BodyHandle[]; mesh: MeshHandle; capsules: { a: Vec3; b: Vec3; radius: number }[][] } {
  const ground = world.createBody({ type: BodyType.Static });
  const mesh = world.createGridMesh(GRID_CELL_COUNT, GRID_CELL_COUNT, GRID_CELL_WIDTH, GRID_MATERIAL_COUNT, true);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });

  const { bodies, capsules } = buildCapsuleChain(world, runtime);
  return { ground, bodies, mesh, capsules };
}

export function buildHitEventDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  return buildHitEventScene(world, runtime).bodies;
}

export function hitEventGroundSize(): Vec3 {
  return [80, 1, 80];
}

export function createHitEventBodies(): RenderBody[] {
  // Rebuild capsule layout for render (same float32 path as physics).
  let r = f32(0.75);
  let y = r;
  const l = f32(1.5);
  let offset = f32(0.05);
  const perBody: { a: Vec3; b: Vec3; radius: number }[][] = [];
  let current: { a: Vec3; b: Vec3; radius: number }[] = [];

  for (let i = 0; i < SHAPE_COUNT; i++) {
    current.push({
      a: [offset, y, 0],
      b: [0, f32Add(y, l), f32Mul(-1, offset)],
      radius: r,
    });
    if ((i + 1) % SHAPES_PER_BODY === 0 || i === SHAPE_COUNT - 1) {
      perBody.push(current);
      current = [];
    }
    y = f32Add(y, f32Add(l, f32Mul(2, r)));
    r = f32Mul(f32(0.95), r);
    offset = f32Mul(-1, offset);
  }

  return perBody.map((capsules) => ({
    kind: "compound" as const,
    position: [0, 0, 0] as [number, number, number],
    parts: capsules.map((c) => ({
      kind: "ragdoll-capsule" as const,
      a: c.a,
      b: c.b,
      radius: c.radius,
      color: COLOR,
    })) as [RenderPart, ...RenderPart[]],
  }));
}

export const hitEventCamera: RenderSpec["camera"] = cameraFromSetView(0, 30, 100, [0, 5, 0]);

export const dumpSampleName = "Hit";
export const dumpSampleId = "events/hit";
export const dumpCppSampleName = "Hit";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: HitEventState;
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, bodies, mesh } = buildHitEventScene(world, runtime);
  return {
    world,
    handles: [ground, ...bodies],
    state: { mesh },
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}
