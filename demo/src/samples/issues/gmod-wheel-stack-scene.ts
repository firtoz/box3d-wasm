import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Mul } from "../f32";
import { METAL_WHEEL1_VERTS } from "./gmod-wheel-verts";

const WHEEL_COUNT = 30;
const HEIGHT = f32(0.171);
const SPACING = f32Add(HEIGHT, f32(0.006));
const START_Y = f32Add(f32Mul(0.5, HEIGHT), f32(0.004));
const FRICTION = f32(0.6);

function wheelRenderPoints(): [number, number, number][] {
  const points: [number, number, number][] = [];
  for (let i = 0; i < METAL_WHEEL1_VERTS.length; i += 3) {
    points.push([METAL_WHEEL1_VERTS[i]!, METAL_WHEEL1_VERTS[i + 1]!, METAL_WHEEL1_VERTS[i + 2]!]);
  }
  return points;
}

export function buildGmodWheelStackDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  // Upstream builds per-piece hulls then wraps their points; wrapping the full vertex
  // table yields the same convex hull used for shapes (multi-piece shapes are commented out).
  const hull = runtime.createHullFromPoints(METAL_WHEEL1_VERTS);
  const handles: BodyHandle[] = [];

  for (let i = 0; i < WHEEL_COUNT; i++) {
    const body = world.createBody({
      type: BodyType.Dynamic,
      position: [0, f32Add(START_Y, f32Mul(i, SPACING)), 0],
    });
    runtime.createShapeFromHull(body, hull, { friction: FRICTION });
    handles.push(body);
  }

  runtime.destroyHull(hull);
  world.setContactTuning(240, 10, 3);
  return handles;
}

export function gmodWheelStackGroundSize(): Vec3 {
  return [10, 1, 10];
}

export function createGmodWheelStackBodies(): RenderBody[] {
  const points = wheelRenderPoints();
  const bodies: RenderBody[] = [];
  for (let i = 0; i < WHEEL_COUNT; i++) {
    bodies.push({
      kind: "hull",
      points,
      position: [0, START_Y + i * SPACING, 0],
      color: 0x94a3b8,
    });
  }
  return bodies;
}

export const gmodWheelStackCamera: RenderSpec["camera"] = cameraFromSetView(0, 12, 5, [0, 0.85, 0]);

export const dumpSampleName = "GMod Wheel Stack";
export const dumpSampleId = "issues/gmod-wheel-stack";
export const dumpCppSampleName = "GMod Wheel Stack";
export const dumpGroundSize = gmodWheelStackGroundSize;
export const dumpBuildDynamicBodies = buildGmodWheelStackDynamicBodies;
