import { B3_PI, BodyType, type Box3DRuntime, type PhysicsWorld, type Quat, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "./generic-host";
import { cameraFromSetView } from "./shared";
import { f32, f32Add, f32Mul } from "./f32";

const H1: Vec3 = [0.2, 0.02, 0.04];
const H2: Vec3 = [0.2, 0.04, 0.02];
const ANGLE_STEP = f32Mul(0.1, B3_PI);
const ANGLE_LIMIT = f32Add(B3_PI, 0.001);
const IDENTITY: Quat = [0, 0, 0, 1];

type Spawn = {
  position: Vec3;
  rotation: Quat;
  half: Vec3;
};

function edgeCrossingSpawns(runtime: Box3DRuntime): Spawn[] {
  const { direction: axis } = runtime.getLengthAndNormalize([0.1, 0.9, 0]);
  const spawns: Spawn[] = [];

  const pushRow = (z: number, baseHalf: Vec3, fallingHalf: Vec3, baseY: number, fallingY: number) => {
    let x = f32(-10);
    for (let angle = f32(-B3_PI); angle < ANGLE_LIMIT; angle = f32Add(angle, ANGLE_STEP)) {
      spawns.push({ position: [x, baseY, z], rotation: IDENTITY, half: baseHalf });
      spawns.push({
        position: [x, fallingY, z],
        rotation: runtime.makeQuatFromAxisAngle(axis, angle),
        half: fallingHalf,
      });
      x = f32Add(x, 1);
    }
  };

  pushRow(-2, H1, H1, H1[1], f32Mul(20, H1[1]));
  pushRow(0, H2, H2, H2[1], f32Mul(20, H2[1]));
  pushRow(2, H1, H2, H1[1], f32Mul(20, H1[1]));
  return spawns;
}

export function buildEdgeCrossingDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  for (const spawn of edgeCrossingSpawns(runtime)) {
    const body = world.createBody({
      type: BodyType.Dynamic,
      position: spawn.position,
      rotation: spawn.rotation,
    });
    runtime.createHullShape(body, spawn.half);
    handles.push(body);
  }
  return handles;
}

export function edgeCrossingGroundSize(): Vec3 {
  return [40, 1, 40];
}

export function createEdgeCrossingBodies(runtime: Box3DRuntime): RenderBody[] {
  return edgeCrossingSpawns(runtime).map((spawn, i) => ({
    kind: "box" as const,
    size: [2 * spawn.half[0], 2 * spawn.half[1], 2 * spawn.half[2]] as [number, number, number],
    position: spawn.position,
    rotation: spawn.rotation,
    color: i % 2 === 0 ? 0x94a3b8 : 0xf59e0b,
  }));
}

export const edgeCrossingCamera: RenderSpec["camera"] = cameraFromSetView(0, 25, 10, [0, 0, 0]);

export const dumpSampleName = "Edge Crossing";
export const dumpSampleId = "edge-crossing";
export const dumpCppSampleName = "Edge Crossing";
export const dumpGroundSize = edgeCrossingGroundSize;
export const dumpBuildDynamicBodies = buildEdgeCrossingDynamicBodies;
