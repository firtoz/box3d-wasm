import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const B3_AXIS_Y: Vec3 = [0, 1, 0];

export function buildCompoundSimpleDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];

  const compound = runtime.createCompoundFromHulls([
    {
      halfWidths: [4, 0.5, 4],
      transform: { position: [1, -0.5, 0], rotation: [0, 0, 0, 1] },
      friction: 0.6,
    },
  ]);

  const q = runtime.makeQuatFromAxisAngle(B3_AXIS_Y, 0.25 * Math.PI);
  const body = world.createBody({ type: BodyType.Static, position: [2, -1, 0], rotation: q });
  runtime.createCompoundShape(body, compound);
  runtime.destroyCompound(compound);
  handles.push(body);

  world.setContactRecycleDistance(0);

  const sphere = world.createBody({ type: BodyType.Dynamic, position: [0, 2, 0], isAwake: true });
  runtime.createSphereShape(sphere, [0, 0, 0], 0.25);
  handles.push(sphere);

  return handles;
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildCompoundSimpleDynamicBodies(world, runtime) };
}

export function compoundSimpleGroundSize(): Vec3 {
  return [10, 1, 10];
}

export const compoundSimpleBodies: RenderBody[] = [
  {
    kind: "compound", position: [2, -1, 0], rotation: [0, 0.382683, 0, 0.92388], type: BodyType.Static,
    parts: [
      { kind: "box", size: [8, 1, 8], position: [1, -0.5, 0], color: 0x223047 },
    ],
  },
  { kind: "sphere", radius: 0.25, position: [0, 2, 0], color: 0xf59e0b },
];

export const compoundSimpleCamera: RenderSpec["camera"] = { position: [27.55, 22.5, 27.55], target: [0, 0, 0] };

export const dumpSampleName = "Simple";
export const dumpSampleId = "compound/simple";
export const dumpCppSampleName = "Simple";
