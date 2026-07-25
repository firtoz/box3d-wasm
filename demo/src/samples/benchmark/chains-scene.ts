import {
  BodyType,
  type BodyHandle,
  type Box3DRuntime,
  type MeshHandle,
  type PhysicsWorld,
  type ShapeHandle,
  type Vec3,
} from "box3d-wasm";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Mul, f32Sub } from "../f32";

/** Match upstream Release (`m_isDebug=false`). */
export const CHAINS_GRID_COUNT = 25;
export const CHAINS_LINK_COUNT = 4;
export const CHAINS_LINK_RADIUS = f32(0.125);
export const CHAINS_LINK_EXTENT = f32(0.25);
export const CHAINS_BODY_COUNT = CHAINS_GRID_COUNT * CHAINS_GRID_COUNT * CHAINS_LINK_COUNT;
export const CHAINS_COLOR = 0x60a5fa;

const WIND_BASE: Vec3 = [f32(20), 0, 0];
const DRAG = f32(1);
const LIFT = f32(1);
const MAX_SPEED = f32(20);
const NOISE_ALPHA = f32(0.05);

export interface ChainsState {
  tipShapeIds: ShapeHandle[];
  noise: Vec3;
  mesh: MeshHandle | null;
}

export function buildChainsScene(
  world: PhysicsWorld,
  _runtime: Box3DRuntime,
): { ground: BodyHandle; links: BodyHandle[]; state: ChainsState } {
  const ground = world.createBody({ type: BodyType.Static });
  const mesh = world.createWaveMesh(80, 80, 1, 0.5, 0.05, 0.01);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });

  const tipShapeIds: ShapeHandle[] = [];
  const links: BodyHandle[] = [];
  const linkExtent = CHAINS_LINK_EXTENT;
  const linkRadius = CHAINS_LINK_RADIUS;

  let x = f32Mul(-1, CHAINS_GRID_COUNT);
  for (let rowIndex = 0; rowIndex < CHAINS_GRID_COUNT; rowIndex++) {
    let z = f32Mul(-1, CHAINS_GRID_COUNT);
    for (let columnIndex = 0; columnIndex < CHAINS_GRID_COUNT; columnIndex++) {
      let prevBody: BodyHandle | null = null;
      for (let i = 0; i < CHAINS_LINK_COUNT; i++) {
        const y = f32Add(f32Mul(f32Sub(1, f32Mul(2, i)), linkExtent), 3);
        const body = world.createBody({
          type: i === 0 ? BodyType.Static : BodyType.Dynamic,
          position: [x, y, z],
          enableSleep: false,
        });
        const shape = world.createCapsuleShape(body, [0, -linkExtent, 0], [0, linkExtent, 0], linkRadius);
        links.push(body);

        if (i === CHAINS_LINK_COUNT - 1) {
          tipShapeIds.push(shape);
        }

        if (i > 0 && prevBody !== null) {
          world.createSphericalJoint(prevBody, body, {
            localFrameA: { position: [0, -linkExtent, 0] },
            localFrameB: { position: [0, linkExtent, 0] },
            enableSpring: true,
            hertz: 1,
            dampingRatio: 0.7,
            enableMotor: true,
            maxMotorTorque: 1,
          });
        }

        prevBody = body;
      }
      z = f32Add(z, 2);
    }
    x = f32Add(x, 2);
  }

  return {
    ground,
    links,
    state: { tipShapeIds, noise: [0, 0, 0], mesh },
  };
}

export function chainsGroundSize(): Vec3 {
  return [40, 1, 40];
}

export function forEachChainsLink(callback: (position: Vec3, color: number) => void): void {
  let x = -1 * CHAINS_GRID_COUNT;
  for (let rowIndex = 0; rowIndex < CHAINS_GRID_COUNT; rowIndex++) {
    let z = -1 * CHAINS_GRID_COUNT;
    for (let columnIndex = 0; columnIndex < CHAINS_GRID_COUNT; columnIndex++) {
      for (let i = 0; i < CHAINS_LINK_COUNT; i++) {
        const y = (1 - 2 * i) * CHAINS_LINK_EXTENT + 3;
        callback([x, y, z], CHAINS_COLOR);
      }
      z += 2;
    }
    x += 2;
  }
}

export const chainsCamera = cameraFromSetView(0, 15, 50, [0, 5, 0]);

export const dumpSampleName = "Chains";
export const dumpSampleId = "benchmark/chains";
export const dumpCppSampleName = "Chains";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: ChainsState;
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, links, state } = buildChainsScene(world, runtime);
  return {
    world,
    handles: [ground, ...links],
    state,
    dispose: () => {
      if (state.mesh !== null) world.destroyMesh(state.mesh);
    },
  };
}

/** Wind runs before the physics step (matches BenchmarkChains::Step). */
export function dumpStep(
  _world: PhysicsWorld,
  runtime: Box3DRuntime,
  _handles: readonly BodyHandle[],
  _frame: number,
  _dt: number,
  state: ChainsState,
): void {
  const { length: speed, direction } = runtime.getLengthAndNormalize(WIND_BASE);
  const windVec: Vec3 = [
    f32(speed * f32(direction[0] + state.noise[0])),
    f32(speed * f32(direction[1] + state.noise[1])),
    f32(speed * f32(direction[2] + state.noise[2])),
  ];
  for (const shapeId of state.tipShapeIds) {
    runtime.applyShapeWind(shapeId, windVec, DRAG, LIFT, MAX_SPEED, false);
  }
  const rand = runtime.randomVec3([-0.3, -0.3, -0.3], [0.3, 0.3, 0.3]);
  state.noise = runtime.lerpVec3(state.noise, rand, NOISE_ALPHA);
}
