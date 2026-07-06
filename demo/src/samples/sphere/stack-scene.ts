import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";

export function buildSphereStackDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  for (let i = 0, y = 0.75; i < 30; i++, y += 1.5) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [0, y, 0], isAwake: true, angularVelocity: [0, 0, 0] });
    runtime.createSphereShape(body, [0, 0, 0], 0.5, { rollingResistance: 0.1 });
    handles.push(body);
  }
  return handles;
}

export function sphereStackGroundSize(): Vec3 {
  return [15, 1, 15];
}

export const dumpSampleName = "Sphere Stack";
export const dumpSampleId = "sphere-stack";
export const dumpCppSampleName = "Sphere Stack";
export const dumpGroundSize = sphereStackGroundSize;
export const dumpBuildDynamicBodies = buildSphereStackDynamicBodies;
