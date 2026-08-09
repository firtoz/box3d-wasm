import { B3_AXIS_Y, B3_PI, BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { f32, f32Add, f32Mul, f32Sub } from "../f32";
import { cameraFromSetView } from "../shared";

/** Match upstream `JengaStack`: 30 layers × 2 hulls, `h=1`, `r=0.1`, rollingResistance 0.05. */
const COUNT = 30;
const H = f32(1);
const R = f32(0.1);
const HALF: Vec3 = [H, R, R];
const FULL: [number, number, number] = [2 * H, 2 * R, 2 * R];
const OFFSET = f32Sub(H, f32Mul(2, R)); // h - 2*r

function layerPose(i: number): { x: number; z: number; alpha: number; y: number } {
  const iF = f32(i);
  const odd = (i & 1) === 1;
  const alpha = odd ? 0 : f32Mul(0.5, B3_PI);
  const x = odd ? 0 : OFFSET;
  const z = odd ? OFFSET : 0;
  const y = f32Mul(f32Add(f32Mul(2.1, iF), 0.5), R);
  return { x, z, alpha, y };
}

export function buildJengaStackDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];

  for (let i = 0; i < COUNT; i++) {
    const { x, z, alpha, y } = layerPose(i);
    const q = runtime.makeQuatFromAxisAngle(B3_AXIS_Y, alpha);

    {
      const body = world.createBody({ type: BodyType.Dynamic, position: [x, y, z], rotation: q });
      runtime.createHullShape(body, HALF, { rollingResistance: 0.05 });
      handles.push(body);
    }
    {
      const body = world.createBody({ type: BodyType.Dynamic, position: [-x, y, -z], rotation: q });
      runtime.createHullShape(body, HALF, { rollingResistance: 0.05 });
      handles.push(body);
    }
  }

  return handles;
}

export function jengaStackGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createJengaStackBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < COUNT; i++) {
    const { x, z, alpha, y } = layerPose(i);
    const halfA = f32Mul(0.5, alpha);
    const rotation: [number, number, number, number] = [0, Math.sin(halfA), 0, Math.cos(halfA)];
    bodies.push({ kind: "box", size: FULL, position: [x, y, z], rotation, color: 0xf59e0b });
    bodies.push({ kind: "box", size: FULL, position: [-x, y, -z], rotation, color: 0xf59e0b });
  }
  return bodies;
}

export const jengaStackCamera: RenderSpec["camera"] = cameraFromSetView(35, 15, 12, [0, 2, 0]);

export const dumpSampleName = "Jenga Stack";
export const dumpSampleId = "jenga-stack";
export const dumpCppSampleName = "Jenga Stack";
export const dumpGroundSize = jengaStackGroundSize;
export const dumpBuildDynamicBodies = buildJengaStackDynamicBodies;
