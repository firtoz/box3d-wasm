import { B3_AXIS_X, BodyType, quatFromAxisAngle, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type ShapeId, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const radius = 0.1;
const windAngle = 0.25;

export function buildWindDropDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const rotation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, windAngle);

  const body = world.createBody({
    type: BodyType.Dynamic,
    position: [0, 10, 0],
    rotation,
    gravityScale: 0.5,
  });
  runtime.createHullShape(body, [4 * radius, 0.1 * radius, 4 * radius], { density: 2 });
  handles.push(body);

  return handles;
}

interface WindDropState { shapeId: ShapeId | null; }

export function createWindDrop(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[]; state: WindDropState } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, [15, 1, 15], {});
  const handles = buildWindDropDynamicBodies(world, runtime);
  const shapes = world.getBodyShapes(handles[0] as BodyHandle);
  return { world, handles: [ground, ...handles], state: { shapeId: shapes[0] ?? null } };
}

export function windDropGroundSize(): Vec3 { return [15, 1, 15]; }

export function createWindDropBodies(): RenderBody[] {
  const q = quatFromAxisAngle(B3_AXIS_X, windAngle);
  return [
    { kind: "box", size: [8 * radius, 0.2 * radius, 8 * radius], position: [0, 10, 0], rotation: q, color: 0x60a5fa },
  ];
}

export const windDropCamera: RenderSpec["camera"] = { position: [-45, 15, 20], target: [0, 5, 0] };

export const dumpSampleName = "Wind Drop";
export const dumpSampleId = "shapes/wind-drop";
export const dumpCppSampleName = "Wind Drop";
export const dumpCreate = createWindDrop;

export function dumpPostStep(_world: PhysicsWorld, runtime: Box3DRuntime, _handles: readonly number[], _frame: number, _dt: number, state: unknown): void {
  const s = state as WindDropState;
  if (s.shapeId !== null) {
    runtime.applyShapeWind(s.shapeId, [0, 0, 0], 1.0, 4.0, 10.0, false);
  }
}
