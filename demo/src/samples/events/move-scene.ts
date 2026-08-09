import {BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Div, f32Mul, f32Sub } from "../f32";

const PIVOT: Vec3 = [0, 1, 0];
const BOX_HALF: Vec3 = [0.5, 10, 0.5];
const BOX_OFFSET: Vec3 = [0, 10, 0];
const LINEAR_V: Vec3 = [f32(-10), 0, 0];

export function buildMoveEventDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const body = world.createBody({
    type: BodyType.Dynamic,
    position: PIVOT,
  });
  runtime.setBodyName(body, "big box");

  runtime.createTransformedHullShape(
    body,
    BOX_HALF,
    { position: BOX_OFFSET },
    [1, 1, 1],
    { enableHitEvents: true },
  );

  const center = world.getBodyWorldCenter(body);
  const rx = f32Sub(PIVOT[0], center[0]);
  const ry = f32Sub(PIVOT[1], center[1]);
  const rz = f32Sub(PIVOT[2], center[2]);
  const rr = f32(f32Mul(rx, rx) + f32Mul(ry, ry) + f32Mul(rz, rz));
  if (rr > 0) {
    const invRr = f32Div(1, rr);
    // omega = (1/rr) * cross(v, r); v = {-10,0,0}
    const omega: Vec3 = [
      f32Mul(invRr, f32Sub(f32Mul(LINEAR_V[1], rz), f32Mul(LINEAR_V[2], ry))),
      f32Mul(invRr, f32Sub(f32Mul(LINEAR_V[2], rx), f32Mul(LINEAR_V[0], rz))),
      f32Mul(invRr, f32Sub(f32Mul(LINEAR_V[0], ry), f32Mul(LINEAR_V[1], rx))),
    ];
    runtime.setBodyAngularVelocity(body, omega);
    world.setBodyLinearVelocity(body, LINEAR_V);
  }

  return [body];
}

export function moveEventGroundSize(): Vec3 {
  return [40, 1, 40];
}

export const moveEventBodies: RenderBody[] = [
  {
    kind: "box",
    size: [1, 20, 1],
    position: PIVOT,
    localPosition: BOX_OFFSET,
    color: 0x60a5fa,
  },
];

export const moveEventCamera: RenderSpec["camera"] = cameraFromSetView(0, 30, 40, [0, 5, 0]);

export const dumpSampleName = "Move";
export const dumpSampleId = "events/move";
export const dumpCppSampleName = "Move";
export const dumpGroundSize = moveEventGroundSize;
export const dumpBuildDynamicBodies = buildMoveEventDynamicBodies;
