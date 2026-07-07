import type { BodyType, Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildInclinedPlaneDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const B3_AXIS_X: Vec3 = [1, 0, 0];
  const angle = 40 * (Math.PI / 180);
  const q = runtime.makeQuatFromAxisAngle(B3_AXIS_X, angle);

  const ramp = world.createBody({ type: 0 as BodyType, position: [0, 7.5, -5], rotation: q });
  runtime.createHullShape(ramp, [16, 0.5, 10], { friction: 1 });
  handles.push(ramp);

  for (let i = 0; i < 5; i++) {
    const body = world.createBody({ type: 2 as BodyType, position: [-10 + 5 * i, 15.75, -10.6], rotation: q });
    runtime.createHullShape(body, [1, 1, 1], { friction: (i + 1) * (i + 1) * 0.04 });
    handles.push(body);
  }

  return handles;
}

export function inclinedPlaneGroundSize(): Vec3 {
  return [50, 1, 50];
}

const _a = 40 * (Math.PI / 180);
const _q: [number, number, number, number] = [Math.sin(_a / 2), 0, 0, Math.cos(_a / 2)];
export const inclinedPlaneBodies: RenderBody[] = [
  { kind: "box", size: [32, 1, 20], position: [0, 7.5, -5], rotation: _q, color: 0x94a3b8, type: 0 as BodyType },
  ...Array.from({ length: 5 }, (_, i) => ({
    kind: "box" as const, size: [2, 2, 2] as [number, number, number],
    position: [-10 + 5 * i, 15.75, -10.6] as [number, number, number],
    rotation: _q,
    color: (0x60a5fa + i * 0x050505) as number,
  })),
];

export const inclinedPlaneCamera: RenderSpec["camera"] = { position: [5, 9, 28], target: [0, 8, 0] };

export const dumpSampleName = "Inclined Plane";
export const dumpSampleId = "shapes/inclined-plane";
export const dumpCppSampleName = "Inclined Plane";
export const dumpGroundSize = inclinedPlaneGroundSize;
export const dumpBuildDynamicBodies = buildInclinedPlaneDynamicBodies;
