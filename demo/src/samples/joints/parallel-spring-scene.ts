import {
  B3_AXIS_X,
  B3_AXIS_Y,
  B3_AXIS_Z,
  B3_PI,
  BodyType,
  quatFromAxisAngle,
  type BodyHandle,
  type Box3DRuntime,
  type PhysicsWorld,
  type Quat,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Mul } from "../f32";

const HERTZ = f32(10);
const DAMPING_RATIO = f32(0.7);
const BOX_HALF: Vec3 = [0.5, 1.5, 0.25];
const BODY_POS: Vec3 = [0, 4, 0];
export const PARALLEL_SPRING_BODY_ROTATION = quatFromAxisAngle(B3_AXIS_X, f32Mul(0.25, B3_PI));
/** Local frame A maps joint Z→world Y when body A has identity rotation (matches `computeQuatBetweenUnitVectors(Z,Y)`). */
export const PARALLEL_SPRING_LOCAL_FRAME_A: Quat = quatFromAxisAngle(B3_AXIS_X, -0.5 * B3_PI);
/** C++ `drawScale = 2` → segment length `0.1 * jointScale * drawScale`. */
export const PARALLEL_SPRING_DRAW_LENGTH = 0.2;

export function buildParallelSpringDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const walls = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createTransformedHullShape(walls, [20, 5, 0.1], { position: [0, 5, -20] });
  runtime.createTransformedHullShape(walls, [20, 5, 0.1], { position: [0, 5, 20] });
  runtime.createTransformedHullShape(walls, [0.1, 5, 20], { position: [-20, 5, 0] });
  runtime.createTransformedHullShape(walls, [0.1, 5, 20], { position: [20, 5, 0] });

  const rotation = runtime.makeQuatFromAxisAngle(B3_AXIS_X, f32Mul(0.25, B3_PI));
  const body = world.createBody({
    type: BodyType.Dynamic,
    position: BODY_POS,
    rotation,
  });
  runtime.createHullShape(body, BOX_HALF);

  // Prefer WASM for dump parity; overlay uses PARALLEL_SPRING_LOCAL_FRAME_A + initial body rot.
  const frameA: Quat = runtime.computeQuatBetweenUnitVectors(B3_AXIS_Z, B3_AXIS_Y);
  const frameB: Quat = runtime.invMulQuat(rotation, frameA);

  world.createParallelJoint(walls, body, {
    localFrameA: { rotation: frameA },
    localFrameB: { rotation: frameB },
    hertz: HERTZ,
    dampingRatio: DAMPING_RATIO,
    collideConnected: true,
  });

  return [walls, body];
}

export function parallelSpringGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const parallelSpringBodies: RenderBody[] = [
  {
    kind: "compound",
    type: BodyType.Static,
    position: [0, -1, 0],
    parts: [
      { kind: "box", size: [40, 10, 0.2], position: [0, 5, -20], color: 0x64748b },
      { kind: "box", size: [40, 10, 0.2], position: [0, 5, 20], color: 0x64748b },
      { kind: "box", size: [0.2, 10, 40], position: [-20, 5, 0], color: 0x64748b },
      { kind: "box", size: [0.2, 10, 40], position: [20, 5, 0], color: 0x64748b },
    ],
  },
  {
    kind: "box",
    size: [1, 3, 0.5],
    position: BODY_POS,
    rotation: PARALLEL_SPRING_BODY_ROTATION,
    color: 0x38bdf8,
  },
];

export const parallelSpringCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 15, [0, 2, 0]);

export const dumpSampleName = "Parallel Spring";
export const dumpSampleId = "joints/parallel-spring";
export const dumpCppSampleName = "Parallel Spring";
export const dumpGroundSize = parallelSpringGroundSize;
export const dumpBuildDynamicBodies = buildParallelSpringDynamicBodies;
