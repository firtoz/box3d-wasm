import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";

export function buildFilterJointDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const a = objectWorld.createBody({ type: BodyType.Dynamic, position: [2, 4, 0] });
  a.createHullShape([0.5, 0.5, 0.5]);

  const b = objectWorld.createBody({ type: BodyType.Dynamic, position: [-2, 4, 0] });
  b.createHullShape([0.5, 0.5, 0.5]);

  objectWorld.createFilterJoint(a, b);
  return [a.handle, b.handle];
}

export const filterJointGroundSize = (): Vec3 => [20, 1, 20];
export const filterJointVisibleBodies = [
  { index: 0, size: [0.5, 0.5, 0.5], position: [2, 4, 0], color: 0x38bdf8 },
  { index: 1, size: [0.5, 0.5, 0.5], position: [-2, 4, 0], color: 0xf97316 },
] as const;
export const filterJointCamera = { position: [12, 10, 18] as [number, number, number], target: [0, 2, 0] as [number, number, number] };

export const dumpSampleName = "Filter";
export const dumpSampleId = "joints/filter";
export const dumpCppSampleName = "Filter";
export const dumpGroundSize = filterJointGroundSize;
export const dumpBuildDynamicBodies = buildFilterJointDynamicBodies;
