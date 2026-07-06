import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody } from "./generic-host";

const f = Math.fround;

const scales: [Vec3, Vec3, Vec3, Vec3] = [
  [f(1), f(1), f(1)],
  [f(-0.75), f(1), f(1)],
  [f(1.2), f(1), f(-0.9)],
  [f(0.9), f(0.9), f(0.9)],
];

export function buildCylinderStackDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const hull = runtime.createCylinder(1, 0.5, 0, 15);

  for (let i = 0; i < 10; i++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [0, f(1.1 * i), 0] });
    runtime.createTransformedShapeFromHull(body, hull, { position: [0, 0, 0], rotation: [0, 0, 0, 1] }, scales[i % 4]);
    handles.push(body);
  }

  runtime.destroyHull(hull);
  return handles;
}

export function cylinderStackGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const cylinderStackBodies: RenderBody[] = Array.from({ length: 10 }, (_, i) => {
  const scale = scales[i % 4];
  return {
    kind: "cylinder",
    radius: 0.5,
    height: 1,
    yOffset: 0.5 * Math.abs(scale[1]),
    position: [0, 1.1 * i, 0],
    scale: [Math.abs(scale[0]), Math.abs(scale[1]), Math.abs(scale[2])],
    color: 0x38bdf8,
    type: BodyType.Dynamic,
  };
});

export const dumpSampleName = "Cylinder Stack";
export const dumpSampleId = "cylinder-stack";
export const dumpCppSampleName = "Cylinder Stack";
export const dumpGroundSize = cylinderStackGroundSize;
export const dumpBuildDynamicBodies = buildCylinderStackDynamicBodies;
