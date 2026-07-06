import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";

export function buildSpinningBookDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const halfWidths: Vec3 = [0.35, 0.08, 0.5];
  const bodies: Array<{ position: Vec3; angularVelocity: Vec3 }> = [
    { position: [-2, 2, 0], angularVelocity: [5, 0.01, 0.01] },
    { position: [0, 2, 0], angularVelocity: [0.01, 5, 0.01] },
    { position: [2, 2, 0], angularVelocity: [0.01, 0.01, -5] },
  ];
  for (const def of bodies) {
    const body = world.createBody({ type: BodyType.Dynamic, position: def.position, gravityScale: 0, angularVelocity: def.angularVelocity, isAwake: true });
    runtime.createHullShape(body, halfWidths);
    handles.push(body);
  }
  return handles;
}

export function spinningBookGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const dumpSampleName = "Spinning Book";
export const dumpSampleId = "bodies/spinning-book";
export const dumpCppSampleName = "Spinning Book";
export const dumpGroundSize = spinningBookGroundSize;
export const dumpBuildDynamicBodies = buildSpinningBookDynamicBodies;
