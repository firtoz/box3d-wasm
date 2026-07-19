import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

export function buildJengaStackDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const B3_AXIS_Y: Vec3 = [0, 1, 0];

  for (let i = 0; i < 40; i++) {
    const even = (i & 1) === 0;
    const alpha = even ? 0.5 * Math.PI : 0;
    const q = runtime.makeQuatFromAxisAngle(B3_AXIS_Y, alpha);
    const x = even ? 1.75 : 0;
    const z = even ? 0 : 1.75;
    {
      const body = world.createBody({ type: BodyType.Dynamic, position: [x, 0.5 * i + 0.25, z], rotation: q });
      runtime.createHullShape(body, [2.5, 0.25, 0.25], { rollingResistance: 0.01 });
      handles.push(body);
    }
    {
      const body = world.createBody({ type: BodyType.Dynamic, position: [-x, 0.5 * i + 0.25, -z], rotation: q });
      runtime.createHullShape(body, [2.5, 0.25, 0.25], { rollingResistance: 0.01 });
      handles.push(body);
    }
  }

  return handles;
}

export function jengaStackGroundSize(): Vec3 {
  return [60, 1, 60];
}

export function createJengaStackBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < 40; i++) {
    const even = (i & 1) === 0;
    const a = even ? 0.5 * Math.PI : 0;
    const x = even ? 1.75 : 0;
    const z = even ? 0 : 1.75;
    bodies.push({ kind: "box", size: [5, 0.5, 0.5], position: [x, 0.5 * i + 0.25, z], rotation: [0, Math.sin(a / 2), 0, Math.cos(a / 2)], color: 0xf59e0b });
    bodies.push({ kind: "box", size: [5, 0.5, 0.5], position: [-x, 0.5 * i + 0.25, -z], rotation: [0, Math.sin(a / 2), 0, Math.cos(a / 2)], color: 0xf59e0b });
  }
  return bodies;
}

export const jengaStackCamera: RenderSpec["camera"] = { position: [10, 20, 25], target: [0, 6, 0] };

export const dumpSampleName = "Jenga Stack";
export const dumpSampleId = "jenga-stack";
export const dumpCppSampleName = "Jenga Stack";
export const dumpGroundSize = jengaStackGroundSize;
export const dumpBuildDynamicBodies = buildJengaStackDynamicBodies;
