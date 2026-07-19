import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { Box3DRng } from "../box3d-rng";

const count = 20;

type SphereDef = { center: Vec3; radius: number };

function createSphereDefs(): SphereDef[] {
  const rng = new Box3DRng();
  const h = 10;
  const lower: Vec3 = [-h, -h, -h];
  const upper: Vec3 = [h, h, h];
  const defs: SphereDef[] = [];
  for (let i = 0; i < count; i++) {
    defs.push({
      center: rng.randomVec3(lower, upper),
      radius: rng.randomFloatRange(0.01 * h, 0.05 * h),
    });
  }
  return defs;
}

export function buildCompoundSpheresDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const defs = createSphereDefs();
  const compound = runtime.createCompoundFromSpheres(defs.map((def) => ({ center: def.center, radius: def.radius })));
  const body = world.createBody();
  runtime.createCompoundShape(body, compound);
  runtime.destroyCompound(compound);
  return [body];
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildCompoundSpheresDynamicBodies(world, runtime) };
}

export function compoundSpheresGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createCompoundSpheresBodies(): RenderBody[] {
  return [{ kind: "compound", position: [0, 0, 0], type: BodyType.Static, parts: createSphereDefs().map((def) => ({ kind: "sphere", radius: def.radius, position: def.center, color: 0x38bdf8 })) as [any, ...any[]] }];
}

export const compoundSpheresCamera: RenderSpec["camera"] = { position: [45, 30, 45], target: [0, 0, 0] };

export const dumpSampleName = "Spheres";
export const dumpSampleId = "compound/spheres";
export const dumpCppSampleName = "Spheres";
export const dumpGroundSize = compoundSpheresGroundSize;
export const dumpBuildDynamicBodies = buildCompoundSpheresDynamicBodies;
