import { BodyType, type Box3DRuntime, type CompoundMeshEntry, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { Box3DRng } from "../box3d-rng";
import { cameraFromSetView } from "../shared";
import { f32Mul, f32Sub } from "../f32";

const GRID_COUNT = 2;
const A = 4;
const EXTENTS: Vec3 = [A, 0.5 * A, A];
const IDENTITY_ROTATION = [0, 0, 0, 1] as const;

function meshTilePosition(i: number, j: number, rng: Box3DRng): Vec3 {
  return [
    f32Mul(f32Sub(f32Mul(2, i), GRID_COUNT), A),
    f32Mul(rng.randomFloatRange(-0.5, 0.25), A),
    f32Mul(f32Sub(f32Mul(2, j), GRID_COUNT), A),
  ];
}

export function createMeshTilePositions(): Vec3[] {
  const rng = new Box3DRng();
  const positions: Vec3[] = [];
  for (let i = 0; i < GRID_COUNT; i++) {
    for (let j = 0; j < GRID_COUNT; j++) {
      positions.push(meshTilePosition(i, j, rng));
    }
  }
  return positions;
}

function createMeshTileEntries(meshHandle: CompoundMeshEntry["meshHandle"]): CompoundMeshEntry[] {
  return createMeshTilePositions().map((position) => ({
    meshHandle,
    transform: { position, rotation: [...IDENTITY_ROTATION] },
    scale: [1, 1, 1],
  }));
}

export function buildMeshTileDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const mesh = world.createBoxMesh([0, 0, 0], EXTENTS, true);
  const compound = runtime.createCompoundFromMeshes(createMeshTileEntries(mesh));
  world.destroyMesh(mesh);

  const ground = world.createBody({ type: BodyType.Static });
  runtime.createCompoundShape(ground, compound);
  runtime.destroyCompound(compound);
  return [ground];
}

export function createMeshTile(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildMeshTileDynamicBodies(world, runtime) };
}

export function meshTileGroundSize(): Vec3 {
  return [12, 0.5, 12];
}

export function createMeshTileBodies(): RenderBody[] {
  const parts = createMeshTilePositions().map((position) => ({
    kind: "box" as const,
    size: [2 * EXTENTS[0], 2 * EXTENTS[1], 2 * EXTENTS[2]] as [number, number, number],
    position,
    color: 0x94a3b8,
  }));
  return [{ kind: "compound", position: [0, 0, 0], type: BodyType.Static, parts: [parts[0]!, ...parts.slice(1)] }];
}

export const meshTileCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 45, [0, 0, 0]);

export const dumpSampleName = "Mesh Tile";
export const dumpSampleId = "compound/mesh-tile";
export const dumpCppSampleName = "Mesh Tile";
export const dumpCreate = createMeshTile;
