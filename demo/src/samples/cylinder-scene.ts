import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";

export function buildCylinderDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const hull = runtime.createCylinder(1, 0.25, 0, 12);
  const body = world.createBody({ type: BodyType.Dynamic, position: [0, 2, 0], isAwake: true, linearVelocity: [0, 0, 0] });
  runtime.createShapeFromHull(body, hull, { rollingResistance: 0.05 });
  runtime.destroyHull(hull);
  return [body];
}

export function cylinderGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const dumpSampleName = "Cylinder";
export const dumpSampleId = "cylinder";
export const dumpCppSampleName = "Cylinder";
export const dumpGroundSize = cylinderGroundSize;
export const dumpBuildDynamicBodies = buildCylinderDynamicBodies;
