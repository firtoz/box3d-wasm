import { B3_PI, BodyType, type BodyId, type Box3DRuntime, type CompoundHullEntry, type CompoundSphereEntry, type CompoundCapsuleEntry, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { Box3DRng } from "../box3d-rng";
import { cameraFromSetView } from "../shared";
import { f32Mul, f32Sub } from "../f32";

const GRID_COUNT = 8;
const A = 4;
const HALF: Vec3 = [A, 0.5 * A, A];

function addVec(a: Vec3, b: Vec3): Vec3 {
  return [a[0] + b[0], a[1] + b[1], a[2] + b[2]];
}

export function createVillageParts(rng: Box3DRng): {
  hulls: CompoundHullEntry[];
  spheres: CompoundSphereEntry[];
  capsules: CompoundCapsuleEntry[];
} {
  const hulls: CompoundHullEntry[] = [];
  const spheres: CompoundSphereEntry[] = [];
  const capsules: CompoundCapsuleEntry[] = [];
  for (let i = 0; i < GRID_COUNT; i++) {
    const x = f32Mul(f32Sub(f32Mul(2, i), GRID_COUNT), A);
    for (let j = 0; j < GRID_COUNT; j++) {
      const z = f32Mul(f32Sub(f32Mul(2, j), GRID_COUNT), A);
      const y = f32Mul(rng.randomFloatRange(-0.25, 0.125), A);
      const p: Vec3 = [x, y, z];
      if ((i & 1) && (j & 1)) {
        const p1 = addVec(p, rng.randomVec3([-A, A, -A], [A, 2 * A, A]));
        const p2 = addVec(p, rng.randomVec3([-A, A, -A], [A, 2 * A, A]));
        const radius = rng.randomFloatRange(0.1, 0.5);
        if (capsules.length < spheres.length) capsules.push({ center1: p1, center2: p2, radius });
        else spheres.push({ center: p1, radius });
      }
      hulls.push({ halfWidths: HALF, transform: { position: p, rotation: [0, 0, 0, 1] } });
    }
  }
  return { hulls, spheres, capsules };
}

export function buildVillageDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const rng = new Box3DRng(12345);
  const parts = createVillageParts(rng);
  const compound = runtime.createCompoundParts(parts);
  const rotation = runtime.makeQuatFromAxisAngle([0, 1, 0], f32Mul(-1.15, B3_PI));
  const ground = world.createBody({ type: BodyType.Static, position: [-1, -0.5, 2], rotation });
  runtime.createCompoundShape(ground, compound, 1000);
  runtime.destroyCompound(compound);
  return [ground];
}

export function villageGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createVillageBodies(): RenderBody[] {
  const rng = new Box3DRng(12345);
  const parts = createVillageParts(rng);
  const renderParts = [
    ...parts.hulls.map((h) => ({
      kind: "box" as const,
      size: [2 * h.halfWidths[0], 2 * h.halfWidths[1], 2 * h.halfWidths[2]] as [number, number, number],
      position: h.transform.position as [number, number, number],
      color: 0x64748b,
    })),
    ...parts.spheres.map((s) => ({
      kind: "sphere" as const,
      radius: s.radius,
      position: s.center as [number, number, number],
      color: 0xf59e0b,
    })),
    ...parts.capsules.map((c) => ({
      kind: "capsule" as const,
      axis: "y" as const,
      radius: c.radius,
      length: Math.hypot(c.center2[0] - c.center1[0], c.center2[1] - c.center1[1], c.center2[2] - c.center1[2]),
      position: [
        0.5 * (c.center1[0] + c.center2[0]),
        0.5 * (c.center1[1] + c.center2[1]),
        0.5 * (c.center1[2] + c.center2[2]),
      ] as [number, number, number],
      color: 0x38bdf8,
    })),
  ];
  const first = renderParts[0];
  if (first === undefined) return [];
  return [{
    kind: "compound",
    parts: [first, ...renderParts.slice(1)],
    position: [-1, -0.5, 2],
    type: BodyType.Static,
  }];
}

export const villageCamera: RenderSpec["camera"] = cameraFromSetView(45, 10, 5, [0, 10, 0]);

export const dumpSampleName = "Village";
export const dumpSampleId = "compound/village";
export const dumpCppSampleName = "Village";
export const dumpGroundSize = villageGroundSize;
export const dumpBuildDynamicBodies = buildVillageDynamicBodies;

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildVillageDynamicBodies(world, runtime) };
}
