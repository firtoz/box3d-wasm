import {BodyType, type BodyId, type Box3DRuntime, type MeshHandle, type PhysicsWorld, type ShapeDef, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { Box3DRng } from "../box3d-rng";
import { cameraFromSetView } from "../shared";
import { f32, f32Mul, f32Sub } from "../f32";
import { MESH_DROP_WAVE } from "../mesh-drop-shared";

export type ContinuousMeshDropShapeType = "box" | "capsule" | "cylinder" | "sphere";
export const CONTINUOUS_MESH_DROP_SHAPE_TYPES: ContinuousMeshDropShapeType[] = ["box", "capsule", "cylinder", "sphere"];

/** Continuous Mesh Drop defaults: 32×32 grid, walls, fixed dump seed. */
export const CONTINUOUS_MESH_DROP_GRID = 32;
/** Commented fixed seed in upstream `MeshDrop::Generate`. */
export const CONTINUOUS_MESH_DROP_SEED = 1910133196;

const BOX_HALF: Vec3 = [f32(0.02), f32(0.2), f32(0.04)];
const GROUND_AMP = f32(0.5);

export interface ContinuousMeshDropResult {
  ground: BodyId;
  bodies: BodyId[];
  mesh: MeshHandle;
}

function createGroundWithWalls(world: PhysicsWorld, runtime: Box3DRuntime, mesh: MeshHandle): BodyId {
  const ground = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  const filter = { categoryBits: 1 };
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1], ...filter });

  const gridCount = MESH_DROP_WAVE.xCount;
  const cellWidth = MESH_DROP_WAVE.cellWidth;
  const extent = f32Mul(0.5, f32Mul(gridCount, cellWidth));
  const halfHeight = f32(1);

  // Walls match Continuous MeshDrop::CreateGround (transformed box hulls).
  runtime.createTransformedHullShape(
    ground,
    [extent, halfHeight, f32(0.1)],
    { position: [0, halfHeight, f32Mul(-1, extent)] },
    [1, 1, 1],
    filter,
  );
  runtime.createTransformedHullShape(
    ground,
    [extent, halfHeight, f32(0.1)],
    { position: [0, halfHeight, extent] },
    [1, 1, 1],
    filter,
  );
  runtime.createTransformedHullShape(
    ground,
    [f32(0.1), halfHeight, extent],
    { position: [f32Mul(-1, extent), halfHeight, 0] },
    [1, 1, 1],
    filter,
  );
  runtime.createTransformedHullShape(
    ground,
    [f32(0.1), halfHeight, extent],
    { position: [extent, halfHeight, 0] },
    [1, 1, 1],
    filter,
  );

  return ground;
}

export interface ContinuousMeshDropOptions {
  seed?: number;
  amplitude?: number;
  collide?: boolean;
  shapeType?: ContinuousMeshDropShapeType;
}

/**
 * Continuous / Mesh Drop with defaults: box shape, collide=true (no self-filter),
 * RR 0.1, amplitude 0.5, fixed seed for dump/demo parity.
 */
export function createContinuousMeshDrop(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  seedOrOptions: number | ContinuousMeshDropOptions = CONTINUOUS_MESH_DROP_SEED,
): ContinuousMeshDropResult {
  const options: ContinuousMeshDropOptions = typeof seedOrOptions === "number" ? { seed: seedOrOptions } : seedOrOptions;
  const seed = options.seed ?? CONTINUOUS_MESH_DROP_SEED;
  const amplitude = options.amplitude ?? GROUND_AMP;
  const collide = options.collide ?? true;
  const shapeType = options.shapeType ?? "box";
  const mesh = world.createWaveMesh(
    MESH_DROP_WAVE.xCount,
    MESH_DROP_WAVE.zCount,
    MESH_DROP_WAVE.cellWidth,
    amplitude,
    MESH_DROP_WAVE.rowFrequency,
    MESH_DROP_WAVE.columnFrequency,
  );
  const ground = createGroundWithWalls(world, runtime, mesh);

  const rng = new Box3DRng(seed);
  const grid = CONTINUOUS_MESH_DROP_GRID;
  const halfGrid = f32Mul(0.5, grid);
  const bodies: BodyId[] = [];
  const rollingResistance = shapeType === "capsule" ? 0.4 : 0.1;
  const filter: ShapeDef = collide ? {} : { categoryBits: 2, maskBits: 1 };
  const shapeDef: ShapeDef = { rollingResistance, ...filter };
  const cylinder = shapeType === "cylinder" ? runtime.createCylinder(0.4, 0.05, 0, 6) : null;

  // m_collide = true → do not apply category/mask filter (boxes collide with each other).
  for (let i = 0; i < grid; i++) {
    for (let j = 0; j < grid; j++) {
      const linearVelocity = rng.randomVec3Uniform(-1, 1);
      const angularVelocity = rng.randomVec3Uniform(-5, 5);
      const position: Vec3 = [
        f32Mul(0.5, f32Sub(f32(i), halfGrid)),
        f32(5),
        f32Mul(0.5, f32Sub(f32(j), halfGrid)),
      ];
      const body = world.createBody({
        type: BodyType.Dynamic,
        position,
        linearVelocity,
        angularVelocity,
      });
      if (shapeType === "capsule") {
        world.createCapsuleShape(body, [0, f32(-0.2), 0], [0, f32(0.2), 0], f32(0.05), shapeDef);
      } else if (shapeType === "sphere") {
        world.createSphereShape(body, [0, 0, 0], f32(0.05), shapeDef);
      } else if (shapeType === "cylinder" && cylinder !== null) {
        runtime.createShapeFromHull(body, cylinder, shapeDef);
      } else {
        runtime.createHullShape(body, BOX_HALF, shapeDef);
      }
      bodies.push(body);
    }
  }

  if (cylinder !== null) runtime.destroyHull(cylinder);
  return { ground, bodies, mesh };
}

export function continuousMeshDropGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function continuousMeshDropBodyFor(
  shapeType: ContinuousMeshDropShapeType,
  position: [number, number, number],
): RenderBody {
  const color = 0x60a5fa;
  if (shapeType === "capsule") {
    return { kind: "capsule", radius: 0.05, length: 0.4, axis: "y", position, color };
  }
  if (shapeType === "sphere") {
    return { kind: "sphere", radius: 0.05, position, color };
  }
  if (shapeType === "cylinder") {
    return { kind: "cylinder", radius: 0.05, height: 0.4, segments: 6, yOffset: 0.2, position, color };
  }
  return {
    kind: "box",
    size: [2 * BOX_HALF[0], 2 * BOX_HALF[1], 2 * BOX_HALF[2]],
    position,
    color,
  };
}

export function createContinuousMeshDropBodies(shapeType: ContinuousMeshDropShapeType = "box"): RenderBody[] {
  const grid = CONTINUOUS_MESH_DROP_GRID;
  const halfGrid = f32Mul(0.5, grid);
  const extent = f32Mul(0.5, f32Mul(MESH_DROP_WAVE.xCount, MESH_DROP_WAVE.cellWidth));
  const halfHeight = f32(1);
  const bodies: RenderBody[] = [
    {
      kind: "compound",
      position: [0, 0, 0],
      type: BodyType.Static,
      parts: [
        { kind: "box", size: [2 * extent, 2 * halfHeight, 0.2], position: [0, halfHeight, -extent], color: 0x94a3b8 },
        { kind: "box", size: [2 * extent, 2 * halfHeight, 0.2], position: [0, halfHeight, extent], color: 0x94a3b8 },
        { kind: "box", size: [0.2, 2 * halfHeight, 2 * extent], position: [-extent, halfHeight, 0], color: 0x94a3b8 },
        { kind: "box", size: [0.2, 2 * halfHeight, 2 * extent], position: [extent, halfHeight, 0], color: 0x94a3b8 },
      ],
    },
  ];
  for (let i = 0; i < grid; i++) {
    for (let j = 0; j < grid; j++) {
      bodies.push(continuousMeshDropBodyFor(shapeType, [
        f32Mul(0.5, f32Sub(f32(i), halfGrid)),
        f32(5),
        f32Mul(0.5, f32Sub(f32(j), halfGrid)),
      ]));
    }
  }
  return bodies;
}

export const continuousMeshDropCamera: RenderSpec["camera"] = cameraFromSetView(0, 30, 20, [0, 0, 0]);

export function continuousMeshDropWaveParams(amplitude: number = GROUND_AMP) {
  return {
    ...MESH_DROP_WAVE,
    amplitude,
    position: [0, 0, 0] as [number, number, number],
  };
}

export const dumpSampleName = "Continuous Mesh Drop";
export const dumpSampleId = "continuous/mesh-drop";
export const dumpCppSampleName = "Continuous Mesh Drop";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  state: { mesh: MeshHandle };
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, bodies, mesh } = createContinuousMeshDrop(world, runtime);
  return {
    world,
    handles: [ground, ...bodies],
    state: { mesh },
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}
