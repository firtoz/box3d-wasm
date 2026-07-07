import { BodyType, type Box3DRuntime, type CompoundHullEntry, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { Box3DRng } from "../box3d-rng";

const GRID_COUNT = 50;
const A = 4;
const half: Vec3 = [A, 0.5 * A, A];

function createTileEntries(): CompoundHullEntry[] {
  const rng = new Box3DRng();
  const entries: CompoundHullEntry[] = [];
  for (let i = 0; i < GRID_COUNT; i++) {
    const x = (2 * i - GRID_COUNT) * A;
    for (let j = 0; j < GRID_COUNT; j++) {
      const z = (2 * j - GRID_COUNT) * A;
      const y = rng.randomFloatRange(-0.5, 0.25) * A;
      entries.push({ halfWidths: half, transform: { position: [x, y, z], rotation: [0, 0, 0, 1] } });
    }
  }
  return entries;
}

export function buildTileFloorDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const compound = runtime.createCompoundFromHulls(createTileEntries());
  const ground = world.createBody({ type: BodyType.Static, position: [-2, 1, -3] });
  runtime.createCompoundShape(ground, compound);
  runtime.destroyCompound(compound);
  handles.push(ground);

  const sphere = world.createBody({ type: BodyType.Dynamic, position: [3, 12, 0] });
  runtime.createSphereShape(sphere, [0, 0, 0], 0.25, {});
  handles.push(sphere);

  return handles;
}

export function tileFloorGroundSize(): Vec3 { return [20, 1, 20]; }

export function createTileFloorBodies(): RenderBody[] {
  return [
    { kind: "sphere", radius: 0.25, position: [3, 12, 0], color: 0xf59e0b },
  ];
}

export const tileFloorCamera: RenderSpec["camera"] = { position: [45, 30, 45], target: [0, 2, 0] };

export const dumpSampleName = "Tile Floor";
export const dumpSampleId = "compound/tile-floor";
export const dumpCppSampleName = "Tile Floor";
export const dumpGroundSize = tileFloorGroundSize;
export const dumpBuildDynamicBodies = buildTileFloorDynamicBodies;

export function createTileFloor(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildTileFloorDynamicBodies(world, runtime) };
}

export const dumpCreate = createTileFloor;
