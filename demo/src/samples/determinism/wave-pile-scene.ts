import {BodyType, type BodyId, type Box3DRuntime, type HeightFieldHandle, type PhysicsWorld, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { rockHullPoints } from "../continuous/stall-scene";
import { f32, f32Add, f32Mul, f32Sub } from "../f32";
import { cameraFromSetView } from "../shared";

export const WAVE_PILE_FIELD_COUNT = 21;
export const WAVE_PILE_FIELD_SCALE: Vec3 = [f32(1), f32(0.6), f32(1)];
export const WAVE_PILE_ROW_FREQUENCY = f32(0.08);
export const WAVE_PILE_COLUMN_FREQUENCY = f32(0.06);
export const WAVE_PILE_GRID = 5;
export const WAVE_PILE_LAYERS = 4;
export const WAVE_PILE_BODY_COUNT = 100;

const WAVE_PILE_SEED = 52977;
const WAVE_PILE_SPACING = f32(1.7);
const WAVE_PILE_BOX_HALF: Vec3 = [f32(0.45), f32(0.3), f32(0.55)];
const WAVE_PILE_DYNAMIC_SHAPE = { rollingResistance: f32(0.3) } as const;
const ZERO_JITTER: Vec3 = [0, 0, 0];

export interface WavePileResult {
  ground: BodyId;
  bodies: BodyId[];
  heightField: HeightFieldHandle;
}

export function wavePileExtent(): number {
  return f32Mul(WAVE_PILE_FIELD_SCALE[0], WAVE_PILE_FIELD_COUNT - 1);
}

export function wavePileGroundPosition(): Vec3 {
  const offset = f32Mul(f32(-0.5), wavePileExtent());
  return [offset, 0, offset];
}

function bodyPosition(layer: number, i: number, j: number, jitter: Vec3): Vec3 {
  const halfGrid = f32Mul(f32(0.5), WAVE_PILE_GRID - 1);
  const x = f32Add(f32Mul(WAVE_PILE_SPACING, f32Sub(i, halfGrid)), jitter[0]);
  const y = f32Add(
    f32Add(f32(2.5), f32Mul(f32(1.6), layer)),
    f32Mul(f32(0.3), jitter[1]),
  );
  const z = f32Add(f32Mul(WAVE_PILE_SPACING, f32Sub(j, halfGrid)), jitter[2]);
  return [x, y, z];
}

export function createWavePile(world: PhysicsWorld, runtime: Box3DRuntime): WavePileResult {
  runtime.setRandomSeed(WAVE_PILE_SEED);

  const heightField = world.createWave(
    WAVE_PILE_FIELD_COUNT,
    WAVE_PILE_FIELD_COUNT,
    WAVE_PILE_FIELD_SCALE,
    WAVE_PILE_ROW_FREQUENCY,
    WAVE_PILE_COLUMN_FREQUENCY,
    false,
  );

  const ground = world.createBody({ position: wavePileGroundPosition() });
  world.createHeightFieldShape(ground, heightField, {});

  const rock = runtime.createRock(f32(0.55));
  const bodies: BodyId[] = [];
  try {
    for (let layer = 0; layer < WAVE_PILE_LAYERS; layer++) {
      for (let i = 0; i < WAVE_PILE_GRID; i++) {
        for (let j = 0; j < WAVE_PILE_GRID; j++) {
          const index = bodies.length;
          const body = world.createBody({
            type: BodyType.Dynamic,
            position: bodyPosition(layer, i, j, runtime.randomVec3Uniform(f32(-0.3), f32(0.3))),
            rotation: runtime.randomQuat(),
          });

          switch (index % 4) {
            case 0:
              world.createSphereShape(body, [0, 0, 0], f32(0.5), WAVE_PILE_DYNAMIC_SHAPE);
              break;
            case 1:
              world.createCapsuleShape(body, [0, f32(-0.3), 0], [0, f32(0.3), 0], f32(0.35), WAVE_PILE_DYNAMIC_SHAPE);
              break;
            case 2:
              world.createHullShape(body, WAVE_PILE_BOX_HALF, WAVE_PILE_DYNAMIC_SHAPE);
              break;
            default:
              world.createShapeFromHull(body, rock, WAVE_PILE_DYNAMIC_SHAPE);
              break;
          }

          bodies.push(body);
        }
      }
    }
  } finally {
    runtime.destroyHull(rock);
  }

  return { ground, bodies, heightField };
}

export function wavePileGroundSize(): Vec3 {
  const halfExtent = f32Mul(f32(0.5), wavePileExtent());
  return [halfExtent, f32(1), halfExtent];
}

export function createWavePileBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let layer = 0; layer < WAVE_PILE_LAYERS; layer++) {
    for (let i = 0; i < WAVE_PILE_GRID; i++) {
      for (let j = 0; j < WAVE_PILE_GRID; j++) {
        const index = bodies.length;
        const position = bodyPosition(layer, i, j, ZERO_JITTER);
        switch (index % 4) {
          case 0:
            bodies.push({ kind: "sphere", radius: 0.5, position, color: 0x60a5fa });
            break;
          case 1:
            bodies.push({ kind: "capsule", axis: "y", radius: 0.35, length: 0.6, position, color: 0x34d399 });
            break;
          case 2:
            bodies.push({ kind: "box", size: [0.9, 0.6, 1.1], position, color: 0xfbbf24 });
            break;
          default:
            bodies.push({ kind: "hull", points: rockHullPoints(0.55), position, color: 0xa78bfa });
            break;
        }
      }
    }
  }
  return bodies;
}

export const wavePileCamera: RenderSpec["camera"] = cameraFromSetView(45, 25, 25, [0, 0, 0]);

export const wavePileHeightFieldVisual = {
  rowCount: WAVE_PILE_FIELD_COUNT,
  columnCount: WAVE_PILE_FIELD_COUNT,
  scale: WAVE_PILE_FIELD_SCALE,
  rowFrequency: WAVE_PILE_ROW_FREQUENCY,
  columnFrequency: WAVE_PILE_COLUMN_FREQUENCY,
  position: wavePileGroundPosition(),
} as const;

export const dumpSampleName = "Wave Pile";
export const dumpSampleId = "determinism/wave-pile";
export const dumpCppSampleName = "Wave Pile";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  state: { heightField: HeightFieldHandle };
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, bodies, heightField } = createWavePile(world, runtime);
  return {
    world,
    handles: [ground, ...bodies],
    state: { heightField },
    dispose: () => {
      world.destroyHeightField(heightField);
    },
  };
}
