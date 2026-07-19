import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { Box3DRng } from "../box3d-rng";

const count = 20;

type HullDef = { halfWidths: Vec3; position: Vec3; rotation: [number, number, number, number] };

function createHullDefs(): HullDef[] {
  const rng = new Box3DRng();
  const h = 10;
  const lower: Vec3 = [-h, -h, -h];
  const upper: Vec3 = [h, h, h];
  const defs: HullDef[] = [];
  for (let i = 0; i < count; i++) {
    defs.push({
      halfWidths: [
        rng.randomFloatRange(0.01 * h, 0.05 * h),
        rng.randomFloatRange(0.01 * h, 0.05 * h),
        rng.randomFloatRange(0.01 * h, 0.05 * h),
      ],
      position: rng.randomVec3(lower, upper),
      rotation: rng.randomQuat(),
    });
  }
  return defs;
}

export function buildCompoundHullsDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const defs = createHullDefs();
  const compound = runtime.createCompoundFromHulls(defs.map((def) => ({ halfWidths: def.halfWidths, transform: { position: def.position, rotation: def.rotation } })));
  const body = world.createBody();
  runtime.createCompoundShape(body, compound);
  runtime.destroyCompound(compound);
  return [body];
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildCompoundHullsDynamicBodies(world, runtime) };
}

export function compoundHullsGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createCompoundHullsBodies(): RenderBody[] {
  return [{ kind: "compound", position: [0, 0, 0], type: BodyType.Static, parts: createHullDefs().map((def) => ({ kind: "box", size: [2 * def.halfWidths[0], 2 * def.halfWidths[1], 2 * def.halfWidths[2]], position: def.position, rotation: def.rotation, color: 0x38bdf8 })) as [any, ...any[]] }];
}

export const compoundHullsCamera: RenderSpec["camera"] = { position: [45, 30, 45], target: [0, 0, 0] };

export const dumpSampleName = "Hulls";
export const dumpSampleId = "compound/hulls";
export const dumpCppSampleName = "Hulls";
export const dumpGroundSize = compoundHullsGroundSize;
export const dumpBuildDynamicBodies = buildCompoundHullsDynamicBodies;
