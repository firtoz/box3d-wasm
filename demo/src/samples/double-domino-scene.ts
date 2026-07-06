import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "./generic-host";

const dominoHalf: Vec3 = [0.125, 0.5, 0.25];
const f = Math.fround;

export function buildDoubleDominoDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  let x = f(-7.5);

  for (let i = 0; i < 15; i++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [x, 0.5, 0] });
    runtime.createHullShape(body, dominoHalf, { friction: 0.6, density: 4 });
    if (i === 0) world.applyLinearImpulse(body, [0.2, 0, 0], [x, 1.0, 0], true);
    handles.push(body);
    x = f(x + f(1.01));
  }

  return handles;
}

export function doubleDominoGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createDoubleDominoBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  let x = f(-7.5);
  for (let i = 0; i < 15; i++) {
    bodies.push({ kind: "box", size: [0.25, 1.0, 0.5], position: [x, 0.5, 0], color: 0x38bdf8, type: BodyType.Dynamic });
    x = f(x + f(1.01));
  }
  return bodies;
}

export const doubleDominoCamera: RenderSpec["camera"] = { position: [0, 15, 15], target: [0, 0.5, 1.0] };

export const dumpSampleName = "Double Domino";
export const dumpSampleId = "double-domino";
export const dumpCppSampleName = "Double Domino";
export const dumpGroundSize = doubleDominoGroundSize;
export const dumpBuildDynamicBodies = buildDoubleDominoDynamicBodies;
